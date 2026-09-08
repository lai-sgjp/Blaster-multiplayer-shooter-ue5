# B06 match loop — Gate0 READY
Goal: server10swarmup/120smatch/10scooldown then restart; clients display synchronized remaining time and phase. No firing/reloading/damage outside InProgress. Scores reset each newmatch.
Allowlist: new Character/BlasterGameState.h/.cpp, BlasterGameMode.h/.cpp, BlasterPlayerState.h/.cpp, CombatComponent.cpp, BlasterCharacter.cpp, BlasterHUD.cpp, actual BP_BlasterGameMode GameStateClass, docs.
Use engine AGameState server-time sync and MatchState replication; own replicated phase deadline. Server drives StartMatch/EndMatch/RestartGame. Local HUD only reads replicated phase/time. Timer timings editable defaults, tests may temporarily shorten and restore.
Verify complete build; both peers observe WaitingToStart/InProgress/WaitingPostMatch/restart and clock bounds; fire/damage rejected outside match, accepted during; scoreboard resets. Existing loop resources/Steam real external session not altered.

## Current delivery status (2026-09-08)
PASS WITH FOLLOW-UP for the frozen core slice. See REVIEW-B01-B10-core.md and ../../docs/VERIFICATION.md (project docs/VERIFICATION.md) for current evidence and explicit limits; historical Gate0 and intermediate blockers above are preserved. Optional art/network extensions are not claimed complete.
