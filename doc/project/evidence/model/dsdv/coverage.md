# DSDV coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [features](../../protocol/dsdv/features.md), [results](results.md)

Run record: 2026-11-08, branch `feat/dsdv-routing`, debug mode, General/run/seed 0
for unit, module and forwarding cases; Mobile/run/seed 0 for the 32-second mobile
case. Fresh debug/release builds, feature-off compilation and an isolated DSDV
profile passed. Exact commands, artifacts, limits and gate gaps are in [results](results.md).

| Feature | Support | Direct checks |
| --- | --- | --- |
| DSDV-F-SELECT | supported | DsdvFoundation, DsdvGraph, DsdvPaperTables, DsdvForwarding |
| DSDV-F-LOSS | supported | DsdvGraph, DsdvDissemination, DsdvLifecycle, DsdvScheduling, DsdvMobile |
| DSDV-F-DUMPS | supported | DsdvDissemination, DsdvScheduling, DsdvForwarding, DsdvPaperTables |
| DSDV-F-SETTLE | supported | DsdvDissemination, DsdvScheduling |
| DSDV-F-PROFILE | supported | DsdvFoundation, DsdvLifecycle, DsdvScheduling, DsdvForwarding, DsdvMobile |

All eight correctness tests pass; none declares an expected failure. The new
Mobile fingerprint also passes. Support is bounded by these checks, not a claim
of exhaustive correctness under arbitrary loss, latency, topology or run length.

| English check | Passing evidence |
| --- | --- |
| choice, arithmetic, encoding | DsdvFoundation; DsdvPaperTables |
| graph, recovery | DsdvGraph; DsdvMobile |
| expiry, lifecycle | DsdvLifecycle |
| correction, incremental, payload, settling, deadlines | DsdvDissemination; DsdvScheduling |
| periodic, forwarding | DsdvForwarding; DsdvMobile; startup-loss rounds in DsdvGraph |

## Pass log

* Specification freeze: independent catalog, English procedures and hand-worked
  oracles written before implementation; no runs claimed.
* Implementation pass: four unit, two module and two protocol cases passed;
  one new fingerprint passed; same-seed and recorder/logging neutrality matched.
