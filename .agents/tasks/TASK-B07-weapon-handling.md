# TASK-B07-weapon-handling
状态：PASS WITH FOLLOW-UP
日期：2026-09-08；前置：presentation-spawn-revision；计划包 B03/B05/B07。
目标：三种枪械不同弹匣、射击间隔、后座力及本地准心反馈。
课程：Weapon Aim Mechanics / Ammo / Different Weapon Types；Transcript 未读取；依据真实源码。
基线：30 发、共享 0.15 秒，未实现后座力；此前 Editor/Game 和双人 PIE 通过。
Gate 0 白名单：Source/Blaster/Weapon/Weapon.h/.cpp、Source/Blaster/BlasterComponent/CombatComponent.h/.cpp、Source/Blaster/HUD/BlasterHUD.cpp；Content/Blueprints/Weapon/BP_Weapon、BP_HitscanWeapon、BP_ShotgunWeapon；本任务卡、REVIEW-B07-weapon-handling.md、.agents/worklog.md、docs/WEAPON-HANDLING.md、docs/PRESENTATION-REVISION.md、learn/networking/weapon-handling.md；Saved 下测试脚本/日志。
数据流：拥有客户端按当前枪间隔发请求→Server 校验阶段/Owner/弹药/间隔→扣弹、伤害与原 multicast；可靠 Client RPC 只给射手确认后座力，HUD读取本地反馈。后座力修改控制朝向，后续 TraceAim 随之上移；恢复采用增量，不覆盖玩家鼠标输入。拒绝射击不触发后座力。
Tick：沿用已有本地 FOV Tick，插值恢复残余后座力与准心反馈。无新增模块。控制朝向 API 已查 UE5.6 Controller.h，俯仰限制使用 PlayerCameraManager.h。反射与 Client RPC 经 UHT/Build 验证；弱武器引用避免生命周期残留。动画、Steam、Enhanced Input 不改。
Editor：自动保存三种现有枪械参数、正常关闭并重启编辑器，双人 PIE；无需用户点击。资产保留备份。
验收：Editor Build；双人 PIE 初始容量/耗弹/换弹、不同射速、射手俯仰变化且观察者不变；快速重复请求拒绝；停火恢复、切枪冷却、死亡清理。完整主路径前不归档。
非目标：新枪模、声音、伤害平衡、散布系统、Steam 实网、打包；不提交、不推送、不撤销用户修改。
复习：为何服务端必须独立检查间隔？为何后座力只能操作拥有者控制器？为什么换枪不能清空下一次开火时间？
取舍：确认后反馈避免幽灵后座力，但高延迟时反馈会延迟；后续可加入预测与确认去重。


执行结果：白名单代码与三蓝图已保存。Editor/Game最终构建成功；三蓝图编译0错误0警告；双人初始容量、不同射速/后座力、补弹、非法重复/换弹中请求、切枪冷却、ADS、死亡重生已验证。真实客户端步枪2秒20发，霰弹枪2秒3发。相机极限用隔离ClientApplyRecoil调用验证89.9度。完整证据见docs/WEAPON-HANDLING.md和Saved/weapon-handling-*.json。独立审查REVIEW-B07-weapon-handling.md。用户口述未验证；无Git提交。唯一后续：高延迟下预测后座力与ShotId确认去重（非本轮必需）。
