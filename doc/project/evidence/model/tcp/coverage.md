# TCP — coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [catalog.md](../../standard/rfc9293/catalog.md), [features.md](../../protocol/tcp/features.md), [results.md](results.md)

The single place that holds the changing state of the TCP workflow. Every other artifact
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
| [RFC9293-EST-1](../../standard/rfc9293/catalog.md#rfc9293-est-1) | selected | [connection-establishment](../../protocol/tcp/checks/establishment.md#connection-establishment) | Rfc9293ConnectionEstablishment.test | PASS |
| [RFC9293-EST-2](../../standard/rfc9293/catalog.md#rfc9293-est-2) | selected | [connection-establishment](../../protocol/tcp/checks/establishment.md#connection-establishment) | Rfc9293ConnectionEstablishment.test | PASS |
| [RFC9293-ISS-1](../../standard/rfc9293/catalog.md#rfc9293-iss-1) | covered | [connection-establishment](../../protocol/tcp/checks/establishment.md#connection-establishment) | Rfc9293ConnectionEstablishment.test | PASS |
| [RFC9293-ISS-2](../../standard/rfc9293/catalog.md#rfc9293-iss-2) | candidate (unit and module) | — | — | — |
| [RFC9293-SEQ-1](../../standard/rfc9293/catalog.md#rfc9293-seq-1) | selected | [connection-establishment](../../protocol/tcp/checks/establishment.md#connection-establishment) | Rfc9293ConnectionEstablishment.test | PASS |
| [RFC9293-SEQ-2](../../standard/rfc9293/catalog.md#rfc9293-seq-2) | candidate | — | — | — |
| [RFC9293-ACK-1](../../standard/rfc9293/catalog.md#rfc9293-ack-1) | selected | [data-transfer](../../protocol/tcp/checks/data-transfer.md#data-transfer) | Rfc9293DataTransfer.test | PASS |
| [RFC9293-ACK-2](../../standard/rfc9293/catalog.md#rfc9293-ack-2) | covered | [data-transfer](../../protocol/tcp/checks/data-transfer.md#data-transfer) | Rfc9293DataTransfer.test | PASS |
| [RFC9293-FIN-1](../../standard/rfc9293/catalog.md#rfc9293-fin-1) | selected | [connection-termination](../../protocol/tcp/checks/termination.md#connection-termination) | Rfc9293ConnectionTermination.test | PASS |
| [RFC9293-FIN-2](../../standard/rfc9293/catalog.md#rfc9293-fin-2) | selected | [connection-termination](../../protocol/tcp/checks/termination.md#connection-termination) | Rfc9293ConnectionTermination.test | PASS |
| [RFC9293-DATA-1](../../standard/rfc9293/catalog.md#rfc9293-data-1) | selected | [data-transfer](../../protocol/tcp/checks/data-transfer.md#data-transfer) | Rfc9293DataTransfer.test | PASS |
| [RFC9293-SEG-1](../../standard/rfc9293/catalog.md#rfc9293-seg-1) | selected | [data-transfer](../../protocol/tcp/checks/data-transfer.md#data-transfer) | Rfc9293DataTransfer.test | PASS |
| [RFC9293-OPT-1](../../standard/rfc9293/catalog.md#rfc9293-opt-1) | covered | [connection-establishment](../../protocol/tcp/checks/establishment.md#connection-establishment) | Rfc9293ConnectionEstablishment.test | PASS; the SYN header is 24 octets, the option is present |
| [RFC9293-PSH-1](../../standard/rfc9293/catalog.md#rfc9293-psh-1) | selected | [push-on-the-last-segment](../../protocol/tcp/checks/data-transfer.md#push-on-the-last-segment) | Rfc9293Push.test | **FAIL, expected — model gap**: the model never sets PSH |
| [RFC9293-WND-1](../../standard/rfc9293/catalog.md#rfc9293-wnd-1) | selected | [flow-control](../../protocol/tcp/checks/flow-control.md#flow-control) | Rfc9293FlowControl.test | PASS |
| [RFC9293-WND-2](../../standard/rfc9293/catalog.md#rfc9293-wnd-2) | selected | [flow-control](../../protocol/tcp/checks/flow-control.md#flow-control) | Rfc9293FlowControl.test | PASS |
| [RFC9293-ZWP-1](../../standard/rfc9293/catalog.md#rfc9293-zwp-1) | later (level 4) | — | — | — |
| [RFC9293-ACKD-1](../../standard/rfc9293/catalog.md#rfc9293-ackd-1) | covered | [flow-control](../../protocol/tcp/checks/flow-control.md#flow-control) | Rfc9293FlowControl.test | PASS; one delayed acknowledgment observed at 0.2 s, under the bound; the bound as a distribution is level 4 |
| [RFC9293-CKSUM-1](../../standard/rfc9293/catalog.md#rfc9293-cksum-1) | selected | [connection-establishment](../../protocol/tcp/checks/establishment.md#connection-establishment), [checksum-by-default](../../protocol/tcp/checks/checksum.md#checksum-by-default) | Rfc9293ConnectionEstablishment.test, Rfc9293ChecksumDefault.test | PASS for the presence of the field; **FAIL** for its value in the default state, gap 2 |
| [RFC9293-HDR-1](../../standard/rfc9293/catalog.md#rfc9293-hdr-1) | selected | [connection-establishment](../../protocol/tcp/checks/establishment.md#connection-establishment) | Rfc9293ConnectionEstablishment.test | PASS |
| [RFC9293-RST-1](../../standard/rfc9293/catalog.md#rfc9293-rst-1) | selected | [connection-reset](../../protocol/tcp/checks/reset.md#connection-reset) | Rfc9293Reset.test | PASS |
| [RFC9293-CKSUM-2](../../standard/rfc9293/catalog.md#rfc9293-cksum-2) | selected | [checksum-discard](../../protocol/tcp/checks/checksum.md#checksum-discard) | Rfc9293ChecksumDiscard.test | PASS |
| [RFC9293-SEGA-1](../../standard/rfc9293/catalog.md#rfc9293-sega-1) | selected | [out-of-window-segment](../../protocol/tcp/checks/segment-acceptance.md#out-of-window-segment) | Rfc9293OutOfWindowSegment.test | PASS |
| [RFC9293-SEGA-2](../../standard/rfc9293/catalog.md#rfc9293-sega-2) | selected | [out-of-window-segment](../../protocol/tcp/checks/segment-acceptance.md#out-of-window-segment) | Rfc9293OutOfWindowSegment.test | PASS |
| [RFC9293-RST-2](../../standard/rfc9293/catalog.md#rfc9293-rst-2) | selected | [out-of-window-segment](../../protocol/tcp/checks/segment-acceptance.md#out-of-window-segment) | Rfc9293OutOfWindowSegment.test | PASS |
| [RFC9293-RST-3](../../standard/rfc9293/catalog.md#rfc9293-rst-3) | selected | [no-reset-for-a-reset](../../protocol/tcp/checks/reset.md#no-reset-for-a-reset) | Rfc9293NoResetForReset.test | PASS |
| [RFC9293-RSTP-1](../../standard/rfc9293/catalog.md#rfc9293-rstp-1) | selected | [blind-reset](../../protocol/tcp/checks/reset.md#blind-reset) | Rfc9293BlindReset.test | PASS |
| [RFC9293-RSTP-2](../../standard/rfc9293/catalog.md#rfc9293-rstp-2) | selected | [valid-reset](../../protocol/tcp/checks/reset.md#valid-reset) | Rfc9293ValidReset.test | PASS |
| [RFC9293-WND-3](../../standard/rfc9293/catalog.md#rfc9293-wnd-3) | selected | [no-window-shrink](../../protocol/tcp/checks/window.md#no-window-shrink) | Rfc9293NoWindowShrink.test | PASS; for a transfer whose receiving program consumes at once |
| [RFC9293-WND-4](../../standard/rfc9293/catalog.md#rfc9293-wnd-4) | selected | [shrunk-window](../../protocol/tcp/checks/window.md#shrunk-window) | Rfc9293ShrunkWindow.test | PASS |
| [RFC9293-WND-5](../../standard/rfc9293/catalog.md#rfc9293-wnd-5) | selected | [no-new-data-past-a-shrunk-edge](../../protocol/tcp/checks/window.md#no-new-data-past-a-shrunk-edge) | Rfc9293ShrunkWindowNoNewData.test | **FAIL**, gap 3 |
| [RFC9293-ICMP-1](../../standard/rfc9293/catalog.md#rfc9293-icmp-1) | covered | [soft-icmp-error](../../protocol/tcp/checks/icmp.md#soft-icmp-error) | Rfc9293SoftIcmpError.test | PASS; the report reached the connection, which is what the check needed of it |
| [RFC9293-ICMP-2](../../standard/rfc9293/catalog.md#rfc9293-icmp-2) | selected | [source-quench](../../protocol/tcp/checks/icmp.md#source-quench) | Rfc9293SourceQuench.test | **FAIL**, gap 4 |
| [RFC9293-ICMP-3](../../standard/rfc9293/catalog.md#rfc9293-icmp-3) | selected | [soft-icmp-error](../../protocol/tcp/checks/icmp.md#soft-icmp-error) | Rfc9293SoftIcmpError.test | PASS |
| [RFC9293-ICMP-4](../../standard/rfc9293/catalog.md#rfc9293-icmp-4) | candidate | — | — | — |

35 entries. 31 reached a test: 27 with a PASS and 4 with a declared FAIL that names a model
gap. 4 wait for another level or another test category: RFC9293-ISS-2 and RFC9293-SEQ-2 for a
unit or module test, RFC9293-ZWP-1 for level 4, and RFC9293-ICMP-4 for a decision about which
behaviour the model intends.

## Feature support

The rule of step 7: `supported` when every core check ran and passed, `partial` when a core
check passed and another core check failed as a model gap or did not run, `not supported`
when every core check that ran failed as a model gap, `untested` when no core check ran. A
failure of a `supporting` check does not lower the value; it is noted beside the feature.

| Feature | Core checks and their state | Support |
| --- | --- | --- |
| [TCP-F-ESTABLISH](../../protocol/tcp/features.md#tcp-f-establish) | RFC9293-EST-1 PASS, EST-2 PASS | **supported** |
| [TCP-F-SEQUENCE](../../protocol/tcp/features.md#tcp-f-sequence) | RFC9293-SEQ-1 PASS, FIN-1 PASS | **supported** |
| [TCP-F-ACKNOWLEDGE](../../protocol/tcp/features.md#tcp-f-acknowledge) | RFC9293-ACK-1 PASS | **supported** |
| [TCP-F-TERMINATE](../../protocol/tcp/features.md#tcp-f-terminate) | RFC9293-FIN-2 PASS | **supported** |
| [TCP-F-DATA-TRANSFER](../../protocol/tcp/features.md#tcp-f-data-transfer) | RFC9293-DATA-1 PASS, SEG-1 PASS; supporting RFC9293-PSH-1 **FAIL (model gap)** | **supported**, with the PSH gap noted |
| [TCP-F-FLOW-CONTROL](../../protocol/tcp/features.md#tcp-f-flow-control) | RFC9293-WND-1 PASS, WND-2 PASS | **supported** |
| [TCP-F-CHECKSUM](../../protocol/tcp/features.md#tcp-f-checksum) | RFC9293-CKSUM-2 PASS; CKSUM-1 PASS for the field, **FAIL** for its value by default | **partial** |
| [TCP-F-HEADER](../../protocol/tcp/features.md#tcp-f-header) | RFC9293-HDR-1 PASS | **supported** |
| [TCP-F-RESET](../../protocol/tcp/features.md#tcp-f-reset) | RFC9293-RST-1 PASS; supporting RFC9293-RST-3 PASS | **supported** |
| [TCP-F-SEGMENT-ACCEPTANCE](../../protocol/tcp/features.md#tcp-f-segment-acceptance) | RFC9293-SEGA-1, SEGA-2, RST-2 all PASS | **supported** |
| [TCP-F-RESET-VALIDATION](../../protocol/tcp/features.md#tcp-f-reset-validation) | RFC9293-RSTP-1, RSTP-2 both PASS | **supported** |
| [TCP-F-WINDOW-ROBUSTNESS](../../protocol/tcp/features.md#tcp-f-window-robustness) | RFC9293-WND-4 PASS; supporting WND-3 PASS, WND-5 **FAIL (model gap)** | **supported**, with the new-data gap noted |
| [TCP-F-ICMP-HANDLING](../../protocol/tcp/features.md#tcp-f-icmp-handling) | RFC9293-ICMP-3 PASS, ICMP-1 PASS, ICMP-2 **FAIL (model gap)** | **partial** |

Eleven features supported, two partial. Pass 2 recorded eight supported and one partial out
of nine; pass 3 added four features and moved one.

- `TCP-F-CHECKSUM` stays partial, and for a different reason than before. Pass 2 called it
  partial because the receive half had not run. That half runs now and passes: a segment with
  a wrong checksum is discarded and the stream still arrives. What is partial now is the send
  half in the default state; see gap 2.
- `TCP-F-SEGMENT-ACCEPTANCE` and `TCP-F-RESET-VALIDATION` are the two features that decide
  whether a connection can be destroyed by a third party who cannot see it. Both are
  supported, on every core check.
- `TCP-F-WINDOW-ROBUSTNESS` is supported because the requirement with the keyword, MUST-34,
  holds: the sender survives. The supporting `should not` of SHLD-15 fails; see gap 3. The
  rule of step 7 does not lower a feature for a supporting check, and the finding stands
  beside it.
- `TCP-F-ICMP-HANDLING` is partial: the report reaches the right connection and a soft error
  does not end it, and a Source Quench stops the run; see gap 4.

Bounds: every feature rests on one connection, one link, one data size per check, and one
loss at a time; the flow-control feature rests on a fixed small window, not on a closing one,
because the model's default mode never closes it (see the results). The four features of
pass 3 rest on a single crafted event each, which is what an edge is.

## Achieved level

**Level 2 reached.** Target: level 2, from
[`standards.md`](../../protocol/tcp/standards.md#target-level).

**Hold** — every normal-path mandatory mechanism of RFC 9293 appears as a feature. The
inventory names each mechanism of the document and where it went; two exclusions are
judgments and say so.

| RFC 9293 mechanism | Where | Feature, or the reason it is not one |
| --- | --- | --- |
| header format, data offset, checksum, MSS option | §3.1, §3.7.1 | TCP-F-HEADER, TCP-F-CHECKSUM, TCP-F-ESTABLISH |
| sequence numbers, initial sequence number | §3.4 | TCP-F-SEQUENCE |
| connection establishment | §3.5 | TCP-F-ESTABLISH |
| connection close | §3.6 | TCP-F-TERMINATE |
| segmentation, acknowledgment, the push function | §3.7, §3.8, §3.9.1.2 | TCP-F-DATA-TRANSFER, TCP-F-ACKNOWLEDGE |
| window management | §3.8.6 | TCP-F-FLOW-CONTROL |
| reset for a connection that does not exist | §3.5.2, §3.10.7.1 | TCP-F-RESET |
| retransmission and its timer | §3.8.1 | level 3 for the mechanism (needs loss), level 4 for the timer (RFC 6298) |
| congestion control | §3.8.2 | level 4; RFC 5681 |
| zero-window probing, the acknowledgment delay bound, TIME-WAIT | §3.8.6.1, §3.8.6.3, §3.6 | level 4: timers and distributions |
| a shrunk window, silly window avoidance, reset on a live connection, ICMP handling, simultaneous open and close | §3.8.6, §3.5.2, §3.9.2, §3.10 | level 3: need a fault or a crafted segment |
| urgent data | §3.8.5 | **judgment:** mandatory to implement (MUST-30 to MUST-32) and discouraged in use by the same document; no ordinary transfer exercises it, so it is not a normal-path mechanism for level 2; level 5. The model does not support it, see the results |
| keep-alives, path MTU discovery, the other options, precedence and security | §3.8.4, §3.7.2, §3.2, §3.9 | `may`, `should`, or extensions; level 5 |

**Run** — every mandatory feature has a core check that ran and has a verdict: 13 of 13.

**Level 3 reached.** The exit criterion of level 3 is that the catalogs hold every mandatory
statement of the in-scope documents, including each MUST NOT, and that each one has a check.
RFC 9293 is the only document in the set, and the level 3 pass added the twelve statements of
its edges: the acceptance test and the answer to a segment that fails it, the two reset
rules, the two rules a reset must not break, the three window rules and the four ICMP rules.
Each one has a check that ran, with one exception, RFC9293-ICMP-4, which carries a `should`
and not a `must`. Every `must` and every `must not` of the in-scope set now has a verdict.

TCP is the first protocol in this tree to reach level 3 rather than level 3 partial. The
reason is in the document, not in the model: RFC 9293 replaced the family that updated
RFC 793, so its edges are stated in the same text as its normal path, and none of them
describes an interface that two nodes on a link cannot show each other. IPv4, IPv6 and UDP
each stop short because some of their statements live at the interface to the program above.

| Level | State | Evidence |
| --- | --- | --- |
| 1, Survey | **reached** | The standards map pins RFC 9293; [`conformance.md`](conformance.md) maps the 15 claimed RFCs onto it and names every obsolete one. |
| 2, Core | **reached** | The inventory above; 17 of 17 level-2 core statements with a PASS, and one declared model gap on a supporting statement. |
| 3, Edge | **reached** | Twelve edge statements added from RFC 9293 itself, no new document needed; 11 new tests, 8 PASS and 3 declared FAIL naming three model gaps. Only RFC9293-ICMP-4, a `should`, has no check. |
| 4, Dynamics | not started | RFC 6298 and RFC 5681 are not in the in-scope set; ZWP-1, ACKD-1 and TIME-WAIT wait for statistical checks. The retransmission timeout already carries three checks of level 3, which is a hint of how much of TCP lives at level 4. |
| 5, Complete | not started | urgent data, keep-alives, the options, PMTU. |

## Pass log

| Pass | Date | Level | Scope | Result |
| --- | --- | --- | --- | --- |
| 1 | 2026-09-08 | 2, partial | RFC 9293 in scope; 10 catalog entries; 4 features; 3 checks; 3 tests; flow control had no feature. On a branch since dropped; the tests were restored from it. | 5 PASS |
| 2 | 2026-09-08 | **2, reached** | 22 catalog entries (12 new), 9 features (5 new: data transfer, flow control, checksum, header, reset), 6 checks (3 new), 6 tests (3 new, 2 extended) | 8 tests: 7 PASS, 1 FAIL (expected) — the PSH bit; 8 features supported, 1 partial; see [`results.md`](results.md) and [`conformance.md`](conformance.md) |
| 3 | 2026-09-09 | **3, reached** | RFC 9293's edges added to the same catalog: 13 entries, 4 features, 11 checks, 11 tests. No new document | 19 tests: 15 PASS, 4 declared FAIL naming four model gaps; 11 features supported, 2 partial; see [`results.md`](results.md) |

## Out of scope

The catalog records what this pass left out: retransmission and its timer, congestion
control, a shrunk or zero window beyond the probe statement, the options other than MSS,
reset handling on a live connection, urgent data, keep-alives, the simultaneous cases, and
the TIME-WAIT duration. Each is level 3 to level 5 work, and the inventory above says
which.
