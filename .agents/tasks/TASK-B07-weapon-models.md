# B07 weapon models — Gate0 READY
Goal: keep projectile rifle and add server hitscan and8pellet shotgun variants sharing owner/cooldown/ammo/reload validation. Player-visible variants configured as independent BP assets and later placed via B08 pickup slice.
Allowlist: Weapon.h, CombatComponent.cpp, new /Game/Blueprints/Weapon/BP_HitscanWeapon and BP_ShotgunWeapon duplicates of verified BP_Weapon; docs. Preserve mesh/socket contract for prototype variants. Separate art models require separate socket preflight and are not changed now.
Server enum chooses model; hitscan traces muzzle along validated aim, shotgun random cone generated server-side and aggregates damage per target before ApplyDamage. One shell/ammo per trigger, all models share authoritative cadence. Cosmetic multicast remains; no client target/hit authority.
Verify build, BP compile, PIE spawn/equip each variant, hitscan immediate damage with no projectile, shotgun one ammo and bounded8pellet totaldamage32, wall obstruction, projectile regression. SSR B09; real cosmetic tracers/art follow-up.

## Current delivery status (2026-09-08)
PASS WITH FOLLOW-UP for the frozen core slice. See REVIEW-B01-B10-core.md and ../../docs/VERIFICATION.md (project docs/VERIFICATION.md) for current evidence and explicit limits; historical Gate0 and intermediate blockers above are preserved. Optional art/network extensions are not claimed complete.
