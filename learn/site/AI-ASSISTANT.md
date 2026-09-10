# 蕾米埃尔 · AI 学习助教

## 启动与填写 API Key

1. 双击 `learn/启动学习助教.cmd`。需要 Node.js 22 或以上（当前本机已有）。启动器在后台运行本机服务，并打开 **http://127.0.0.1:38761**。
2. 点击右下角角色，打开 **⚙ API 设置**。
3. 填写 **API Base URL**（如服务商给出的 `https://.../v1`）、**模型名称**、**接口格式**和 **API Key**。Base URL 不包含 `/chat/completions` 或 `/responses`，也不能在 URL 中携带 Key。
4. 点击“保存设置”。保存不会请求付费模型；打开课程并发送问题后才会调用 API。若模型拒绝 token 参数，可在“通用兼容（max_tokens）”与“新版 token 参数（max_completion_tokens）”之间切换；Responses 使用独立请求格式。

无需把 Key 填到代码、终端命令、聊天框或进度文件里。页面保存后清空 Key 输入框，不回显；更换 Base URL 必须重填 Key，以免把旧 Key 发给其他服务。

如果仍用 `learn/index.html` 双击打开，可以离线阅读和查看历史记录，但不会从 `file://` 页面直接调用 API。旧离线页面与新 HTTP 地址有不同的浏览器存储：**先在旧页面导出学习备份，再在新地址恢复备份**。网站不会自动搬走或清除旧记录。

## 问答、评价和进度

- 打开一节课，点击角色输入问题。助教使用这节课的摘要、真实源码片段和最近对话解释。
- 在课程里填写“我的面试回答”“概念卡”或“验证记录”，点击对应助教入口，选择内容后点“评价当前回答”。
- 评价包含总体反馈、0–100 分、具体优点、缺口、修正示例与下一步。模型未按格式返回、截断、超时或拒绝时，不保存半截评价，不改进度。
- 评价自动留档；**点击“采纳这些进度建议”后**才补充理解、源码追踪、面试复述与待复习标记。保留已有勾选，永远不由 AI 勾选“验证记录”。学习者仍可自行取消勾选。
- 每课保留最近 **5 次评价、6 轮问答**，与原学习笔记一同导出/恢复。旧备份不含 AI 字段也能导入。记录属于本机浏览器，建议定期导出；浏览器存储满时会提示。
- 正在生成时可以取消等待；服务商可能已开始计算或计费，因此取消不保证免费，也不会自动重试产生第二笔请求。

发送范围在面板底部说明：仅当前课摘要/源码、你明确发送的问题或所选回答、最近相关对话。不会自动上传整库笔记或角色图片。没有远端工具执行、文件写入或命令执行能力。角色人设与教学契约见 [角色卡](character-card.md)。

## Key 的具体存放位置

生产设置保存在仓库之外：

```text
%LOCALAPPDATA%\BlasterLab\assistant\config.json
```

Windows 默认对应 `C:\Users\Lai\AppData\Local\BlasterLab\assistant\config.json`。文件只保存 `protectedKey` 密文，使用 **当前 Windows 用户的 DPAPI**，不能直接拿到另一用户/另一机器解密。API Key 只在后端内存解密并放入 API Authorization 请求头，不写入课程 JavaScript、localStorage、导出备份或日志。需要移除时在设置里点击“移除已存 Key”。

密钥仍应由可信本机用户保护；DPAPI 不防止已取得同一 Windows 用户权限的恶意程序。不要手工复制明文 Key 到项目。兼容本地 API 时仅允许 loopback HTTP，远端地址必须 HTTPS。上游重定向被拒绝。

高级用法支持 `BLASTER_TUTOR_API_KEY`、`BLASTER_TUTOR_BASE_URL`、`BLASTER_TUTOR_MODEL` 启动环境变量。存在这些变量时网页保存被拒绝，需修改启动环境并重启。日常使用优先网页设置，不建议在终端命令历史中输入 Key。

## Git 防护

- `.gitignore` 忽略 `.env`、本地 credentials/secrets/API 配置、DPAPI 密文和测试产物。
- `.githooks/pre-commit` 调用 `learn/site/check-secrets.cjs`，检查暂存区凭据文件名、常见 Key/Token/私钥格式；发现后拒绝提交，仅报告文件名与行号。
- 当前仓库启用 `git config core.hooksPath .githooks`。新克隆需要执行一次该设置。若已有其他 hooksPath，应先合并钩子，避免覆盖自己的流程。
- 静态扫描不能识别所有未知格式，不能代替“密钥始终放在仓库外”的设计。本轮不读取其他项目 Key，也不将任何真实 Key 用于测试或提交。

## 运行与故障

本机服务监听 127.0.0.1:38761，不向局域网开放；只服务固定的前端文件，不能用 URL 读取后端、配置和其他仓库文件。启动器发现已运行的本项目服务会复用；端口被其他程序占用时显示错误，不自动跳到其他端口造成进度看似丢失。

日志位于 `%LOCALAPPDATA%\BlasterLab\assistant\server*.log`，只记录启动或概括错误。可在任务管理器中结束对应的 Node 进程停止服务；重新运行启动器恢复。脚本更新后要重启后台服务。

首次填 Key 后可问“请用本课代码解释 Owner 和 Authority”。401/403 检查 Key/权限，429 检查限流或余额，模型/格式错误检查服务商文档。当前实现不会擅自更换模型或服务地址。**本次只用模拟提供商和测试凭据验收，尚未用你的真实 API Key 进行第三方模型调用。**

实现参考：[OpenAI Chat API](https://developers.openai.com/api/reference/resources/chat)、[Responses API](https://developers.openai.com/api/reference/cli/resources/responses/methods/create)、[密钥与生产实践](https://developers.openai.com/api/docs/guides/production-best-practices)。兼容服务的模型、计费和参数以其文档为准。
