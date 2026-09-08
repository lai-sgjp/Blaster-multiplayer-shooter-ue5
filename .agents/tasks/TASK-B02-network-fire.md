# TASK-B02-network-fire

状态：READY
日期：2026-09-08
前置：B01数据与图验证通过，视觉打磨后续
课程：第5节Projectile/Multiplayer Fire；Transcript未读取。

## Gate 0

唯一目标：客户端按住鼠标左键可请求连续开火；服务器验证已装备武器、拥有者、枪口、目标方向/范围和冷却，只由服务器生成复制弹丸；未装备/冷却/非法目标拒绝。

白名单：CombatComponent.h/.cpp、BlasterCharacter.h/.cpp、Weapon.h/.cpp、新增Source/Blaster/Weapon/BlasterProjectile.h/.cpp；Content/Input/Actions/IA_Fire、Content/Input/IMC_Default（先核验实际路径）、BP_BlasterCharacter.AttackAction；相关任务/日志/learn说明。

输入→本地Combat定时请求→ServerFire→服务器校验→权威Projectile→移动/销毁复制；Multicast只负责开火表现。所有命中判定仅服务器；B04才引入生命/计分，不提前把伤害逻辑塞进Character。

最小表现先用可见球形弹丸与短暂枪口调试线便于验收，课程Montage/VFX/弹壳仍需后续资源切片，不宣称B02整包完成。未批准变更其他美术资产。

UE5.6：使用已有EnhancedInput、UProjectileMovementComponent、Server RPC _Implementation、Replicated属性注册与计时器清理。无新第三方依赖。

验证：完整Editor Build；Listen Server+Client合法连续开火；无武器、射速、反向/过远/非有限目标失败路径；停止输入/销毁Pawn清除计时器；弹丸从服务器生成且生命周期有界。

Git：未授权commit/push；保留既有修改。

## Current delivery status (2026-09-08)
PASS WITH FOLLOW-UP for the frozen core slice. See REVIEW-B01-B10-core.md and ../../docs/VERIFICATION.md (project docs/VERIFICATION.md) for current evidence and explicit limits; historical Gate0 and intermediate blockers above are preserved. Optional art/network extensions are not claimed complete.
