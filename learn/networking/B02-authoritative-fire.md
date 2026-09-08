# B02：服务器开火与本地表现

2026-09-08，UE5.6。当前为原型，不代表完整作品集完成。

## 数据流

鼠标左键 IA_Fire → Character::Attack → Combat::SetFireButtonPressed → 本地定时器 → TraceAim → ServerFire。

服务器检查武器拥有者、附着、Muzzle、有限目标、距离、方向、冷却和枪口遮挡，成功后生成复制的 BlasterProjectile，并广播表现。客户端只请求目标，没有伤害和弹丸生成决定权。

弹丸移动由 ProjectileMovement 驱动，服务器决定碰撞销毁，最长存活3秒。客户端的动态 Montage、弹壳和枪口调试线是表现；弹壳不复制、不阻挡角色或可见性检测，2秒自动清除。

## 为什么冷却要处理到达抖动

客户端每0.15秒发送，但服务器收到的间隔可能略少于0.15秒。最初直接拒绝这些请求导致3秒只有14发。现允许20毫秒到达容差，同时将下一截止时间推进到 `max(now, previousDeadline) + interval`。提前抵达不会累计出更快的长期射速。

## 实测

- 完整 Editor 构建通过。
- Listen Server + Client：3秒20发，15秒100发，两端计数一致。
- 过远、反向、NaN直接服务器调用被拒绝；同帧20次合法调用只生成1发。
- 松开后计数停止、Montage结束、弹壳清空；按住时销毁Pawn，两端清理完成。
- 两端活动Montage和IK有效；15秒测试最后一次PIE0错误，仍有既有警告。

## 资源和限制

`Content/Blueprints/Character/Animation/B02Fire` 是独立重定向结果。源为 AnimStarterPack Fire_Rifle_Hip / Fire_Rifle_Ironsights。主骨骼绑定与关键姿态已检查；重定向对衣物辅助骨骼报错，衣物迁移与视觉质量未验收。DefaultSlot接在B01上半身分层前，最终FABRIK保持执行。

缺少弹壳插槽，暂用角色右侧近似抛壳位置；枪口仍是调试线、弹丸是球体。精确命中时序、最终VFX、打包、高延迟和Dedicated Server仍需后续验证。截图工具当前返回编辑器视口，不能拿它证明玩家视觉效果。

## 复盘题

1. 为什么客户端可以提出瞄点，但不能决定生成权威弹丸？
2. 为什么弹壳不用复制，而弹丸需要复制？
3. 20毫秒冷却容差如何避免变成长期加速开火？
