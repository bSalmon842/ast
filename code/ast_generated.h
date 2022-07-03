#define Preproc_MemberOffset(structName, member) (u32)&((structName *)0)->member

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

IntrospectMemberDef introspected_Collider[] =
{
{ MemberType_ColliderType, "type", Preproc_MemberOffset(Collider, type), 1 },
{ MemberType_v3f, "origin", Preproc_MemberOffset(Collider, origin), 1 },
{ MemberType_v2f, "dims", Preproc_MemberOffset(Collider, dims), 1 },
{ MemberType_Array_CollisionInfo, "collisions", Preproc_MemberOffset(Collider, collisions), ColliderType_Count },
};

