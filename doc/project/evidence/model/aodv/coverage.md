# AODV — coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/aodv/standards.md), [conformance.md](conformance.md)

The single place that holds the changing state of the AODV workflow. Every other artifact of this protocol states what a standard says and never changes again unless the standard changes. This one changes on every pass.

Historical level 1 survey — no build or run in that survey:

- Date: 2026-09-23
- INET: branch `topic/standards-tests-wave0`, commit `e360ca980e`, tree clean
- Trees: src `16dc528e10`, tests/protocol `6f0a6bdb05`
- Target level: 1

The historical survey had no catalog, feature map or protocol tests. The selected correction
effort now adds those artifacts and the bounded evidence below. This does not satisfy the
exhaustive criteria for a higher achieved level.

## Achieved level

**Level 1, reached.** Target: level 1, from [`standards.md`](../../protocol/aodv/standards.md#target-level).

The exit criterion of level 1 has three parts, and all three hold:

| Part of the criterion | State | Evidence |
| --- | --- | --- |
| The standards map pins an in-scope set | holds | RFC 3561 §5, §6.1, §6.3 to §6.7 and §6.11, and RFC 5148 §5, chosen from the register: [`standards.md`](../../protocol/aodv/standards.md#in-scope-set) |
| Every model claim is mapped onto the map | holds | each of the 11 places in the model that name a document has a row in [`conformance.md`](conformance.md#part-1--the-claims) |
| Every obsolete claim is named | holds, vacuously | neither RFC 3561 nor RFC 5148 has an `obsoleted_by` or `updated_by` relation in the register, so no citation of the model can be obsolete; see [`conformance.md`](conformance.md#part-1--the-claims) |

| Level | State | What it needs |
| --- | --- | --- |
| 1, Survey | **reached** | — |
| 2, Core | selected checks in progress | the complete catalogs of RFC 3561 (§5, §6.1, §6.3 to §6.7, §6.11) and RFC 5148 (§5), the feature map, the checks, and a two- or three-hop wireless chain |
| 3, Edge | not started | the request-validation and duplicate-suppression rules of §6.4 and §6.5, and a unidirectional link (§6.8) |
| 4, Dynamics | not started | the Hello message interval and local-connectivity check (§6.9, §6.10), the ring-search backoff (§6.3), and the RFC 5148 jitter bound, with a tolerance |
| 5, Complete | not started | local repair and actions after reboot (both claimed by the model, see [`conformance.md`](conformance.md#part-1--the-claims)), multiple interfaces, subnet aggregation, and the AODV-to-wired gateway |

## Pass log

| Pass | Date | Level | Scope | Result |
| --- | --- | --- | --- | --- |
| 1 | 2026-09-23 | **1, reached** | RFC 3561 and RFC 5148 downloaded; the standards map; the claims of the model | no run; no obsolete claim; one claim (local repair) reads as a refusal but is a claim by the guide's own rule, because the switch already exists |
| 2, partial | 2026-11-09 | selected corrections | AODV-F-FRESHNESS, selected RREQ/RREP/Hello/RERR host-route cases | eight parent assertion failures become passes, with a passing preparation control and arithmetic unit test. See [results.md](results.md). Broader plan rows remain pending. |
| 3, partial | 2026-11-09 | selected corrections | AODV-F-LIFETIME, reverse refresh and advertisement duration | five protocol parent failures and four missing configuration rejections become passes. Two protocol parents remain passing controls. Combined selection: 16 protocol and eight module passes. |
| 4, partial | 2026-11-09 | selected corrections | AODV-F-DISCOVERY, phase ownership, queue release and exhaustion | 15 protocol and two module parent assertion failures become passes. Combined selection: 31 protocol and ten module passes. See discovery-ownership.md and results.md for limits. |
| 5, partial | 2026-11-09 | selected corrections | AODV-F-DISCOVERY, wire timing and parameter bounds | Five protocol parent controls pass unchanged. Of 35 module cases, 23 expose missing bounds, four expose defaults, six change existing rejection diagnostics and two already pass. Corrected combined scope: 36 protocol and 45 module passes. |
| 6, partial | 2026-11-09 | selected corrections | AODV-F-LINK, ACK obligations and actual failed-link attribution | 19 protocol and three module parent runtime failures become passes. Two exact-deadline protocol cases already pass. Corrected combined scope: 57 protocol and 48 module passes, with all existing expectations unchanged. |

| 7, partial | 2026-11-09 | selected corrections | AODV-F-ERROR, transit and precursor handling | 18 protocol and two module parent runtime failures become passes; seven protocol and one module parent controls remain passing. Combined scope: 82 protocol and 51 module passes. |

| 8, partial | 2026-11-09 | selected corrections | AODV-F-ERROR, outgoing capacity | Two 256-record parent failures become passes; one- and 255-record controls already pass. Four capacity cases and two adjacent admission controls pass after fresh debug/release builds. |

| 9, partial | 2026-11-09 | selected corrections | AODV-F-LINK, Hello observations | 16 protocol and two module parent runtime failures become passes, with ten protocol parent controls. Combined scope: 112 protocol and 53 module passes. |

| 10, partial | 2026-11-09 | selected corrections | AODV-F-RECOVERY, lifecycle ownership | 24 protocol and five module parent runtime failures become passes, with four protocol/two module controls. Final combined scope: 140 protocol and 60 module passes; native removal order preserves existing expectations. |

## Selected feature evidence

| Feature | Evidence | Limit |
| --- | --- | --- |
| AODV-F-FRESHNESS | Eight IPv4 UDP/IP regression cases and the request/reply control pass after correction. | Selected host-route scenarios only. See [reviewer document](../../../../../tests/protocol/aodv/route-freshness.md) for individual assertions. |
| AODV-F-LIFETIME | Seven protocol cases and four configuration rejection cases pass, including recipient expiry and exact millisecond boundaries. | Controlled IPv4 replay and route-state injection. No independent serialized-byte oracle. See [reviewer document](../../../../../tests/protocol/aodv/route-lifetimes.md). |
| AODV-F-DISCOVERY, ownership | 15 protocol and two module cases pass, covering finite rate-limited demand, direct-neighbor completion, shared gateway owners, terminal disposal and output constraints. | Deterministic IPv4 AckingWirelessInterface. IPv6 and later recovery/repair profiles remain pending. See [reviewer document](../../../../../tests/protocol/aodv/discovery-ownership.md). |
| AODV-F-DISCOVERY, timing/bounds | Five wire timing controls and 35 parameter cases pass. | Selected ring, backoff, jitter and configuration boundaries. See [reviewer document](../../../../../tests/protocol/aodv/discovery-timing.md). |
| AODV-F-LINK, validation/attribution | 21 protocol and three module cases pass, including concurrent ACKs, exact ties, real failed replies/data, quarantine and route replacement. | Direct AckingMac and independent nonaggregating DCF. Ethernet/UDP supplies the exact ACK-order controls. Hello loss, restart reset and broader metadata profiles remain pending. See [reviewer document](../../../../../tests/protocol/aodv/link-validation.md). |
| AODV-F-ERROR, transit and precursor handling | 25 protocol and three module cases pass, including N preservation, independent ownership, prefix ambiguity and actual-send budget windows. | IPv4 scripted UDP/IP. Prefix identity uses public record setup; real subnet learning, partitioning and lifecycle coverage remain assigned. See [reviewer document](../../../../../tests/protocol/aodv/transit-errors.md). |
| AODV-F-LINK, Hello observation | 26 protocol and two real three-node module cases pass, including incoming/outgoing distinctions, original-source independence, metadata loss, exact ties and validation failure. | IPv4 AckingMac, independent DCF and Ethernet profiles. Restart reset, selected interfaces and IPv6 runtime remain pending. See [reviewer document](../../../../../tests/protocol/aodv/hello-liveness.md). |
| AODV-F-RECOVERY | 28 protocol and seven module cases pass, including exact deadline orders, zero-time/double restart, control bypass, queue phases and reentrant cleanup. | IPv4 public lifecycle operations and owning IP adapters. Bounded IPv6 runtime and later interface/repair lifecycle integration remain pending. See [reviewer document](../../../../../tests/protocol/aodv/reboot-recovery.md). |
| Other selected features | Not yet established by this correction stage. | Interface, subnet and repair stages remain assigned in the plan. |
