from pathlib import Path
PROJECT_ROOT = Path(__file__).resolve().parents[2].as_posix()
import unreal, json, time
from pathlib import Path
worlds = unreal.EditorLevelLibrary.get_pie_worlds(False)
cls = unreal.load_class(None, '/Game/Blueprints/Character/BP_BlasterCharacter.BP_BlasterCharacter_C')
players = [p for w in worlds for p in unreal.GameplayStatics.get_all_actors_of_class(w,cls) if p.is_locally_controlled()]
samples=[]
started=time.monotonic()
def sample(dt):
    elapsed=time.monotonic()-started
    for p in players:
        p.add_movement_input(unreal.Vector(1,0,0),1,False)
        mesh=p.get_component_by_class(unreal.SkeletalMeshComponent)
        cam=p.get_component_by_class(unreal.CameraComponent)
        arm=p.get_component_by_class(unreal.SpringArmComponent)
        samples.append({'t':round(elapsed,3),'player':p.get_path_name(),'capsule':str(p.get_actor_location()),'mesh':str(mesh.get_world_location()),
                        'root_local':str(mesh.get_socket_transform('root',unreal.RelativeTransformSpace.RTS_COMPONENT).translation),
                        'camera':str(cam.get_world_location()),'arm_distance':cam.get_world_location().distance(arm.get_world_location())})
    if elapsed>4:
        unreal.unregister_slate_post_tick_callback(handle)
        Path(PROJECT_ROOT + '/Saved/camera-telemetry.json').write_text(json.dumps(samples,indent=2))
        print('CAMERA_TELEMETRY_DONE',len(samples))
handle=unreal.register_slate_post_tick_callback(sample)
