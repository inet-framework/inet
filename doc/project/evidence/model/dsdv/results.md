# DSDV results

> **Kind:** report · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks](../../protocol/dsdv/checks.md)

At specification freeze no test had run. The plan supplies the implementation contract; the source
owner is `src/inet/routing/dsdv/`, using public packet, UDP, lifecycle, interface
and route-table APIs. No sealed source changes are required.

The independent NED declaration was written first in `/tmp/dsdv-stage/Dsdv.ned`
because the installed module name collides with the excluded implementation.
Its installation and the router's UDP attachment belong to the adapter change.
Legacy `DsdvHello` wire classes remain isolated compatibility types and are not
used as an algorithm or encoding oracle.

## State foundation, 2026-11-08

From `/home/miguel/workspace/inet-dsdv`, after `source .envrc` and activating
`/tmp/dsdv-python/bin/activate`:

```
make -j8 MODE=debug
make -j4 MODE=debug
inet_run_unit_tests -p inet -m debug --no-build \
  -f 'Dsdv(Foundation|Dissemination|Graph)\.test$' --no-concurrent \
  --log-file /tmp/dsdv-unit-runner.log
```

Fresh debug build: exit 0. Unit cases: **3 PASS**, no expected failures,
seed 0 (the unit algorithms draw no random numbers). Raw output:
`/tmp/dsdv-unit.log`; generated artifacts: `tests/unit/work/Dsdv*/`.
Selection, signed metric and checked sequence boundaries, byte-exact encoding,
malformed input, advertised-view settling, corrective obligations, payload
overflow, separate freshness clocks, retained floors, restart state, graph
shortest paths and successor acyclicity passed these bounded checks.

An initial encoding check mutated byte 23, which changes a high sequence byte,
instead of byte 27, which changes parity. The independent test oracle was
corrected; no production change was required. Early runner attempts before the
fresh library was linked were setup errors, not behavioral evidence.

Scoped architecture and naming checks both exited 0. Production UDP integration,
kernel scheduling, lifecycle, release and feature-off checks remain pending.

## Production attachment and forwarding

After `make -j4 MODE=debug` (exit 0), run:

```
inet_run_protocol_tests -p inet -m debug --no-build -w dsdv \
  -f 'DsdvForwarding\.test$' --no-concurrent \
  --log-file /tmp/dsdv-forwarding-runner.log
```

Observed **1 PASS**, no expected failure; configuration General, run/seed 0,
10 seconds. `tests/protocol/dsdv/work/DsdvForwarding/test.out` records 284
accepted updates, 8 line replies, 8 pair replies, zero disabled-line replies
and both intermediate IPv4 ingress paths. The verifier checks every converged
line route's owner, /32 prefix, MANET source, hop count and adjacent next hop.
The isolated node has no owned learned route. The disabled line has no route
to its remote endpoint. Local ARP is used; connected and configurator routes
are disabled. Payload capacity is three entries, exercising split full dumps.

Fixture corrections: explicit C++ namespace, assigned sink port, /24 static
interface allocation (the configurator reserves network/broadcast addresses),
and `initFromDisplayString=false` to honor configured coordinates. The observer
uses IPv4 ingress signals: this checkout's IPv4 egress path does not emit its
declared `packetSentToLower` signal. No core change was made for that gap.
Exact-start radio checks are disabled to permit the required startup collisions.

The adapter uses `RoutingProtocolBase`'s inherited `NUM_INIT_STAGES`, UDP callback
ownership, `InterfaceReq` and `FragmentationReq`, and generic route APIs with
`getSource()==this`. A real interface-down test found the need for
`Enter_Method_Silent()` in the signal handler; the adapter includes it.

Only after design freeze were external old example/showcase parameter
references inspected for migration. The removed `dsdv.netmask` override and
the showcase's IP-level attachment were updated. No old algorithm, serializer,
tests or behavior served as an oracle. Legacy wire classes remain compiled but
the new adapter never creates or consumes them as its native representation.

## Lifecycle and kernel expiry

```
inet_run_module_tests -p inet -m debug --no-build \
  -f 'DsdvLifecycle\.test$' --no-concurrent \
  --log-file /tmp/dsdv-lifecycle-runner.log
```

Observed **1 PASS**, General/run/seed 0, eight seconds, fresh debug adapter.
Artifacts: `tests/module/work/DsdvLifecycle/`. The test injects datagrams at the
production UDP callback boundary; it does not claim radio or UDP delivery.
Actual kernel events establish that a normal-priority refresh at the exact
neighbor expiry timestamp cancels that expiry. Destination expiry still occurs
when later valid packets omit that destination. An unrelated manual route
survives DSDV's interface-down cleanup. All three protocol timers are canceled.
Carrier and UP state together restore operation without a module restart.
The recovery startup view repeats a withdrawal already advertised before the
outage. ScenarioManager then performs real node shutdown/startup and
crash/startup; retained floors and self-sequence advances are asserted separately.

## Kernel dissemination

```
inet_run_module_tests -p inet -m debug --no-build \
  -f 'Dsdv(Lifecycle|Scheduling)\.test$' --no-concurrent \
  --log-file /tmp/dsdv-module-runner.log
```

Observed **2 PASS**, no expected failures, General/run/seed 0. The scheduling
case runs for two seconds. Raw output: `/tmp/dsdv-module.log`; artifacts:
`tests/module/work/DsdvScheduling/` and `DsdvLifecycle/`. Scheduling injects at
the production callback boundary and inspects actual IPv4 projections, pending
event timestamps, advertised state and emitted submission signals. It establishes
full-dump damping, a non-sliding .45-second deadline despite successive metric
and sequence updates, baseline-equal corrective submission, correction during
settling, sequence-only silence, periodic promotion and three synchronous
datagrams on required-entry overflow at capacity two. Invalid version and
duplicate destination packets leave both route freshness and expiry scheduling
unchanged. Pure optional-sequence overflow is separately covered by the unit case.

The state self-audit added an urgent-coalescing case: a newer finite change
arriving before an already obligatory send must inherit that urgency. The
foundation's required set now makes the current record eligible immediately;
its unit check verifies that no old advertisement or later deadline survives.

The additional `DsdvPaperTables.test` unit case passed (General/run/seed 0)
through the same debug unit runner with filter `DsdvPaperTables\.test$`.
It reconstructs the literal sequences, metrics and next hops from the original
paper's §4 Tables 1–4, including MH1's change from sequence 406/metric 2 through
MH2 to sequence 516/metric 3 through MH6. The versioned profile carries sender
identity in its header rather than reserving a sender-first entry. The original
PDF table images were inspected independently; generated artifact directory:
`tests/unit/work/DsdvPaperTables/`, log `/tmp/dsdv-paper-test.log`.

Final lifecycle expansion also performs down/up/carrier restoration within one
kernel event, then asserts that the rebound UDP socket is open. This passed;
the actual dispatcher/provider ordering did not exhibit a delayed-close race,
so no extra socket-generation state or shutdown mechanism was introduced.
The forwarding case additionally observes the source's real ARP request and
asserts that it resolves adjacent next hop 10.0.0.2, not endpoint 10.0.0.4.
Both expanded tests passed against the restored fresh debug library; logs:
`/tmp/dsdv-flap-test.log` and `/tmp/dsdv-forwarding-arp.log`.

The final signal audit added a received odd-sequence withdrawal to the scheduling
case. The adapter reports `invalidationReason=remoteWithdrawal` when removing
the corresponding owned IPv4 projection; malformed and ignored packets do not
emit this transition. Debug and release rebuilds and that focused case passed
(`/tmp/dsdv-observability-*-build.log`, `/tmp/dsdv-observability-test.log`).

The final startup audit tests valid empty input immediately after interface-up,
before its startup advertisement callback. This exposed a cancellation path:
ordinary advertisement rescheduling could see no triggered obligations and
cancel the mandatory startup full dump. `scheduleTimers()` now preserves that
dump until its callback submits it. Both modes rebuilt and the expanded
lifecycle case passed; logs: `/tmp/dsdv-startup-{debug,release}-build.log` and
`/tmp/dsdv-startup-test.log`.

## Mobile example, isolation and repeatability

`DsdvMobile.test` runs `examples/dsdv` configuration Mobile, run/seed 0, for
32 seconds through `DsdvRouter`'s production attachment. It exports the recorded
`pingRxSeq:vector` using `opp_scavetool export -F CSV-R` with an exact module/name
filter. Replies occur before t=8, none in [12,22], and replies resume after t=26.
The run records 57 requests and 24 replies; this is a delivery check, not a
throughput or loss-performance claim. Artifacts are in
`tests/protocol/dsdv/work/DsdvMobile/`, including `simulation.log`,
`mobile.vec`, `mobile.sca`, and `replies.csv`.

Feature profiles were saved outside the tree and restored after each campaign:

```
opp_featuretool disable -f Dsdv
make -j8 MODE=debug
# Verify nm -D -C src/libINET_dbg.so contains no inet::dsdv:: symbols.
opp_featuretool disable -f all
opp_featuretool enable -r DsdvExamples
make -j8 MODE=debug
```

Both builds exited 0. The isolated profile enables 12 required features and
disables AODV, DYMO, GPSR and the other routing implementations. Running the
mobile check's extracted `run.py` against that library also exited 0 and passed
all three delivery windows. Logs: `/tmp/dsdv-feature-off-build.log`,
`/tmp/dsdv-isolated-build.log`, `/tmp/dsdv-isolated-features.log`,
`/tmp/dsdv-isolated-mobile.log`; isolated artifacts: `/tmp/dsdv-minimal-run/`.
The restored full profile then rebuilt in debug and release successfully.

A first-time Mobile/32s/run-0 `tplx` fingerprint is `2bf5-fcd0`. It is a new row
for a new example; no existing recorded expectation was changed. Two same-seed
runs match it. Disabling scalar/vector recorders and changing logging to
non-express/off also matches it. Logs: `/tmp/dsdv-fingerprint-discovery.log`,
`/tmp/dsdv-repeat2.log`, `/tmp/dsdv-observer-neutrality.log`. Old DSDV trajectory
expectations were neither used as correctness oracles nor regenerated.

## Final focused verification

Working directory is the repository root, with `source .envrc` and
`source /tmp/dsdv-python/bin/activate`. This checkout exposes native `inet_run_*`
runners; the packaged skill's `opp_run_*`/`--load` interface is absent. The local
runner's help and implementation were checked before selecting these commands.

```
make -j8 MODE=debug
make -j8 MODE=release
inet_run_unit_tests -p inet -m debug --no-build \
  -f 'Dsdv(Foundation|Dissemination|Graph|PaperTables)\.test$' --no-concurrent \
  --log-file /tmp/dsdv-final-unit-runner.log
inet_run_module_tests -p inet -m debug --no-build \
  -f 'Dsdv(Lifecycle|Scheduling)\.test$' --no-concurrent \
  --log-file /tmp/dsdv-final-module-runner.log
inet_run_protocol_tests -p inet -m debug --no-build -w dsdv \
  -f 'Dsdv(Forwarding|Mobile)\.test$' --no-concurrent \
  --log-file /tmp/dsdv-final-protocol-runner.log
inet_run_fingerprint_tests -p inet -m debug --no-build \
  -w '^examples/dsdv$' -c '^Mobile$' -r '^0$' --no-concurrent \
  --log-file /tmp/dsdv-final-fingerprint-runner.log
```

Observed: **4 unit PASS, 2 module PASS, 2 protocol PASS, 1 fingerprint PASS**;
no expected failures or zero-case selections. All commands and both builds
exited 0. Raw runner output is in the matching `/tmp/dsdv-final-*.log` files;
generated test artifacts remain under their respective `work/Dsdv*/` directories.
The eight correctness checks use General/run/seed 0 except the Mobile example,
which uses Mobile/run/seed 0. No seed range or performance uncertainty is claimed.

## Project gates and capability gaps

| Gate | Status | Scope and interpretation |
| --- | --- | --- |
| Debug and release compilation | PASS | Fresh libraries in both modes |
| Scoped architecture and naming | PASS | New DSDV source/module subtree |
| Commit and classification checks | PASS | `origin/master..HEAD`; required trailers retained |
| Source seals | PASS | No sealed source changed |
| Document seal index | PASS | No stale index or changed sealed unit |
| Global architecture | existing findings | Unchanged infrastructure/protocol coupling candidates, including ledgered AV-ORG cases; no DSDV hit |
| Global naming | existing findings | Unchanged plural/underscore directories and assets; no DSDV declaration hit |
| Global interfaces | existing findings | 13 existing `I<Stem>` violations outside DSDV; ledger holds known contract cases |
| C++ lint | NOT_RUN | `clang-tidy` is absent; checker exits 2 |
| Skill-suite package checks from local AGENTS.md | NOT_RUN | This is an INET checkout; `scripts/validate_skill_suite.py`, `scripts/package_skill_suite.py` and `tests/skill-suite/` are absent |

Global gate logs are `/tmp/dsdv-gate-{architecture,naming,interfaces,commits,
classification,source-seals,seals,cpp}.log`. Skill package/deployment commands
were attempted using a new temporary deployment directory outside the repository;
their missing-infrastructure diagnostics are in `/tmp/dsdv-skill-*.log`. These
are capability gaps, not passing checks or DSDV behavior failures. Existing
unrelated violations and the user's pre-existing `AGENTS.md` edit were preserved.

The self-audit retraced packet ownership, numeric bounds, timer priority,
initialization stages, stop/crash/restart, route ownership, interface recovery,
feature dependencies, wire length accounting and observational neutrality.
No new source exception or baseline approval is required. No external simulator
or excluded DSDV algorithm was consulted. The tests and declared profile bound
the conformance claim; global repository cleanup and unavailable tooling remain
outside this implementation's scope.
