# B01：检查自动重定向的转身候选动画

代理已新建独立的 Rig、Retargeter 和两张目标骨架动画，尚未接入 ABP_Blaster。请先检查候选动画的姿势质量，不要直接用它们替换已通过的 Idle。

## 打开两张动画

在 Content Browser 中打开：

- `Content/Blueprints/Character/Animation/B01Turn/Turn_Left_B01`
- `Content/Blueprints/Character/Animation/B01Turn/Turn_Right_B01`

确认 Preview Mesh 为 `Content/Assets/LearningKit/LearningKit_Games/Assets/Characters/Character/Mesh/SK_EpicCharacter`。分别播放一遍，并拖动时间轴检查开头、中间和结尾。

需要观察：人物是否正常站立、双脚是否接近地面、身体有没有异常压低或扭曲、左右转身方向是否合理。特别检查起始姿势：自动采样得到 pelvis 高度约 32.5，而现有 `Content/Assets/LearningKit/LearningKit_Games/Assets/Characters/Character/Animations/ThirdPerson_Retarget/Idle_Equipped` 的起始 pelvis 高度约 70.4。源 Turn_Left_Anim 的 Hips 高度本身约 49.2，因此不能认定只调整目标高度就能修复。

回报时说明“能正常站立转身”或具体异常（例如蹲得很低、脚陷地、身体扭曲）。不要求你重做 Rig；代理已配置好独立副本，拿到视觉判断后可以继续定位源动作与重定向姿势。

## 已生成的配置资产

- `Content/Blueprints/Character/Animation/B01Turn/IK_B01_Kachujin`：源 Rig，22 条链，重定向根 Hips。
- `Content/Blueprints/Character/Animation/B01Turn/IK_B01_EpicCharacter`：目标 Rig，22 条链，重定向根 pelvis。
- `Content/Blueprints/Character/Animation/B01Turn/RTG_B01_Turn`：精确同名链映射，目标自动对齐。

原有 Rig 和动作未修改。若候选动作姿势异常，不要直接在动画上抬高骨盆或覆盖源动作；这可能掩盖腿长、参考姿势或源动作类型的问题。

## 尚未完成的内容

候选动画通过姿势检查后，仍需确定上身 Yaw 的处理方式，再实现原地转身状态、Rotate Root Bone 与多人验证。当前 `AO_EpicCharacter_Rifle` 只有 Pitch 三个样本，不能直接当作二维 Yaw/Pitch Aim Offset 使用。B01 整包及后续射击系统尚未完成。
