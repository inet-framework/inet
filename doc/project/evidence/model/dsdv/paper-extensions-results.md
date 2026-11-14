# DSDV paper extension execution record

> **Kind:** report · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [plan](../../../../../plan/pending/dsdv-paper-extensions.md)

The extension was developed from the implementation pending upstream in PR #1444 at `dc594f8b2e0b2cd6c7ef23f484af356143202e00`. The user authorized implementation and publication on 2026-11-14. This branch's prerequisite copy ends at `ab395478ab8182e3c2f635901e1f3d770096a2c6`: its reviewer-document placement, size explanations, observer icon and two prose claims were corrected during the whole-PR audit. The original PR and its branches were not rewritten. Every mapped prerequisite and extension commit retains identical production source, feature dependencies and fingerprint-store bytes; the only test-input change is the forwarding observer's display icon. The focused forwarding rerun passes (`/tmp/dsdv-extensions-display-forwarding.log`). This record's no-baseline-change claims apply to the extension commits; inherited prerequisite baseline changes remain in #1444's own evidence.

The implementation series is C0 `ad874294ff`, C1 `e8a93caab1`, C2 `3b9cbc0c5b`, C3 `2084ba7a30`, C4 `e81556809f`. The scarcity-only optional-ordering correction was folded into C1, changing descendant identifiers. Final corrected-source builds, all 18 correctness cases, all 13 scoped nonvisual fingerprint comparisons, exact-parent refreshes and isolated feature-off compilation pass. The feature-checkpoint sections preserve their earlier observations; the final integration and parent tables below identify the current tested revisions. PR publication and audit evidence are recorded separately in the PR and review reports.

| Feature | Build | Direct tests | Parent comparison |
| --- | --- | --- | --- |
| C1 incremental fairness | debug PASS | dissemination unit and fairness protocol PASS | Exact parent `ad874294ff` FAIL, C1 `e8a93caab1` PASS |
| C2 MAC feedback | debug PASS | dissemination unit, lifecycle/scheduling modules and MAC protocol PASS | Exact parent `e8a93caab1` FAIL, C2 `3b9cbc0c5b` PASS |
| C3 adaptive full dumps | debug PASS | dissemination unit, scheduling/invalid-config modules and adaptive protocol PASS | Exact parent `3b9cbc0c5b` FAIL, C3 `2084ba7a30` PASS |
| C4 base-station routes | debug/release PASS | wire/graph/dissemination units, six lifecycle/gateway modules and gateway protocol PASS | New profile cannot run unchanged on the parent |

The rows above preserve each feature's focused checkpoint. The pre-correction C4 checkpoint at `f15537d5ca889d56d7a96a6165d7992f4339c4df` has fresh debug/release compilation and **4 unit, 8 module and 6 protocol cases passing**. Its isolated IPv4+UDP feature-off build also passes. The first fingerprint run's four `~tND` mismatches motivated the scarcity-only correction. The current `e81556809f` reruns pass all 18 correctness cases; the 13/13 fingerprint comparison uses byte-identical simulation inputs from the preceding source revision, as detailed below. These are executed observations, not dry-run selections. The extension changes no fingerprint baseline.

## C1: Incremental fairness

The authoring contract was checked against `DsdvTable::prepare`, `suspend` and its adapter caller before editing: the table owns the cursor, required entries keep priority, only submitted optional entries advance it, and a full dump/session resets it. The staged C1 production snapshot was built before its direct tests. Later C2 source edits are not part of this result.

From the checkout root on 2026-11-14, debug mode:

```sh
make -j8 MODE=debug
opp_run_unit_tests --load . -p inet -m debug --no-build -f 'DsdvDissemination\.test$'
opp_run_protocol_tests --load . -p inet -m debug --no-build -w '(^|/)dsdv$' -f 'DsdvIncrementalFairness\.test$'
doc/project/enforcement/check-architecture.sh src/inet/routing/dsdv
```

Build, scoped architecture and both selected tests exited 0, with one unit and one protocol case executed. The protocol fixture uses General/run/seed 0 and observes real UDP/radio delivery of optional destinations 4, 5, 6, required-only capacity and full-dump reset. The unit case additionally checks wraparound, candidate removal and uniqueness.

Raw build/unit logs: `/tmp/dsdv-extensions-c1-build.log`, `/tmp/dsdv-extensions-c1-unit.log`; final protocol log: `/tmp/dsdv-c1-protocol-final.log`. Generated protocol output is `tests/protocol/dsdv/work/DsdvIncrementalFairness/test.out`; later protocol runs may replace generated work. The first fixture attempts exposed missing outgoing-interface metadata and an incomplete include; the corrected fixture reaches the intended routing assertions.

At this C1 checkpoint, the parent explanation was source-grounded and release/integration checks were pending. The later exact-parent comparison and integrated build results below supersede that checkpoint's unexecuted items; its raw logs remain unchanged.

## C2: MAC failure feedback

The implementation contract was checked before editing: the adapter filters the emitting interface and borrows failed-frame metadata, while the table owns failed-successor invalidation. Feedback-disabled nodes do not resolve ARP or subscribe to MAC hints. The C2 source snapshot was freshly built and tested before C3 edits/builds.

From the checkout root, debug mode, 2026-11-14:

```sh
make -j8 MODE=debug
opp_run_unit_tests --load . -p inet -m debug --no-build -f 'DsdvDissemination\.test$'
opp_run_module_tests --load . -p inet -m debug --no-build -f 'Dsdv(Lifecycle|Scheduling)\.test$'
opp_run_protocol_tests --load . -p inet -m debug --no-build -w '(^|/)dsdv$' -f 'DsdvMacFailure\.test$'
doc/project/enforcement/check-architecture.sh src/inet/routing/dsdv
```

All commands exited 0: one unit case, two module cases and one protocol case. The protocol case runs three separated lines in General/run/seed 0: AckingMac feedback on, AckingMac timeout-only, and IEEE 802.11 ad hoc DCF feedback on. It observes real failed deliveries, early route removal and received odd-sequence withdrawals, while the disabled control waits for expiry and initializes with an intentionally unavailable ARP module reference. The module cases cover absent/wrong metadata, unrelated emitters, failed old receivers after a successor switch, repeated hints and inactive/restart behavior. These observations establish consumption of the tested MAC hints, not physical-disconnection detection or general HCF support.

Logs: `/tmp/dsdv-extensions-c2-build.log`, `/tmp/dsdv-extensions-c2-unit.log`, `/tmp/dsdv-extensions-c2-module.log`, `/tmp/dsdv-extensions-c2-architecture.log`, `/tmp/dsdv-c2-protocol-final.log`. Raw protocol output is `tests/protocol/dsdv/work/DsdvMacFailure/test.out`. Its DCF radio explicitly uses unit-disk signal representation to match the selected medium. The parent explanation was still source-based at this feature checkpoint; the later exact-revision comparison below records the executed difference.

## C3: Adaptive full dumps

The authoring contract was checked against the periodic callback and route-transition owner. A saturating table counter supplies selected-route history; the adapter computes bounded intervals from one submitted full dump. Disabled adaptation preserves startup phase, periodic draws and send opportunities. No timeout is rejected because of the receiver's own sending interval.

From the checkout root on 2026-11-14, debug mode:

```sh
make -j8 MODE=debug
opp_run_unit_tests --load . -p inet -m debug --no-build -f 'DsdvDissemination\.test$'
opp_run_module_tests --load . -p inet -m debug --no-build -f 'Dsdv(Scheduling|AdaptiveInvalidBounds|AdaptiveInvalidBudget)\.test$'
opp_run_protocol_tests --load . -p inet -m debug --no-build -w '(^|/)dsdv$' -f 'DsdvAdaptiveDumps\.test$'
doc/project/enforcement/check-architecture.sh src/inet/routing/dsdv
```

All commands exited 0, executing one unit, three module and one protocol case. Two module cases deliberately fail initialization and pass only when the specific invalid-configuration diagnostic/exit code matches. The protocol case checks quiet growth, churn reduction, byte-cost floors, interval clamps, urgent withdrawal, a triggered fragmented dump without periodic postponement, and heterogeneous peer intervals. The quiet receiver's first periodic spacing is exactly 2 s; its assertion observes packets before requiring the new interval statistic. The reciprocal heterogeneous pair remains reachable with no expiry through its observation window. General/run/seed 0 is used; no performance improvement is inferred.

Logs: `/tmp/dsdv-extensions-c3-build.log`, `/tmp/dsdv-extensions-c3-unit.log`, `/tmp/dsdv-extensions-c3-module.log`, `/tmp/dsdv-extensions-c3-architecture.log`, `/tmp/dsdv-c3-protocol-agent.log`. Raw protocol output is `tests/protocol/dsdv/work/DsdvAdaptiveDumps/test.out`. Exact-parent execution and release compilation were pending at this feature checkpoint; the later records below supply those results.

## C4: Base-station default routes

The implementation contract was traced through table freshness, UDP validation, IPv4 default replacement/iteration and lifecycle callbacks before editing. Version 2 adds one flags byte per entry while retaining the fixed header and version-1 bytes. One configured base station advertises sequenced capability; the adapter derives its default route without importing external prefixes. Reconciliation uses one conditional deferred event and preserves foreign routes. The test descriptor adds Ethernet without changing core DSDV's dependencies.

On 2026-11-14, fresh debug compilation and three selected unit cases passed: `DsdvFoundation`, `DsdvDissemination`, `DsdvGraph`. The lifecycle/scheduling controls passed, as did the four new gateway modules: `DsdvGatewayReconciliation`, `DsdvGatewayInvalidPayload`, `DsdvGatewayOrigin`, `DsdvUnexpectedInterface`. Invalid payload and unexpected-interface cases pass by requiring the specific initialization rejection. The origin module checks wired/ad hoc default replacement, unchanged-availability notifications without sequence/send churn, 50-byte version-2 fairness and overflow through adapter output. Reconciliation also checks rejection leaves neighbor clocks and expiry unchanged.

The focused protocol command exited 0 with one General/run/seed 0 case:

```sh
opp_run_protocol_tests --load . -p inet -m debug --no-build -w '(^|/)dsdv$' -f 'DsdvBaseStation\.test$'
```

It observes no-uplink startup, real mobile/relay/base/wired-router/external-host forwarding, intermediate traversal, urgent capability loss, retained host routes, stale/mixed-version/wrong-origin input, foreign defaults, wired/wireless recovery and DSDV-controller restart. It does not establish whole Ethernet-node reboot behavior or external Internet health. The C3 parent cannot execute the new profile unchanged, so no measured parent forwarding failure is claimed.

Logs: `/tmp/dsdv-extensions-c4-build-final.log`, `/tmp/dsdv-extensions-c4-unit.log`, `/tmp/dsdv-extensions-c4-module.log`, `/tmp/dsdv-extensions-c4-gateway-module.log`, `/tmp/dsdv-extensions-c4-reconciliation-final.log`, `/tmp/dsdv-extensions-c4-review-module.log`, `/tmp/dsdv-c4-protocol-agent.log`. The first gateway-module attempt reached setup errors (interface module-name matching and mobility coordinates); corrected focused reruns pass. Checkpoint release compilation also exited 0, log `/tmp/dsdv-extensions-final-release-build.log`. These checkpoint records precede the scarcity-only correction; final integration results follow.

## Final integrated verification

The following commands ran from `/home/miguel/workspace/inet-dsdv` against corrected C4 `e81556809f7e934747d8aa9e84fdb0768e379957`, using OMNeT++ 6.4.0aipre2. The builds and all three focused runners exited 0. Debug is the behavioral mode; release was compiled, not used for a second behavioral campaign.

```sh
make -j8 MODE=debug
make -j8 MODE=release
opp_run_unit_tests --load . -p inet -m debug --no-build -f 'Dsdv(Foundation|Graph|PaperTables|Dissemination)\.test$'
opp_run_module_tests --load . -p inet -m debug --no-build -f 'Dsdv(Lifecycle|Scheduling|AdaptiveInvalidBounds|AdaptiveInvalidBudget|GatewayReconciliation|GatewayInvalidPayload|GatewayOrigin|UnexpectedInterface)\.test$'
opp_run_protocol_tests --load . -p inet -m debug --no-build -w '(^|/)dsdv$' -f 'Dsdv(Forwarding|Mobile|IncrementalFairness|MacFailure|AdaptiveDumps|BaseStation)\.test$'
```

| Category | Executed result | Raw log |
| --- | --- | --- |
| Debug compilation | PASS | `/tmp/dsdv-extensions-published-debug-build.log` |
| Release compilation | PASS | `/tmp/dsdv-extensions-published-release-build.log` |
| Unit | 4 PASS | `/tmp/dsdv-extensions-published-unit.log` |
| Module | 8 PASS | `/tmp/dsdv-extensions-published-module.log` |
| Protocol | 6 PASS | `/tmp/dsdv-extensions-published-protocol.log` |
| Scoped fingerprints | 13/13 ingredient comparisons PASS on identical simulation bytes | `/tmp/dsdv-extensions-order-fingerprint.log` |
| Isolated feature-off compilation | PASS at `e81556809f` | `/home/miguel/workspace/inet-dsdv-featureoff-evidence-20261114/evidence/published-copy-code/record.json` |

Unit cases cover foundation bytes/arithmetic, logical graphs, dissemination and the paper tables. Module cases cover lifecycle/scheduling, gateway reconciliation/origin behavior and four invalid-configuration rejections; a rejection case is PASS only when the intended diagnostic and exit status match. Protocol cases use General/run/seed 0, except the existing mobile fixture's Mobile/run/seed 0 configuration. They include both existing host-routing controls and all four extension fixtures. Compiler warnings about hidden listener overloads are present in fixture builds; the listed tests still compile and execute their assertions.

The first campaign had 9/13 nonvisual ingredients passing and four `~tND` mismatches, recorded in `/tmp/dsdv-extensions-final-fingerprint.log`. Preserving destination order when all optional entries fit, while rotating under scarcity, corrects that regression. The final exact-order unit regression and 13/13 fingerprint rerun pass without changing expectations.

The isolated feature-off check at `e81556809f` enables only IPv4 and UDP, with Dsdv, DsdvExamples and DsdvTests disabled. Its normal `make MODE=debug -j4` build exits 0 and records 400 selected objects, zero DSDV objects and zero DSDV dynamic symbols, with tracked source matching that revision. Feature state, macro header, compiler options, source hashes and library provenance are archived beside `evidence/published-copy-code/record.json`; `evidence/published-copy-build.exit` records the successful build in the auxiliary feature-off checkout, and `evidence/published-copy-featureoff.md` explains its reproduction and limits. This is compilation/linking of the minimal disabled profile, not protocol execution or an all-feature matrix. Earlier records are retained as historical evidence.

## Scoped fingerprints and gate limits

The read-only fingerprint API campaign exited 0 with **13/13 ingredient comparisons PASS across five simulations**, using debug mode, run/seed 0, stored expectations and no rebuild. The executed revision was `c9b8448ffff11636a2bd23b01bf65f25d7220abb`, before the prerequisite documentation corrections. All production source, selected simulation configurations and stored expectations are byte-identical at `e81556809f`; the corrected test-observer icon is outside these simulations. The final build/correctness and parent-comparison campaigns above use the new identifiers. Discovery was deliberately bounded before selecting configurations:

```python
from opp_repl import load_opp_file, get_simulation_project, run_fingerprint_tests
load_opp_file("inet.opp")
project = get_simulation_project("inet")
project.ini_file_folders = ["examples/dsdv", "examples/manetrouting/dsdv", "showcases/routing/manet"]
result = run_fingerprint_tests(
    simulation_project=project, mode="debug", build=False,
    concurrent=False, stored_only=True,
    working_directory_filter=r"^(examples/(dsdv|manetrouting/dsdv)|showcases/routing/manet)$",
    config_filter=r"^(Mobile|DynamicIPv4|IPv4|MultiIPv4|Dsdv)$",
    run_number=0, ingredients_list=["tplx", "~tNl", "~tND"],
)
```

| Simulation directory | Configuration | Limit | Stored comparisons |
| --- | --- | --- | --- |
| `examples/dsdv` | Mobile | 32 s | `tplx`: 1/1 PASS |
| `examples/manetrouting/dsdv` | DynamicIPv4 | 22 s | `tplx`, `~tNl`, `~tND`: 3/3 PASS |
| `examples/manetrouting/dsdv` | IPv4 | 10 s | `tplx`, `~tNl`, `~tND`: 3/3 PASS |
| `examples/manetrouting/dsdv` | MultiIPv4 | 10 s | `tplx`, `~tNl`, `~tND`: 3/3 PASS |
| `showcases/routing/manet` | Dsdv | 100 s | `tplx`, `~tNl`, `~tND`: 3/3 PASS |

The campaign script is archived as `audit/run-dsdv-fingerprints.py`, and the raw log is `/tmp/dsdv-extensions-order-fingerprint.log`. Its leaf-result check requires exactly 13 comparisons, all PASS; the top-level five-simulation count is not substituted for ingredient results. These fingerprints detect unintended trajectory changes in the selected stored cases, not protocol correctness or arbitrary loss robustness. Visual `tyf` comparisons were not rerun in this scoped nonvisual campaign; the older host-profile visual record remains separate.

Scoped DSDV architecture and naming checks pass, source seals pass, the documentation seal check reports 20 documents passing, and scoped links pass for 10 pending-plan, 7 model, 7 protocol-reviewer and 1 catalog files. Global gates are not clean: the project link gate reports 181 missing external-standard file references, and the archived-plan directory has 23 old broken links outside the DSDV plan. Architecture/naming/interface gates retain unrelated findings outside the edited DSDV paths. No exception ledger or baseline was changed to hide those results. This build/test record does not substitute for the separately recorded PR audit.

## Exact-parent protocol comparisons

Fresh DSDV compilation and linking in the isolated checkout `/home/miguel/workspace/inet-dsdv-parent-evidence-20261114` establish the following bounded debug comparisons. Each row executes one General/run/seed 0 protocol case on each revision. The auxiliary library uses independently copied/reflinked unchanged common objects, not hardlinks to the active checkout; this is not claimed as a clean rebuild of every INET source. `ldd` records identify the auxiliary checkout's own INET and protocol-support libraries.

| Behavior | Immediate parent and observed failure | Feature result | Fixture compatibility |
| --- | --- | --- | --- |
| C1 fairness | `ad874294ffcdf7b58a59b1322e174b5e4d7fe789`: rotation assertion for destinations 4, 5, 6 fails at 55.23167256399 s | `e8a93caab1a3135470365e86bcd9a76bf5ebda7e`: PASS | Identical committed fixture; no adaptation |
| C2 MAC feedback | `e8a93caab1a3135470365e86bcd9a76bf5ebda7e`: `route == nullptr` fails at 3.6 s after a real enabled-Acking MAC failure | `3b9cbc0c5b10d33685cd4359d515f8fe5ae6fbc1`: PASS | Parent omits three assignments for unavailable feedback/ARP parameters only |
| C3 adaptive timing | `3b9cbc0c5b10d33685cd4359d515f8fe5ae6fbc1`: remote periodic gap is 1 s, so the 2 s packet-spacing assertion fails at 1.549135835868 s | `2084ba7a306f986c30e5fbc002ec8116502757ea`: PASS | Parent omits nine assignments for unavailable adaptive parameters only |

All comparison builds exited 0; each intended parent test exited 1 at the stated behavioral assertion, and its feature test exited 0. Neither omitted-parameter adaptation changes topology, traffic, mobility, existing timing settings, observer code or assertions. C2's parent stops in the first enabled Acking group, so no separate measured parent DCF failure is claimed. C3's parent stops before the heterogeneous checks; that scenario is additional coverage, not a measured parent failure. C4 introduces a profile the C3 parent cannot initialize unchanged and has no claimed parent forwarding comparison.

Final exact-reference records are under `/home/miguel/workspace/inet-dsdv-parent-evidence-20261114/evidence/published-copy-refresh/`: `comparison.md` explains the measured results, `run.log` records all six executed cases, and each `c1-parent`/`c1-feature`, `c2-parent`/`c2-feature` and `c3-parent`/`c3-feature` folder holds its `record.json`, original/used fixture, adaptation patch, stderr and library provenance. They verify the current revisions and enabled feature prerequisites. Earlier comparison folders remain historical records for their named revisions; they are not substituted for the refreshed runs.

The reproducible auxiliary command form is `make -C src -j4 MODE=debug`, followed by `opp_run_protocol_tests --load . -p inet -m debug --no-build -w '(^|/)dsdv$' -f '<case>\.test$'` for the named fairness, MAC or adaptive case. The auxiliary reports record the environment setup and complete invocations. These comparisons establish the specific repairs and controlled policy, not arbitrary loss robustness, general MAC support, whole-channel overhead guarantees or a performance improvement.
