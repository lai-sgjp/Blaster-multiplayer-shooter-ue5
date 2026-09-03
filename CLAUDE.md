# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概览

Blaster 是基于 **UE 5.6** 的 C++ 多人第三人称射击游戏（TPS），由 Udemy《Unreal Engine 5 C++ Multiplayer Shooter》课程驱动，但**不照搬 UE5.0 课程代码**——所有实现须在 UE5.6 下迁移验证。这是一个学习/作品集项目，不是纯工程项目。

技术要点：
- **输入**：Enhanced Input（`UInputAction` + `IMC`，`UEnhancedInputLocalPlayerSubsystem` 管理）
- **网络**：SteamSockets NetDriver + Steam OnlineSubsystem（AppId 480 测试用），最大玩家 50
- **渲染**：Lumen GI/Reflection、Ray Tracing、Virtual Shadow Maps、DX12/SM6
- **文档语言**：全部为中文（`.agents/` 工作流文档、代码注释、worklog）

引擎安装在 `E:\programs\Epic Games\UE_5.6`。

## 强制工作流（实施任何功能前先读）

本项目由任务卡驱动的项目式学习流程约束。**开始写代码前必须阅读**：

| 文件 | 作用 |
| --- | --- |
| `.agents/agents.md` | AI 行为规则：代码风格、UE 反射/网络检查清单、默认回答结构、危险操作确认 |
| `.agents/BLASTER_WORKFLOW.md` | 强制推进流程：Gate 0 冻结 → 最小实现 → UE5.6 迁移门 → 验证阶梯 → 独立审查 → 归档 |
| `.agents/BLASTER_ACCELERATED_LEARNING_PLAN.md` | B00-B12 工作包路线图与四周计划 |
| `.agents/TASK_TEMPLATE.md` / `.agents/REVIEW_TEMPLATE.md` | 任务卡与审查模板 |
| `.agents/Blaster_Context.md` | 学习者背景与项目约定（类职责边界、偏好） |

核心规则（详见上述文档）：
- 每次只推进一个 `TASK-Bxx-yyy` 垂直切片，实施前必须完成 Gate 0（冻结目标、网络数据流、文件白名单、Editor 操作、验收证据、非目标）。
- 核心权威逻辑保持在 C++；蓝图只用于派生类、Mesh/动画/蒙太奇/资源绑定、UMG 布局和简单表现。
- 网络权威在 Server：伤害、命中、弹药、拾取、分数、比赛状态由 Server 决定；客户端只做即时表现。
- 不要编造 UE5.6 API；不确定时查引擎头文件、真实编译错误或让用户提供报错。
- 现有未提交改动属于用户，除非明确授权，不得撤销、混入或提交；提交前只暂存任务白名单内文件，且必须有用户明确授权。

## 构建、运行与验证

该项目没有单元测试；"跑测试"即编译 + PIE 多人验证。

**编译（Development Editor 增量构建）：**
```bash
"E:\programs\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" BlasterEditor Win64 Development -Project="E:\work\unreal_projects\Blaster\Blaster.uproject" -WaitMutex
```
低内存机器上若 UBA 卡住（action 长时间不执行），追加 `-NoUBA -MaxParallelActions=1`（见 `.agents/worklog.md` 2026-07-15 记录）。

**启动编辑器：**
```bash
"E:\programs\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe" "E:\work\unreal_projects\Blaster\Blaster.uproject"
```
运行中的 `UnrealEditor.exe` / `LiveCodingConsole.exe` 会锁住 `Binaries/Win64/UnrealEditor-Blaster.dll` 导致链接失败（LNK1104）；改 C++ 后需关闭编辑器再完整构建。

**验证阶梯**（按顺序留存真实证据，`Saved/Logs/Blaster.log` 可查运行日志）：
1. Development Editor Build 通过
2. 单机冒烟：加载地图无阻塞
3. 多人主路径：**Listen Server + 1 Client PIE**，分别记录服务端、拥有客户端、模拟客户端观察
4. 失败路径：至少一次非法/边界输入或网络时序（无武器开火、换弹中开火、重复 Pickup、晚加入、Pawn 销毁后 UI 更新）
5. 白名单外回归

编辑器默认地图为 `Lobby`；关卡流程 `MapStartUp → Lobby → TransitionLevel → BlasterMap`（SeamlessTravel 由 `ALobbyGameMode` 在 2 人满后触发）。

## 代码架构

### 模块与目录

唯一模块 `Blaster`（Runtime, Default），位于 `Source/Blaster/`。`Blaster.Build.cs` 公共依赖：Core, CoreUObject, Engine, InputCore, EnhancedInput；私有依赖 Slate, SlateCore；UMG 通过 `.uproject` 的 `AdditionalDependencies` 引入。新增模块依赖时须同步更新 Build.cs 与 .uproject。

```
Source/Blaster/
├── Character/        # ABlasterCharacter（角色+输入）、ABlasterPlayerController、ABlasterGameMode、UBlasterAnimInstance
├── BlasterComponent/ # UCombatComponent（战斗逻辑）
├── Weapon/           # AWeapon（拾取、网络复制、EWeaponState 状态机）
├── GameModes/        # ALobbyGameMode（大厅、SeamlessTravel）
└── HUD/              # UOverheadWidget（头顶玩家名）
```

### 类职责约定（学习者既定架构，勿随意破坏）

- **Character**：移动、Enhanced Input 绑定、基础表现
- **CombatComponent**：战斗状态、开火、换弹、瞄准
- **Weapon**：武器表现与开火行为
- **PlayerController**：HUD 与本地输入相关逻辑（只属于对应客户端，不适合复制给所有人）
- **PlayerState**：分数、击杀、死亡等所有客户端需知的玩家状态
- **GameMode**：比赛规则（只存在于 Server）
- **GameState**：比赛全局状态同步（时间、队伍分数）
- **HUD / Widget**：只在本地客户端创建和更新，Server 不直接操作客户端 Widget

### 网络同步的三种既有模式（实现新功能时对照选择）

1. **权威分支模式**（如 Equip）：客户端发 `Server, Reliable` RPC，Server 上 `HasAuthority()` 直接改状态，靠复制下推。
2. **双头模式**（如 Aim）：本地立即改复制变量做即时表现，同时发 Server RPC；Server 验证后复制回所有端，`OnRep` 刷新。
3. **引擎内置模式**（如 Crouch）：优先用引擎自带的复制流程（`ACharacter::Crouch()` + `bIsCrouched` + `OnRep_IsCrouched`），不重复实现 Replicated 变量。

Replicated 变量必须在 `GetLifetimeReplicatedProps` 注册；RepNotify 命名 `OnRep_*`；RPC 遵守 UE 命名与声明规则（`_Implementation` 由 UHT 生成，不手写）。

## 当前项目状态与下一步

代码处于 **B00/B01 阶段**（课程第 4 节）：已完成武器拾取/装备、OverlappingWeapon 复制、RPC、装备姿势、蹲伏、瞄准。未实现：攻击/射击、血量/伤害、弹药、MatchState、拾取物、Lag Compensation 等（B02-B12）。

B00 基线验收要求：双客户端能完成进入、拾取、装备、瞄准、蹲伏。计划中下一步为 **B01（第 4 节动画与网络收尾：姿势、Pitch、转身、IK、蹲伏未装备动画）**。

## 文档与日志约定

- `.agents/worklog.md`：每次协作轮次后**追加**记录，只追加不覆盖历史。记录用户目标、检查证据、原因与决策、实际修改、验证状态、Editor 待办。即使只做了分析答疑也要记录。
- `.agents/learning-journal.md`：沉淀可复习的技术知识（按主题分类，末尾追加）。
- 源文件编码为 UTF-8（中文注释）；不要写成 GBK。
- 改完代码后，按 `.agents/Blaster_Context.md` 的偏好：解释"为什么这样写"、指出课程代码与 UE5.6 的差异、告诉用户如何在 UE 编辑器中验证。
