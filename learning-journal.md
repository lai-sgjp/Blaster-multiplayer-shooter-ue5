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

## 更新记录

| 日期 | 更新内容 |
| --- | --- |
| 2026-07-13 | 初始化文档，覆盖全部已实现功能的技术要点 |