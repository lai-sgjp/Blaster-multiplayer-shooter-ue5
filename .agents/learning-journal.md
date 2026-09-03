# Blaster 项目学习知识库

维护本知识库的目的是将你从 Blaster 项目中学到的 UE5 C++ 多人游戏开发知识点系统化记录，便于复习和面试准备。每次有新的学习内容（新功能、调试经验、源码理解）时，在此文档末尾追加记录。

---

## 1. UE5 项目构建系统

### 1.1 Target.cs 文件体系

**知识点：** UE5 通过 Target.cs 文件定义构建目标。Game Target 和 Editor Target 分别控制游戏包和编辑器的编译配置。

**项目应用：**
- Blaster.Target.cs — 游戏目标，TargetType.Game
- BlasterEditor.Target.cs — 编辑器目标，TargetType.Editor
- 统一使用 BuildSettingsVersion.V5 和 EngineIncludeOrderVersion.Unreal5_6，匹配 UE5.6 引擎版本

### 1.2 Build.cs 模块依赖管理

**知识点：** UE 的模块化构建系统通过 Build.cs 声明模块依赖。依赖分 Public 和 Private 两种作用域，Public 依赖会透传给引用此模块的其他模块，Private 则不会。

**项目应用（Blaster.Build.cs）：**
PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
PrivateDependencyModuleNames.AddRange(new string[] {"Slate", "SlateCore"});

- 额外在 .uproject 的 AdditionalDependencies 中声明了 "UMG" 用于支持 Widget

### 1.3 .uproject 中的插件管理

**知识点：** .uproject 文件不仅声明项目模块，还控制插件是否启用及其作用范围。

**项目应用（Blaster.uproject）：**
- OnlineSubsystemSteam — 全部平台启用（Steam 在线子系统）
- SteamSockets — 全部平台启用（Steam P2P 网络传输层）
- PCG / PCGGeometryScriptInterop — 全部平台启用（程序化生成关卡）
- ModelingToolsEditorMode — 仅 Editor 启用（建模工具）

---

## 2. UE5 Enhanced Input 系统

### 2.1 核心概念

| 概念 | 说明 |
| --- | --- |
| Input Action | 定义输入事件的身份（如 Move, Jump），包含 Value Type（bool, float, Vector2D 等） |
| Input Mapping Context (IMC) | 将 Input Action 映射到具体按键/轴，支持优先级叠加 |
| Enhanced Input Component | 替代旧版 InputComponent，绑定 Action 到回调 |
| Trigger Event | 控制回调解发时机（Triggered, Started, Completed, Ongoing） |

### 2.2 标准绑定流程

步骤：
1. 在 C++ 中用 UPROPERTY(EditAnywhere, BlueprintReadOnly) 声明 UInputAction* 和 UInputMappingContext* 指针
2. 在蓝图中创建 IA_ 和 IMC_ 资产，赋值给 Character
3. BeginPlay 中将 IMC 添加到 UEnhancedInputLocalPlayerSubsystem
4. SetupPlayerInputComponent 中通过 CastChecked<UEnhancedInputComponent> 绑定回调

### 2.3 FInputActionValue 提取

Enhanced Input 的 Action 值统一通过 FInputActionValue 传递，通过模板方法 Get<T>() 提取。

移动方向计算：FVector2D 的 Y 对应 W/S（前后），X 对应 A/D（左右）

**工程经验：**
- 移动和视角用 ETriggerEvent::Triggered（持续触发）
- 一次性操作（跳跃、装备）用 ETriggerEvent::Started（按下一次触发一次）
- 不要用 ETriggerEvent::Completed 来绑定"按下"操作——它在按键抬起时才触发

---

## 3. 角色移动与相机系统

### 3.1 CharacterMovementComponent 配置

**知识点：** ACharacter 内置 UCharacterMovementComponent，通过 GetCharacterMovement() 访问。

**项目配置：**
- GetCharacterMovement()->bOrientRotationToMovement = true（角色面朝移动方向）
- GetCharacterMovement()->SetWalkableFloorAngle(46.f)（最大可爬坡角度）
- GetCharacterMovement()->MaxStepHeight = 60.f（最大台阶高度）
- bUseControllerRotationYaw = false（角色不自动旋转）

**网络同步注意事项：** bOrientRotationToMovement = true 搭配 bUseControllerRotationYaw = false 是 TPS 多人游戏的标配——角色的 Yaw 旋转由移动方向驱动，摄像机旋转由 Controller 控制。

### 3.2 SpringArm + Camera 第三人称相机

**知识点：** USpringArmComponent 提供弹簧臂效果处理碰撞回弹和延迟跟随；UCameraComponent 作为视口。

**关键要点：**
- bUsePawnControlRotation = true（弹簧臂）——弹簧臂的 Yaw/Pitch 跟随鼠标/手柄输入
- bUsePawnControlRotation = false（相机）——相机保持相对弹簧臂固定
- bDoCollisionTest + ProbeSize 控制相机碰撞检测

### 3.3 视角输入处理

AddControllerYawInput(LookAxisValue.X) 控制水平视角
AddControllerPitchInput(LookAxisValue.Y) 控制垂直视角

---

## 4. UE 反射系统

### 4.1 反射宏体系

| 宏 | 用途 | 检查要点 |
| --- | --- | --- |
| UCLASS() | 标记 UObject 派生类 | 必须有 GENERATED_BODY() |
| USTRUCT() | 标记结构体 | 可在内部使用 UPROPERTY |
| UENUM() | 标记枚举 | 建议用 uint8 作为类型 |
| UPROPERTY() | 标记成员变量 | 决定 GC、复制、蓝图可见性 |
| UFUNCTION() | 标记成员函数 | 支持 RPC、蓝图调用、委托绑定 |
| GENERATED_BODY() | 生成反射代码 | 每个 UCLASS/USTRUCT/UENUM 都必须有 |

### 4.2 UPROPERTY 常见用法

- VisibleAnywhere — 运行时只读，编辑器中可见
- EditAnywhere, BlueprintReadOnly — 编辑器中可配置，蓝图中只读
- VisibleAnywhere, meta = (AllowPrivateAccess = "true") — 私有变量但对蓝图编辑器可见
- meta = (BindWidget) — 自动绑定到同名的 UMG 控件
- Replicated — 标记变量需要网络复制
- ReplicatedUsing = OnRep_VariableName — 复制并在收到时触发 OnRep

### 4.3 友元类的使用场景

**项目应用：** UCombatComponent 声明 friend class ABlasterCharacter。合理的 Component-Owner 模式。

---

## 5. 网络复制系统

### 5.1 基础设置

三要素：
1. Actor 的 bReplicates = true（构造函数中设置）
2. 组件的 SetIsReplicated(true)（如需组件复制）
3. 在 GetLifetimeReplicatedProps 中注册需要复制的变量

### 5.2 DOREPLIFETIME 与同步条件

- COND_OwnerOnly — 只复制给 Actor 的 Owner（当前角色的客户端）
- 默认 DOREPLIFETIME — 复制给所有客户端

### 5.3 RepNotify (OnRep)

**知识点：** ReplicatedUsing 绑定的函数在客户端收到复制更新时自动调用。Server 端不会调用 OnRep。

**两头执行的模式：** SetWeaponState（Server 直接执行）+ OnRep_WeaponState（客户端收到复制后执行同样的变更）

### 5.4 RPC 系统

| RPC 类型 | 声明前缀 | 执行位置 | 典型用途 |
| --- | --- | --- | --- |
| Server | UFUNCTION(Server, Reliable) | 在 Server 上执行 | 客户端请求 Server 做某事 |
| Client | UFUNCTION(Client, Reliable) | 在 Owning Client 上执行 | Server 通知特定客户端 |
| NetMulticast | UFUNCTION(NetMulticast, Reliable) | 所有客户端 + Server | 全局播放效果 |

**装备 RPC 流程：**
客户端按 E -> EKeyPressed() 判断 HasAuthority()
  - 如果是 Server：直接调用 Combat->EquipWeapon()
  - 如果是 Client：调用 ServerEquipButtonPressed() (Server RPC)

**工程经验：**
- Reliable 保证消息一定到达，适用于装备、射击等关键操作
- Unreliable 适用于频繁更新但可丢失的数据
- RPC 函数名带 _Implementation 后缀
- Server RPC 必须由客户端调用，不能由 Server 自己调用

### 5.5 网络角色 (Net Role) 模型

| Role | 含义 | 谁拥有 |
| --- | --- | --- |
| ROLE_Authority | 权威端（Server） | 只有 Server |
| ROLE_AutonomousProxy | 自主代理（本地客户端） | 当前玩家控制的 Pawn |
| ROLE_SimulatedProxy | 模拟代理（其他客户端） | 其他玩家控制的 Pawn |
| ROLE_None | 无网络角色 | 非复制 Actor |

---

## 6. 武器系统

### 6.1 武器状态机

使用 UENUM 定义武器状态（EWS_Initial / EWS_Equipped / EWS_Dropped），通过 Replicated 变量同步。

### 6.2 拾取碰撞系统

核心架构：WeaponMesh（根组件，无碰撞）+ AreaSphere（Overlap 检测，仅在 Server 启用）+ PickupWidget（默认隐藏）

完整拾取流程：
1. Server 端 BeginPlay：AreaSphere 启用 Overlap
2. 玩家走进 AreaSphere（仅 Server 触发）-> OnSphereOverlap -> SetOverlappingWeapon(this)
3. Server 修改 OverlappingWeapon（Replicated COND_OwnerOnly）
4. 客户端收到 OnRep：新武器显示 Widget，旧武器隐藏

### 6.3 Socket 附着

使用 GetSocketByName 获取角色骨骼上的 Socket，通过 AttachActor 将武器附着。

### 6.4 Widget 显示控制

客户端负责 UI 表现，Server 负责 Overlap 逻辑。

---

## 7. 组件化设计（CombatComponent）

### 7.1 UActorComponent 设计模式

UE5 推荐将功能按职责分解到 UActorComponent 中。

**关键设计决策：**
- PrimaryComponentTick.bCanEverTick = false（性能优化）
- 使用友元类 friend class ABlasterCharacter
- 在 PostInitializeComponents 中建立 Owner-Component 双向引用

### 7.2 PostInitializeComponents 的作用

执行顺序：构造函数 -> PostInitializeComponents -> BeginPlay
构造时 Component 可能还没初始化完成，PostInitializeComponents 确保所有 Subobject 都已就绪。

---

## 8. 动画蓝图集成

### 8.1 UAnimInstance 的工作方式

自定义 AnimInstance 通过 NativeInitializeAnimation 和 NativeUpdateAnimation 替代蓝图事件。

**三个核心参数：**
- Speed — 水平速度大小，决定 Idle/Walk/Run 动画混合
- bIsInAir — 是否在空中，决定跳跃/下落动画
- bIsAccelerating — 是否有加速输入，区分主动移动和惯性滑行

**经验：** TryGetPawnOwner() 在刚初始化时可能返回 nullptr，需要在每帧更新中重试。

### 8.2 C++ 与蓝图的动画协作

C++ 负责每帧更新运动数据，蓝图 ABP 负责驱动状态机和 BlendSpace。

---

## 9. 游戏模式与关卡流

### 9.1 GameMode 职责

AGameMode 只在 Server 上存在，负责游戏规则（默认 Pawn、Controller）。

### 9.2 SeamlessTravel 流程

PostLogin 中检测玩家数，2 人满后用 GameMode::ServerTravel 跳转到游戏地图。bUseSeamlessTravel = true 保持连接不中断，?listen 后缀允许后续加入。

---

## 10. MultiplayerSessions 插件

### 10.1 插件架构

- UMultiplayerSessionsSubsystem（UGameInstanceSubsystem）— 封装 IOnlineSessionPtr
- UMenu（UUserWidget）— Host / Join 按钮，绑定 Subsystem 委托

### 10.2 为什么用 GameInstanceSubsystem

GameInstance 生命周期贯穿整个游戏进程，不受关卡切换影响。Subsystem 比重写 GameInstance 更灵活。

### 10.3 Steam Sockets vs Steam P2P

- Steam P2P — 更成熟但性能较低
- Steam Sockets（UE5.0+ 引入）— 更接近标准 Berkeley Sockets，性能更高

---

## 11. Widget / UI 系统

### 11.1 BindWidget 自动绑定

UPROPERTY(meta = (BindWidget)) 让 UMG 自动将蓝图中同名控件绑定到变量。

### 11.2 Widget 生命周期

NativeDestruct -> RemoveFromParent 防止内存泄漏。

### 11.3 通过 PlayerState 获取玩家名

玩家名称等需要跨网络共享的数据存放在 APlayerState 中，自动复制到所有客户端。

---

## 12. 多人游戏调试技巧

### 12.1 UE_LOG 调试

### 12.2 网络角色可视化

OverheadWidget 显示每个 Pawn 的 RemoteRole，实时判断复制是否正常工作。

### 12.3 常见网络问题检查链

1. bReplicates = true？
2. GetLifetimeReplicatedProps 注册？
3. RPC 声明正确？
4. Server RPC 从客户端调用？
5. OnRep 触发？
6. 数据只从 Server 写入？

---

## 13. 配置系统要点

### 13.1 DefaultEngine.ini
- 网络驱动：SteamSocketsNetDriver
- 渲染：Lumen GI + Lumen Reflection + RTX + 虚拟阴影
- 默认 RHI：DX12 SM6

### 13.2 多平台渲染

Windows (DX12 SM6) + Linux (Vulkan SM6)

### 13.3 关卡打包

MapStartUp -> Lobby -> TransitionLevel -> BlasterMap

---

## 14. 架构经验总结

### 14.1 职责划分

| 类 | 职责 |
| --- | --- |
| Character | 输入、移动、组件管理 |
| ActorComponent | 战斗、武器、生命等可复用子系统 |
| PlayerController | 输入模式、HUD、仅归属该客户端的功能 |
| PlayerState | 玩家名、分数（跨网络共享） |
| GameState | 比赛时间、全局状态 |
| GameMode | 规则、生成（仅 Server） |
| GameInstanceSubsystem | 跨关卡持久数据 |

### 14.2 C++ 与蓝图分工

C++：核心逻辑、网络同步、性能代码。蓝图：动画、UI、关卡编排。

### 14.3 Tick 使用原则

不需要 Tick 的坚决禁用（CombatComponent 和 Weapon 都已关闭）。用 Timer/Callback 替代持续检测。

---

## 15. Character 蹲伏与移动复制

### 15.1 Crouch 是移动组件处理的请求

`ACharacter::Crouch()` 不会立即修改 `bIsCrouched`，而是先检查 `CanCrouch()`，通过后设置 `CharacterMovement->bWantsToCrouch`。`UCharacterMovementComponent` 在后续移动更新中修改胶囊体并调用 `SetIsCrouched(true)`。

要允许角色蹲伏，需要在角色构造函数中启用移动能力：

```cpp
GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
```

若没有启用，开发构建日志会输出 `crouching is disabled on this character`，此时 Enhanced Input 即使已经正确触发，`bIsCrouched` 也不会变为 `true`。

### 15.2 多人同步职责

- 本地玩家通过 Enhanced Input 调用 `Crouch()` / `UnCrouch()`。
- 蹲伏属于 CharacterMovement 的预测与网络移动流程，不需要额外编写 Server RPC。
- `ACharacter::bIsCrouched` 已由引擎声明为 RepNotify 属性，模拟代理通过 `OnRep_IsCrouched()` 更新胶囊和表现。
- 动画实例只读取 `bIsCrouched`；不要再复制一个重复的动画布尔值。

### 15.3 动画状态机边界

如果动画图先按“是否装备武器”在两个状态机之间切换，而蹲伏状态只存在于 `Equipped` 状态机，则未装备时只会改变胶囊体，不会播放蹲伏动画。必须在当前实际输出的状态机中也配置蹲伏状态，或把通用蹲伏逻辑提到两个分支都能使用的动画层级。

**UE5.6 源码参考：**
- `Engine/Source/Runtime/Engine/Private/Character.cpp`：`ACharacter::Crouch`
- `Engine/Source/Runtime/Engine/Private/Components/CharacterMovementComponent.cpp`：`CanCrouchInCurrentState` 与蹲伏状态更新
- `Engine/Source/Runtime/Engine/Classes/GameFramework/Character.h`：`bIsCrouched`、`OnRep_IsCrouched`

﻿## 16. RPC系统篇

### 16.1 为什么要 RPC？

多人游戏中，每个 Client 的变量修改只对自己可见。要让 Server 和其他 Client 知道某件事发生了，需要主动通知。
UE 提供了两种互补的机制：
- **Replicated 变量（UPROPERTY(Replicated)）**：Server → 所有 Clients，单向自动同步。"是什么"
- **RPC（UFUNCTION(Server/Client/NetMulticast)）**：执行远程代码。"做什么"

### 16.2 RPC 的三种类型

| RPC 类型 | 声明 | 谁调用 | 谁执行 |
|---|---|---|---|
| Server RPC | `UFUNCTION(Server, Reliable)` | Client | Server |
| Client RPC | `UFUNCTION(Client, Reliable)` | Server | 该 Actor 的 Owning Client |
| NetMulticast RPC | `UFUNCTION(NetMulticast, Reliable)` | Server | 所有机器（Server + 全部 Client） |

### 16.3 `_Implementation` 后缀的由来

声明 RPC 函数后，UHT（Unreal Header Tool）在编译时会自动生成一个**同名分发函数**，负责网络打包和路由。
开发者需要写的实际逻辑放在 `函数名_Implementation` 中：

```cpp
// .h — 声明 RPC
UFUNCTION(Server, Reliable)
void ServerSetAiming(bool bIsAiming);

// .cpp — 实现 RPC 逻辑（注意 _Implementation 后缀）
void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
    bAiming = bIsAiming;
}
```

UHT 生成的 `ServerSetAiming` 函数负责：检查调用来源 → 如果是 Client 则打包网络消息 → 发送到 Server → Server 收到后调用 `_Implementation`。

### 16.4 Reliable vs Unreliable

- `Reliable`：保证 RPC 调用一定到达。适合装备、瞄准、射击等关键操作。
- `Unreliable`：不保证到达，但不会阻塞网络。适合高频但可丢失的数据（如位置更新）。

### 16.5 "双头模式"：本地立即执行 + Server RPC

瞄准代码展示了标准模式：
```cpp
void UCombatComponent::SetAiming(bool bIsAiming)
{
    bAiming = bIsAiming;            // (A) 本地立即生效
    ServerSetAiming(bIsAiming);     // (B) 通知 Server
}
```

(A) 让本地 Client 立刻获得响应（动画、UI 即时更新），
(B) 通过 Server RPC 让 Server 也更新 bAiming，然后 Replicated 自动同步给其他 Client。

### 16.6 HasAuthority() 分支模式

装备武器展示了另一种模式：
```cpp
void ABlasterCharacter::EKeyPressed()
{
    if (HasAuthority())          // 如果已经是 Server
    {
        Combat->EquipWeapon(OverlappingWeapon);
    }
    else                         // 如果是 Client
    {
        ServerEquipButtonPressed();
    }
}
```

区别：Equip 模式**不**先本地执行，而是由 Server 全权决定；Aim 模式本地先执行，Server 随后同步覆盖。

### 16.7 三种网络模式对比

| 功能 | 本地处理 | 通知 Server | 同步给他人 |
|---|---|---|---|
| Equip | `HasAuthority()` 判断，不先执行 | Server RPC | EquippedWeapon Replicated |
| Aim | `bAiming = true` 立即生效 | Server RPC | bAiming Replicated |
| Crouch | `Crouch()` → CharacterMovement | **不需要自己写 RPC** | bIsCrouched 内置 RepNotify |

**UE5.6 源码参考：**
- `Engine/Source/Runtime/Engine/Public/Net/UnrealNetwork.h`：`DOREPLIFETIME` 宏定义、`FRepLayout`
- `Engine/Source/Runtime/Engine/Classes/Engine/ActorChannel.h`：Actor 复制通道
- `Engine/Source/Runtime/CoreUObject/Public/UObject/CoreNet.h`：RPC 声明相关宏

### 16.8 端到端数据流（以瞄准为例）

1. Client A：右键按下 → `AimButtonPressed()` → `Combat->SetAiming(true)`
2. Client A：立刻设置本地 `bAiming = true` → 动画系统读到瞄准状态
3. Client A：调用 `ServerSetAiming(true)` → UHT 分发代码打包为网络消息
4. Server：收到消息 → `ServerSetAiming_Implementation(true)` → `bAiming = true`
5. Server：Replicated 检测到 `bAiming` 变化 → 推送给所有 Client
6. Client B：收到复制 → `bAiming = true` → 动画系统读到瞄准状态

以上就是 **RPC → Server 验证 → Replicated 同步** 的完整链路。

﻿### 16.9 如何广播动画（远程播放）

多人游戏中，一个 Client 的动画需要在其他所有机器上播放，有三种实现方式：

#### 方式 1：Replicated bool 驱动（类似 Aim 模式）

```cpp
UPROPERTY(Replicated)
bool bIsReloading;
```

在动画蓝图中读取这个 bool，切换对应的动画状态。Server 修改后自动同步到所有 Client。

**适用场景**：简单的状态切换动画（瞄准、换弹姿态）。
**优点**：和你现有的瞄准代码是同一个模式，实现简单。
**缺点**：只能控制状态切换，无法精确控制 Montage 播放时机。

#### 方式 2：NetMulticast RPC（广播函数调用）

```cpp
UFUNCTION(NetMulticast, Reliable)
void MulticastPlayReloadMontage();

void ABlasterCharacter::MulticastPlayReloadMontage_Implementation()
{
    PlayAnimMontage(ReloadMontage);  // 所有机器都播放
}
```

NetMulticast 是第三种 RPC：由 Server 调用，但**所有机器（Server + 全部 Client）都执行**。

**适用场景**：Montage 播放、粒子特效、声音等需要精确时机的效果。
**优点**：精确控制播放时机。
**缺点**：如果 Client 已经预测性地先播了一遍，Server 的 NetMulticast 又播一遍会重复。

#### 方式 3：RepNotify（复制到达时触发回调）

```cpp
UPROPERTY(ReplicatedUsing = OnRep_Reloading)
bool bIsReloading;

UFUNCTION()
void OnRep_Reloading();
```

变量复制到达 Client 时自动调用 `OnRep_` 函数执行额外逻辑。

**适用场景**：复制到达时需要执行额外逻辑（如修改碰撞、播放音效）。
**优点**：精确区分"数据到达"和"本地修改"。
**缺点**：Server 端不触发 OnRep，需要处理 Server 端的单独逻辑。

#### 三种方式选型对比

| 方式 | 你的参考代码 | 适用场景 |
|------|------------|---------|
| Replicated bool 驱动 | `bAiming` 模式 | 简单的状态切换动画 |
| NetMulticast RPC | 尚未写过 | Montage 播放、粒子特效、声音 |
| RepNotify | `OverlappingWeapon` 模式 | 复制到达时需要执行额外逻辑 |

后续攻击/射击系统通常会组合使用：**NetMulticast RPC + Montage** 实现射击动画，**Replicated 变量** 实现状态同步，**RepNotify** 实现到达时的特效触发。

---

## 更新记录

| 日期 | 更新内容 |
| --- | --- |
| 2026-07-13 | 初始化文档，覆盖全部已实现功能的技术要点 |
| 2026-07-15 | 增加 Character 蹲伏能力、移动复制与动画状态机排查经验 |
| 2026-07-15 | 增加 RPC 系统篇 16.9：动画广播的三种实现方式（Replicated bool / NetMulticast RPC / RepNotify）与选型对比 |
| 2026-07-15 | 增加 RPC 系统篇：三种 RPC 类型、_Implementation 后缀、Reliable vs Unreliable、双头模式、HasAuthority 分支模式、端到端数据流 |

## 17. UE5 Persona 动画编辑 - Lean 动画制作

### 17.1 背景：BlendSpace 需要 Lean 变体

在 BlendSpace 中混合跑步动画时，需要为同一运动制作不同方向的倾斜变体（如左倾/右倾），通常通过旋转 root bone 实现。

### 17.2 核心原理：编辑器中的"预览" vs "数据"

在 UE5 的 Persona（Animation Sequence 编辑器）中：
- **旋转 root bone 只是视口预览**，动画数据未被修改
- **必须 Add Key** 才能将当前骨骼变换写入动画轨道数据（关键帧）
- 不加 Key 直接保存，旋转不会被保留

### 17.3 "加点 Key 再删掉"的工作流 Trick

老师的流程：旋转 root bone → Add Key → Remove Key → 另存为新动画。

**为什么这样有效？**
1. **Add Key**：把 root bone 的旋转变换提交到动画轨道，动画序列内部标记为"已修改"
2. **Remove Key**：删除显式 keyframe，但变换已被编辑器记录为该动画中 root bone 的"基准状态"
3. 最终效果：root bone 有旋转偏移但动画中没有多余 keyframe，对 BlendSpace 更友好（避免意外插值）

### 17.4 你的替代方案（同样正确）

```
1. 保存原始跑步动画为 Run_LeanLeft
2. 打开 Run_LeanLeft，旋转 root bone
3. 在首帧（循环动画还需尾帧）Add Key
4. 保存
5. 对 Run_LeanRight 重复，旋转方向相反
```

**两套流程等价**：
| 方案 | Keyframe | 是否可用 |
|------|----------|---------|
| 加 Key 再删（老师 trick） | 无显式 keyframe | 是 |
| 保留 Key（标准流程） | 有显式 keyframe | 是 |

**给初学者的建议**：先用保留 keyframe 的标准流程，直观理解原理后，如需更"干净"的动画数据再用 trick。

---

## 更新记录

| 日期 | 更新内容 |
| --- | --- |
| 2026-07-13 | 初始化文档，覆盖全部已实现功能的技术要点 |
| 2026-07-15 | 增加 Character 蹲伏能力、移动复制与动画状态机排査经验 |
| 2026-07-15 | 增加 RPC 系统篇 16.9：动画广播的三种实现方式与选型对比 |
| 2026-07-15 | 增加 RPC 系统篇：三种 RPC 类型、_Implementation 后缀、Reliable vs Unreliable、双头模式、HasAuthority 分支模式、端到端数据流 |
| 2026-07-15 | 增加第 17 节：Persona Lean 动画制作原理与工作流对比 |
