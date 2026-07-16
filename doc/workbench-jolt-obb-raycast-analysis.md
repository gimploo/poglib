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
| **`entity_id` added to `ecs_collider_jolt_userdata_t`** | Raycast hit → `GetUserData(bodyID)` → `entity_id` directly. Additive, backward-compatible field. |
| **`physics_sys_jolt_raycast` signature unchanged** | No filter parameter. Picking is unfiltered — closest hit wins. |
| **Zero filters, zero new layers, zero new bodies** | The simplest possible approach. No `ObjectLayerFilter` helpers, no broadphase table resize, no parkour changes. |

### What was considered and rejected

- **Creating OBB colliders for colliderless entities**: rejected — pollutes the ECS with collider components on entities that shouldn't have them, complicates serialization, and requires either a workbench-specific object layer (with broadphase table resizing) or an exclude-filter on parkour raycasts to avoid occlusion. Too much machinery for an editor convenience.
- **Math-based OBB intersection (no Jolt)**: rejected — requires writing and maintaining ray-vs-OBB intersection math when Jolt already does this better.
- **Layer filters**: rejected — Jolt provides no "disable these layers for raycasts" flag; the `ObjectLayerFilter` callback is the only mechanism, and it's unnecessary if we don't add extra bodies.

---

## 3. Technical Breakdown

### 3.1 Collider User Data — Add `entity_id`

To resolve a raycast hit back to an entity, the picking code needs the `entity_id` from the struck body's Jolt user data. Currently `ecs_collider_jolt_userdata_t` (`types.h:189`) stores `objectlayertype` + `dimension` (+ a DEBUG-only collider pointer) but **not** the entity_id.

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

**Wiring**: `colliderbatchqueue_add` gains an `entity_id` parameter. `ecs_componentmanager_add` (`component.h:209`) passes the entity_id from the pool entry header (`ecs_component_poolentry_t.entity_id`). `colliderbatchqueue_upload_to_jolt` (`colliderbatchqueue.h:138`) sets `.entity_id` when building the user data, alongside the existing `.objectlayertype` and `.dimension`.

The `entity_id` is also stored on `ecs_component_collider_t.internal.entity_id` so the batch queue can read it at upload time without carrying it through a parallel data structure.

This field is generally useful (any raycast hit can now report which entity it struck), not strictly a workbench concern.

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
    picked = userdata->entity_id;
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
| `ecs_collider_jolt_userdata_t` | Gains `entity_id` field — additive, existing readers unaffected |
| `colliderbatchqueue_add` | Gains `entity_id` parameter — internal API, updated at the single call site |
| ECS collider pipeline | **Untouched** |
| Workbench struct / editor state | **Untouched** |
| Game entity types (`vedanta_entity_type`) | **Untouched** |
| Physics rules registration in `main.c` | **Untouched** |

---

## 6. Files Summary

| File | Changes |
|------|---------|
| `ecs/component/types.h` | Add `entity_id` to `ecs_component_collider_t.internal` and `ecs_collider_jolt_userdata_t`. |
| `ecs/component/colliderbatchqueue.h` | `colliderbatchqueue_add` gains `entity_id` param; `upload_to_jolt` sets `.entity_id` in user data. |
| `ecs/component.h` | Pass `entity_id` to `colliderbatchqueue_add` at the call site. |
| `util/workbench/workbench-editor.h` | Replace `check_mouse_closest_entity()` point-distance loop with single unfiltered `physics_sys_jolt_raycast` + `userdata->entity_id` read. Remove `workbench_editor__internal_closest_point_on_ray`. |
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
