# DSDV conformance

> **Kind:** report · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [coverage](coverage.md)

## Claims

The DSDV model claims the bounded IPv4 profile of Perkins/Bhagwat (SIGCOMM 1994), §§3–4, including fair sequence-only selection and optional MAC failure feedback, adaptive full dumps and one configured base station. `DsdvTable.cc` owns route freshness, metrics, odd-sequence withdrawals, advertised state and dissemination history. `Dsdv.cc` supplies UDP, owned `/32` routes and the optional derived `/0`, timers and lifecycle integration. The [model profile](profile.md) owns the explicit adaptation formula, failed-frame hint assumptions and version-2 capability encoding. Ordinary routers retain one ad hoc interface; the configured base station may add its named external uplink.

## Matrix

Measured on 2026-11-14 at corrected C4 revision `e81556809f`, using the [extension execution record](paper-extensions-results.md): fresh debug/release and isolated feature-off compilation, 4 unit, 8 module and 6 protocol cases pass, with all 13 scoped nonvisual fingerprint ingredients unchanged. Refreshed exact-parent protocol comparisons identify the intended parent failure and feature pass for each of C1–C3. These bounded results do not make unrelated project-wide gates clean or substitute for a separately recorded PR audit.

| Feature | Claimed | Measured support | Verdict |
| --- | --- | --- | --- |
| DSDV-F-SELECT | yes | supported | confirmed within the checked traces and graphs |
| DSDV-F-LOSS | yes | supported | confirmed within controlled loss, expiry and recovery fixtures |
| DSDV-F-LINK-FEEDBACK | opt-in | supported | confirmed for enabled Acking/ad hoc DCF hints and disabled timeout-only control |
| DSDV-F-DUMPS | yes | supported | confirmed within checked capacities and update sequences |
| DSDV-F-ADAPT | opt-in | supported | confirmed for the explicit quiet/churn/cost/clamp policy and bounded heterogeneous fixture |
| DSDV-F-FAIR | yes | supported | confirmed for controlled sequence-only send opportunities and version-2 adapter output |
| DSDV-F-SETTLE | yes | supported | confirmed for the explicit model smoothing/deadline law |
| DSDV-F-BASE-STATION | opt-in | supported | confirmed for one origin, configured wired return routes, capability loss/recovery and controller restart |
| DSDV-F-PROFILE | yes | supported | confirmed for tested version-1/version-2 bytes, malformed-input atomicity, attachment and lifecycle paths |

The damping ambiguity, parity/metric limits, checked finite-run sequences, retained invalid floors, smoothing equation, restart rules, adaptation formula and wire encoding are engineering choices, not unambiguous paper requirements. Layer-2 routing, IPv6, arbitrary-prefix propagation, simultaneous gateway election, security, asymmetric links, alternate metrics, bounded garbage collection and module recreation without persistent state remain outside the claim. The gateway test covers DSDV-controller restart, not whole Ethernet-node reboot or remote Internet health. MAC hints do not establish physical-disconnection detection or support for every MAC variant. No performance improvement or whole-channel overhead bound is claimed.

The exact-parent comparisons establish fairness, early route withdrawal after the tested MAC failure and adaptive packet spacing on their named feature revisions. C2's parent stops at the first enabled Acking assertion, and C3's parent stops before the heterogeneous checks, so neither later parent scenario is inferred. The C3 parent cannot initialize the new gateway profile unchanged; no measured parent gateway forwarding failure is claimed. Historical 2026-11-08 host-only evidence remains in [the original results](results.md), separately from the extension checks and their coverage limits.
