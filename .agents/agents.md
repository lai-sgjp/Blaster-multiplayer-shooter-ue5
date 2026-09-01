# Blaster Project - Agents Context

## 项目基本信息

- 项目类型：Unreal Engine 5 C++ 多人射击游戏
- 引擎版本：UE 5.6
- IDE：Visual Studio 2026
- 主要语言：C++
- 蓝图使用原则：蓝图主要用于配置、动画、UI 表现和派生类，不把核心游戏逻辑写在蓝图中
- 学习来源：Udemy Unreal Engine 5 C++ Multiplayer Shooter 类课程
- 当前目标：在理解课程内容的基础上，用 UE5.6 兼容写法完成多人射击游戏功能

## 项目概述

Blaster 是一款基于 Unreal Engine 5.6 的多人第三人称射击游戏（TPS），采用 Steam Sockets 作为网络传输层，Steam Online Subsystem 提供大厅与会话管理。项目当前处于核心框架构建阶段。

| 属性 | 值 |
| --- | --- |
| 引擎版本 | UE 5.6 |
| 构建工具版本 | BuildSettingsVersion.V5 |
| IncludeOrderVersion | Unreal5_6 |
| C++ 标准 | C++17 |
| 主模块 | Blaster (Runtime, Default 加载) |
| 目标平台 | Windows (DX12 SM6)、Linux (Vulkan SM6) |
| 渲染特性 | Lumen GI + Lumen Reflection、Ray Tracing、Virtual Shadow Maps |
| 网络方案 | SteamSockets (Steam P2P) + Steam Online Subsystem |
| 输入系统 | Enhanced Input System |

## 目录结构

`
Blaster/
├── Config/
│   ├── DefaultEngine.ini          # 引擎设置、网络驱动（SteamSockets）、渲染（Lumen/RTX）、VRM
│   ├── DefaultGame.ini            # 游戏设置、关卡打包规则（MapStartUp/Lobby/TransitionLevel/BlasterMap）
│   ├── DefaultInput.ini           # 输入映射（Enhanced Input 默认）
│   └── DefaultEditor.ini          # 编辑器预览配置
├── Content/
│   ├── Assets/                    # 美术资源包（AnimStarterPack, FPS_Weapon_Bundle, LearningKit, Mixamo, ThirdPerson 等）
│   ├── Blueprints/                # 蓝图（Character, GameModes, HUD, Weapon）
│   ├── Characters/                # 角色模型（Burnice, Mannequins, vva）
│   ├── Input/                     # 输入配置（Actions/, IMC_Default, IMC_MouseLook）
│   └── Maps/                      # 关卡（MapStartUp, Lobby, TransitionLevel, BlasterMap）
├── Plugins/
│   ├── MultiplayerSessions/       # 自定义会话管理插件
│   └── VRM4U/                     # VRM 模型导入插件
└── Source/
    ├── Blaster.Target.cs           # Game 目标（V5, Unreal5_6）
    ├── BlasterEditor.Target.cs     # Editor 目标（V5, Unreal5_6）
    └── Blaster/
        ├── Blaster.h / .cpp        # 主模块入口，IMPLEMENT_PRIMARY_GAME_MODULE
        ├── Blaster.Build.cs        # 依赖：Core, CoreUObject, Engine, InputCore, EnhancedInput, Slate, SlateCore
        ├── BlasterComponent/
        │   └── CombatComponent     # 战斗逻辑组件（武器装备）
        ├── Character/
        │   ├── BlasterCharacter    # 玩家角色（弹簧臂相机、Enhanced Input、头顶 Widget、网络复制）
        │   ├── BlasterAnimInstance # 动画蓝图实例（Speed, bIsInAir, bIsAccelerating）
        │   ├── BlasterGameMode     # 主游戏模式（默认 Pawn / Controller）
        │   └── BlasterPlayerController # 玩家控制器
        ├── GameModes/
        │   └── LobbyGameMode       # 大厅模式（2 人满后 SeamlessTravel）
        ├── HUD/
        │   └── OverheadWidget      # 头顶控件（网络角色 + 玩家名）
        └── Weapon/
            └── Weapon              # 武器（拾取系统、网络复制、状态枚举）
`

## C++ 代码架构

### 模块依赖（Blaster.Build.cs）

- Core / CoreUObject / Engine --- UE 基础框架
- InputCore / EnhancedInput --- Enhanced Input 输入系统
- Slate / SlateCore --- UI 框架
- UMG --- 用户控件（.uproject 附加依赖）

### 核心类职责

#### ABlasterCharacter
- **位置:** Source/Blaster/Character/BlasterCharacter.h/.cpp
- **继承:** ACharacter
- **职责:** 第三人称玩家角色控制器
- **关键组件:** SpringArmComponent, CameraComponent, UCombatComponent, UWidgetComponent（头顶）
- **输入绑定 (Enhanced Input):** MoveAction, LookAction, JumpAction, EquipAction, AttackAction(待实现), DodgeAction(待实现)
- **网络复制:** OverlappingWeapon (COND_OwnerOnly), ServerEquipButtonPressed (RPC)
- **移动设置:** bOrientRotationToMovement=true, MaxStepHeight=60, WalkableFloorAngle=46

#### UCombatComponent
- **位置:** Source/Blaster/BlasterComponent/CombatComponent.h/.cpp
- **继承:** UActorComponent
- **职责:** 战斗核心逻辑，当前实现武器装备流程
- **关键函数:** EquipWeapon(AWeapon*) --- 武器附着到 RightHandSocket
- **网络:** 通过 Character 触发 Server RPC

#### AWeapon
- **位置:** Source/Blaster/Weapon/Weapon.h/.cpp
- **继承:** AActor
- **职责:** 可拾取武器，支持网络复制
- **状态枚举:** EWeaponState (EWS_Initial, EWS_Equipped, EWS_Dropped)
- **碰撞系统:** WeaponMesh(Root, NoCollision) + AreaSphere(Server 端 Pawn Overlap)
- **拾取流程:** Server 检测 Overlap -> Character.SetOverlappingWeapon -> 客户端 OnRep 显示 Widget -> E 装备
- **网络:** WeaponState Replicated, OnRep_WeaponState 控制 UI 和碰撞

#### ABlasterGameMode
- **位置:** Source/Blaster/Character/BlasterGameMode.h/.cpp
- **继承:** AGameMode
- **职责:** 默认 Pawn=ABlasterCharacter, Controller=ABlasterPlayerController

#### ALobbyGameMode
- **位置:** Source/Blaster/GameModes/LobbyGameMode.h/.cpp
- **继承:** AGameMode
- **职责:** PostLogin 检测玩家数，2 人满后 SeamlessTravel 到 BlasterMap

#### UBlasterAnimInstance
- **位置:** Source/Blaster/Character/BlasterAnimInstance.h/.cpp
- **继承:** UAnimInstance
- **职责:** 每帧更新运动参数 Speed(水平)、bIsInAir、bIsAccelerating

#### UOverheadWidget
- **位置:** Source/Blaster/HUD/OverheadWidget.h/.cpp
- **继承:** UUserWidget
- **职责:** 显示网络角色类型和玩家名

#### ABlasterPlayerController
- **位置:** Source/Blaster/Character/BlasterPlayerController.h/.cpp
- **继承:** APlayerController
- **职责:** 玩家控制器，预留 OnPossess 接口

### MultiplayerSessions 插件

- **UMultiplayerSessionsSubsystem** (UGameInstanceSubsystem): 封装 IOnlineSessionPtr 提供会话 CRUD，暴露多播委托供 Menu 绑定
- **UMenu** (UUserWidget): Host/Join 按钮，MenuSetup(PublicConnections, MatchType, LobbyPath)

## 配置要点

### 网络
- NetDriver: SteamSocketsNetDriver（DefaultEngine.ini 配置）
- OnlineSubsystem: Steam（AppId 480，开发测试用）
- 最大玩家数: 50

### 渲染
- 动态全局光照: Lumen (r.DynamicGlobalIlluminationMethod=1)
- 反射方法: Lumen (r.ReflectionMethod=1)
- 光线追踪: 启用 (r.RayTracing=True)
- 虚拟阴影贴图: 启用 (r.Shadow.Virtual.Enable=1)
- 默认 RHI: DX12 | Shader Model: SM6
- 皮肤缓存: 启用

### 关卡流程
1. MapStartUp --- 启动地图（含 PCG 程序化生成）
2. Lobby --- 编辑器默认地图，等待玩家加入
3. TransitionLevel --- 无缝旅行过渡地图
4. BlasterMap --- 主游戏地图

## 当前开发状态

### 已完成
- [x] 角色移动 + 视角控制（Enhanced Input）
- [x] 武器拾取 + 装备（含网络复制、Server RPC）
- [x] 多人大厅 + 会话管理（Steam + MultiplayerSessions）
- [x] Lobby 到 BlasterMap 关卡切换（SeamlessTravel）
- [x] 头顶信息显示（网络角色、玩家名）
- [x] 动画蓝图基础（速度、空中、加速状态）

### 待实现
- [ ] 攻击/射击系统
- [ ] 生命值与伤害系统
- [ ] 闪避/翻滚动作
- [ ] 掉落武器
- [ ] HUD（准星、血量、弹药）
- [ ] 游戏规则逻辑（计分、重生）
- [ ] VRM 角色绑定
- [ ] PCG 关卡打磨

## AI 行为规则

## 项目推进流程（强制）

本项目采用任务卡驱动的项目式学习流程。实施任何新功能前，先阅读并遵守以下文档：

- [Blaster 推进工作流](BLASTER_WORKFLOW.md)
- [任务卡模板](TASK_TEMPLATE.md)
- [审查模板](REVIEW_TEMPLATE.md)
- [四周学习计划](BLASTER_ACCELERATED_LEARNING_PLAN.md)

除非用户明确要求一次规划多个任务，每次只推进一个 `TASK-Bxx-yyy` 垂直切片。开始实现前必须完成 Gate 0：冻结目标、所有权/网络数据流、文件白名单、编辑器操作、验收证据与非目标。课程原始实现基于 UE5.0；每张任务卡均须记录 UE5.6 迁移检查，不能直接照搬旧 API。

任务完成的最低证据为 Development Editor Build、Listen Server + 1 Client PIE 主路径，以及一个失败路径。完成后先独立审查；只有结论为 `PASS` 或 `PASS WITH FOLLOW-UP` 才能归档，并且只推荐一个下一任务。

### 通用规则
1. 回答必须优先考虑 UE5.6 和 C++
2. 不要默认使用蓝图实现核心逻辑，除非用户明确要求
3. 如果课程代码和 UE5.6 API 存在差异，需要指出差异并给出兼容写法
4. 生成代码前，先说明要修改哪些文件
5. 修改代码时，尽量保持改动小而明确
6. 不要一次性重构大量文件，除非用户明确要求

### UE 反射系统检查清单
涉及 UE 反射系统时，必须检查：
- UCLASS / USTRUCT / UENUM
- GENERATED_BODY
- UPROPERTY
- UFUNCTION

### 网络同步检查清单
涉及网络同步时，必须说明：
- 哪些逻辑运行在 Server
- 哪些逻辑运行在 Client
- 哪些变量需要 Replicated
- 哪些函数需要 Server RPC / Client RPC / NetMulticast
- 是否需要 OnRep

### GC 与性能
- 涉及 UObject / Actor / Component 引用时，必须检查 GC 风险，必要时使用 UPROPERTY
- 尽量避免 Tick。如果使用 Tick，必须说明原因

### 危险操作确认
执行危险操作前必须征求用户确认，包括：
- 删除文件、覆盖文件
- git reset、git clean
- 批量移动文件
- 修改大量课程代码

### 其他
- 如果要运行命令，先说明命令的目的
- 如果无法确定 API 是否存在，不要编造，应该建议查 UE5.6 文档或让用户提供编译报错

## 代码风格要求

1. 类名遵守 UE 命名规范：
   - A 开头：Actor
   - U 开头：UObject / Component
   - F 开头：Struct
   - E 开头：Enum
   - I 开头：Interface
2. 头文件尽量使用前向声明，cpp 中 include 具体头文件
3. 使用 nullptr，不使用 NULL
4. 使用 UE 类型：FString, FName, FText, TArray, TMap, TObjectPtr
5. 对蓝图可配置参数优先使用：EditDefaultsOnly, EditAnywhere, BlueprintReadOnly, BlueprintReadWrite
6. 对运行时内部变量避免过度暴露给蓝图
7. 日志使用 UE_LOG，不使用 std::cout
8. 不要把所有逻辑写进 Character，优先考虑 ActorComponent / PlayerController / PlayerState / GameState / Subsystem 等职责划分

## UE Multiplayer 特别规则

1. 客户端输入可以在本地响应，但权威游戏状态应由 Server 决定
2. 伤害、击杀、分数、拾取、武器发射结果等关键逻辑应由 Server 验证
3. UI 只在本地客户端更新，不要在 Server 直接操作客户端 Widget
4. PlayerController 只属于对应客户端，不适合复制给所有人显示的状态
5. PlayerState 适合保存玩家名字、分数、击杀、死亡等需要所有客户端知道的数据
6. GameState 适合保存比赛时间、队伍分数、全局状态
7. GameMode 只存在于 Server
8. Replicated 变量需要在 GetLifetimeReplicatedProps 中注册
9. RepNotify 函数命名通常使用 OnRep_VariableName
10. RPC 函数需要遵守 UE 命名和声明规则

## 默认回答结构

### 实现功能请求
当用户请求实现功能时，按以下结构回答：
1. 功能目标确认
2. 推荐放在哪些 UE 类中
3. 文件修改清单
4. 最小实现步骤
5. 关键代码
6. UE 编辑器/VS 中需要做的操作
7. 编译与测试步骤
8. 常见错误与排查方法

### Debug 请求
当用户请求 Debug 时，按以下结构回答：
1. 错误类型判断
2. 关键报错定位
3. 最可能原因
4. 修复方案
5. 验证步骤
6. 如果仍失败，需要用户补充的信息

## Skills 索引

以下 skills 放在 \C:\Users\Lai\.codex\skills\ 下，Codex 会根据对话内容自动匹配并加载对应 skill。

### UE5 C++ Engineer
**位置**: \C:\Users\Lai\.codex\skills\ue5-cpp-engineer\SKILL.md\
**适合任务**:
- 创建/修改 UE C++ 类（Actor, Character, Component, GameMode, PlayerController 等）
- 实现战斗、武器、生命、拾取、交互、HUD 相关的 C++ 逻辑
- 将课程代码迁移到 UE5.6
- 学习 UObject 反射、AActor 生命周期、网络复制等引擎源码
**面试价值**: UObject 生命周期、网络属性复制、GC 机制源码级理解

### UE5 Multiplayer Debugger
**位置**: \C:\Users\Lai\.codex\skills\ue5-multiplayer-debugger\SKILL.md\
**适合任务**:
- 客户端看不见服务器同步结果 / RPC 不执行 / OnRep 不触发
- 变量没有复制 / 客户端可以作弊
- 武器开火不同步 / 伤害只在本地生效
- 分数、血量、弹药不同步
**调试流程**: 14 步检查链（bReplicates → GetLifetimeReplicatedProps → RPC 声明 → OnRep → NetMulticast）

### UE5 Build Error Analyst
**位置**: \C:\Users\Lai\.codex\skills\ue5-build-error-analyst\SKILL.md\
**适合任务**:
- 编译错误 / UHT 错误 / UBT 错误 / LNK2019 / LNK2001
- GENERATED_BODY 报错 / 找不到类型 / 模块依赖错误
- Live Coding 失败 / UE Editor 打不开 / .sln 生成失败
**核心方法**: 找到第一处真正错误 → 判断错误类型 → 按优先级链排查

### UE5 Course Tutor
**位置**: \C:\Users\Lai\.codex\skills\ue5-course-tutor\SKILL.md\
**适合任务**:
- Udemy UE5 C++ Multiplayer Shooter 课程概念理解
- 课程代码逐段解释 + 旧版本迁移到 UE5.6
- 课后复盘：知识点总结 + 代码复盘 + 常见坑 + 下一节预习
- 每个课程主题映射到引擎源码路径
**教学原则**: 先概念 → 再代码 → 再 UE 框架位置 → 再源码映射

### UE5 Code Reviewer
**位置**: \C:\Users\Lai\.codex\skills\ue5-code-reviewer\SKILL.md\
**适合任务**:
- 提交前代码审查：UE 反射正确性 / GC 安全 / 空指针风险
- 网络同步完整性 / 防作弊检查 / 职责划分
- 性能 / UE5.6 兼容性 / 可维护性 / 日志调试
**10 维评分**: 正确性 / UE 规范 / 多人同步 / 可维护性 / 源码理解深度

### UE5 Replication Expert
**位置**: \C:\Users\Lai\.codex\skills\ue5-replication-expert\SKILL.md\
**适合任务**:
- 武器开火同步 / 弹药同步 / 血量同步 / 伤害判定
- 分数同步 / Match State / 角色状态（瞄准、换弹）
- Montage 播放同步 / Projectile / HitScan
- Lag Compensation / Server Rewind
- Online Session 管理
**分析框架**: 10 问分析模板（何人拥有 → bReplicates → 数据源 → RPC → Server 验证 → Replicated → OnRep → 延迟体验 → 作弊风险）

## 学习实践 Tips

### 1. 新增 Skill 时的操作流程
1. 在 \C:\Users\Lai\.codex\skills\ 下用 \init_skill.py\ 创建
2. 编辑 \SKILL.md\ 填入内容
3. 在本索引末尾加上对应条目标记
4. 运行 \quick_validate.py\ 验证

### 2. Skill 索引维护
如果你新增了 skill 但忘记更新索引，可以对 Codex 说：
> 扫描我的 skills 目录，读取所有 SKILL.md 的标题和描述，然后更新 agents.md 中的 Skills 索引

### 3. 源码学习记录习惯
学习完一个引擎源码主题（如 UObject 生命周期）后：
- 在本索引下方或新建 \.agents/source-code-notes.md\ 记录
- 格式：日期 + 源码路径 + 关键理解 + 和 Blaster 项目的关联

### 4. 代码提交前自检
提交前快速扫一遍 Code Reviewer 的 10 维检查清单，或在 Codex 中说：
> 帮我 review 当前改动

### 5. 面试准备复习
面试前，通读本索引中标注了"面试价值"的条目，然后让 Codex 模拟面试：
> 针对 UObject 生命周期准备 3 道面试题，先问再给我答案


### 6. 学习知识库

每次完成新功能或解决重要 Bug 后，在项目根目录的 [learning-journal.md](learning-journal.md) 中追加记录。该文档按技术主题分类，记录知识点、代码示例和工程经验，用于复习和面试准备。
在 Codex 中可以直接问：学习知识库中关于 [主题] 的内容。

### 7. 项目工作日志

在实施任务、排障或复盘时，按 [Blaster 推进工作流](BLASTER_WORKFLOW.md) 的归档要求在项目根目录的 [worklog.md](worklog.md) 中追加项目记忆，不得覆盖或改写已有历史。用户明确要求只读、仅规划或限定文件范围时，必须遵守其范围，不应因本规则额外修改日志。

每轮记录至少包含：
- 日期与主题
- 用户目标和现象
- 检查步骤与关键证据
- 原因分析和技术决策
- 实际修改的文件与内容
- 编译、测试或尚未验证的事项
- UE 编辑器中的操作和后续待办

 `worklog.md` 用于记录协作过程与工程决策；`learning-journal.md` 继续用于沉淀可复习的技术知识。即使本轮只进行了分析、答疑或方案研讨而没有修改代码，也必须记录结论。

### 8. 代码复盘与概念教学

当用户已完成某几小节的编码（代码已 `git add` 但未 `commit`），请求帮助理解代码中的概念时：

 1. 读取当前 git staged diff，完整理解用户实际写了哪些代码变更
 2. **禁止修改任何游戏代码**，禁止撤销 staged 内容，禁止提交
 3. 如果该教学场景有对应的已有 skill（如 `ue5-course-tutor`），优先参考其教学结构
 4. 按"概念本质 → 代码对照 → 端到端数据流"的路径解释困惑点
 5. 提供横向对比：同一问题的不同实现模式（如 Equip vs Aim vs Crouch 三种网络模式对比）
 6. 给出练习题帮助用户验证理解
 7. 更新 `worklog.md` 记录本次复盘过程的结论，更新 `learning-journal.md` 沉淀可复习的技术知识
 8. 如果 `agents.md` 中没有该场景的处理指令，在此追加
