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

Give every entity a real **ECS collider component** shaped as an oriented bounding box (OBB), then use Jolt's `CastRay` for picking. Jolt's `CastRay` performs a proper ray-vs-shape intersection test that respects the box's position, orientation, and scale, giving pixel-accurate selection.

The key simplification over the earlier design: OBBs are **normal ECS collider components** routed through the existing `ecs_componentmanager_add` + `colliderbatchqueue` pipeline — not hand-rolled Jolt bodies managed by the workbench. This reuses all existing body-creation, broadphase-insertion, debug-draw, and cleanup machinery, and eliminates the need for any raycast layer filters.

### Key design decisions

| Decision | Rationale |
|----------|-----------|
| **OBB = real `ecs_component_collider_t` (cube shape)** | Reuses existing collider pipeline (`ecs_componentmanager_add` → `colliderbatchqueue` → Jolt). No manual `JPH_BodyInterface` calls in workbench; debug draw and body cleanup are automatic. |
| **OBBs created once at `workbench_init()`, permanent** | Avoids per-toggle create/destroy cost; toggling is free. No toggle-scoped lifecycle to manage. |
| **Only entities without a Static/Dynamic collider get an OBB** | Entities that already have a sim collider are raycastable as-is. Kinematic-only entities (`JPH_CharacterVirtual`) aren't in the broadphase, so they still receive an OBB. |
| **Dedicated workbench object layer (dynamic, internal)** | OBBs don't collide with the player in the sim (no collision pairs registered vs `GB_PLAYER`). Game code (`main.c`, `game.h`) remains unaware of the layer. |
| **`entity_id` added to `ecs_collider_jolt_userdata_t`** | Raycast hit → `GetUserData(bodyID)` → `entity_id` directly. Additive, backward-compatible field; useful beyond picking. |
| **Zero raycast filters** | Picking is unfiltered (every entity has a collider). Parkour is unfiltered and treats OBB hits as inert (see §3.6). No `ObjectLayerFilter` helpers needed. |
| **`physics_sys_jolt_raycast` signature unchanged** | No filter parameter; both call sites stay exactly as they are. |
| **Scale → half-extent mapping** | `transform.scale` used directly as cube half-extents. `{1,1,1}` → 2×2×2 OBB. Generous for picking accuracy. |

---

## 3. Technical Breakdown

### 3.1 Object Layer Registration

The workbench needs its own Jolt object layer so OBB colliders don't interact with gameplay physics in the simulation.

**Why this is needed**: Jolt's `JPH_BroadPhaseLayerInterfaceTable` is allocated *once* during `physics_sys_jolt_start_simulation()` with a fixed capacity equal to the current `objectlayer_count`. It cannot be resized or appended to after the physics system is running. `workbench_init()` runs *after* the game has already started physics (the broadphase table is already locked), so the workbench has no way to slot in a new layer at that point.

The alternative — declaring `WORKBENCH_OBJECT_LAYER` up front inside `poggen_register_physics_rules()` in `src/main.c` — would work mechanically, but it couples vedanta's game code to an internal concern of poglib's workbench. The whole point of this design is that `main.c` and `game.h` remain unaware that the workbench exists.

Pre-sizing the table to `MAX_COLLISION_INTERACTABILITY_ENTRIES` (32) at creation time reserves headroom, so the workbench can register its layer dynamically at `workbench_init()` without any game-code change. The cost is a fixed ~32-entry table instead of ~7 entries — negligible, and it buys back the decoupling.

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

> **Note on raycasts**: Jolt's collision matrix (`ObjectLayerPairFilterTable`, populated by `physics_sys_jolt_set_interaction_rules`) gates **simulation collision pairs only** — it is *not* consulted by `CastRay`. The workbench layer being non-colliding with `GB_PLAYER` means OBBs won't push the player around in the sim; it does **not** make OBBs invisible to raycasts. Raycast layering is handled by an `ObjectLayerFilter` callback — which this design deliberately avoids (see §3.5, §3.6).

### 3.2 OBB Collider Creation

At `workbench_init()`, after registering the layer, iterate all entities and add a cube collider component to any entity that lacks a Static/Dynamic collider:

```
for each active entity with ECS_CMP_TRANSFORM (skip world camera entity):
    query existing components (ECS_CMP_COLLIDER)
    if entity already has a Static or Dynamic collider:
        skip  // already raycastable
    // entity has no sim collider (or only a Kinematic/CharacterVirtual one) -> add OBB
    ecs_componentmanager_add(ecs, entity.id,
        .signature = ECS_CMP_COLLIDER,
        .collider = {
            .shape_type       = COLLIDER_SHAPE_TYPE_CUBE,
            .motion_type      = JPH_MotionType_Static,
            .object_layer_type = WORKBENCH_OBJECT_LAYER,
            .dim.cube         = { transform.scale.x, transform.scale.y, transform.scale.z },
            .internal.position    = transform.position,
            .internal.orientation = transform.orientation,
        })
```

No manual Jolt API calls. `ecs_componentmanager_add` (`component.h:163`) enqueues the collider in the `colliderbatchqueue`; the existing per-frame flush `ecs_componentmanager_update` → `colliderbatchqueue_upload_to_jolt` (`component.h:391`, `colliderbatchqueue.h:60`) handles `JPH_BoxShape_Create`, `CreateAndAddBody`, broadphase insertion, and user-data assignment. `OptimizeBroadPhase` is invoked automatically after static bodies drain (see `colliderbatchqueue.h:166`).

**Scale → half-extent mapping**: `transform.scale` is used directly as half-extents. A `{1, 1, 1}` scale entity gets a 2×2×2 OBB. A `{1, 2.5, 1}` wall gets a 2×5×2 OBB. This is generous for picking accuracy, which is desirable.

**Why skip entities with a Static/Dynamic collider**: they already have a body in the broadphase that `CastRay` can hit, so an OBB would be redundant and could occlude the real collider during picking. Kinematic bodies (`JPH_CharacterVirtual`) are **not** in the broadphase, so kinematic-only entities still receive an OBB.

### 3.3 Raycast Function — Unchanged

`physics_sys_jolt_raycast` keeps its current signature and behavior:

```c
JPH_RayCastResult physics_sys_jolt_raycast(const vec3f_t ray_pos, const vec3f_t dir);
```

Internally it calls `JPH_NarrowPhaseQuery_CastRay(npq, origin, dir, &hit, NULL, NULL, NULL)` (`jolt-wrapper.h:342`) — all filters NULL, hits the closest body of any layer. This is exactly what picking wants now that every entity has a collider.

No call site changes. No new parameter. No backward-compatibility break.

### 3.4 Collider User Data — Add `entity_id`

To resolve a hit back to an entity, the picking code needs the `entity_id` from the struck body's Jolt user data. Currently `ecs_collider_jolt_userdata_t` (`types.h:189`) stores `objectlayertype` + `dimension` (+ a DEBUG-only collider pointer) but **not** the entity_id.

**Change** — add `entity_id` (additive, backward-compatible):

```c
struct ecs_collider_jolt_userdata_t {
    JPH_ObjectLayer             objectlayertype;
    collider_shape_dimension_t  dimension;
    u32                         entity_id;          // NEW
#ifdef DEBUG
    struct {
        ecs_component_collider_t *ecs_collider;
    } internal;
#endif
};
```

**Wiring**: `colliderbatchqueue_add` gains an `entity_id` parameter (the id lives in the component pool entry header `ecs_component_poolentry_t.entity_id`, `types.h:209`, not in the collider struct itself). `colliderbatchqueue_upload_to_jolt` (`colliderbatchqueue.h:138`) sets `.entity_id` when building the user data, alongside the existing `.objectlayertype` and `.dimension`.

This field is generally useful (any raycast hit can now report which entity it struck), not strictly a workbench concern.

### 3.5 Picking Rewrite

`workbench_editor__internal__check_mouse_closest_entity()` replaces the O(n) point-distance loop with a single unfiltered Jolt raycast:

```c
// Build world ray from mouse NDC (keep existing math at lines 42-59)
vec3f_t dir = /* ... same as current ... */;

JPH_RayCastResult hit = physics_sys_jolt_raycast(cam->position, glms_vec3_scale(dir, 1000.0f));

if (hit.bodyID) {
    const ecs_collider_jolt_userdata_t *ud =
        (ecs_collider_jolt_userdata_t *)JPH_BodyInterface_GetUserData(
            global_physics_sys_jolt_instance->bodyinterface, hit.bodyID);
    global_workbench->editor.mouse_closest_to_entity_id = ud->entity_id;
} else {
    global_workbench->editor.mouse_closest_to_entity_id = 0;
}
```

No filter. Every entity has a raycastable collider, so the closest hit is the entity under the cursor. The existing two-click confirmation logic (`prev_selected_entity_id` tracking) is unchanged.

### 3.6 Parkour Raycasts — Unfiltered, OBBs Treated as Inert

`player_parkour_update_state` (`collision-scene.h:632`) runs every frame in both edit and play mode, issuing unfiltered `physics_sys_jolt_raycast` calls. With permanent OBBs in the broadphase, parkour rays **will** hit workbench OBBs.

This is safe because of how parkour consumes hits (`collision-scene.h:699-725`): it casts the user data to `ecs_collider_jolt_userdata_t*` and compares `objectlayertype` against the gameplay enums (`GB_VAULTABLE`, `GB_CLIMBABLE_1M`, `GB_CLIMBABLE_2M`, `GB_WALLRUNABLE`). A workbench OBB's `objectlayertype` is `WORKBENCH_OBJECT_LAYER`, which matches **none** of these — so every `allowed_action.*` flag stays false and the OBB is treated as inert. No crash: OBBs carry a real `ecs_collider_jolt_userdata_t` (same struct as gameplay colliders), unlike the earlier raw-`entity_id` design.

**Accepted trade-off — rare occlusion**: `CastRay` returns only the single closest hit. If a decorative prop's OBB sits within 1 unit of the player and a real vaultable/wallrunable collider is directly behind it, the parkour ray hits the OBB first and never reaches the gameplay collider, silently suppressing that action for that frame. This is expected to be rare in practice (decorative props are rarely placed within 1 unit of the player's path against a gameplay collider). If it surfaces, it can be addressed later (see §8) without rearchitecting.

No changes to `collision-scene.h`.

### 3.7 Debug Renderer — Automatic

`workbench__internal__show_colliders` (`workbench.h:669`) already iterates the ECS collider pool (`workbench.h:689`) and calls `JPH_PhysicsSystem_DrawBodies` for sim bodies. Because OBBs are now ordinary `ecs_component_collider_t` entries, they are drawn automatically by the existing code — no separate `DrawBodies` pass, no `JPH_BodyDrawFilter`, no special-case color logic. The workbench layer's static cube bodies render with the same `JPH_BodyManager_ShapeColor_InstanceColor` styling as other colliders.

### 3.8 Entity Lifecycle Hooks

Because OBBs are regular collider components, the existing editor action paths already do the right thing through `ecs_componentmanager_add` / `ecs_componentmanager_remove`:

| Editor action | OBB handling |
|--------------|--------------|
| `workbench_editor_copypaste_entity()` (Ctrl+C) | Duplicated entity gets an OBB via the normal component copy (add to `ECS_CMP_COLLIDER` if the source had a workbench OBB) |
| `workbench_editor_delete_entity()` (Delete) | `ecs_componentmanager_remove` runs `ecs_componentmanager__internal_cmp_cleanup` (`component.h:394`), which calls `JPH_BodyInterface_RemoveAndDestroyBody` (`component.h:400`) — the OBB body is destroyed automatically |
| Undo (restore deleted) | Component is re-added → re-enqueued → re-created by `colliderbatchqueue_upload_to_jolt` |

No workbench-specific body tracking. No ghost-body edge case: the ECS owns the collider, and ECS removal always destroys the Jolt body.

---

## 4. Performance Implications

| Aspect | Impact |
|--------|--------|
| OBB collider creation (~1000 entities) | One-time cost at `workbench_init` (~1-5ms). Routed through existing batch queue; broadphase optimized after static drain. |
| Broadphase | ~N extra static bodies in spatial tree (N = entities lacking a sim collider). Negligible overhead for static bodies. |
| Picking (per frame, edit mode) | O(1) single raycast vs O(n) linear entity scan. **Faster** for large scenes. |
| Parkour raycasts (per frame, all modes) | Identical — unfiltered raycast, same cost as today. May hit OBBs but the comparison cost is unchanged. |
| Workbench toggle | **Zero** physics cost — OBBs are permanent, not toggled. |
| Memory | ~1KB per OBB body (~1MB for 1000 entities). Bodies + shapes live on Jolt's internal heap; collider components live in the ECS pool. |

---

## 5. Backward Compatibility

| Caller | Change |
|--------|--------|
| `physics_sys_jolt_raycast` signature | **Unchanged** — no filter parameter added |
| `collision-scene.h` parkour raycast call site | **Unchanged** |
| `ecs_collider_jolt_userdata_t` | Gains `entity_id` field — additive, existing readers unaffected |
| `colliderbatchqueue_add` | Gains `entity_id` parameter — internal API, updated at the single call site (`component.h:209`) |
| ECS collider pipeline | **Untouched** (OBBs flow through it like any other collider) |
| Workbench struct / editor state | **Untouched** |
| Game entity types (`vedanta_entity_type`) | **Untouched** |
| Physics rules registration in `main.c` | **Untouched** |

---

## 6. Files Summary

| File | Changes |
|------|---------|
| `physics/jolt-wrapper.h` | Pre-size broadphase tables to `MAX_COLLISION_INTERACTABILITY_ENTRIES` (32) instead of runtime count; add `physics_sys_jolt_register_object_layer()`. **No** raycast signature change; **no** filter helpers. |
| `util/workbench.h` | `workbench_init()`: register workbench layer + add OBB collider components to colliderless entities. `show_colliders()`: **unchanged** (draws OBBs automatically). `copypaste_entity()`/`delete_entity()`: OBB handled by existing ECS component add/remove. |
| `util/workbench/workbench-editor.h` | Replace `check_mouse_closest_entity()` point-distance loop with a single unfiltered `physics_sys_jolt_raycast` + `userdata->entity_id` read. |
| `util/workbench/constants.h` (new) | `WORKBENCH_OBJECT_LAYER` constant. |
| `ecs/component/types.h` | Add `entity_id` field to `ecs_collider_jolt_userdata_t`. |
| `ecs/component/colliderbatchqueue.h` | `colliderbatchqueue_add` gains `entity_id` param; `colliderbatchqueue_upload_to_jolt` sets `.entity_id` in the user data. |
| `ecs/component.h` | Pass `entry->entity_id` to `colliderbatchqueue_add` at the single call site (`component.h:209`). |
| *(vedanta)* `scenes/collision-scene.h` | **No changes** |
| *(vedanta)* `main.c` | **No changes** |
| *(vedanta)* `game.h` | **No changes** |

---

## 7. Risks & Mitigations

| Risk | Severity | Mitigation |
|------|----------|------------|
| Rare parkour occlusion by OBB | Low | Accepted for v1. OBB within 1 unit of player masking a gameplay collider behind it is uncommon. Revisit if it surfaces (§8). |
| OBB shapes don't match entity visuals | Low | Scale = half-extent is generous; can add scaling factor later if needed. |
| Broadphase table resize breaks existing collision | Low | Only increases capacity; existing mappings unchanged. |
| OBB not updated after transform edit | Low | Selection is per-click and OBB is for approximate picking; if entity moves significantly, re-selecting will use new body position (but body doesn't auto-update — acceptable for v1). |

---

## 8. Future Improvements

- **Auto-sync OBB on transform change**: In `workbench_editor__internal__slider_on_release()`, update the selected entity's OBB body via `JPH_BodyInterface_SetPositionAndRotation()` (or remove + re-add through the batch queue).
- **Revisit parkour occlusion if it surfaces**: if the rare-occlusion trade-off bites in practice, add an exclude-filter on the parkour raycast only (picking stays unfiltered). This is a 1-line change and does not require the full filter-helper apparatus.
- **Shape type selection**: Allow choosing between OBB, sphere, or capsule shapes per entity for better picking fits.
- **Convex hull from mesh**: For entities with `ECS_CMP_MODEL`, extract a convex hull from the model's vertex data for pixel-perfect picking.
