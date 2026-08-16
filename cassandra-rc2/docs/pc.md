# PressCylinder – Bug & Undefined Behaviour Report

- **Division by zero**: `m_pvs[i]->update((_pvs_raw[i] * 100) / _settings.maxload_threshold)` (PressCylinder.cpp) is executed even when `maxload_threshold` is **0**. User‐supplied JSON can set this value to 0 because it's clamped in `[0, PRESS_CYLINDER_MAX_LOAD_CLAMP]`; clamp lower bound should be 1.

- **Stale PV values on sensor error**: `pvsOk()` returns `false` as soon as one `getWeight()` call fails, but previously read `_pvs_raw` entries remain unchanged. Subsequent logic (state machine, solenoid control, diagnostics) may consume these stale values.

- **`has_stalled()` uses the last cylinder only**: `last_pv` is overwritten inside the loop, so the stall check always compares every cylinder against the PV of the *last* valid cylinder, not the minimum/average as intended.

- **`min_pv` initialisation**: `uint32_t min_pv = -1` relies on unsigned wrap-around. If *no* loadcell is attached the diff check becomes `(max_pv - 0xFFFFFFFF)` which overflows. Guard with a counter of valid sensors.

- **Memory leak**: `m_pvs[i] = new PressCylinderValue(...)` is never deleted; add destructor or migrate to static allocation.

- **Unbalanced / balance flip timing**: In `MODE_AUTO_MULTI_BALANCED` the active cylinder flips every `BALANCE_INTERVAL` even if both PVs are already balanced (< `BALANCE_MIN_PV_DIFF`). Consider conditional flip only when imbalance persists. 