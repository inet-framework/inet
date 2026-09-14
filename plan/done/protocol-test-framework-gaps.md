# Protocol test framework — the gaps the standards passes found

**Status:** done. Started 2026-09-11 on `topic/rfc-tests-tcp-level4`, finished 2026-09-14 on `master`.
Every item below is implemented and covered by a self test. Phase 2, which removed the old
words, is done.

Seven protocols now have a standards pass, and nearly every one left a `notes.md` with a
"tooling quirks" section. This plan collects what those sections ask for, adds what the TCP
level 4 pass found, and settles the vocabulary before any of it is written.

The sources are
[`ipv4/notes.md`](../../doc/project/evidence/model/ipv4/notes.md#tooling-quirks),
[`arp/notes.md`](../../doc/project/evidence/model/arp/notes.md#tooling-quirks),
[`ipv6/notes.md`](../../doc/project/evidence/model/ipv6/notes.md#tooling-quirks),
[`tcp/notes.md`](../../doc/project/evidence/model/tcp/notes.md#scenario-and-tooling-quirks),
[`quic/notes.md`](../../doc/project/evidence/model/quic/notes.md#the-framework-cannot-see-quic),
and [`dhcp/notes.md`](../../doc/project/evidence/model/dhcp/notes.md#scenario-quirks).

## What the passes reported, ranked by how many hit it

| Gap | Passes | State |
| --- | --- | --- |
| One relay carries one rule | ipv4, ipv6, tcp, dhcp | **done**, item 4 below |
| The engine is a strictly sequential consumer | ipv4, arp, dhcp, tcp level 4 | **done**, item 3 below |
| A unit-bearing capture breaks the next step | tcp, ipv6 | **done**, item 1 below |
| An address field cannot be compared in an expression | arp, dhcp | not in this plan |
| A scalar signal refuses a predicate | quic, tcp level 4 | **done**, 2026-09-14 |
| A scalar signal type aborts the run | quic, tcp level 4 | **done**, c39f2ee13c |
| A scalar signal compares by equality only | tcp level 4 | **done**, c39f2ee13c |
| The raw event trace is off in a self-contained test | arp | not in this plan |
| The tester ends the run at its verdict | arp | not in this plan |

## The principle behind the vocabulary

**A word that compares must say whether it filters or asserts. A word that names a place or
a name compares nothing, so it stays bare.**

A comparison can always be read two ways — *pick the ones where*, or *require that* — and
today the framework mixes them. Worse, one word already means three things:
`EventPattern::packet(e)` and `EventPattern::match(e)` set the same field, `match(lambda)` is
an overload of the second, and `Interception::match(e)` is a third meaning on another type.

The scope words are unaffected: `on`, `source`, `signal`, `protocol`, `dispatch`, `iface`.

## Item 1 — the capture bug

`formatCaptureValue` in `EventPattern.cc` pushes **every** stored capture through
`intValue()`, in a loop, whether or not the step's expression names it. A capture of a
unit-bearing field such as `24B` therefore breaks the **next** step with "Attempt to use the
value '24B' as a dimensionless number".

- [x] Convert a capture when the expression names it, not before.
- [x] Keep a unit-bearing value in its text form.
- [x] The TCP and IPv6 notes record the workaround; both now say it is fixed.
- [x] `self/CaptureWithUnit.test` holds both halves. It fails without the repair with the
      reported error, and passes with it.

## Item 2 — filter and assertion

A pattern has two halves and one vocabulary. Split them.

- **A filter picks the event.** When nothing matches, the step misses its deadline.
- **An assertion must hold on the picked event.** When it does not, the step fails at once
  and looks no further.

| | Filter | Assertion |
| --- | --- | --- |
| expression over the packet | `filterPacket(e)` | `assertPacket(e)` |
| | — | `assertNotPacket(e)` |
| predicate over the event | `filterEvent(f)` | `assertEvent(f)` |
| scalar equality | `filterValue(v)` | `assertValue(v)` |
| | — | `assertNotValue(v)` |
| scalar lower bound | `filterValueAtLeast(v)` | `assertValueAtLeast(v)` |
| scalar upper bound | `filterValueAtMost(v)` | `assertValueAtMost(v)` |
| scalar range | `filterValueBetween(lo, hi)` | `assertValueBetween(lo, hi)` |

Position words compare nothing and stay bare: `first()`, and `nth(k)`, the word the relay
already uses. `first()` is `nth(1)`; it earns its place by making the intent visible.

Three decisions that this table records:

- **There is no `assertNotThat`.** A lambda negates itself, so `assertNotThat(f)` is exactly
  `assertEvent(!f)`. `assertNotPacket` is not redundant in the same way: it differs from
  `assertPacket` of a negated expression when the chunk is **absent**, which is the case the
  ARP and IPv6 passes lost time to.
- **`assertValueAtLeast` does not collide with `atLeastTimes`.** The cardinality family carries
  the `Times` suffix, and that suffix is the distinction.
- **The verb is `filter`, not `select`.** INET's own `PacketFilter` uses the word.

Semantics, stated exactly:

1. A step needs at least one filter. An assertion alone picks every event.
2. Without a position word, an assertion applies to the **first** event the filter matches.
3. A step with an assertion **fails on the picked event**. It never looks for a later one.
   That is the whole difference from a filter.
4. `never` takes no assertion, and the framework refuses one. "This must not happen, and
   when it happens it must hold P" means nothing.
5. With `exactlyTimes(n, ...)` and its relatives, the assertion must hold on **each** match.

- [x] Add the six filter words and the eight assertion words.
- [x] Add `first()` and `nth(k)` to a pattern.
- [x] Rename the internal fields from `sel*` to `flt*`, on 2026-09-14. Nineteen fields in
      `EventPattern`, and their uses in `EventPattern.cc`, `ProtocolTester.cc` and
      `ProtocolTestDescriber.cc`. The user-facing vocabulary already said `filter`; only the
      private fields still said `sel`. No verdict moved. One test description quoted the old
      code by field name, and the quote is now a sentence instead.
- [x] The describer renders an assertion differently from a filter, after an arrow, and
      `str()` now shows the scalar filters and the position word it used to omit.
- [x] `tcp/Rfc5681InitialWindow.test` states its rule directly. Verified both ways: with a
      bound below the real window it fails with "the value is 1072, and it must be at
      most 100".

## Item 3 — the non-blocking step

The engine keeps one `currentStep` cursor, offers each event to that step alone, and waits
out a window before it advances. Four reported gaps come from that one design:

- a greedy step consumes its whole window (three ARP tests moved an observation because of it);
- two consecutive `never` steps cannot cover one window (IPv4 wrote one `never` with a
  combined lambda instead);
- two consecutive `once` steps cannot read one event (DHCP observed one message at two
  modules instead);
- a pattern cannot bind the first publication of a signal (TCP level 4).

`anyOf` and `unordered` do not reach any of them. Both combine *what may match* inside one
step; neither lets anything run beside it.

Two rules remove all four:

- **(a) A step may run alongside the steps after it.** The word is `meanwhile`, and it takes
  a whole step: `.meanwhile(never(...))`. The cardinality words become free builders beside
  `on`, so a step can be written without a program to hold it.
- **(b) One event reaches every active step, not the first one only.** Rule (b) alone fixes
  the DHCP case.

- [x] Make the cardinality words free builders that return a step: `never`, `atMostTimes`,
      `atLeastTimes`.
- [x] Add `meanwhile(step)`.
- [x] Offer an event to every active step.
- [x] A concurrent step carries its own window, its own anchor and its own count.
- [x] `self/TwoGuardsOneWindow.test` needs both halves at once: two guards open together,
      an ordered observation inside them, and one event that a guard and the ordered step
      both read. Verified: a guard that should fire reports
      "forbidden event occurred at t=0.1 for the guard of step 0".

## Item 4 — the relay holds one rule

`PacketTap::configure` assigns its fields, so each `intercept` clause replaces the one
before it. Four passes worked around it with two taps in series, and the DHCP pass had to
write down the arithmetic of which occurrence the second tap sees.

A tap accumulates rules. Each clause adds one. A frame is offered to the rules in order and
the first that matches applies; a frame that matches none passes.

- [x] `PacketTap` holds a list of rules.
- [x] `configure` appends instead of replacing.
- [x] Add `pass()` as an explicit action, which shadows a later rule for the frames it names.
- [x] `self/TwoRulesOneRelay.test` proves it, and is decisive in both directions: without
      the first rule step 1 misses its deadline, without the second the `never` fires.
- **A compiled `PacketFilter` does not survive a copy.** A rule holds one by pointer, and the
  list is a `deque`, so nothing that is compiled ever moves. A `vector<Rule>` with the filter
  by value segfaults during the tester's initialize.

## Item 5 — the builder and the step adder share a word

`intercept` is both the free builder that makes a clause and the method that adds it, so a
line reads `intercept(intercept("tap")...)`. `inject` has the same shape. `on` avoids it by
accident, because the adder is `once`.

| Role | Today | Proposed |
| --- | --- | --- |
| build a relay clause | `intercept("tap")` | `tap("tap")` |
| build an injection | `inject("host1")` | `at("host1")` |
| add either to the program | `.intercept(...)`, `.inject(...)` | unchanged |

`Interception::match(e)` becomes `filterPacket(e)`, which removes the third meaning of `match`.

- [x] Add `tap(name)` and `at(name)`.
- [x] Add `Interception::filterPacket`.

## The migration

**Phase 1 is additive and nothing breaks.** Every new word arrives beside the old one, and
the roughly 1700 existing call sites keep working.

**Phase 2 is done**, on 2026-09-14. 871 call sites changed and the old words are gone from
the API. Splitting `.match(` by its argument was mechanical after all, and all seven cases
the split could not decide turned out to be prose.

Three things a blanket rename got wrong. None was caught by the compiler alone; the suites
found every one.

- `Injection::packet(builder)` is **not** a filter. It supplies the packet to inject, and its
  argument is a function, not an expression. 26 call sites were renamed wrongly and restored.
- The **step adders** `ProtocolTest::inject` and `ProtocolTest::intercept` keep their verbs.
  A rename of the free builders that does not exclude a declaration renames them too, and
  then every test that adds a step fails to compile.
- Six mentions were prose, and two of those described a limitation that had since been
  fixed, so they were wrong twice over.

## Order of work

1. Item 1, the capture bug. Smallest, and two passes lost a cycle to it.
2. Item 2, the vocabulary. Three of the nine remaining TCP level 4 tests need it.
3. Item 5, the builder words. Cheap, and it goes with item 2.
4. Item 4, the relay rules. Four of the nine remaining TCP tests drop a segment.
5. Item 3, the non-blocking step. The deepest change, and it removes four workarounds.
6. ~~`AUTHORING.md` documents the new vocabulary, and the notes lose the quirks that are
   gone.~~ Done. Seven quirks across five notes now say which ones are fixed, and two
   follow-up items are closed.
