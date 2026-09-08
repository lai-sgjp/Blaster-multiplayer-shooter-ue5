# Blaster 项目技术栈总览（面试版）

这份文档是整个 `learn/` 知识库的入口。目标不是背插件名称，而是让你能回答面试官最关心的三件事：

1. 这个系统为什么这样分层？
2. 一次输入或网络请求如何走到最终结果？
3. 哪些状态由服务器决定，哪些只是本地表现？

文档依据当前工作区的源码、配置、Content 资产和 `docs/VERIFICATION.md`。当前工作区存在未提交改动，因此面试前要以你实际能打开、编译和解释的版本为准。

## 一分钟项目介绍

Blaster 是一个基于 UE5.6.1 的 C++ 多人第三人称射击原型，采用 Unreal Gameplay Framework 组织角色、战斗组件、武器、GameMode、GameState 和 PlayerState。输入使用 Enhanced Input；网络采用服务器权威模型，通过 Replication、Server/Client/NetMulticast RPC 和 RepNotify 同步状态与表现；战斗支持 Projectile、Hitscan、Shotgun 三种命中模型，并为 Hitscan 实现了服务端历史命中回溯。大厅使用 OnlineSubsystem、OnlineSubsystemSteam 和 SteamSockets，UI 使用 UMG、UWidgetComponent 和 AHUD Canvas，动画使用 AnimInstance、Blend Space、Aim Offset、IK Retargeting 和 FABRIK 数据链路。

这是课程衍生的学习/作品集项目。面试时应准确区分：已经有源码和 PIE 证据的功能、只有资产或配置的功能，以及尚未完成公网 Steam、独立服务器和完整打包验收的部分。

## 技术栈地图

| 层次 | 技术 | 项目中的证据 | 面试重点 |
| --- | --- | --- | --- |
| 引擎 | Unreal Engine 5.6.1 | `Blaster.uproject`、UE5.6 Target 配置 | Gameplay Framework、反射、生命周期、网络复制 |
| 语言与构建 | C++、UnrealBuildTool、UnrealHeaderTool、Visual Studio | `Source/Blaster.Build.cs`、`Blaster.Target.cs`、`BlasterEditor.Target.cs` | 模块依赖、UHT 生成代码、Editor/Game 两种 Target |
| 核心架构 | `ACharacter`、`AActor`、`UActorComponent`、GameMode、GameState、PlayerState、PlayerController、Subsystem | `Source/Blaster/`、`Plugins/MultiplayerSessions/` | 职责边界、生命周期、对象所有权 |
| 输入 | Enhanced Input | `ABlasterCharacter::BeginPlay`、`SetupPlayerInputComponent`、`Content/Input/` | Mapping Context、Input Action、Trigger、输入到 Gameplay 的链路 |
| 多人网络 | Actor/Component Replication、RPC、RepNotify、Ownership、Server Time | `GetLifetimeReplicatedProps`、`ServerFire`、`OnRep_*` | Server 权威、防作弊、复制变量与表现分离 |
| 战斗 | Projectile、Hitscan、Shotgun、Damage Delegate、弹药、换弹、双武器 | `UCombatComponent`、`AWeapon`、`ABlasterProjectile` | 一次开火的完整数据流和服务器校验 |
| 延迟补偿 | 自定义 Hitscan Server Rewind | `ULagCompensationComponent` | 历史帧、插值、时间合法性、当前世界遮挡限制 |
| 动画 | AnimInstance、AnimGraph、Blend Space、Aim Offset、IK Retargeter、FABRIK、Turn in Place | `UBlasterAnimInstance`、`ABP_Blaster` 及 `Content/Blueprints/Character/Animation/` | 动画参数从哪里来、远端 Pitch、网络状态与视觉状态的关系 |
| UI | UMG、`UUserWidget`、`UWidgetComponent`、`AHUD::DrawHUD`、Canvas | `UOverheadWidget`、`ABlasterHUD` | UI 只消费本地已同步状态，不在 Server 直接操作 Widget |
| 在线会话 | OnlineSubsystem、OnlineSubsystemSteam、SteamSockets、SeamlessTravel | `MultiplayerSessions` 插件、`DefaultEngine.ini`、`ALobbyGameMode` | Session CRUD、委托回调、Host/Join、Travel 边界 |
| 渲染与关卡 | Lumen、Ray Tracing、Virtual Shadow Maps、DX12/SM6、PCG | `Config/DefaultEngine.ini`、`Blaster.uproject`、`Content/Maps/` | 如何区分核心玩法技术和展示/资产技术 |
| 角色资产 | VRM4U、IK Rig、IK Retargeting、Anim Starter Pack、LearningKit | `Plugins/VRM4U/`、`Content/Characters/` | 资源骨骼兼容、重定向、第三方插件边界 |
| 编辑器工具 | Unreal MCP Bridge | `Plugins/UnrealMCPBridge/` | Editor-only 工具不等于运行时游戏模块 |
| 验证 | Development Editor/Game Build、Listen Server + 1 Client PIE、失败路径 | `docs/VERIFICATION.md`、`.agents/BLASTER_WORKFLOW.md` | 用证据说明“已编译”“已运行”“已验证”的区别 |

## 架构总图

```text
拥有客户端 Enhanced Input
        |
        v
ABlasterCharacter  ---- 相机、移动、输入、基础状态
        |
        v
UCombatComponent  ---- 战斗状态、武器、弹药、RPC、开火节奏
        |
        +--> AWeapon / ABlasterProjectile
        +--> Hitscan -> ULagCompensationComponent
        +--> Shotgun -> Server Trace + damage aggregation
        |
        v
Server 权威伤害 -> Health / Eliminated
        |
        +--> ABlasterGameMode   （只在 Server：规则、计分、重生）
        +--> ABlasterGameState  （复制全局比赛阶段和公告）
        +--> ABlasterPlayerState（复制跨 Pawn 生命周期的玩家统计）
        |
        v
Replication / OnRep / Multicast
        |
        v
本地 AnimInstance / HUD / Widget / 射击表现
```

## 三条必须讲清楚的数据流

### 1. 输入到移动、瞄准和装备

`Content/Input/IA_*.uasset` 描述动作，`IMC_Default` 把键盘或鼠标映射到动作。`ABlasterCharacter::BeginPlay` 把 Mapping Context 加入本地 `UEnhancedInputLocalPlayerSubsystem`，`SetupPlayerInputComponent` 使用 `UEnhancedInputComponent::BindAction` 绑定 C++ 回调。

- 移动：`MoveAction` -> `Move` -> 根据 Controller Yaw 计算 Forward/Right -> `AddMovementInput` -> `CharacterMovementComponent`。
- 瞄准：`AimAction` -> `CombatComponent::SetAiming` -> 本地立即更新并调用 `ServerSetAiming` -> `bAiming` 复制到其他端 -> 本地 FOV 和 AnimInstance 消费。
- 装备：`EquipAction` -> `ServerEquipButtonPressed` -> Server 重新查询重叠武器并验证 Owner -> `EquipWeapon` -> 武器状态、Owner、附着和复制变化。

关键判断：输入是客户端产生的，但不能因此让客户端直接决定伤害、弹药、分数或比赛状态。

### 2. 一次开火

`AttackAction` 只负责设置 `bFireHeld`。本地 `FireOnce` 做节奏控制和瞄准射线，然后把 `HitTarget`、请求武器、服务器时间和递增 `ShotId` 发给 `ServerFire`。Server 再验证比赛阶段、时间、序号、武器 Owner、弹药、射速、枪口遮挡和目标方向，验证通过后才选择 Projectile、Hitscan 或 Shotgun 模型并扣弹。

伤害由 Server 结算；`MulticastFire` 只广播动画、弹壳、曳光和枪口表现，`ClientApplyRecoil` 只把后座力反馈给开火者。这样表现可以广播，权威玩法仍然集中在 Server。

### 3. 伤害到重生和 HUD

权威命中调用 `UGameplayStatics::ApplyDamage`，Server 端角色的 `OnTakeAnyDamage` 进入 `ReceiveDamage`，减少 `Health`。生命值归零时设置 `bEliminated`，由 `ABlasterGameMode::RecordElimination` 处理得分、死亡数和公告，并在计时器到期后 `RestartPlayer`。`Health`、`bEliminated`、`PlayerState` 和 `GameState` 的数据复制到各客户端，`ABlasterHUD::DrawHUD` 只读取当前本地 Pawn、PlayerState 和 GameState 绘制界面。

## 面试优先级

### 第一优先级：必须能白板讲清楚

1. UE Gameplay Framework 中 GameMode、GameState、PlayerState、PlayerController、Pawn/Character 的区别。
2. Replication、Server RPC、Client RPC、NetMulticast、Ownership 和 RepNotify 的关系。
3. `ServerFire` 如何防止客户端伪造武器、时间、射速和命中结果。
4. Projectile、Hitscan、Shotgun 的结算时机，以及为什么 SSR 只服务于 Hitscan。
5. `UBlasterAnimInstance` 如何从角色和武器读取动画参数，为什么动画表现不应该反过来决定权威玩法。

### 第二优先级：用来拉开差距

1. Server Rewind 的历史帧采样、插值和时间窗口。
2. `UActorComponent`、`UPROPERTY`、GC 和 `CreateDefaultSubobject` 的关系。
3. Enhanced Input 的 Mapping Context / Action / Trigger 结构。
4. Online Session 委托、`UGameInstanceSubsystem`、Host/Join 和 ServerTravel/ClientTravel。
5. PIE 验证和公网 Steam 验证的边界。

### 第三优先级：展示工程视野，但不要喧宾夺主

Lumen、Ray Tracing、Virtual Shadow Maps、PCG、VRM4U 和 MCP Bridge 都是项目中的技术或工具，但它们不是当前多人战斗主线。除非岗位明确问渲染、工具链或角色资产，否则先讲完服务器权威和数据流。

## 建议学习顺序

| 顺序 | 学习文档 | 学完后的输出 |
| --- | --- | --- |
| 1 | [UE C++、反射、构建和 Gameplay Framework](cpp/01-ue5-cpp-and-gameplay-framework.md) | 能从类职责和生命周期解释项目结构 |
| 2 | [Authority、Replication 和 RPC](networking/01-authority-replication-rpc.md) | 能画出 Server/Owner/Proxy 的状态流 |
| 3 | [射击战斗闭环与 Server Rewind](gameplay/01-shooter-combat-pipeline.md) | 能完整复盘一枪和一次淘汰 |
| 4 | [动画数据链路](animation/02-animation-data-pipeline.md) | 能解释移动、瞄准、Lean、转身和 IK |
| 5 | [HUD、UMG 与本地表现](ui/01-hud-and-umg-data-flow.md) | 能解释 UI 为什么只消费复制状态 |
| 6 | [Steam Session 与关卡 Travel](online/01-steam-session-and-travel.md) | 能区分 Session、Travel 和 Gameplay Replication |
| 7 | [构建、配置和验证](tooling/01-build-config-and-validation.md) | 能给出可靠的 UE5.6 编译和多人验收方案 |

已有的专题深挖仍然有效：网络 B02–B11 在 `learn/networking/`，B01 动画和 Editor 操作在 `learn/animation/` 与 `learn/editor/`，真实验证和限制在 `docs/VERIFICATION.md`。

## 当前能力边界

- Listen Server + 1 Client PIE、Development 构建和多条失败路径已有项目记录。
- Steam 配置、Session API 和 Travel 代码存在；不能把本机 PIE 说成跨机器公网 Steam 已验收。
- Content 目录含大量本地资产且当前被 Git 忽略；只拿源码克隆不能复现完整 Editor 体验。
- FABRIK 的 C++ 左手变换数据链已准备，但最终 AnimGraph 节点接线和完整视觉验收仍应按 `learn/editor/B01-fabrik-animgraph-setup.md` 检查。
- Shotgun/Projectile 的 SSR、预测弹药、专用服务器、断线重连和完整 Cook/Stage 打包不属于当前已完成证据。

## 面试用项目陈述模板

> 我在 UE5.6 的 C++ 多人射击项目中，按 Gameplay Framework 拆分了 Character、CombatComponent、Weapon、GameMode、GameState 和 PlayerState。客户端只提交输入和开火请求，Server 校验 Owner、比赛阶段、弹药、时间和射速后完成伤害结算，再通过 Replication、OnRep 和 Multicast 更新状态与表现。命中模型包括服务器 Projectile、即时 Hitscan 和 Shotgun；针对 Hitscan，我在服务器维护约 1 秒的骨骼历史并按客户端同步的服务器时间做插值回溯。项目通过 Listen Server + 1 Client PIE 验证，但公网 Steam、专用服务器和最终打包仍明确区分为未验收边界。

## 复习检查

不要求先回答才能继续学习，但复习时应该能自己画出：

1. `IA_Fire` 到 `ReceiveDamage` 的完整链路。
2. `Health`、`Score`、`Defeats`、`PhaseDeadline` 分别属于哪个类，为什么。
3. 一个客户端伪造 `RequestedWeapon` 或 `ShotTime` 时，Server 在哪里拒绝它。
4. 一个远端角色的 `AO_Pitch`、`Lean` 和 `LeftHandTransform` 分别来自什么数据。
5. `CreateSession`、`FindSessions`、`JoinSession` 和 `ServerTravel` 谁负责什么。
