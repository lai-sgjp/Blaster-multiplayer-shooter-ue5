# 动画数据链路：AnimInstance、Aim Offset、Lean、转身与 FABRIK

## 1. 本节目标

动画系统在这个项目里是“Gameplay 状态的表现层”。你需要理解：C++ 产生哪些参数，AnimGraph 如何消费参数，哪些状态来自网络复制，以及为什么动画不应成为服务器命中判定的唯一依据。

## 2. 数据入口

`UBlasterAnimInstance::NativeInitializeAnimation` 通过 `TryGetPawnOwner()` 得到 `ABlasterCharacter`。之后 `NativeUpdateAnimation` 每帧读取角色、CharacterMovement、CombatComponent 和当前武器，更新蓝图可读变量。

| AnimInstance 参数 | 数据来源 | 用途 |
| --- | --- | --- |
| `Speed` | 角色水平 Velocity 的长度 | Idle/Run Blend Space |
| `bIsInAir` | `CharacterMovement->IsFalling()` | Jump/Fall 状态 |
| `bIsAccelerating` | 当前加速度是否非零 | Run/CrouchRun 状态转换 |
| `bWeaponEquipped` | `ABlasterCharacter::IsWeaponEquipped()` | Equipped/Unequipped 状态机 |
| `bIsCrouched` | Character 的 `bIsCrouched` | 蹲伏 Idle/Run |
| `bAiming` | CombatComponent 的 `bAiming` | 瞄准姿势/相机表现 |
| `YawOffset` | MovementRotation 与 AimRotation 的归一化差值 | 方向移动和持枪姿势 |
| `Lean` | 相邻帧 Actor Yaw 变化 / DeltaSeconds，再插值 | 身体倾斜 |
| `AO_Pitch` | `GetBaseAimRotation().Pitch` | Aim Offset 的上下瞄准 |
| `LeftHandTransform` | 武器 `LeftHandSocket` 转到角色 `hand_r` 空间 | FABRIK Effector |
| `bLeftHandIKValid` | Socket、骨骼、附着关系全部有效 | 启用/禁用 IK 求解 |

## 3. 远端 Pitch 为什么要映射

本地角色的 `BaseAimRotation.Pitch` 可以直接用于 Aim Offset；远端角色的网络旋转量可能以 `270..360` 表示向下方向。项目在远端且 `AO_Pitch > 90` 时把 `270..360` 映射到 `-90..0`，使动画资产统一使用角度范围。

这不是“把网络同步关掉”，而是对已经到达客户端的旋转数据做表现层归一化。它也说明了面试中的一个重要边界：网络传输格式和动画资产的输入范围可以不同，需要在消费端适配。

## 4. YawOffset 与 Lean 的区别

### YawOffset：移动方向相对瞄准方向

角色向前跑但枪口向右瞄准时，移动方向和 Aim Rotation 的差值会让 Blend Space 选择对应的侧向持枪动画。它描述的是“现在该播放什么移动姿势”。

### Lean：旋转变化产生的视觉倾斜

`Lean` 用相邻两帧 Actor Rotation 的 Yaw 差值除以 DeltaSeconds，再通过 `FInterpTo` 平滑。它描述的是“旋转变化有多快”，不是权威的角色朝向，也不是新的复制变量。

当前 `ABlasterCharacter::Tick` 的策略是：装备武器时使用 Controller Rotation Yaw，未装备时让 CharacterMovement 朝移动方向旋转；远端代理消费复制后的 Actor Rotation。动画不直接旋转 Actor，从而避免表现层和网络运动状态互相打架。

## 5. Aim Offset、Blend Space 和状态机

当前资源路径中的关键资产：

- `Content/Blueprints/Character/Animation/ABP_Blaster`
- `Content/Blueprints/Character/Animation/BS_EquippedRun`
- `Content/Blueprints/Character/Animation/BS_UnequippedIdleWalkRun`
- `Content/Assets/LearningKit/LearningKit_Games/Assets/Characters/Character/Animations/ThirdPerson_Retarget/`

概念分工：

- AnimSequence：单段时间轴动画；
- Blend Space：按 Speed、Yaw 等轴在多个样本之间混合；
- Aim Offset：按 Pitch/Yaw 选择上下/左右瞄准姿势；
- State Machine：根据 `bWeaponEquipped`、`bIsCrouched`、`bIsAccelerating` 等状态决定使用哪一类姿势；
- AnimGraph：把状态机输出、Aim Offset、Slot、Layer 和 IK 节点组成最终 Pose。

一个常见错误是把“有 Aim 动画”当成“有可直接使用的 Aim Offset”。资产必须使用兼容的 Skeleton，轴范围也要和 C++ 传入值一致。项目当前已发现一些 Mannequin 与 `SK_EpicCharacter_Skeleton` 的骨骼兼容/重定向边界，详见 `learn/animation/B01-animation-network.md` 和 `learn/editor/B01-aim-offset-editor-setup.md`。

## 6. 左手 FABRIK 的数据链

项目 C++ 已准备的链路是：

```text
EquippedWeapon->WeaponMesh
        |
        +--> LeftHandSocket 世界变换
        |
        v
CharacterMesh->TransformToBoneSpace("hand_r", ...)
        |
        v
LeftHandTransform + bLeftHandIKValid
        |
        v
AnimGraph FABRIK：Tip=hand_l，Root=upperarm_l，Effector Bone=hand_r
```

`bLeftHandIKValid` 的含义只是“输入数据有效”，它不会自动让 AnimGraph 使用 FABRIK。最终节点仍需要在 `ABP_Blaster` 中接入，并在站立、移动、蹲伏、瞄准和未装备状态下验证。资产前置和 Editor 点击路径见 `learn/editor/B01-fabrik-socket-setup.md`、`B01-fabrik-animgraph-setup.md`。

为什么要先做 Socket/Bone preflight？因为缺少 `LeftHandSocket` 或目标骨骼时，使用武器原点会产生“看似有数据、实际姿势错误”的假修复。动画任务中，资源证据是代码修改的前置条件。

## 7. Turning in Place

`UpdateTurning` 只在持枪、低速、未跳跃、未蹲伏且有左右转动画时启用。它累计 `RootYawOffset`，当偏移超过约 90 度时选择左/右动画，并随着动画播放把剩余根偏移平滑回零。

关键设计点：

- Actor Rotation 仍然是网络运动状态；
- Rotate Root Bone/转身动画只改变最终 Pose；
- 动画曲线 `B01TurnYaw` 用来扣除源动画自身已经包含的旋转，避免重复旋转；
- 动画资源必须与目标 Skeleton 兼容，Mixamo 动画不能未经重定向直接接到目标 ABP。

## 8. 动画与网络的边界

- `Health`、`bEliminated`、`bAiming`、`EquippedWeapon` 等 Gameplay 状态通过网络同步；
- `Speed`、`YawOffset`、`Lean`、`AO_Pitch` 是各端根据当前实例状态计算的表现参数，不需要为了每一帧动画新增 RPC；
- 动画可以显示角色正在瞄准，但不能据此在客户端决定命中；
- Server 端为历史命中采样开启骨骼 Pose 刷新，是因为服务器需要有可查询的骨骼变换，不代表服务器在播放动画 Montage 来结算伤害。

## 9. 引擎源码阅读路线

- AnimInstance：`Engine/Source/Runtime/Engine/Classes/Animation/AnimInstance.h`
- AnimNode 基础：`Engine/Source/Runtime/Engine/Private/Animation/AnimNodeBase.cpp`
- FABRIK：`Engine/Source/Runtime/AnimGraphRuntime/Private/AnimNodes/AnimNode_Fabrik.cpp`
- Rotate Root Bone：`Engine/Source/Runtime/AnimGraphRuntime/Private/AnimNodes/AnimNode_RotateRootBone.cpp`
- Character Movement：`Engine/Source/Runtime/Engine/Classes/GameFramework/CharacterMovementComponent.h`

本项目的实际 C++ 入口是 `Source/Blaster/Character/BlasterAnimInstance.cpp`；先用断点或日志确认参数，再到引擎节点实现里看 Pose 如何被修改。

## 10. 验证步骤

1. 单机观察未装备、装备、移动、蹲伏、跳跃和瞄准。
2. Listen Server + 1 Client：观察本地 Pawn、远端 Pawn 和两端瞄准 Pitch 是否一致。
3. 在 `ABP_Blaster` 中确认 `bIsCrouched AND NOT bIsAccelerating` 会进入蹲伏 Idle，站起且移动会回到 Run。
4. 装备真实武器后检查 `LeftHandSocket`、`hand_r`、`hand_l` 和 `upperarm_l`；未装备时 IK 必须关闭并重置 Transform。
5. 转身时确认是动画 Pose 变化，而不是客户端直接修改复制 Actor Rotation。

## 11. 面试快答

**问：为什么不把 Lean 或 YawOffset 复制给所有客户端？**

答：它们是各端根据已复制的运动和瞄准状态计算出的视觉参数，不是新的权威 Gameplay 状态。复制每帧 Lean 会增加网络成本，也会把表现算法和网络状态耦合；只有当表现依赖服务器无法重建的离散事件时，才考虑复制事件或状态。

**问：FABRIK 的 Root、Tip 和 Effector 分别是什么？**

答：Tip 是要被约束的末端骨骼，这里是左手 `hand_l`；Root 是允许求解的骨骼链起点，这里是 `upperarm_l`；Effector 是目标位置/旋转来源，这里用右手空间中的武器 `LeftHandSocket` 变换。

**问：为什么远端 Aim Pitch 经常需要额外映射？**

答：网络旋转的角度表示可能跨越 0/360 边界，远端得到的值不一定落在动画资产使用的 `-90..90` 范围。应在 AnimInstance 消费端归一化，而不是新增一个重复的网络变量。

## 12. 小练习

1. 给 `Speed`、`YawOffset`、`AO_Pitch` 和 `LeftHandTransform` 分别写出“来源—变换—AnimGraph 消费者”。
2. 如果装备武器时左手突然跑到原点，按“资源、附着、Socket、骨骼、空间、AnimGraph”顺序列出排查链。
3. 解释为什么 CharacterMovement 的朝向策略和 Rotate Root Bone 不能同时随意修改 Actor Rotation。
