# UE5 C++、反射、构建与 Gameplay Framework

## 1. 本节目标

学完后，你应该能解释：

- 为什么 `ABlasterCharacter` 是 `ACharacter`，战斗逻辑却放在 `UCombatComponent`；
- `GameMode`、`GameState`、`PlayerState` 和 `PlayerController` 为什么不能互相替代；
- `UCLASS`、`UPROPERTY`、`UFUNCTION`、`GENERATED_BODY` 和 `DOREPLIFETIME` 在项目中分别解决什么问题；
- C++ 文件如何通过 `Build.cs`、Target 和 UHT/UBT 进入 UE5.6 构建流程。

## 2. 先建立 UE 对象模型

UE C++ 不是普通 C++ 类直接被引擎调用。带 UE 反射宏的类型会被 UHT 扫描，生成反射和序列化相关代码；UBT 再根据模块依赖编译这些代码。

| 宏/类型 | 在 Blaster 中的例子 | 作用 |
| --- | --- | --- |
| `UCLASS()` | `ABlasterCharacter`、`AWeapon`、`UCombatComponent` | 让类型进入反射系统、Class Registry 和蓝图/属性系统 |
| `GENERATED_BODY()` | 每个 UE 类声明内部 | 接入 UHT 生成代码；不是普通 C++ 宏的装饰品 |
| `UPROPERTY()` | `Health`、`EquippedWeapon`、`CameraBoom` | 暴露编辑器/蓝图、参与 GC 追踪或网络元数据 |
| `UFUNCTION()` | `ReceiveDamage`、`OnRep_Eliminated`、RPC | 让函数可被反射、委托、蓝图或网络系统识别 |
| `UENUM(BlueprintType)` | `EWeaponState`、`EFireModel` | 让枚举进入反射和蓝图类型系统 |
| `TObjectPtr<T>` | `LagCompensation`、`WeaponMesh` | UE5 风格的 UObject 引用，配合 `UPROPERTY` 让生命周期更明确 |

一个重要区别：`UPROPERTY` 不等于“变量自动复制”。网络复制仍要在 `GetLifetimeReplicatedProps` 中用 `DOREPLIFETIME` 注册；只有声明 `Replicated` 或 `ReplicatedUsing` 还不够。

## 3. 项目中的职责分层

```text
ABlasterCharacter
  ├── 输入、移动、相机、角色生命周期
  ├── Health / Eliminated 等当前 Pawn 状态
  └── 持有 UCombatComponent

UCombatComponent
  ├── 装备、开火、换弹、瞄准、双武器
  ├── RPC 和复制的战斗状态
  └── 不把所有战斗逻辑塞进 Character

AWeapon / ABlasterProjectile / ABlasterPickup
  └── 各自负责武器、飞行物和补给 Actor 的状态与碰撞

ABlasterGameMode
  └── 仅 Server：比赛规则、生成、重生、计分
ABlasterGameState
  └── 全局比赛状态：阶段截止时间、公告
ABlasterPlayerState
  └── 跨 Pawn 重生保留的玩家统计：Score、Defeats
ABlasterPlayerController
  └── 单个玩家的控制通道，不作为全局玩家状态容器
```

对应源码：

- `Source/Blaster/Character/BlasterCharacter.h/.cpp`
- `Source/Blaster/BlasterComponent/CombatComponent.h/.cpp`
- `Source/Blaster/Weapon/Weapon.h/.cpp`
- `Source/Blaster/Character/BlasterGameMode.h/.cpp`
- `Source/Blaster/Character/BlasterGameState.h/.cpp`
- `Source/Blaster/Character/BlasterPlayerState.h/.cpp`

### GameMode 与 GameState 的面试说法

`GameMode` 只存在于服务器，适合做“规则裁判”：谁能生成、什么时候开始/结束、击杀如何计分、何时重生。客户端拿不到 GameMode 的权威实例，所以需要把需要展示给所有客户端的结果写进 `GameState` 或 `PlayerState`。

本项目中，`ABlasterGameMode::Tick` 驱动 WaitingToStart、InProgress 和 WaitingPostMatch；`ABlasterGameState::PhaseDeadline` 复制给客户端，HUD 用同步后的服务器时间显示倒计时。

`PlayerState` 比 Pawn 更适合保存分数和死亡数，因为淘汰时 Pawn 会被销毁并由新 Pawn 替换，而同一个玩家的 PlayerState 可以继续存在。

## 4. UE 对象生命周期在本项目中的落点

以 `ABlasterCharacter` 为例：

1. 构造函数中用 `CreateDefaultSubobject` 创建 Camera Boom、Camera、Widget、Combat 和 Lag Compensation Component。
2. `PostInitializeComponents` 把 `Combat->Character` 绑定到当前角色；此时组件已经创建，但 Gameplay World 运行逻辑尚未完全开始。
3. `BeginPlay` 注册伤害委托、Server 生成默认武器、添加本地 Enhanced Input Mapping Context。
4. `SetupPlayerInputComponent` 在 Pawn 被控制后绑定 Input Action 回调。
5. `Tick` 只处理需要逐帧同步的角色朝向策略；动画参数在 `UBlasterAnimInstance::NativeUpdateAnimation` 中读取。
6. `EndPlay` 清理角色计时器；`UCombatComponent::EndPlay` 清理开火和换弹计时器，并在销毁时处理武器掉落。

这解释了两个常见坑：

- 不能在构造函数中依赖运行时的 Controller、GameState 或世界中的其他 Actor；
- Timer、Delegate 和动态生成 Actor 必须考虑 `EndPlay`，否则 Pawn 销毁后可能继续回调悬空状态。

## 5. GC 与引用写法

项目中这些是需要关注的 UObject 引用：

```cpp
UPROPERTY(VisibleAnywhere)
TObjectPtr<ULagCompensationComponent> LagCompensation;

UPROPERTY(Replicated)
AWeapon* EquippedWeapon;
```

第一种是组件引用，必须让 UE 认识它；第二种虽然不是 `TObjectPtr`，仍通过 `UPROPERTY` 参与反射/复制。面试时不要说“所有指针都必须是 `TObjectPtr`”，准确说法是：被 UE 管理的 UObject 引用要放进合适的 `UPROPERTY`，具体类型受项目和 UE5.6 API 约定影响；临时观察指针或非 UObject 数据不应机械加上 `UPROPERTY`。

## 6. Build.cs、Target 与 UHT/UBT

### 模块依赖

`Source/Blaster/Blaster.Build.cs` 把 `Core`、`CoreUObject`、`Engine`、`InputCore` 和 `EnhancedInput` 作为公共依赖，把 `Slate` 和 `SlateCore` 作为私有依赖。代码一旦 include 了某个模块的公开类型，依赖关系要在 Build.cs 中明确，而不是依赖“编辑器碰巧能编译”。

### Target

`Source/Blaster.Target.cs` 是游戏目标，`BlasterEditor.Target.cs` 是编辑器目标。两者都使用 `BuildSettingsVersion.V5` 和 `EngineIncludeOrderVersion.Unreal5_6`，并加入 `Blaster` 模块。

### 构建链

```text
头文件中的 UE 宏
       |
       v
UHT 生成反射/网络所需代码
       |
       v
UBT 按 Target + Build.cs 组织模块
       |
       v
BlasterEditor 或 Blaster Game 二进制
```

`GENERATED_BODY` 报错、找不到反射类型、模块符号未解析和链接错误通常不是同一类问题。排查时先找日志中的第一条真实错误，再分别判断是 UHT、UBT、C++ 编译、链接还是 Editor/Live Coding DLL 占用。

## 7. UE5.6 迁移要点

- 输入优先使用 Enhanced Input 的 `UInputAction`、`UInputMappingContext`、`UEnhancedInputComponent`，不要默认照搬旧的 Action/Axis 绑定教程。
- 头文件尽量前向声明，具体类型放到 `.cpp` include，减少编译耦合。
- 使用 `nullptr`、UE 容器和 UE 数学类型；不要在 UE Gameplay 代码中用 `std::cout` 代替 `UE_LOG`。
- `bCanCrouch` 需要在 `CharacterMovementComponent` 的 NavAgentProperties 中启用；基础 `bIsCrouched` 同步优先复用 CharacterMovement 内置流程。
- RPC 的声明写在头文件，UHT 生成 `_Implementation` 入口；不要手写重复声明。
- 课程代码只是动机参考，UE5.6 的头文件、实际编译和运行证据才是最终裁决。

## 8. 引擎源码阅读路线

本机引擎路径：`E:/programs/Epic Games/UE_5.6/Engine/Source/Runtime/`。

- UObject/反射：`CoreUObject/Public/UObject/Object.h`、`Class.h`、`UObjectGlobals.h`
- Actor 生命周期：`Engine/Public/GameFramework/Actor.h`、`Engine/Private/Actor.cpp`
- Component：`Engine/Public/Components/ActorComponent.h`
- Character/移动：`Engine/Public/GameFramework/Character.h`、`CharacterMovementComponent.h`
- GameMode/GameState：`Engine/Public/GameFramework/GameModeBase.h`、`GameStateBase.h`
- 网络注册：`Engine/Public/Net/UnrealNetwork.h`
- Enhanced Input：`EnhancedInput/Public/EnhancedInputComponent.h`、`InputTriggers.h`

先在项目里找到调用点，再去引擎源码找生命周期或数据结构；不要一开始通读整个引擎。

## 9. 如何验证自己学懂了

### 源码验证

从项目根目录执行：

```powershell
rg -n "UCLASS|UPROPERTY|UFUNCTION|GENERATED_BODY" Source/Blaster
rg -n "GetLifetimeReplicatedProps|DOREPLIFETIME" Source/Blaster
rg -n "CreateDefaultSubobject|PostInitializeComponents|BeginPlay|EndPlay" Source/Blaster
```

然后任选 `Health`、`EquippedWeapon` 和 `PhaseDeadline`，说明它们的声明位置、写入者、复制注册位置和消费者。

### 小练习

1. 如果把 `Defeats` 放进 `ABlasterCharacter` 而不是 `ABlasterPlayerState`，Pawn 重生时会发生什么？
2. `UCombatComponent` 为什么是 Component 而不是第二个 Character？说出至少两个好处。
3. 新增一个 `UObject` 指针时，如何判断是否需要 `UPROPERTY`？
4. 给 `Blaster.Build.cs` 添加一个模块依赖前，你会从哪里确认模块名？

## 10. 面试快答

**问：GameMode 和 GameState 的区别？**

答：GameMode 只在服务器存在，执行规则和裁决；GameState 会复制给客户端，承载比赛阶段、时间和全局状态。Blaster 的重生/计分由 GameMode 做，倒计时和公告由 GameState 同步给 HUD。

**问：`UPROPERTY` 的核心价值是什么？**

答：它让 UObject 成员进入 UE 的反射体系，并可参与编辑器、蓝图、序列化、GC 或网络元数据。它本身不自动完成网络复制，复制还要声明 `Replicated` 并在 `GetLifetimeReplicatedProps` 注册。

**问：为什么不把所有逻辑写进 Character？**

答：Character 负责 Pawn 生命周期、移动和输入入口；战斗状态放 Component，武器和飞行物是独立 Actor，比赛规则放 GameMode，跨 Pawn 的玩家统计放 PlayerState。这样职责清晰，也更容易处理网络、重生和复用。
