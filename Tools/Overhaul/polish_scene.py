import unreal as u, math
actors=u.get_editor_subsystem(u.EditorActorSubsystem)
levels=u.get_editor_subsystem(u.LevelEditorSubsystem)
cube=u.load_asset('/Engine/BasicShapes/Cube')
mats={n:u.load_asset('/Game/Street/Materials/M_'+n) for n in ['Glass','Stone','Metal','Roof','Brick','Wood']}
def detail(name,p,s,mat,rot=None):
    a=actors.spawn_actor_from_class(u.StaticMeshActor,u.Vector(*p),rot or u.Rotator())
    a.set_actor_label('Finish '+name);a.static_mesh_component.set_static_mesh(cube)
    a.static_mesh_component.set_material(0,mats[mat]);a.set_actor_scale3d(u.Vector(*(v/100 for v in s)))
    a.static_mesh_component.set_collision_profile_name('NoCollision');a.set_folder_path('Street/Finish')
    return a
for name in ['StreetArena','StreetLobby','StreetStart']:
    levels.load_level('/Game/Street/Maps/'+name)
    for old in actors.get_all_level_actors():
        if old.get_actor_label().startswith('Finish '): actors.destroy_actor(old)
    for a in list(actors.get_all_level_actors()):
        if isinstance(a,u.DirectionalLight):
            a.set_actor_rotation(u.Rotator(pitch=-25,yaw=70),False)
            a.light_component.set_editor_property('intensity',30000)
            a.light_component.set_light_color(u.LinearColor(1,0.72,0.48))
        elif isinstance(a,u.SkyLight): a.light_component.set_editor_property('intensity',0.65)
        elif isinstance(a,u.PostProcessVolume):
            s=a.get_editor_property('settings');s.auto_exposure_min_brightness=12;s.auto_exposure_max_brightness=12;a.set_editor_property('settings',s)
        label=a.get_actor_label()
        if label.startswith('Corner quoin'):
            actors.destroy_actor(a);continue
        if label.startswith('Stair landing'): a.set_actor_scale3d(u.Vector(7,9,0.3))
        if label.startswith('Rail') and name=='StreetArena':
            loc=a.get_actor_location();scale=a.get_actor_scale3d()
            if abs(abs(loc.x)-1600)<5 or abs(abs(loc.x)-2200)<5:
                if label.startswith('Rail post') and abs(loc.y)>1300: actors.destroy_actor(a);continue
                if scale.y>30: a.set_actor_scale3d(u.Vector(scale.x,26,scale.z))
        if label.startswith('Masonry block'):
            loc=a.get_actor_location();scale=a.get_actor_scale3d();w=scale.x*100;d=scale.y*100;h=scale.z*100
            for xx in [loc.x-w/2,loc.x+w/2]:
                for yy in [loc.y-d/2,loc.y+d/2]: detail('quoin',(xx,yy,h/2),(55,55,h),'Stone')
            for side in [-1,1]:
                xx=loc.x+side*(w/2+8)
                for y in range(int(loc.y-d/2+150),int(loc.y+d/2-80),280):
                    for z in range(200,int(h-90),350):
                        detail('side frame',(xx,y,z),(15,155,210),'Stone')
                        detail('side glass',(xx+side*10,y,z),(8,128,185),'Glass')
                        detail('side mullion',(xx+side*17,y,z),(8,7,185),'Metal')
            # Pitched roof with visible ridge and eaves.
            for sy in [-1,1]:
                detail('pitched roof',(loc.x,loc.y+sy*d/4,h+145),(w+90,d/2+80,24),'Roof',u.Rotator(roll=sy*17))
            detail('roof ridge',(loc.x,loc.y,h+260),(w+110,30,24),'Metal')
    # Far ground closes horizon holes outside compact halls.
    if name!='StreetArena':
        ground=detail('surrounding pavement',(0,0,-120),(18000,18000,120),'Stone')
        ground.static_mesh_component.set_collision_profile_name('NoCollision')
        if name=='StreetLobby':
            for x,y,w,d in [(0,-1725,3500,50),(-1725,0,50,3500),(1725,0,50,3500)]:
                wall=detail('courtyard boundary',(x,y,115),(w,d,230),'Brick')
                wall.static_mesh_component.set_collision_profile_name('BlockAll')
        for x in range(-5000,5001,2000):
            detail('distant skyline',(x,4800,1100),(1400,1400,2200),'Brick')
    if name=='StreetArena':
        for tag,p in [('Loot_Rifle',(-4600,1800,70)),('Loot_Carbine',(4600,-1800,70)),('Loot_Shotgun',(0,-2100,870))]:
            if not any(tag in [str(t) for t in a.tags] for a in actors.get_all_level_actors()):
                point=actors.spawn_actor_from_class(u.TargetPoint,u.Vector(*p));point.set_editor_property('tags',[tag])
    for a in actors.get_all_level_actors():
        if isinstance(a,u.CameraActor):
            a.set_actor_location(u.Vector(750,-1000,240),False,False)
            a.set_actor_rotation(u.Rotator(pitch=-4,yaw=108),False)
    levels.save_current_level()
u.EditorLevelLibrary.set_level_viewport_camera_info(u.Vector(750,-1000,240),u.Rotator(pitch=-4,yaw=108))
print('STREET_FINISH_COMPLETE')
