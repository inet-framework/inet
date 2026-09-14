# The workarounds the framework refactor made unnecessary

**Status:** done. Started and finished 2026-09-14 on `master`.

The protocol test framework gained four things during the TCP level 4 pass: an assertion
beside a filter, a step that runs beside the steps after it, a relay that holds a list of
rules, and a predicate that may read a scalar signal. Twenty tests across six suites were
written around the absence of those things.

None of the twenty is wrong. Each is harder to read than it now needs to be, and two groups
establish **less** than they could, which is the reason to do this at all rather than leave
it as tidying.

## The rule this work follows

**A verdict may not change by accident.** Every one of these tests passes or fails for a
known reason today. A rewrite that changes a verdict is either a finding or a mistake, and
the two must not be confused, so each group ends with a full run compared against the run
before it. Where a verdict does change, the change is explained before it is committed.

The order is by what each group gains, not by how many files it touches.

## Group 1 — the QUIC flow-control check

**The only one where the limit shaped the check and not just its spelling.**

`quic/Rfc9000FlowControl.test` carries a long note naming three framework limits. Two are
gone. It says the engine "is strictly sequential and single-pass", and that a `never` window
could not be sized between two events less than 200 microseconds apart: any window large
enough to be robust would run past the point the next step needed to observe.

`meanwhile` removes exactly that. The guard runs beside the ordered steps, so its window no
longer has to end before the next observation begins.

- [x] Rewritten with two concurrent guards. It establishes **more** than before: the rule
      is now a guard that reports a violation at the moment it happens, instead of a
      condition folded into a later step's match, which could only report a deadline miss.
- [x] The note keeps the one limit that still holds and says the other two are gone.
- [x] `quic/notes.md` item 6 corrected; it contradicted the same file's follow-up list.

## Group 2 — five ARP tests that read a printed line

**These established less than they could. Done on 2026-09-14.**

A greedy step consumed its whole window, so five ARP tests moved their observation to a line
the receiving program prints at the end of the run. That read the application's summary
rather than the protocol exchange, and it could not say when the packet arrived. The plan
said six; the suite held five, and no printed-line assertion is left anywhere in
`tests/protocol`.

With `meanwhile` the count runs beside the steps that follow, so the observation is back on
the wire.

**Attempted first on 2026-09-14 and stopped. `meanwhile` was necessary and not sufficient.**

`Rfc826CachedMapping` was rewritten so that the count runs beside the steps and the five
datagrams are observed on the wire. It passed, and it was **not decisive**: demanding six
datagrams instead of five passed as well. The run had ended at t=0.3 with two datagrams
received, so the step that was meant to count five had resolved on something else.

Two things stand in the way, and neither is the greedy step:

- **The tester ends the run at its verdict.** A program step for "all five arrive" makes the
  last arrival the verdict, and the run stops there. The ARP notes already record this from
  the other side: a first version of `Rfc1122ArpPacketQueue` asserted a printed line and got
  `received 0 packets`, because the verdict came first. Moving an observation *into* the
  program moves the end of the run with it.
- **`exactlyTimes(n)` resolves on the nth match and does not forbid an n+1th.** It reads as
  "exactly n" and means "at least n", so it cannot say "five and no more". That is worth a
  look on its own: the word promises more than the step delivers.

The printed line, for all its faults, asserts a total after the run has finished, which no
step of the current engine can do. So the workaround is better judged than it looked, and
this group needs the two points above settled first.

- [x] A step may observe without ending the run, since 2026-09-14. Group 3's engine fix
      answers this one: `enterStep` waits while a guard is outstanding, and the last guard
      to resolve ends the program. So a count carried by `meanwhile` runs its window out
      after the ordered steps are done, and the run no longer stops at the last arrival.
      The six rewrites below are the proof.
- [x] `exactlyTimes` means what its name says, since 2026-09-14. It was the smaller of the
      two blockers and it is gone.
- [x] **A receiving UDP module reports each datagram twice. The model emits it twice.**
      Answered on 2026-09-14, and the tester is not at fault.
      `LayeredProtocolBase::handleLowerMessage` emits `packetReceivedFromLower` and then
      calls `handleLowerPacket`; `Udp::handleLowerPacket` calls `processUDPPacket`, which
      emits the same signal again. A temporary print in the tester's `receiveSignal` showed
      two callbacks for one packet id, at one simulation time, from one module context. UDP
      is alone in this: IPv4 and IPv6 override `handleMessageWhenUp`, so the base class
      never runs for them, and TCP emits nothing of its own. No recorded statistic doubles,
      because no `@statistic` in `Udp.ned` sources the signal. The defect is written up in
      `doc/project/evidence/model/udp/notes.md`. No source file is changed here.

      The observation moves instead of the model: `packetSentToUpper` at `<host>.ipv4.ip`
      still carries the UDP header, so `udp.destPort` filters on it, and it is emitted once
      per delivered datagram. A probe of five datagrams counts five there, and both four and
      six fail.
- [x] The five ARP tests, on 2026-09-14. Each one keeps its verdict and gains an
      observation on the link:

      | Test | The fact that left the printed line |
      | --- | --- |
      | `Rfc826CachedMapping` | five datagrams delivered up at host B, counted at `hostB.ipv4.ip` |
      | `Rfc1122NoDestinationUnreachable` | no ARP reply for 10.0.0.99 reaches host A |
      | `Rfc1122ArpFloodPrevention` | no ARP reply for 10.0.0.99 reaches host A |
      | `Rfc826ThirdStationRequest` | host C's reply on the link, exactly once |
      | `Rfc826LearningFromRequest` | host B's datagram delivered up at host A, exactly once |

      Two of them also widen an absence watch to the whole run, because the guard no longer
      has to start after an ordered step.

      **Every new check was made to fail.** Each count was run at one below and one above
      its true number, and both failed. Each new `never` was retargeted at an event that
      does occur, and failed. Full run after the group: the self suite is 19 PASS, and the
      ARP suite is 13 PASS with the 3 failures that `arp/results.md` already records as
      gaps 1 and 2. No other suite has a changed file, so no other verdict moved.

## Group 3 — nine tests that carry a combined guard

**A failure here cannot say which rule broke.**

`silenceBroken`, in both `Ipv4Mutations.h` and `UdpMutations.h`, is one predicate that tests
two unrelated rules at once, because two `never` steps could not cover one window. Nine
tests use it.

Split into two named guards, a failure names the rule that broke, which today it cannot.

- [x] Four IPv4 tests and four UDP tests, seven of them split in two and one in three.
- [x] Each guard names its own rule, so a failure says which one broke.
- [x] `silenceBroken` is gone from both helper headers.
- [x] The engine had to learn to wait for an outstanding guard. See below.

### The engine had to learn to wait

Writing this group found a gap in `meanwhile` itself. When the ordered steps ran out, the
engine decided PASS at once, **even with a guard still open**. Every test of this group ends
with its guards, so each would have passed without the guard looking at anything: the exact
shape of a check that cannot fail, in the feature built to prevent them.

`enterStep` now waits while a guard is outstanding, and the last guard to resolve ends the
program. Without that, group 3 would have turned nine honest checks into nine vacuous ones.

## Not in this plan

**Five tests declare two relays where one would now do**: `ipv6/Rfc8200UnassignedNextHeader`,
`ipv6/Rfc8200UnrecognizedNextHeader`, `tcp/Rfc9293SourceQuench`, `dhcp/Rfc2131LeaseExpiry`,
`ipv4/Rfc791SameIdDifferentProtocol`. They are correct and clear, and folding them to one
relay changes nothing about what they establish. The DHCP one also carries a comment working
out which occurrence the second relay sees, which would go with it. Do this when one of them
is next touched for another reason.
