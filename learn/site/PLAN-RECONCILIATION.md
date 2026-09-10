# 计划合并与课程覆盖

更新于 2026-09-10。唯一教学顺序来自 `learn/INTERVIEW-LEARNING-ROADMAP.md`；旧四周工程计划保留编号和独有知识。每天约 60–90 分钟是主线建议，不把两套周计划叠加。每课 35 分钟是阅读起点，实际源码与 Editor 验证可以跨次完成。

## 已处理冲突与冗余

| 原问题 | 统一后的处理 |
| --- | --- |
| 四周工程交付与六阶段学习并行 | 新路线控制教学节奏，旧 Week 仅为历史工程时间盒 |
| 旧计划要求先解释预测数据流 | 统一为先教学再邀请复述，检查题不阻塞 |
| 旧计划仍称完成到课程第 4 节 | 标为过时基线，现状查源码和验证矩阵 |
| README 又提供第三种学习顺序 | 改为新路线阶段 0–5 |
| 网络、开火、动画等多份材料重复 | 每知识点只设一个主课程，旧专题作为可展开深读，不重复计进度 |
| 旧计划的预测、其他 SSR、Teams/CTF | 保留选修设计，明确未实现，不冒充现有能力 |
| 实现进度与学习进度混淆 | 所有课程初始为待学；实现边界独立展示 |

## 新路线逐项覆盖

| 阶段 | 待学内容 | 网站课程 |
| --- | --- | --- |
| 0 | HSR 职责迁移、GAS 与轻量 Component 取舍 | L01 |
| 1 | Actor/Character/Component/Controller、GameMode/State/PlayerState、生命周期 | L02 |
| 1 | Authority、Local、Owner、自主/模拟代理 | L03 |
| 1 | Replicated、RepNotify、注册、OwnerOnly、组件复制 | L04–L05 |
| 1 | Server/Client/Multicast、Reliable 不可信、数据与表现分离 | L05、L07 |
| 2 | 本地输入/计时、服务器射速、相机与枪口 | L06 |
| 2 | HitTarget、RequestedWeapon、ShotTime、ShotId 验证顺序 | L07 |
| 2 | Projectile/Hitscan/Shotgun、ApplyDamage、伤害/淘汰/计分/重生 | L08、L10 |
| 2 | 历史采样、时间窗口、两帧插值、骨骼盒与世界遮挡 | L11–L12 |
| 2 | 为什么不是所有武器都有 SSR | L08、L28 |
| 3 | Input Action、Mapping Context、Trigger、LocalPlayer、全部战斗输入 | L14–L15 |
| 3 | AnimInstance 生命周期、Speed/Air/Yaw/Pitch/Lean/Turning、移动数据流 | L16–L17、L32 |
| 3 | Socket、BoneSpace、FABRIK、AnimGraph 保存验收 | L18 |
| 3 | IK Retargeting 与第三方骨架兼容 | L19 |
| 4 | Subsystem、异步 Delegate、Create/Find/Join/Destroy | L20 |
| 4 | ServerTravel/ClientTravel/SeamlessTravel、Lobby PostLogin | L21 |
| 4 | HUD/UMG/WidgetComponent、复制状态、UI 生命周期 | L22–L24 |
| 4 | PIE / NULL / Steam / 跨机器不同证据 | L21、L26 |
| 5 | Build.cs Public/Private、Target、UHT/UBT、Runtime/Editor | L25 + 构建专题 |
| 5 | Editor/Game 构建、双端主/失败路径、日志/截图、Content/DX12/发布边界 | L26 |
| 5 | 三分钟介绍、源码证据、技术取舍、最终通关清单 | L27 + 面试路线原文 |

## 旧工作包映射（不再重复排课）

| 包 | 对应课程/深读 | 需要保留的边界 |
| --- | --- | --- |
| B00 | L01–L05、L14 | 原 UE 基础按需复习 |
| B01 | L16–L19、L32 + 动画/Editor 全部操作讲义 | FABRIK 视觉接线仍待办 |
| B02 | L05–L08 + 权威开火专题 | 枪口/弹道/弹壳表现不决定伤害 |
| B03 | L06、L15、L23 | 当前 TraceAim 与课程反投影概念分开 |
| B04 | L02、L10 | 重生不等于持久化或重连 |
| B05 | L09 | 服务器资源与换弹状态，预测是后续 |
| B06 | L22 | 晚加入从状态恢复 |
| B07 | L08–L09、L30 + weapon-handling 专题 | 三种命中模型已在源码；正式 SMG/Sniper/Grenade 等差异化资产与特效不自动视作已完成 |
| B08 | L09、L13、L30 | 当前支持 Health/Ammo/Speed，Jump/Shield 为扩展设计 |
| B09 | L07、L11–L12、L28 | Projectile/Shotgun SSR 与完整预测待扩展 |
| B10 | L24、L28、L31 + 公告专题 | 公告已有状态模型；完整离开/预测/领先者表现按证据逐项核验 |
| B11 | L29 + CTF 设计原文 | 未实现可玩 Teams/CTF |
| B12 | L25–L27 + 验证/架构原文 | 如实区分 AI 协作、Editor 操作及验证范围 |

每阶段仍按新路线要求留下：数据流、三个入口、成功/失败路径证据与面试回答。课程中的概念卡、验证记录和面试回答分别承载这些产出；无需重复填写另一份周计划。旧资料保留为历史参考，发生事实冲突时以当前源码与实际运行证据裁决。
