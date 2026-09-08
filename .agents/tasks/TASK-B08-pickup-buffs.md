# B08B pickup/buffs — Gate0 READY
Goal: map has three weapon variants and reusable health/ammo/speed pickups; server applies bounded effect once, hides pickup30s then returns. Health25 capped100, reserve30 capped180, speed900 for8s then600. Effects ignore eliminated/full/inactive-match as appropriate.
Allowlist: new Pickups/BlasterPickup.h/.cpp, Character/BlasterCharacter.h/.cpp, BlasterGameMode.h/.cpp, CombatComponent.h/.cpp, HUD/BlasterHUD.cpp, actual BP_BlasterGameMode WeaponPickupClasses; docs. Spawn positions based on tested BlasterMap start platform coordinates. Existing map asset/art not altered.
Server owns pickup/effect/timers; actor hidden state replicated; client collision disabled. Speed value replicated with OnRep; timers cleared on EndPlay/death. HUD labels readable names. Reusable bounded actors avoid accumulation.
Verify build, two-peer health/ammo/speed effect, full-health pickup not consumed, one overlap application, speed expiry, pickup return; map has three configured weapon types. Preserve B08A TeleportPhysics fix.

## Current delivery status (2026-09-08)
PASS WITH FOLLOW-UP for the frozen core slice. See REVIEW-B01-B10-core.md and ../../docs/VERIFICATION.md (project docs/VERIFICATION.md) for current evidence and explicit limits; historical Gate0 and intermediate blockers above are preserved. Optional art/network extensions are not claimed complete.
