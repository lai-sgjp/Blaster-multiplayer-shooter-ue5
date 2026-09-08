# B01：目标骨骼 Aim Offset 的 Editor 准备

这一步由用户在 Unreal Editor 中完成，Codex 在资产出现后继续自动检查和接线。此时不要修改 `Content/Blueprints/Character/Animation/ABP_Blaster`。

## 已确认的资源位置

- 目标 Skeletal Mesh：`Content/Assets/LearningKit/LearningKit_Games/Assets/Characters/Character/Mesh/SK_EpicCharacter`
- 目标 Skeleton：`Content/Assets/LearningKit/LearningKit_Games/Assets/Characters/Character/Mesh/SK_EpicCharacter_Skeleton`
- 目标动画目录：`Content/Assets/LearningKit/LearningKit_Games/Assets/Characters/Character/Animations/ThirdPerson_Retarget/`
- 源步枪 Aim 姿势：`Content/Characters/Mannequins/Anims/Rifle/AIM/`
- 现有源 Aim Offset：`Content/Characters/Mannequins/Anims/Rifle/AIM/AO_Rifle`；它绑定 `SK_Mannequin`，不能直接给目标 ABP 使用。

## A. 创建目标 IK Rig

1. 在 Content Browser 导航到 `Content/Assets/LearningKit/LearningKit_Games/Assets/Characters/Character/Mesh/`。
2. 右键 `SK_EpicCharacter`（注意不要右键同目录的 `SK_EpicCharacter_Skeleton`）→ `Animation` → `Create IK Rig`。
3. 将资产保存到 `Content/Assets/LearningKit/LearningKit_Games/Assets/Characters/Character/Animations/ThirdPerson_Retarget/`，命名为 `IK_EpicCharacter`。
4. 打开 `IK_EpicCharacter`，在 IK Rig Editor 的预览网格/Preview Mesh 字段确认是 `SK_EpicCharacter`。
5. 在左侧 Hierarchy 选中 `root`，右键选择 `Set Retarget Root`。
6. 在 `Retarget Chains` 面板使用 `Auto Generate Retarget Chains`（如果 UE5.6 当前界面提供该按钮）。若没有该按钮，点击 `Add New Chain`，至少建立并检查这些链：`pelvis`、`spine`、`neck/head`、左右 `clavicle/upperarm/lowerarm/hand`、左右 `thigh/calf/foot`。起始骨骼和结束骨骼必须从当前 Hierarchy 选择，不要手写不存在的名称。
7. 保存 `IK_EpicCharacter`。预期结果：打开资产时没有 Skeleton mismatch；Preview Mesh 显示目标 EpicCharacter，Retarget Chains 至少覆盖躯干、双臂和双腿。

## B. 创建 Manny 到 EpicCharacter 的 IK Retargeter

1. 优先检查现有源 IK Rig：`Content/Characters/Burnice/IK_VRM4U_UE5Mannequin`。打开后确认其 Preview Mesh 为 `Content/Characters/Mannequins/Meshes/SKM_Manny_Simple`。
2. 若预览网格不是 Manny，回到 `Content/Characters/Mannequins/Meshes/`，右键 `SKM_Manny_Simple` → `Animation` → `Create IK Rig`，按 A 节方式为它建立源 Rig。
3. 在 Content Browser 右键源 IK Rig → `Create IK Retargeter`，Source IK Rig 选择 Manny Rig，Target IK Rig 选择刚保存的 `IK_EpicCharacter`。
4. 将 Retargeter 保存到 `Content/Assets/LearningKit/LearningKit_Games/Assets/Characters/Character/Animations/ThirdPerson_Retarget/`，命名为 `RTG_Mannequin_to_EpicCharacter`。
5. 打开该 Retargeter，检查 `Chain Mapping`：源链和目标链应成对映射；至少确认躯干、双臂、双腿和手部链不是红色/空映射。保存。

## C. 重定向三张 Aim 姿势

1. 在 Content Browser 打开 `Content/Characters/Mannequins/Anims/Rifle/AIM/`。
2. 只选择 `MM_Rifle_Idle_ADS_AO_CU`、`MM_Rifle_Idle_ADS_AO_CC`、`MM_Rifle_Idle_ADS_AO_CD`。
3. 右键 → `Retarget Animation` → `Duplicate Anim Assets and Retarget`（若当前版本菜单名称不同，在打开的 Retargeter 中使用 `Export Selected Animations`）。
4. Retargeter 选择 `RTG_Mannequin_to_EpicCharacter`，输出目录选择 `Content/Assets/LearningKit/LearningKit_Games/Assets/Characters/Character/Animations/ThirdPerson_Retarget/`，名称保留 `AO_CU`、`AO_CC`、`AO_CD` 后缀。
5. 打开导出的三张动画，在 Details 面板的 `Skeleton` 只读字段确认均为 `SK_EpicCharacter_Skeleton`。如果仍是 `SK_Mannequin`，立即停止，不要创建 Aim Offset，检查 Retargeter 的 Target IK Rig 和输出目标。

## D. 创建目标 Aim Offset 资产

> 当前进度：`AO_EpicCharacter_Rifle` 已存在，Skeleton、Preview Base Pose 和 Pitch 轴已由 Codex 核验/设置完成；你现在不需要再改 Details 左侧的轴字段。

1. 如果目标目录中还没有带 `AO_CU`、`AO_CC`、`AO_CD` 后缀的三张目标动画，先按 C 节从 `Content/Characters/Mannequins/Anims/Rifle/AIM/` 重定向；`Idle_Aiming`、`Idle_Equipped`、`Crouch_Idle_Aim` 是普通状态/基础姿势，不能单独替代上、中、下三个 Aim 样本。
2. 打开 `AO_EpicCharacter_Rifle`，在右下 `Asset Browser` 的搜索框逐个搜索三张目标动画。
3. 将 `AO_CU` 拖到样本网格 Pitch 的 `+90` 端，将 `AO_CC` 拖到 `0` 中点，将 `AO_CD` 拖到 `-90` 端。当前资产是单轴配置；如果界面显示 Pitch 在水平轴，就沿底部水平线放置：左侧 `-90`、中间 `0`、右侧 `+90`。不要把点放到垂直轴。
4. 如果拖放后的点位置不精确，选中样本点后直接在网格上拖动到对应刻度；UE5.6 的 Aim Offset 编辑器通常把点坐标显示在选中样本的详情/提示中。
5. 在预览窗口把预览点拖到 `-90`、`0`、`+90`，预期分别看到低头、平视、抬头趋势；如果方向相反，只交换 CU/CD 的位置，不修改 C++。
6. 保存后只需向 Codex 报告三张目标样本动画的完整相对位置和 `AO_EpicCharacter_Rifle` 位置。此时不要自己接入 ABP，Codex 会先读取样本与轴，再扩展现有 AnimGraph。

## 常见失败排查

- 菜单没有 `Create IK Rig`：确认右键的是 Skeletal Mesh `SK_EpicCharacter`/`SKM_Manny_Simple`，不是 Skeleton 或普通动画。
- IK Rig 预览全黑或骨骼不对：检查 `Preview Mesh` 是否指向对应 Skeletal Mesh，并重新设置 `root` 为 Retarget Root。
- Retargeter 链映射为空：源 Rig 和目标 Rig 的链名/骨骼端点没有建立，先回到各自 `Retarget Chains` 面板补链。
- 导出动画仍显示 `SK_Mannequin`：没有真正执行重定向，或导出时选错 Target IK Rig；不要把它接到 `ABP_Blaster`。
- Aim Offset 创建菜单缺失：先确认三张输入动画已经是目标 Skeleton 的 `AnimSequence`，并通过右键菜单的 Animation 分类创建单轴资产。

## E. 接入现有 ABP_Blaster AnimGraph

### 先理解接线关系

站立和蹲伏都已经有自己的 `bAiming` 分支。Aim Offset 只应处理“瞄准姿势”这一侧，不应放在顶层 `bWeaponEquipped` 状态机之后，否则未装备武器时也会套用步枪瞄准姿势。

站立状态的现有关系是：

`Idle_Aiming` → `Blend Poses by bool` 的 `BlendPose_0` → `State Result`

蹲伏状态的现有关系是：

`Crouch_Idle_Aim` → `Blend Poses by bool` 的 `BlendPose_0` → `State Result`

Aim Offset 的正确关系是：

`Idle_Aiming/Crouch_Idle_Aim` → Aim Offset 的 `Base Pose`；Aim Offset 的 `Pose` → 原 `BlendPose_0`。`AO_Pitch` → Aim Offset 的单轴数值输入（通常显示为 `X`；若节点显示 `Pitch`，连接到 `Pitch`）。

### 站立 Idle 接线

1. 在 Content Browser 导航到 `Content/Blueprints/Character/Animation/`，双击 `ABP_Blaster`。
2. 在左侧 `My Blueprint` 面板双击 `AnimGraph`，双击 `Equipped` 状态机，再双击 `Idle` 状态。
3. 找到标题为 `Sequence Player 'Idle_Aiming'` 的节点。它的 Pose 输出当前直接连到 `Blend Poses by bool` 的 `BlendPose_0`。
4. 从 `Idle_Aiming` 的 Pose 输出引脚拖出，在搜索框输入 `Aim Offset`，选择 `Aim Offset`/`Aim Offset Blend Space Player` 节点。
5. 选中新增节点，在右侧 `Details` 面板的 Asset/Animation Asset 字段选择 `AO_EpicCharacter_Rifle`，完整资产位置为 `Content/Assets/LearningKit/LearningKit_Games/Assets/Characters/Character/Animations/ThirdPerson_Retarget/AO_EpicCharacter_Rifle`。
6. 将 `Idle_Aiming` Pose 接到 Aim Offset 的 `Base Pose` 输入。
7. 在 `My Blueprint` 的 Variables 中找到 `AO_Pitch`，拖入图表创建变量读取节点；将其输出接到 Aim Offset 的 `X` 或 `Pitch` 输入。
8. 删除原来 `Idle_Aiming` 到 `BlendPose_0` 的直连线，把 Aim Offset 的 Pose 输出接到同一个 `BlendPose_0` 输入。

### 蹲伏 CrouchIdle 接线

1. 返回 `Equipped` 状态机，双击 `CrouchIdle` 状态。
2. 找到标题为 `Sequence Player 'Crouch_Idle_Aim'` 的节点。
3. 按站立步骤 4–8 再接入一个 Aim Offset 节点，Asset 仍选择 `AO_EpicCharacter_Rifle`，输入仍使用 `AO_Pitch`。
4. 只替换 `Crouch_Idle_Aim` 到该状态 `Blend Poses by bool` 的 `BlendPose_0` 直连线；不要改变 `Crouch_Idle_Rifle_Ironsights`、`bAiming` 或状态机转换。

### 保存与预期结果

1. 点击工具栏 `Compile`，预期无红色错误。
2. 点击 `Save All` 或保存 `ABP_Blaster`。
3. 两个状态图都应满足：Aim Offset 只有一个 Base Pose 输入、一个 `AO_Pitch` 数值输入，输出回到原 `BlendPose_0`；`BlendPose_1` 和 `bAiming` 连线保持不变。
4. 完成后告诉 Codex“站立和蹲伏两个分支已接线并保存”。Codex 会自动读取蓝图、编译、审查，并继续 PIE 验证。

## F. PIE 运行时验证

静态编译只能证明节点类型和连线合法，不能证明角色真的随瞄准输入改变姿势。当前 MCP 运行时采样器不能枚举原生 `AnimInstance` 上的 `AO_Pitch`、`YawOffset`、`Lean`，所以这一项需要在 Editor 中观察画面。

### 单机验证

1. 确认 `ABP_Blaster` 已保存，关闭可能仍在运行的 PIE 后点击工具栏 `Play`。
2. 使用项目现有的拾取、装备和瞄准输入，让角色进入装备并瞄准状态。
3. 保持站立，缓慢向上、向下移动视角。预期枪械上抬/下压和上半身姿势连续变化。
4. 蹲下后再次上下移动视角。预期 `CrouchIdle` 分支也会产生同样的连续变化。
5. 松开瞄准或卸下武器。预期非瞄准/未装备姿势仍由原来的 `BlendPose_1` 和顶层状态机控制，不出现步枪 Aim Offset 残留。

### Listen Server 双端验证

1. 点击 `Play` 右侧下拉箭头，在 `Number of Players` 设置为 `2`，网络模式选择 `Play As Listen Server`，再点击 `Play`。
2. 在客户端角色上装备并瞄准，上下移动视角；切换到服务器窗口观察该远端角色。
3. 再在服务器角色上上下瞄准，切换到客户端窗口观察远端角色。
4. 预期双方远端角色的瞄准姿势都随视角更新，没有只在拥有端生效、远端卡在平视或突然跳变的情况。

### 失败排查

- 完全不变化：回到 `Content/Blueprints/Character/Animation/ABP_Blaster` 的 `Idle` 和 `CrouchIdle`，确认 Aim Offset 的 `X/Pitch` 输入确实连接 `AO_Pitch`，并确认节点 Asset 是 `AO_EpicCharacter_Rifle`。
- 方向相反：在 `Content/Assets/LearningKit/LearningKit_Games/Assets/Characters/Character/Animations/ThirdPerson_Retarget/AO_EpicCharacter_Rifle` 中交换 `AO_CU` 与 `AO_CD` 的位置；不要修改 C++。
- 站立有效、蹲伏无效：只检查 `CrouchIdle` 的 `Crouch_Idle_Aim -> BasePose -> Aim Offset -> BlendPose_0` 链，不改状态转换。
- 远端不更新：先确认远端角色确实进入 `bAiming` 分支，再检查角色移动/视角复制；不要为 `AO_Pitch` 新增 RPC。

## G. 接入装备移动、YawOffset 与 Lean

### 为什么不能直接替换 Idle

`BS_EquippedRun` 没有 Idle 样本，中心附近就是 Jog 姿势。如果把它直接替换 `Idle` 状态，角色站着不动时也会跑步。因此要把它放进只在移动时进入的 `Run` 状态。

### 创建状态

1. 在 `Content/Blueprints/Character/Animation/ABP_Blaster` 中打开 `AnimGraph` → `Equipped` 状态机。
2. 在空白处右键 → `Add State`，创建 `Run`。
3. 再创建 `CrouchRun`。它用于避免蹲下移动时播放站立的 `BS_EquippedRun`。

### Run 状态图

1. 双击 `Run` 状态。
2. 从 Content Browser 拖入 `Content/Blueprints/Character/Animation/BS_EquippedRun`，生成 Blend Space Player。
3. 将 Blend Space Player 的输出接到 `State Result`。
4. 从 `My Blueprint → Variables` 拖出 `YawOffset` 和 `Lean` 两个读取节点。
5. `YawOffset` → Blend Space Player 的 `X`；`Lean` → `Y`。
6. 预期：X 控制左右/前后方向偏移，Y 控制四组 Lean 动画之间的垂直选择；不要把 `Speed` 接到这个资产，因为它的 X 轴不是 Speed。

### CrouchRun 状态图

1. 双击 `CrouchRun` 状态。
2. 从 Content Browser 拖入 `Content/Assets/LearningKit/LearningKit_Games/Assets/Characters/Character/Animations/EpicCharacter_CrouchWalk_2D`。
3. 将输出接到 `State Result`，从 `My Blueprint → Variables` 拖出 `Speed` 并接到 Blend Space Player 的 `X`。
4. 该现有蹲伏资产是 1D Speed Blend Space，没有方向/Lean 轴；本阶段先保证蹲伏移动不播放站立 Jog，蹲伏移动的方向 Lean 将在后续有目标资产后补齐。

### 状态转换

在 `Equipped` 状态机中，用状态输出引脚拖到目标状态创建转换；双击每条转换，在规则图中使用 `My Blueprint → Variables` 的布尔变量，并按下表设置。`AND` 使用 `AND` 节点，取反使用 `NOT` 节点，最终连接转换结果节点的 `Result`。

| 转换 | 规则 |
| --- | --- |
| `Idle → Run` | `bIsAccelerating AND NOT bIsCrouched` |
| `Run → Idle` | `NOT bIsAccelerating AND NOT bIsCrouched` |
| `Run → CrouchIdle` | `bIsCrouched AND NOT bIsAccelerating` |
| `Idle → CrouchIdle`（已有） | `bIsCrouched` |
| `CrouchIdle → Idle`（已有） | `NOT bIsCrouched` |
| `CrouchIdle → Run` | `NOT bIsCrouched AND bIsAccelerating` |
| `Run → CrouchRun` | `bIsCrouched AND bIsAccelerating` |
| `CrouchRun → Run` | `NOT bIsCrouched AND bIsAccelerating` |
| `CrouchIdle → CrouchRun` | `bIsCrouched AND bIsAccelerating` |
| `CrouchRun → CrouchIdle` | `bIsCrouched AND NOT bIsAccelerating` |
| `CrouchRun → Idle` | `NOT bIsCrouched AND NOT bIsAccelerating` |

不要删除原有 `Idle ↔ CrouchIdle` 转换；只按表补齐移动状态的转换。保存并 Compile 后，预期 `Equipped` 状态机有 `Idle / Run / CrouchIdle / CrouchRun` 四个状态，且停止移动时不会停在 Run。

### 当前实际图的纠正项

Codex 已回读当前蓝图，发现四个状态虽然都存在，但还需要以下修正：

1. 当前缺少 `Idle → Run`。在 `Idle` 状态输出引脚拖到 `Run` 状态输入引脚创建转换，规则为 `bIsAccelerating AND NOT bIsCrouched`。
2. 当前缺少 `Run → Idle`。从 `Run` 输出拖到 `Idle` 输入创建转换，规则为 `NOT bIsAccelerating AND NOT bIsCrouched`。
3. 双击现有 `CrouchRun → Run` 转换，把规则改为 `NOT bIsCrouched AND bIsAccelerating`。不要使用 `NOT (bIsAccelerating AND bIsCrouched)`。
4. 双击现有 `CrouchRun → CrouchIdle` 转换，把规则改为 `bIsCrouched AND NOT bIsAccelerating`。
5. 增加 `Run → CrouchIdle`，规则为 `bIsCrouched AND NOT bIsAccelerating`。
6. 增加 `CrouchIdle → Run`，规则为 `NOT bIsCrouched AND bIsAccelerating`。
7. 增加 `CrouchRun → Idle`，规则为 `NOT bIsCrouched AND NOT bIsAccelerating`。
8. 原有 `Idle ↔ CrouchIdle`、`Run ↔ CrouchRun`、`CrouchIdle ↔ CrouchRun` 连接保留；不要重复创建同方向转换。

回读用户本次修改后，当前只剩两条规则方向互换，需要再次打开对应转换修正：

- `Run → CrouchIdle` 当前误设为 `NOT bIsCrouched AND bIsAccelerating`，应为 `bIsCrouched AND NOT bIsAccelerating`。
- `CrouchRun → Run` 当前误设为 `bIsCrouched AND NOT bIsAccelerating`，应为 `NOT bIsCrouched AND bIsAccelerating`。

其他五条新增转换和两个状态内部图已经存在，不要重复创建或修改。

完成后状态机应能形成四种稳定状态：站立不动 `Idle`、站立移动 `Run`、蹲伏不动 `CrouchIdle`、蹲伏移动 `CrouchRun`。

### 失败排查

- `YawOffset` 或 `Lean` 不在 Variables：先 Compile `ABP_Blaster`，关闭并重新打开蓝图；若仍不存在，停止操作并报告，不要创建同名变量。
- Run 状态静止时播放跑步：检查 `Idle → Run` 是否错误地只连接了 `bIsCrouched`，或把 `BS_EquippedRun` 放进了 Idle 状态。
- 前后方向反了：先记录实际表现，不要立即交换 X 轴；需要确认 `YawOffset` 的符号和资产样本方向后再改。
- CrouchRun 没有 Lean：这是当前 `EpicCharacter_CrouchWalk_2D` 只有 Speed 轴的已知限制，不要把站立 `BS_EquippedRun` 强接到蹲伏状态。
