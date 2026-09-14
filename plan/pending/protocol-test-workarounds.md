# The workarounds the framework refactor made unnecessary

**Status:** in progress. Started 2026-09-14 on `master`.

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

## Group 2 — six ARP tests that read a printed line

**These establish less than they could.**

A greedy step consumed its whole window, so six ARP tests moved their observation to a line
the receiving program prints at the end of the run. That reads the application's summary
rather than the protocol exchange, and it cannot say when the packet arrived.

With `meanwhile` the count runs beside the steps that follow, so the observation returns to
the wire.

- [ ] `Rfc826CachedMapping`, `Rfc826LearningFromRequest`, `Rfc826ThirdStationRequest`,
      `Rfc1122ArpFloodPrevention`, `Rfc1122NoDestinationUnreachable`, and the sixth the
      grep finds.
- [ ] Each observation moves from the printed line to the event it is about.
- [ ] The deviation notes that explain the workaround go with it.

## Group 3 — nine tests that carry a combined guard

**A failure here cannot say which rule broke.**

`silenceBroken`, in both `Ipv4Mutations.h` and `UdpMutations.h`, is one predicate that tests
two unrelated rules at once, because two `never` steps could not cover one window. Nine
tests use it.

Split into two named guards, a failure names the rule that broke, which today it cannot.

- [ ] Four IPv4 tests and five UDP tests.
- [ ] Two guards each, with a describe that names the rule.
- [ ] `silenceBroken` goes from both helper headers once nothing calls it.

## Not in this plan

**Five tests declare two relays where one would now do**: `ipv6/Rfc8200UnassignedNextHeader`,
`ipv6/Rfc8200UnrecognizedNextHeader`, `tcp/Rfc9293SourceQuench`, `dhcp/Rfc2131LeaseExpiry`,
`ipv4/Rfc791SameIdDifferentProtocol`. They are correct and clear, and folding them to one
relay changes nothing about what they establish. The DHCP one also carries a comment working
out which occurrence the second relay sees, which would go with it. Do this when one of them
is next touched for another reason.
