from pathlib import Path
PROJECT_ROOT = Path(__file__).resolve().parents[2].as_posix()
import unreal as u
task=u.AssetImportTask()
task.filename=PROJECT_ROOT + '/Tools/Overhaul/Fonts/StreetBold.otf'
task.destination_path='/Game/Street/UI';task.destination_name='StreetFont'
task.replace_existing=True;task.automated=True;task.save=True;task.factory=u.FontFileImportFactory()
u.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
print('BOLD_IMPORTED',task.imported_object_paths)
