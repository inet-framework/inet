# TCP level 4 — the two control loops, and the obsolete-citation sweep

**Status:** done. Started 2026-09-11 on `topic/rfc-tests-tcp-level4`, finished
2026-09-14 on `master`.

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
- [x] Step 6: the tests. All five written.
      - `Rfc6298FirstMeasurement.test` fails and finds a **defect**. The model has no
        first-measurement case: `TcpBaseAlg` smooths the first measurement like any other,
        so the smoothed value is R/8 rather than R and the timeout is 2.4 times what
        RFC 6298 gives.
      - `Rfc6298BackoffDoubling.test` passes. The model doubles at every expiry, 3 to 6 to
        12 to 24 seconds. Only the link side is checked: the expiry publishes no signal, so
        the estimator side of the check cannot be observed.
      - `Rfc6298KarnsRule.test` passes. Exactly one sample is taken, and the expiry
        cancels the measurement. The observable is the absence of a publication, which
        needed the guard that does not block.
      - `Rfc6298TimeoutAfterLostSyn.test` passes, with a caveat for the results: the
        model reaches three seconds because that is its default, not because the SYN loss
        raised it.
      - `Rfc6298InitialTimeout.test` passes. The initial timeout is three seconds, which
        RFC 6298 permits as a value above the recommended one second. The timeout is not
        published as a signal before the first measurement, so the check reads it
        behaviourally, from the interval to the first retransmission.
- [x] Steps 7 to 9: run, results, conformance, categories. Done on 2026-09-14 in commit
      5df12238a5. All ten level 4 tests are in `model/tcp/results.md`, the rules are in
      `conformance.md`, and `categories.md` and `coverage.md` carry the two control loops.

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
- [x] Step 6: the tests. All five written.
      - `Rfc5681FastRetransmit.test` passes. Three duplicates repair the loss, the
        threshold falls, and the window becomes the threshold plus three segments.
        Observation 4 is a sharpening candidate, not a claim.
      - `Rfc5681InitialWindow.test` passes, after a correction: it read the first
        publication of `cwnd`, which is already one acknowledgment later than the initial
        window, so it passed without establishing anything. It measures the window on the
        wire now.
      - `Rfc5681WindowAfterLostSyn.test` passes, measured the same way. The model
        implements the rule and quotes RFC 5681 in the code.
      - `Rfc5681TimeoutResponse.test` passes. The window becomes one segment and the
        threshold falls inside the bound. Needed `fromNth` on the relay, so that a whole
        window is removed and fast retransmit cannot repair the loss first.
      - `Rfc5681SlowStartGrowth.test` passes. Writing it found a framework fault: a
        predicate was refused on a scalar event, so the guard held over nothing and the
        check could not fail. Fixed, and the check is decisive now.
- [x] Steps 7 to 9: run, results, conformance, categories. Done on 2026-09-14 in commit
      5df12238a5. All ten level 4 tests are in `model/tcp/results.md`, the rules are in
      `conformance.md`, and `categories.md` and `coverage.md` carry the two control loops.

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

One finding is already known from the code, and it is a **defect** by the new rule. With
`increasedIWEnabled` the model computes the initial window by the RFC 3390 formula,
`min(4*SMSS, max(2*SMSS, 4380))`. RFC 5681 replaced that formula with a table. The two agree
at SMSS 536 and at 1460, and the formula exceeds the table for every SMSS from 1096 to 1459:
at 1096 it gives 4380 bytes, four segments, where the table allows 3288 bytes and three
segments. Code exists and produces the wrong value, so the test carries no declaration.

## Rules this pass follows

Master rewrote these rules on 2026-09-11, after this branch started. The guide now carries a
third principle, *a claimed feature gets a test*, and five failure classes instead of three.
This pass follows the new rules.

- No source file changes.
- The catalogs and the checks come from the RFC text alone. The code enters at step 6.
- A failing test stays. It is a finding about the model, and the most valuable output of the
  workflow.
- **A failure is declared expected only when the model does not claim the behaviour.** The
  test is: does code exist for this specific behaviour? If it exists, the failure is a
  **defect** and nothing is declared. Only an absent behaviour — no code path, no function,
  a TODO, a stated limitation — may be declared.
- A statement whose check cannot be built is an **untestable claim**: the test is written, it
  fails unconditionally, and its description names the missing part. It is not an omission.

This changes what this pass expects to find. Both control loops are written in the model, so
almost every difference the checks find will be a **defect**, and almost nothing will carry a
declaration. The initial-window finding below is the clearest case: the code computes a value
and computes the wrong one.

It also changes how the third framework gap is recorded. A check that cannot bind the first
publication of a signal is an untestable claim, so it becomes a failing test that names the
missing part, and not a `later` row in the ledger.

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
