# HWMP mesh — implementation and verification ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [Mesh features](../../protocol/wifi/features.md), [Mesh checks](../../protocol/wifi/checks.md), [Implementation plan](../../../../../plan/pending/ieee80211-hwmp.md)

This ledger separates selected behavior, implementation work and executed evidence. Implementation was requested on 2026-11-10 against source `91219b0fa7d2794ee67ac8a3dd45c3a09cfc45b0`. The C1 evidence documents define the selected profile; they do not establish runtime support. All planned behavior checks below are `NOT_RUN` until their named tests exist, select nonzero cases and execute against the matching library. No fingerprint or expected-result update is authorized.

## Check-to-test map

Protocol test paths below are relative to `tests/protocol/wifi/mesh/`. Reviewer documents travel with the behavior commit. The feature map and catalog contain the normative requirements; this table is the implementation-side mapping.

| Check suffix (`WIFI-CHECK-`) | Planned protocol test | Commit | Reviewer document | Verdict |
| --- | --- | --- | --- | --- |
| WIRE | MeshWireFormats.test | C2 | hwmp-c2-wire.md | PASS at codec boundary; live consumers remain later checks |
| PEERING | MeshPeering.test | C3 | hwmp-c3-peering.md | PASS for selected OFDM profiles |
| PEERING-FAILURE | MeshPeeringFailure.test | C3 | hwmp-c3-peering.md | PASS |
| BEACON-TIMING | MeshBeaconTiming.test | C3 | hwmp-c3-peering.md | PASS |
| FORWARDING | MeshForwarding.test | C4 | hwmp-c4-forwarding.md | PASS with fixture path provider |
| GROUP-FORWARDING | MeshGroupForwarding.test | C4 | hwmp-c4-forwarding.md | PASS |
| AIRTIME | MeshAirtimeMetric.test | C5, H2, H3, P3 | hwmp-c5-airtime.md | PASS for unaggregated OFDM; aggregation/high-throughput extensions remain later stages |
| DISCOVERY | HwmpDiscovery.test | C6 | hwmp-c6-discovery.md | PASS with production HWMP and both metric IDs |
| PATH-SELECTION | HwmpPathSelection.test | C6 | hwmp-c6-discovery.md | PASS, separate ID-1/ID-2 runs |
| DISCOVERY-LIMITS | HwmpDiscoveryLimits.test | C6 | hwmp-c6-discovery.md | PASS |
| PATH-ERROR | HwmpPathError.test | C7 | hwmp-c7-errors.md | PASS |
| REDISCOVERY | HwmpRediscovery.test | C7 | hwmp-c7-errors.md | PASS with actual terminal DATA failure and alternate relay |
| ROOT-PREQ | HwmpProactivePreq.test | C8 | hwmp-c8-roots.md | PASS, both metric-ID runs |
| RANN | HwmpRann.test | C8 | hwmp-c8-roots.md | PASS, both metric-ID runs |
| ROOT-COEXISTENCE | HwmpRootCoexistence.test | C8 | hwmp-c8-roots.md | PASS, both metric-ID runs |
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

Supplementary cases are `tests/unit/Ieee80211MeshSerializer_1.test` (PASS at C2), `tests/unit/Ieee80211AirtimeMetric_1.test` (PASS at C5), and the planned `tests/module/Ieee80211MeshLifecycle_1.test` (NOT_RUN). Unit arithmetic and codec assertions supplement the production-path checks; lifecycle tests establish isolated owner cleanup, not safe whole-NIC restart.

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
| Target reply sequence | Known requested sequence is the increment baseline only if serial-newer; otherwise use own sequence | Resolves the unordered half-range locally; USN=1 reserved bytes never supply a baseline |
| Received PREQ identities | Received-Lifetime retention, capacity 4096; drop new identity before mutation when full | Bounds history without silently evicting an unexpired identity; no infinite duplicate-history guarantee |
| HWMP pacing anchor | Successful ownership handoff to management | Type-3 service request has no confirmation; this does not guarantee actual PPDU spacing after contention or sleep buffering |
| Self-destination admission | Admit reverse next hop on target PREP generation and actual transmitter on accepted local-originator PREP reception | Derived integration bookkeeping for required final-destination admission, including validation-only replies; no learned route to self |
| Reflected control | Reject received PREQ.originator=self and PREP.target=self before learning | Explicit no-self-learning policy, preserving node-owned sequence authority; normal target/reply cases remain supported |
| PERR reason interpretation | Nonzero NO-FORWARDING accepts equal/newer after stale rejection; DESTINATION-UNREACHABLE requires newer | Explicit reading of the source's conjunction structure; zero is special only for NO-FORWARDING |
| Deferred PERRs | One node-global handoff interval; exact semantic-key deduplication and recipient/TTL grouping, at most 19 distinct destinations/action | Immediate invalidation is separate; no deferred re-increment or per-recipient rate-limit bypass |
| Deferred PERR retention | Snapshot precursor expiry for path errors; established transmitter dependency for unknown transit; prune unsent obsolete errors on repair | Values retain no packet ownership; management inactivity/owner cleanup clears volatile notifications while pacing history survives |
| Proactive PREP sequence | Increment the local sequence before each locally generated proactive PREP; forwarded PREPs retain their sequence | Explicit interpretation of 14.11.8.3 with Table 14-21, whose broadcast-response update is ambiguous; preserves fresh reverse information without an equal-sequence lifetime exception |
| Flag-zero root invitation | Retain the latest applicable invitation and reply when local source demand exists, in either event order | A successful local response handoff suppresses repetition for that invitation; this is not knowledge of the root's cache |
| Root interval authority | Accepted RANN Interval updates one shared current scalar in wire order; own advertisements use it and forwarded announcements retain their received value | Configuration supplies the initial value; confirmation interval remains separate |
| Root Action pacing | One station-wide RANN gate and one station-wide confirmation gate, alongside per-target PREQ history | Batch only equal actual receivers, with deterministic scheduling between receivers; no per-root bypass |
| Root confirmation and source work | One per-target procedure with independent purposes; a confirmation counts as a source attempt only when source work is due | Source cancellation or expiry cannot cancel independent root confirmation or consume extra source attempts |
| RANN waiting | Wait for usable forwarding information, forwarding/TTL eligibility and sending interval only | No additional random forwarding delay is selected; RANN alone does not establish a usable data path |
| Mesh Action packing | At most 2302 encoded HWMP IE bytes after Category and Action | The selected non-HT management body limit is 2304 bytes including those two fields; PHY length limits are an additional bound |
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
| Supported lifecycle | Isolated mesh owner cleanup/restart; operational topology failure; preserve node-owned sequences and required pacing history | Clear volatile paths, pending queues and samples; existing whole-NIC MAC child cleanup does not establish safe node stop/start |

The independent arithmetic control uses O=100 microseconds, r=100 Mb/s, ef=0 and Bt=8192 bits. Expected ID-1 cost is 18; expected ID-2 costs are 18192 at n=1 and 10692 at n=4. A three-MSDU PPDU updates n from 1 to 5/4. An initial failed attempt updates ef to 1/8; its successful retry updates ef to 7/64. These examples define test oracles; they are not executed test results.

## Standard extraction record

The corpus status check reports ready/fresh for `ieee80211-2024`, source hash beginning `c08beb0e16bf`. Document-specific lint retains 14 ambiguous-heading errors and 56 unresolved-reference warnings; this is not a clean lint verdict. The independent extraction checked 18 outgoing references across selected HWMP receive, drift, awake, doze and termination nodes; those references resolved. That bounded result does not clear all corpus dependencies.

The source PDF was used for mathematical grouping in the airtime formula, mesh A-MSDU layout/address/lifetime rules, the PERR table cross-reference mismatch, the PREQ receipt/propagation sequence mismatch and Block Ack originator pseudocode interrupted by Figure 10-42 in extracted text. C1 corrected the plan's A-MSDU endpoint, mesh sequence and expiry assumptions: source aggregates use common endpoints and one sequence, duplicate admission applies to the aggregate, and committed aggregate expiry follows the last constituent deadline. Normative source facts and interpretations stay in the [standards map](../../protocol/wifi/standards.md) and [catalog](../../standard/ieee80211/catalog.md). Neither extraction nor document link validation establishes an implemented feature.

## Execution record

| Stage | State | Evidence |
| --- | --- | --- |
| C1 requirements/check transcription | COMPLETE | 34 catalog entries mapped to features, 29 checks mapped to proposed tests; independent standards extraction and scoped links checked |
| Initial debug library build | PASS | `make MODE=debug -j8`, raw log `/tmp/inet-mesh-initial-build.log`; C2 edits require the subsequent fresh build |
| C2 matching debug library | PASS | `/tmp/inet-mesh-c2-complete-build.log` |
| C2 direct units | PASS, 3/3 | Mesh, legacy management and MPDU-delimiter serializers; `/tmp/inet-mesh-c2-unit-complete.log` |
| C2 boundary protocol fixture | PASS, 1/1 | `/tmp/inet-mesh-c2-protocol-complete.log`; no claim of live peering/forwarding |
| C3 peering and timing | PASS, 3/3 | Fresh debug build, live peering/fault/timing fixtures; raw `/tmp/inet-mesh-c3-raw/` |
| C4 native forwarding and peer retirement | PASS, 2/2 | Fixture path provider, actual radio data and three retirement boundaries |
| C5 directional airtime and reporting | PASS, 1/1 new protocol; 5/5 units; 2/2 modules | Actual OFDM DATA/ACK outcomes, both metric IDs, retry/retirement boundaries and cleanup-complete publication |
| C6 reactive HWMP and pending source queues | PASS, 3/3 new files (4 runs) | Production discovery, directional selection, bounded queues/retries, exact deadlines and isolated-owner lifecycle |
| C7 path errors and rediscovery | PASS, 2/2 new files | Terminal DATA/peer-close causes, reason-specific PERRs, global pacing and alternate-path delivery |
| C8 proactive roots and coexistence | PASS, 3/3 new files, six metric-ID runs | All fifteen mesh files pass; retained known legacy failure; five units and two modules pass |
| Remaining live mesh behavior | NOT_RUN | HT/VHT and power save remain future stages |
| C5 committed release compilation checkpoint | PASS | Isolated clean `7264369846`, matching release dependencies; no warning/error |
| Release compilation | NOT_RUN | Required before push |
| C2 legacy protocol controls | 4/5 test files PASS; one preexisting unexpected failure | Raw Block Ack retry run 0 has protocol deadline failure, run 1 has runtime abort; immediate parent reproduces both at identical time/event |
| Parent legacy fingerprints | PASS, 19 simulations / 56 checks | Native stored budgets, all expected PASS; baseline unchanged |
| C4 committed legacy fingerprint checkpoint | PASS, 19 simulations / 56 checks | All calculated values byte-identical to C1; final combined tree remains to be checked |
| C7 committed legacy fingerprint checkpoint | PASS, 19 simulations / 56 checks | All calculated values byte-identical to C1 after documented build-storage recovery |
| Final-tree legacy fingerprints | NOT_RUN | Same exact scope still required on the final combined tree |
| Feature-disabled compilation | NOT_RUN | Required by C9 |
| Examples and capture decode | NOT_RUN | Required by C10 |
| Upstream PR | NOT_CREATED | Publication follows implementation and verification |

C2 verification ran from this checkout on `8a76ae140f` plus the C2 changes, in debug mode. The unit selector was `Ieee80211(MeshSerializer|MgmtFrameSerializer|MpduSubframeHeaderSerializer)_1\.test$`; the protocol suite selector was `^tests/protocol/wifi$` with file selector `/mesh/MeshWireFormats\.test$`. Both used the installed runners with `--load . -p inet -m debug --no-build` after a fresh matching build. The mesh protocol fixture fixes run 0 and seed 0. Independent review found no remaining actionable source issue. Mermaid diagrams rendered with Mermaid 10.9.3 and Firefox headless and were visually inspected.

The legacy selector was `/common/(WifiAssociation|WifiDataDelivery|WifiDuplicateWithRetry|WifiBlockAckDuplicateAddba|WifiBlockAckDataRetry)\.test$`. Its four passing test files are distinct from `WifiBlockAckDataRetry`: retained raw output shows a run-0 protocol deadline failure at event 3708/time 1.023764276562 s, followed by a run-1 simultaneous-transmission runtime error at event 2639/time 1.014105463846 s. A successful simulation-run summary does not override a printed protocol failure. Raw unit, boundary-protocol and failing legacy artifacts are retained under `/tmp/inet-mesh-c2-raw/` and copied under `audit/hwmp-implementation/c2-raw/`. Independent immediate-parent reproduction at `8a76ae140f` built and loaded its own debug libraries with identical feature state, fixture and seed, and reproduced both failures at identical event numbers and simulation times. The failed legacy control therefore predates C2; it remains a failed control. Parent evidence is `/tmp/inet-mesh-c1-parent-evidence.md`. No expectation or fingerprint was changed.

The supplied `AGENTS.md` asks for skill-suite scripts, but this checkout has no `scripts/` directory. Those package checks do not exist here; no skill package or deployment is changed. Use the active checkout's documented INET gates for this work.

## C3 peering and timing execution

C3 ran on source `320e19d47c` plus the C3 diff after `make MODE=debug -j4`, exit 0. `opp_run_protocol_tests --load . -p inet -m debug --no-build -w '^tests/protocol/wifi$'` selected `/(mesh/(MeshPeering|MeshPeeringFailure|MeshBeaconTiming|MeshWireFormats)|common/(WifiAssociation|WifiDataDelivery|WifiDuplicateWithRetry|WifiBlockAckDuplicateAddba|WifiBlockAckDataRetry)|11n/N_Amsdu)\.test$`. Nine files passed and the known Block Ack retry control failed at the same two parent event/time boundaries. All four mesh files passed. The three C3 fixtures use General/run 0/seed 0; their [reviewer document](../../../../../tests/protocol/wifi/mesh/hwmp-c3-peering.md) explains the stimuli, deterministic channel separation, timing oracles and scope. After correcting Address 3 in two controlled management stimuli, the two affected fixtures passed again. Codec units also passed 3/3 with the C2 selector.

Retained logs are `/tmp/inet-mesh-c3-final-build.log`, `/tmp/inet-mesh-c3-complete-protocol.log`, `/tmp/inet-mesh-c3-unit.log` and the final fixture logs listed in the reviewer document. Raw outputs are under `/tmp/inet-mesh-c3-raw/{complete-protocol,fixture-final,unit}/`. Three reviewer diagrams rendered with Mermaid 10.9.3 and Firefox and were visually inspected; screenshots are under `audit/hwmp-implementation/diagrams-c3/`. Scoped architecture and release-note checks passed. The global interface gate reports 16 preexisting classes, including unchanged `ITransmitStep` and `IReceiveStep`; no violation is introduced by `IMeshManagement`. Whole-scope naming reports 32 legacy candidates and no new mesh candidate. Global final gates and native fingerprint comparisons remain NOT_RUN for the final combined tree.

## C4 native forwarding execution

C4 ran on `f6a9863f8f` plus its working diff with a matching debug build. The [reviewer document](../../../../../tests/protocol/wifi/mesh/hwmp-c4-forwarding.md) records the exact source/receive path, fixture path-owner responsibilities, literal address/TTL/length checks, precursor and path lifetime assertions, and IFS/on-air/ACK-wait retirement cases. Both new files passed; the complete six-file mesh selection passed. Five of six legacy controls passed, while `WifiBlockAckDataRetry` retained the same two parent failures. Four unit files, including the new cancellation assertions in `Ieee80211TxNavChoice_1`, and two frame-sequence/Rx completion module controls passed. The final two-fixture rerun also verifies already-queued unrelated data survives every peer-close case and both endpoints are ready before re-peering.

The protocol suite selector is `^tests/protocol/wifi$`, with file selector `/(mesh/(MeshForwarding|MeshGroupForwarding|MeshPeering|MeshPeeringFailure|MeshBeaconTiming|MeshWireFormats)|common/(WifiAssociation|WifiDataDelivery|WifiDuplicateWithRetry|WifiBlockAckDuplicateAddba|WifiBlockAckDataRetry)|11n/N_Amsdu)\.test$`. The unit selector adds `TxNavChoice` to C2's group. The module selector is `Ieee80211(FrameSequenceObservations|RxTransmissionCompletion)_1\.test$`. All use `--load . -p inet -m debug --no-build` after the recorded matching build. Logs are `/tmp/inet-mesh-c4-{final-build,final-protocol,focused-final,unit,module}.log`, with raw artifacts under `/tmp/inet-mesh-c4-raw/`. Scoped architecture, release/migration format and whitespace checks passed; independent review found no remaining actionable issue. Three diagrams rendered and were visually inspected under `audit/hwmp-implementation/diagrams-c4/`. No baseline, expected failure, seal or exception ledger changed. This establishes native forwarding with a fixture provider, not production HWMP discovery.

## Parent native trajectory reference

The isolated C1 parent `8a76ae140f` completed 19 simulations and 56 fingerprint checks, exit 0, with no execution error or mismatch. Selection was `-w '^(examples/adhoc/qos|examples/wireless/qos|showcases/wireless/(aggregation|blockack)|showcases/routing/manet)$' -r '^0$'`, using local `--load . -p inet -m debug --no-build --no-concurrent`. All run/seed values are zero. The native stored limits are 10 seconds for the nine QoS runs, one second for the seven aggregation/Block Ack showcase runs, and 100 seconds for the three MANET runs. Ingredients are `tplx`, `~tNl`, `~tND`, except Gpsr has no stored `~tND`; no selected key has multiple time limits. The stored limits differ from some INI defaults and were preserved exactly.

The campaign ran for 829.414 seconds after selection; CPU allowance remained 600 seconds per simulation. Parent source and store hashes remained unchanged. Exact tuples and calculated/stored fingerprints are retained in `/tmp/inet-mesh-c1-native-fingerprint-comparison-record.json`; execution and normalized records are `/tmp/inet-mesh-c1-native-fingerprint-execution.md` and `/tmp/inet-mesh-c1-native-fingerprint-normalized.json`, with raw logs under `/tmp/inet-mesh-c1-native-fingerprint-raw/`. This is the parent reference for a later identical final-tree comparison; it neither proves mesh correctness nor clears the separate failing protocol control.

## C4 committed trajectory checkpoint

An isolated clean worktree at `03489ad1f9272adc65df67a7f2deea56456964a2` completed an incremental debug build and the identical serial native campaign above. All 19 simulations reached their stored limits and all 56 checks passed, exit 0, in 823.375 seconds. Joining the exact case/run/limit/ingredient keys against the preserved C1 reference found 56 byte-identical calculated fingerprint values, zero differences and no first divergent case. Feature state and the baseline store remained unchanged; the active in-progress C5 tree was not used.

Execution, normalized and per-case records are `/tmp/inet-mesh-c4-checkpoint-fingerprint-execution.md`, `/tmp/inet-mesh-c4-checkpoint-fingerprint-normalized.json` and `/tmp/inet-mesh-c4-checkpoint-fingerprint-comparison-record.json`; raw logs are under `/tmp/inet-mesh-c4-checkpoint-fingerprint-raw/`. Copies are retained under `audit/hwmp-implementation/`. This intermediate trajectory check does not replace final-tree verification or establish mesh protocol correctness.

## C5 directional airtime execution

C5 ran on `03489ad1f9` plus its working diff after a fresh matching `make MODE=debug -j4`, exit 0. The new General/run-0/seed-0 production fixture passed with both metric IDs: actual DATA/ACK attempts drive the directional EWMA and encoded costs, terminal timeout flags, local metric replies, accepted optional-rate updates and rejected fixed-mode incompatibility. Strengthened cases check collision-only exhaustion, TTL/pre-transmission drops, retired callbacks, both corrupt-response timing outcomes, Close interruption, fresh re-peering and isolated management stop with queued data. A test-only transparent recipient-service proxy verifies that the real recipient cleanup returns before the synchronous closed-peer observer runs; no production telemetry was added.

The final protocol selection passed all seven mesh files and five of six legacy controls. `WifiBlockAckDataRetry` retained its independently reproduced run-0 protocol failure and run-1 runtime error at the same event/time boundaries; it remains a failed control. All five selected units passed, including both-ID arithmetic, below-one aggregation rejection, rounding and saturation; the two frame-sequence/Rx completion module controls also passed. Exact selectors, raw artifacts and boundaries are recorded in the [C5 reviewer document](../../../../../tests/protocol/wifi/mesh/hwmp-c5-airtime.md). Final logs are `/tmp/inet-mesh-c5-final-{build,protocol,unit,module}.log`, with retained raw artifacts under `/tmp/inet-mesh-c5-raw/`. Scoped architecture, release-note format and whitespace checks passed. Three reviewer diagrams rendered and were visually inspected under `audit/hwmp-implementation/diagrams-c5/`. This establishes the selected unaggregated OFDM estimator/reporting policy; production HWMP and aggregation remain subsequent stages.

An isolated clean worktree at committed C5 `7264369846` also completed `make MODE=release -j4`, exit 0, with matching release OMNeT++ dependencies and no compiler warning/error. Features and the baseline store remained unchanged; no simulations ran. Records are `/tmp/inet-mesh-c5-release-checkpoint.md` and `/tmp/inet-mesh-c5-release-checkpoint-record.json`, with the build log beside them and copies under `audit/hwmp-implementation/`. This intermediate compilation checkpoint does not replace the final combined-tree release build.

## C6 reactive discovery execution

C6 ran on `7264369846` plus its working diff after a fresh matching debug build. All three new protocol files passed; path selection executes separate ID-1 and ID-2 runs. They exercise actual three-station control/data exchange, both PREQ- and PREP-driven pending reverse release, asymmetric directional costs, freshness/wrap/half-range/saturation, flags and multiple targets, unsupported/malformed/nonpeer admission, bounded shared queues, retry budgets, exact expiry and preserved identities/pacing across isolated restarts. The [C6 reviewer document](../../../../../tests/protocol/wifi/mesh/hwmp-c6-discovery.md) explains each oracle and the model policies separately from IEEE requirements.

The final 16-file protocol batch passed all ten mesh files and five of six legacy controls. `WifiBlockAckDataRetry` retained both exact parent failure boundaries. Five selected units and two frame-sequence/Rx module controls passed. Initial expanded-fixture failures were corrected oracles: later DATA changed the cost estimate after control receipt, and a purported worse metric was actually cheaper in ID-2 units. The late-Close failure at 180.04 ms/event 1714 was a real isolated-management-down defect; the narrow mesh down-input discard fixed it, and the final fixture keeps that delivery reachable rather than isolating the radio.

Raw final fixtures/logs are under `/tmp/inet-mesh-c6-raw/{protocol-final,unit-final,module-final}/`; build and category logs use `/tmp/inet-mesh-c6-*`. Scoped architecture, changed NED/MSG naming, source-seal, release/migration format and whitespace checks passed. Existing broader naming/interface findings remain explicitly outside those scoped passes. All three diagrams rendered with Mermaid 10.9.3 and Firefox and were visually inspected under `audit/hwmp-implementation/diagrams-c6/`. No baseline, declared expectation, source seal or exception ledger was changed. Final combined-tree compilation/fingerprints remain separate gates; path-error/proactive/high-throughput/power-save claims remain pending their stages.

## C7 path-error execution

C7 ran on `ed1b774a4f` plus its working diff with a fresh matching debug build. Both new fixtures passed. Actual terminal DATA acknowledgment failure invalidates the selected B–C path once, sends the required error and enables later A–D–C discovery/delivery while an unrelated route remains usable. Separate on-air peer closure retires queued originals without a second terminal failure. The receive-table fixture covers each reason/sequence/next-hop/TTL case, whole-body external rejection, forwarding-disabled behavior, pressure splitting 25 destinations into 19+6, another recipient's ten destinations, and the global default handoff interval.

Additional passing cases cover closed/re-peered precursor admission, expired deferred dependencies, exact duplicates versus differing semantic values, immediate repair pruning, stop/restart pacing, inactive management, rejected-response recovery, internal-collision retry exhaustion and unknown-transit exclusions. High-cardinality records are created through valid control processing; the fixture does not claim twenty-five additional live radio endpoints. The [C7 reviewer document](../../../../../tests/protocol/wifi/mesh/hwmp-c7-errors.md) records setup, assertions, interpretation boundaries and exact selectors.

The final batch passed all twelve mesh files and five of six legacy controls (17/18 files). The remaining Block Ack retry control retained both exact parent failures. Five units and two modules passed. Logs are `/tmp/inet-mesh-c7-{build-final,protocol-final,unit-final,module-final}.log`; raw generated fixtures/output are under `/tmp/inet-mesh-c7-raw/{protocol-final,unit-final,module-final}/`. Scoped architecture/naming/seal, release/migration format and whitespace checks passed; independent review found no actionable issue. All three diagrams rendered and were visually inspected under `audit/hwmp-implementation/diagrams-c7/`. Final combined-tree gates, L1 mechanical lint corrections and later mechanisms remain pending. No expectation or baseline was rewritten.

## C7 committed trajectory checkpoint

A clean isolated worktree at `95fe1da335` ran the same 19 native simulations/56 fingerprint checks as C1 and C4, serially with unchanged stored limits, features and baseline store. All simulations reached their native limits; all checks passed, exit 0, and all 56 calculated values were byte-identical to C1. First difference is null. Runtime was 787.553 seconds.

The initial debug link failed with SIGBUS because the 16-GiB `/tmp` filesystem was full; no simulation ran against that output or the retained C4 library. Relocating 7,177 task-owned output entries to the larger filesystem preserved bytes, modes, timestamps and source paths. A supported `LN='ln -sf'` install override handled the cross-filesystem library link. Fresh relink and explicit loader verification established the matching C7 debug library before the successful campaign. Retained release output remained identified as C5.

Records are `/tmp/inet-mesh-c7-checkpoint-fingerprint-execution.md`, `/tmp/inet-mesh-c7-checkpoint-fingerprint-comparison-record.json` and `/tmp/inet-mesh-c7-checkpoint-fingerprint-normalized.json`, with raw segments under `/tmp/inet-mesh-c7-checkpoint-fingerprint-raw/` and copies under `audit/hwmp-implementation/`. This pre-HT/VHT checkpoint is additional trajectory evidence; final combined-tree validation remains required.

## C8 proactive-root execution

C8 ran on `95fe1da335` plus its working diff after a fresh matching debug build. All three new fixtures pass with metric ID 1/run 0 and metric ID 2/run 1. The proactive fixture includes source demand both before and after a flag-zero invitation, sustained flag-one refresh, idle leaf-only traffic and completed root-state observation. The RANN fixture checks held-confirmation propagation, individually addressed confirmation without an existing path, source expiry while confirmation continues, interval boundaries, directional costs, pacing history across isolated restart and inactive-management pause/recovery. Coexistence checks two actual roots, ordinary discovery, shared source/confirmation spacing, receiver-compatible batching, the 2302-byte bound, pressure spills, fair recipient progress and independent expiry. Synthetic root records are pressure inputs, not additional live radio endpoints.

The final batch passed all fifteen mesh files and five of six legacy controls (20/21 files). `WifiBlockAckDataRetry` retains the exact independently reproduced parent failures: run-0 step-2 deadline at 1.023764276562 s/event 3708 and run-1 exact-start abort at 1.014105463846 s/event 2639. Five units and two module controls pass. Exact selectors and explanation are in the [C8 reviewer document](../../../../../tests/protocol/wifi/mesh/hwmp-c8-roots.md). Logs are `/home/miguel/inet-mesh-c8-{build-final,protocol-final,unit-final,module-final}.log`, with generated fixtures, support snapshots and output under `/home/miguel/inet-mesh-c8-raw/{protocol-final,unit-final,module-final}/`. Copies are retained under `audit/hwmp-implementation/`.

Earlier RANN failures were fixture assumptions about injected-control ownership and a fresh restart interval. Coexistence setup needed to hold an intermediate reply and put specific INI intervals before wildcard settings. These corrections changed no production behavior or expected result. Independent final review is clear, including the source-demand/observation corrections, fixture oracles and inactive-management case. Scoped architecture, changed NED/MSG naming, source-seal, release/migration format and whitespace checks pass. All three diagrams rendered and were visually inspected, and scoped local links/anchors passed. Final combined-tree compilation/fingerprints and later high-throughput/power-save stages remain separate requirements. No baseline or expected failure was rewritten.
