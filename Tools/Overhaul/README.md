# Street overhaul tools

Run the asset scripts inside UE 5.6 editor with `py "<project>/Tools/Overhaul/<script>.py"`.

Rebuild order: build_street.py → polish_scene.py → fix_stairs.py → load StreetStart → finish_paving.py → import_bold.py → import_audio.py → add_ambient.py.
The generator overwrites only Content/Street. Old maps are preserved. Build Editor and Game from the same source revision BEFORE cooking; Blueprint native property layout must match the packaged executable.

- make_audio.py runs with ordinary Python and writes original short WAV cues; import_audio.py saves UE SoundWave assets.
- Font source is Noto Sans SC Bold (OFL), see Fonts/OFL.txt and docs/OVERHAUL-SOURCES.md.
- test_lobby.py: 2, 3 or 8 player listen PIE, validates host authority and the minimum of three. test_travel.py then launches via the host.
- test_combat.py: fresh two-player arena PIE in progress, server-authoritative hitscan/projectile/shotgun evidence.
- test_inventory.py: fresh arena PIE in progress, owner inventory cap/full-health/heal/menu/death checks. It destroys runtime pickups in this test world.
- test_walk.py: single-player arena PIE; actually walks up two flights. It waits for InProgress because the warmup pawn is replaced at match start.
- test_collision.py: stop PIE, open StreetArena in editor. Six weapon-channel geometry probes with debug lines.
- camera_telemetry.py: host/client movement telemetry; output Saved/camera-telemetry.json.

Tests create and manipulate disposable PIE actors. Never execute them in a live public match. Results are in Saved and must be inspected for errors/false checks before claiming a pass.

- make_ambient.py generates the original low-volume wind loop before add_ambient.py imports and places it.
- test_latency_inventory.py: fresh two-player StreetArena PIE, after warmup, tests owner RPC/stock and speed refresh at NetEmulation.PktLag 100; restores lag afterwards.

Important: UE editor Python forces actor RPC callspace to Local, including nested C++ calls. Use the WITH_EDITOR/PIE-only blaster.QAUseSpeed deferred native command for the latency inventory test. Direct Python RPC calls are NOT evidence of a remote network request.

Workstation-specific window capture/benchmark launchers and temporary inspection scripts are local-only and ignored. Published scripts resolve paths from their own project checkout.
