# DSDV results

> **Kind:** report · **Status:** draft · **Seal:** none · **Owns:** — · **Stands on:** [checks](../../protocol/dsdv/checks.md)

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
