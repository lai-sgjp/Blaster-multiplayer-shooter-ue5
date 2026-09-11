from pathlib import Path
PROJECT_ROOT = Path(__file__).resolve().parents[2].as_posix()
import unreal as u,time,json,traceback
from pathlib import Path
ws=u.EditorLevelLibrary.get_pie_worlds(False);sw=next(w for w in ws if u.GameplayStatics.get_game_mode(w));cw=next(w for w in ws if not u.GameplayStatics.get_game_mode(w))
cls=u.load_class(None,'/Script/Blaster.BlasterCharacter');ic=u.load_class(None,'/Script/Blaster.InventoryComponent');pk=u.load_class(None,'/Script/Blaster.BlasterPickup')
pc=cp=sp=ci=si=screen=None
for w in ws:u.SystemLibrary.execute_console_command(w,'NetEmulation.PktLag 100')
report={'request_path':'editor-only deferred native RPC; not Python direct ProcessEvent','latency_ms':100,'checks':[],'errors':[]};stage=0;t=time.monotonic()
def check(n,v):report['checks'].append({'name':n,'pass':bool(v)})
def speed():return sp.get_component_by_class(u.CharacterMovementComponent).get_editor_property('max_walk_speed')
def finish():
 for w in ws:u.SystemLibrary.execute_console_command(w,'NetEmulation.PktLag 0')
 u.unregister_slate_post_tick_callback(handle);Path(PROJECT_ROOT + '/Saved/street-latency-inventory.json').write_text(json.dumps(report,indent=2));print('LATENCY_DONE',report)
def tick(dt):
 global stage,t,pc,cp,sp,ci,si,screen
 try:
  e=time.monotonic()-t
  if stage==0:
   if str(u.GameplayStatics.get_game_state(sw).get_editor_property('MatchState'))!='InProgress':return
   pc=u.GameplayStatics.get_player_controller(cw,0);cp=u.GameplayStatics.get_player_pawn(cw,0)
   if not cp or not cp.get_editor_property('PlayerState'):return
   sp=next(p for p in u.GameplayStatics.get_all_actors_of_class(sw,cls) if p.get_editor_property("PlayerState") and p.get_editor_property("PlayerState").get_editor_property("PlayerId")==cp.get_editor_property("PlayerState").get_editor_property("PlayerId"))
   ci=cp.get_component_by_class(ic);si=sp.get_component_by_class(ic)
   sp.set_actor_location(u.Vector(0,-3500,110),False,True)
   supplies=[a for a in u.GameplayStatics.get_all_actors_of_class(sw,pk) if a.get_editor_property('Kind')==u.BlasterPickupKind.SPEED]
   assert len(supplies)>=3,'Requires three original arena speed pickups'
   for a in supplies[:3]:a.set_actor_location(sp.get_actor_location(),False,True)
   stage=1;t=time.monotonic()
  elif stage==1 and e>1.5:
   check('owner receives three speed items',ci.count(u.SupplyKind.SPEED)==3)
   pc.set_menu_open(True);u.SystemLibrary.execute_console_command(cw,'blaster.QAUseSpeed',pc);stage=2;t=time.monotonic()
  elif stage==2 and e>1:
   check('remote request changes server speed',speed()==900);check('owner quantity decremented',ci.count(u.SupplyKind.SPEED)==2);stage=3
  elif stage==3 and e>4:
   u.SystemLibrary.execute_console_command(cw,'blaster.QAUseSpeed',pc);u.SystemLibrary.execute_console_command(cw,'blaster.QAUseSpeed',pc);stage=4;t=time.monotonic()
  elif stage==4 and e>1:
   check('back to back use consumes only once',ci.count(u.SupplyKind.SPEED)==1);check('speed does not stack',speed()==900);stage=5
  elif stage==5 and e>5.5:
   check('second use refreshes duration',speed()==900);stage=6
  elif stage==6 and e>9:
   check('speed returns to baseline after refreshed expiry',speed()==600);pc.set_menu_open(False);finish();stage=99
 except Exception:
  report['errors'].append(traceback.format_exc());finish();stage=99
handle=u.register_slate_post_tick_callback(tick)
