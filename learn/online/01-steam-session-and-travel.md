# Steam Session、OnlineSubsystem 与关卡 Travel

## 1. 本节目标

把三个经常被混在一起的概念分开：

- Session：玩家如何发现和加入一个在线房间；
- Travel：玩家如何从大厅进入游戏地图；
- Gameplay Replication：进入同一 World 后，Actor 和状态如何同步。

## 2. 项目配置

### `.uproject`

`Blaster.uproject` 启用 `OnlineSubsystemSteam`、`SteamSockets`、`PCG` 和 `PCGGeometryScriptInterop`。`AdditionalDependencies` 中包含 Engine 和 UMG。

### `DefaultEngine.ini`

项目把 `GameNetDriver` 指向 `SteamSocketsNetDriver`，并设置 OnlineSubsystem 的默认平台服务为 Steam；同时保留 `IpNetDriver` 作为 fallback。启动地图和过渡地图也在这里配置：

```text
MapStartUp -> Lobby -> TransitionLevel -> BlasterMap
```

配置存在不等于公网联机已验证。当前项目文档明确记录的是本机 Listen Server + 1 Client PIE；跨机器 Steam、公网、专用服务器和断线重连仍要单独验收。

## 3. 自定义 MultiplayerSessions 插件

插件位于 `Plugins/MultiplayerSessions/`，核心类型是 `UMultiplayerSessionsSubsystem`，继承 `UGameInstanceSubsystem`。

为什么用 GameInstanceSubsystem？

- GameInstance 跨地图生命周期稳定；
- Session 不应依附某一张 Gameplay Map 或某一个 Pawn；
- 菜单 Widget 可以通过 `GetGameInstance()->GetSubsystem<...>()` 获取它；
- Session 操作是异步的，Subsystem 适合集中保存接口和委托句柄。

插件公共依赖包括 `OnlineSubsystem`、`OnlineSubsystemSteam`、`SteamSockets`、`UMG`、`Slate` 和 `SlateCore`。

## 4. Host 流程

```text
UMenu::HostButtonClicked
        |
        v
UMultiplayerSessionsSubsystem::CreateSession
        |
        +--> 已有 Session：DestroySession 后重新 Create
        +--> 创建 FOnlineSessionSettings
        |      bShouldAdvertise / bAllowJoinInProgress / Presence / Connections
        |
        v
OnlineSubsystem SessionInterface 异步回调
        |
        v
UMenu::OnCreateSession
        |
        v
World->ServerTravel(PathToLobby + "?listen")
```

`CreateSession` 只是在在线层创建房间；真正让 Host 进入大厅的是回调里的 `ServerTravel`。Session 发现和地图 Travel 是两个阶段。

## 5. Join 流程

```text
UMenu::JoinButtonClicked
        |
        v
FindSessions(MaxSearchResults)
        |
        v
OnFindSessions：按 MatchType 过滤
        |
        v
JoinSession(SessionResult)
        |
        v
OnJoinSession：GetResolvedConnectString
        |
        v
PlayerController->ClientTravel(Address)
```

Session 的完成回调必须清理对应 delegate handle，避免重复回调和生命周期泄漏。`UMenu::NativeDestruct` 还要恢复输入模式并处理 UI 生命周期。

## 6. Lobby 到比赛地图

`ALobbyGameMode::PostLogin` 观察 `GameState->PlayerArray`。满足两名玩家后，Server 开启 `bUseSeamlessTravel` 并调用：

```cpp
World->ServerTravel("/Game/Maps/BlasterMap?listen");
```

这一步是 Server 主导的地图切换，不等同于玩家通过 Online Session 加入。进入 `BlasterMap` 后，`ABlasterGameMode` 才负责出生点、默认武器、比赛阶段、伤害、计分和重生。

## 7. Session、Travel、Replication 的边界

| 问题 | 负责系统 |
| --- | --- |
| 房间是否可被发现 | OnlineSubsystem/Session Interface |
| 玩家如何获得连接地址 | Session Resolve Connect String |
| Host/Join 如何进入某张地图 | ServerTravel/ClientTravel/SeamlessTravel |
| 进入地图后谁能改 Health/Ammo | Gameplay Server Authority |
| 远端客户端如何看到角色和状态 | Actor/Component Replication |

面试中如果只说“Steam 负责同步所有东西”，说明边界还不清楚。Steam/OnlineSubsystem 负责在线服务和连接层；游戏规则和 Actor 状态仍由 UE 网络层和游戏 Server 负责。

## 8. UE5.6 与现实验证边界

- `OnlineSubsystemSteam`、`SteamSockets` 的配置要和引擎插件版本匹配；不要只凭旧课程的 `.ini` 片段。
- PIE 中使用 NULL/本地网络或同进程测试，不等价于真实 Steam 跨机器会话。
- `MapStartUp`、`Lobby`、`TransitionLevel` 和 `BlasterMap` 都要在 Content 中真实存在并保存。
- Online delegate 的失败路径要恢复按钮状态，避免用户看到按钮永久禁用。

## 9. 引擎源码阅读路线

- Session 接口：`Engine/Source/Runtime/OnlineSubsystem/Public/Interfaces/OnlineSessionInterface.h`
- Session 设置：`Engine/Source/Runtime/OnlineSubsystem/Public/OnlineSessionSettings.h`
- GameInstanceSubsystem：`Engine/Source/Runtime/Engine/Classes/Subsystems/GameInstanceSubsystem.h`
- Travel：`Engine/Source/Runtime/Engine/Classes/Engine/World.h`、`GameFramework/PlayerController.h`
- Seamless Travel：`Engine/Source/Runtime/Engine/Private/World.cpp`、`GameModeBase.cpp`

## 10. 验证步骤

1. 启动 `MapStartUp`，确认菜单能拿到 Session Subsystem。
2. Host：点击 Host，观察 `CreateSession` 回调和 Lobby Travel。
3. Join：搜索 Session，按 MatchType 过滤，解析地址并 ClientTravel。
4. 两名玩家进入 Lobby 后，观察 Server 是否只触发一次 SeamlessTravel。
5. 进入 `BlasterMap` 后再验证 Gameplay Replication，而不是把“能进地图”当成“战斗同步已通过”。

## 11. 面试快答

**问：OnlineSubsystem 和 NetDriver 是什么关系？**

答：OnlineSubsystem 提供身份、Session、搜索和加入等在线服务抽象；NetDriver 负责游戏连接和网络数据传输，项目把默认 GameNetDriver 配置为 SteamSockets 并保留 IP fallback。二者协作，但职责不同。

**问：为什么 Session Subsystem 放在 GameInstance？**

答：Session 跨地图、跨 Pawn 生命周期，GameInstanceSubsystem 的生命周期比菜单和关卡稳定，适合持有 Session Interface、搜索结果和异步 delegate。

## 12. 小练习

1. 画出 Host 与 Join 两条时序图，标明哪个回调是异步返回的。
2. 如果 `FindSessions` 成功但找不到匹配 `MatchType`，UI 应如何恢复？
3. 解释“Session 加入成功但 Gameplay 仍不同步”可能是哪些层的问题。
