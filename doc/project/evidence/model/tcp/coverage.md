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
| [RFC9293-EST-1](../../standard/rfc9293/catalog.md#rfc9293-est-1) | selected | [connection-establishment](../../protocol/tcp/checks.md#connection-establishment) | Rfc9293ConnectionEstablishment.test | PASS |
| [RFC9293-EST-2](../../standard/rfc9293/catalog.md#rfc9293-est-2) | selected | [connection-establishment](../../protocol/tcp/checks.md#connection-establishment) | Rfc9293ConnectionEstablishment.test | PASS |
| [RFC9293-ISS-1](../../standard/rfc9293/catalog.md#rfc9293-iss-1) | covered | [connection-establishment](../../protocol/tcp/checks.md#connection-establishment) | Rfc9293ConnectionEstablishment.test | PASS |
| [RFC9293-ISS-2](../../standard/rfc9293/catalog.md#rfc9293-iss-2) | candidate (unit and module) | — | — | — |
| [RFC9293-SEQ-1](../../standard/rfc9293/catalog.md#rfc9293-seq-1) | selected | [connection-establishment](../../protocol/tcp/checks.md#connection-establishment) | Rfc9293ConnectionEstablishment.test | PASS |
| [RFC9293-SEQ-2](../../standard/rfc9293/catalog.md#rfc9293-seq-2) | candidate | — | — | — |
| [RFC9293-ACK-1](../../standard/rfc9293/catalog.md#rfc9293-ack-1) | selected | [data-transfer](../../protocol/tcp/checks.md#data-transfer) | Rfc9293DataTransfer.test | PASS |
| [RFC9293-ACK-2](../../standard/rfc9293/catalog.md#rfc9293-ack-2) | covered | [data-transfer](../../protocol/tcp/checks.md#data-transfer) | Rfc9293DataTransfer.test | PASS |
| [RFC9293-FIN-1](../../standard/rfc9293/catalog.md#rfc9293-fin-1) | selected | [connection-termination](../../protocol/tcp/checks.md#connection-termination) | Rfc9293ConnectionTermination.test | PASS |
| [RFC9293-FIN-2](../../standard/rfc9293/catalog.md#rfc9293-fin-2) | selected | [connection-termination](../../protocol/tcp/checks.md#connection-termination) | Rfc9293ConnectionTermination.test | PASS |
| [RFC9293-DATA-1](../../standard/rfc9293/catalog.md#rfc9293-data-1) | selected | [data-transfer](../../protocol/tcp/checks.md#data-transfer) | Rfc9293DataTransfer.test | PASS |
| [RFC9293-SEG-1](../../standard/rfc9293/catalog.md#rfc9293-seg-1) | selected | [data-transfer](../../protocol/tcp/checks.md#data-transfer) | Rfc9293DataTransfer.test | PASS |
| [RFC9293-OPT-1](../../standard/rfc9293/catalog.md#rfc9293-opt-1) | covered | [connection-establishment](../../protocol/tcp/checks.md#connection-establishment) | Rfc9293ConnectionEstablishment.test | PASS; the SYN header is 24 octets, the option is present |
| [RFC9293-PSH-1](../../standard/rfc9293/catalog.md#rfc9293-psh-1) | selected | [push-on-the-last-segment](../../protocol/tcp/checks.md#push-on-the-last-segment) | Rfc9293Push.test | **FAIL, expected — model gap**: the model never sets PSH |
| [RFC9293-WND-1](../../standard/rfc9293/catalog.md#rfc9293-wnd-1) | selected | [flow-control](../../protocol/tcp/checks.md#flow-control) | Rfc9293FlowControl.test | PASS |
| [RFC9293-WND-2](../../standard/rfc9293/catalog.md#rfc9293-wnd-2) | selected | [flow-control](../../protocol/tcp/checks.md#flow-control) | Rfc9293FlowControl.test | PASS |
| [RFC9293-ZWP-1](../../standard/rfc9293/catalog.md#rfc9293-zwp-1) | later (level 4) | — | — | — |
| [RFC9293-ACKD-1](../../standard/rfc9293/catalog.md#rfc9293-ackd-1) | covered | [flow-control](../../protocol/tcp/checks.md#flow-control) | Rfc9293FlowControl.test | PASS; one delayed acknowledgment observed at 0.2 s, under the bound; the bound as a distribution is level 4 |
| [RFC9293-CKSUM-1](../../standard/rfc9293/catalog.md#rfc9293-cksum-1) | selected | [connection-establishment](../../protocol/tcp/checks.md#connection-establishment) | Rfc9293ConnectionEstablishment.test | PASS |
| [RFC9293-CKSUM-2](../../standard/rfc9293/catalog.md#rfc9293-cksum-2) | later (level 3) | — | — | — |
| [RFC9293-HDR-1](../../standard/rfc9293/catalog.md#rfc9293-hdr-1) | selected | [connection-establishment](../../protocol/tcp/checks.md#connection-establishment) | Rfc9293ConnectionEstablishment.test | PASS |
| [RFC9293-RST-1](../../standard/rfc9293/catalog.md#rfc9293-rst-1) | selected | [connection-reset](../../protocol/tcp/checks.md#connection-reset) | Rfc9293Reset.test | PASS |

22 entries: 17 reached a test and passed; 1 reached a test and failed as a model gap; 4 wait
for level 3, level 4, or another test category.

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
| [TCP-F-CHECKSUM](../../protocol/tcp/features.md#tcp-f-checksum) | RFC9293-CKSUM-1 PASS; CKSUM-2 did not run (level 3) | **partial** |
| [TCP-F-HEADER](../../protocol/tcp/features.md#tcp-f-header) | RFC9293-HDR-1 PASS | **supported** |
| [TCP-F-RESET](../../protocol/tcp/features.md#tcp-f-reset) | RFC9293-RST-1 PASS | **supported** |

Eight features supported, one partial. The one model gap of this pass sits on a supporting
statement, RFC9293-PSH-1, so by the rule it does not lower `TCP-F-DATA-TRANSFER`; it is a
MUST-level gap all the same, and [`conformance.md`](conformance.md#findings) carries it as
a finding. Bounds: every feature rests on one connection, one link, one data size per check,
and no loss; the flow-control feature rests on a fixed small window, not on a closing one,
because the model's default mode never closes it (see the results).

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

**Run** — every mandatory feature has a core check that ran and has a verdict: 9 of 9, all
core checks PASS.

| Level | State | Evidence |
| --- | --- | --- |
| 1, Survey | **reached** | The standards map pins RFC 9293; [`conformance.md`](conformance.md) maps the 15 claimed RFCs onto it and names every obsolete one. |
| 2, Core | **reached** | The inventory above; 17 of 17 level-2 core statements with a PASS, and one declared model gap on a supporting statement. |
| 3, Edge | not started | RFC9293-CKSUM-2, reset on a live connection, a shrunk window, ICMP handling; RFC 9293 already holds the text, no new document is needed. |
| 4, Dynamics | not started | RFC 6298 and RFC 5681 are not in the in-scope set; ZWP-1, ACKD-1 and TIME-WAIT wait for statistical checks. |
| 5, Complete | not started | urgent data, keep-alives, the options, PMTU. |

## Pass log

| Pass | Date | Level | Scope | Result |
| --- | --- | --- | --- | --- |
| 1 | 2026-09-08 | 2, partial | RFC 9293 in scope; 10 catalog entries; 4 features; 3 checks; 3 tests; flow control had no feature. On a branch since dropped; the tests were restored from it. | 5 PASS |
| 2 | 2026-09-08 | **2, reached** | 22 catalog entries (12 new), 9 features (5 new: data transfer, flow control, checksum, header, reset), 6 checks (3 new), 6 tests (3 new, 2 extended) | 8 tests: 7 PASS, 1 FAIL (expected) — the PSH bit; 8 features supported, 1 partial; see [`results.md`](results.md) and [`conformance.md`](conformance.md) |

## Out of scope

The catalog records what this pass left out: retransmission and its timer, congestion
control, a shrunk or zero window beyond the probe statement, the options other than MSS,
reset handling on a live connection, urgent data, keep-alives, the simultaneous cases, and
the TIME-WAIT duration. Each is level 3 to level 5 work, and the inventory above says
which.
