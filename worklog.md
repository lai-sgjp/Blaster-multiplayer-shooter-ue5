# Blaster 项目工作日志

本日志记录 Codex 与项目开发者每轮研讨的检查过程、关键证据、技术决策、代码变更和验证结果，用于在后续协作中恢复工程上下文。记录只追加，不覆盖历史。

---

## 2026-07-15：Shift 无法切换蹲伏与站立

### 用户目标和现象

- 已创建 `IA_Crouch`，并在 `IMC_Default` 中映射 Left Shift 和 Right Shift。
- 已在 `BP_BlasterCharacter` 为 `CrouchAction` 指定 `IA_Crouch`。
- C++ 已将 `CrouchAction` 的 `Started` 事件绑定到 `CrouchButtonPressed()`，函数在 `Crouch()` 与 `UnCrouch()` 之间切换。
- 动画实例读取 `ABlasterCharacter::bIsCrouched`，但按 Shift 后没有蹲伏或站立动作。

### 检查步骤与关键证据

1. 检查 `BlasterCharacter.h/.cpp`：输入属性、回调声明和 `BindAction` 链路完整，回调使用 `ETriggerEvent::Started`，适合按键切换。
2. 检查 `BlasterAnimInstance.h/.cpp`：`bIsCrouched = BlasterCharacter->bIsCrouched` 每帧更新，反射属性带 `UPROPERTY(BlueprintReadOnly)`，动画蓝图可以读取。
3. 检查动画截图：`Idle <-> CrouchIdle` 的正反向条件正确，但蹲伏状态只位于 `Equipped` 状态机；未装备分支不会使用它。
4. 检查 UE5.6 源码：`ACharacter::Crouch()` 只有在 `CanCrouch()` 成功时才设置 `bWantsToCrouch`；`UCharacterMovementComponent::CanCrouchInCurrentState()` 首先要求 `CanEverCrouch()` 为真。
5. 检查 `Saved/Logs/Blaster.log`：第 2842-2845 行明确输出 `BP_BlasterCharacter_C_0 is trying to crouch, but crouching is disabled on this character! (check CharacterMovement NavAgentSettings)`。

### 原因与技术决策

- 根因：角色没有启用 `CharacterMovement` 的 `bCanCrouch` 移动能力。输入已经到达 `Crouch()`，但被引擎能力检查拒绝，因此 `bIsCrouched` 始终不会变为 `true`。
- 次要动画问题：未装备武器时，AnimGraph 输出 `Unequipped` 状态机，而截图中的 `CrouchIdle` 只存在于 `Equipped` 状态机。
- 使用引擎自带 CharacterMovement 蹲伏网络流程，不新增自定义 Replicated 变量、Server RPC、Client RPC 或 NetMulticast。`ACharacter::bIsCrouched` 和 `OnRep_IsCrouched()` 已负责模拟代理同步。

### 实际修改

- `agents.md`：新增每轮协作都维护 `worklog.md` 的强制规则。
- `learning-journal.md`：新增蹲伏请求、移动复制和动画状态机边界知识。
- `worklog.md`：创建项目工作日志并记录本轮完整过程。
- `Source/Blaster/Character/BlasterCharacter.cpp`：经用户确认后从 GBK 无损规范化为 UTF-8，并在构造函数中加入 `GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;`。

### 验证状态与编辑器待办

- 当前已由日志和 UE5.6 源码确认根因，C++ 修复已写入。
- `BlasterCharacter.cpp` 原先使用旧的 GBK 编码；已在用户批准后无语义变化地规范化为 UTF-8，中文注释保持正确。
- 使用 UE5.6 UBT 执行 `BlasterEditor Win64 Development` 增量构建。`[1/4] Compile [x64] BlasterCharacter.cpp` 成功，证明本次 C++ 修复通过编译。
- 完整构建在链接阶段以 `LNK1104` 停止：运行中的 `UnrealEditor.exe`（PID 28560）与 `LiveCodingConsole.exe` 正占用 `Binaries/Win64/UnrealEditor-Blaster.dll`。未强制关闭编辑器，以免丢失尚未保存的蓝图改动；关闭编辑器后需要再执行一次完整构建。
- 首次构建的 UBA 因提交内存不足持续延迟 action；已停止本轮启动的残留 UBT 进程，并使用 `-NoUBA -MaxParallelActions=1` 完成源码编译验证。最终没有遗留 `dotnet`、`cl` 或 `link` 构建进程。
- `git diff --check` 未发现补丁空白错误，仅报告 Git 未来会按仓库设置把 LF 转为 CRLF。
- 编译后在 `BP_BlasterCharacter > Character Movement: NavMesh Movement > Movement Capabilities` 检查 `Can Crouch`。若蓝图保存了旧的显式覆盖值，需要点击属性旁的黄色重置箭头，使其继承 C++ 默认值，或直接勾选。
- 在装备武器后测试当前 `Equipped` 蹲伏动画；若希望未装备也播放蹲伏动画，需要在 `Unequipped` 状态机增加对应 `CrouchIdle` 及双向规则，或后续统一两套移动状态机的公共蹲伏分支。
- PIE 至少使用 Listen Server + 1 Client：分别在两个窗口切换蹲伏，确认本机胶囊、对端姿势以及低矮空间下无法站起的碰撞行为。


---

## 2026-07-15：RPC 概念复盘与教学

### 用户目标和现象

- 已完成课程四个小节（remote procedure calls, equipped animation pose, crouching, aiming）的编码，代码已 `git add` 但未 commit。
- 对 RPC 中的关键概念感到困惑：`UFUNCTION(Server, Reliable)` 的含义、`_Implementation` 后缀的由来、Replicated 与 RPC 的区别、"rep" 缩写到底指什么。
- 需求：不修改代码、不撤销 staged 内容、不提交，仅帮助理解概念并给出练习。

### 检查步骤与关键证据

1. 读取 git staged diff，梳理本轮改动涉及的 5 个源文件
2. 识别出三种不同的网络同步模式：Equip（HasAuthority 分支）、Aim（双头模式）、Crouch（引擎内置）
3. 对比 ue5-course-tutor skill 的教学结构与 agents.md 已有指令，确认缺少"代码复盘与概念教学"场景的处理指令
4. 验证 learning-journal.md 中已有网络复制基础（第 5 节），但缺少 RPC 系统专题

### 原因分析与技术决策

- 用户混淆的根本原因：三个关键词（Replicated、RepNotify、Reliable）容易混为一谈，且 `_Implementation` 后缀的 UHT 生成机制对初学者不透明
- 决定在 agents.md 增加"8. 代码复盘与概念教学"指令，规范此类场景的处理流程
- 决定在 learning-journal.md 新增第 16 节 RPC 系统篇，覆盖：三种 RPC 类型、_Implementation 后缀、Reliable vs Unreliable、双头模式、HasAuthority 分支模式、三种网络模式对比、端到端数据流
- 提供了三道练习题帮助用户验证理解

### 实际修改的文件与内容

- `agents.md`：新增第 8 节"代码复盘与概念教学"，定义处理流程
- `learning-journal.md`：新增第 16 节 RPC 系统篇，更新更新记录表
- 未修改任何游戏 C++ 代码

### 验证状态与后续待办

- 学习知识库中 RPC 专题已建立
- agents.md 中复盘教学指令已建立
- 建议用户先阅读 RPC 系统篇内容，再自行完成练习题
- 后续用户可继续提问或要求批改练习题答案
