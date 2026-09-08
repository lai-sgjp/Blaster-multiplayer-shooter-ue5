# Server 权威、Replication、RPC 与 RepNotify

## 1. 本节目标

你要建立一个简单但严格的判断顺序：

> 谁拥有这个状态？谁可以请求改变它？谁负责验证？哪些数据需要复制？客户端收到后如何更新表现？

多人游戏面试不只是背 `Server`、`Client` 和 `Multicast` 三个词，而是要能把一条真实链路讲完。

## 2. 三种网络角色

| 角色 | 在 Blaster 中的职责 |
| --- | --- |
| Server/Authority | 生成 Actor、验证开火/装备/拾取、结算伤害、修改 Health/Ammo/Score、驱动比赛阶段 |
| Owning Client/Autonomous Proxy | 产生输入、发送自己拥有对象的 Server RPC、做本地相机和即时输入反馈 |
| Remote Client/Simulated Proxy | 接收复制状态，驱动远端角色、动画、HUD 或表现，不应代替 Server 结算玩法 |

`IsLocallyControlled()` 表示当前实例是否由本地玩家控制；`HasAuthority()` 表示当前实例是否是权威端。两者不是同一个概念：Listen Server 的本地角色可能同时满足两者，远端客户端通常只满足前者或都不满足，取决于具体 Actor。

## 3. 复制变量的完整链路

一个变量成为可靠的复制状态，至少要经过：

```text
Server 改变 C++ 成员
        |
        v
头文件声明 Replicated / ReplicatedUsing
        |
        v
GetLifetimeReplicatedProps 注册 DOREPLIFETIME
        |
        v
Actor/Component 必须允许复制
        |
        v
客户端收到新值
        |
        +--> 普通 Replicated：客户端读取新值
        +--> ReplicatedUsing：调用 OnRep_* 更新本地表现
```

### 项目实例

| 状态 | 所属 | 声明/注册 | 客户端用途 |
| --- | --- | --- | --- |
| `Health` | `ABlasterCharacter` | `Replicated` + `DOREPLIFETIME` | HUD 显示生命条 |
| `bEliminated` | `ABlasterCharacter` | `ReplicatedUsing=OnRep_Eliminated` | 禁止移动/开火、显示淘汰表现 |
| `MoveSpeed` | `ABlasterCharacter` | `ReplicatedUsing=OnRep_MoveSpeed` | 更新 `CharacterMovement->MaxWalkSpeed` |
| `OverlappingWeapon` | `ABlasterCharacter` | `COND_OwnerOnly` + `OnRep_OverlappingWeapon` | 只有拥有者看到拾取提示 |
| `EquippedWeapon` | `UCombatComponent` | `Replicated` | 远端判断持枪/驱动动画和表现 |
| `Ammo`、`WeaponState` | `AWeapon` | `Replicated` / `ReplicatedUsing` | 弹药 HUD、碰撞、物理和拾取状态 |
| `PhaseDeadline`、公告 | `ABlasterGameState` | `Replicated` | 所有客户端倒计时和击杀公告 |
| `Defeats` | `ABlasterPlayerState` | `Replicated` | 统计 HUD 和跨 Pawn 保存 |

注意：`UCombatComponent` 在 `ABlasterCharacter` 构造函数中调用 `SetIsReplicated(true)`；只在 Component 头文件写 `UPROPERTY(Replicated)` 而不让 Component 复制，仍然不会产生预期结果。

## 4. RPC 不是权限替代品

### 当前项目中的 RPC

| RPC | 方向 | 目的 | Server 是否仍需验证 |
| --- | --- | --- | --- |
| `ServerEquipButtonPressed` | Client -> Server | 请求装备附近武器 | 必须重新查询重叠、Owner、淘汰和换弹状态 |
| `ServerSetAiming` | Client -> Server | 同步瞄准状态 | 必须确认角色、武器和淘汰状态 |
| `ServerFire` | Client -> Server | 提交开火请求 | 必须验证时间、序号、武器、Owner、弹药、射速、枪口遮挡和方向 |
| `ServerReload` | Client -> Server | 请求换弹 | 必须确认比赛阶段、弹药、武器和当前状态 |
| `ServerSwapWeapons` | Client -> Server | 请求切换主副武器 | 必须确认两把武器都归属于角色且不在换弹/淘汰状态 |
| `ClientApplyRecoil` | Server -> Owning Client | 只给开火者应用后座力 | 客户端仍检查本地控制、武器和淘汰状态 |
| `MulticastFire` | Server -> 所有相关客户端 | 播放动画、弹壳、曳光等表现 | 不能用它结算伤害 |

RPC 的 `Reliable` 只描述传输可靠性，不代表参数可信，也不代表调用者有权做任何事情。Server RPC 还要求调用者是拥有该 Actor/Component 的客户端；即使 RPC 被调用，Server 仍要对所有关键参数做业务验证。

## 5. 以 `ServerFire` 为例的防作弊链

`UCombatComponent::ServerFire_Implementation` 的校验顺序体现了服务器权威思想：

1. 当前比赛必须是 `MatchState::InProgress`。
2. `ShotId` 必须递增，拒绝重复或倒退请求。
3. `ShotTime` 必须有限，不能未来太多，也不能超出允许的历史窗口。
4. 请求武器必须等于当前 `EquippedWeapon`。
5. 角色必须有效、未淘汰、有 Controller，并且是 Authority。
6. 武器 Owner 必须是当前角色，弹药必须大于零，不能处于换弹。
7. `HitTarget` 不能包含 NaN，方向必须和角色的瞄准方向大致一致。
8. 武器必须附着在角色 Mesh，存在 `Muzzle` Socket，视线到枪口不能被世界遮挡。
9. 通过所有检查后，才生成 Projectile、做 Hitscan/Shotgun 判定、扣弹和广播表现。

这就是“客户端提交意图，服务器重建并裁决结果”，而不是“客户端告诉服务器我打中了谁”。

## 6. 三种状态同步模式

### A. 权威分支

装备、换弹、掉落等由 Server 直接改变复制变量。客户端只发请求，不在本地修改权威弹药或武器 Owner。

### B. 本地表现 + Server 校验

瞄准和输入反馈允许本地先改变相机或输入状态，同时调用 Server RPC；Server 接受后通过复制把状态传播给其他端。后座力是另一个例子：真正开火由 Server 确认，Server 通过 Client RPC 把后座力只发给拥有者。

### C. 引擎内置同步

蹲伏优先调用 `ACharacter::Crouch/UnCrouch` 和 `CharacterMovementComponent` 的内置网络流程，不重复造一个自定义 crouch RPC。项目构造函数开启 `bCanCrouch`，动画读取 `bIsCrouched`。

## 7. Replication 与 Multicast 的边界

- Replication 适合持续状态：生命、弹药、武器状态、比赛时间、统计数据。
- `RepNotify` 适合“收到新状态后需要在客户端做本地反应”：关闭碰撞、切换移动速度、更新拾取 Widget。
- Multicast 适合一次性表现事件：开火动画、弹壳和曳光；它不应承担不可丢失的比赛状态。
- UI 不复制 Widget 本身。Server 复制数据，客户端 HUD/Widget 读取数据并更新。
- `COND_OwnerOnly` 可以减少无关客户端看到的状态；拾取提示只需要发给拥有角色的客户端。

## 8. 网络验证方法

项目的基本验证是 Listen Server + 1 Client PIE：

1. Server 装备、开火、造成伤害，客户端是否看到正确结果。
2. 远端客户端尝试请求另一角色的战斗行为，Server 是否拒绝。
3. 重复 `ShotId`、过期 `ShotTime`、换弹中开火、空弹匣开火是否被拒绝。
4. 角色淘汰后继续输入，计时器和 RPC 是否停止。
5. 开启高延迟模拟后，是否仍然只有 Server 扣弹和结算一次伤害。

参考命令和证据见 `docs/VERIFICATION.md` 与 `learn/networking/B02-authoritative-fire.md`、`B09-hitscan-rewind.md`。

## 9. 引擎源码阅读路线

- 网络宏和复制注册：`Engine/Source/Runtime/Engine/Public/Net/UnrealNetwork.h`
- Actor 复制入口：`Engine/Source/Runtime/Engine/Private/ActorChannel.cpp`、`Engine/Private/NetDriver.cpp`
- Replication layout：`Engine/Source/Runtime/Engine/Private/RepLayout.cpp`
- RPC/Actor 所有权相关：`Engine/Public/GameFramework/Actor.h`、`Engine/Private/Actor.cpp`
- Character 移动同步：`Engine/Public/GameFramework/CharacterMovementComponent.h` 与对应 `Private` 实现

面试准备不需要背私有函数名，但要能从项目的 `DOREPLIFETIME`、RPC 声明和 `HasAuthority` 继续追到引擎的复制入口。

## 10. 面试快答

**问：Server RPC 为什么不能直接由任意客户端调用？**

答：RPC 运行在拥有该 Actor 的连接上，Ownership 是网络权限边界；但“能调用”不等于“参数可信”。Server 仍要检查角色身份、对象归属、比赛阶段、冷却、资源和空间关系。

**问：Replicated 和 RepNotify 有什么区别？**

答：二者都同步变量；RepNotify 在客户端收到新值时额外调用 `OnRep_*`，适合把网络状态转换成本地组件或 UI 表现。它不是 Server 端修改后的回调，所以 Server 需要立即表现时，项目中会显式调用同一个 `OnRep` 逻辑。

**问：为什么开火表现用 Multicast，伤害不也用 Multicast？**

答：开火动画和曳光是一次性表现事件，可以由 Server 确认后广播；伤害必须只在 Server 计算，否则多个客户端都可能重复或伪造结算。

## 11. 小练习

1. 把 `Health`、`Ammo`、`PhaseDeadline` 和 `Defeats` 分别归类为 Pawn、Actor、GameState、PlayerState 状态，并说明生命周期。
2. 设计一个“客户端请求拾取补给”的 Server 校验清单，至少包含距离、比赛状态、重复消费和资源上限。
3. 如果客户端伪造 `HitTarget` 指向身后目标，当前 `ServerFire` 通过哪些空间检查拒绝它？还有什么误差没有完全消除？
