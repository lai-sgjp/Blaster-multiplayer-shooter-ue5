# HUD、UMG 与本地表现数据流

## 1. 本节目标

理解项目里两种 UI 路线：

- `UUserWidget`/`UWidgetComponent`：适合头顶玩家信息、菜单和可视化控件；
- `AHUD::DrawHUD`/Canvas：当前项目用来绘制屏幕 HUD、准星、血量、弹药、公告和补给标签。

核心原则：Server 复制游戏数据，客户端 UI 读取本地已同步数据；Server 不直接操作某个客户端的 Widget。

## 2. 项目中的 UI 类

| 类 | 基类 | 位置 | 职责 |
| --- | --- | --- | --- |
| `ABlasterHUD` | `AHUD` | `Source/Blaster/HUD/BlasterHUD.*` | `DrawHUD` 中绘制准星、血量、弹药、比赛倒计时和公告 |
| `UOverheadWidget` | `UUserWidget` | `Source/Blaster/HUD/OverheadWidget.*` | 显示 Remote Role 和 PlayerState 名称 |
| `UWidgetComponent` | Actor Component | `ABlasterCharacter`、`AWeapon` | 把 Widget 作为世界空间组件附着到角色或武器 |
| `UMenu` | `UUserWidget` | `Plugins/MultiplayerSessions/.../Menu.*` | Host/Join 按钮和 Session 回调 |

## 3. HUD 如何消费网络状态

`ABlasterHUD::DrawHUD` 每帧从本地 PlayerController 读取：

```text
PlayerOwner
  ├── GetPawn() -> ABlasterCharacter -> Health / IsEliminated / EquippedWeapon
  ├── GetPlayerState() -> Score / Defeats / Ping
  └── GetWorld()->GetGameState() -> MatchState / PhaseDeadline / Announcement
```

然后计算：

- 当前比赛阶段和服务器时间的差值 -> 倒计时；
- PlayerState 分数和死亡数 -> 统计；
- Character Health -> 生命条；
- CombatComponent 的武器、Ammo、CarriedAmmo 和 `bReloading` -> 弹药面板；
- `TraceAim` 的本地结果 -> 准星是否命中角色；
- Weapon/Pickup Actor 的世界位置投影 -> 屏幕空间补给标签。

HUD 不会修改 Health、Ammo 或 Score。它只是把已经由 Server 决定并复制到本地的状态变成视觉信息。

## 4. UMG Widget 的生命周期

`UOverheadWidget` 使用 `BindWidget` 绑定蓝图中的 TextBlock，通过 `ShowPlayerNetRole` 显示角色的远端网络角色和 PlayerState 名称。`NativeDestruct` 中移除 Widget，避免它在所属 Actor 或世界销毁后继续留在视口/树中。

菜单 `UMenu::MenuSetup` 做了三件事：

1. 加入 Viewport 并设置 UI-only 输入模式；
2. 从 `UGameInstance` 获取 `UMultiplayerSessionsSubsystem`；
3. 绑定 Host/Join/Session 完成委托。

按钮回调只发起 Session 操作，真正的异步结果通过委托回来后再 Travel 或恢复按钮状态。

## 5. 为什么 Server 不能直接访问客户端 Widget

Widget 属于本地 UI 世界，可能每个客户端布局、分辨率和显示对象都不同。Server 没有某个客户端的有效 Widget 实例，直接操作也不能可靠地传播。正确数据流是：

```text
Server 修改 Health / GameState / PlayerState
        |
        v
Replication 到对应客户端
        |
        v
客户端 HUD / Widget 从本地对象读取并绘制
```

如果需要一次性表现事件，可以复制状态、发送 Client RPC 或 Multicast 事件，但仍要让客户端自己更新 UI，而不是让 Server 直接调用 `SetText`。

## 6. Canvas HUD 与 UMG 的取舍

当前 `ABlasterHUD` 使用 Canvas 的好处是原型阶段迭代快、可以直接按屏幕比例绘制准星和小型信息面板；复杂菜单、可复用控件、动画和设计师编辑则更适合 UMG Widget。

面试时不必把 Canvas 说成最终 UI 架构。可以说：项目用 Canvas 快速验证战斗信息闭环，菜单和头顶信息用 UMG；如果进入产品化阶段，会进一步拆分 Widget、ViewModel/Presenter、可本地化文本和样式资源。

## 7. 验证步骤

1. Listen Server + 1 Client 中，确认两端都能看到自己的 Health/Ammo/Score。
2. Server 造成伤害后，客户端 HUD 是否在复制后更新；客户端本地不能伪造 Health。
3. 淘汰后 HUD 是否显示淘汰状态、隐藏准星并在重生后重新绑定新 Pawn。
4. GameState 阶段切换时两端倒计时是否使用 `GetServerWorldTimeSeconds` 保持接近。
5. 关闭或销毁菜单后，输入模式是否从 UI-only 恢复为 Game-only。

详细真实证据见 `../../docs/VERIFICATION.md`、`../../docs/PRESENTATION-REVISION.md` 和 `../../Source/Blaster/HUD/BlasterHUD.cpp`。

## 8. 引擎源码阅读路线

- HUD：`Engine/Source/Runtime/Engine/Classes/GameFramework/HUD.h`
- Canvas：`Engine/Source/Runtime/Engine/Classes/Engine/Canvas.h`
- Widget：`Engine/Source/Runtime/UMG/Public/Blueprint/UserWidget.h`
- WidgetComponent：`Engine/Source/Runtime/UMG/Public/Components/WidgetComponent.h`
- PlayerState：`Engine/Source/Runtime/Engine/Classes/GameFramework/PlayerState.h`

## 9. 面试快答

**问：HUD 数据应该从哪里读？**

答：从本地 PlayerController、当前 Pawn、PlayerState 和 GameState 读取。它们是网络复制到本地的状态；HUD 不应该自己做伤害、计分或权威倒计时。

**问：为什么 Ping 可以在 HUD 显示但不作为伤害结果？**

答：Ping 是诊断和体验信息，来源于 PlayerState/网络统计；伤害仍由 Server 根据合法请求和命中模型结算，不能让客户端用 Ping 直接改结果。

## 10. 小练习

1. 将 HUD 当前显示项分成 Character、CombatComponent、Weapon、PlayerState、GameState 五类。
2. 设计一个“击杀公告”从 Server 到 HUD 的两种实现：复制短时消息和 Client RPC，各自的优缺点是什么？
3. 解释为什么重生后 HUD 必须重新从 `PlayerController->GetPawn()` 读取，而不能永久缓存旧 Pawn。
