# B01：接入左手 FABRIK（UE5.6）

2026-09-08：C++ 已完整构建通过；双人 PIE 已确认客户端装备后，服务器与拥有客户端得到相同的 LeftHandTransform，未装备时 bLeftHandIKValid=false。尚未创建 FABRIK 节点，以下接线与视觉验收待完成。

## 打开与接线

1. 打开 `Content/Blueprints/Character/Animation/ABP_Blaster`，进入顶层 `AnimGraph`，不要进入 Equipped 状态机内部。
2. 现有 `Equipped` 连到 `Blend Poses by bool` 的 True Pose（内部名 BlendPose_0）。只替换这条线，保持 Unequipped 到 False Pose 的连接。
3. 添加 FABRIK。将装备分支接成下图；从本地姿势接到 FABRIK 时，编辑器可能自动插入空间转换节点，已有转换时不要重复添加。

```text
Equipped → Local To Component → FABRIK → Component To Local → Blend Poses by bool.True Pose
Unequipped ───────────────────────────────────────────────→ Blend Poses by bool.False Pose
```

4. 从变量列表拖入 `LeftHandTransform` 的 Get，接到 FABRIK 的 Effector Transform。若变量不显示，开启 Show Inherited Variables；确认当前 Editor 已加载本次构建。
5. 选中 FABRIK，在 Details 中配置：

| 字段 | 值 |
| --- | --- |
| Tip Bone | hand_l |
| Root Bone | upperarm_l |
| Effector Transform Space | Bone Space |
| Effector Target / Use Socket | false（使用骨骼） |
| Effector Target / Bone Reference / Bone Name | hand_r |
| Effector Rotation Source | Maintain Component Space Rotation（先保持动画手腕旋转） |
| Alpha Input Type | Bool |
| Alpha Bool Blend / Blend Settings | Blend In Time 和 Blend Out Time 都设为 0，确保无效数据立即关闭 |

6. 将 `bLeftHandIKValid` 的 Get 接到 FABRIK 的布尔启用输入（显示名可能为 Enabled / bEnabled；原生属性名 bAlphaBoolEnabled）。不要把 Alpha 固定为 1。
7. Compile 应为 0 errors / 0 warnings，再 Save。

UE5.6 的 `EffectorTarget` 取代旧课程的 `EffectorTransformBone` 配置；后者在当前引擎头文件中是 DEPRECATED。Bone Space 指的是 Effector 相对于 hand_r；FABRIK 的姿势输入仍需 Component Space，两者并不冲突。

## 验收

Listen Server + 1 Client，双方轮流拾取武器，观察站立、移动、瞄准和蹲伏时左手是否贴枪；同时检查未装备姿势不受 IK 影响。保存接线后告知代理，代理可回读图、检查字段、重新编译并继续自动验证。

若左手被拉到错误位置，优先核对 Bone Space、hand_r、LeftHandTransform 连接及有效开关；若只出现握持位置偏差，需要在预览中调整武器 LeftHandSocket。不要以更换为 Muzzle 修复。

## 本机源码依据

- `E:/programs/Epic Games/UE_5.6/Engine/Source/Runtime/AnimGraphRuntime/Public/BoneControllers/AnimNode_Fabrik.h`：EffectorTarget、RootBone、TipBone 与空间类型。
- `E:/programs/Epic Games/UE_5.6/Engine/Source/Runtime/AnimGraphRuntime/Public/BoneControllers/AnimNode_SkeletalControlBase.h`：AlphaInputType、bAlphaBoolEnabled、AlphaBoolBlend。

工具边界：本次 Unreal MCP 的 build_graph/add_node 不支持 AnimGraphNode_Fabrik；Editor Python 暴露节点类型，但没有图节点创建、初始化和插入接口。因此没有修改插件或尝试构造不完整动画节点。
