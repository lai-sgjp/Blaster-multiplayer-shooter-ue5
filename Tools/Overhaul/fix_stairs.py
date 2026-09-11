import unreal as u,math
actors=u.get_editor_subsystem(u.EditorActorSubsystem);levels=u.get_editor_subsystem(u.LevelEditorSubsystem)
levels.load_level('/Game/Street/Maps/StreetArena')
for a in list(actors.get_all_level_actors()):
 if a.get_actor_label()=='Movement stair ramp':actors.destroy_actor(a)
 elif a.get_actor_label().startswith('Stair tread'):
  loc=a.get_actor_location()
  if abs(abs(loc.y)-1700)<1 and 'StairEntryFixed' not in [str(t) for t in a.tags]:
   loc.x+=-300 if loc.x<0 else 300;a.set_actor_location(loc,False,False);a.set_editor_property('tags',['StairEntryFixed'])
  a.static_mesh_component.set_collision_response_to_channel(u.CollisionChannel.ECC_PAWN,u.CollisionResponseType.ECR_IGNORE)
def ramp(x,y,z,h,run,width,axis):
 a=actors.spawn_actor_from_class(u.StaticMeshActor,u.Vector(x+axis*run/2,y,z+h/2+10),u.Rotator(pitch=axis*math.degrees(math.atan2(h,run))))
 a.set_actor_label('Movement stair ramp');a.set_actor_hidden_in_game(True);a.set_folder_path('Street/Movement')
 c=a.static_mesh_component;c.set_static_mesh(u.load_asset('/Engine/BasicShapes/Cube'));a.set_actor_scale3d(u.Vector(math.hypot(run,h)/100,width/100,.12))
 c.set_collision_profile_name('Custom');c.set_collision_enabled(u.CollisionEnabled.QUERY_ONLY);c.set_collision_response_to_all_channels(u.CollisionResponseType.ECR_IGNORE);c.set_collision_response_to_channel(u.CollisionChannel.ECC_PAWN,u.CollisionResponseType.ECR_BLOCK)
for x in [-1900,1900]:
 for y in [-1700,1700]:ramp(x+(-1020 if x<0 else 1020),y,0,400,720,300,1 if x<0 else -1)
for y in [-2100,2100]:
 for x in [-1,1]:ramp(x*1450,y,400,400,700,280,-x)
ramp(-2500,0,-200,200,800,600,-1);ramp(2500,0,-200,200,800,600,1)
levels.save_current_level();print('STREET_MOVEMENT_RAMPS_SAVED')
