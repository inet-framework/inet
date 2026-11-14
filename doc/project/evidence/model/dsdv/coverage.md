# DSDV coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [features](../../protocol/dsdv/features.md), [extension execution](paper-extensions-results.md), [original host-profile results](results.md)

The final corrected-source pass on 2026-11-14, branch `feat/dsdv-paper-extensions`, revision `e81556809f`, has fresh debug/release compilation, **4 unit, 8 module and 6 protocol cases passing**, and **13/13 scoped nonvisual fingerprint ingredients passing**. The isolated IPv4+UDP feature-off build also passes on that exact revision, and all three exact-parent protocol comparisons fail at their intended parent assertion and pass on the current feature revision. Behavioral runs use debug mode, General/run/seed 0 except the existing Mobile/run/seed 0 fixture. The [extension execution record](paper-extensions-results.md) owns commands, logs, historical checkpoints, comparison adaptations and limits. No fingerprint baseline was changed.

| Feature | Support | Direct checks |
| --- | --- | --- |
| DSDV-F-SELECT | supported | DsdvFoundation, DsdvGraph, DsdvPaperTables, DsdvForwarding |
| DSDV-F-LOSS | supported | DsdvGraph, DsdvDissemination, DsdvLifecycle, DsdvScheduling, DsdvMobile |
| DSDV-F-LINK-FEEDBACK | supported within tested hint paths | DsdvDissemination, DsdvLifecycle, DsdvScheduling, DsdvMacFailure |
| DSDV-F-DUMPS | supported | DsdvDissemination, DsdvScheduling, DsdvForwarding, DsdvPaperTables |
| DSDV-F-ADAPT | supported for the explicit model policy | DsdvDissemination, DsdvScheduling, DsdvAdaptiveInvalidBounds, DsdvAdaptiveInvalidBudget, DsdvAdaptiveDumps |
| DSDV-F-FAIR | supported within controlled send opportunities | DsdvDissemination, DsdvIncrementalFairness, DsdvGatewayOrigin |
| DSDV-F-SETTLE | supported | DsdvDissemination, DsdvScheduling |
| DSDV-F-BASE-STATION | supported for one configured origin | DsdvFoundation, DsdvGraph, DsdvDissemination, DsdvGatewayReconciliation, DsdvGatewayOrigin, DsdvBaseStation |
| DSDV-F-PROFILE | supported within tested versions/configurations | DsdvFoundation, DsdvLifecycle, DsdvScheduling, DsdvGatewayInvalidPayload, DsdvUnexpectedInterface, DsdvForwarding, DsdvMobile, DsdvBaseStation |

All 18 selected correctness cases pass on the corrected source. Four module cases intentionally reject invalid configurations and pass only when their specific diagnostic and exit status match. Support is bounded by these observations, not exhaustive correctness under arbitrary loss, latency, topology or run length. The first fingerprint pass exposed an all-fit optional-ordering regression; preserving destination order in that case and rotating only under scarcity restores all selected nonvisual expectations. The final pass reruns the exact-order unit regression as well as the protocol controls.

| English check | Passing evidence |
| --- | --- |
| choice, arithmetic, encoding | DsdvFoundation; DsdvPaperTables |
| graph, recovery | DsdvGraph; DsdvMobile |
| expiry, lifecycle | DsdvLifecycle |
| failed next-hop hints, unrelated successors | DsdvScheduling; DsdvMacFailure |
| correction, incremental, payload, settling, deadlines | DsdvDissemination; DsdvScheduling |
| fair sequence-only selection | DsdvDissemination; DsdvIncrementalFairness; version-2 adapter cases in DsdvGatewayOrigin |
| periodic, forwarding | DsdvForwarding; DsdvMobile; startup-loss rounds in DsdvGraph |
| bounded adaptive timing, urgent-send independence, heterogeneous peers | DsdvAdaptiveDumps; DsdvScheduling |
| external defaults, sequenced capability, loss/recovery, version rejection | DsdvBaseStation; DsdvGatewayReconciliation; DsdvGatewayOrigin; DsdvFoundation; DsdvGraph |

## Pass log

* Specification freeze: independent catalog, English procedures and hand-worked
  oracles written before implementation; no runs claimed.
* Implementation pass: four unit, two module and two protocol cases passed;
  one new fingerprint passed; same-seed and recorder/logging neutrality matched.
* Those original host-profile observations date to 2026-11-08 on `feat/dsdv-routing`; the [original results](results.md) retain that historical fingerprint record. They are not fresh validation of the extension branch.
* Extension pass, 2026-11-14: C1–C4 implemented; corrected C4 debug/release builds, isolated feature-off build, 18 focused correctness cases and 13 stored nonvisual fingerprint ingredients pass. Refreshed exact-parent fairness, MAC-feedback and adaptive-spacing comparisons fail at their intended parent assertion and pass on their respective current feature commit. Known unrelated project-wide gate findings and scope limits are recorded separately from this bounded coverage.
