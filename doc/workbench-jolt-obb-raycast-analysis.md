# Workbench Jolt OBB Raycast Selection — Analysis Report

## 1. Problem Statement

### Current picking mechanism

Entity selection in the workbench editor (`workbench-editor.h:40`) uses a **point-distance ray check**:

```
for each entity with ECS_CMP_TRANSFORM:
    d = closest_point_on_ray(cam_pos, ray_dir, entity.position)
    if d < 5.0: pick this entity
```

The function `workbench_editor__internal_closest_point_on_ray` computes the perpendicular distance from the entity's **position point** to the mouse ray. It does not account for:

- Entity **scale** (a wall scaled `{1, 2.5, 1}` is treated identically to a point)
- Entity **shape** (cube, capsule, cylinder — all treated as a dimensionless point)

### Why this is a problem

- Large-scaled entities are hard to click because the actual visual bounds are much larger than the single point used for distance computation
- Entities with scale `{10, 0.1, 10}` (e.g., a ground plane) are nearly impossible to select because their transform position may be at their center, but the 5-unit threshold doesn't map cleanly
- Tightly packed entities are ambiguous — the closest-by-point-distance may not be the entity under the mouse cursor visually

---

## 2. Proposed Solution

Use **Jolt Physics raycasts** against oriented bounding box (OBB) colliders placed on every entity in the scene. Jolt's `CastRay` performs a proper ray-vs-shape intersection test that respects the box's position, orientation, and scale, giving pixel-accurate picking.

### Key design decisions

| Decision | Rationale |
|----------|-----------|
| **OBB bodies created once at `workbench_init()`** | Avoids per-toggle create/destroy cost; toggling is free |
| **OBBs are permanent, static bodies** | Zero per-frame physics simulation cost; only broadphase membership |
| **Dedicated object layer** (`WORKBENCH_OBJECT_LAYER`) | Isolates OBBs from gameplay physics — no collision pairs registered |
| **Object layer registered internally by workbench** | Game code (`main.c`, `game.h`) remains unaware of the layer |
| **Raycast extended with optional layer filter** | `physics_sys_jolt_raycast(origin, dir, filter)` — NULL = unfiltered (backward compatible) |
| **entity_id stored in Jolt user data** | Raycast hit → `GetUserData(bodyID)` → entity_id directly, no lookup tables needed |
| **OBB shape: `JPH_BoxShape` with body rotation** | Scale → half-extents, orientation from quaternion → naturally oriented box |
| **No data in `workbench_t` struct** | User data on bodies carries entity_id; no tracking arrays |

---

## 3. Technical Breakdown

### 3.1 Object Layer Registration

The workbench needs its own Jolt object layer so OBB bodies don't interact with gameplay physics.

**Problem**: `poggen_register_physics_rules()` in `src/main.c` currently creates broadphase tables with exact capacity (`objectlayer_count`). Adding layers after `physics_sys_jolt_start_simulation()` is impossible because the `JPH_BroadPhaseLayerInterfaceTable` is fixed-size.

**Solution**: In `physics_sys_jolt_set_interaction_rules()` (jolt-wrapper.h), resize the table creation to use `MAX_COLLISION_INTERACTABILITY_ENTRIES` (32) instead of the runtime count. This reserves capacity for dynamic layer registration. Add a new function:

```c
void physics_sys_jolt_register_object_layer(
    physics_sys_jolt_t *self,
    u16 object_layer,
    u16 broadphase_type
);
```

Called by `workbench_init()` to map `WORKBENCH_OBJECT_LAYER` → `PHY_BP_STATIC`.

**Implication**: No change required in `src/main.c` or `src/game.h`. The workbench initializes its own layer. Minor memory increase for broadphase tables (from ~7 entries to 32 entries — negligible).

### 3.2 OBB Body Creation

At `workbench_init()`, after registering the layer:

```
for each active entity with ECS_CMP_TRANSFORM (skip world camera entity 1):
    box = JPH_BoxShape_Create(half_extents = transform.scale, convex_radius)
    body = body_interface.CreateAndAddBody(
        shape = box,
        position = transform.position,
        rotation = transform.orientation (quaternion),
        motion_type = Static,
        object_layer = WORKBENCH_OBJECT_LAYER
    )
    JPH_BodyInterface_SetUserData(body_interface, body.id, (u64)entity_id)
```

The body's rotation (from entity's orientation quaternion) naturally orients the axis-aligned box shape, producing an OBB in world space.

**Scale → half-extent mapping**: `transform.scale` is used directly as half-extents. A `{1, 1, 1}` scale entity gets a 2×2×2 OBB. A `{1, 2.5, 1}` wall gets a 2×5×2 OBB. This is generous for picking accuracy, which is desirable.

### 3.3 Raycast Function Extension

Current signature:
```c
JPH_RayCastResult physics_sys_jolt_raycast(vec3f_t origin, vec3f_t dir);
```

Extended to:
```c
JPH_RayCastResult physics_sys_jolt_raycast(
    vec3f_t origin,
    vec3f_t dir,
    const JPH_ObjectLayerFilter *filter
);
```

When `filter == NULL`: passes `NULL` to `JPH_NarrowPhaseQuery_CastRay()` — hits everything (exact backward compatibility).
When `filter != NULL`: passes it to CastRay — only hits the filtered layer(s).

**Backward compatibility**: The single existing call site in `collision-scene.h:661` changes to `physics_sys_jolt_raycast(origin, dir, NULL)` to preserve unfiltered behavior.

### 3.4 Layer Filter Helpers

Since `JPH_ObjectLayerFilter_SetProcs` sets a global callback, a single proc handles both include and exclude modes via userData:

```c
typedef struct {
    enum { FILTER_INCLUDE, FILTER_EXCLUDE } mode;
    u16 layer;
} objlayer_filter_ctx_t;

// Single global callback
bool objlayer_filter_fn(void *ud, JPH_ObjectLayer layer) {
    objlayer_filter_ctx_t *ctx = ud;
    return (ctx->mode == FILTER_INCLUDE) ? (layer == ctx->layer) : (layer != ctx->layer);
}
```

Factory functions (lazily initialized, cached as file-scope statics):
- `physics_sys_jolt_objlayer_filter_include(u16 layer)` — for workbench picking raycast
- `physics_sys_jolt_objlayer_filter_exclude(u16 layer)` — for parkour raycasts

### 3.5 Picking Rewrite

`workbench_editor__internal__check_mouse_closest_entity()` replaces the O(n) point-distance loop with a single Jolt raycast:

```c
// Build world ray from mouse NDC (keep existing math at lines 42-59)
vec3f_t dir = /* ... same as current ... */;

JPH_ObjectLayerFilter *filter = physics_sys_jolt_objlayer_filter_include(WORKBENCH_OBJECT_LAYER);
JPH_RayCastResult hit = physics_sys_jolt_raycast(cam->position, dir * 1000.0f, filter);

if (hit.bodyID) {
    u64 ud = JPH_BodyInterface_GetUserData(bodyinterface, hit.bodyID);
    global_workbench->editor.mouse_closest_to_entity_id = (u32)ud;
} else {
    global_workbench->editor.mouse_closest_to_entity_id = 0;
}
```

The existing two-click confirmation logic (`prev_selected_entity_id` tracking) is unchanged.

### 3.6 Parkour Raycast Protection

Without intervention, permanent OBB bodies in the broadphase would be hit by the game's parkour raycasts (`collision-scene.h:661`), occluding real gameplay colliders. The parkour code reads user data as `ecs_collider_jolt_userdata_t*` to check `objectlayertype`, which would crash or give wrong results on a workbench body (whose user data is just `entity_id`).

**Solution**: The parkour raycast line changes from:
```c
content->raycast_hits[idx] = physics_sys_jolt_raycast(ray_pos, dirs[idx]);
```
to:
```c
content->raycast_hits[idx] = physics_sys_jolt_raycast(ray_pos, dirs[idx],
    physics_sys_jolt_objlayer_filter_exclude(WORKBENCH_OBJECT_LAYER));
```

Workbench OBBs are invisible to parkour detection.

### 3.7 Debug Renderer — OBB Color Differentiation

When the "show colliders" toggle is active, workbench OBBs should render in a distinct color to differentiate them from gameplay colliders.

**Approach**: Use `JPH_PhysicsSystem_DrawBodies()` twice with `JPH_BodyDrawFilter`:

1. **First pass**: Draw gameplay bodies filtered to exclude `WORKBENCH_OBJECT_LAYER` — uses existing `JPH_BodyManager_ShapeColor_InstanceColor`
2. **Second pass**: Draw only workbench OBB bodies with a distinct style/color

The `BodyDrawFilter` callback uses `JPH_Body_GetObjectLayer(body)` to check the layer. The same include/exclude pattern as the ObjectLayerFilter is used.

### 3.8 Entity Lifecycle Hooks

Since OBB bodies are permanent, entity add/delete during editing must also manage OBBs:

| Editor action | OBB action |
|--------------|------------|
| `workbench_editor_copypaste_entity()` (Ctrl+C) | Create OBB body for duplicated entity |
| `workbench_editor_delete_entity()` (Delete) | Destroy OBB body for deleted entity |
| Undo (restore deleted) | Re-create OBB body (body already destroyed) |

**Edge case — ghost bodies**: If an OBB body's entity_id no longer exists in ECS (e.g., entity deleted but body not cleaned up), the picking raycast might return a stale entity_id. Mitigation: after `GetUserData()`, validate the entity_id exists in the ECS entity hashtable before using it.

---

## 4. Performance Implications

| Aspect | Impact |
|--------|--------|
| OBB body creation (~1000 entities) | One-time cost at init (~1-5ms). Static bodies, no runtime cost |
| Broadphase | ~1000 extra static bodies in spatial tree. Negligible overhead for static bodies |
| Picking (per frame) | O(1) single raycast vs O(n) linear entity scan. **Faster** for large scenes |
| Parkour raycasts (per frame) | Identical — filtered raycast has same cost as unfiltered |
| Workbench toggle | **Zero** physics cost — bodies already exist |
| Memory | ~1KB per OBB body (~1MB for 1000 entities). Bodies + shapes live on Jolt's internal heap |

---

## 5. Backward Compatibility

| Caller | Change |
|--------|--------|
| `collision-scene.h:661` (parkour raycast) | Add exclude-filter parameter (1-line change) |
| Any future `physics_sys_jolt_raycast(origin, dir)` callers | Must add 3rd parameter `NULL` to compile — unfiltered, identical behavior |
| ECS collider pipeline | **Untouched** |
| Workbench struct / editor state | **Untouched** |
| Game entity types (`vedanta_entity_type`) | **Untouched** |
| Physics rules registration in `main.c` | **Untouched** |

---

## 6. Files Summary

| File | Changes |
|------|---------|
| `physics/jolt-wrapper.h` | Resize broadphase tables to `MAX_COLLISION_INTERACTABILITY_ENTRIES`; add `register_object_layer()`; extend `raycast()` with optional filter param; add include/exclude filter factory helpers (ObjectLayerFilter + BodyDrawFilter) |
| `util/workbench.h` | `workbench_init()`: register layer + create OBBs; `show_colliders()`: separate DrawBodies pass for OBBs; `copypaste_entity()`: create OBB; `delete_entity()`: destroy OBB; picking redirect to editor |
| `util/workbench/workbench-editor.h` | Replace `check_mouse_closest_entity()` with filtered Jolt raycast |
| `util/workbench/constants.h` (new) | `WORKBENCH_OBJECT_LAYER`, `WORKBENCH_OBB_DEBUG_COLOR` |
| *(vedanta)* `scenes/collision-scene.h` | Add exclude filter to parkour raycast (in a follow-up PR) |
| *(vedanta)* `main.c` | **No changes** |
| *(vedanta)* `game.h` | **No changes** |

---

## 7. Risks & Mitigations

| Risk | Severity | Mitigation |
|------|----------|------------|
| OBB shapes don't match entity visuals | Low | Scale = half-extent is generous; can add scaling factor later if needed |
| Stale body ID after entity delete | Medium | Validate entity_id exists in ECS before using picked result |
| Broadphase table resize breaks existing collision | Low | Only increases capacity; existing mappings unchanged |
| `ObjectLayerFilter_SetProcs` global conflict | Low | Single proc with mode-switching covers all use cases; no other code uses this API |
| OBB body not updated after transform edit | Low | Selection is per-click and OBB is for approximate picking; if entity moves significantly, re-selecting will use new body position (but body doesn't auto-update — acceptable for v1) |

---

## 8. Future Improvements

- **Auto-sync OBB on transform change**: In `workbench_editor__internal__slider_on_release()`, call `JPH_BodyInterface_SetPositionAndRotation()` for the selected entity's OBB body
- **Shape type selection**: Allow user to choose between OBB, sphere, or capsule shapes per entity for even better picking fits
- **Convex hull from mesh**: For entities with `ECS_CMP_MODEL`, extract a convex hull from the model's vertex data for pixel-perfect picking
