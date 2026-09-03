# TASK-B01-animation-network-finish

状态：GATE 0
创建日期：2026-09-01
前置任务：TASK-B00-current-baseline
计划包：B01

## 1. 目标与课程定位

- 玩家可见目标：装备步枪后，本地玩家和远端玩家在移动、跳跃、上下瞄准及原地转身时显示稳定、一致的全身姿势和左手贴枪表现。
- 验收一句话：Listen Server + 1 Client 中，自主代理与模拟代理的方向偏移、Lean、Pitch、转身、Root Bone 和左手 IK 无明显跳变或抖动，未装备蹲伏仍正常。
- 课程主题：Leaning and Strafing、Aim Walking、Applying Aim Offsets、Pitch in Multiplayer、FABRIK IK、Turning in Place、Rotate Root Bone、Net Update Frequency、Crouch Unequipped。
- Transcript：未读取；需要具体 Editor 操作或课程动机时再读取。
- 必须理解：Actor rotation 与 Aim Offset 的分工；远端 Pitch 映射；FABRIK 的表现职责；自主代理与模拟代理的更新差异。

## 2. 当前事实基线

| 项目 | 状态 | 备注 |
| --- | --- | --- |
| Speed / InAir / Accelerating | 已实现并验证 | B00 用户 PIE 验证。 |
| Equipped / Crouched / Aiming | 已实现并验证 | B00 用户 PIE 验证。 |
| Yaw Offset / Lean | 未实现 | AnimInstance 无对应计算。 |
| Aim Offset Pitch 网络映射 | 未实现 | Character 无 AO_Pitch 更新。 |
| FABRIK 左手 IK | 未实现 | C++ 无左手骨骼变换数据。 |
| Turning in Place / Rotate Root Bone | 未实现 | 无转身状态与 AO_Yaw。 |

## 3. Gate 0：范围冻结

- 唯一交付：完成第 4 节动画与网络表现闭环。
- 允许修改的 C++ 文件：`Source/Blaster/Character/BlasterCharacter.h/.cpp`、`Source/Blaster/Character/BlasterAnimInstance.h/.cpp`；只有编译证明需要时才补充 include。
- 允许的 Editor 操作：修改角色动画蓝图、AnimGraph、Aim Offset、FABRIK、Transform/Rotate Root Bone 和相关动画资产绑定。
- 明确非目标：射击 Montage、Projectile、准星/FOV、伤害、换弹、武器类型、网络会话。
- 不得触碰：无关代码、地图、美术源文件、现有用户改动和 Git 历史。
- 变更触发条件：目标、白名单、数据流或验收方式变化时返回 Gate 0。

## 4. 网络所有权与数据流

| 参与者 | 输入/事件 | 权威决定 | 复制或 RPC | 表现/UI |
| --- | --- | --- | --- | --- |
| 拥有客户端 | 控制旋转与移动 | CharacterMovement/Controller 产生状态 | 使用引擎已有移动与旋转复制 | 立即计算 Aim Offset、Lean 和 IK。 |
| Server | 接收并模拟移动 | 保持角色移动权威 | 使用 ACharacter 内建复制 | 按相同动画数据驱动视口。 |
| 模拟客户端 | 接收远端位置、旋转 | 不决定玩法状态 | 消费引擎复制结果 | 计算远端 Pitch、Lean、转身和 IK。 |
| AnimInstance | 每帧读取 Pawn | 不持有权威玩法状态 | 不新增复制变量 | 暴露动画图所需表现参数。 |

数据流：输入/控制旋转 -> CharacterMovement -> 引擎移动/旋转复制 -> 各机器 Character/AnimInstance 读取状态 -> AnimGraph Aim Offset、Lean、FABRIK 与 Root Bone。

## 5. UE5.6 迁移检查

| 检查项 | 状态 | 证据/方式 |
| --- | --- | --- |
| Enhanced Input / LocalPlayer 生命周期 | 无需变化 | B00 已验证。 |
| Build.cs、include、模块、UHT | 尚未验证 | Development Editor Build。 |
| UPROPERTY、GC、对象生命周期 | 尚未验证 | 新增引用保持 UPROPERTY。 |
| Authority、Role、Ownership | 尚未验证 | 区分本地与模拟代理。 |
| Replication、RepNotify、RPC | 无需新增 | 复用 CharacterMovement/BaseAimRotation。 |
| Animation / IK / 移动平滑 | 尚未验证 | UE5.6 AnimGraph + 双端 PIE。 |
| HUD/Widget | 无需变化 | 本任务无 HUD。 |
| SteamSockets / OnlineSubsystem | 无需变化 | 不修改会话配置。 |

## 6. 验收计划

- Development Editor Build：UHT、编译和链接通过。
- 单机冒烟：站立、八方向移动、跳跃、蹲伏、瞄准无明显姿势破坏。
- Listen Server + 1 Client：双方观察上下瞄准、移动 Lean、原地转身和左手贴枪。
- 失败路径：未装备时不执行武器 Aim Offset/FABRIK，蹲伏移动保持正确。
- 回归：B00 拾取、装备、瞄准和蹲伏仍通过。

## Git 边界

- 可暂存文件：任务白名单内实际修改文件、任务卡、审查/日志记录。
- 提交授权：未请求。
