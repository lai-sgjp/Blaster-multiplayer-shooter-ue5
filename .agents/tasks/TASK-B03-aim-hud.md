# B03 Aim HUD — Gate0 READY
Goal: equipped local player sees center crosshair with movement/air/aim spread and target color, smooth aim FOV; firing and HUD share camera trace.
Allowlist: new Source/Blaster/HUD/BlasterHUD.h/.cpp, CombatComponent.h/.cpp, BlasterGameMode.cpp, BP_BlasterGameMode HUDClass after actual path verification, learn/task/worklog.
Authority unchanged: HUD/FOV local presentation only; server validates existing fire. Avoid new replicated UI state. No health/ammo in this slice.
Verify: Editor build, Listen Server+Client HUD class local, FOV90->60->90, unarmed no reticle, target trace color/spread state; regression firing. Visual test evidence must show real player view; screenshot tool viewport mismatch remains a limitation.
Gate0 update: allow BlasterCharacter.cpp to set mesh Visibility response Block in BeginPlay. Engine Pawn/CharacterMesh defaults ignore Visibility, so target crosshair detection requires explicit project response; keep capsule unchanged.

## Current delivery status (2026-09-08)
PASS WITH FOLLOW-UP for the frozen core slice. See REVIEW-B01-B10-core.md and ../../docs/VERIFICATION.md (project docs/VERIFICATION.md) for current evidence and explicit limits; historical Gate0 and intermediate blockers above are preserved. Optional art/network extensions are not claimed complete.
