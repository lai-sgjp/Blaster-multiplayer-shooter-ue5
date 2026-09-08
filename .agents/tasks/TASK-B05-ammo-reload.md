# B05 ammo/reload — Gate0 READY
Goal: server authoritative30round magazine/90reserve, R reload transfers only missing rounds after1.5s; fire denied when empty/reloading/dead; HUD follows replicated values.
Allowlist: Weapon.h/.cpp, CombatComponent.h/.cpp, BlasterCharacter.h/.cpp, BlasterHUD.cpp, new IA_Reload and existing IMC_Default/BP_BlasterCharacter ReloadAction, docs.
Flow: local R -> ServerReload validates -> replicated bReloading + server timer -> bounded transfer/reserve decrement -> ready. ServerFire spends one ammo only after successful spawn. EndPlay clears both timers. Death or weapon invalid cancels transfer. No client ammo authority. Reload montage later art slice.
Verify build + ListenServerClient:30shot cap, dryfire no spawn, R30/60, tactical reload preserves total, repeatedR cannot duplicate transfer, no fire while reload, pawn elimination cancels timer.

## Current delivery status (2026-09-08)
PASS WITH FOLLOW-UP for the frozen core slice. See REVIEW-B01-B10-core.md and ../../docs/VERIFICATION.md (project docs/VERIFICATION.md) for current evidence and explicit limits; historical Gate0 and intermediate blockers above are preserved. Optional art/network extensions are not claimed complete.
