# HWMP mesh — implementation and verification ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [Mesh features](../../protocol/wifi/features.md), [Mesh checks](../../protocol/wifi/checks.md), [Implementation plan](../../../../../plan/pending/ieee80211-hwmp.md)

This ledger separates selected behavior, implementation work and executed evidence. Implementation was requested on 2026-11-10 against source `91219b0fa7d2794ee67ac8a3dd45c3a09cfc45b0`. The C1 evidence documents define the selected profile; they do not establish runtime support. All planned behavior checks below are `NOT_RUN` until their named tests exist, select nonzero cases and execute against the matching library. No fingerprint or expected-result update is authorized.

## Check-to-test map

Protocol test paths below are relative to `tests/protocol/wifi/mesh/`. Reviewer documents travel with the behavior commit. The feature map and catalog contain the normative requirements; this table is the implementation-side mapping.

| Check suffix (`WIFI-CHECK-`) | Planned protocol test | Commit | Reviewer document | Verdict |
| --- | --- | --- | --- | --- |
| WIRE | MeshWireFormats.test | C2 | hwmp-c2-wire.md | NOT_RUN |
| PEERING | MeshPeering.test | C3 | hwmp-c3-peering.md | NOT_RUN |
| PEERING-FAILURE | MeshPeeringFailure.test | C3 | hwmp-c3-peering.md | NOT_RUN |
| BEACON-TIMING | MeshBeaconTiming.test | C3 | hwmp-c3-peering.md | NOT_RUN |
| FORWARDING | MeshForwarding.test | C4 | hwmp-c4-forwarding.md | NOT_RUN |
| GROUP-FORWARDING | MeshGroupForwarding.test | C4 | hwmp-c4-forwarding.md | NOT_RUN |
| AIRTIME | MeshAirtimeMetric.test | C5, H2, H3, P3 | hwmp-c5-airtime.md | NOT_RUN |
| DISCOVERY | HwmpDiscovery.test | C6 | hwmp-c6-discovery.md | NOT_RUN |
| PATH-SELECTION | HwmpPathSelection.test | C6 | hwmp-c6-discovery.md | NOT_RUN |
| DISCOVERY-LIMITS | HwmpDiscoveryLimits.test | C6 | hwmp-c6-discovery.md | NOT_RUN |
| PATH-ERROR | HwmpPathError.test | C7 | hwmp-c7-errors.md | NOT_RUN |
| REDISCOVERY | HwmpRediscovery.test | C7 | hwmp-c7-errors.md | NOT_RUN |
| ROOT-PREQ | HwmpProactivePreq.test | C8 | hwmp-c8-roots.md | NOT_RUN |
| RANN | HwmpRann.test | C8 | hwmp-c8-roots.md | NOT_RUN |
| ROOT-COEXISTENCE | HwmpRootCoexistence.test | C8 | hwmp-c8-roots.md | NOT_RUN |
| HT-NEGOTIATION | MeshHtNegotiation.test | H1 | hwmp-h1-ht-vht.md | NOT_RUN |
| VHT-NEGOTIATION | MeshVhtNegotiation.test | H1 | hwmp-h1-ht-vht.md | NOT_RUN |
| HIGH-THROUGHPUT-DATA | MeshHighThroughputData.test | H1 | hwmp-h1-ht-vht.md | NOT_RUN |
| AMSDU | MeshAmsdu.test | H2 | hwmp-h2-amsdu.md | NOT_RUN |
| AMPDU-BLOCK-ACK | MeshAmpduBlockAck.test | H3 | hwmp-h3-ampdu.md | NOT_RUN |
| AGGREGATION-LIMITS | MeshAggregationLimits.test | H3 | hwmp-h3-ampdu.md | NOT_RUN |
| POWER-MODES | MeshPowerModes.test | P1 | hwmp-p1-power-modes.md | NOT_RUN |
| TIM-DTIM | MeshTimDtim.test | P1 | hwmp-p1-power-modes.md | NOT_RUN |
| AWAKE-WINDOW | MeshAwakeWindow.test | P2 | hwmp-p2-doze.md | NOT_RUN |
| LIGHT-SLEEP | MeshLightSleep.test | P2 | hwmp-p2-doze.md | NOT_RUN |
| DEEP-SLEEP | MeshDeepSleep.test | P2 | hwmp-p2-doze.md | NOT_RUN |
| SERVICE-PERIOD | MeshPeerServicePeriod.test | P3 | hwmp-p3-service-periods.md | NOT_RUN |
| POWER-RECOVERY | MeshPowerSaveRecovery.test | P3 | hwmp-p3-service-periods.md | NOT_RUN |
| POWER-HT-VHT | MeshPowerSaveHtVht.test | P3 | hwmp-p3-service-periods.md | NOT_RUN |

Supplementary planned cases are `tests/unit/Ieee80211MeshSerializer_1.test`, `tests/unit/Ieee80211AirtimeMetric_1.test` and `tests/module/Ieee80211MeshLifecycle_1.test`. Unit arithmetic and codec assertions supplement the production-path checks; lifecycle tests establish isolated owner cleanup, not safe whole-NIC restart. All are `NOT_RUN` at C1.

## Model decisions

These are selected modeling policies, not requirements attributed to IEEE. The [plan](../../../../../plan/pending/ieee80211-hwmp.md) defines ownership and the complete implementation contract. Defaults are implemented in NED with units; this ledger records their rationale and evidence limits.

| Decision | Selected policy | Reason and boundary |
| --- | --- | --- |
| Profile | Open, one fixed channel, one interface, endpoints only, LLC/SNAP, EPD disabled | Bounded mesh profile; neither portals nor proxy addressing is claimed |
| PHY | Fixed legal per-peer tuple; RTS, fragmentation and adaptive rate control disabled | Enables reproducible directional airtime and capability checks |
| Metric selection | ID 1 for OFDM; ID 2 default for HT/VHT; explicit ID 1 comparison allowed mesh-wide | Follows high-rate recommendation without changing metric ID per peer |
| Pending MSDUs | Shared capacity 64 packets and 256 KiB; five-second per-packet wait; drop new on overflow | Bounds memory and waiting; count existing LLC payload bytes before adding mesh encapsulation |
| Duplicate retention | One second; seed locally originated group identity | Bounded duplicate history, not infinite duplicate exclusion |
| Transit A-MSDU | Preserve constituent list and mesh forwarding-unit identity; do not split/merge transit units | Conservative endpoint-profile policy, avoiding unprovided identity rewriting rules |
| Sequence comparison | Unsigned difference strictly between zero and 2^31 is newer; exact half-range unordered | Avoids signed overflow and chooses an explicit resolution of ambiguous serial ordering |
| Encoded cost | Ceiling conversion; finite clamp to [1, UINT32_MAX]; wide saturating path addition | Prevents cost wraparound from favoring a prohibitively expensive path |
| Error samples | Per-direction `ef = 7/8 * ef + 1/8 * sample`, initially zero | Actual unicast payload ACK success=0 or on-air ACK timeout=1, each attempt separately; excludes holds/internal collision/group/management/QoS Null |
| Error approximation | Use traffic samples to estimate nominal-frame corruption | Assumes size-independent frame-error probability; not a claim of measurement for every size |
| Aggregation samples | Per-direction `n = 7/8 * n + 1/8 * m`, initially one, fractional | One count per actually completed payload PPDU, including retries; nested constituents counted as MSDUs; no time decay |
| ID 1 aggregation | Formula always uses n=1 | Aggregation observation does not change this metric's definition |
| Metric overhead | AC_BE AIFS, mean initial contention, selected preamble/header, SIFS and legal ACK/BA duration | Approximation of channel cost, not per-packet contention or sleep waiting time |
| Incoming Link Metric Report | Answer requests; do not overwrite local outgoing sample estimator | Standard permits an update but does not require one; local direction/sample authority stays explicit |
| Estimate reset | Peer closure or accepted PHY tuple change resets ef/n; aggregation permission change resets n only | Samples belong to the current relationship and transmission profile |
| Deep-sleep maintenance | Every ten local beacon periods, listen to established peer with lowest MAC address; invalid reference holds awake for reacquisition | Standard leaves deep-sleep synchronization algorithm local |
| Mode transition trigger | Explicit peer policy; lower-activity commitment follows acknowledged exchange | Requested mode differs from effective mode while confirmation is pending |
| Supported lifecycle | Isolated mesh owner cleanup/restart; operational topology failure | Existing whole-NIC MAC child cleanup does not establish safe node stop/start |

The independent arithmetic control uses O=100 microseconds, r=100 Mb/s, ef=0 and Bt=8192 bits. Expected ID-1 cost is 18; expected ID-2 costs are 18192 at n=1 and 10692 at n=4. A three-MSDU PPDU updates n from 1 to 5/4. An initial failed attempt updates ef to 1/8; its successful retry updates ef to 7/64. These examples define test oracles; they are not executed test results.

## Standard extraction record

The corpus status check reports ready/fresh for `ieee80211-2024`, source hash beginning `c08beb0e16bf`. Document-specific lint retains 14 ambiguous-heading errors and 56 unresolved-reference warnings; this is not a clean lint verdict. The independent extraction checked 18 outgoing references across selected HWMP receive, drift, awake, doze and termination nodes; those references resolved. That bounded result does not clear all corpus dependencies.

The source PDF was used for mathematical grouping in the airtime formula, mesh A-MSDU layout/address/lifetime rules, the PERR table cross-reference mismatch, the PREQ receipt/propagation sequence mismatch and Block Ack originator pseudocode interrupted by Figure 10-42 in extracted text. C1 corrected the plan's A-MSDU endpoint, mesh sequence and expiry assumptions: source aggregates use common endpoints and one sequence, duplicate admission applies to the aggregate, and committed aggregate expiry follows the last constituent deadline. Normative source facts and interpretations stay in the [standards map](../../protocol/wifi/standards.md) and [catalog](../../standard/ieee80211/catalog.md). Neither extraction nor document link validation establishes an implemented feature.

## Execution record

| Stage | State | Evidence |
| --- | --- | --- |
| C1 requirements/check transcription | COMPLETE | 34 catalog entries mapped to features, 29 checks mapped to proposed tests; independent standards extraction and scoped links checked |
| Initial debug library build | IN_PROGRESS | `make MODE=debug -j8`, raw log `/tmp/inet-mesh-initial-build.log` |
| Protocol behavior | NOT_RUN | Planned check-to-test map above |
| Release compilation | NOT_RUN | Required before push |
| Legacy controls/fingerprints | NOT_RUN | Explicit selected controls in the plan; baseline unchanged |
| Feature-disabled compilation | NOT_RUN | Required by C9 |
| Examples and capture decode | NOT_RUN | Required by C10 |
| Upstream PR | NOT_CREATED | Publication follows implementation and verification |

The supplied `AGENTS.md` asks for skill-suite scripts, but this checkout has no `scripts/` directory. Those package checks do not exist here; no skill package or deployment is changed. Use the active checkout's documented INET gates for this work.
