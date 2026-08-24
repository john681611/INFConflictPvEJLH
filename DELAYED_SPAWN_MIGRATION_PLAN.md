# Delayed Spawn Migration Plan (Reforger 1.8) - Revert-Safe

## Goal

Reduce AI spawn hitching (especially Ambient Skirmishes) while staying compatible with Reforger 1.8 automatic AI lifecycle behavior (dormancy/despawn/AI limits), and keeping rollback simple.

---

## Final sense-check (before code changes)

### Confirmed available 1.8+ group spawn controls

From API/changelog evidence already collected:

- `SetNumberOfMembersToSpawn(int)`
- `GetNumberOfMembersToSpawn()`
- `BeginDelayedSpawn()`
- `SpawnDelayedGroupMember(int)`
- `EndDelayedSpawn()`
- `SpawnAllImmediately()`
- `IsInitializing()`

Also changelog mentions:

- `AIGroup::ForceActivateAllMembers`

### Confirmed high-spike spawn surfaces

- Ambient skirmish bulk group spawns call `JLH_AddonSpawnUtility.SpawnGroup(...)` in 7 call sites.
- QRF group spawn helpers and virtual wave re-materialization spawn full groups, then prune.
- Recap ghost defence and attack spawns follow similar full-spawn then prune pattern.

### Confirmed existing scheduling infrastructure

- Ambient manager and area already run scheduled ticks and use `CallLater(...)` in multiple places.
- QRF and Recap already have stable `Tick()` loops suitable for incremental/batched processing.

### Known uncertainty to avoid in phase 1

- Whether `ForceActivateAllMembers` is present in the local compile target/runtime API surface right now.
- To minimize risk, do not make phase 1 dependent on it.

---

## Low-risk, revert-safe implementation strategy

## Principle: Additive, gated, localized

1. Add new code paths; do not remove old behavior immediately.
2. Feature-flag everything new (default OFF first).
3. Keep changes concentrated in a small number of helper functions and call sites.
4. Roll out by scenario type, not globally.

---

## Phase 1 (smallest useful change): Stop overspawn-then-prune where requested count is known

### What to change

Add **new helper(s)** (do not mutate existing signature behavior globally):

- In `JLH_AddonSpawnUtility` add a dedicated spawn helper that can:
  - optionally set member count before spawn (`SetNumberOfMembersToSpawn(requestedUnits)`)
  - keep current fallback behavior if unsupported/fails

Then use that helper only at selected high-value call sites:

- QRF virtual infantry package spawns
- Recap ghost/virtual infantry package spawns
- Ambient skirmish plan-driven infantry spawns where requested unit counts are explicit/derivable

### Why this is low risk

- Existing `SpawnGroup(...)` remains untouched for unrelated flows.
- Existing prune logic remains as fallback safety net.
- Functional behavior should stay equivalent, with reduced spawned-then-deleted churn.

### Revert

- Flip feature flag OFF, or revert only the small set of call site swaps to old helper.

---

## Phase 2 (main hitch reduction): Delayed spawn for infantry-only bursts

### What to change

Introduce delayed spawn path for large infantry bursts:

- `BeginDelayedSpawn()`
- spawn in small batches per tick (`SpawnDelayedGroupMember(index)`)
- `EndDelayedSpawn()` when done
- safety timeout fallback: `SpawnAllImmediately()`

Apply first to:

- Ambient skirmish infantry-heavy events (highest spike pain)
- Then QRF/Recap virtual infantry activation flows

Do **not** apply initially to:

- Vehicle crew/seat-critical mount flows (driver/gunner determinism sensitive)

### Why this is low risk

- Existing tick/callqueue systems already exist.
- Batching only in infantry package flows avoids mount-order regressions.

### Revert

- Flag OFF = instant return to old immediate-spawn behavior.
- No schema/data migration required.

---

## Phase 3 (optional): Activation hardening only if compile target confirms API

If runtime API confirms availability:

- Add optional `ForceActivateAllMembers` call in centralized activation helper.

If not confirmed:

- Keep current `group.ActivateAI()` + control component activation only.

### Revert

- Disable optional call via compile/flag guard.

---

## Minimal-change patch topology (to keep diffs small)

1. **`JLH_AddonSpawnUtility.c`**
   - Add new helper(s) only (no broad signature replacement).
   - Add internal feature flags/limits defaults.

2. **`JLH_DCF_AmbientSkirmishArea.c`**
   - Add one area-level flag for delayed infantry spawn pilot.
   - Swap only selected heavy infantry group spawn call sites to helper.

3. **`JLH_DCF/Nodes/QRF/JLH_QRFNodeComponent.c`**
   - Use helper in virtual infantry package spawn path.
   - Keep existing prune and cleanup logic.

4. **`JLH_DCF/Nodes/Recap/JLH_RecapNodeComponent.c`**
   - Mirror QRF approach in recap virtual/ghost infantry package path.

No changes to vehicle patrol mount flow in phase 1/2.

---

## Suggested default tunables

- `m_bUseRequestedMemberSpawnCount` = OFF initially (then ON after validation)
- `m_bUseDelayedInfantrySpawn` = OFF initially
- `m_iDelayedSpawnBatchSize` = 2-4 units per tick
- `m_iDelayedSpawnForceCompleteTimeoutMs` = 2000-3500ms

---

## Validation checklist before broad enablement

1. Ambient skirmish heavy event no longer causes same spike severity.
2. QRF wave progression unchanged (no stalled wave completion).
3. Recap attack lifecycle unchanged (launch, sustain, cleanup).
4. Vehicle crew mount success rate unchanged.
5. No increase in orphaned/uncleaned runtime entities.
6. Existing debug logs still explain spawn outcomes clearly.

---

## Rollout order

1. Ship with flags OFF.
2. Enable requested-member-count path in a test session.
3. Enable delayed infantry spawn for ambient skirmish only.
4. If stable, enable for QRF virtual packages.
5. If stable, enable for Recap packages.

---

## Fast rollback plan

If anything regresses:

1. Turn OFF delayed infantry flag.
2. Turn OFF requested-member-count flag.
3. Keep fallback prune behavior as-is.

This should restore old runtime behavior without deleting new code.

