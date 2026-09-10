# Blaster Lab

双击 `learn/index.html` 即可使用。不需要 npm 安装、不需要后端、没有外部 CDN 或账号。网站完整包含项目讲义与源码快照，断网可读。

## 学习与记录

从总览进入推荐课程，依次阅读原理、项目例子、源码、验证和面试表达。每课四项自评分别打勾；主线百分比是已勾选项 /（主线课数 × 4），四项全勾选才算完成一课。选修单独统计。标记“待复习”后在复习页集中回看。阅读不自动证明你已掌握，更不代表游戏功能已验收。

笔记与自评保存在当前浏览器 localStorage。更换浏览器、移动网站、清除浏览器数据或使用隐私窗口可能看不到原记录。定期点击“导出学习备份”；换浏览器后用侧栏“恢复备份”。窄屏顶部提供“备份 / 恢复”按钮。备份按每课修改时间合并，旧备份不会覆盖较新的本地课程记录。无法读取或写入存储时会显示错误，不假装保存成功。

用户笔记不写入源码或 Git，备份文件 `blaster-progress-*.json` 在 learn 内永久忽略。网站无在线 AI 会话；教学内容和参考回答离线提供。可将导出的笔记交给后续教学任务继续指导。

## 内容维护

- `author_lessons.py`：课程编辑源，包含通俗解释、实例、提问、回答和源码入口。
- `lessons.json`：生成的课程数据。
- `build_content.py`：读取当前仓库讲义与源码，验证路径/符号后生成 `content.js`，含行号与 SHA256。
- `content.js`：提交到仓库以支持双击离线阅读；源码改动后重新生成，网页不会自动读取磁盘上的新源码。
- `progress.js`：独立的进度验证、统计与合并规则。

在项目根目录执行（需要 Python 3 与 Python-Markdown）：

```powershell
python learn/site/author_lessons.py
python learn/site/build_content.py
node --test learn/site/progress.test.cjs
```

浏览器验证脚本 `browser.test.cjs` 依赖 Playwright，可通过 `NODE_PATH` 指定已有安装。不把机器专用路径和浏览器缓存提交。截图和测试临时文件放入被忽略的 `test-results/`。本次结果见 `VERIFICATION.md`。
