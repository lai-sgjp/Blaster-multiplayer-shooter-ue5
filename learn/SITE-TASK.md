# 学习网站任务

Gate 0：READY，2026-09-10。目标：将面试学习路线的阶段 0–5 与 B00–B12 扩展知识组成可离线学习、记录与可视化进度的网站。

交付状态：PASS WITH FOLLOW-UP。独立审查与真实浏览器/单元测试已完成，详见 site/VERIFICATION.md。仅保留构建依赖未作漏洞扫描的说明，不阻塞离线使用。

白名单：learn/index.html、learn/site/*、learn/README.md 的入口更新、.agents/worklog.md 的日志追加。用户追加要求合并计划、commit/push 和永久忽略后，Gate 0 扩展为允许整理 learn/INTERVIEW-LEARNING-ROADMAP.md、.agents/BLASTER_ACCELERATED_LEARNING_PLAN.md 以及 .gitignore；保留并包含现有相关路线与日志修改，不改写历史。无需修改 UE 源码或 Editor 资产。课程 Transcript 未读取；教学以本地计划、源码与资料为准。

数据流：已核对的课程内容及源码 → 静态快照 → 浏览器；用户笔记/自评 → localStorage → JSON 导出恢复。无账号、无远端写入。学习进度和游戏实现/验证状态分别展示。

验收：所有课可阅读；计划覆盖映射无遗漏；源码入口有效；笔记刷新保留；进度计算、导出/导入、损坏备份拒绝、移动端布局通过浏览器验证。网站任务以浏览器测试代替与本次变更无关的 UE Build/PIE，不新增游戏验证结论。

UE5.6 迁移：引用当前 Enhanced Input、RPC _Implementation、AnimInstance 与模块源码；既有游戏运行证据仅作资料引用。完成后审查与自评记录在 site/VERIFICATION.md。
