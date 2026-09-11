import unreal as u
m=u.load_asset('/Game/Street/Materials/M_Paving')
u.MaterialEditingLibrary.delete_all_material_expressions(m)
p=u.MaterialEditingLibrary.create_material_expression(m,u.MaterialExpressionWorldPosition)
c=u.MaterialEditingLibrary.create_material_expression(m,u.MaterialExpressionCustom)
c.set_editor_property('output_type',u.CustomMaterialOutputType.CMOT_FLOAT3)
i=u.CustomInput();i.set_editor_property('input_name','P');c.set_editor_property('inputs',[i])
c.set_editor_property('code','float2 q=P.xy/float2(85,52); q.x+=fmod(floor(q.y),2)*0.5; float2 f=frac(q); float joint=step(0.018,f.x)*step(0.026,f.y); float v=frac(sin(dot(floor(q),float2(12.9898,78.233)))*43758.5453); return lerp(float3(0.11,0.10,0.085),float3(0.27,0.255,0.22)*(0.87+v*0.24),joint);')
u.MaterialEditingLibrary.connect_material_expressions(p,'',c,'P');u.MaterialEditingLibrary.connect_material_property(c,'',u.MaterialProperty.MP_BASE_COLOR)
u.MaterialEditingLibrary.recompile_material(m);u.EditorAssetLibrary.save_loaded_asset(m)
for a in u.get_editor_subsystem(u.EditorActorSubsystem).get_all_level_actors():
    if isinstance(a,u.CameraActor):
        a.camera_component.set_field_of_view(55)
        a.camera_component.set_editor_property('constrain_aspect_ratio',False)
        a.set_actor_location(u.Vector(750,-720,180),False,False)
        a.set_actor_rotation(u.Rotator(pitch=-3,yaw=112),False)
u.get_editor_subsystem(u.LevelEditorSubsystem).save_current_level()
print('PAVING_AND_CAMERA_SAVED')
