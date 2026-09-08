# B02 firing presentation
Gate 0 READY, 2026-09-08. User authorizes rough prototype and autonomous completion.
Goal: accepted server shot plays local upper-body fire montage and cosmetic shell on both peers. Authoritative projectile unchanged.
Allowlist: CombatComponent.h/.cpp, new Weapon/BlasterCasing.h/.cpp, BlasterAnimationEditorLibrary.h/.cpp, new Content/Blueprints/Character/Animation/B02Fire/*, ABP_Blaster, BP_BlasterCharacter Combat defaults; learn and task/worklog docs.
Assets: source AnimStarterPack Fire_Rifle_Hip/Ironsights on UE4 skeleton; create isolated retarget using existing B01 target chain contract. Original packs unchanged. Empty 556x45 shell available; original weapon has no eject socket, use explicit mesh-local approximate ejection offset for prototype.
Flow: accepted ServerFire -> unreliable MulticastFire -> dynamic montage and nonreplicated shell only; no client damage authority. Add upper-body slot before final FABRIK, preserve turning and unequipped path. Editor helper backs up graph links and rolls back compilation failure.
Verify: full Editor build, ABP compile, Listen Server+Client montage/casing observations, no cosmetic authority side effects, bounded shell lifetime; retain rough visual limitation. No commit/push. No transcript read.

## Current delivery status (2026-09-08)
PASS WITH FOLLOW-UP for the frozen core slice. See REVIEW-B01-B10-core.md and ../../docs/VERIFICATION.md (project docs/VERIFICATION.md) for current evidence and explicit limits; historical Gate0 and intermediate blockers above are preserved. Optional art/network extensions are not claimed complete.
