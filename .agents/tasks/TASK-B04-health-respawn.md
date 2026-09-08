# B04 health/elimination/respawn — Gate0 READY
Goal: projectile server damage reduces replicated100health; zerohealth disables pawn, awards score/defeat once, then respawns; HUD reads current pawn and persistent PlayerState.
Allowlist: BlasterCharacter.h/.cpp, CombatComponent.cpp, BlasterProjectile.cpp, BlasterGameMode.h/.cpp, BlasterHUD.cpp, new Character/BlasterPlayerState.h/.cpp; actual BP_BlasterGameMode PlayerStateClass; docs.
Authority: only server damage delegate registered; only server scores/respawns. RepNotify applies cosmetic disable both peers. Elimination timer owned by pawn and cleared at EndPlay; weak-safe controller use at respawn. Persistent PlayerState survives pawn replacement. Prevent eliminated input/fire/equip; no repeated score. Weapons destroyed with eliminated pawn for now; full drop rules B08.
HealthHUD simple text/bar, no dissolve/VFX in this core slice. Verify complete build, dual-peer health, one elimination/score/defeat, respawn/newPawn100health, repeat damage cannot award twice, firing while dead rejected.

## Current delivery status (2026-09-08)
PASS WITH FOLLOW-UP for the frozen core slice. See REVIEW-B01-B10-core.md and ../../docs/VERIFICATION.md (project docs/VERIFICATION.md) for current evidence and explicit limits; historical Gate0 and intermediate blockers above are preserved. Optional art/network extensions are not claimed complete.
