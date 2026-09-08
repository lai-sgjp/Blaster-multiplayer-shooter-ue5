# B01：动画与网络表现

## 当前目标

让装备武器后的本地角色和远端角色，在移动、瞄准、Lean、转身、IK 和蹲伏状态下保持稳定一致的姿势表现。

## 已确认的概念

- `AnimSequence` 是单段时间轴动画。
- `Blend Space` 根据输入轴（例如 Speed/Direction）混合多个动画。
- `Aim Offset` 根据 Yaw/Pitch 姿势网格输出瞄准姿势。
- `Aim_Space_Hip` 和 `Aim_Space_Ironsights` 当前实际是 `AnimSequence`，不是可直接放入 AnimGraph 的 Aim Offset 资产。
- 这两个资源使用 `UE4_Mannequin_Skeleton`，而 `ABP_Blaster` 使用 `SK_EpicCharacter_Skeleton`；接入前必须确认 IK Retargeter 路径。

## 资源检查结果（2026-09-05）

- 项目已有真正的 `AimOffsetBlendSpace`：`Content/Characters/Mannequins/Anims/Rifle/AIM/AO_Rifle`。
- `AO_Rifle` 的 Skeleton 是 `Content/Characters/Mannequins/Meshes/SK_Mannequin`，仍与 `ABP_Blaster` 的 `Content/Assets/LearningKit/LearningKit_Games/Assets/Characters/Character/Mesh/SK_EpicCharacter_Skeleton` 不同，因此不能直接接入。
- `AO_Rifle` 使用三个垂直瞄准姿势：`Content/Characters/Mannequins/Anims/Rifle/AIM/MM_Rifle_Idle_ADS_AO_CU`、`.../MM_Rifle_Idle_ADS_AO_CC`、`.../MM_Rifle_Idle_ADS_AO_CD`。
- 已发现的 Retargeter 只覆盖 Manny -> Burnice、Kachujin -> vva；尚未发现 `SK_Mannequin -> SK_EpicCharacter_Skeleton` 的现成 Retargeter。

### 目标骨骼的现有动画与缺口

- `Content/Assets/LearningKit/LearningKit_Games/Assets/Characters/Character/Animations/ThirdPerson_Retarget/` 已有目标骨骼动画：`Idle_Aiming`、`Crouch_Idle_Aim`、`Crouch_Idle_Rifle_Ironsights`、`Jog_*_Rifle` 以及八个 `Jog_*_Lean_*`。
- MCP 读取 `Idle_Aiming` 和 `Jog_Fwd_Lean_L` 的只读 `Skeleton` 属性，均指向 `Content/Assets/LearningKit/LearningKit_Games/Assets/Characters/Character/Mesh/SK_EpicCharacter_Skeleton`。
- 同一目标目录按 `AimOffsetBlendSpace` 类型精确查询结果为空。因此 B01 当前不是缺少普通目标动画，而是缺少目标 Skeleton 上可供 AnimGraph 使用的 Aim Offset 资产。
- 后续合法路径是：在 Editor 中为目标 Skeleton 准备兼容的 Aim Offset 姿势/资产（必要时先完成 IK Rig/IK Retargeter），再把它接入现有 `Content/Blueprints/Character/Animation/ABP_Blaster` 的 `Equipped/Idle` 瞄准分支。
- `AO_Pitch` 是角度单位：本地值来自 `BaseAimRotation.Pitch`，远端值会把 `270..360` 映射为 `-90..0`。因此新目标 Aim Offset 的单轴建议设为 `-90..90`；不能照搬 `AO_Rifle` 的 `-1..1` 轴范围。
- 已确认 `Content/Blueprints/Character/Animation/BS_EquippedRun` 是未被引用的目标骨骼 2D Blend Space：X 为 `YawOffset`（-100..100），Y 样本为 Lean（-100..100），样本是四方向步枪 Jog 与八个 Lean 变体；它没有 Idle 样本，必须通过移动状态门控。
- `Content/Blueprints/Character/Animation/ABP_Blaster` 的 `Equipped` 状态机现有 `Idle/CrouchIdle/Run/CrouchRun`。`Run` 使用 `BS_EquippedRun`，`CrouchRun` 使用目标骨骼的 `EpicCharacter_CrouchWalk_2D`，从而避免把无 Idle 样本的 Jog Blend Space 常驻在静止状态。

## Equipped 状态转换结果（2026-09-05）

- `Run -> CrouchIdle`：`bIsCrouched AND NOT bIsAccelerating`。含义是角色已蹲下且已停止，应直接进入蹲伏静止。
- `CrouchRun -> Run`：`NOT bIsCrouched AND bIsAccelerating`。含义是角色已站起且仍在移动，应直接进入站立跑动。
- 修复入口按状态机名称、起点状态和终点状态唯一定位转换；目标不是恰好一条时拒绝写入。本次回读整个 `Equipped` 转换表，确认只有上述两条规则变化。
- `ABP_Blaster` 编译结果为 0 errors / 0 warnings，Blueprint review score 100，会话级验证 verdict 为 pass。

这里的两个布尔值都来自 AnimInstance 每帧读取的角色状态：`bIsCrouched` 表示角色当前蹲伏状态，`bIsAccelerating` 表示 CharacterMovement 当前有非零加速度。状态机用两者组合决定姿势，而不是新增 RPC 或网络变量。

## C++ 表现参数

- `YawOffset`：移动方向相对瞄准方向的水平偏移。
- `AO_Pitch`：本地/远端瞄准 Pitch 映射到动画使用的角度范围。
- `Lean`：根据角色旋转变化计算并平滑的身体倾斜表现。

## 当前边界

这些是各端 `AnimInstance` 消费的表现参数，不是新的玩法权威状态；B01 不为它们新增 RPC 或复制变量。Aim Offset 和 Equipped 移动状态机的静态验证已通过，下一项人工证据是 Listen Server + 1 Client PIE 视觉验证；FABRIK、Turning in Place 与 Rotate Root Bone 仍属于后续 B01 工作。

## FABRIK 前置 RED 阻塞（2026-09-05）

- live Unreal MCP doctor：`verdict=ready`；Editor 为 UE 5.6.1，bridge 19ms，plugin freshness/index/PIE 检查均通过，当前编辑世界为 `/Game/Maps/BlasterMap`。
- 地图唯一实际测试武器为 `BP_Weapon_C_1`，其 `WeaponMesh` 由 Editor Python 回读为 `/Game/Assets/FPS_Weapon_Bundle/Weapons/Meshes/AR4/SK_AR4_X.SK_AR4_X`。
- 该 SkeletalMesh 的 live socket 证据为 `num_sockets=1`、名称仅 `Muzzle`；`find_socket("LeftHandSocket")` 返回 `None`。因此当前不能读取有效左手握把变换，不能用组件原点或其他 socket 替代。
- `ABP_Blaster` 的 live 状态机回读仍显示 `Unequipped` 与 `Equipped`，但 `read_anim_blueprint` 不展开顶层 AnimGraph；在 socket 停止条件命中后未执行任何 AnimGraph 写入。源码也保持 RED：WeaponMesh getter、EquippedWeapon getter、LeftHandTransform 均未添加。
- ABP skeleton 的 Editor Python `bone_tree` 长度为 84，但当前 MCP Python 接口把 `BoneNode` 名称显示为匿名结构，不能据此声称已确认 `hand_r`、`hand_l`、`upperarm_l`。socket 缺失已经足够触发任务卡停止条件。

### 恢复条件

用户需在项目根目录对应资产 `Content/Assets/FPS_Weapon_Bundle/Weapons/Meshes/AR4/SK_AR4_X` 的 Skeleton Mesh 编辑器中新增并保存名为 `LeftHandSocket` 的 socket（位置应是左手握把，不应修改角色状态机或武器附着 socket）。完成后重新执行 live socket 与目标 Skeleton 骨骼核验，再继续 C++ getter/变换数据、完整 Development Editor Build 和顶层 FABRIK 接线。

## 2026-09-08：左手 IK 数据准备已验证

LeftHandSocket 已存在，角色 hand_r/hand_l/upperarm_l 已核验。C++ 每帧读取已附着武器的握点世界变换，再通过 TransformToBoneSpace 变成 hand_r 空间；bLeftHandIKValid 只表示数据可用，必须接到 FABRIK 的启用输入才会限制求解器。

Development Editor Build 成功。客户端实际装备后，服务器与拥有客户端输出一致变换；未装备的四个实例全部关闭 IK 并重置变换。未改变网络权威规则，复用现有武器复制与附着。

FABRIK 节点仍未接入。准确步骤见 ../editor/B01-fabrik-animgraph-setup.md；接线后还需双端站立/移动/蹲伏/瞄准视觉验证。原有“握点缺失”记录只代表历史状态。
