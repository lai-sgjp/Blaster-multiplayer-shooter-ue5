from pathlib import Path
PROJECT_ROOT = Path(__file__).resolve().parents[2].as_posix()
import unreal as u, time, json, traceback
from pathlib import Path
worlds=u.EditorLevelLibrary.get_pie_worlds(False)
server=next(w for w in worlds if u.GameplayStatics.get_game_mode(w))
# The host requests a five-second launch; no per-player ready requirement.
u.GameplayStatics.get_player_controller(server,0).call_method('ServerRequestStart',())
start=time.monotonic()
def tick(dt):
    elapsed=time.monotonic()-start
    ws=u.EditorLevelLibrary.get_pie_worlds(False)
    if elapsed>9 and all('StreetArena' in w.get_name() for w in ws):
        result={'pass':len(ws)==len(worlds),'worlds':[w.get_name() for w in ws],'elapsed':elapsed}
    elif elapsed>30: result={'pass':False,'worlds':[w.get_name() for w in ws],'timeout':elapsed}
    else:return
    u.unregister_slate_post_tick_callback(handle)
    Path(PROJECT_ROOT + '/Saved/street-travel-host-test.json').write_text(json.dumps(result,indent=2))
    print('TRAVEL_TEST_DONE',result)
handle=u.register_slate_post_tick_callback(tick)
