# 射击战斗闭环：从输入到伤害、重生与 Server Rewind

## 1. 本节目标

不要把“开枪”理解成一个函数。本项目的一枪至少包含：输入、节奏、瞄准目标、网络请求、服务器验证、命中模型、伤害、弹药、表现和失败路径。

## 2. 类职责

| 类 | 负责什么 | 不应该负责什么 |
| --- | --- | --- |
| `ABlasterCharacter` | 输入入口、相机、移动、生命、淘汰 | 复杂开火算法、全局比赛规则 |
| `UCombatComponent` | 当前武器、弹药、开火/换弹/瞄准/换枪、RPC | 直接拥有世界级计分规则 |
| `AWeapon` | 武器状态、弹匣、碰撞拾取、附着/掉落 | 决定比赛是否结束 |
| `ABlasterProjectile` | 飞行物移动和 Server 碰撞 | 客户端结算伤害 |
| `ULagCompensationComponent` | Server 保存历史骨骼盒、查询历史命中 | 移动真实角色回到过去 |
| `ABlasterGameMode` | 计分、比赛阶段、重生 | 直接更新客户端 Widget |

## 3. 一枪的完整数据流

```text
IA_Fire / AttackAction
        |
        v
ABlasterCharacter::Attack / StopAttack
        |
        v
CombatComponent::SetFireButtonPressed
        |
        v
本地 FireOnce：检查本地间隔、弹药、比赛状态，TraceAim
        |
        v
ServerFire(HitTarget, RequestedWeapon, ShotTime, ShotId)
        |
        +--> Server 校验失败：直接丢弃
        |
        +--> Projectile：生成权威 Projectile
        +--> Hitscan：历史盒求交 -> ApplyDamage
        +--> Shotgun：8 条服务器射线 -> 按 Actor 聚合伤害
        |
        v
Weapon::SpendRound + TotalShotsFired
        |
        +--> ClientApplyRecoil（只给开火者）
        +--> MulticastFire（动画/弹壳/曳光表现）
        |
        v
ReceiveDamage -> Health -> Eliminated -> GameMode 计分/重生
```

## 4. 本地 FireOnce 不是权威开火

`UCombatComponent::FireOnce` 只在本地控制角色上运行，职责是让输入手感可用：

- `NextLocalFireTime` 防止本地 Timer 高频重复发送；
- 检查本地武器、换弹、死亡和弹药状态；
- 从摄像机方向得到 `HitTarget`；
- 用 `AGameState::GetServerWorldTimeSeconds()` 加上 Ping/2 估计得到 `ShotTime`；
- 递增 `NextLocalShotId`，把请求交给 `ServerFire`。

客户端的检查只是减少无效请求，不能作为防作弊机制。客户端可以被修改，所以真正的弹药扣除和射速限制必须再次在 Server 执行。

## 5. 三种命中模型

| 模型 | Server 做什么 | 体验/取舍 |
| --- | --- | --- |
| Projectile | 从枪口生成 `ABlasterProjectile`，由服务器碰撞事件调用 `ApplyDamage` | 有飞行时间和可观察轨迹；需要复制移动和处理飞行物生命周期 |
| Hitscan | 从枪口做即时命中；项目通过 `ULagCompensationComponent::ConfirmHit` 查询历史盒 | 即时命中；需要处理网络延迟、历史时间和世界遮挡 |
| Shotgun | Server 生成 8 个 `VRandCone` 方向，每条射线单独检测，再按 Actor 聚合伤害 | 一发多弹丸；聚合避免同一目标收到多次独立结算带来的重复逻辑 |

当前数值证据：Projectile 20 点，Hitscan 身体 20/头部 40，Shotgun 每个命中弹丸 4 点、最多 8 个弹丸。面试时说“项目当前配置”，不要把它说成通用平衡结论。

### 为什么 Trace 从摄像机而不是枪口开始？

第三人称相机和枪口不在同一点。客户端先用摄像机方向确定玩家看向的目标，Server 再从枪口朝这个目标发射/检测，并额外验证视线到枪口没有世界遮挡。这样兼顾瞄准直觉和枪口不能穿墙的约束，但近墙、枪口偏移和复杂遮挡仍可能带来设计取舍。

## 6. Server Rewind 的实现逻辑

`ULagCompensationComponent` 只在 Server Tick，每秒约 30 次采样角色的 Head、Spine、Pelvis、双上臂和双大腿变换，并保留约 1 秒历史。

```text
当前服务器角色 Mesh
        |
        v
每帧保存 7 个骨骼的世界变换 + 时间戳
        |
        v
收到带 ShotTime 的 Hitscan 请求
        |
        v
找到 ShotTime 两侧历史帧并按 Alpha 插值
        |
        v
把射线变换到每个历史盒的局部空间
        |
        v
LineExtentBoxIntersection，选最先命中的目标
```

它没有把真实角色移动回过去，而是用历史变换做数学查询，因此不会把回溯过程暴露给其他 Gameplay 系统。当前限制也必须讲清：它使用的是粗粒度骨骼盒，当前世界遮挡仍按即时世界检测，Shotgun 和 Projectile 的 SSR 尚未扩展为同样的历史模型。

## 7. 弹药、换弹与双武器

- `AWeapon::Ammo` 是当前弹匣，Server 扣除并复制。
- `UCombatComponent::CarriedAmmo` 是角色携带备弹，Server 换弹时把缺口补到弹匣。
- `bReloading` 是复制状态；换弹用 Timer 延迟 1.5 秒完成。
- `FinishReload` 必须再次检查比赛阶段、角色是否仍有效、是否淘汰、武器是否仍属于角色。因为计时器完成时世界状态可能已经变了。
- 主武器放在 `EquippedWeapon`，副武器放在 `SecondaryWeapon`；切换不仅是隐藏显示，还涉及 Owner、附着、碰撞和掉落生命周期。
- 角色销毁时 `CombatComponent::EndPlay` 清理 Timer，Server 的 `DropWeapons` 处理武器掉落。

## 8. 伤害、淘汰与比赛

```text
Server 命中
  -> UGameplayStatics::ApplyDamage
  -> ABlasterCharacter::ReceiveDamage
  -> Health -= Damage
  -> Health <= 0：bEliminated = true
  -> 停止移动/输入/开火、掉落武器
  -> GameMode::RecordElimination
       -> PlayerState.Score += 1
       -> Victim PlayerState.Defeats += 1
       -> GameState 公告
  -> 3 秒后 GameMode::RespawnPlayer
```

`bEliminated` 是防止重复结算的门。死亡时清理 Speed Timer，避免旧 Pawn 的临时 Buff 影响重生后的角色；新 Pawn 由 `RestartPlayer` 重新生成，PlayerState 继续保留玩家统计。

## 9. 补给和失败路径

`ABlasterPickup::OnOverlap` 只在 Server 响应，先检查比赛阶段、角色状态和 `bConsumed`，然后调用 `Heal`、`AddCarriedAmmo` 或 `ApplySpeedBuff`。资源已满时不消费，成功后隐藏 30 秒再激活。

至少要验证：

1. 没有武器时开火；
2. 空弹匣、没有备弹、换弹中开火；
3. 重复 ShotId、过期 ShotTime、错误武器指针；
4. 淘汰后继续开火、换弹、换枪；
5. 满血拾取 Health、满备弹拾取 Ammo；
6. 同一 Pickup 被两个客户端同时重叠时只消费一次。

现有具体数值和 PIE 结果见 `../../docs/VERIFICATION.md`、`../networking/B03-B05-combat-loop.md`、`../networking/B06-B08-match-and-pickups.md`、`../networking/B09-hitscan-rewind.md`。

## 10. 引擎源码阅读路线

- Damage：`Engine/Source/Runtime/Engine/Classes/GameFramework/Actor.h`、`Engine/Private/Actor.cpp`
- Projectile Movement：`Engine/Source/Runtime/Engine/Classes/GameFramework/ProjectileMovementComponent.h`
- Collision/Trace：`Engine/Source/Runtime/Engine/Private/Collision/` 与 `World` 查询接口
- Timer：`Engine/Source/Runtime/Engine/Public/TimerManager.h`
- GameMode/重生：`Engine/Public/GameFramework/GameModeBase.h`

项目级代码先看 `CombatComponent.cpp` 的 `FireOnce`、`ServerFire_Implementation`、`MulticastFire_Implementation`，再看引擎如何执行碰撞、Timer 和 Damage Delegate。

## 11. 面试快答

**问：为什么不能相信客户端传来的 HitTarget？**

答：客户端只能提交瞄准意图，不能直接提交结算结果。Server 要检查时间、序号、武器 Owner、射速、弹药、方向、枪口遮挡，并在 Server 世界中重新做命中查询；延迟补偿也只是 Server 的历史数据查询。

**问：为什么 Shotgun 要先按 Actor 聚合伤害？**

答：多个弹丸可能命中同一 Actor。先聚合可以在一次 Server 请求中形成确定的最终伤害，再调用一次 Damage 流程，避免重复触发淘汰/事件逻辑，也方便后续扩展伤害类型。

**问：为什么 PlayerState 保存分数？**

答：Pawn 会因为淘汰和重生替换，PlayerState 的生命周期代表玩家而不是某一次 Pawn，因此分数和死亡数不会随 Pawn 销毁丢失。

## 12. 小练习

1. 画出 Projectile 和 Hitscan 的差异：命中发生在哪一刻、由哪个对象触发 Damage、客户端看到了什么。
2. 解释 `ShotId` 去重和 `NextServerFireTime` 分别防什么问题。
3. 假设要加入爆炸武器，哪些逻辑应放在 Projectile，哪些逻辑应放在 CombatComponent 或 Damage 系统？
