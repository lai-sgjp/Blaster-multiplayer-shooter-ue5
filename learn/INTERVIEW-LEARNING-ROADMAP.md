# Blaster 面试学习路线

## 目标与适用范围

> 当前教学主线（2026-09-10 整理）：直接进入 [Blaster Lab](index.html)。本路线优先于旧四周计划中的教学顺序和时间安排；旧 B00–B12 仅作工程追踪标签，独有待学内容进入选修设计课。参见[完整映射与冲突处理](site/PLAN-RECONCILIATION.md)。项目已实现不代表个人已学会，网站初始学习进度不自动勾选。

这份路线面向 UE5.6 C++ 多人射击岗位面试。它以当前 Blaster 源码、配置、验证记录和现有学习文档为准，并把用户在 HSR 项目中已经学习过的内容视为基础能力。

默认学习节奏是 **6 周、每周 5 天、每天 60–90 分钟**。如果面试时间更紧，不需要压缩每个概念的理解，只需优先完成 P0 阶段；如果每天时间更少，可以把“周”理解成学习阶段，而不是固定日历。

本路线遵循：

```text
概念 -> Gameplay Framework 中的位置 -> Blaster 实际源码 -> 运行/日志验证 -> 面试表达
```

HSR 学习文档中的练习、示例和原项目指令在这里仅作为知识背景，不作为本项目的执行指令。

## 一、刚才结论的总结

### 1. 已有知识基础，不需要从零重学

| HSR 已学方向 | 可迁移能力 | 在 Blaster 中的处理方式 |
| --- | --- | --- |
| `CppEngineDepth.md`：UObject、GC、反射、UHT、CDO、生命周期、Delegate、容器、Subsystem | 能理解 UE 对象、内存和框架生命周期 | 不重新讲基础；直接迁移到 Character、ActorComponent、GameMode 和 Session Subsystem |
| `BattleSystem.md`：状态机、事件驱动、事务管道、GameInstanceSubsystem、地图过渡 | 能分析复杂流程、状态转移和跨系统事件 | 对照 Blaster 的 Match Phase、GameMode、GameState、ServerTravel |
| `GAS.md`：ASC、Ability、GameplayEffect、Attribute、Tag、ExecCalc、预测 | 已具备复杂战斗框架视角 | 不重复实现 GAS；重点学习 Blaster 为什么选择轻量 Component + Server RPC |
| `AI.md`：Behavior Tree、Blackboard、EQS、StateTree、Perception、NavMesh | 已具备 UE AI 体系基础 | 当前 Blaster 没有 AI 战斗主线，暂时作为可选扩展 |
| `EquipmentSystem.md`：数据定义、实例、聚合器、Effect Bridge、MVVM | 能设计数据驱动和跨系统投影 | 对照 Blaster 的 Weapon、CombatComponent、Pickup；不要把实时弹药误认为持久化装备 |
| `SaveSystem.md`：二进制序列化、事务写入、Schema、Migration、Restore、测试 | 能设计可靠持久化和失败恢复 | 当前 Blaster 没有存档主线，暂时不投入学习时间 |

这里的“已有”表示已经有系统学习材料和架构理解，不等于每个主题都能脱离代码完整实现。验证方式仍然是回到 Blaster 源码，用自己的话讲出数据流。

### 2. Blaster 真正新增的技术缺口

| 优先级 | 新技术 | 当前项目证据 | 为什么值得优先学习 |
| --- | --- | --- | --- |
| P0 | UE Actor/Component 网络复制、RPC、Ownership、Authority、RepNotify | `CombatComponent`、`Weapon`、`BlasterCharacter`、`GameState`、`PlayerState` | 这是多人射击面试的核心；GAS PredictionKey 不能替代 Actor 网络权限模型 |
| P0 | 服务器权威射击与防作弊 | `ServerFire`、`ServerReload`、`ServerSwapWeapons`、Pickup Server 校验 | 需要能说明客户端提交意图、Server 重建并裁决结果 |
| P0 | Server Rewind / Lag Compensation | `LagCompensationComponent` 保存历史帧并按 `ShotTime` 插值 | 这是 Blaster 相对普通 UE 项目的明显亮点，也是 HSR 没有覆盖的实时网络问题 |
| P0 | 多人 Gameplay Framework 分层 | `GameMode`、`GameState`、`PlayerState`、`Character`、`CombatComponent` | 要能解释规则、全局状态、玩家生命周期和 Pawn 状态为什么分开 |
| P1 | Enhanced Input | `IMC_Default`、`IA_*`、`SetupPlayerInputComponent` | 要能从输入资产追到 C++ 回调，再追到本地表现和 Server RPC |
| P1 | 射击游戏动画与 IK | `BlasterAnimInstance`、Aim Offset、Lean、Turn in Place、FABRIK、IK Retargeting | HSR 文档覆盖 AI 动画相关概念，但没有覆盖第三人称多人射击的动画数据链 |
| P1 | OnlineSubsystem、Steam Session 与 Travel | `MultiplayerSessionsSubsystem`、`Menu`、`LobbyGameMode`、`ServerTravel`、`ClientTravel` | GameInstanceSubsystem 概念已经学过，新的部分是 Session API、异步 Delegate 和网络地图切换 |
| P1 | UMG/HUD 与网络状态解耦 | `BlasterHUD`、`OverheadWidget`、`WidgetComponent` | 要能解释 Server 不直接操作 Widget，客户端如何消费复制状态 |
| P2 | UBT、UHT、Build.cs、Target、PIE 验证 | `Blaster.Build.cs`、两个 Target、`docs/VERIFICATION.md` | 用于回答工程化问题和证明功能，而不是单纯“我能编译” |

### 3. 暂时不要投入主线的方向

- GAS 的完整实现、复杂装备、云存档和 AI 行为树：这些已经在 HSR 中形成基础，而且不是当前 Blaster 的核心运行路径。
- Lumen、Ray Tracing、PCG、VRM4U：属于项目中的渲染或资产技术，除非岗位明确要求，否则排在多人网络和战斗之后。
- Unreal MCP Bridge：适合作为工具链或 Editor 自动化加分项，不应占用核心玩法面试准备时间。

## 二、学习方法：每次学习必须产出四样东西

每个主题不要只读文档，固定完成下面四步：

1. **概念卡**：用自己的话写出“它解决什么问题、谁负责、谁不能负责”。
2. **源码追踪**：从一个真实入口追到最终状态或表现，例如 `IA_Fire -> ServerFire -> ReceiveDamage`。
3. **验证证据**：通过编译、Listen Server + 1 Client PIE、日志、截图或失败路径确认理解。
4. **面试表达**：用 60–90 秒回答一个“为什么这样设计”的问题。

推荐单次学习时间分配：

```text
15 分钟概念
30 分钟源码
20 分钟验证或画图
15 分钟面试复述
```

如果当天没有回答检查题，也不能阻塞学习；检查题用于发现缺口，不是开始学习的前置条件。

## 三、六周学习路线

### 第 0 阶段：建立 HSR -> Blaster 映射（半天到 1 天）

目标：避免把 HSR 的架构直接套到 Blaster，先理解两个项目的职责边界不同。

重点对照：

| HSR 概念 | Blaster 对应物 | 不能直接等同的地方 |
| --- | --- | --- |
| `BattleCoordinator` | `GameMode + GameState + CombatComponent` 的组合 | Blaster 的规则和状态需要考虑 Server/Client 复制 |
| `TurnManager` | `BlasterGameMode` 的 Match Phase/Timer | HSR 是回合调度，Blaster 是实时比赛阶段 |
| `GameInstanceSubsystem` | `MultiplayerSessionsSubsystem` | 生命周期相似，但 Session 是异步在线接口 |
| GAS Ability/Effect | `CombatComponent + Weapon + ApplyDamage` | Blaster 不使用 GAS，资源和伤害由 Server 代码直接裁决 |
| Equipment Loadout | `EquippedWeapon + SecondaryWeapon + Pickup` | Blaster 是运行时武器状态，不是持久化装备实例 |

源码入口：

- `Source/Blaster/Character/BlasterGameMode.cpp`
- `Source/Blaster/Character/BlasterGameState.cpp`
- `Source/Blaster/BlasterComponent/CombatComponent.cpp`
- `Plugins/MultiplayerSessions/Source/MultiplayerSessions/Private/MultiplayerSessionsSubsystem.cpp`

阶段产出：一张“HSR 与 Blaster 职责映射表”，并能解释为什么 Blaster 不需要把所有战斗逻辑改成 GAS。

小练习：分别写出 `GameMode`、`GameState`、`PlayerState` 在 Blaster 中各自拥有的一个状态，以及 Pawn 重生后哪些状态会保留。

### 第 1 周：Gameplay Framework 与网络权限（P0）

目标：掌握多人游戏里“谁拥有状态、谁可以请求修改、谁负责验证”。

学习内容：

- `ACharacter`、`AActor`、`UActorComponent`、`APlayerController` 的职责。
- `GameMode` 只存在于 Server；`GameState` 复制全局比赛状态；`PlayerState` 保存跨 Pawn 的玩家统计。
- `HasAuthority()`、`IsLocallyControlled()`、Owner、Autonomous Proxy、Simulated Proxy 的区别。
- `UPROPERTY(Replicated)`、`ReplicatedUsing`、`DOREPLIFETIME`、`OnRep_*`。
- Server RPC、Client RPC、NetMulticast 的用途和边界；Reliable 不等于参数可信。
- `COND_OwnerOnly`、组件复制开关、网络状态与本地表现分离。

项目源码：

- `Source/Blaster/BlasterComponent/CombatComponent.h/.cpp`
- `Source/Blaster/Character/BlasterCharacter.h/.cpp`
- `Source/Blaster/Character/BlasterGameMode.h/.cpp`
- `Source/Blaster/Character/BlasterGameState.h/.cpp`
- `Source/Blaster/Character/BlasterPlayerState.h/.cpp`
- `Source/Blaster/Weapon/Weapon.h/.cpp`

验证任务：

1. 画出 `ServerFire`、`Health`、`PhaseDeadline` 和 `Defeats` 的 Server/Client 数据流。
2. 在 Listen Server + 1 Client PIE 中验证装备、弹药、淘汰和 HUD 是否同步。
3. 解释重复 `ShotId`、错误武器 Owner 和淘汰后请求分别在哪里被拒绝。

面试输出：能够回答“为什么 Server RPC 不是权限替代品？”以及“为什么 Score 放在 PlayerState 而不是 Character？”

对应资料：[Authority、Replication、RPC 与 RepNotify](networking/01-authority-replication-rpc.md)。

### 第 2 周：服务器权威射击与 Server Rewind（P0）

目标：完整讲清“一枪”而不是只会说“调用 Fire 函数”。

学习内容：

- 本地输入、Fire Timer、本地射速限制和 Server 射速限制的区别。
- `HitTarget`、`RequestedWeapon`、`ShotTime`、`ShotId` 为什么都必须重新验证。
- Projectile、Hitscan、Shotgun 的命中时机和权威对象。
- `ApplyDamage`、`ReceiveDamage`、淘汰、计分、重生的完整链路。
- 历史帧采样、时间窗口、两帧插值、骨骼盒求交和世界遮挡。
- 为什么当前 SSR 主要服务 Hitscan，而不是直接把 Projectile 或 Shotgun 也回溯。

项目源码：

- `Source/Blaster/BlasterComponent/CombatComponent.cpp` 的 `FireOnce`、`ServerFire_Implementation`、`MulticastFire_Implementation`
- `Source/Blaster/BlasterComponent/LagCompensationComponent.cpp`
- `Source/Blaster/Weapon/BlasterProjectile.cpp`
- `Source/Blaster/Character/BlasterCharacter.cpp` 的伤害与淘汰路径
- `Source/Blaster/Character/BlasterGameMode.cpp` 的计分和重生路径

验证任务：

1. 画出 `IA_Fire -> FireOnce -> ServerFire -> 命中模型 -> ApplyDamage -> RespawnPlayer`。
2. 验证空弹匣、换弹中、重复 ShotId、过期 ShotTime、错误武器和淘汰后开火。
3. 在高延迟条件下确认 Server 只扣一次弹、只结算一次伤害。

面试输出：能够回答“为什么客户端不能直接告诉 Server 我打中了谁？”以及“Server Rewind 为什么不把真实角色移动回过去？”

对应资料：[射击战斗闭环与 Server Rewind](gameplay/01-shooter-combat-pipeline.md)、[Hitscan 回溯专题](networking/B09-hitscan-rewind.md)。

### 第 3 周：Enhanced Input、动画数据流与 IK（P1）

目标：把“输入”和“动画”从蓝图资产追到 C++ 数据，再解释远端角色为什么能正确表现。

学习内容：

- Input Action、Input Mapping Context、Trigger、`BindAction` 和本地 LocalPlayer Subsystem。
- 输入回调如何进入移动、瞄准、射击、换弹和切枪。
- `UAnimInstance::NativeInitializeAnimation` 与 `NativeUpdateAnimation`。
- Speed、Air、Yaw/Pitch、Lean、Turning in Place 的数据来源。
- Weapon Socket -> `TransformToBoneSpace` -> `LeftHandTransform` -> FABRIK 的链路。
- 代码准备完成与 AnimGraph 最终接线、Socket 保存、Persona 验收之间的边界。
- IK Retargeting 和第三方角色骨骼兼容问题。

项目源码与资产：

- `Source/Blaster/Character/BlasterCharacter.cpp`
- `Source/Blaster/Character/BlasterAnimInstance.cpp`
- `Content/Input/`
- `Content/Blueprints/Character/Animation/ABP_Blaster`
- `Content/Assets/FPS_Weapon_Bundle/Weapons/Meshes/AR4/SK_AR4_X`

验证任务：

1. 画出 `IA_Aim` 到本地 FOV、`bAiming` 复制和 AnimInstance 的链路。
2. 分别说明本地角色和远端角色的 Pitch、Lean、LeftHandTransform 从哪里来。
3. 在 Editor 中完成或复查 Socket、Aim Offset、FABRIK 节点，并保存后重新验证。

面试输出：能够回答“动画状态为什么不能反过来决定伤害？”以及“为什么左手 IK 数据可以复制/计算，但最终 FABRIK 仍需要 AnimGraph 资产？”

对应资料：[动画数据链路](animation/02-animation-data-pipeline.md)、[FABRIK Editor 操作](editor/B01-fabrik-animgraph-setup.md)。

### 第 4 周：Online Session、Travel、HUD 与 UMG（P1）

目标：理解从菜单到大厅、从大厅到比赛，以及网络状态如何落到本地 UI。

学习内容：

- `UGameInstanceSubsystem` 的生命周期和 Session API 的异步 Delegate。
- `CreateSession`、`FindSessions`、`JoinSession`、Session 销毁重建。
- `ServerTravel`、`ClientTravel`、`SeamlessTravel` 的职责边界。
- Lobby `PostLogin` 为什么由 Server 决定何时进入比赛地图。
- `AHUD::DrawHUD`、`UUserWidget`、`UWidgetComponent` 的使用边界。
- HUD 如何读取 `GameState`、`PlayerState`、本地 Pawn 和 CombatComponent，而不是复制 Widget。
- PIE Session、NULL Subsystem、Steam Session 和跨机器公网验证的区别。

项目源码：

- `Plugins/MultiplayerSessions/Source/MultiplayerSessions/Private/MultiplayerSessionsSubsystem.cpp`
- `Plugins/MultiplayerSessions/Source/MultiplayerSessions/Private/Menu.cpp`
- `Source/Blaster/GameModes/LobbyGameMode.cpp`
- `Source/Blaster/HUD/BlasterHUD.cpp`
- `Source/Blaster/HUD/OverheadWidget.cpp`
- `Config/DefaultEngine.ini`

验证任务：

1. 画出 Host、Find、Join、Lobby、ServerTravel、ClientTravel 的时序图。
2. 分别验证本地 NULL Session、Listen Server PIE 和 Steam 配置，不把其中一种结果冒充另一种。
3. 解释为什么 GameMode 可以更新 Phase，但 HUD 必须在客户端读取 GameState。

面试输出：能够回答“Session 和 Gameplay Replication 是什么关系？”以及“为什么 Session 成功不代表公网联机已经验收？”

对应资料：[Steam Session 与关卡 Travel](online/01-steam-session-and-travel.md)、[HUD 与 UMG 数据流](ui/01-hud-and-umg-data-flow.md)。

### 第 5 周：构建、验证与面试工程化表达（P2）

目标：把“我写过功能”提升为“我知道如何构建、证明和说明它”。

学习内容：

- `Blaster.Build.cs` 的 Public/Private Module Dependency。
- Game Target 与 Editor Target 的区别。
- UHT 生成代码、`GENERATED_BODY`、反射声明和 UBT 编译过程。
- Editor Build、Game Build、Listen Server PIE、日志、截图和失败路径的证据等级。
- 运行时编辑器工具和游戏 Runtime 模块的边界。
- 不能把 Content 被 Git 忽略、DX12 未稳定、公网 Steam 未验收、Dedicated Server 未验收说成已完成。

项目文件：

- `Source/Blaster/Blaster.Build.cs`
- `Source/Blaster.Target.cs`
- `Source/BlasterEditor.Target.cs`
- `Blaster.uproject`
- `docs/VERIFICATION.md`
- `docs/ARCHITECTURE.md`

验证任务：

1. 完成一次 Development Editor Build 和一次 Development Game Build。
2. 完成 Listen Server + 1 Client 的主路径和至少一条失败路径。
3. 写一段 3 分钟项目介绍，只陈述已经有源码或运行证据的能力。

面试输出：能够解释“编译通过、PIE 通过、Steam 跨机器通过、打包通过”分别证明了什么，不能证明什么。

对应资料：[构建、配置、插件与验证](tooling/01-build-config-and-validation.md)、[验证矩阵](../docs/VERIFICATION.md)。

## 四、每周复盘标准

每完成一个阶段，必须留下以下四项内容：

- 一张端到端数据流图。
- 三个关键源码入口和各自的职责。
- 一个成功路径和一个失败路径的验证结果。
- 三道面试题：一题定义、一题设计取舍、一题故障排查。

如果只能保留最小产出，优先保留：

```text
一枪数据流图
GameMode/GameState/PlayerState 对照表
Server RPC 验证清单
Session/Travel 时序图
动画参数来源表
```

## 五、面试前最终通关清单

- [ ] 能区分 `HasAuthority()`、`IsLocallyControlled()` 和 Ownership。
- [ ] 能从 `DOREPLIFETIME` 讲到 `OnRep` 和本地表现。
- [ ] 能完整复述 `ServerFire` 的参数验证顺序。
- [ ] 能解释 Projectile、Hitscan、Shotgun 的结算时机。
- [ ] 能说明 Server Rewind 的时间窗口和插值目的。
- [ ] 能解释 GameMode、GameState、PlayerState 为什么不能互相替代。
- [ ] 能画出 Create/Find/Join Session 到 Travel 的时序。
- [ ] 能解释 Enhanced Input 从资产到 C++ 回调的路径。
- [ ] 能解释动画数据和权威玩法为什么分离。
- [ ] 能区分源码已验证、资产已存在、配置已存在和尚未验收的功能。

## 六、建议的下一步

下一次学习直接从第 1 周开始：打开 `CombatComponent`、`BlasterGameMode`、`BlasterGameState` 和 `BlasterPlayerState`，先画出状态归属图，再逐个追踪 `ServerFire`、`Health`、`PhaseDeadline`、`Defeats` 的写入者和消费者。

第一道检查题是：

> 如果把 `Defeats` 放回 `ABlasterCharacter`，角色重生时会发生什么？为什么 `ABlasterPlayerState` 更合适？

这道题不作为继续学习的阻塞条件；它只是用来确认你已经把 HSR 的对象生命周期知识迁移到了多人射击场景。
