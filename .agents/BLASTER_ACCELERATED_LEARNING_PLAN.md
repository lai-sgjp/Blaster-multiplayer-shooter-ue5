# Blaster UE5.6 项目式速成学习计划

## 1. 目标与事实基线

- 目标：四周内完成一个可以用于实习投递的 UE5.6 C++ 多人 TPS 作品集版本。
- 学习方式：不按顺序观看课程；以可运行的垂直功能切片推进，需要背景时再读取对应讲座的 Transcript、课程提交和 UE5.6 引擎证据。
- 课程基线：课程源码最初基于 UE5.0；本项目使用 UE5.6、Enhanced Input、BuildSettingsVersion V5 和 UE5.6 Include Order。
- 演示门禁：本机 PIE，Listen Server + 1 Client。Steam 外部联机是非阻断加分项。
- 当前工程大致完成到课程第 4 节的装备、RPC、装备姿势、蹲伏、瞄准和部分 Aim Offset/Lean 内容；必须先通过基线验证，不能只依据源码存在判断完成。
- 课程最终参考提交 `b7145be5d8c73b444394c20845ac59517b009101` 对应 `Teams and Capture the Flag Maps`。

## 2. 每个工作包的固定循环

每次只激活一个工作包，并产生一个玩家可见、可独立验收的结果。

1. **课程定位**：列出对应章节、讲座/提交主题和本次只需理解的概念；Transcript 只在需要时读取，不要求看完整视频。
2. **现状核对**：读取当前 C++、资产绑定说明、Git 状态和上一任务证据，区分已实现、已编译、已 PIE 和未验证。
3. **Gate 0**：冻结唯一目标、Server/Client 所有权、数据流、允许文件、Editor 操作、失败路径和明确非目标。
4. **UE5.6 迁移检查**：先确认 UE5.0 写法在 UE5.6 的 API、模块、反射、生命周期、网络和 Editor 工作流中仍成立，再开始实现。
5. **实现与教学**：先由用户解释预测的数据流，再实现最小切片；Codex 解释关键代码和 UE 框架职责，不进行无关重构。
6. **验证**：Development Editor Build、Listen Server + 1 Client 主路径、至少一个失败路径，并检查两端日志/状态。
7. **复盘**：用户口述“谁拥有状态、谁发 RPC、谁收到复制、UI 从哪里读”；形成 2-3 道面试题。
8. **归档**：只用真实证据更新 worklog、learning journal、任务状态和作品集证据；Git 操作需单独确认。

## 3. UE5.0 到 UE5.6 固定迁移门禁

每张工程任务卡必须检查以下项目，并记录“无需变化 / 已迁移 / 尚未验证”。

- **输入**：课程的旧 Input Mapping/Action Binding 不直接照搬；保持项目现有 Enhanced Input，检查 `UInputAction`、Mapping Context、`ETriggerEvent` 和 LocalPlayer Subsystem 生命周期。
- **构建与头文件**：使用 UE5.6 Target/BuildSettings；检查显式 include、前向声明、模块依赖、`*.generated.h` 顺序和 UHT 错误。
- **对象引用**：确认 UObject/Actor/Component 引用的 `UPROPERTY`、GC 生命周期和 UE5.6 可接受的裸指针/TObjectPtr 边界，不做纯语法式批量改写。
- **网络角色**：不使用已移除的直接 `Role`/`RemoteRole` 成员写法；使用 `HasAuthority()`、`GetLocalRole()`、`GetRemoteRole()` 和 Ownership 证据。
- **复制/RPC**：核对 `GetLifetimeReplicatedProps`、`DOREPLIFETIME`、RepNotify、RPC `_Implementation`、Actor Owner 和调用方向；不因课程可运行就省略 Server 验证。
- **动画**：核对 UE5.6 Persona、AnimGraph、Montage、Aim Offset、FABRIK、Transform Bone 和动画通知的实际 Editor UI；不把预览变换误当成已保存资产数据。
- **移动/旋转**：检查 CharacterMovement 的 crouch、proxy rotation、root-bone rotation 与网络平滑，避免复制已有引擎状态。
- **UI**：Widget 只由本地 PlayerController/HUD 创建和更新；处理 Pawn/PlayerState/HUD 尚未就绪的初始化顺序。
- **在线子系统**：保持当前 SteamSockets/OnlineSubsystem 配置，课程 UE5.0 Session 写法只有在外部联机加分任务中才迁移验证。
- **弃用与兼容性证据**：无法确认的 API 必须查 UE5.6 头文件或用真实编译错误判断，不凭记忆替换。

## 4. 四周执行路线

### Week 1：完成第 4 节并打通开火

#### B00 当前基线验收（0.5-1 天）

- 覆盖：第 4 节已完成的 Weapon Class、Pickup Widget、Variable Replication、Equipping Weapons、RPC、Equipped Pose、Crouching、Aiming。
- 结果：双客户端能完成进入地图、拾取、装备、瞄准和蹲伏；明确哪些 Aim/动画内容只是资产存在而未验证。
- 失败路径：非权威客户端不能直接决定装备结果；未装备状态的蹲伏动画边界明确。

#### B01 第 4 节动画与网络收尾（1-2 天）

- 课程覆盖：Leaning and Strafing、Idle and Jumps、Aim Walking、Applying Aim Offsets、Pitch in Multiplayer、FABRIK IK、Turning in Place、Rotate Root Bone、Net Update Frequency、Crouch Unequipped。
- 结果：本地、自主代理和模拟代理的瞄准姿势、Pitch、转身、手部 IK 与蹲伏状态一致且无明显抖动。
- 必须掌握：Actor rotation 与 aim offset 的分工；local/remote pitch 映射；FABRIK 只负责表现；为什么模拟代理需要不同更新路径。

#### B02 第 5 节最小网络开火（1-2 天）

- 课程覆盖：Projectile Weapon Class、Fire Montage、Fire Weapon Effects、Fire Effects in Multiplayer、Hit Target、Spawning Projectile、Projectile Movement、Tracer、Replicating Hit Target、Projectile Hit Events、Bullet Shells/Shell Physics。
- 结果：装备武器后可连续开火，两端看到一致的 Montage、枪口/弹道/弹壳表现，Server 生成并拥有权威 Projectile。
- 失败路径：无武器、射速冷却中、客户端伪造目标时不产生权威命中。

### Week 2：瞄准、伤害、淘汰和弹药

#### B03 第 6 节 Weapon Aim Mechanics（1-2 天）

- 课程覆盖：Blaster HUD/PlayerController、Drawing Crosshairs、Crosshair Spread、Weapon Rotation、Zoom While Aiming、Shrink/Color Crosshair、Extend Trace Start、Hitting Character、Proxy Rotation、Automatic Fire。
- 结果：准星根据速度、空中、瞄准和目标类型变化；相机缩放平滑；枪口/右手朝向与屏幕瞄点一致；自动开火受 Server 规则约束。
- 必须掌握：屏幕中心反投影、camera trace 与 muzzle trace 的差异、插值只改善表现而不改变权威结果。

#### B04 第 7 节 Health and Player Stats（2 天）

- 课程覆盖：Health、HUD Health、Damage、Blaster GameMode、Elim Animation、Respawning、Dissolve、Disable Movement、Elim Bot、OnPossess、Score and Defeats。
- 结果：Server 结算伤害；Health 同步到两端；淘汰后禁用输入/碰撞并重生；PlayerState 正确记录 score/defeats。
- 失败路径：已淘汰角色不重复结算；重生 Pawn 更换后 HUD 能重新获得正确初始值。

#### B05 第 8 节 Ammo（1-2 天）

- 课程覆盖：Weapon Ammo、Can Fire、Carried Ammo、Reloading、Combat State、Allow Weapon Fire、Updating Ammo、Reload Effects、Auto Reload，以及课程中的 shotgun/reload 分支前置。
- 结果：弹匣、携带弹药、换弹状态、HUD 和自动换弹在双客户端一致；本地响应与 Server 校正边界明确。
- 失败路径：满弹匣、无携带弹药、换弹中开火、淘汰中换弹均不会污染权威状态。

### Week 3：比赛循环、武器类型和拾取

#### B06 第 9 节 Match States（2 天）

- 课程覆盖：Game Timer、Sync Client/Server Time、Match State、OnMatchStateSet、Warmup Timer/Time、Custom Match States、Cooldown Announcement、Restart Game、Blaster GameState。
- 结果：WaitingToStart → InProgress → Cooldown → Restart 的完整比赛闭环；两客户端倒计时与状态一致。
- 失败路径：晚加入客户端能通过 Server 时间和当前 MatchState 恢复正确 HUD，不从本地开始重新计时。

#### B07 第 10 节 Different Weapon Types（2-3 天）

- 课程覆盖：Hit Scan Weapons、Beam Particles、Submachine Gun、Rocket Projectile/Trail/Movement、Shotgun、Weapon Scatter、Sniper Rifle/Scope、Grenade Launcher/Projectile Grenades、Reload Animations、Shotgun Reload、Grenade Assets/Montage/Attachment/Spawn/HUD/Multiplayer。
- 结果：至少交付 Hitscan、Projectile、Shotgun 三种权威命中模型；SMG/Sniper/Grenade Launcher 作为这些模型的配置或表现实例。
- 必须掌握：三种命中模型的数据流、散布随机性由谁决定、Shotgun 多 pellet 聚合伤害、Projectile 生命周期和爆炸范围伤害。
- 时间保护：作品集硬门禁是三种模型；正式模型/音效/VFX 打磨不阻塞闭环。

#### B08 第 11 节 Pickups 与双武器（1-2 天）

- 课程覆盖：Pickup Class、Ammo/Health/Speed/Jump/Shield Pickups、Pickup Spawn Point、Default Weapon、Secondary Weapon、Swap/Drop、Weapon Swap Animation。
- 结果：Server 权威生成/消费 Pickup；Buff 生命周期可观察；默认武器和主副武器切换、掉落、死亡清理一致。
- 失败路径：重复 overlap 不重复消费；无合法副武器时不能交换；掉落武器恢复正确碰撞与拾取状态。

### Week 4：延迟补偿、多人扩展与作品集交付

#### B09 第 12 节 Lag Compensation（2-3 天，核心学习/加分实现）

- 课程覆盖：Lag Compensation Component、Hit Boxes、Frame Package/History、Rewinding Time、Frame Interpolation、Confirm Hit/Score Request、Shotgun SSR、Projectile Prediction/SSR、Local Projectile Spawn、SSR Limits、Client Prediction for Ammo/Aim/Reload、Cheating and Validation、Headshots。
- 结果：完整理解并能画出 Server-Side Rewind 数据流；至少完成 Hitscan SSR 的可验证切片，Shotgun/Projectile SSR 在时间不足时保留为明确后续任务而非遗漏。
- 失败路径：超出历史窗口、非法 hit time、非拥有者请求、重复请求和当前武器不匹配均被拒绝。
- 求职证据：记录延迟、Server 权威、公平性、内存/CPU 成本和作弊面之间的取舍。

#### B10 第 13 节 More Multiplayer Features（1-2 天）

- 课程覆盖：High Ping Warning、Local Fire Effects、Scatter Replication、Shotgun Fire RPC、Weapon Swap/Secondary Weapon 修复、Leave Game/Return Main Menu、Player Bookkeeping、Crown、Elim Announcements/Dynamic Announcements。
- 结果：高延迟提示、离开比赛清理、击杀公告和领先者表现可演示；预测表现不会重复播放。
- 失败路径：主机/客户端离开、Pawn 已销毁、HUD 尚未创建以及重复淘汰事件均能安全处理。

#### B11 Teams 与 Capture the Flag（1-2 天，加分门禁）

- 课程覆盖：Teams GameMode、Team Colors、Friendly Fire、Team Scores、Cooldown Announcement、Flag Hold/Pickup/Drop、Team PlayerStarts、CTF GameMode、Match Type Selection、Subsystem Access、Teams/CTF Maps。
- 结果：若核心闭环已稳定，交付 Team Deathmatch 与 CTF 的最小可玩规则；否则保留完整设计卡、数据流和验收矩阵，在作品集中明确标为后续。
- 失败路径：友伤规则、错误旗帜归属、重复得分、持旗者淘汰、掉线和错误出生点。

#### B12 作品集收尾（1-2 天）

- 固化一条 3-5 分钟演示：双客户端加入 → 拾取/切枪 → 三种武器 → 伤害/淘汰/重生 → 计分 → 比赛结束。
- 产出 README、架构图、网络数据流图、验证矩阵、已知限制、演示脚本、简历三条项目描述和 8-12 道面试题。
- 每个成果标注：用户独立完成、AI 协作完成、Editor 配置、已自动验证、已 PIE、未验证；不把课程复刻包装成原创系统设计。

## 5. 课程覆盖与优先级

| 课程章节 | 计划包 | 交付等级 | 不遗漏策略 |
|---|---|---|---|
| 第 4 节 The Weapon | B00-B01 | Core | 补完剩余动画、IK、代理旋转和网络更新频率 |
| 第 5 节 Firing Weapons | B02 | Core | 完整 Projectile 开火表现与权威链路 |
| 第 6 节 Weapon Aim Mechanics | B03 | Core | HUD、准星、FOV、Trace、自动开火 |
| 第 7 节 Health and Player Stats | B04 | Core | 伤害、淘汰、重生、PlayerState |
| 第 8 节 Ammo | B05 | Core | 弹药、换弹、CombatState、HUD |
| 第 9 节 Match States | B06 | Core | 时间同步、比赛状态与重启 |
| 第 10 节 Different Weapon Types | B07 | Core | Hitscan/Projectile/Shotgun 三模型；其余作为实例 |
| 第 11 节 Pickups | B08 | Core | Pickup、Buff、默认/副武器、切换与掉落 |
| 第 12 节 Lag Compensation | B09 | Portfolio+ | Hitscan SSR 必做；Shotgun/Projectile SSR 可按时间降级 |
| 第 13 节 More Multiplayer Features | B10 | Portfolio+ | 高 Ping、预测表现、离开、公告、领先者 |
| Teams/CTF 后续章节 | B11 | Optional+ | 不删除知识点；核心稳定后实现，否则保留验收设计 |

## 6. 时间失控时的裁剪顺序

四周是交付时间盒，不是删除知识点的理由。若实际进度落后，按以下顺序裁剪实现深度，同时保留课程覆盖卡和复盘：

1. 先裁正式美术、音效、粒子和地图打磨。
2. 再裁 Teams/CTF 的实际实现，但保留 GameMode/计分/Flag 权威设计。
3. 再将 Projectile/Shotgun SSR 降为设计与源码复盘，保留 Hitscan SSR 实现。
4. 不裁射击、Health、Ammo、淘汰/重生、MatchState、三种武器模型和双客户端验证。

## 7. Transcript 与课程源码使用规则

- 每张任务卡只读取与当前切片直接相关的讲座 Transcript，不要求提前消费整门课。
- Transcript 用于提取老师的设计动机、实现顺序、Editor 操作和常见坑；课程提交用于确认真实文件变化；UE5.6 源码/编译用于裁决版本差异。
- 三者冲突时优先级为：本项目真实状态与 UE5.6 证据 > 课程最终提交 > UE5.0 Transcript 的具体 API 写法。
- 当前 Codex Browser Plugin 服务文件缺失，尚不能从已登录 Udemy 面板自动读取 Transcript；在插件恢复前，不得声称已经读取讲稿。公开提交历史已经用于建立本计划的主题覆盖矩阵。

## 8. 完成定义

- Core 工作包 B00-B08 均有 Build、双客户端 PIE 主路径和失败路径证据。
- B09 至少完成 Hitscan SSR 或形成有证据的延期说明；B10 的离开/公告等多人表现至少完成一条可演示链路。
- 能独立解释一次完整数据流：输入 → 本地响应 → Server RPC → Server 验证 → 权威状态 → Replication/OnRep/Multicast → 本地 HUD/表现。
- 可从干净启动稳定录制演示，不依赖临时 Editor 状态或未保存资产。
- README、架构图、验证矩阵、演示脚本、简历描述和面试复盘全部与真实完成状态一致。
