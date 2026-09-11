from pathlib import Path
PROJECT_ROOT = Path(__file__).resolve().parents[2].as_posix()
"""Static geometry weapon-query checks, against the saved editor map."""
import unreal as u,json
from pathlib import Path
w=u.get_editor_subsystem(u.UnrealEditorSubsystem).get_editor_world()
cases=[('railing aperture',(-2300,0,470),(-2100,0,470),False),('railing top bar',(-2300,0,500),(-2100,0,500),True),('solid building',(-3300,-3900,250),(-3300,-2900,250),True),('second floor',(-1900,0,550),(-1900,0,300),True),('third floor',(0,-2100,950),(0,-2100,650),True),('open stair entrance',(-2400,-1700,495),(-1900,-1700,495),False)]
report=[]
for name,start,end,expected in cases:
 hit=u.SystemLibrary.line_trace_single(w,u.Vector(*start),u.Vector(*end),u.TraceTypeQuery.ECC_WEAPON_TRACE,False,[],u.DrawDebugTrace.FOR_DURATION,True,draw_time=30)
 report.append({'name':name,'start':start,'end':end,'expectedBlock':expected,'blocked':bool(hit),'pass':bool(hit)==expected,'hit':str(hit)})
Path(PROJECT_ROOT + '/Saved/street-collision-test.json').write_text(json.dumps(report,indent=2))
print('COLLISION_TEST_DONE',report)
