# 街区改造：交付与验证记录

日期：2026-09-11。状态：可运行首版已落地，完整体验验收尚未全部通过。所有结果来自本机，不代表多台机器、多 Steam 账号的公网测试。

## 启动和房间规则

Windows Development 包：`Saved/StreetBuild/Windows/Blaster.exe`。保留整个 Windows 文件夹运行，不要只复制 exe。默认使用现有 Steam OnlineSubsystem / SteamSockets，开发 App ID 为 480；先启动并登录 Steam，再创建或加入房间。

房间最多八人，**至少三人，房主主动开始**。不会因满员或全员准备自动开局。房主可取消五秒倒计时；加入、离开会取消倒计时，需要房主再次发起。非房主请求由服务器拒绝。不包含专用服务器、房主迁移、账号后台或跨局背包。

Tab / Esc 打开同一个随身装备菜单。菜单停止本地移动、射击和瞄准，不暂停世界；仍可受到伤害。医疗包和加速道具各最多三个，医疗恢复25、加速900持续8秒，重复使用刷新持续时间。声音和灵敏度设置保存在本机。

## 实际交付

- `Content/Street/Maps/StreetStart.umap`：小型开始场景和固定角色镜头。
- `Content/Street/Maps/StreetLobby.umap`：可走动匹配广场、成员与人数、房主开始入口。
- `Content/Street/Maps/StreetArena.umap`：约120×100米街区、下沉中央区、三条路线、二层连廊和局部三层平台；八个分散出生点。
- 新材质、原创模块化建筑组合、栏杆/楼梯/长椅/灯/树/站棚、中文粗体字体、反馈声和薄层 Widget Blueprint。源资产与旧角色、旧地图分开保存。
- C++：统一头部规则和伤害倍率、四态准心、服务器命中反馈、武器查询通道、背包所有者复制、菜单输入生命周期、Steam 会话包装、容量与开局权限检查。
- UMG：开始、房间、HUD、背包和设置采用统一暖黑/米白/橙色；目前是可运行设计首版，细节密度和动效尚未达到参考图的完成度。
- `Tools/Overhaul/README.md`：资产构建和测试脚本顺序；`docs/OVERHAUL-SOURCES.md`：设计参考、字体许可与原创资源说明。

注意：仓库现有忽略规则包含 Content 和 Saved。地图确实已保存、也已打包，但不会自动出现在 Git diff 中。`docs/OVERHAUL-ASSET-MANIFEST.json` 记录本次 Street 资产的大小及 SHA256。未暂存、提交或推送，也未覆盖旧地图。

## 验证结果

| 项目 | 结果与证据 |
|---|---|
| Development Editor / Game | 两个目标成功构建；最终同源重建后 cook。编译日志与打包日志在 Saved。 |
| Windows cook / stage / archive | `Saved/street-package-final.log` 成功；实际启动包，进入开始场景并创建 Steam 房间、离开回到开始场景。 |
| Widget Blueprint | W_StreetScreen 编译/校验通过，0错误、0警告；不将其等同于所有布局交互通过。 |
| 代码自动化 | Blaster.Street.CombatAndInventoryBoundaries 通过；只覆盖头部/倍率/库存边界等基础规则。 |
| 2 / 3 / 8 人房主开局 | `street-host-start-{2,3,8}.json` 全部检查通过。两人不能开始、远端不能越权、房主开始/取消均已测。 |
| 八人旅行 | `street-travel-host-test.json`：8个 PIE 世界进入 StreetArena。 |
| 包容量和断线 | 独立打包主机+7个本机IP客户端成功连接；第9客户端收到 Server full；主机关闭后客户端回到开始地图。日志 `Saved/QA/ninth-client.log` 和 `client-*.log`。 |
| Steam 集成 | `street-packaged-final.log`：Steam Client/Server API 初始化、创建监听房间、退出清理成功。尚未验证跨机器搜索/加入与八个 Steam 账号。 |
| 头身伤害 | `street-combat-test.json`：服务器实际普通射线40/20、投射物40/20；霰弹按每颗头8/身4汇总，记录52/32，散布可导致混合命中。 |
| 背包与死亡 | `street-inventory-test.json`：9项通过，包括上限3、第四地面拾取保留、满血不消耗、实际恢复25、菜单中被杀、死亡关闭并清空。 |
| 楼梯通行 | `street-walk-test.json`：6个实际移动检查通过，走上二层和三层。修正二层楼板边缘阻挡胶囊的问题；辅助坡面只阻挡 Pawn。 |
| 武器碰撞 | `street-collision-test.json`：6项几何探测通过，包含栏杆空隙穿过、实体栏杆/墙/楼板挡弹和入口通行。不是整个地图逐三角形穷举。 |
| 摄像机 | `camera-telemetry.json`：新地图平地4秒持续移动，主机/客户端胶囊到相机距离325.461cm稳定。没有修改动画根位移；旧地图原始逐帧样本未保留，不能作为完整前后对照。 |
| 实际菜单录像 | `PackagedInventoryInteraction.mp4`：10秒本地真实窗口无声录制，Tab打开、等待、关闭；截图同名PNG。窗口边缘的桌面挂件不属于游戏资产。 |
| 分辨率 | 1080p界面、2560×1440开始界面、3440×1440开始界面已截图；超宽屏检查发现相机黑边并修正。尚未穷举三种分辨率下每个设置/结算状态。 |

## 性能范围

原生1920×1080，中等质量，r.ScreenPercentage=100，主机渲染+七个本机 null-RHI 客户端。CSV共15000帧，统计末6000帧静态出生点视角：平均帧时间3.239ms，P95 3.933ms，P99 4.288ms；GameThread平均1.742ms，RenderThread3.233ms，GPU2.730ms。当前样本更接近渲染线程约束。

证据：`Saved/street-performance.json` 与 `Saved/StreetBuild/Windows/Blaster/Saved/Profiling/CSV/Profile(20260911_195050).csv`。这不是八人真实交火，也不是八台机器渲染；尚未完成固定路线前后帧时间对照，不能宣布完整60FPS性能目标已验收。

## 修复过程中遇到的问题

- 投射物球形接触在当前角色头部位置先返回 backpack 骨骼；沿实际飞行段对受击 Mesh 细查后头部伤害通过。
- 二层台阶最后一段伸入连廊，角色头部撞到楼板；将梯口移到楼板外缘、增加仅Pawn碰撞的辅助坡面后实际步行通过。武器仍查询可见台阶实体。
- 早期包的 Editor 与 Game 反射布局不同，造成启动崩溃；统一重建并完整重 cook 后已实际运行。不要混用旧二进制和新 cook 资产。
- CSV早期启动参数在RHI初始化前触发引擎断言；改为地图启动后的 ExecCmds 采样，15000帧已成功完成。

## 尚未完成的完整验收

1. 多台机器、独立 Steam 账号的搜索/加入/断线提示和八人对战，无法用本机 NULL/IP 与单账号创建代替。
2. 移动目标和模拟延迟下，全部武器的服务器回溯、换弹、连续按火后开关菜单及重生组合回归；已有六次伤害测试覆盖面有限。
3. 每一处门窗、贴墙枪口、坡道/高点交火的实际多人体验录像；本轮只验证有限几何位置和楼梯路线。
4. 全状态1080p/1440p/超宽屏焦点与鼠标恢复、完整结算表现和更丰富图标/动效。当前结算为状态提示，尚非完整成绩榜。
5. 参考图级别的场景细节和动画协调仍需美术迭代；现有建筑是原创模块化首版，有明显简化。没有将灰盒通行成功等同于最终美术验收。
6. 八人真实交火固定路线的前后性能对照、动态瓶颈和声音主观听感验收。

## 五轴自评

| 维度 | 分数 | 依据与下一步 |
|---|---:|---|
| 准确性 | 4 | 构建、伤害、容量、库存、步行均有本地证据；跨账号Steam与完整回溯矩阵未验证。 |
| 完整性 | 2 | 三场景与核心系统已落地，但上述完整体验、美术、网络和性能验收仍有实质缺口。 |
| 清晰性 | 4 | 已区分代码、资产、打包、运行和未测结果；证据较多，需按表查阅。 |
| 可操作性 | 4 | 可直接运行完整Windows包，资产和重建脚本已保存；跨机器验证需要独立账号/机器。 |
| 简洁性 | 3 | 大任务记录较长，但保留失败修复与边界避免误认全部完成。 |

平均3.4/5。优先补跨机器Steam与动态战斗、全状态UI、固定路线性能。用户要求的是完整成品，因此不能把此首版评价为全部完成。

## 用户反馈修复：Client 1 准心

用户在640×480左右的直接多人PIE客户端中报告准心不可见，并提供截图。客户端生命、弹药UMG和武器正常。准心改为StreetWidget::NativePaint在所属本地玩家的Widget几何中心绘制，最小物理像素线宽加黑描边；Canvas HUD仅保留地面拾取标签。Editor/Game构建通过。`Saved/ClientReticle640.png`实际Client 1截图可见准心，`Saved/ClientReticleAfterMenu.png`证明关闭菜单后恢复。未声称已确定旧Canvas路径在所有机器上不可见的唯一根因。独立复审未发现阻断项；整个StreetWidget强制每帧绘制存在可优化空间。

## 延迟下远端库存补测

`Saved/street-latency-inventory.json`：100ms模拟延迟，7项全部通过。服务器日志明确 authority=1 / netmode=2：速度900、库存3→2→1、同帧第二次请求被0.25秒冷却拒绝、不叠倍率、刷新后过原到期时刻仍有效、最终恢复600。

测试发现UE5.6的AActor::GetFunctionCallspace在GAllowActorScriptExecutionInEditor为true时强制Local；因此Python直接或嵌套调用RPC都不能当远端网络证据。本次新增仅WITH_EDITOR且仅PIE可用的blaster.QAUseSpeed，将请求排到后续引擎tick，确认真实服务器执行。此前库存失败是无效测试调用；服务器伤害/库存边界测试仍有效，但早期大厅“remote client cannot start”的Python条目仅证明本地函数拒绝，不作为真实远端RPC验收。容量、服务器房主请求和八世界旅行的事实不受此限制。

本次另观察到重开PIE时OnlineSubsystemNull::Shutdown的SessionInterface.IsUnique handled ensure。PIE随后继续运行，未导致本轮测试失败；多次切换Steam/NULL的编辑器会话清理还需专门回归。

## 最终包复核

最新BuildCookRun成功退出0（155.39秒，归档曾被本次七个无界面基准客户端占用，核实PID后关闭并自动重试成功）。归档exe与staged exe SHA256一致：A91DB19534F5D2DDAD46BF84C54984F51D69E3B5236939CA644F18FEDC403FCD。没有保留占用CPU的测试客户端。

`Saved/StreetUltrawideFixed00000.png`为最终3440×1440原生截图，确认固定相机黑边消除。`Saved/PackagedReticleFinal.png`为最终Windows包的准心截图；`Saved/PackagedInventoryInteraction.mp4`已更新为最终版本实际菜单开关录像。用户提供的Client 1缺准心问题以修复后640×480截图为直接验证证据。

提交说明：本次发布调整为只跟踪 Content/Street 原创资源；旧资源包仍被忽略。Saved 中的日志、视频和打包文件仅保留本机。生成和测试脚本已使用工程相对路径；原工程角色、武器及蓝图依赖仍需本地已有资源。
