from pathlib import Path
PROJECT_ROOT = Path(__file__).resolve().parents[2].as_posix()
import unreal as u, time, json, traceback
from pathlib import Path
ws=u.EditorLevelLibrary.get_pie_worlds(False)
w=next(x for x in ws if u.GameplayStatics.get_game_mode(x))
cls=u.load_class(None,'/Game/Blueprints/Character/BP_BlasterCharacter.BP_BlasterCharacter_C')
p=u.GameplayStatics.get_player_pawn(w,0)
inv=p.get_component_by_class(u.load_class(None,'/Script/Blaster.InventoryComponent'))
pickup_class=u.load_class(None,'/Script/Blaster.BlasterPickup')
report={'checks':[]};stage=0;start=time.monotonic();drops=[]
def check(name,value):report['checks'].append({'name':name,'pass':bool(value)})
def count():return inv.count(u.SupplyKind.MEDICAL)
def finish():
    u.unregister_slate_post_tick_callback(handle)
    Path(PROJECT_ROOT + '/Saved/street-inventory-test.json').write_text(json.dumps(report,indent=2))
    print('INVENTORY_TEST_DONE',report)
def tick(dt):
    global stage,start
    try:
        if str(u.GameplayStatics.get_game_state(w).get_editor_property('MatchState'))!='InProgress':return
        elapsed=time.monotonic()-start
        if stage==0:
            for old in u.GameplayStatics.get_all_actors_of_class(w,pickup_class): old.destroy_actor()
            p.set_actor_location(u.Vector(0,-3500,110),False,True)
            for i in range(4):
                t=u.Transform(location=u.Vector(0,-3500,110))
                statics=u.get_default_object(u.GameplayStatics)
                a=statics.call_method('BeginDeferredActorSpawnFromClass',(w,pickup_class,t,u.SpawnActorCollisionHandlingMethod.ALWAYS_SPAWN,None,u.SpawnActorScaleMethod.MULTIPLY_WITH_ROOT))
                statics.call_method('FinishSpawningActor',(a,t,u.SpawnActorScaleMethod.MULTIPLY_WITH_ROOT));drops.append(a)
            stage=1;start=time.monotonic()
        elif stage==1 and elapsed>.5:
            check('stock capped at three',count()==3)
            check('fourth world pickup remains',sum(not x.get_editor_property('bHidden') for x in drops)==1)
            inv.server_use(u.SupplyKind.MEDICAL);stage=2
        elif stage==2 and elapsed>1:
            check('full health does not consume',count()==3)
            u.GameplayStatics.apply_damage(p,30,None,None,u.DamageType)
            inv.server_use(u.SupplyKind.MEDICAL);stage=3
        elif stage==3 and elapsed>1.5:
            check('medical restores twenty-five',p.get_editor_property('Health')==95)
            check('one item consumed',count()==2)
            # Open menu through its actual widget callback, then damage the pawn.
            p.get_controller().set_menu_open(True)
            check('menu opens',p.get_controller().is_menu_open())
            u.GameplayStatics.apply_damage(p,200,None,None,u.DamageType)
            stage=4
        elif stage==4 and elapsed>2:
            check('death closes menu',not p.get_controller().is_menu_open())
            check('death clears inventory',count()==0)
            check('death applies during gameplay',p.get_editor_property('Health')==0)
            finish();stage=5
    except Exception:
        report['error']=traceback.format_exc();finish();stage=5
handle=u.register_slate_post_tick_callback(tick)
