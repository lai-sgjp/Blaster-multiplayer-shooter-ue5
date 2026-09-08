# B08A default/dual/drop — Gate0 READY
Goal: respawn with default rifle; pick second weapon, Q swaps; third pickup replaces/drops active weapon; death drops both. Server validates ownership and reload/death boundaries. Fix spawn-on-pawn overlap binding order.
Allowlist: Weapon.h/.cpp, CombatComponent.h/.cpp, BlasterCharacter.h/.cpp, BlasterHUD.cpp, new IA_Swap+IMC_Default+BP_BlasterCharacter DefaultWeaponClass/SwapAction; docs. Default uses verified BP_Weapon. Secondary hidden while stored (no backpack socket currently); both actor states replicated. Original art unchanged.
Server-only equip/swap/drop; weapon ammo persists with each actor. Dropped actors detach and replicate physics movement, server pickup overlap. EndPlay destroyed pawn cleans owned weapons without double-drop. No reload-swap, no stealing owned weapons, clear stale overlap reference after equip.
Verify build, two-peer default rifle, two weapons/ammo switch, third replacement, death/drop/reacquire, spawn overlap without leaving/reentering. Buff pickup slice later.

## Current delivery status (2026-09-08)
PASS WITH FOLLOW-UP for the frozen core slice. See REVIEW-B01-B10-core.md and ../../docs/VERIFICATION.md (project docs/VERIFICATION.md) for current evidence and explicit limits; historical Gate0 and intermediate blockers above are preserved. Optional art/network extensions are not claimed complete.
