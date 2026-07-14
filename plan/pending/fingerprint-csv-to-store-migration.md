# Fingerprint baseline: CSV → store.json migration, then sim-time-limit optimization

## Goal

Replace INET's GitHub-CI fingerprint testing with opp_ci (opp_repl). Do it in two
phases, correctness before speed:

1. **Phase 1 — faithful migration**: make `tests/fingerprint/store.json` an exact
   regeneration of the authoritative `tests/fingerprint/*.csv` files, so opp_ci
   provably runs the *identical* fingerprint tests GitHub CI does.
2. **Phase 2 — optimization** (only after Phase-1 parity is trusted): reduce
   oversized `sim_time_limit`s so runs are fast, sync with `omnetpp.ini` where
   sensible, add new entries, retire the CSVs.

## Background / why this shape

- **opp_ci / opp_repl** read the per-fingerprint `sim_time_limit` from
  `store.json`; if absent, they fall back to the config's `omnetpp.ini`. They
  never read the CSVs.
- **INET GitHub CI** reads `tests/fingerprint/*.csv` (14 files) — the maintained,
  authoritative baseline.
- The committed `store.json` has **diverged** from the CSVs (independently
  re-recorded at some point, e.g. `BMacWithUnitDiskRadio` tplx = `8192-d648` in
  store.json vs `7743-0a38` in `examples.csv`). It is **not trusted** — safe to
  regenerate wholesale.
- Same scope, different values: store.json has 4479 entries; the CSV rows ×
  ingredients land in the same ballpark. So this is a *re-value*, not a re-scope.

### Architecture decision (resolved)
- **store.json is the single source of truth** going forward; the CSVs are legacy
  and get retired as part of removing GitHub CI.
- Phase 1 makes store.json == CSV content (revert the divergent values), giving
  provable GitHub parity. Only after that do we change anything (Phase 2).
- Work happens directly on **`topic/migrate_to_opp_repl`** (worktree
  `/home/levy/workspace/inet-master`, currently at `a17eaec6` — the commit opp_ci
  tests). Commits advance the branch; opp_ci re-points its `git_ref` to the new
  SHA to validate. Master (still on CSV-based GitHub CI) is untouched until the
  cutover.

## Tooling that exists
- `opp_repl/test/fingerprint/old.py :: update_correct_fingerprints_from_csvs()` —
  globs every `tests/fingerprint/*.csv`, parses (working_dir, run-args,
  sim_time_limit, fingerprints, result), and writes store.json. It does
  find-or-insert, so a **clean regenerate = `clear()` then import** (otherwise
  stale divergent entries linger).
- `opp_repl/test/fingerprint/store.py :: FingerprintStore` — `clear()`,
  `insert_fingerprint()`, `update_fingerprint()`, `write()`.
- Phase-2 re-recording: `opp_update_fingerprint_test_results`
  (`opp_repl.test.fingerprint.task :: update_fingerprint_test_results`).

---

## Phase 1 — Faithful CSV → store.json migration + parity validation

- [x] **1.1 Clean-regenerate store.json from the CSVs.** DONE — `clear()` +
      `update_correct_fingerprints_from_csvs` over the 14 CSVs → 5434 entries
      (5441 CSV fingerprints − 7 exact-duplicate collapses). BMac tplx reverted
      `8192-d648`→`7743-0a38` (CSV value). Required an opp_repl enhancement:
- [x] **1.1a Itervar run-filter support** (opp_repl, `opp_ci` branch). 156 CSV
      rows / 528 fingerprints select their run by an iteration-variable filter
      (`-r '$datarate==... && $repetition==0'`), not a run number; the importer
      `int()`-ed them and dropped all 528. Fixed importer to keep the filter
      verbatim as `run_number`, and `get_fingerprint_test_tasks(stored_only)` to
      build a task per string-`run_number` store entry (cloning a same-config
      enumerated task) so `opp_run -r '<filter>'` runs them. Committed + pushed.
- [ ] **1.2 Verify faithfulness/completeness** (do NOT trust silently):
      - per-CSV entry counts = rows × ingredient-count; total ≈ prior 4479.
      - spot-check known values: `BMacWithUnitDiskRadio` tplx must now be
        `7743-0a38` (the CSV value), not `8192-d648`.
      - every CSV row is represented; no store-only leftovers.
      - PASS/FAIL/ERROR expectations and multi-limit rows carried over correctly.
- [ ] **1.3 Commit** the regenerated `store.json` on
      `topic/migrate_to_opp_repl` with a message documenting it is a verbatim
      regenerate from the CSVs (list the source files + counts).
- [ ] **1.4 Validate via opp_ci** against this commit:
      - point the opp_ci fingerprint run's `git_ref` at the new commit SHA.
      - run it (SLOW — limits unchanged; shard or use the cpu-time cap / monitor
        trick to bound wall-clock for this one-time check).
      - reconcile the results against GitHub CI's known-green:
        - all-green ⇒ **parity achieved**.
        - any FAIL/ERROR ⇒ classify: (a) omnetpp/inet **version misalignment**
          (opp_ci pins vs what the CSVs were recorded against), (b) genuine
          behavior diff, (c) stale CSV. Document each; resolve by aligning
          versions or flagging for CSV maintenance. This is the *point* of a
          faithful run — surface real diffs, not hide them.
- [ ] **1.5 Deliverable**: documented opp_ci-fingerprint ≡ GitHub-CI-fingerprint,
      with any residual diffs explained.

## Phase 2 — Optimize sim-time-limits (deferred until Phase-1 parity is trusted)

- [ ] **2.1 Choose per-config target limits.** For each oversized config, pick the
      smallest `sim_time_limit` that still exercises the important behavior
      (allowed 1ms … 1000s). Domain decision — see the discovery table below.
- [ ] **2.2 Reduce + re-record in store.json** via
      `opp_update_fingerprint_test_results` at the new limits (store.json is now
      authoritative; CSVs are being retired).
- [ ] **2.3 ini sync / consolidation** (optional): where a config's fingerprint
      limit should equal its `omnetpp.ini` `sim-time-limit`, drop the explicit
      store.json limit and rely on the ini fallback. Decide per config. NOTE:
      changing the ini cascades to statistical/chart baselines for that config —
      track and regenerate those "other affected test results" if we go this way.
- [ ] **2.4 Group-B (event-dense, already-tiny limit)**: out of scope for limit
      reduction (e.g. clockdrift 0.1s → ~100s CPU). Flag for sim-level
      optimization or accept as the runtime floor.
- [ ] **2.5 Retire the CSVs** as part of GitHub-CI removal.
- [ ] **2.6 Re-validate via opp_ci**: fast (target <10 min) + green.

## Phase 3 — Resume opp_ci testing on the new baseline
- [ ] opp_ci fingerprint runs fast + green against the optimized store.json.
- [ ] Continue opp_ci ⟷ GitHub parity for the remaining test kinds.

---

## Discovery data (reference for Phase 2)

Measured on opp_ci (`select=shortest`, per-sim CPU time). 48 sims ≥ 60s CPU.
Full run is bottlenecked by **BMac (~53 min, ~6× the next)**; without it the suite
finishes in < 10 min.

### A) Oversized limit → reduce (21 configs)
| CPU | limit | config |
|----:|------:|--------|
| 1270s | 100s | examples/wireless/nic/BMacWithUnitDiskRadio |
| 510s | 200s | examples/ieee8021as/Network_with_cross_traffic |
| 485s | 10000s | examples/inet/hierarchical99/AdaptiveProbabilisticBroadcast |
| 423s | 100s | examples/inet/dctcp/TcpRenoIncast |
| 396s | 100s | examples/ospfv2/fulltest/General |
| 346s | 200s | examples/inet/flatnet/General |
| 345s | 100s | examples/inet/dctcp/DcTcpIncast |
| 293s | 10000s | examples/inet/hierarchical99/ProbabilisticBroadcast |
| 229s | 200s | examples/ieee8021as/Network |
| 220/204/135/120s | 180s | examples/ieee8021d/RSTP-LargeNet, RSTP-LinkReconnect, STP-LinkReconnect, STP-LargeNet |
| 208s | 10000s | examples/inet/hierarchical99/IPv6 |
| 182s | 500s | examples/ospfv2/areas/General |
| 122s | 200s | examples/ieee8021as/Network_daisy_chain |
| 114s | 500s | examples/ethernet/arptest2/ARPTest |
| 106/60s | 100s | examples/ospfv2/areatests/backboneandonestub, backbone |
| 65s | 1000s | examples/seaport/General |
| 63s | 100s | showcases/routing/manet/Gpsr |

### B) Limit already < 60s but sim slow → event-dense (27 configs; limit reduction won't help)
| CPU | limit | config |
|----:|------:|--------|
| 408/280s | 50s | examples/inet/nclients/General (r0,r1) |
| 324s | 20s | examples/manetrouting/multiradio/SingleRadio |
| 297/170s | 2s | examples/wireless/synchronized/NonSynchronized, Synchronized |
| 246s | 5s | examples/voipstream/VoIPStreamLargeNet/General |
| 176/172/171/167s | 1s | showcases/tsn/…/gptpandtas/LinkFailure, Recovery, NormalOperation, Failover |
| 159/95/70s | 10-20s | examples/manetrouting/dymo,gpsr/MultiIPv6, IPv6 |
| 100/96/91/91/88/88s | 0.1s | showcases/tsn/timesynchronization/clockdrift/* (6) — 1000× real-time, sim-perf issue |
| 92/89/83s | 10s | examples/adhoc/qos, examples/wireless/qos/MacQos* |
| 80/76s | 1s | showcases/measurement/flow/Basic, AnyLocation |
| 68/64/60s | 2-22s | pcaprecording, dsdv/DynamicIPv4, VoIPStreamTrafficTest |

---

## Decision log
- store.json is the source of truth; CSVs retired with GitHub CI. (user)
- Regenerate store.json cleanly from CSVs — nothing trusted is lost yet. (user)
- Limits (Phase 2) reduced to the smallest that still shows the important
  behavior, 1ms–1000s. (user)
- Branch: work directly on topic/migrate_to_opp_repl (worktree inet-master); no
  sub-branch. opp_ci re-points git_ref to the new commit to validate.

## Open items to resolve during execution
- Exact clean-regenerate invocation (verify `clear()` semantics).
- ~~Version parity: is opp_ci's omnetpp/inet == what the CSVs were recorded with?~~
  RESOLVED: run 53 = 1717 PASS, 0 fingerprint mismatches. The faithful store
  matches opp_ci's omnetpp-6.x.
- One-time slow Phase-1 validation: shard vs cpu-time cap.

## Outcome (2026-07-14) — Phase 1 green
Faithful CSV→store run is green on opp_ci: 1717 PASS, 7 expected-ERROR, 0
unexpected (~142s), across itervar and multi-ingredient groups, tyf excluded.
Two opp_repl framework fixes were needed: (a) aggregate `expected` = "no
unexpected children" (was `expected_result==result`, which mislabeled
all-expected-ERROR groups); (b) simulation-config collection now drops configs
whose feature is disabled in the build (their network NED isn't compiled, so
they can only ERROR) — resolved NED-folder-aware from `.oppfeaturestate`.

OSG showcases: opp_env now conditionally keeps `WITH_OSG=yes` when the host has
OpenSceneGraph (was always forced off). On OSG-capable workers the
`showcases/visualizer/osg` configs build and run; on OSG-less builds they are
skipped by (b). This commit exists to force a fresh workspace so the worker
rebuilds omnetpp with the conditional recipe.
