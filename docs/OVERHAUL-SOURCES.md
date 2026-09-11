# Sources and asset provenance

- Scene geometry, materials, layout and short feedback WAV files: original project scripts in Tools/Overhaul. Cubes, cylinders and spheres come from Unreal Engine basic shapes. Existing Blaster character/weapons are retained; no paid assets were acquired.
- Noto Sans SC Bold: https://github.com/notofonts/noto-cjk/tree/main/Sans/OTF/SimplifiedChinese (NotoSansCJKsc-Bold.otf), SIL Open Font License 1.1. License retained at Tools/Overhaul/Fonts/OFL.txt. StreetFont FontFace embeds the static bold font. The earlier variable font experiment is not the final font face.
- Visual/interaction guidance, not imported application code: https://github.com/anthropics/skills/blob/main/skills/frontend-design/SKILL.md and https://github.com/nextlevelbuilder/ui-ux-pro-max-skill . Applied a coherent palette and bold Chinese typography, visible disabled/hover states, original game content and real-window screenshot critique; implemented with native UE UMG, not a web view.
- User screenshots: composition, warm brick/stone, metal walkways and interface hierarchy reference only. No game screenshots or third-party character art were copied into the game.

The current Content/ exclusion is inherited from this repository. New assets are saved locally in Content/Street; reproduction requires the original project assets plus the scripts/fonts/audio in Tools/Overhaul. No Git commit or push was performed.

提交说明：本次发布调整为只跟踪 Content/Street 原创资源；旧资源包仍被忽略。Saved 中的日志、视频和打包文件仅保留本机。生成和测试脚本已使用工程相对路径；原工程角色、武器及蓝图依赖仍需本地已有资源。
