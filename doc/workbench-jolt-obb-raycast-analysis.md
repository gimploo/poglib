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

Replace the O(n) point-distance loop with a single **Jolt Physics raycast** against entities' **existing collider components**. Jolt's `CastRay` performs a proper ray-vs-shape intersection test that respects the collider's position, orientation, and dimensions, giving accurate picking for any entity that has a collider.

### Key design decisions

| Decision | Rationale |
|----------|-----------|
| **Raycast against existing colliders only** | No new bodies, no OBB creation, no workbench-specific layer. Reuses the colliders already in the broadphase. |
| **Colliderless entities are not pickable** | Entities without a collider component have nothing in Jolt to raycast against. This is an accepted limitation — most editable entities (walls, ground, obstacles) have colliders. |
| **`entity_id` stored on collider; `ecs_collider` pointer always available in userdata** | Raycast hit → `GetUserData(bodyID)` → `internal.ecs_collider` → `internal.entity_id`. No redundant data in userdata. |
| **`physics_sys_jolt_raycast` signature unchanged** | No filter parameter. Picking is unfiltered — closest hit wins. |
| **Zero filters, zero new layers, zero new bodies** | The simplest possible approach. No `ObjectLayerFilter` helpers, no broadphase table resize, no parkour changes. |

### What was considered and rejected

- **Creating OBB colliders for colliderless entities**: rejected — pollutes the ECS with collider components on entities that shouldn't have them, complicates serialization, and requires either a workbench-specific object layer (with broadphase table resizing) or an exclude-filter on parkour raycasts to avoid occlusion. Too much machinery for an editor convenience.
- **Math-based OBB intersection (no Jolt)**: rejected — requires writing and maintaining ray-vs-OBB intersection math when Jolt already does this better.
- **Layer filters**: rejected — Jolt provides no "disable these layers for raycasts" flag; the `ObjectLayerFilter` callback is the only mechanism, and it's unnecessary if we don't add extra bodies.

---

## 3. Technical Breakdown

### 3.1 Collider User Data — `entity_id` via collider pointer

To resolve a raycast hit back to an entity, the picking code needs the `entity_id` from the struck body's Jolt user data. Currently `ecs_collider_jolt_userdata_t` (`types.h:190`) stores `objectlayertype` + `dimension` and a DEBUG-only `internal.ecs_collider` pointer.

**Changes**:

1. Add `entity_id` to `ecs_component_collider_t.internal` — set during component creation in `ecs_componentmanager_add` (`component.h:209`), not in the batch queue.
2. Make `internal.ecs_collider` pointer in `ecs_collider_jolt_userdata_t` always available (remove `#ifdef DEBUG` guard) — it points back to the `ecs_component_collider_t`, which now carries `entity_id`.

```c
struct ecs_collider_jolt_userdata_t {
    JPH_ObjectLayer             objectlayertype;
    collider_shape_dimension_t  dimension;
    struct {
        ecs_component_collider_t *ecs_collider;
    } internal;
};
```

No `entity_id` field in the userdata — it's accessed via `userdata->internal.ecs_collider->internal.entity_id`, avoiding redundant storage.

**Wiring**: `ecs_componentmanager_add` sets `collider->internal.entity_id = entity_id` at component creation time. `colliderbatchqueue_upload_to_jolt` always sets `.internal.ecs_collider = collider` when building the user data. `colliderbatchqueue_add` is unchanged (no `entity_id` parameter).

### 3.2 Picking Rewrite

`workbench_editor__internal__check_mouse_closest_entity()` replaces the O(n) point-distance loop with a single unfiltered Jolt raycast:

```c
// Build world ray from mouse NDC (keep existing math)
vec3f_t dir = /* ... same as current ... */;

const vec3f_t ray_dir = glms_vec3_scale(dir, 1000.0f);
JPH_RayCastResult hit = physics_sys_jolt_raycast(cam->position, ray_dir);

if (hit.bodyID) {
    const ecs_collider_jolt_userdata_t *const userdata =
        (ecs_collider_jolt_userdata_t *)JPH_BodyInterface_GetUserData(
            global_physics_sys_jolt_instance->bodyinterface, hit.bodyID);
    picked = userdata->internal.ecs_collider->internal.entity_id;
} else {
    picked = 0;
}
```

No filter. The closest collider hit is the entity under the cursor. The existing two-click confirmation logic (`prev_selected_entity_id` tracking) is unchanged.

The old `workbench_editor__internal_closest_point_on_ray` helper is removed — it's no longer used.

### 3.3 Limitation — Colliderless Entities Not Pickable

Entities without an `ECS_CMP_COLLIDER` component (e.g., decorative props, lights, markers) have no body in Jolt's broadphase and therefore cannot be picked via raycast. This is an accepted trade-off for v1.

**Why this is acceptable**: the workbench editor is primarily used to place and adjust gameplay-relevant entities (walls, ground, obstacles, vaultables) — all of which have colliders. Decorative-only entities are typically placed once and rarely need re-selection.

**Future mitigation** (see §5): if colliderless entity picking becomes needed, an OBB collider can be added to those entities on a case-by-case basis, or a math-based fallback can be reintroduced for the colliderless subset only.

---

## 4. Performance Implications

| Aspect | Impact |
|--------|--------|
| Picking (per frame, edit mode) | O(1) single raycast vs O(n) linear entity scan. **Faster** for large scenes. |
| Parkour raycasts | **Unchanged** — same unfiltered raycast, no new bodies in broadphase. |
| Memory | Negligible — one extra `u32` per collider's user data. |
| Init / toggle cost | **Zero** — no new bodies created, no layers registered. |

---

## 5. Backward Compatibility

| Caller | Change |
|--------|--------|
| `physics_sys_jolt_raycast` signature | **Unchanged** |
| `collision-scene.h` parkour raycast call site | **Unchanged** |
| `ecs_collider_jolt_userdata_t` | `internal.ecs_collider` pointer made always available (not DEBUG-only); no `entity_id` field added |
| `colliderbatchqueue_add` | **Unchanged** — no new parameters |
| `ecs_component_collider_t.internal` | Gains `entity_id` field, set at component creation |
| ECS collider pipeline | **Untouched** |
| Workbench struct / editor state | **Untouched** |
| Game entity types (`vedanta_entity_type`) | **Untouched** |
| Physics rules registration in `main.c` | **Untouched** |

---

## 6. Files Summary

| File | Changes |
|------|---------|
| `ecs/component/types.h` | Add `entity_id` to `ecs_component_collider_t.internal`; make `ecs_collider` pointer in `ecs_collider_jolt_userdata_t` always available (remove DEBUG guard). |
| `ecs/component/colliderbatchqueue.h` | `upload_to_jolt` always sets `.internal.ecs_collider` (no DEBUG guard). `colliderbatchqueue_add` unchanged. |
| `ecs/component.h` | Set `collider->internal.entity_id = entity_id` at component creation, before adding to batch queue. |
| `util/workbench/workbench-editor.h` | Replace `check_mouse_closest_entity()` point-distance loop with single unfiltered `physics_sys_jolt_raycast` + `userdata->internal.ecs_collider->internal.entity_id` read. Remove `workbench_editor__internal_closest_point_on_ray`. |
| *(vedanta)* | **No changes** |

---

## 7. Risks & Mitigations

| Risk | Severity | Mitigation |
|------|----------|------------|
| Colliderless entities not pickable | Low | Accepted for v1. Most editable entities have colliders. Can add OBBs or math fallback later if needed. |
| OBB/collider not updated after transform edit | Low | Existing behavior — the collider body doesn't auto-sync when the transform is edited in the workbench. Separate from this change. |

---

## 8. Future Improvements

- **Picking for colliderless entities**: if needed, add OBB colliders to specific entity types, or reintroduce a math-based ray-vs-OBB check for the colliderless subset only (no Jolt bodies).
- **Auto-sync collider on transform change**: In `workbench_editor__internal__slider_on_release()`, update the selected entity's collider body via `JPH_BodyInterface_SetPositionAndRotation()`.
- **Shape-aware picking**: for entities with non-cube colliders (capsule, sphere, cylinder), the raycast already hits the actual shape — no change needed.
