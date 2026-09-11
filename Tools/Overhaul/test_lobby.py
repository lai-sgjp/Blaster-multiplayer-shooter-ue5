from pathlib import Path
PROJECT_ROOT = Path(__file__).resolve().parents[2].as_posix()
"""Run in a 2, 3 or 8 player listen PIE lobby. Never treat capacity as a start threshold."""
import unreal as u, json, time, traceback
from pathlib import Path
worlds=u.EditorLevelLibrary.get_pie_worlds(False)
server=next(w for w in worlds if u.GameplayStatics.get_game_mode(w))
controllers=u.GameplayStatics.get_all_actors_of_class(server,u.PlayerController)
host=u.GameplayStatics.get_player_controller(server,0)
client=next(u.GameplayStatics.get_player_controller(w,0) for w in worlds if w!=server)
state=u.GameplayStatics.get_game_state(server)
report={'worlds':len(worlds),'players':len(controllers),'checks':[]}
start=time.monotonic();stage=0

def check(name,value): report['checks'].append({'name':name,'pass':bool(value)})
def launch(): return state.get_editor_property('LaunchTime')
def request(c): c.call_method('ServerRequestStart',())
def finish():
    u.unregister_slate_post_tick_callback(handle)
    Path(f'{PROJECT_ROOT}/Saved/street-host-start-{len(controllers)}.json').write_text(json.dumps(report,indent=2))
    print('HOST_START_TEST_DONE',json.dumps(report))
def tick(dt):
    global stage
    try:
        elapsed=time.monotonic()-start
        if stage==0:
            check('no automatic start',launch()==0)
            request(client);stage=1
        elif stage==1 and elapsed>.8:
            check('remote client cannot start',launch()==0)
            request(host);stage=2
        elif stage==2 and elapsed>1.6:
            check('host requires at least three', (launch()>0)==(len(controllers)>=3))
            if len(controllers)<3:finish();stage=9;return
            request(host);stage=3
        elif stage==3 and elapsed>2.4:
            check('host can cancel countdown',launch()==0)
            request(host);stage=4
        elif stage==4 and elapsed>3.2:
            check('host can restart countdown',launch()>0)
            request(host);finish();stage=9
    except Exception:
        report['error']=traceback.format_exc();finish();stage=9
handle=u.register_slate_post_tick_callback(tick)
