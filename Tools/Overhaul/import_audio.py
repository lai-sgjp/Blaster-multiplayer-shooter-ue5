from pathlib import Path
PROJECT_ROOT = Path(__file__).resolve().parents[2].as_posix()
import unreal as u
for name in ['Body','Head','Pickup','Hurt']:
    task=u.AssetImportTask();task.filename=PROJECT_ROOT + '/Tools/Overhaul/Audio/'+name+'.wav';task.destination_path='/Game/Street/Audio';task.automated=True;task.replace_existing=True;task.save=True
    u.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
print('STREET_AUDIO_SAVED')
