#ifndef CMT_GENERATED_H
#define Preproc_MemberOffset(structName, member) (u32)&((structName *)0)->member

IntrospectEnumDef introspected_EntityType[] =
{
{ EnumType_u16, "Entity_Null", 0x0000000000000000 },
{ EnumType_u16, "Entity_Player", 0x0000000000000001 },
{ EnumType_u16, "Entity_Shot_Player", 0x0000000000000002 },
{ EnumType_u16, "Entity_Shot_UFO", 0x0000000000000003 },
{ EnumType_u16, "Entity_Asteroids", 0x0000000000000004 },
{ EnumType_u16, "Entity_UFO", 0x0000000000000005 },
{ EnumType_u16, "Entity_Debug_Wall", 0x0000000000000006 },
{ EnumType_u16, "Entity_Count", 0x0000000000000007 },
};

IntrospectEnumDef GetEnumDef_EntityType(u64 value)
{
IntrospectEnumDef result = {};

for (u64 i = 0; i < ARRAY_COUNT(introspected_EntityType); ++i)
{
if (introspected_EntityType[i].value == value)
{
result = introspected_EntityType[i];
break;
}
}

return result;
}

IntrospectMemberDef introspected_Entity[] =
{
{ MemberType_EntityType, "type", Preproc_MemberOffset(Entity, type), 1 },
{ MemberType_b32, "active", Preproc_MemberOffset(Entity, active), 1 },
{ MemberType_s32, "index", Preproc_MemberOffset(Entity, index), 1 },
{ MemberType_Collider, "collider", Preproc_MemberOffset(Entity, collider), 1 },
{ MemberType_f32, "angle", Preproc_MemberOffset(Entity, angle), 1 },
{ MemberType_f32, "dA", Preproc_MemberOffset(Entity, dA), 1 },
{ MemberType_v3f, "pos", Preproc_MemberOffset(Entity, pos), 1 },
{ MemberType_v2f, "dP", Preproc_MemberOffset(Entity, dP), 1 },
{ MemberType_v3f, "newPos", Preproc_MemberOffset(Entity, newPos), 1 },
{ MemberType_v2f, "dims", Preproc_MemberOffset(Entity, dims), 1 },
{ MemberType_void, "extraInfo", Preproc_MemberOffset(Entity, extraInfo), 1 },
};

IntrospectEnumDef introspected_ColliderType[] =
{
{ EnumType_u16, "ColliderType_None", 0x0000000000000000 },
{ EnumType_u16, "ColliderType_Player", 0x0000000000000001 },
{ EnumType_u16, "ColliderType_Asteroid", 0x0000000000000002 },
{ EnumType_u16, "ColliderType_UFO", 0x0000000000000003 },
{ EnumType_u16, "ColliderType_Shot_Player", 0x0000000000000004 },
{ EnumType_u16, "ColliderType_Shot_UFO", 0x0000000000000005 },
{ EnumType_u16, "ColliderType_Debug_Wall", 0x0000000000000006 },
{ EnumType_u16, "ColliderType_Debug_Particle", 0x0000000000000007 },
{ EnumType_u16, "ColliderType_Count", 0x0000000000000008 },
};

IntrospectEnumDef GetEnumDef_ColliderType(u64 value)
{
IntrospectEnumDef result = {};

for (u64 i = 0; i < ARRAY_COUNT(introspected_ColliderType); ++i)
{
if (introspected_ColliderType[i].value == value)
{
result = introspected_ColliderType[i];
break;
}
}

return result;
}

IntrospectMemberDef introspected_Collider[] =
{
{ MemberType_ColliderType, "type", Preproc_MemberOffset(Collider, type), 1 },
{ MemberType_v3f, "origin", Preproc_MemberOffset(Collider, origin), 1 },
{ MemberType_v2f, "dims", Preproc_MemberOffset(Collider, dims), 1 },
{ MemberType_Array_CollisionInfo, "collisions", Preproc_MemberOffset(Collider, collisions), ColliderType_Count },
};

#define GetEnumDef(type, value) GetEnumDef_##type##(value)

#define CMT_GENERATED_H
#endif // CMT_GENERATED_H
