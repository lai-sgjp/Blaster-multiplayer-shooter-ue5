# Blaster 项目式推进工作流

## 目的与适用范围

本流程用于将 Udemy 的 UE5.0 Multiplayer Shooter 课程转化为 Blaster 的 UE5.6 C++ 项目成果。目标不是线性跟课或堆叠代码，而是每次交付一个能独立验证、能解释网络数据流、能用于作品集陈述的垂直切片。

权威顺序：本项目真实状态和 UE5.6 编译/运行证据 > 课程源码提交 > UE5.0 Transcript 的具体 API 写法。Transcript 只在当前任务需要课程动机、步骤或 Editor 操作时读取；未实际读到时，任务卡必须标注为“未读取”，不能假称已核对。

## 工作状态

| 状态 | 含义 |
| --- | --- |
| `PLANNED` | 已从 B00-B12 路线中选定，尚未读取现状。 |
| `GATE 0` | 正在冻结范围；禁止修改实现。 |
| `READY` | Gate 0 与 UE5.6 迁移检查完成，可以实施。 |
| `IN PROGRESS` | 仅在任务白名单内实施与验证。 |
| `REVISE` | 审查或验证发现问题，回到同一任务修复。 |
| `BLOCKED` | 缺少可恢复的前置条件、权限或 Editor 证据。 |
| `PASS` | 全部验收项、主路径和失败路径通过。 |
| `PASS WITH FOLLOW-UP` | 核心目标通过，已记录不阻塞的明确后续项。 |

## 每个任务的固定循环

### 0. 恢复真实状态

读取上一任务卡、审查记录、当前 Git 状态和实际 C++/资产绑定。将结论分为“已实现”“已编译”“已 PIE 验证”“尚未验证”；不得以文件存在代替运行验证。现有未提交改动属于用户，除非明确授权，不撤销、不混入、不提交。

### 1. 定位课程与学习覆盖

在任务卡中记录计划包、课程章节、讲座主题/课程提交和本次只需理解的概念。课程路径以 `.agents/BLASTER_ACCELERATED_LEARNING_PLAN.md` 的 B00-B12 矩阵为准。需要 Transcript 时，只读与切片直接相关的讲座，并记录是否实际成功读取。

### 2. Gate 0：冻结任务

在任何代码或资产修改前完成 `TASK_TEMPLATE.md`：

- 一个玩家可见、独立可验收的目标；
- Server、拥有客户端、其他客户端和本地 UI 的责任边界；
- 输入到表现的网络数据流；
- 精确文件白名单和明确非目标；
- 用户须在 UE Editor 完成的动作；
- Build、双客户端 PIE 主路径和失败路径证据；
- Git 边界与提交范围。

目标、白名单、所有权或验证方式发生实质变化时，退回 Gate 0 重新冻结。

### 3. 最小实现与教学

先用学习者能理解的语言讲清楚本任务涉及的概念、数据流和验证方法，再邀请学习者用自己的话复述；复述用于巩固理解，不得作为没有教学前置的考试或阻塞条件。Codex 负责 C++、编译、日志和代码解释；学习者负责 Editor 中的蓝图派生、资源绑定、动画/Widget 配置和 PIE 观察。核心权威逻辑保持在 C++：客户端可做即时表现，Server 决定伤害、命中、弹药、拾取、分数和比赛状态。

### 3.1 自动推进与 Editor 操作边界

Codex 应自动推进所有不依赖用户点击的工作，包括现状检查、文档和 C++ 修改、构建、日志验证、可脚本化检查和结果记录。仅在必须由用户操作 Unreal Editor、需要用户确认阶段取舍，或遇到权限/破坏性操作/缺少证据的阻塞时暂停。

交给用户的 Editor 步骤必须给出从项目根目录开始的完整相对路径（例如 `Content/Blueprints/Character/Animation/ABP_Blaster`），并包含：打开方式、面板位置、字段或节点名称、预期结果、失败排查。实际资产路径未被检查确认前，不得猜测路径。

每个完成的知识点在项目根目录 `learn/` 下按主题分类记录。`.agents/learning-journal.md` 作为已有历史资料保留；新知识优先写入 `learn/`，工作日志仍记录协作过程、证据和待办。

### 4. UE5.0 到 UE5.6 迁移门

每张任务卡都要逐项记为“无需变化 / 已迁移 / 尚未验证”，并以 UE5.6 头文件、编译或运行证据裁决：

- Enhanced Input 与 LocalPlayer Subsystem 生命周期；
- Build.cs、显式 include、模块依赖和 UHT；
- `UPROPERTY`、GC、对象/组件生命周期；
- `HasAuthority()`、Local/Remote Role、Ownership；
- Replication、RepNotify、RPC `_Implementation` 和 Server 验证；
- UE5.6 的动画/Persona、Montage、Aim Offset、IK、移动和网络平滑；
- 本地 HUD/Widget 的初始化顺序；
- SteamSockets / Online Subsystem（仅相关任务）。

不凭记忆替换 API；遇到不确定项时先查 UE5.6 证据或实际编译错误。

### 5. 验证阶梯

按从低到高的顺序留存真实结果：

1. 编译：Development Editor Build，记录命令、结果与首个关键错误（如有）。
2. 单机冒烟：加载目标地图，确认必需资源、蓝图和 Widget 未阻塞。
3. 多人主路径：Listen Server + 1 Client PIE，分别记录服务端、拥有客户端和模拟客户端观察结果。
4. 失败路径：至少验证一次非法/边界输入或网络时序，例如无武器开火、换弹中开火、重复 Pickup、晚加入、Pawn 销毁后 UI 更新。
5. 回归：确认任务白名单外的已通过核心链路未明显退化。

Steam 外部联机是加分项，不替代 PIE 证据。

### 6. 独立审查与修订

实现者完成后，使用 `REVIEW_TEMPLATE.md` 重新从范围、UE5.6、反射/GC、网络权威、Owner/OnRep、失败路径和证据审查。发现阻塞问题则状态为 `REVISE`，修复后复验；不得用“课程源码如此实现”替代审查结论。

### 7. 学习与作品集门

在归档前，学习者应能口述：谁拥有状态、谁调用 RPC、Server 如何验证、什么被复制、谁由 OnRep/Multicast 更新、HUD 从哪里读。记录 2-3 道面试题、一个取舍或已知限制，以及真实完成范围。课程复刻必须如实标注 AI 协作、用户 Editor 配置和未验证项。

### 8. 归档与下一步

只有 `PASS` 或 `PASS WITH FOLLOW-UP` 可归档。按用户授权更新任务卡、审查记录、`worklog.md` 与 `learning-journal.md`；提交前只暂存本任务允许的文件，检查 diff，且必须有用户明确的提交授权。归档时只推荐一个下一任务，通常为 B00 → B01 → B02 的顺序，除非当前阻塞证据要求回退。

## B00-B12 推进矩阵

| 包 | 工作包 | 完成门槛 |
| --- | --- | --- |
| B00 | 当前基线验收 | 双客户端完成进入、拾取、装备、瞄准、蹲伏。 |
| B01 | 第 4 节动画与网络收尾 | 本地/自主/模拟代理姿势、Pitch、转身、IK 与蹲伏一致。 |
| B02 | 最小网络开火 | Server 权威 Projectile；两端有一致的开火表现。 |
| B03 | Weapon Aim Mechanics | HUD、准星、FOV、Trace 与自动开火边界成立。 |
| B04 | Health and Player Stats | 伤害、淘汰、重生、PlayerState 计分双端一致。 |
| B05 | Ammo | 弹匣、携带弹药、换弹、Combat State 和 HUD 一致。 |
| B06 | Match States | 时间同步和 WaitingToStart/InProgress/Cooldown/Restart 闭环。 |
| B07 | Different Weapon Types | Hitscan、Projectile、Shotgun 三个权威命中模型。 |
| B08 | Pickups 与双武器 | Pickup/Buff、主副武器、交换/掉落和死亡清理。 |
| B09 | Lag Compensation | 至少可验证 Hitscan SSR；其余 SSR 明确降级或完成。 |
| B10 | More Multiplayer Features | 高 Ping、离场清理、公告或领先者至少一条可演示链路。 |
| B11 | Teams 与 CTF | 核心闭环稳定后完成最小 Team/CTF，或保留验收设计。 |
| B12 | 作品集收尾 | 演示、README、架构/数据流图、验证矩阵与面试复盘真实一致。 |

## 停止条件

以下任一情况必须暂停实施并向用户报告：Gate 0 未冻结、白名单外修改、UE5.6 API 无证据、需要破坏性 Git 操作、需要修改不在本任务内的资产、或无法取得主路径/失败路径证据。不得通过扩大范围掩盖阻塞。
