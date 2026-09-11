from pathlib import Path
PROJECT_ROOT = Path(__file__).resolve().parents[2].as_posix()
import unreal as u,time,json,traceback
from pathlib import Path
w=u.EditorLevelLibrary.get_pie_worlds(False)[0];p=u.GameplayStatics.get_player_pawn(w,0);pc=p.get_controller();pc.set_menu_open(False)
initialized=False
waypoints=[(-1900,-1700,490),(-1670,-1700,490),(-1670,-2100,490),(-1400,-2100,490),(-700,-2100,890),(0,-2100,890)]
report={'points':[]};index=0;start=time.monotonic();total=start

def done():
 u.unregister_slate_post_tick_callback(handle);Path(PROJECT_ROOT + '/Saved/street-walk-test.json').write_text(json.dumps(report,indent=2));print('WALK_TEST_DONE',report)
def tick(dt):
 global index,start,p,initialized
 try:
  if not initialized:
   if str(u.GameplayStatics.get_game_state(w).get_editor_property('MatchState'))!='InProgress':return
   p=u.GameplayStatics.get_player_pawn(w,0);p.set_actor_location(u.Vector(-3080,-1700,110),False,True);initialized=True;start=time.monotonic()
  if index>=len(waypoints):done();return
  x,y,z=waypoints[index];loc=p.get_actor_location();delta=u.Vector(x-loc.x,y-loc.y,0)
  if delta.length()<30:
   report['points'].append({'target':waypoints[index],'actual':[loc.x,loc.y,loc.z],'pass':abs(loc.z-z)<50});index+=1;start=time.monotonic();return
  if time.monotonic()-start>8:report['failedAt']=index;report['actual']=[loc.x,loc.y,loc.z];done();return
  p.add_movement_input(delta.normal(),1,True)
 except Exception:report['error']=traceback.format_exc();done()
handle=u.register_slate_post_tick_callback(tick)
