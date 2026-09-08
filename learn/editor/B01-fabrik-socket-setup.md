# B01 FABRIK 恢复前置：武器 LeftHandSocket

当前切片被安全暂停，因为项目实际测试武器没有左手握把 socket。该操作必须由用户在 Unreal Editor 中完成；本代理不修改武器 socket，也不使用 `Muzzle` 或组件原点替代。

## 资产路径

从项目根目录打开：

`Content/Assets/FPS_Weapon_Bundle/Weapons/Meshes/AR4/SK_AR4_X`

这是 live Editor 从 `BlasterMap` 中唯一 `BP_Weapon_C_1 -> WeaponMesh` 实例回读出的 Skeletal Mesh。

## Editor 操作

1. 在 Content Browser 打开上述 `SK_AR4_X` Skeletal Mesh。
2. 在 Skeletal Mesh Editor 的 Skeleton Tree 中定位武器左手实际握持位置对应的骨骼；以视口中的步枪左手握把为准，不改角色的 `RightHandSocket` 附着 socket。
3. 在该武器骨骼上新增 Socket，名称必须逐字为 `LeftHandSocket`（大小写一致）。把 socket 移到左手应握住的握把位置，并用视口预览确认方向适合后续 FABRIK effector。
4. 保存 `SK_AR4_X` 资产。不要修改 `ABP_Blaster`、角色状态机、Aim Offset 或 `RightHandSocket`。

## 预期结果与回报

- 在 Socket 预览/管理列表中可看到唯一名称 `LeftHandSocket`；原有 `Muzzle` 保持不变。
- 保存后告知代理“已完成 LeftHandSocket 并保存”，代理会重新运行 live 检查：`num_sockets` 应至少为 2，且 `find_socket("LeftHandSocket")` 返回非空。
- 代理随后还会核验 `ABP_Blaster` skeleton 的 `hand_r`、`hand_l`、`upperarm_l`，再决定是否继续 C++ getter、`LeftHandTransform` 和顶层 FABRIK 接线。

## 如果找不到合适骨骼

不要凭空创建骨骼或把 socket 放在 `Muzzle`。保留 Editor 状态并报告 Skeleton Tree 中实际可用的骨骼名称，等待重新确定资产方案；任务卡的停止条件优先于继续写入。
