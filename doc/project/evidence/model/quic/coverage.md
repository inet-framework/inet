# QUIC — coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [catalog.md](../../standard/rfc9000/catalog.md), [features.md](../../protocol/quic/features.md), [results.md](results.md)

The single place that holds the changing state of the QUIC workflow. Every other artifact
of this protocol states what a standard says and never changes again unless the standard
changes. This one changes on every pass.

**No step edits an artifact of an earlier step. Steps 5, 6 and 7 record their outcome here.**

State of the ledger: run of 2026-09-08, on top of the IPv4 branch, source identical to
`master`.

## Statement coverage

`Status` is the state of the workflow, not of the standard: `selected` (a check targets it),
`covered` (a selected check establishes it as a side effect), `candidate` (practical, not yet
chosen), `later` (needs a toolset beyond the current level, or another test category).

| Catalog ID | Status | Check section | Test file | Verdict |
| --- | --- | --- | --- | --- |
| [RFC9000-PKT-1](../../standard/rfc9000/catalog.md#rfc9000-pkt-1) | selected | [connection-establishment](../../protocol/quic/checks.md#connection-establishment) | Rfc9000ConnectionEstablishment.test | PASS; the connection identifier is always 0 in this model, so the identifier half is degenerate |
| [RFC9000-PKT-2](../../standard/rfc9000/catalog.md#rfc9000-pkt-2) | selected | [connection-establishment](../../protocol/quic/checks.md#connection-establishment) | Rfc9000ConnectionEstablishment.test | PASS |
| [RFC9000-PKT-3](../../standard/rfc9000/catalog.md#rfc9000-pkt-3) | selected | [stream-data-transfer](../../protocol/quic/checks.md#stream-data-transfer) | Rfc9000StreamDataTransfer.test | PASS |
| [RFC9000-SIZE-1](../../standard/rfc9000/catalog.md#rfc9000-size-1) | selected | [connection-establishment](../../protocol/quic/checks.md#connection-establishment) | Rfc9000ConnectionEstablishment.test | PASS for the client half; **the server half is not checked, and the code does not implement it** |
| [RFC9000-STR-1](../../standard/rfc9000/catalog.md#rfc9000-str-1) | selected, and **not established** | [stream-identifiers](../../protocol/quic/checks.md#stream-identifiers) | Rfc9000StreamIdentifiers.test | PASS, but the pass is a coincidence of the application's choice; see below |
| RFC9000-STR-1, decisive angle | candidate, **no test** | [stream-identifier-assignment](../../protocol/quic/checks.md#stream-identifier-assignment) | — | a test was written and withdrawn: it passed without exercising the case |
| [RFC9000-STR-2](../../standard/rfc9000/catalog.md#rfc9000-str-2) | selected | [stream-data-transfer](../../protocol/quic/checks.md#stream-data-transfer) | Rfc9000StreamDataTransfer.test | PASS on a path that does not reorder; the buffering half is level 3 |
| [RFC9000-STR-3](../../standard/rfc9000/catalog.md#rfc9000-str-3) | selected | [stream-data-transfer](../../protocol/quic/checks.md#stream-data-transfer) | Rfc9000StreamDataTransfer.test | PASS |
| [RFC9000-FC-1](../../standard/rfc9000/catalog.md#rfc9000-fc-1) | selected | [flow-control](../../protocol/quic/checks.md#flow-control) | Rfc9000FlowControl.test | PASS for the stream limit; the connection limit is untouched |
| [RFC9000-ACK-1](../../standard/rfc9000/catalog.md#rfc9000-ack-1) | selected | [acknowledgment](../../protocol/quic/checks.md#acknowledgment) | Rfc9000Acknowledgment.test | PASS; the delay bound is level 4 |
| [RFC9000-CLOSE-1](../../standard/rfc9000/catalog.md#rfc9000-close-1) | selected | [connection-close](../../protocol/quic/checks.md#connection-close) | Rfc9000ConnectionClose.test | PASS |

10 entries, all reached a test, all passed. One check exists without a test, and one
passing check establishes less than its verdict suggests.

## Feature support

The rule of step 7: `supported` when every core check ran and passed, `partial` when a core
check passed and another core check failed as a model gap or did not run, `not supported`
when every core check that ran failed as a model gap, `untested` when no core check ran.

| Feature | Core checks and their state | Support |
| --- | --- | --- |
| [QUIC-F-PACKET](../../protocol/quic/features.md#quic-f-packet) | RFC9000-PKT-1, PKT-2, PKT-3 all PASS | **supported** |
| [QUIC-F-INITIAL-SIZE](../../protocol/quic/features.md#quic-f-initial-size) | RFC9000-SIZE-1 PASS for the client half only | **partial** |
| [QUIC-F-STREAMS](../../protocol/quic/features.md#quic-f-streams) | RFC9000-STR-2, STR-3 PASS; RFC9000-STR-1 passed vacuously | **partial** |
| [QUIC-F-FLOW-CONTROL](../../protocol/quic/features.md#quic-f-flow-control) | RFC9000-FC-1 PASS | **supported** |
| [QUIC-F-ACKNOWLEDGE](../../protocol/quic/features.md#quic-f-acknowledge) | RFC9000-ACK-1 PASS | **supported** |
| [QUIC-F-CLOSE](../../protocol/quic/features.md#quic-f-close) | RFC9000-CLOSE-1 PASS | **supported** |

Four supported, two partial — and the two partial values are the honest part of this pass.
Neither comes from a failing test; both come from a check that covers less of its statement
than the statement demands:

1. **QUIC-F-INITIAL-SIZE** is `partial` because RFC 9000 §14.1 binds both endpoints and the
   check observes only the client. The code confirms the asymmetry: the client pads, the
   server does not.
2. **QUIC-F-STREAMS** is `partial` because RFC9000-STR-1's passing check would pass whether
   or not the model implements the encoding. The model documents that it does not.

The rule of step 7 would have made both `supported`, because every core check that ran
passed. Recording them as `partial` is a deliberate departure from the mechanical rule, and
the reason is stated here rather than hidden: a support value that rests on a vacuous check
is worth less than the word suggests, and the ledger is where that has to be visible.

Bounds on the four supported values: every one rests on one connection over a path with no
loss and no reordering, with packet protection absent, and on checks that read chunk types
because the framework cannot dissect QUIC.

## Achieved level

**Level 2 partial.** Target: level 2, from
[`standards.md`](../../protocol/quic/standards.md#target-level).

The exit criterion of level 2 has two halves. The first holds; the second does not.

**Hold** — every normal-path mandatory mechanism of RFC 9000 appears as a feature:

| RFC 9000 mechanism | Where | Feature, or the reason it is not one |
| --- | --- | --- |
| long and short packet headers, version, connection identifiers | §17.2, §17.3 | QUIC-F-PACKET |
| packet numbers and their spaces | §12.3 | QUIC-F-PACKET |
| the Initial datagram floor | §14.1 | QUIC-F-INITIAL-SIZE |
| streams: identifiers, ordered delivery, STREAM frames | §2.1, §2.2, §19.8 | QUIC-F-STREAMS |
| flow control at two levels | §4.1 | QUIC-F-FLOW-CONTROL |
| acknowledgment of ack-eliciting packets | §13.2.1 | QUIC-F-ACKNOWLEDGE |
| immediate close | §10.2 | QUIC-F-CLOSE |
| the handshake and packet protection | §7, RFC 9001 | level 5: the cryptographic content is another document's |
| loss detection, congestion control, the idle timeout | §13, RFC 9002 | level 4 |
| errors, stateless reset, version negotiation, address validation, migration, stream reset | §10, §11, §6, §8, §9, §3 | level 3: each needs a fault or a crafted packet |

**Run** — not every mandatory feature has a core check that establishes it. Two do not:

1. `QUIC-F-STREAMS`: RFC9000-STR-1 has no check that could fail if the model ignored the
   encoding, and the model does ignore it.
2. `QUIC-F-INITIAL-SIZE`: the server half of the requirement has no check.

| Level | State | Evidence |
| --- | --- | --- |
| 1, Survey | **reached** | The standards map pins RFC 9000; [`conformance.md`](conformance.md) maps the model's declared standards onto it, including its twelve declared deviations. |
| 2, Core | **partial** | 10 of 10 statements passed, but two of the six features rest on a check that covers less than its statement. The two blockers above are the work. |
| 3, Edge | not started | errors, reordering, version negotiation, the anti-amplification limit; RFC 9000 already holds the text. Blocked first by the tooling: a QUIC dissector. |
| 4, Dynamics | not started | RFC 9002 is not in the in-scope set. |
| 5, Complete | not started | RFC 9001, the remaining frame types, migration, 0-RTT, key updates. |

## Pass log

| Pass | Date | Level | Scope | Result |
| --- | --- | --- | --- | --- |
| 1 | 2026-09-08 | **2, partial** | RFC 9000 in scope; 10 catalog entries; 6 features; 7 checks; 6 tests | 6 PASS; 4 features supported, 2 partial; the framework cannot dissect QUIC, so every check reads chunk types; see [`results.md`](results.md) and [`conformance.md`](conformance.md) |

## Out of scope

The catalog records what this pass left out: the error and reset mechanisms, version
negotiation, address validation and migration, loss detection and congestion control, the
cryptographic handshake, and the frame types beyond STREAM, ACK, PADDING and
CONNECTION_CLOSE. The catalog's closing section names the level that reaches each one.
