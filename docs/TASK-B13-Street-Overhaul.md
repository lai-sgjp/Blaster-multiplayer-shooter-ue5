# TASK-B13 Street overhaul

Status: IN PROGRESS. User approved the complete multi-slice plan on 2026-09-11.

## Gate 0
Deliver three saved UE worlds, consistent authoritative head/body hits, local inventory menu, host-controlled lobby and unified native UMG presentation. Existing maps are preserved. No account backend, purchases, cross-round inventory or Git publishing. Transcript not read; this is the user's production polish request.

Ownership: Combat and hit queries own shot validation; InventoryComponent owns server-only quantities and owner replication; LobbyGameMode owns host authorization, countdown/travel; PlayerController owns local input/UI and session navigation. Widgets only present local state. Character retains movement/health.

Allowed files: Source/Blaster/{Blaster.h,Blaster.Build.cs,Character,BlasterComponent,Weapon,HUD,GameModes,Tests}; Source/Blaster/Pickups/BlasterPickup.cpp; Config/DefaultEngine.ini, Config/DefaultGame.ini; Tools/Overhaul; docs/TASK-B13-Street-Overhaul.md, docs/OVERHAUL-VERIFICATION.md; .agents/worklog.md. Assets: Content/Street (new), locomotion sequences actually referenced by the character if measured root displacement requires correction. No old map overwrite.

Editor work is explicitly delegated to Codex by the user. Use editor Python for reproducible material/module/world creation, save assets and inspect viewport. Runtime C++ requires a full Editor restart after UHT changes. Engine reference: Engine/Source/Runtime/Engine/Private/GameFramework/SpringArmComponent.cpp; Engine/Public and Classes/GameFramework/PlayerController.h; UMG/Public/Blueprint/UserWidget.h.

## Verification
Build Editor/Game; meaningful automation for damage/stock boundaries; two-client gameplay and 8-client lobby/capacity; menu input/death/travel; collision apertures and muzzle clearance; camera telemetry; three resolutions and packaged Windows smoke. Record each independently, never infer runtime success from compilation. UI/GC/Enhanced Input compatibility decided against installed UE5.6 headers/build.

## Latest user amendment
2026-09-11: host requests start, with more than two players (minimum three), maximum eight; no all-ready or full-room requirement. Steam listen sessions, no dedicated-server host migration. Any join/leave cancels countdown and requires a fresh host request.

## Implemented and measured so far
- Editor builds pass; server shots show hitscan and projectile 40 head / 20 body. Projectile contact on current mesh reported backpack; skeletal segment refinement resolves head. Shotgun aggregates per pellet.
- 2/3/8-player host-start PIE checks pass. Eight worlds travelled to StreetArena after host request. These are local PIE connections, not eight Steam accounts.
- Owner inventory/menu: cap 3, fourth pickup retained, full health rejects use, heal25, menu can open, damage kills while menu open, death closes menu and clears stock: 9 checks pass.
- New flat-ground camera telemetry has constant measured capsule-to-camera distance 325.461 cm on host and client over four seconds; no animation edits made.
- Three maps saved under Content/Street; original modules/materials and original short feedback audio, Noto Sans SC bold OFL font. Final visual/package verification remains in progress.
- Independent review fixes: cook/default-map inclusion, capacity PreLogin, create-session delegate lifetime, unavailable session error, failed travel cleanup. Native listen-host identity is the supported scope.

- Windows包实际Steam创建/退出成功，八个本机IP进程容量及第九拒绝已验证；不是八个Steam账号。
- 用户直接Client 1 PIE准心缺失反馈已修复：UMG NativePaint、黑描边和物理像素最小值，640×480及菜单关闭后实拍通过。
- 100ms下真实远端库存RPC7项通过；Python直调RPC强制Local的测试限制已记录并用Editor-only下一tick入口避开。
- 完整结果与仍未完成的美术/跨账号Steam/动态战斗/性能矩阵见 OVERHAUL-VERIFICATION.md；不标记完整计划全部验收。
