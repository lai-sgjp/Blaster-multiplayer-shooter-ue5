from pathlib import Path
PROJECT_ROOT = Path(__file__).resolve().parents[2].as_posix()
import unreal as u, time, json, traceback
from pathlib import Path
ws=u.EditorLevelLibrary.get_pie_worlds(False)
w=next(w for w in ws if u.GameplayStatics.get_game_mode(w))
cls=u.load_class(None,'/Game/Blueprints/Character/BP_BlasterCharacter.BP_BlasterCharacter_C')
players=u.GameplayStatics.get_all_actors_of_class(w,cls)
p=players[0]
players[1].set_actor_location(u.Vector(-4900,3800,110),False,True)
v=None
combat=p.get_component_by_class(u.load_class(None,'/Script/Blaster.CombatComponent'))
weaponclass=u.load_class(None,'/Script/Blaster.Weapon')
result={'shots':[],'errors':[]};stage=0;start=time.monotonic();shot=100;trial=0;g=None
statics=u.get_default_object(u.GameplayStatics)
def spawn(cls,tr):
    a=statics.call_method('BeginDeferredActorSpawnFromClass',(w,cls,tr,u.SpawnActorCollisionHandlingMethod.ALWAYS_SPAWN,None,u.SpawnActorScaleMethod.MULTIPLY_WITH_ROOT))
    return statics.call_method('FinishSpawningActor',(a,tr,u.SpawnActorScaleMethod.MULTIPLY_WITH_ROOT))
cases=[('BP_HitscanWeapon','head'),('BP_HitscanWeapon','spine_02'),('BP_Weapon','head'),('BP_Weapon','spine_02'),('BP_ShotgunWeapon','head'),('BP_ShotgunWeapon','spine_02')]
def finish():
    u.unregister_slate_post_tick_callback(handle)
    Path(PROJECT_ROOT + '/Saved/street-combat-test.json').write_text(json.dumps(result,indent=2))
    print('COMBAT_TEST_DONE',json.dumps(result))
def tick(dt):
    global stage,start,shot,trial,g,v
    try:
        state=u.GameplayStatics.get_game_state(w)
        if str(state.get_editor_property('MatchState'))!='InProgress':return
        elapsed=time.monotonic()-start
        if trial>=len(cases):finish();return
        model,bone=cases[trial]
        if stage==0:
            p.set_actor_location(u.Vector(-1000,-3600,110),False,True)
            if v: v.destroy_actor()
            victim_transform=u.Transform(location=u.Vector(-1000,-2700,110))
            v=spawn(cls,victim_transform)
            # Remove unowned test weapons so the overlap query selects this case.
            for old in u.GameplayStatics.get_all_actors_of_class(w,weaponclass):
                if old.get_owner() is None: old.destroy_actor()
            tr=u.Transform(location=p.get_actor_location())
            g=spawn(u.load_class(None,'/Game/Blueprints/Weapon/'+model+'.'+model+'_C'),tr)
            stage=1;start=time.monotonic()
        elif stage==1 and elapsed>.4:
            p.call_method('ServerEquipButtonPressed',())
            target=v.get_component_by_class(u.SkeletalMeshComponent).get_socket_location(bone)
            p.get_controller().set_control_rotation(u.MathLibrary.find_look_at_rotation(p.get_actor_location()+u.Vector(0,0,64),target))
            stage=2
        elif stage==2 and elapsed>1.1:
            target=v.get_component_by_class(u.SkeletalMeshComponent).get_socket_location(bone)
            point=u.Vector_NetQuantize();point.x=target.x;point.y=target.y;point.z=target.z
            combat.call_method('ServerFire',(point,g,state.get_server_world_time_seconds(),shot));shot+=1
            stage=3
        elif stage==3 and elapsed>1.8:
            result['shots'].append({'model':model,'bone':bone,'damage':100-v.get_editor_property('Health'),'ammo':g.get_editor_property('Ammo'),'owner':str(g.get_owner())})
            trial+=1;stage=0;start=time.monotonic()
    except Exception:
        result['errors'].append(traceback.format_exc());finish();stage=99
handle=u.register_slate_post_tick_callback(tick)
