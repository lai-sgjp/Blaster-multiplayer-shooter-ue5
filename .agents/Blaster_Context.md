# Blaster_Context.md

## 我的学习背景

我正在学习 UE5 C++ 游戏开发，主要跟随 Udemy 的 Unreal Engine 5 C++ Multiplayer Shooter 类课程。
我的目标不是简单复制课程代码，而是理解 UE5.6 中 C++ 多人游戏开发的底层逻辑。

## 我的技术栈

- Unreal Engine：5.6
- IDE：Visual Studio 2026
- 编程语言：C++
- 构建系统：Unreal Build Tool
- 版本管理：Git
- AI 工具：Codex + CC Switch + DeepSeek
- 主要学习方向：
  - UE Gameplay Framework
  - Character
  - PlayerController
  - GameMode / GameState
  - PlayerState
  - Replication
  - RPC
  - Weapon System
  - Combat Component
  - HUD / UMG
  - Animation Montage
  - Hit Scan / Projectile
  - Lag Compensation
  - Match State
  - Online Session

## 我的偏好

1. 主要使用 C++ 实现核心功能。
2. 蓝图只用于：
   - 创建 C++ 类的 Blueprint 子类
   - 设置 Mesh、Animation、Montage、Sound、Particle
   - UMG Widget 布局
   - 简单表现逻辑
3. 我希望 AI 不只是给代码，还要解释为什么这样写。
4. 我希望 AI 发现课程代码和 UE5.6 差异时主动提醒。
5. 我希望 AI 每次帮我写代码后，都告诉我如何在 UE 编辑器中验证。

## 当前项目约定

如果没有特别说明：
- Character 类负责角色移动、输入绑定、基础表现
- CombatComponent 负责战斗状态、开火、换弹、瞄准
- Weapon 类负责武器表现和开火行为
- PlayerController 负责 HUD 和本地输入相关逻辑
- PlayerState 负责分数、击杀、死亡等玩家状态
- GameMode 负责比赛规则
- GameState 负责比赛全局状态同步
- HUD / Widget 只在本地客户端创建和更新

## 我希望 AI 避免的问题

1. 不要把多人游戏权威逻辑写在客户端。
2. 不要在 GameMode 中写客户端 UI 逻辑。
3. 不要在 Tick 中做大量轮询。
4. 不要随便使用 GetPlayerController(0) 处理多人逻辑。
5. 不要忽略 Authority 和 Ownership。
6. 不要生成无法通过 UE Header Tool 的代码。
7. 不要省略 Build.cs 模块依赖。
8. 不要编造 UE5.6 API。
