# TCP level 4 — the two control loops, and the obsolete-citation sweep

**Status:** in progress. Started 2026-09-11 on `topic/rfc-tests-tcp-level4`.

Three tasks, from the answer to "how can we extend the TCP tests":

1. **The obsolete-citation sweep.** The TCP code cites seven superseded documents. A
   level 1 survey activity: no build, no run.
2. **RFC 6298 at level 4.** The retransmission timer.
3. **RFC 5681 at level 4.** Congestion control.

## Why level 4 is cheaper than the standards map assumed

[`standards.md`](../../doc/project/evidence/protocol/tcp/standards.md) says both documents
"need statistical or timing checks rather than single-segment wire checks". That is only
half true. The model declares every control variable as a NED signal:

```
cwnd  ssthresh  rto  srtt  rttvar  dupAcks  numRtos  pipe  sndSacks  rcvSacks
```

So the state-signal channel of the protocol test framework reads them directly, the way
`ethernet/PlcaBeaconCycle.test` reads the PLCA state machines. A formula and a ratio become
a deterministic check. Only a tolerance-bearing rule stays statistical.

## Task 1 — the obsolete-citation sweep

| The code cites | The current document | Relation |
| --- | --- | --- |
| RFC 1323 | RFC 7323 | obsoletes |
| RFC 2581 | RFC 5681 | obsoletes |
| RFC 2001 | RFC 5681 | through RFC 2581 |
| RFC 3782 | RFC 6582 | obsoletes |
| RFC 2988 | RFC 6298 | obsoletes |
| RFC 3517 | RFC 6675 | obsoletes |
| RFC 1981 | RFC 8201 | obsoletes |

- [x] Confirm each relation from the RFC-editor metadata, not from memory.
- [x] Record every citation with its file, in the document list of `standards.md`.
- [x] Add the finding to part 1 of `conformance.md`, beside the RFC 793 finding.
- [x] Do not change a source file. A citation is a model claim, and this pass measures
      the model as it is.

## Task 2 — RFC 6298, the retransmission timer

- [x] Step 1: cache `standard/rfc6298/rfc6298.txt`.
- [x] Step 2: RFC 6298 enters the in-scope set; the override table gains the RFC 2988 row.
- [x] Step 3: `standard/rfc6298/catalog.md`, identifiers `RFC6298-*`. 18 entries.
- [x] Step 4: the feature map gains the retransmission timer features. Four: RTO-ESTIMATOR, RTO-BOUNDS, RTO-BACKOFF, RTT-SAMPLING.
- [x] Step 5: `protocol/tcp/checks/retransmission-timer.md`. Five checks.
- [ ] Step 6: the tests. Not started.
- [ ] Steps 7 to 9: run, results, conformance, categories.

Candidate checks, all deterministic through a signal:

| Rule | Clause | Signal |
| --- | --- | --- |
| The first RTO is 1 second before a measurement | §2.1 | `rto` |
| The first measurement sets `SRTT = R` and `RTTVAR = R/2` | §2.2 | `srtt`, `rttvar` |
| A later measurement uses the 1/8 and 1/4 gains | §2.3 | `srtt`, `rttvar` |
| `RTO = SRTT + max(G, 4*RTTVAR)` | §2.3 | `rto` |
| The RTO has a 1 second lower bound | §2.4 | `rto` |
| A timeout doubles the RTO | §5.5 | `rto`, `numRtos` |
| Karn: a retransmitted segment gives no measurement | §3 | `srtt` |

## Task 3 — RFC 5681, congestion control

- [x] Step 1: cache `standard/rfc5681/rfc5681.txt`.
- [x] Step 2: RFC 5681 enters the in-scope set; the override table gains the RFC 2581 row.
- [x] Step 3: `standard/rfc5681/catalog.md`, identifiers `RFC5681-*`. 21 entries.
- [x] Step 4: the feature map gains the congestion control features. Six: CONGESTION-WINDOW, INITIAL-WINDOW, LOSS-RESPONSE, FAST-RETRANSMIT, RESTART-IDLE, DELAYED-ACK.
- [x] Step 5: `protocol/tcp/checks/congestion-control.md`. Five checks.
- [~] Step 6: the tests. One of five written and passing: `Rfc5681InitialWindow.test`.
- [ ] Steps 7 to 9: run, results, conformance, categories.

Candidate checks:

| Rule | Clause | Signal |
| --- | --- | --- |
| The initial window follows the SMSS table | §3.1 | `cwnd` |
| Slow start doubles the window each round trip | §3.1 | `cwnd` |
| `ssthresh = max(FlightSize/2, 2*SMSS)` on a loss | §3.1 | `ssthresh` |
| Congestion avoidance adds about one SMSS per round trip | §3.1 | `cwnd` |
| Three duplicate ACKs start a fast retransmit | §3.2 | `dupAcks`, the wire |
| Fast recovery inflates and then deflates the window | §3.2 | `cwnd` |
| A timeout drops the window to one SMSS | §3.1 | `cwnd` |

Which algorithm each check runs against matters. `TcpReno` and `TcpNewReno` answer to
RFC 5681. `TcpTahoe`, `TcpVegas` and `TcpWestwood` have no RFC, so this pass does not judge
them.

## Rules this pass follows

- No source file changes. A gap becomes a `%# expected-result: FAIL` test and a row in
  `results.md`.
- A failing test stays. It is a finding about the model.
- The catalogs and the checks come from the RFC text alone. The code enters at step 6.

## What step 6 found about the framework

Level 4 is the first level whose checks read a control variable. Three gaps appeared at
once, and two are repaired on this branch.

1. **The state channel refused two signal types.** It handled `intval_t` alone, and threw
   "Unsupported signal data type" for anything else. `cwnd`, `ssthresh` and `dupAcks` are
   `uintval_t`; `rto`, `srtt` and `rttvar` are times. Repaired: four overloads now feed one
   normaliser, and the event carries a `double`, with a time in seconds.
2. **A scalar could be compared for equality only.** `is(v)` was the whole API, and almost
   every rule of these two documents states a bound: "at most 4 segments", "no less than one
   second". Repaired: `isAtLeast`, `isAtMost` and `isBetween`.
3. **A pattern cannot bind the first publication of a signal.** The engine matches the first
   event that satisfies a pattern, so `once(cwnd isAtMost(B))` passes on a model whose first
   window is too large and whose second is not. A check of an initial value must therefore
   forbid a bad value inside a time window, which ties the check to the scenario times.
   Not repaired. This is the sharpest framework request of the pass.

A fourth finding belongs to the model rather than the framework. The window is published
only when the algorithm changes it, not when the connection sets it up, so "the initial
window" is observable as the first publication and not before it.

## What step 6 found about the build

The two wireless tests of the element suite fail in this worktree and pass in `inet-master`,
from the same commit and the same framework. The cause is neither: the two checkouts have
different INET **feature sets**. `inet-master` enables all 126 features; a fresh worktree
disables 10, among them the network emulation group and `TcpLwip`. The different module set
changes the initialization order, and that order decides whether
`PacketMultiplexer::mapRegistrationForwardingGates` is reached before the module caches its
gates.

The feature set turned out not to be the cause either. `.oppfeaturestate` is now copied from
`inet-master` and the two trees build the same feature set, and the two tests still fail
here.

The cause is the removed `PacketMultiplexer` change, and one experiment proves it. With the
change applied in this worktree the two tests pass; with it reverted they crash with
`Unknown gate ... during network initialization`, which is the exact failure the change
describes. The source trees, the test files and the framework are identical between the two
checkouts, so `inet-master` passes for a reason inside its own incremental build and not
because the model is sound without the change.

**master does not pass its own element suite from a clean build.** The change should go
back. This pass does not restore it, because a standards pass changes no source file.
