# Blaster 学习知识库

这是 Blaster 项目随开发推进持续增长的知识库。新知识点按主题进入子目录，避免所有内容堆在一个长文档中。

## 面试学习入口

先读[项目技术栈总览](PROJECT-TECH-STACK.md)，再按“UE C++ -> 网络 -> 战斗 -> 动画 -> UI -> Online -> 构建验证”的顺序学习。每章都包含真实源码路径、端到端数据流、UE5.6 注意事项、验证方法、面试快答和小练习。

### 主线课程

- [UE C++、反射、构建与 Gameplay Framework](cpp/01-ue5-cpp-and-gameplay-framework.md)
- [Server 权威、Replication、RPC 与 RepNotify](networking/01-authority-replication-rpc.md)
- [射击战斗闭环与 Server Rewind](gameplay/01-shooter-combat-pipeline.md)
- [动画数据链路：AnimInstance、Aim Offset、Lean、转身与 FABRIK](animation/02-animation-data-pipeline.md)
- [HUD、UMG 与本地表现数据流](ui/01-hud-and-umg-data-flow.md)
- [Steam Session、OnlineSubsystem 与关卡 Travel](online/01-steam-session-and-travel.md)
- [UE5.6 构建、配置、插件与验证方法](tooling/01-build-config-and-validation.md)

### 深挖资料

- `animation/`：B01 动画网络、Aim Offset、Turn in Place 和 FABRIK Editor 操作。
- `networking/`：B02–B11 的开火、战斗闭环、比赛、拾取、SSR 和公告专题。
- `editor/`：必须由用户在 Unreal Editor 中完成的资产、Skeleton、Socket、AnimGraph 和 Persona 操作。
- `debugging/`：Editor 启动、构建和工具链故障复盘。
- `../docs/ARCHITECTURE.md`、`../docs/VERIFICATION.md`、`../docs/PORTFOLIO.md`：架构、验证边界和面试/作品集复盘。

## 分类

- `animation/`：AnimSequence、Blend Space、Aim Offset、IK、Persona、移动姿势。
- `networking/`：Replication、RPC、Ownership、RepNotify、Listen Server 和 PIE。
- `cpp/`：UE C++、反射、GC、生命周期、模块和 UE5.6 迁移。
- `editor/`：必须由用户执行的 Unreal Editor 操作，包含项目相对路径、面板、字段、预期结果和排查方法。
- `debugging/`：构建、日志、工具链和故障复盘。
- `gameplay/`：战斗系统、伤害、弹药、重生和延迟补偿。
- `ui/`：HUD、UMG、Widget 生命周期和本地表现。
- `online/`：OnlineSubsystem、Steam Session 和关卡 Travel。
- `tooling/`：Build.cs、Target、INI、插件和验证阶梯。

`.agents/learning-journal.md` 是已有历史资料；新条目优先放在本目录。
