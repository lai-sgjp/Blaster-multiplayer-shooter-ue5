from pathlib import Path
PROJECT_ROOT = Path(__file__).resolve().parents[2].as_posix()
import unreal as u
t=u.AssetImportTask();t.filename=PROJECT_ROOT + '/Tools/Overhaul/Audio/StreetWind.wav';t.destination_path='/Game/Street/Audio';t.automated=True;t.replace_existing=True;t.save=True
u.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t])
sound=u.load_asset('/Game/Street/Audio/StreetWind');sound.set_editor_property('looping',True);u.EditorAssetLibrary.save_loaded_asset(sound)
levels=u.get_editor_subsystem(u.LevelEditorSubsystem);actors=u.get_editor_subsystem(u.EditorActorSubsystem)
for name in ['StreetStart','StreetLobby','StreetArena']:
 levels.load_level('/Game/Street/Maps/'+name)
 for a in actors.get_all_level_actors():
  if a.get_actor_label()=='Street_Wind':actors.destroy_actor(a)
 a=actors.spawn_actor_from_class(u.AmbientSound,u.Vector(0,0,200));a.set_actor_label('Street_Wind');a.audio_component.set_sound(sound);a.audio_component.set_volume_multiplier(.3)
 levels.save_current_level()
levels.load_level('/Game/Street/Maps/StreetStart')
print('STREET_AMBIENCE_SAVED')
