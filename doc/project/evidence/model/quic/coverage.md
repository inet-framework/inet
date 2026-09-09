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
| [RFC9000-PKT-1](../../standard/rfc9000/catalog.md#rfc9000-pkt-1) | selected | [connection-establishment](../../protocol/quic/checks/establishment.md#connection-establishment) | Rfc9000ConnectionEstablishment.test | PASS; the connection identifier is always 0 in this model, so the identifier half is degenerate |
| [RFC9000-PKT-2](../../standard/rfc9000/catalog.md#rfc9000-pkt-2) | selected | [connection-establishment](../../protocol/quic/checks/establishment.md#connection-establishment) | Rfc9000ConnectionEstablishment.test | PASS |
| [RFC9000-PKT-3](../../standard/rfc9000/catalog.md#rfc9000-pkt-3) | selected | [stream-data-transfer](../../protocol/quic/checks/streams.md#stream-data-transfer) | Rfc9000StreamDataTransfer.test | PASS |
| [RFC9000-SIZE-1](../../standard/rfc9000/catalog.md#rfc9000-size-1) | selected | [connection-establishment](../../protocol/quic/checks/establishment.md#connection-establishment), [server-initial-datagram-size](../../protocol/quic/checks/establishment.md#server-initial-datagram-size) | Rfc9000ConnectionEstablishment.test, Rfc9000ServerInitialSize.test | PASS for the client half; **FAIL** for the server half, gap 1 |
| [RFC9000-STR-1](../../standard/rfc9000/catalog.md#rfc9000-str-1) | **no check** | [stream-identifiers](../../protocol/quic/checks/streams.md#stream-identifiers) | Rfc9000StreamIdentifiers.test | the test passes and establishes nothing: probe 1 of pass 2 showed that the model has one identifier, 0, whatever the application asks, so no check on the wire can fail |
| RFC9000-STR-1, decisive angle | **no check**, and none is possible | [stream-identifier-assignment](../../protocol/quic/checks/streams.md#stream-identifier-assignment) | — | the same reason. The check stays written, for a model that assigns identifiers |
| [RFC9000-STR-2](../../standard/rfc9000/catalog.md#rfc9000-str-2) | selected | [stream-data-transfer](../../protocol/quic/checks/streams.md#stream-data-transfer), [ordered-delivery-under-reordering](../../protocol/quic/checks/streams.md#ordered-delivery-under-reordering) | Rfc9000StreamDataTransfer.test, Rfc9000ReorderedDelivery.test | PASS, and the buffering half now runs: data that arrives early is kept |
| [RFC9000-STR-3](../../standard/rfc9000/catalog.md#rfc9000-str-3) | selected | [stream-data-transfer](../../protocol/quic/checks/streams.md#stream-data-transfer) | Rfc9000StreamDataTransfer.test | PASS |
| [RFC9000-FC-1](../../standard/rfc9000/catalog.md#rfc9000-fc-1) | selected | [flow-control](../../protocol/quic/checks/flow-control.md#flow-control) | Rfc9000FlowControl.test | PASS for the stream limit; the connection limit is untouched |
| [RFC9000-ACK-1](../../standard/rfc9000/catalog.md#rfc9000-ack-1) | selected | [acknowledgment](../../protocol/quic/checks/acknowledgment.md#acknowledgment) | Rfc9000Acknowledgment.test | PASS; the delay bound is level 4 |
| [RFC9000-CLOSE-1](../../standard/rfc9000/catalog.md#rfc9000-close-1) | selected | [connection-close](../../protocol/quic/checks/close.md#connection-close) | Rfc9000ConnectionClose.test | PASS |
| [RFC9000-VER-1](../../standard/rfc9000/catalog.md#rfc9000-ver-1) | selected | [version-negotiation](../../protocol/quic/checks/version-negotiation.md#version-negotiation) | Rfc9000VersionNegotiation.test | **FAIL**, gap 2 |
| [RFC9000-VER-2](../../standard/rfc9000/catalog.md#rfc9000-ver-2) | candidate | — | — | needs a Version Negotiation packet delivered to an endpoint; the relay can make one now |
| [RFC9000-AMP-1](../../standard/rfc9000/catalog.md#rfc9000-amp-1) | selected | [anti-amplification-limit](../../protocol/quic/checks/address-validation.md#anti-amplification-limit) | Rfc9000AntiAmplification.test | PASS |
| [RFC9000-ERR-1](../../standard/rfc9000/catalog.md#rfc9000-err-1) | selected | [unknown-frame-type](../../protocol/quic/checks/frame-validation.md#unknown-frame-type) | Rfc9000UnknownFrameType.test | **FAIL**, gap 3 |

14 entries. 11 reached a test: 8 with a PASS and 3 with a declared FAIL that names a model
gap. RFC9000-STR-1 carries `no check`, with a proof rather than a suspicion behind it; and
RFC9000-VER-2 is a candidate the relay could now reach.

## Feature support

The rule of step 7: `supported` when every core check ran and passed, `partial` when a core
check passed and another core check failed as a model gap or did not run, `not supported`
when every core check that ran failed as a model gap, `untested` when no core check ran.

| Feature | Core checks and their state | Support |
| --- | --- | --- |
| [QUIC-F-PACKET](../../protocol/quic/features.md#quic-f-packet) | RFC9000-PKT-1, PKT-2, PKT-3 all PASS | **supported** |
| [QUIC-F-INITIAL-SIZE](../../protocol/quic/features.md#quic-f-initial-size) | RFC9000-SIZE-1 PASS for the client half, **FAIL** for the server half | **partial** |
| [QUIC-F-STREAMS](../../protocol/quic/features.md#quic-f-streams) | RFC9000-STR-2 PASS, including its buffering half; STR-3 PASS; STR-1 has no check | **partial** |
| [QUIC-F-FLOW-CONTROL](../../protocol/quic/features.md#quic-f-flow-control) | RFC9000-FC-1 PASS | **supported** |
| [QUIC-F-ACKNOWLEDGE](../../protocol/quic/features.md#quic-f-acknowledge) | RFC9000-ACK-1 PASS | **supported** |
| [QUIC-F-CLOSE](../../protocol/quic/features.md#quic-f-close) | RFC9000-CLOSE-1 PASS | **supported** |
| [QUIC-F-VERSION-NEGOTIATION](../../protocol/quic/features.md#quic-f-version-negotiation) | RFC9000-VER-1 **FAIL**; VER-2 has no check | **not supported** |
| [QUIC-F-ADDRESS-VALIDATION](../../protocol/quic/features.md#quic-f-address-validation) | RFC9000-AMP-1 PASS | **supported** |
| [QUIC-F-FRAME-VALIDATION](../../protocol/quic/features.md#quic-f-frame-validation) | RFC9000-ERR-1 **FAIL** | **not supported** |

Five supported, two partial, two not supported.

The two `partial` values are the same two the level 2 pass recorded, and both are firmer now.
Neither rests on a suspicion any more:

1. **QUIC-F-INITIAL-SIZE** was partial because the server half had no check. It has one, and
   the half fails; see gap 1.
2. **QUIC-F-STREAMS** was partial because RFC9000-STR-1 passed vacuously. A probe showed why
   it cannot do otherwise, so the statement carries `no check` and the feature stays partial
   for a proven cause. Its other half improved: RFC9000-STR-2 now runs with data arriving out
   of order, which is the harder half of what it demands, and the model keeps what arrives
   early.

The two `not supported` values are new, and they are honest applications of the rule: for each
of those features every core check that ran failed as a model gap. Neither feature exists in
the model at all — a version nobody speaks is ignored, and an unknown frame type stops the
run.

Bounds, unchanged from pass 1 and worth repeating: every feature rests on one connection over
one link, with one stream identifier, one connection identifier, and no packet protection.

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

**Run** — every mandatory feature now has a core check that ran and has a verdict: 9 of 9.
The two blockers the level 2 pass recorded are closed, and neither closed the way a reader
would have hoped:

1. `QUIC-F-STREAMS`: RFC9000-STR-1 had no check that could fail. A probe settled why. The
   model has one stream identifier, 0, whatever the application asks, so no check on the wire
   can fail. The statement carries `no check` with that proof, and the feature keeps
   `partial` for a stated cause rather than a missing test.
2. `QUIC-F-INITIAL-SIZE`: the server half now has a check, and it fails. See gap 1.

**Level 2 reached, level 3 partial.** Level 2's exit criterion is met: every normal-path
mandatory mechanism is a feature, and every mandatory feature has a core check with a verdict.
Level 3 is partial: four of the subjects the catalog files at level 3 are done, and five are
not.

| Level | State | Evidence |
| --- | --- | --- |
| 1, Survey | **reached** | The standards map pins RFC 9000; [`conformance.md`](conformance.md) maps the model's declared standards onto it, including its twelve declared deviations. |
| 2, Core | **reached** | The inventory above; the two blockers of pass 1 are closed, one by a check that fails and one by a probe that proves no check can succeed. |
| 3, Edge | **partial** | Four subjects done: ordered delivery under reordering, version negotiation, the anti-amplification limit, and the frame encoding error. Still out: the stateless reset, address validation with Retry and tokens, path validation and migration, stream reset, and a packet that cannot be decrypted. |
| 4, Dynamics | not started | RFC 9002 is not in the in-scope set. |
| 5, Complete | not started | RFC 9001, the remaining frame types, migration, 0-RTT, key updates. |

The tooling no longer gates level 3. Pass 1 recorded that a QUIC dissector was needed before
the edges could be reached; pass 2 reached four of them by selecting datagrams on their UDP
fields and their size, and changing the QUIC chunks directly. A dissector would still make
every check of this suite shorter and plainer.

## Pass log

| Pass | Date | Level | Scope | Result |
| --- | --- | --- | --- | --- |
| 1 | 2026-09-08 | **2, partial** | RFC 9000 in scope; 10 catalog entries; 6 features; 7 checks; 6 tests | 6 PASS; 4 features supported, 2 partial; the framework cannot dissect QUIC, so every check reads chunk types; see [`results.md`](results.md) and [`conformance.md`](conformance.md) |
| 2 | 2026-09-09 | **2 reached, 3 partial** | The two level 2 blockers closed with two probes; 4 catalog entries, 3 features, 5 checks, 5 tests added | 11 tests: 8 PASS, 3 declared FAIL naming three model gaps; 5 features supported, 2 partial, 2 not supported; see [`results.md`](results.md) |

## Out of scope

The catalog records what this pass left out: the error and reset mechanisms, version
negotiation, address validation and migration, loss detection and congestion control, the
cryptographic handshake, and the frame types beyond STREAM, ACK, PADDING and
CONNECTION_CLOSE. The catalog's closing section names the level that reaches each one.
