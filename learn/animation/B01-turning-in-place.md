# B01：原地转身资源准备与证据

## 为什么不能直接连接 Rotate Root Bone

Actor Rotation 决定角色整体朝向；Rotate Root Bone 在动画姿势中旋转根骨骼，会影响其全部后代。需要同时确定上身如何保持瞄准方向、腿部何时转身、转身后如何恢复根偏移。现有 AO_EpicCharacter_Rifle 只有 Pitch -90/0/+90 三个样本，无 Yaw 覆盖。新增几个变量并不能完成这个闭环。

UE5.6 本机源码：`E:/programs/Epic Games/UE_5.6/Engine/Source/Runtime/AnimGraphRuntime/Private/AnimNodes/AnimNode_RotateRootBone.cpp` 的 Evaluate_AnyThread 将目标旋转转换到 Mesh Space 并乘到根骨骼旋转。

## 2026-09-08 已执行

现有 Mixamo `Turn_Left_Anim` 和 `Turn_Right_Anim` 使用 Kachujin skeleton，与 ABP 的 SK_EpicCharacter_Skeleton 不同。现有 IK_EpicCharacter 的重定向链列表为空，不能直接作为有效链映射使用。

在 `Content/Blueprints/Character/Animation/B01Turn/` 新建两个专用 IK Rig，均配置 22 条链（Root、Spine、Neck、Head、双臂/腿/脚趾/锁骨与十条手指）。源根为 Hips，目标根为 pelvis。新建 RTG_B01_Turn，使用默认操作栈、精确同名链匹配和目标姿势自动对齐；原资产保持不变。

通过 IKRetargetBatchOperation.duplicate_and_retarget 导出左右动画；该 API 实际把临时动画放在 /Game 根目录，随后按工具返回的实际路径迁入目标目录。根目录可能保留本次重命名的 redirector，未清理或改动其他文件。

| 目标动画 | Skeleton | 时长 |
| --- | --- | --- |
| Turn_Left_B01 | SK_EpicCharacter_Skeleton | 1.2333 秒 |
| Turn_Right_B01 | SK_EpicCharacter_Skeleton | 1.4333 秒 |

两个动画均已保存。取开头、中间、结尾采样，pelvis、thigh_l、foot_l、head 均有时间变化，排除了纯参考姿势输出；root 在这些样本中保持不变。此检查不证明视觉可用或无滑步。

## UE5.6 API 迁移发现

`IKRetargeterController.get_all_chain_settings()` 返回空列表不等于新操作栈未配置。当前引擎 `IKRetargeterController.cpp:1466` 明确返回 ChainSettings_DEPRECATED。应读取 FK op controller 的 get_settings 和 get_source_chain；本次 FK 配置包含 22 条目标链，LeftArm/RightLeg 的源映射正确。

导出警告包含 `Track with name head already exists` 和 InterchangeAssetImportData 依赖加载提示，未据此宣称完全无警告。

## 当前质量阻塞

导出转身动画的初始 pelvis 高度约 32.5，现有 Idle_Equipped 为 70.4；源 Turn_Left_Anim.Hips 本身约 49.2。可能涉及源姿势或重定向比例，尚不能确认。未直接补偿高度、未接入 ABP、未改变 Character 旋转逻辑。

下一步是按 `../editor/B01-turn-animation-preview.md` 观察候选动画，确定是否能用于持枪转身，然后冻结完整原地转身实现合同。当前状态为资源候选已生成，功能未完成。
