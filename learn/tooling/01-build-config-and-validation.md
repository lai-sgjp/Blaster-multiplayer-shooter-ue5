# UE5.6 构建、配置、插件与验证方法

## 1. 本节目标

面试官经常会问“你怎么判断功能真的完成了”。这份文档把项目的构建和证据链讲清楚，避免把“文件存在”“编译成功”“PIE 看到一次”和“公网可交付”混成一个结论。

## 2. 工程入口和配置层

| 文件/目录 | 作用 |
| --- | --- |
| `Blaster.uproject` | UE 项目入口、模块声明、启用的引擎插件 |
| `Source/Blaster/Blaster.Build.cs` | Runtime 模块依赖和编译规则 |
| `Source/Blaster.Target.cs` | Game Target |
| `Source/BlasterEditor.Target.cs` | Editor Target |
| `Config/DefaultEngine.ini` | 地图、NetDriver、OnlineSubsystem、渲染和目标平台设置 |
| `Config/DefaultGame.ini` | 打包、MapsToCook、最大玩家等游戏/打包配置 |
| `Config/DefaultInput.ini` | 引擎输入默认类；具体 Enhanced Input 资产在 Content |
| `Content/` | Blueprint、Input、Map、动画、武器、UI 等资产；当前工作区被 Git 忽略 |
| `Plugins/` | MultiplayerSessions、VRM4U、UnrealMCPBridge 等插件 |

## 3. Runtime 与 Editor-only 插件

不要把所有插件都当成“游戏运行时技术”：

- `MultiplayerSessions`：Runtime 插件，提供在线 Session 封装；
- `VRM4U`：角色导入、运行时加载、编辑器工具和渲染模块；
- `UnrealMCPBridge`：Editor 模块，加载在 `PostEngineInit`，用于资产/蓝图检查和编辑器自动化，不是玩家运行时的核心依赖；
- `OnlineSubsystemSteam`、`SteamSockets`：在线和网络传输层；
- `PCG`、`PCGGeometryScriptInterop`：关卡/程序化生成能力，当前不是战斗权威逻辑。

面试时先说明哪些技术影响 Runtime，再补充 Editor 和资产管线工具。

## 4. 推荐的构建顺序

### Development Editor Build

关闭 Unreal Editor 后执行：

```powershell
& 'E:\programs\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat' BlasterEditor Win64 Development 'E:\work\unreal_projects\Blaster\Blaster.uproject' -WaitMutex -NoHotReloadFromIDE -NoUBA -MaxParallelActions=1
```

这条命令验证 UHT、UBT、模块编译和 Editor 链接。低内存机器使用 `-NoUBA -MaxParallelActions=1` 是稳定性策略，不是功能代码的一部分。

### Game Target Build

Game Target 能补充验证非 Editor 目标的模块/链接问题，但它不等同于 Cook/Stage 完整打包。打包、Cook、Stage、启动和第三方插件运行时还要单独验证。

## 5. 验证阶梯

```text
1. 文件/资源路径检查
        |
2. UHT/UBT 编译
        |
3. 单机地图启动冒烟
        |
4. Listen Server + 1 Client PIE 主路径
        |
5. 非法输入/时序/边界失败路径
        |
6. 高延迟、退出、重生、晚加入等回归
        |
7. 跨机器 Steam / Dedicated Server / Cooked Build（若需求要求）
```

项目的真实矩阵和限制集中在 `../../docs/VERIFICATION.md`。当前可以明确说有 Listen Server + 1 Client PIE 和 Development 构建证据；不能把它们扩大成公网 Steam、专用服务器或发布包证据。

## 6. UE Editor 与 Content 的验证边界

C++ 可以自动检查很多内容，但以下事项必须在 Editor 中真实打开和保存：

- Blueprint 默认类和变量绑定；
- AnimGraph 节点、状态机转换、Aim Offset、FABRIK 接线；
- Skeleton、Socket、IK Rig、IK Retargeter 和动画资产兼容性；
- Widget 蓝图中的 `BindWidget` 控件；
- Maps 中的 PlayerStart、TargetPoint 标签、GameMode 覆盖和 PCG 图。

因此一个可靠的任务记录要分开写：

- 源码已经修改；
- Editor 资产已经保存；
- 构建通过；
- PIE 主路径观察通过；
- 哪些仍是人工视觉或公网待办。

## 7. 渲染配置如何在面试中定位

`DefaultEngine.ini` 启用了 Lumen GI/Reflection、Ray Tracing、Virtual Shadow Maps，Windows 默认 DX12/SM6。历史验证中曾出现 DX12 device hung，因此当前验证使用 D3D11 和帧率限制参数；这说明“项目配置的目标渲染路径”和“本机当前可靠验证路径”可以不同。

渲染技术可以展示工程视野，但不要把它误说成网络或战斗实现。除非面试岗位是渲染方向，应先回答 Gameplay Framework、网络权威和战斗数据流。

## 8. 日志、失败分类和安全

遇到问题时先分类：

| 现象 | 优先检查 |
| --- | --- |
| UHT/GENERATED_BODY 错误 | 头文件宏、生成文件、类名和 Include |
| 找不到类型/模块符号 | Build.cs 依赖、Target、显式 include |
| LNK1104 DLL 被占用 | 关闭 Editor、Live Coding、相关进程 |
| PIE 逻辑不一致 | Authority/Ownership/Replication/OnRep |
| 动画姿势错误 | Skeleton、Socket、AnimGraph、空间转换 |
| Steam 找不到会话 | OnlineSubsystem 配置、Session 回调、现实网络环境 |

诊断日志时不要把密钥、Cookie、Token 或本地安全凭据复制进学习文档。配置文件里即使存在开发服务字段，也只记录字段名和用途，不记录敏感值。

## 9. 引擎源码阅读路线

- UBT/Target：项目 `Build.cs` 和 `Target.cs` 先看，再对照 `Engine/Source/Programs/UnrealBuildTool/`
- UHT：对照 `Engine/Source/Programs/UnrealHeaderTool/`
- Config：`Engine/Source/Runtime/Engine/Private/UnrealEngine.cpp` 和对应 Settings 类
- PIE/World：`Engine/Source/Runtime/Engine/Private/World.cpp`
- Plugin Module：`Engine/Source/Runtime/Core/Public/Modules/ModuleManager.h`

不需要在面试中背构建工具内部代码，但要能解释“宏、模块依赖、Target 和生成二进制如何连接起来”。

## 10. 面试快答

**问：为什么 UE 项目改了 C++ 后常常要关闭 Editor 再构建？**

答：Editor 或 Live Coding 可能占用 `UnrealEditor-Blaster.dll`，链接器无法覆盖正在使用的二进制；关闭相关进程后再做完整 Development Editor Build，才能排除 DLL 占用带来的假失败。

**问：PIE 通过能否说明游戏可以发布？**

答：不能。PIE 证明某条编辑器内运行路径成立；发布还要验证 Cook/Stage、Game Target、打包后资源、插件 Runtime、跨机器网络和启动配置。

**问：为什么 Content 资产也需要记录路径？**

答：C++ 类通常只声明接口，具体 Mesh、Socket、AnimBlueprint、Widget、Map 和默认类由 Content 资产绑定。没有精确资产路径和保存证据，源码本身无法复现完整功能。

## 11. 小练习

1. 解释 `Blaster.Build.cs`、`.uproject` 和 `DefaultEngine.ini` 各自应该放哪类配置。
2. 设计一个“动画 FABRIK 接入”的验收矩阵，至少分源码、资产、构建、单机视觉、双端视觉五列。
3. 把“编译失败”“PIE 失败”“Steam 找不到房间”分别归到构建、Gameplay/资产、在线环境三层，并说明第一步检查什么。
