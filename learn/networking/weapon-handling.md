# 按枪械配置射速与后座力

代码入口：Weapon.h 保存配置，Weapon.cpp BeginPlay 初始化弹匣；CombatComponent 的 FireOnce 使用本地冷却，ServerFire 验证服务器冷却、扣弹并发 ClientApplyRecoil；TickComponent 增量恢复本地视角，BlasterHUD 读取反馈显示。

Ammo 是复制的运行时状态，MagazineCapacity 是蓝图类配置。不能只改容量却保留 Ammo=30，也不能每次装备都重置 Ammo，否则丢弃重捡就能免费补弹。

本地射击计时控制输入节奏，服务器计时控制合法性。两者都保留跨松键/换枪的冷却，才能避免快速点按或切枪绕过间隔。服务器允许少量到达抖动，但用累计下一次时间保证长期速率。

后座力会影响后续朝向，但不应操作模拟代理的控制器。可靠 Client RPC 由服务器确认后发给拥有者，携带开火时参数，旧枪/死亡过滤可防止迟到确认影响新的状态。取舍是高延迟反馈较慢，进一步优化需要预测并按 ShotId 去重。

恢复自己的增量偏移，而不是插值回开枪前的绝对朝向，才能保留鼠标输入。使用现有本地 Tick，遵守 PlayerCameraManager 俯仰限制。部分恢复、部分保留产生需要压枪的差异。

复习：为何客户端 timer 不能替代服务端冷却？为何弹匣只在首次 BeginPlay 初始化？怎样实现不重复的预测后座力？
