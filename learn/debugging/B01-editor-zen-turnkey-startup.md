# B01：Editor 启动时的 Zen 与 Turnkey 阻塞

## 启动数据流

1. UnrealEditor 读取项目和 Engine 配置。
2. `ZenServerInterface.cpp` 为本地 Derived Data Cache 选择 Zen 数据目录。
3. `ValidateDataPath` 会在候选目录创建并删除 `.zen-startup-test-file-<pid>`，用实际写入证明目录可用。
4. `TurnkeySupportModule.cpp` 启动 UAT：`Turnkey -command=VerifySdk -platform=all`，把结果写入项目 `Intermediate/TurnkeyLog_*.log` 和 `TurnkeyReport_*.log`。
5. Editor 继续初始化，项目插件随后开始监听 MCP 端口。

## 本次现象与根因

- 卡住的 Editor 日志先出现 `Skipping Zen config default ... due to an invalid path` 和 `Unable to determine a valid Zen data path`。
- 窗口标题“无有效数据路径配置”来自 `PromptUserUnableToDetermineValidDataPath`，不是 Blaster 项目路径错误。
- 旧进程由受限沙箱启动，无法在 `C:/Users/Lai/AppData/Local/UnrealEngine/Common/Zen/Data` 创建测试文件；模态提示阻塞了 Editor 启动。
- Turnkey 的 `-532462766` 转为十六进制是 `0xE0434352`，表示 CLR/.NET 未处理异常。它不是“SDK 无效”的正常 Turnkey 状态码，不能仅凭该数字重装 SDK。

## 安全恢复与验证

- 先检查目标 PID、子进程、窗口和日志；优先正常关闭。
- 只有正常关闭失败且 PID 已明确确认时，才只结束该 PID，不使用进程树范围去影响其他 Unreal 进程。
- 重新启动时传入完整项目路径，并让 Editor 进程拥有写入用户 AppData 的正常权限；本次没有修改 Zen 配置。
- 恢复后的证据：Zen 8558 状态 OK；Turnkey 平台/设备检测 ExitCode 0；Win64 SDK 状态 Valid；UnrealMCPBridge 在 127.0.0.1:8765 监听；doctor plugin freshness 为 OK。

## 排查要点

- 若再次出现相同标题，先在 `Saved/Logs/Blaster.log` 搜索 `LogZenServiceInstance`，不要先改 `.uproject`。
- 检查候选 Zen 数据目录是否存在、是否可写，以及 Editor 是否从受限执行环境启动。
- `Intermediate/TurnkeyReport_*.log` 不存在通常说明 UAT 在生成报告前异常退出；应先找 UAT/.NET 的第一处真实异常，而不是把最后的 ExitCode 当根因。
