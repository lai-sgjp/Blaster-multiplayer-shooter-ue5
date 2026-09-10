"""Editable course syllabus. Run this, then build_content.py after updating lessons."""
import json
from pathlib import Path

C = 'Source/Blaster/BlasterComponent/CombatComponent.cpp'
A = 'Source/Blaster/Character/BlasterAnimInstance.cpp'
CH = 'Source/Blaster/Character/BlasterCharacter.cpp'
GM = 'Source/Blaster/Character/BlasterGameMode.cpp'
GS = 'Source/Blaster/Character/BlasterGameState.cpp'
PS = 'Source/Blaster/Character/BlasterPlayerState.cpp'
W = 'Source/Blaster/Weapon/Weapon.cpp'
L = 'Source/Blaster/BlasterComponent/LagCompensationComponent.cpp'
S = 'Plugins/MultiplayerSessions/Source/MultiplayerSessions/Private/MultiplayerSessionsSubsystem.cpp'
M = 'Plugins/MultiplayerSessions/Source/MultiplayerSessions/Private/Menu.cpp'
HUD = 'Source/Blaster/HUD/BlasterHUD.cpp'
D = ['learn/cpp/01-ue5-cpp-and-gameplay-framework.md', 'learn/networking/01-authority-replication-rpc.md', 'learn/gameplay/01-shooter-combat-pipeline.md', 'learn/animation/02-animation-data-pipeline.md', 'learn/online/01-steam-session-and-travel.md', 'learn/tooling/01-build-config-and-validation.md', 'learn/networking/B11-teams-ctf-design.md']
lessons = []
def add(stage, title, tags, idea, project, flow, question, answer, lab, refs, docs=(), boundary='源码可追踪；运行结果请在验证记录中填写，不由阅读状态推断。'):
    base_docs = [D[stage]] if stage < 6 or 'CTF' in tags else []
    lessons.append(dict(id=f'L{len(lessons)+1:02}', stage=stage, title=title, tags=tags, idea=idea, project=project, flow=flow.split(' → '), question=question, answer=answer, lab=lab, refs=refs, docs=list(dict.fromkeys(base_docs + list(docs))), boundary=boundary, minutes=35))

add(0,'从 HSR 迁移：先找职责，再套知识','HSR GAS Component B00',
'把一局比赛想成球赛：裁判决定规则，记分牌公布比分，球员档案记录个人成绩，场上的身体负责移动。先确定谁负责什么，比把所有类名背下来更有用。',
'在 Blaster 中，GameMode 管规则，GameState 管共享比赛数据，PlayerState 管玩家统计，Character 与 CombatComponent 管当前身体和战斗。HSR 的 GAS/事务管道是理解复杂系统的背景，但这里的一枪由 RPC 和 ApplyDamage 串起来，没有 ASC/Ability 的主线。',
'玩家意图 → CombatComponent → GameMode 规则 → GameState / PlayerState → 本地 HUD',
'为什么没有把 Blaster 的战斗全部改成 GAS？',
'当前需求用 CombatComponent、Weapon 和服务器伤害链路已经能表达。GAS 能提供 Ability、Effect、Tag 和预测体系，但也引入新的组织成本。我先解释现有权威边界；需要复杂技能组合时才评估迁移，不能把 HSR 中学过 GAS 说成本项目已经使用 GAS。',
'画四个框，把 Health、Defeats、PhaseDeadline、EquippedWeapon 分配进去。再假设 Pawn 被销毁，圈出应保留的玩家数据。',[(GM,'ABlasterGameMode::'),(C,'ServerFire_Implementation')],['learn/INTERVIEW-LEARNING-ROADMAP.md'])
add(1,'身体会重生，玩家档案不会随它消失','Framework Character Actor Component Controller PlayerState B04',
'角色像你在游戏里控制的一具身体；控制器像驾驶员，PlayerState 像比赛档案。身体倒下换一具，死亡次数不应跟着归零。组件则像装在身体上的战斗模块，承担一组相关能力。',
'ABlasterPlayerState::AddDefeat 只在 Authority 上增加 Defeats，并注册复制。Health 属于 Character 当前生命；Score 和 Defeats 属于玩家统计。GameMode 是服务器规则入口，GameState 是客户端也能读取的比赛信息。',
'Server 处理淘汰 → PlayerState 增加 Defeats → 重生新 Pawn → HUD 读取新 Pawn 与原玩家统计',
'Score 为什么不放在 Character？',
'Character 随淘汰和重生被替换，而 PlayerState 的职责是玩家级统计，适合跨 Pawn 保存并让其他客户端读取。但跨地图、退出重连和永久存档是不同问题，不能由跨 Pawn 保留直接推导。',
'在双端 PIE 中记录死亡前、重生后的 Pawn 名称和 Defeats。预期 Pawn 改变，死亡数保留；新一轮重置规则另看 ResetRoundStats。',[(PS,'AddDefeat'),(GM,'RespawnPlayer')])
add(1,'Authority、Local 和 Owner 是三道不同的问题','Authority Ownership Autonomous Proxy Simulated Proxy B00',
'Authority 问“谁能做最终裁决”；Local 问“是不是这台机器正在操控”；Owner 问“这个对象沿哪条连接归属链路关联玩家”。房主机器上的本地角色可以同时是 Authority 和本地控制，不能把它们当互斥标签。',
'FireOnce 要求 IsLocallyControlled，ServerFire 再检查 HasAuthority、武器 Owner 和当前装备。Autonomous Proxy 通常是拥有客户端上的受控 Pawn，其他客户端看到的是 Simulated Proxy；这些不是玩家账号身份。',
'拥有客户端输入 → 所有权允许的 Server RPC → Server 检查请求 → 其他客户端消费复制',
'有 Owner 就代表客户端可以修改权威弹药吗？',
'不代表。Owner 参与 RPC 路由和条件复制，Authority 才表示权威副本。即使 RPC 到达服务器，仍要检查武器、角色状态、弹药、时间和射速。Reliable 保证的也不是参数可信。',
'画 Listen Server 本地玩家、客户端自己的玩家、客户端看到的别人三列，分别填写 Authority、Local、Role；再定位 ServerFire 的 Owner 检查。',[(C,'FireOnce'),(C,'EquippedWeapon->GetOwner()')])
add(1,'复制状态：从 Replicated 到本地表现','RepNotify OnRep DOREPLIFETIME COND_OwnerOnly B00 B05',
'复制像服务器更新共享账本，客户端稍后收到最新值；OnRep 像“收到变化后更新外观”的通知。它不是每一次历史变化的可靠事件队列，也不要假设多个属性总按你期望的顺序到达。',
'Weapon 注册 WeaponState 和 Ammo；OnRep_WeaponState 切换碰撞和拾取提示。SetWeaponState 在本地显式调用 OnRep_WeaponState，以复用服务器侧表现更新。Character 的拾取候选用 COND_OwnerOnly，避免向所有玩家显示别人的拾取提示。',
'Server SetWeaponState → 属性复制 → 客户端 OnRep_WeaponState → 碰撞 / 提示变化',
'只写 UPROPERTY(Replicated) 就够了吗？',
'还要检查 Actor/组件复制是否开启、GetLifetimeReplicatedProps 是否注册，以及相关性和条件。RepNotify 处理收到数据后的本地反应；C++ 服务器赋值不会自动等价于客户端收到复制，所以本项目显式复用状态处理。',
'在 Weapon.cpp 找到三处：注册、写入、消费。再在 Character 中搜索 COND_OwnerOnly，说明为什么其他客户端不需要该值。',[(W,'GetLifetimeReplicatedProps'),(W,'SetWeaponState'),(CH,'COND_OwnerOnly')])
add(1,'RPC 是请求通道，不是可信证明','Server Client NetMulticast Reliable B02',
'把 Server RPC 想成向裁判递交申请，Client RPC 是给指定玩家的通知，Multicast 是面向相关副本的广播。申请送到了，不等于申请内容合法；广播也不替代持久状态。',
'ServerFire 负责裁决；ClientApplyRecoil 负责拥有端后坐力；MulticastFire 负责枪口和弹道等表现。持久 Ammo 通过属性复制。晚加入者需要当前状态，不能依赖过去已经播完的开火广播。',
'ServerFire 校验 → 权威结算 → ClientApplyRecoil → MulticastFire → Ammo 属性同步',
'为什么不在 Multicast 中对命中角色扣血？',
'因为每个副本都会执行表现事件，若让它决定伤害就容易多次结算或产生分歧。服务器统一决定伤害，客户端只呈现结果。组件 RPC 还依赖所属 Actor 的复制和所有权链路。',
'给 ServerFire、ClientApplyRecoil、MulticastFire 分别标出接收者；阅读头文件中的 UFUNCTION 声明，确认方向与可靠性。',[(C,'ServerFire_Implementation'),(C,'MulticastFire_Implementation'),('Source/Blaster/BlasterComponent/CombatComponent.h','UFUNCTION')])
add(2,'一枪的入口：本地计时与服务器射速','FireOnce Timer TraceAim B02 B03',
'本地冷却让按住鼠标不会疯狂发请求；服务器冷却才防止修改客户端后超速射击。前者改善使用体验和流量，后者保护规则。',
'FireOnce 检查本地控制、弹药、换弹和比赛阶段，使用 NextLocalFireTime。TraceAim 实际从相机方向向前做 Trace，并把起点推过相机到身体的距离；它不是“直接照抄课程反投影代码”。服务器另从 Muzzle 检查发射路径。',
'输入按下 → Fire Timer → FireOnce → TraceAim → ServerFire → NextServerFireTime',
'为什么客户端和服务器都要做射速限制？',
'本地限制减少无效请求，服务器限制保证规则。当前服务器用 Max(当前时间, 上次截止时间) 再加 FireInterval 更新截止，允许小幅到达抖动但不让长期射速变快。',
'从 FireOnce 追 NextLocalFireTime，再看 ServerFire 的 0.02 容差和 NextServerFireTime 更新。说明容差为什么不能每发都变成免费加速。',[(C,'FireOnce'),(C,'TraceAim')])
add(2,'逐关检查：ServerFire 的真实验证顺序','ShotId ShotTime RequestedWeapon anti-cheat B02 B09',
'服务器像机场安检：先判断能不能进入流程，再逐项检查身份、时间、装备和路径。跳过任何一层都可能让不合法的意图进入结算。',
'本项目顺序是比赛阶段 → ShotId 单调性并立即记录 → 有限且合法的 ShotTime / 当前武器 → 角色权威、存活、Controller、Owner、弹药、换弹、目标数值、射速 → Muzzle 与附着 → 距离/朝向 → 视点到枪口遮挡 → 命中模型。注意：后续校验失败的 ShotId 也已被消耗。',
'比赛 / ShotId → 时间 / 武器 → 角色 / 弹药 / 射速 → Socket / 朝向 / 遮挡 → 结算',
'重复 ShotId 和过期 ShotTime 是在哪里拒绝的？',
'在 ServerFire_Implementation 的前部。先拒绝非递增 ShotId，然后记录该 ID；再检查时间是否有限、未来是否超过 0.05 秒、过去是否超过 AllowedAge。AllowedAge 为 min(1 秒, 0.2 秒 + Ping 秒)。这限制请求窗口，但不是完整反作弊系统。',
'对照源码逐行写拒绝条件。使用项目已有测试入口或断点观察非法请求；记录重复 ID、旧时间、错误武器分别在何处返回，不能仅把读代码写成运行通过。',[(C,'ServerFire_Implementation')])
add(2,'三种命中模型：子弹、射线与霰弹','Projectile Hitscan Shotgun scatter B07',
'Projectile 像真的扔出一个球，之后碰到东西才结算；Hitscan 像瞬间拉一根线；Shotgun 像一次拉多根散开的线。它们共用开火权限，但命中的时间和判定方法不同。',
'ServerFire 的 Projectile 分支在 Server SpawnActor；Hitscan 调 ConfirmHit 查询历史盒；Shotgun 在 Server 生成 8 条散布方向并用 DamageByActor 聚合伤害，再广播 PelletEnds 供表现。散布不是每台客户端各抽一次。',
'Server 校验 → Projectile / Hitscan / Shotgun → ApplyDamage → SpendRound → 表现',
'霰弹为什么要先按 Actor 聚合伤害？',
'同一目标可能被多颗 pellet 命中。先汇总再提交该目标的伤害，便于保持一次开火的结算语义。当前 Shotgun 走当前世界 Trace，没有使用 Hitscan 的历史回溯路径。',
'用同一张表列三种模型的生成端、命中时刻、是否回溯。找到 PelletEnds 与 DamageByActor，区分表现数据和权威伤害数据。',[(C,'EFireModel::Projectile'),('Source/Blaster/Weapon/BlasterProjectile.cpp','OnHit')],['learn/networking/weapon-handling.md'])
add(2,'弹药、换弹和切枪是一组受约束的状态','Reload Ammo Swap Drop B05 B08',
'换弹像把背包里的子弹搬进弹匣，不是凭空补满。切枪时正在换的那把武器可能已不是当前武器，所以需要状态约束和计时器清理。',
'Weapon::SpendRound 在 Authority 扣弹，AddAmmo 限制不超过容量。CombatComponent 管换弹与主副武器交换；Weapon::Drop 清 Owner、解除附着并恢复拾取碰撞。每一项都影响下一次 ServerFire 是否有效。',
'换弹请求 → ServerReload → 延时完成 / 状态校验 → Ammo 与 CarriedAmmo → 本地 HUD',
'为什么不能在换弹动画结束通知中无条件加满弹药？',
'动画是表现，可能被打断或受帧率影响。权威资源转移应由服务器流程和合法状态决定，实际加弹量受缺口和携带量限制。通知可以协助时序，但不能代替服务器校验。',
'追 ServerReload 与完成函数，计算缺口 10、背包只有 4 时应加多少。再核对换弹中切枪和淘汰会怎样清理。',[(C,'ServerReload_Implementation'),(C,'ServerSwapWeapons_Implementation'),(W,'AddAmmo')],['learn/networking/B03-B05-combat-loop.md'])
add(2,'从伤害到重生：把完整闭环讲出来','ApplyDamage ReceiveDamage Eliminate Respawn B04',
'命中只是开头。扣血、判定死亡、只记一次分、清理旧身体、生成新身体、界面重新取数，任何一环缺失都会出现“打中了但游戏不对”的问题。',
'开火模型调用 ApplyDamage，Character 的 ReceiveDamage 进入生命/淘汰逻辑，GameMode 负责计分与 RespawnPlayer，PlayerState 保留统计。HUD 必须读取当前 Pawn，不能永久引用旧尸体。',
'ApplyDamage → ReceiveDamage → 淘汰保护 → GameMode 计分 → RespawnPlayer → 新 Pawn / HUD',
'怎样避免一个死亡重复记两次分？',
'先由服务器维护淘汰状态，再让后续伤害和淘汰请求受状态保护。计分必须收敛到服务器的单次淘汰路径，不能让多个客户端表现回调分别加分。具体保护需结合 ReceiveDamage 与 GameMode 路径说明。',
'在双端观察击杀一次后的 Score/Defeats，再对已淘汰对象触发伤害边界。记录重生后 Health 初值和 HUD 所读 Pawn。',[(CH,'ReceiveDamage'),(GM,'RespawnPlayer')])
add(2,'回溯保存什么：历史帧与插值','SSR history SampleFrame interpolation B09',
'网络消息到达时，对手已经走开。回溯像查看过去的一张照片；但照片只隔一段时间拍一次，所以要用前后两张估算射击时刻的位置。',
'LagCompensation 在 Server 的 PostPhysics 以目标 30Hz 采样 7 个骨骼盒变换，最多 34 帧且裁掉超出约 1 秒的历史。SampleFrame 用前后帧的时间比例 Blend 变换。30Hz 是 TickInterval 目标，不保证卡顿时仍精确采样。',
'Server 骨骼姿态 → FFrame 历史 → ShotTime → 两帧 Alpha → 插值盒',
'为什么不是直接挑距离最近的一帧？',
'最近帧会带来离散跳变；插值利用前后帧估算中间状态，减少采样间隔导致的误差。但它仍是近似，不能恢复未记录的运动细节，也不能无限增大时间窗口。',
'设旧帧 10.0 秒、新帧 10.1 秒、射击 10.025 秒，手算 Alpha=0.25。再用下方互动实验改变时间，观察窗口拒绝和盒位置。',[(L,'TickComponent'),(L,'SampleFrame')],['learn/networking/B09-hitscan-rewind.md'])
add(2,'回溯怎样命中：历史盒求交与遮挡','ConfirmHit bone-space world obstruction B09',
'可以把子弹线段换算到一个历史盒自己的坐标系里，再判断线是否穿盒。像拿着尺子量一只旋转的箱子，先站到箱子自己的方向上，计算会更简单。',
'ConfirmHit 不移动真实角色：先对当前世界做遮挡 Trace，把最近遮挡当上限，再遍历历史骨骼变换，用 InverseTransformPosition 把射线两端转到盒局部坐标，做 LineExtentBoxIntersection，保留遮挡前最先命中的盒。',
'当前世界遮挡 → SampleFrame → 射线转历史盒局部空间 → 求交 → 最近 Victim / Headshot',
'为什么你的回溯没有把真实角色移回过去？',
'我只查询历史变换上的数学盒，避免改动物理世界再恢复的副作用。边界是使用当前世界遮挡、固定骨骼盒近似以及有限时间窗口，不是把整张地图和所有物体完整回放。',
'画出枪口、墙、历史角色三者顺序。若墙更近，角色交点应被拒绝。指出 BestTime 初值及 HitTime 比较如何实现。',[(L,'ConfirmHit')],['learn/networking/B09-hitscan-rewind.md'])
add(2,'Pickup 与 Buff：只消费一次，效果会结束','Pickup Buff Speed Jump Shield B08',
'两个人同时抢一个补给，就像两个人同时兑换同一张券，必须由一个权威地方决定谁成功。加速还需要结束时间，否则临时效果会变成永久属性。',
'BlasterPickup 当前只支持 Health、Ammo、Speed。OnOverlap 先检查 Authority、bConsumed、存活和比赛阶段，效果真正应用成功后才标记消费、隐藏并关闭碰撞，30 秒后 Reactivate。EndPlay 清理返回计时器。它是运行时补给，不是 HSR 的持久化装备实例。',
'Server 生成 → Overlap 合法性 → 消费保护 → 应用效果 → 到期 / 重生清理',
'为什么 Pickup 的 overlap 不能直接让客户端加血？',
'客户端可以伪造或重复触发请求。服务器必须核对角色、距离/重叠、状态和补给是否可用，并让成功消费只发生一次。客户端仅观察复制后的效果。',
'打开补给专题定位实际类和支持类型，观察两人同抢与满血/满弹边界；记录 Buff 到期后数值是否恢复。',[('Source/Blaster/Pickups/BlasterPickup.cpp','OnOverlap'),(CH,'ApplySpeedBuff')],['learn/networking/B06-B08-match-and-pickups.md'])
add(3,'Enhanced Input：按键如何变成开火意图','Input Action Mapping Context Trigger BindAction B00',
'Input Action 是“想做什么”，Mapping Context 是“哪些键表示这个意图”，Trigger 是“什么时候算触发”。把意图与键分开，就能改变键位而不改战斗代码。',
'BlasterCharacter::SetupPlayerInputComponent 把资产绑定到 C++ 回调。Mapping Context 加在本地 LocalPlayer 子系统上，远端模拟角色不应替别人读取键盘。按下、持续、释放的事件组合还影响自动开火是否停止。',
'本地 Mapping Context → Input Action / Trigger → BindAction → Character 回调 → CombatComponent',
'按键没有反应，你从哪开始查？',
'先确认当前本地 Pawn 被正确 Possess、输入模式允许游戏输入，再检查 Mapping Context 是否加入、Action 资产是否绑定、Trigger 事件是否匹配，最后看回调是否进入战斗逻辑。不能一开始就认定 RPC 坏了。',
'在 SetupPlayerInputComponent 找移动、瞄准、开火、换弹、切枪的绑定，分别写回调名。释放开火键后检查 Timer 是否还在触发。',[(CH,'SetupPlayerInputComponent')])
add(3,'瞄准、FOV 与准星：手感和规则分开','Aim FOV Crosshair Trace B03',
'缩小 FOV 像用望远镜，看得更近；准星只是告诉你大致瞄到哪里。它们是反馈，不能因此替服务器决定伤害。',
'CombatComponent 维护 bAiming 与本地 FOV 更新，AnimInstance 读取 Character 的 IsAiming；TraceAim 提供相机目标，ServerFire 仍从枪口检查距离、方向和障碍。相机看得到不代表枪口射得过去。',
'IA_Aim → Combat bAiming → 本地 FOV → 状态复制 → AnimInstance / HUD',
'相机能看见敌人，为什么子弹不能穿墙？',
'第三人称相机与枪口位置不同。客户端相机目标只代表意图，服务器会检查视点到枪口遮挡，并从枪口做命中计算，避免隔着掩体射击。',
'站在掩体旁比较相机与枪口位置；记录准星命中但枪口遮挡时服务器行为。追 bAiming 的赋值、复制注册和动画读取。',[(C,'bAiming'),(C,'TraceAim'),(A,'bAiming')])
add(3,'动画读什么：Speed、Air、Yaw Offset 与 Lean','NativeInitializeAnimation NativeUpdateAnimation Blend Space B01',
'动画系统像演员：它根据身体实际在做什么来演。Speed 是水平移动快慢，Air 表示是否在空中，YawOffset 表示移动方向与瞄准方向的差，Lean 是转向速度带来的侧倾。',
'NativeInitializeAnimation 获取 Pawn；NativeUpdateAnimation 在 Pawn 无效时重取，去掉速度 Z 得到 Speed，从 Movement 读 IsFalling。YawOffset 用 NormalizedDeltaRotator；Lean 用每帧旋转差除 DeltaSeconds，再限幅和平滑。',
'CharacterMovement / 复制姿态 → AnimInstance 参数 → Blend Space / AnimGraph → 最终骨骼姿势',
'为什么 Lean 要除以 DeltaSeconds？',
'同样一帧转 3 度，在 30FPS 和 120FPS 下代表不同角速度。除以时间得到每秒转速，再插值与限幅，才不把帧率直接变成倾斜程度。',
'手算 0.02 秒转 1 度得到每秒 50 度；在源码标记限幅和平滑。解释为什么远端角色也能从自己的复制副本计算这些参数。',[(A,'NativeInitializeAnimation'),(A,'NativeUpdateAnimation')])
add(3,'远端 Pitch 与原地转身：不要双重旋转','Pitch Turning in Place Rotate Root Bone B01',
'看向上方是瞄准角，整个人转向是 Actor 朝向，脚还没转过来时的姿态补偿是根骨偏移。这三件事若都去旋转同一个身体，就会叠加过头。',
'远端 AO_Pitch 大于 90 时，将 270–360 映射到 -90–0。UpdateTurning 累积 Actor Yaw 的反向补偿，约 89.5 度触发转身，并减去 B01TurnYaw 曲线已烘焙的转角，避免动画自身转向再叠加一遍。',
'BaseAimRotation → Pitch 映射 → Aim Offset；ActorYaw → RootYawOffset → 转身曲线 → Rotate Root Bone',
'为什么远端向下瞄准可能出现异常？',
'远端 Pitch 可能表示成 270–360 度，而 Aim Offset 期待负角度。需要转换到资产使用的区间。转身还应区分 Actor 旋转与骨骼补偿，并检查本地、自主代理和模拟代理。',
'输入 315 度，换算得到 -45 度。再追 RootYawOffset 减去 AnimatedYaw 的代码，解释若不减会发生什么。',[(A,'AO_Pitch'),(A,'UpdateTurning')],['learn/animation/B01-turning-in-place.md','learn/editor/B01-aim-offset-editor-setup.md'])
add(3,'左手 IK：从枪上一个点到一整条手臂','Socket TransformToBoneSpace FABRIK B01',
'先规定左手应抓住枪上的哪个点，再让手臂各关节配合伸过去。Socket 给出目标位置，空间转换让双方使用同一把尺子，FABRIK 才负责解关节。',
'每个网络副本从已复制并附着的武器计算 LeftHandSocket 世界变换，转换到 hand_r 骨骼空间，写 LeftHandTransform。缺少 Socket、骨骼或附着关系时 bLeftHandIKValid 为 false，避免左手伸到原点。这里不是直接复制 LeftHandTransform。',
'复制武器附着 → LeftHandSocket 世界位置 → TransformToBoneSpace(hand_r) → LeftHandTransform → AnimGraph FABRIK',
'为什么有 C++ 数据还不能说 FABRIK 已完成？',
'C++ 只生产目标数据，AnimGraph 必须把该数据接到 FABRIK 并配置空间、骨骼链和 Alpha；资产还要保存并进行双端视觉检查。当前资料记录最终接线和视觉打磨仍有待办。',
'按附带 Editor 操作打开 ABP_Blaster，核对 hand_r 空间、upperarm_l 到 hand_l 的链和有效性 Alpha。保存后观察两端，记录实际结果。',[(A,'LeftHandSocketName'),(A,'TransformToBoneSpace')],['learn/editor/B01-fabrik-animgraph-setup.md','learn/editor/B01-fabrik-socket-setup.md'], '源码数据链可核对；FABRIK 最终资产接线和完整视觉验收仍是待办，学习打勾不代表实现完成。')
add(3,'换角色骨架：Retarget 与资产保存边界','IK Retargeting Skeleton Persona B01',
'同一套动作换到不同比例的人身上，手脚长度和骨骼命名可能不同。Retarget 是把动作意图映射到另一副骨架；它不会自动保证枪口、握点和曲线全都正确。',
'AnimInstance 显式检查 hand_r、hand_l、upperarm_l。转身依赖 B01TurnYaw 曲线，IK 依赖武器 Socket。替换第三方角色时，要逐项核对这些契约，不能只看动画预览能播放。',
'源 Skeleton / IK Rig → Retarget 映射与姿态 → 目标动画 → 曲线 / Socket / AnimGraph → 双端观察',
'动画看起来能播，为什么换角色后左手仍错位？',
'播放成功只证明动作资源能驱动骨架；手部约束还依赖骨骼链、比例、参考姿态、Socket 和空间配置。要先确认数据有效，再检查求解器和保存后的资产。',
'列出目标骨架必须存在的三个手臂骨骼、LeftHandSocket 和 B01TurnYaw。逐项记录检查结果，预览变换与保存的 Key 要分开记录。',[(A,'GetBoneIndex'),(A,'B01TurnYaw')],['learn/editor/B01-turn-animation-preview.md'])
add(4,'Session：房间目录与异步回调','GameInstanceSubsystem Create Find Join Destroy Delegate',
'Session 像房间目录：先登记房间，别人搜索到后再加入。请求提交和完成是两个时刻，不能按同步函数的直觉，调用完就默认成功。',
'MultiplayerSessionsSubsystem 跨地图管理 SessionInterface 和回调句柄。CreateSession 遇到已有房间先销毁再重建；Find 搜索结果由 Menu 筛 MatchType。每个成功或失败出口都要关注 Delegate 清理和按钮恢复。',
'菜单请求 → Subsystem 注册 Delegate → 在线接口异步操作 → 回调 / 清句柄 → UI 恢复或 Travel',
'为什么不用菜单 Widget 自己长期保存 Session 状态？',
'菜单会随地图和界面销毁，而 GameInstanceSubsystem 跨地图更稳定，适合持有会话接口与异步过程。Widget 负责交互，Subsystem 负责在线流程；仍要处理销毁后回调和失败路径。',
'画 Create / Find / Join / Destroy 四条回调线，检查对应句柄何时清除。单独记录“请求返回 false”和“异步回调失败”。',[(S,'CreateSession'),(S,'FindSessions'),(S,'JoinSession'),(S,'DestroySession')])
add(4,'Travel：从房间目录走进真实地图','ServerTravel ClientTravel SeamlessTravel Lobby PostLogin',
'找到酒店房间不代表你已走进去。Session 帮你拿到连接信息，Travel 才切换世界；进入同一个世界以后，Replication 才同步角色。',
'Menu 的创建成功回调 ServerTravel 到 Lobby；加入回调解析连接地址后 ClientTravel。LobbyGameMode::PostLogin 在服务器满足人数条件后启动到比赛地图的无缝旅行。',
'Create / Join 成功 → ServerTravel / ClientTravel → Lobby PostLogin → SeamlessTravel → 比赛 GameMode',
'Session 和 Gameplay Replication 是什么关系？',
'前者解决发现和加入房间，Travel 解决进入目标世界，Replication 解决世界内 Actor 状态同步。Session 成功并不证明战斗复制正常，更不证明跨机器公网路径已经验收。',
'分别画 Host 与 Join 两条线，并标记哪端执行每次 Travel。打开配置资料核对地图链，不改现有在线配置。',[(M,'ServerTravel'),(M,'ClientTravel'),('Source/Blaster/GameModes/LobbyGameMode.cpp','PostLogin')])
add(4,'比赛阶段与晚加入：同步截止时间','MatchState PhaseDeadline ServerTime B06',
'如果每个人进房间都各开一个 60 秒秒表，晚来的人时间一定不同。同步同一个结束时刻，再用服务器时间相减，晚加入者才能立刻显示正确剩余时间。',
'GameMode 推进规则；GameState 复制 PhaseDeadline，HUD 用服务器时间计算剩余时长。阶段和截止时间是可恢复状态，不要求晚加入者听到开局那一刻的广播。',
'GameMode 设置阶段 → GameState PhaseDeadline → 复制 → HUD 截止时间减 ServerTime',
'为什么不用客户端 Tick 每秒减一？',
'本地倒数会受加入时刻、卡顿和计时漂移影响。共同截止时间加服务器时间基准更适合恢复；客户端只是显示结果，阶段转换仍由服务器决定。',
'让第二个客户端晚加入，比较阶段和剩余时间；随后检查 Cooldown 到下一轮的统计重置。',[(GS,'SetPhaseDeadline'),(GM,'Tick'),(HUD,'PhaseDeadline')],['learn/networking/B06-B08-match-and-pickups.md'])
add(4,'HUD 与 UMG：显示复制的数据','DrawHUD WidgetComponent UUserWidget B03 B10',
'记分牌不能自己改裁判的判分。UI 只把当前本地已收到的状态画出来；同一状态可以用 Canvas，也可以用 Widget 显示。',
'BlasterHUD 用 DrawHUD/Canvas 显示战斗信息；OverheadWidget 和 Menu 使用 UMG。HUD 从当前 Pawn、PlayerState、GameState 取值，重生后应重新找到新 Pawn。服务器没有必要复制 Widget 实例。',
'Server 状态 → 复制到本地对象 → HUD 读取当前 Pawn / PS / GS → Canvas / Widget',
'血量不同步时，是先改 Widget 吗？',
'先看服务器 Health 是否正确，再检查属性是否到客户端，最后看 UI 是否读了当前有效对象。若客户端数据已正确而界面不对，才定位绑定和生命周期；层次分开能避免乱改。',
'把 HUD 每个显示项分到 Character、Combat、Weapon、PlayerState、GameState。重生时记录 HUD 是否仍引用旧 Pawn。',[(HUD,'DrawHUD'),('Source/Blaster/HUD/OverheadWidget.cpp','ShowPlayerNetRole')],['learn/ui/01-hud-and-umg-data-flow.md'])
add(4,'公告、Ping 与离开：一次性事件的寿命','Announcements High Ping Leave B10',
'公告像短暂亮起的电子屏，必须知道显示内容和过期时间。网络延迟提示是体验诊断，不是“延迟高就允许随便命中”的通行证。',
'GameState 保存 EliminationMessage 和 AnnouncementDeadline，消息最长 160 字符、显示期为 5 秒。离开与 Pawn 销毁时，UI、计时器和 Session 都有各自的清理责任；具体已验收范围看验证矩阵。',
'Server 淘汰 → GameState 消息 / 截止时间 → 客户端公告 → 到期隐藏；离开 → 清理',
'为什么公告需要截止时间而不只复制字符串？',
'晚收到或晚加入的客户端需要判断消息是否还有效。截止时间让客户端依据服务器时间作相同判断。但单个字符串不是完整事件日志，高频公告可能覆盖前一条。',
'连续发生两次淘汰并晚加入，观察公告是否覆盖、过期是否消失。分别记录主机离开和客户端离开，未测的路径保持待验证。',[(GS,'AnnounceElimination'),(HUD,'DrawHUD')],['learn/networking/B10-announcements.md'])
add(5,'UHT、UBT、Build.cs 和两个 Target','Modules reflection GENERATED_BODY B00 B12',
'UHT 像处理 UE 反射声明的代码生成环节；UBT 组织整个构建。Build.cs 说明一个模块依赖什么，Target.cs 说明这次构建什么程序。它们不是同一种配置。',
'Blaster 的 Build.cs 声明模块依赖，Game 和 Editor 有各自 Target。公共头文件暴露的依赖和只在实现里使用的依赖要区别考虑。Editor 模块能编过，不代表非 Editor 游戏目标也没有依赖问题。',
'uproject / Target / Build.cs → UHT 生成反射支持 → 编译 / 链接 → Editor 或 Game 二进制',
'为什么要同时验证 Game Target？',
'Editor 环境可能掩盖对编辑器模块的依赖。Game Target 验证游戏目标的编译链接，但仍不能证明 Cook/Stage、资源收集、打包启动和网络路径。',
'打开三个构建文件，对照 Type 与依赖列表。按构建文档记录 Editor/Game 各自命令、退出码和日志路径。', [('Source/Blaster/Blaster.Build.cs','PublicDependencyModuleNames'),('Source/Blaster.Target.cs','TargetType'),('Source/BlasterEditor.Target.cs','TargetType')])
add(5,'验证矩阵：让每一句项目介绍都有证据','Build PIE NULL Steam Dedicated Cook B12',
'“车能发动”和“车通过长途路测”不是同一个结论。同样，文件存在、编译成功、双端 PIE、公网 Steam 和发布包运行分别证明不同的事。',
'项目验证文档区分了本地构建/PIE、资产待办、DX12 稳定性和发布网络边界。Steam 配置存在不能证明公网已通，Content 被 Git 忽略还意味着代码仓库未必能单独还原全部资产。',
'源码 / 资产检查 → Editor 与 Game Build → 双端 PIE → 失败 / 延迟路径 → 独立发布与网络验证',
'你怎么证明这个功能完成了？',
'我先限定场景，再给入口、预期、日志和观察结果。例如双端 PIE 验证权威扣弹和重生，另测非法请求；公网 Steam、Dedicated Server 和 Cooked Build 单独列状态，不扩大本地结果。',
'给射击、FABRIK、Steam 各写证据等级和未测项，附日志路径。不要将本网站的学习勾选当成游戏验收。',[(GM,'RespawnPlayer')],['docs/VERIFICATION.md','docs/ARCHITECTURE.md'])
add(5,'三分钟项目介绍：讲一条你真正懂的链路','Interview portfolio B12',
'好的项目介绍像带人沿路线走一次：先说解决什么问题，再讲关键设计，然后拿一个具体例子证明，最后明确边界。堆技术名词容易被追问卡住。',
'用“一枪”串起 Character 输入、Combat 校验、三种命中模型、伤害/重生和 HUD；再以历史盒数学回溯为深入点。课程基础、AI 协作、自己配置和实测的部分要如实区分。',
'项目目标 → 架构职责 → 一枪流程 → 回溯取舍 → 验证证据 / 限制',
'请介绍你的 Blaster 项目。',
'这是 UE5.6 C++ 多人 TPS 学习项目。玩法用 Character、CombatComponent 和 Weapon 拆分，关键状态由服务器裁决；客户端提交射击意图，服务器检查时间、序号、武器和路径，再选择命中模型。Hitscan 用有限历史骨骼盒求交而非移动真实角色。验证范围按本地构建和双端 PIE 记录说明；完整发布和公网等未验收项单独列出。我会明确课程基础及 AI 协作范围。',
'写自己的 3 分钟版本，再录音复述；给每个技术结论配一个函数入口和一个证据。删除所有自己无法解释的形容词。',[(C,'ServerFire_Implementation'),(L,'ConfirmHit')],['learn/INTERVIEW-LEARNING-ROADMAP.md'])
add(6,'预测与其他 SSR：知道缺什么，如何扩展','Projectile Shotgun SSR ammo prediction B09 B10',
'预测像先显示“我认为会成功”的反馈，服务器回答后再对账。回溯则回答“过去那个时刻打中了吗”。两者解决的问题不同，不能因为有 Hitscan 回溯就说全套预测已经完成。',
'当前 FireOnce 发请求，服务器结算后触发后坐力和 Multicast；Ammo 来自权威复制。Projectile/Shotgun 各有独立命中路径，没有直接复用 Hitscan SSR。扩展时要关联 ShotId、限制重复结算，并明确表现与真实资源账本。',
'设计：本地暂记 ShotId → 预测表现 / 暂扣 → Server 裁决 → 确认或回滚 → 去重',
'把 Hitscan 回溯复制到 Projectile 为什么不够？',
'Projectile 有飞行时间和沿途碰撞，不是某一瞬间的一条射线；Shotgun 需要在统一时刻验证多条 pellet 并聚合。预测弹药还需要请求确认和回滚协议。它们是待扩展设计，不能宣称现有实现已覆盖。',
'设计请求成功、被拒绝、乱序到达三条账本流程，确保一发子弹只扣一次；写出 Projectile 轨迹回放的时间与遮挡边界。',[(C,'FireOnce'),(C,'EFireModel::Projectile')],['learn/networking/B09-hitscan-rewind.md'], '扩展设计课：Projectile/Shotgun SSR 与预测弹药尚未实现；引用现有路径用于比较。')
add(6,'Teams 与夺旗：状态机和一次性得分','Teams CTF Friendly Fire Flag B11',
'夺旗像拿着接力棒跑到终点：谁拿着、属于哪队、是否已交棒都必须有唯一答案。靠客户端碰撞回调自行加分，会让重复触碰变成重复得分。',
'当前 B11 文档是设计：PlayerState 放 Team，GameState 放队伍分数；GameMode 处理队伍分配、友伤和出生点；Flag 用 AtBase/Carried/Dropped 表达状态。并没有可运行 Teams/CTF 代码可作为已完成证据。',
'设计：Server 合法拾旗 → Carried → 合法己方得分区且己旗在家 → 原子计分与重置',
'两人同帧抢旗，或者重复进得分区，怎么处理？',
'服务器先检查旗帜状态和归属，只允许一次状态转换。得分与重置作为同一次权威处理；后续请求看到状态已变就拒绝。持旗者死亡、离开和回合结束都需要清理持有引用及计时器。',
'画 Flag 三态转移图，为错误归属、重复得分、持旗者退出和己旗未归位写预期；这是设计练习，不标为运行验证。',[(GM,'ABlasterGameMode::'),(PS,'AddDefeat')],[], 'B11 仅有设计文档，尚无 Teams/CTF 可玩实现。')

add(6,'武器扩展：火箭、手雷、狙击和护盾如何设计','Rocket Grenade Sniper SMG Shield Jump B07 B08',
'武器名字不等于命中算法。冲锋枪可以是高射速射线，狙击枪可以是低射速高伤害射线；火箭和手雷通常是会飞的对象。先选模型，再定义资源、时序和表现，避免每种武器复制一套战斗逻辑。',
'当前代码提供三种 FireModel 与基础弹药/后坐力配置。Rocket 的范围伤害、Grenade 的延时爆炸、Sniper 的 Scope、Shotgun 逐颗换弹，以及 Jump/Shield 补给不应从枚举名字或课程覆盖推断为完成。可复用 ServerFire 权限、Weapon 容量与 Pickup 消费路径。',
'设计：武器配置 → 共用开火校验 → 飞行 / 射线模型 → 单次权威伤害 → 专属表现',
'如何在现有 Projectile 上设计手雷，而不让爆炸重复结算？',
'服务器生成并管理生命周期，用爆炸状态保护碰撞和引信两种入口，让范围伤害只执行一次。客户端只播爆炸效果；返回主菜单、销毁和回合结束要清定时器。Shield 设计应先确定伤害吸收顺序，Jump Buff 应记录到期并恢复基础能力，都需要新任务与验证。',
'分别写手雷的 Flying/Exploded 状态图、护盾的伤害吸收公式以及逐颗换弹的中断规则。列失败路径：碰撞与引信同帧触发、换弹时切枪、Buff 重复拾取。',[(C,'EFireModel::Projectile'),(W,'AddAmmo'),('Source/Blaster/Weapon/BlasterProjectile.cpp','OnHit')],['learn/networking/weapon-handling.md'], '设计扩展：正式武器差异化、手雷/火箭完整机制、Jump/Shield 不作为已实现成果。')
add(6,'离开与领先者表现：跨生命周期的清理设计','Leave Return Menu Crown Bookkeeping B10',
'玩家离开像剧组撤场：人走了，道具、计时器、名单和房间登记也要各自收尾。皇冠则只是领先状态的投影，不应该由客户端看到一次击杀事件后永久贴上。',
'现有 PlayerState 统计和 GameState 公告可作为设计起点；SessionSubsystem 有 DestroySession。若做完整返回主菜单，要安排服务器玩家清理、会话销毁回调和本地 Travel，不能只 Remove Widget。领先者列表应由服务器根据分数算，客户端依复制结果显示。',
'设计：离开请求 → Server 清理玩法引用 → Session 异步销毁 → 本地菜单 / 输入恢复',
'玩家掉线与点击离开按钮为什么不能只共用一个 UI 回调？',
'掉线可能根本没有客户端 UI 回调。服务器需要处理连接离开及角色清理；主动离开还要组织客户端会话销毁与 Travel。皇冠等表现应从有效玩家状态恢复，避免旧 Pawn 销毁后残留。',
'为正常退出、主机退出、销毁会话失败、旧 Pawn 已无效分别画清理顺序。列出需要清除的 Timer、Delegate、玩家引用及 UI 输入模式。',[(S,'DestroySession'),(PS,'AddDefeat'),(GS,'AnnounceElimination')],['learn/networking/B10-announcements.md'], '完整离开链路与领先者表现按后续设计学习，不能以公告已实现代替该功能验收。')
add(3,'远端如何移动：网络平滑、更新频率与蹲伏','CharacterMovement NetUpdateFrequency Crouch B01',
'别人看到你的角色时，收到的是间隔到达的数据，不是连续视频。引擎移动系统处理网络移动与平滑，动画再从本地副本读速度和姿态。提高更新频率能增加信息密度，但不等于治好所有抖动。',
'Blaster 使用 CharacterMovement，AnimInstance 读 GetVelocity、IsFalling 和 bIsCrouched，而不是每帧 RPC 发送全部骨骼。蹲伏能力、碰撞体与动画姿态是不同层；根骨转身补偿叠加错误也会抖，不能全部归咎于网络。',
'本地移动输入 → CharacterMovement 网络移动 → 远端位置 / 速度 / 蹲伏状态 → 平滑与 AnimInstance → 动画',
'为什么不每帧复制 Speed、Lean 和所有骨骼？',
'已有移动和姿态信息能在各端派生这些动画参数，重复传输会增加带宽并引入多个状态源。网络更新频率是带宽与时效取舍；先确定丢包、移动平滑还是 AnimGraph 叠加导致抖动，再选择修复层。',
'分别观察本地与远端移动、跳跃、蹲伏；对照速度来源和 bIsCrouched。记录网络延迟和动画配置，区分胶囊位置变化与骨骼视觉补偿。',[(A,'GetVelocity'),(A,'bIsCrouched'),(CH,'GetCharacterMovement')],['learn/animation/B01-animation-network.md'])

lessons.sort(key=lambda lesson: lesson['stage'])
Path(__file__).with_name('lessons.json').write_text(json.dumps(lessons, ensure_ascii=False, indent=2), encoding='utf-8')
print(f'Authored {len(lessons)} lessons')
