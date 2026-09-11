from pathlib import Path
PROJECT_ROOT = Path(__file__).resolve().parents[2].as_posix()
"""Run inside UE5.6 editor. Creates only /Game/Street assets and maps.
Re-running rebuilds our three generated worlds, never the legacy maps.
"""
import unreal as u, math, json, traceback
from pathlib import Path

ROOT='/Game/Street'
tools=u.AssetToolsHelpers.get_asset_tools()
actors=u.get_editor_subsystem(u.EditorActorSubsystem)
levels=u.get_editor_subsystem(u.LevelEditorSubsystem)
materials={}
for folder in ['Maps','Materials','Meshes','UI','Audio']:
    u.EditorAssetLibrary.make_directory(ROOT+'/'+folder)

def material(name,color,brick=False,glow=False,metal=0):
    path=ROOT+'/Materials/M_'+name
    if u.EditorAssetLibrary.does_asset_exist(path): return u.load_asset(path)
    mat=tools.create_asset('M_'+name,ROOT+'/Materials',u.Material,u.MaterialFactoryNew())
    c=u.MaterialEditingLibrary.create_material_expression(mat,u.MaterialExpressionConstant3Vector)
    c.set_editor_property('constant',u.LinearColor(*color,1))
    output=c
    if brick:
        pos=u.MaterialEditingLibrary.create_material_expression(mat,u.MaterialExpressionWorldPosition)
        custom=u.MaterialEditingLibrary.create_material_expression(mat,u.MaterialExpressionCustom)
        custom.set_editor_property('output_type',u.CustomMaterialOutputType.CMOT_FLOAT3)
        inp=u.CustomInput(); inp.set_editor_property('input_name','P')
        col=u.CustomInput(); col.set_editor_property('input_name','C')
        custom.set_editor_property('inputs',[inp,col])
        custom.set_editor_property('code','float2 q=float2(P.x+P.y,P.z)/float2(55,23); q.x+=fmod(floor(q.y),2)*0.5; float2 f=frac(q); float mortar=step(0.045,f.x)*step(0.085,f.y); float v=0.86+0.14*frac(sin(dot(floor(q),float2(12.9898,78.233)))*43758.5453); return lerp(float3(0.24,0.20,0.16),C*v,mortar);')
        u.MaterialEditingLibrary.connect_material_expressions(pos,'',custom,'P')
        u.MaterialEditingLibrary.connect_material_expressions(c,'',custom,'C')
        output=custom
    u.MaterialEditingLibrary.connect_material_property(output,'',u.MaterialProperty.MP_BASE_COLOR)
    if glow: u.MaterialEditingLibrary.connect_material_property(c,'',u.MaterialProperty.MP_EMISSIVE_COLOR)
    r=u.MaterialEditingLibrary.create_material_expression(mat,u.MaterialExpressionConstant)
    r.set_editor_property('r',0.38 if metal else 0.82)
    u.MaterialEditingLibrary.connect_material_property(r,'',u.MaterialProperty.MP_ROUGHNESS)
    if metal:
        m=u.MaterialEditingLibrary.create_material_expression(mat,u.MaterialExpressionConstant);m.set_editor_property('r',metal)
        u.MaterialEditingLibrary.connect_material_property(m,'',u.MaterialProperty.MP_METALLIC)
    u.MaterialEditingLibrary.recompile_material(mat)
    u.EditorAssetLibrary.save_loaded_asset(mat)
    return mat

for name,color,brick,glow,metal in [
 ('Brick',(0.43,0.20,0.11),True,False,0),('Ochre',(0.60,0.40,0.22),True,False,0),
 ('Stone',(0.65,0.57,0.43),False,False,0),('Paving',(0.25,0.25,0.23),False,False,0),
 ('Metal',(0.055,0.09,0.095),False,False,0.65),('Roof',(0.12,0.17,0.18),False,False,0.3),
 ('Glass',(0.08,0.20,0.22),False,False,0.45),('Amber',(1.0,0.58,0.15),False,True,0),
 ('Red',(0.55,0.065,0.025),False,False,0),('Leaf',(0.16,0.24,0.055),False,False,0),
 ('Wood',(0.24,0.12,0.055),False,False,0)]: materials[name]=material(name,color,brick,glow,metal)

cube=u.load_asset('/Engine/BasicShapes/Cube'); cylinder=u.load_asset('/Engine/BasicShapes/Cylinder'); sphere=u.load_asset('/Engine/BasicShapes/Sphere')
counts={}
def mesh(label,loc,size,mat='Stone',shape=None,collision=True,rotation=None):
    a=actors.spawn_actor_from_class(u.StaticMeshActor,u.Vector(*loc),rotation or u.Rotator())
    a.set_actor_label(label)
    c=a.static_mesh_component
    c.set_static_mesh(shape or cube); c.set_material(0,materials[mat])
    a.set_actor_scale3d(u.Vector(*(s/100 for s in size)))
    c.set_collision_profile_name('BlockAll' if collision else 'NoCollision')
    # Small facade dressing must never retract the third-person camera.
    a.set_folder_path('Street/'+('Structure' if collision else 'Details'))
    return a

def rail(x,y,z,length,along='x'):
    for h in [38,100]: mesh('Rail', (x,y,z+h),(length,7,7) if along=='x' else (7,length,7),'Metal',collision=True)
    for t in range(-int(length/2),int(length/2)+1,140):
        mesh('Rail post',(x+t if along=='x' else x,y if along=='x' else y+t,z+50),(9,9,100),'Metal')

def stairs(x,y,z,height,run=720,width=260,axis=1):
    n=math.ceil(height/18)
    for i in range(n):
        h=(i+1)*height/n
        mesh('Stair tread',(x+axis*(i+0.5)*run/n,y,z+h/2),(run/n+0.2,width,h),'Stone')

def lamp(x,y,z=0):
    mesh('Lamp base',(x,y,z+12),(65,65,24),'Metal')
    mesh('Lamp post',(x,y,z+160),(14,14,300),'Metal',cylinder)
    mesh('Lamp lantern',(x,y,z+325),(68,68,70),'Amber',collision=False)
    mesh('Lamp cap',(x,y,z+370),(90,90,12),'Metal',collision=False)

def tree(x,y,z=0):
    mesh('Planter',(x,y,z+40),(170,170,80),'Stone')
    mesh('Tree trunk',(x,y,z+180),(24,24,280),'Wood',cylinder)
    for dx,dy,dz,sz in [(0,0,400,260),(-55,20,340,200),(60,-20,370,210)]:
        mesh('Tree crown',(x+dx,y+dy,z+dz),(sz,sz,sz),'Leaf',sphere,False)

def sign(text,x,y,z,size=80,yaw=-90):
    a=actors.spawn_actor_from_class(u.TextRenderActor,u.Vector(x,y,z),u.Rotator(yaw=yaw))
    a.set_actor_label('Sign '+text)
    c=a.text_render;c.set_text(text);c.set_world_size(size)
    c.set_horizontal_alignment(u.HorizTextAligment.EHTA_CENTER)
    c.set_text_render_color(u.Color(240,220,175,255))
    return a

def building(x,y,w=1400,d=1100,h=1050,variant=0):
    mat='Brick' if variant%2 else 'Ochre'
    mesh('Masonry block',(x,y,h/2),(w,d,h),mat)
    mesh('Foundation',(x,y,45),(w+40,d+40,90),'Stone')
    for z in [370,740,h]: mesh('Cornice',(x,y,z),(w+65,d+65,30),'Stone')
    mesh('Roof parapet',(x,y,h+55),(w+90,d+90,80),'Roof')
    # Facades on both street sides, with recess-colored windows and lintels.
    for sy in [-1,1]:
        fy=y+sy*(d/2+5)
        for xx in range(int(x-w/2+170),int(x+w/2-60),270):
            for zz in [205,560,905]:
                if zz>h-70: continue
                mesh('Window frame',(xx,fy,zz),(155,14,215),'Stone',collision=False)
                mesh('Window glazing',(xx,fy+sy*9,zz+3),(128,8,185),'Glass',collision=False)
                mesh('Window mullion',(xx,fy+sy*15,zz),(7,8,187),'Metal',collision=False)
                mesh('Window sill',(xx,fy+sy*20,zz-110),(178,52,16),'Stone',collision=False)
        mesh('Shop awning',(x,fy+sy*115,270),(w-80,250,25),'Roof',collision=False,rotation=u.Rotator(pitch=0,roll=sy*8))
    for xx in [x-w/2,x+w/2]:
        for yy in [y-d/2,y+d/2]: mesh('Corner quoin',(xx,yy,h/2),(55,55,h),'Stone',collision=False)

def light_scene():
    sun=actors.spawn_actor_from_class(u.DirectionalLight,u.Vector(0,0,2500),u.Rotator(pitch=-32,yaw=-42))
    sun.light_component.set_editor_property('intensity',50000.0)
    sun.light_component.set_editor_property('atmosphere_sun_light',True)
    sun.light_component.set_mobility(u.ComponentMobility.MOVABLE)
    sun.light_component.set_editor_property('light_color',u.Color(255,215,164,255))
    sky=actors.spawn_actor_from_class(u.SkyLight,u.Vector(0,0,2000))
    sky.light_component.set_mobility(u.ComponentMobility.MOVABLE)
    sky.light_component.set_editor_property('intensity',1.8)
    sky.light_component.set_editor_property('real_time_capture',True)
    actors.spawn_actor_from_class(u.SkyAtmosphere,u.Vector())
    fog=actors.spawn_actor_from_class(u.ExponentialHeightFog,u.Vector(0,0,-250))
    fog.component.set_editor_property('fog_density',0.008)
    pp=actors.spawn_actor_from_class(u.PostProcessVolume,u.Vector())
    pp.set_editor_property('unbound',True)
    settings=pp.get_editor_property('settings')
    settings.set_editor_property('override_auto_exposure_min_brightness',True)
    settings.set_editor_property('override_auto_exposure_max_brightness',True)
    settings.set_editor_property('auto_exposure_min_brightness',11.0)
    settings.set_editor_property('auto_exposure_max_brightness',11.0)
    pp.set_editor_property('settings',settings)

def new_map(name):
    world=u.EditorLoadingAndSavingUtils.new_blank_map(False)
    if not u.EditorLoadingAndSavingUtils.save_map(world,ROOT+'/Maps/'+name): raise RuntimeError('Cannot create '+name)
    light_scene()
def save(name):
    counts[name]=len(actors.get_all_level_actors())
    levels.save_current_level()
    print('STREET_MAP_SAVED',name,counts[name])

def arena():
    new_map('StreetArena')
    # Open center is 2 m lower than the surrounding streets.
    mesh('Lower plaza',(0,0,-240),(12000,10000,80),'Paving')
    mesh('North street',(0,3000,-100),(12000,4000,200),'Paving')
    mesh('South street',(0,-3000,-100),(12000,4000,200),'Paving')
    mesh('West street',(-4250,0,-100),(3500,2000,200),'Paving')
    mesh('East street',(4250,0,-100),(3500,2000,200),'Paving')
    # 2 m stair access down to the plaza on both flanks.
    stairs(-2500,0,-200,200,800,600,-1);stairs(2500,0,-200,200,800,600,1)
    for y in [-1800,1800]:
        # paved slope across the center approaches
        mesh('Plaza approach',(0,y/1.8,-100),(800,1700,45),'Paving',rotation=u.Rotator(roll=math.degrees(math.atan(200/1700))*(1 if y>0 else -1)))
    for x in [-3300,3300]:
        for y in [-2950,2950]: building(x,y,1700,1600,1100,1 if x<0 else 0)
    # two elevated routes run north/south; both ends have stairs.
    for x in [-1900,1900]:
        mesh('Second floor walkway',(x,0,385),(600,3500,30),'Stone')
        for y in [-1500,0,1500]: mesh('Arcade pier',(x,y,0),(95,95,800),'Stone')
        rail(x-300,0,400,2600,'y'); rail(x+300,0,400,2600,'y')
        for y in [-1700,1700]:
            stairs(x+(-720 if x<0 else 720),y,0,400,720,300,1 if x<0 else -1)
    # Two third-floor terraces, reached independently from either side.
    for y in [-2100,2100]:
        mesh('Upper terrace',(0,y,785),(1500,700,30),'Stone')
        for x in [-650,650]: mesh('Terrace pier',(x,y,400),(100,100,800),'Stone')
        rail(0,y-350,800,1500);rail(0,y+350,800,1500)
        for x in [-1,1]:
            mesh('Stair landing',(x*1400,y,385),(700,900,30),'Stone')
            stairs(x*1450,y,400,400,700,280,-x)
    # Plaza fountain is solid cover, with an open loop around it.
    mesh('Fountain plinth',(0,0,-150),(580,580,100),'Stone',cylinder)
    mesh('Fountain bowl',(0,0,-75),(450,450,70),'Metal',cylinder)
    mesh('Fountain centerpiece',(0,0,30),(85,85,180),'Stone',cylinder)
    for x,y in [(-850,-700),(900,600),(-4400,-1100),(4400,1100),(-4500,2200),(4500,-2200)]:
        mesh('Cover planter',(x,y,55 if abs(x)>2500 else -145),(260,140,110),'Stone')
    for x in [-5500,5500]:
        for y in [-4000,-1800,1800,4000]: tree(x,y);lamp(x+230,y+220)
    for y in [-4750,4750]:
        for x in [-4000,-1500,1500,4000]: lamp(x,y)
    # Surrounding facades form a visible boundary, not invisible blocking volumes.
    for x in range(-5000,5001,2000):
        building(x,-5650,1900,1100,1350,0);building(x,5650,1900,1100,1450,1)
    for x in [-6200,6200]: mesh('Boundary masonry',(x,0,450),(400,10000,900),'Brick')
    sign('STATION',0,4850,650,140)
    # Spawn shelter and eight distributed starts.
    for i,(x,y) in enumerate([(-4900,-3800),(-4900,3800),(4900,-3800),(4900,3800),(-1000,-4250),(1000,4250),(-4800,0),(4800,0)]):
        start=actors.spawn_actor_from_class(u.PlayerStart,u.Vector(x,y,110),u.Rotator(yaw=math.degrees(math.atan2(-y,-x))))
        start.set_editor_property('tags',['DistributedSpawn'])
        mesh('Spawn screen',(x+(250 if x<0 else -250),y,125),(45,350,250),'Brick')
    for i,(x,y,z) in enumerate([(-900,0,-130),(900,0,-130),(-1900,0,470),(1900,0,470),(0,-2100,870),(0,2100,870),(-4600,2500,70),(4600,-2500,70),(0,4000,70)]):
        a=actors.spawn_actor_from_class(u.TargetPoint,u.Vector(x,y,z))
        a.set_editor_property('tags',[['Loot_Health','Loot_Ammo','Loot_Speed'][i%3]])
    gm=u.load_class(None,'/Game/Blueprints/Character/BP_BlasterGameMode.BP_BlasterGameMode_C')
    u.EditorLevelLibrary.get_editor_world().get_world_settings().set_editor_property('default_game_mode',gm)
    save('StreetArena')

def small(name,lobby):
    new_map(name)
    width=3500 if lobby else 2400
    mesh('Courtyard',(0,0,-40),(width,width,80),'Paving')
    building(0,1700,2200,1000,1050,0)
    building(-1900,0,900,2400,1000,1);building(1900,0,900,2400,1000,1)
    for x in [-900,900]:
        tree(x,500);lamp(x,-400)
        mesh('Bench seat',(x,-700,65),(320,80,18),'Wood')
        for xx in [-120,120]:mesh('Bench leg',(x+xx,-700,30),(18,60,60),'Metal')
        mesh('Bench back',(x,-735,110),(320,12,85),'Wood',collision=False)
    mesh('Station canopy',(0,700,480),(1700,600,35),'Roof')
    for x in [-780,780]:mesh('Canopy column',(x,700,235),(28,28,470),'Metal')
    mesh('Station fascia',(0,420,450),(1650,25,90),'Metal')
    sign('STATION / 07',0,400,475,65)
    sign('DEPARTURES' if lobby else 'BLASTER',0,1185,700,90)
    for x in [-1,1]:
        mesh('Banner',(x*1100,1120,600),(120,20,340),'Red',collision=False)
    for i in range(8 if lobby else 1):
        a=actors.spawn_actor_from_class(u.PlayerStart,u.Vector(-550+(i%4)*360,-800+(i//4)*400,110),u.Rotator(yaw=90))
    mode='/Script/Blaster.LobbyGameMode' if lobby else '/Script/Blaster.StreetStartMode'
    u.EditorLevelLibrary.get_editor_world().get_world_settings().set_editor_property('default_game_mode',u.load_class(None,mode))
    if not lobby:
        camera=actors.spawn_actor_from_class(u.CameraActor,u.Vector(900,-1550,450),u.Rotator(pitch=-8,yaw=112))
        camera.set_editor_property('tags',['StartCamera'])
        # Display the existing character without a controller; no replacement art.
        char=actors.spawn_actor_from_class(u.load_class(None,'/Game/Blueprints/Character/BP_BlasterCharacter.BP_BlasterCharacter_C'),u.Vector(430,80,105),u.Rotator(yaw=-80))
        char.set_actor_label('Start character display')
    save(name)

try:
    arena();small('StreetLobby',True);small('StreetStart',False)
    task=u.AssetImportTask();task.filename=PROJECT_ROOT + '/Tools/Overhaul/Fonts/StreetBold.otf'
    task.destination_path=ROOT+'/UI';task.destination_name='StreetFont';task.automated=True;task.save=True
    task.factory=u.FontFileImportFactory();tools.import_asset_tasks([task])
    factory=u.WidgetBlueprintFactory();factory.set_editor_property('parent_class',u.load_class(None,'/Script/Blaster.StreetWidget'))
    if not u.EditorAssetLibrary.does_asset_exist(ROOT+'/UI/W_StreetScreen'):
        widget=tools.create_asset('W_StreetScreen',ROOT+'/UI',u.WidgetBlueprint,factory);u.EditorAssetLibrary.save_loaded_asset(widget)
    Path(PROJECT_ROOT + '/Saved/street-build.json').write_text(json.dumps(counts,indent=2))
    print('STREET_BUILD_COMPLETE',counts)
except Exception:
    traceback.print_exc()
    raise
