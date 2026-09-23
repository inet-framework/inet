# TCP — model claims and conformance matrix

> **Kind:** report · **Status:** snapshot 2026-09-23 · **Seal:** none · **Owns:** — · **Stands on:** [features.md](../../protocol/tcp/features.md), [coverage.md](coverage.md), [standards.md](../../protocol/tcp/standards.md)

Step 8 artifact of the standards test workflow. The tests tell what the model does. This
document adds what the model says it intends to do, and compares the two at the level of
features, never at the level of a single test.

The word `defect` means two things, and this document uses the first. In the matrix below it
is a **feature**: a whole mandatory feature that the model claims and does not support. In
[`results.md`](results.md) it is a **statement**: one behaviour that code exists for and gets
wrong. A statement-level defect usually sits inside a feature that otherwise works, which the
matrix then reads as `partial` or as `confirmed with a finding`. That is what happened to
TCP-F-CHECKSUM and TCP-F-WINDOW-ROBUSTNESS: each holds a statement-level defect, and neither
is a feature-level one.

- Claim scan: 2026-09-08, source identical to `master`; the scan of the earlier pass was
  repeated and gave the same result.
  The claim scan did not run again on 2026-09-10. Every source file that part 1 cites is
  identical to the file at the commit above, so the claims still hold.
- Support values: [`coverage.md`](coverage.md#feature-support), from the run of 2026-09-23:
  - Date: 2026-09-23 18:26 +0200
  - INET: branch `topic/standards-tests-wave0`, commit `28536bd0a5` (on `master`), tree clean
  - Trees: src `16dc528e10`, tests/protocol `6f0a6bdb05`
  - OMNeT++: 6.4.0, commit `cf58891643`
  - Build: debug, built from this commit
  - Compiler: Ubuntu clang version 23.0.0
  - Platform: Ubuntu 26.04.1 LTS, Linux 7.0.0-34-generic x86_64
  - Command: `inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/tcp$'`
- Claim scan for the level 4 features: 2026-09-14, corrected on 2026-09-23. The list of
  standards in `Tcp.ned:94-108` names RFC 793 and RFC 2581, the predecessors of RFC 9293 and
  RFC 5681; RFC 2988, the predecessor of RFC 6298, appears in the flavour sources
  (`TcpReno.cc:284`). `Tcp.ned` names neither RFC 5681 nor RFC 6298. `TcpBaseAlg::established` quotes
  the paragraph of RFC 5681 that governs the initial window after a lost SYN, without its
  number (`TcpBaseAlg.cc:138-140`); `TcpBaseAlg::sendData` cites "RFC 5681, page 11" for the
  restart window after an idle period (`TcpBaseAlg.cc:400`); and since `f3064a83d0` two comments of `TcpBaseAlg.cc` cite RFC 6298
  §2.2 and §2.3 (`TcpBaseAlg.cc:335, 344`). The two control loops are claimed through the
  older documents and in code. The earlier text of this item said that `Tcp.ned` names RFC 9293
  and RFC 5681; it does not.

This is the one document of the workflow whose first part reads the model documentation on
purpose. The claims must not travel back into the catalog, the feature map, or the check
descriptions.

## Part 1 — what the model claims

### The module-level claim

Unlike the IPv4 and UDP models, the TCP model states its standards openly. `Tcp.ned`
carries a **Standards** heading with an explicit list:

> "Implementation is based on the following RFCs:
>   - RFC  793 - Transmission Control Protocol
>   - RFC  896 - Congestion Control in IP/TCP Internetworks
>   - RFC 1122 - Requirements for Internet Hosts -- Communication Layers
>   - RFC 1323 - TCP Extensions for High Performance
>   - RFC 2018 - TCP Selective Acknowledgment Options
>   - RFC 2581 - TCP Congestion Control
>   - RFC 2883 - An Extension to the Selective Acknowledgement (SACK) Option for TCP
>   - RFC 3042 - Enhancing TCP's Loss Recovery Using Limited Transmit
>   - RFC 3390 - Increasing TCP's Initial Window
>   - RFC 3517 - A Conservative Selective Acknowledgment (SACK)-based Loss Recovery Algorithm for TCP
>   - RFC 3782 - The `NewReno` Modification to TCP's Fast Recovery Algorithm
>   - RFC 1191 - Path MTU Discovery
>   - RFC 1981 - Path MTU Discovery for IP version 6
>   - RFC 3168 - The Addition of Explicit Congestion Notification (ECN) to IP
>   - RFC 8257 - Data Center TCP (DCTCP): ECN Marking at the Data Sender"
> — [Tcp.ned:93-109](../../../../../src/inet/transportlayer/tcp/Tcp.ned#L93-L109)

The claim is then made specific for the areas this pass checks:

> "Implemented features include the following:
>  - all RFC 793 TCP states and state transitions
>  - connection setup and teardown as in RFC 793
>  - generally, RFC 793-compliant segment processing"
> — [Tcp.ned:111-114](../../../../../src/inet/transportlayer/tcp/Tcp.ned#L111-L114)

The C++ repeats it ([TcpConnection.h:50-52](../../../../../src/inet/transportlayer/tcp/TcpConnection.h#L50-L52):
"The implementation largely follows the functional specification at the end of RFC 793"),
and the segment-processing code is organised by the numbered steps of the RFC
([TcpConnectionRcvSegment.cc:89](../../../../../src/inet/transportlayer/tcp/TcpConnectionRcvSegment.cc#L89)).

### The claimed set is one standards generation behind

Every document the claim names in the areas of this pass has since been replaced. The
counts below come from a sweep of `src/inet/transportlayer/tcp/` and
`src/inet/transportlayer/tcp_common/` on 2026-09-11. Each replacement is taken from the
header of the replacing text, not from memory: RFC 7323 says `Obsoletes: 1323`, RFC 5681
says `Obsoletes: 2581`, RFC 2581 says `Obsoletes: 2001`, RFC 6582 says `Obsoletes: 3782`,
RFC 6298 says `Obsoletes: 2988`, RFC 6675 says `Obsoletes: 3517`, and RFC 8201 says
`Obsoletes: 1981`.

| Claimed | Replaced by | Since | Old citations | New citations |
| --- | --- | --- | --- | --- |
| RFC 793, Transmission Control Protocol | **RFC 9293** | August 2022 | 38 | **0** |
| RFC 3517, SACK-based loss recovery | **RFC 6675** | August 2012 | **52** | **0** |
| RFC 2581, TCP Congestion Control | **RFC 5681** | September 2009 | 48 | 1 |
| RFC 1323, TCP Extensions for High Performance | **RFC 7323** | September 2014 | 29 | **0** |
| RFC 3782, NewReno fast recovery | **RFC 6582** | April 2012 | 17 | **0** |
| RFC 1981, Path MTU Discovery for IPv6 | **RFC 8201** | July 2017 | 9 | **0** |
| RFC 2988, Computing TCP's Retransmission Timer | **RFC 6298** | June 2011 | 8 | **0** |
| RFC 2001, TCP Slow Start and congestion avoidance | **RFC 5681**, through RFC 2581 | September 2009 | 5 | 1 |

Two of the rows matter more than the others. RFC 3517 is the most cited document in the
whole TCP tree, and its replacement appears nowhere. RFC 2001 is two generations old: it
was replaced in 1999 and again in 2009.

This is a claim finding, not a defect. The code may well follow the current text; nothing
here says it does not. It says a reader of the model cannot tell which text the code
answers to, and that a test author who follows the citation reads a document that no longer
governs.

### How this pass reads the claim

RFC 9293 collects RFC 793 and its updates into one text; for the nine features of this pass
the two documents agree on substance. A claim on RFC 793 therefore covers these features,
and the matrix marks them `claimed (through RFC 793)`. The limit of that reading is finding
2.

## Part 2 — conformance matrix

The verdict combines the claim on the governing source document, the support value of the
ledger, and the level of the feature, by the table of step 8.

| Feature | Level | Claimed | Support | Verdict |
| --- | --- | --- | --- | --- |
| TCP-F-ESTABLISH | mandatory | yes, through RFC 793 | supported | **confirmed** |
| TCP-F-SEQUENCE | mandatory | yes, through RFC 793 | supported | **confirmed** |
| TCP-F-ACKNOWLEDGE | mandatory | yes, through RFC 793 | supported | **confirmed** |
| TCP-F-TERMINATE | mandatory | yes, through RFC 793 | supported | **confirmed** |
| TCP-F-DATA-TRANSFER | mandatory | yes, through RFC 793 | supported; supporting PSH-1 failed | **confirmed**, with finding 3 |
| TCP-F-FLOW-CONTROL | mandatory | yes, through RFC 793 | supported | **confirmed** |
| TCP-F-CHECKSUM | mandatory | yes, through RFC 793 | partial | **partial** — the receive half passes; the value the sender writes by default does not, finding 4, a statement-level defect whose repair is blocked |
| TCP-F-HEADER | mandatory | yes, through RFC 793 | supported | **confirmed** |
| TCP-F-RESET | mandatory | yes, through RFC 793 | supported | **confirmed** |
| TCP-F-SEGMENT-ACCEPTANCE | mandatory | yes, through RFC 793 | supported | **confirmed** |
| TCP-F-RESET-VALIDATION | mandatory | yes, through RFC 793 | supported | **confirmed** |
| TCP-F-WINDOW-ROBUSTNESS | mandatory | yes, through RFC 793 | supported | **confirmed**; finding 5 was withdrawn, gap 3 was a test error |
| TCP-F-ICMP-HANDLING | mandatory | yes, through RFC 793 | supported | **confirmed** — finding 6 was repaired by `40c9f04e21` |
| TCP-F-RTO-ESTIMATOR | mandatory | yes, through RFC 2988, and RFC 6298 in two comments | partial | **partial** — the first measurement passes since `f3064a83d0` repaired gap 5; the update of a later measurement, RFC6298-UPD-1, has no check yet |
| TCP-F-RTO-BOUNDS | optional | yes, through RFC 2988 | supported | **confirmed** — one second before any measurement, and a ceiling of 240 seconds |
| TCP-F-RTO-BACKOFF | mandatory | yes, through RFC 2988 | partial | **partial** — the timeout doubles at every expiry on the wire; RFC6298-EXP-1 has no check yet |
| TCP-F-RTT-SAMPLING | mandatory | yes, through RFC 2988 | supported | **confirmed** — no sample is taken from a retransmitted segment |
| TCP-F-CONGESTION-WINDOW | mandatory | yes, through RFC 2581 | partial | **partial** — slow start stays inside one segment per acknowledgment; congestion avoidance, RFC5681-USE-1 and CA-2, has no check yet |
| TCP-F-INITIAL-WINDOW | mandatory | yes, through RFC 2581, and RFC 5681 in one quote | supported | **confirmed** — inside the table, and one segment after a lost SYN |
| TCP-F-LOSS-RESPONSE | mandatory | yes, through RFC 2581 | supported | **confirmed** — the window falls to one segment and the threshold to half the flight |
| TCP-F-FAST-RETRANSMIT | mandatory | yes, through RFC 2581 and RFC 3782 | partial | **partial** — three duplicates repair the loss without the timer; the deflation, RFC5681-FR-5, is not asserted |
| TCP-F-RESTART-IDLE | optional | yes, RFC 5681 in one comment | untested | **unverified** — the check is owed |
| TCP-F-DELAYED-ACK | mandatory | yes, through RFC 1122 and RFC 2581 | partial | **partial** — an out-of-order segment is acknowledged at once; the 500 ms bound waits for a statistical check |

Twenty-three features: sixteen `confirmed`, six `partial`, one `unverified`. No feature
reaches `defect` by the rules of the matrix. The level 4 column is corrected on 2026-09-23 to
the level that [`features.md`](../../protocol/tcp/features.md) gives: TCP-F-RESTART-IDLE is
optional and TCP-F-DELAYED-ACK mandatory, where the earlier matrix had them the other way.
Two MUST-level statements are violated all the same, on supporting statements; findings 3 and
4 say so rather than let the matrix's shape hide it. Five of the six `partial` verdicts come
from core statements without a check, not from a failure; see the owed rows of
[`coverage.md`](coverage.md#statement-coverage).

Pass 3 added four features and changed one verdict. The four new ones describe the same
connection under attack or under a fault, and three of them are `confirmed` on every core
check. That is the headline of this pass: the rules that keep a connection alive when a third
party crafts a segment, or when a peer misbehaves, hold in this model.

## Findings

### 1. The claim is explicit, and the checks agree with it

Every feature of this pass is confirmed or partial, and the confirmation means something
here that it could not mean for IPv4 or UDP: the model named its documents, and the checks
agree with those documents on the normal path. `Tcp.ned` remains the model for how a
module should state its standards.

### 2. The claimed documents are obsolete

The model claims RFC 793, which RFC 9293 replaced in August 2022, and cites the obsolete
congestion, timer and extension documents throughout. For the nine features of this pass
that costs nothing, because the texts agree. It costs something where they do not, and
those places are exactly the next levels: RFC 5681 changed the congestion rules, RFC 6298
changed the retransmission timer, RFC 7323 changed the timestamp and window scale rules. A
reader cannot tell which text the model follows. The repair is a documentation change.

### 3. The PSH bit is never set — a MUST the model does not meet

RFC 9293 §3.9.1.2 binds a sender whose SEND call offers no PUSH flag: it MUST set PSH on the
last buffered segment (MUST-61). The model's send interface offers no PUSH flag, and the
model never sets the bit; its own code marks the place `TODO when to set PSH bit?` and the
SEND processing `FIXME how to support PUSH?` (the exact references are in
[`results.md`](results.md#gap-1-pass-2-the-psh-bit-is-never-set--unimplemented-feature)). `Rfc9293Push.test`
keeps the faithful assertion and declares its failure.

The matrix does not show this as a `defect`, because the push rule is a supporting
statement of `TCP-F-DATA-TRANSFER`, whose core checks — the whole stream acknowledged, no
segment above the MSS — passed. That is the rule of step 7 working as designed: the byte
stream does arrive. It is still a MUST that a real peer would notice on the wire, and it
belongs on the list of the model's owner. The same comments mark urgent data (MUST-30 to
MUST-32) unsupported; that one is level 5.

### 4. The checksum is never optional, and by default the model writes none

RFC 9293 §3.1 leaves no room: "The TCP checksum is never optional. The sender MUST generate
it (MUST-2)." The default `checksumMode` of the ~Tcp module is `"declared"`, in which the
field stays at zero and a flag asserts it correct. A run with the defaults therefore carries
segments with no checksum, and two INET hosts accept each other only because both read the
same flag.

The receive half of the same feature is now checked and passes: a segment whose checksum is
wrong is discarded in silence and the stream still arrives. So the model implements the
mechanism, and the finding is about which mode a user gets without asking. The UDP pass found
the same default one layer down; the two share a correction. See gap 2 of
[`results.md`](results.md).

### 5. New data goes past a window edge that moved backward — withdrawn

**Withdrawn.** Gap 3 was a test error, not a defect, and `Rfc9293ShrunkWindowNoNewData.test`
passes. The text below is the finding as the pass recorded it.

RFC 9293 §3.8.6 asks two things of a sender whose peer shrinks the window: survive it
(MUST-34), and send no new data past the new edge (SHLD-15). The model does the first and not
the second. With the right edge at 26708 it sent a full segment starting at 27144.

The two are separate tests on purpose, so that the failure of the weaker requirement cannot
hide the verdict on the stronger one. See gap 3 of [`results.md`](results.md).

### 6. A Source Quench stops the simulation — repaired by `40c9f04e21`

**Repaired.** The ICMP module discards an unknown type now, and `Rfc9293SourceQuench.test`
passes. The text below is the finding as the pass recorded it.

RFC 9293 §3.9.2.2 requires a TCP implementation to silently discard a received Source Quench
(MUST-55). The model never offers the message to TCP: the ICMP module throws on an unknown
type, and type 4 is unknown to it. The run stops.

The rule exists because the message still arrives at hosts even though RFC 6633 deprecated it
for routers. The same branch is what the IPv4 suite reaches with a type that names no message
at all, so one correction in the ICMP module closes both findings. See gap 4 of
[`results.md`](results.md).

### 7. Flow control exists, and the default mode never uses it

`Tcp.ned:53-58` says that in the default "autoread" mode the advertised window never
decreases, "so there is effectively no flow control". The flow-control check confirms the
sender's half — it respects a small fixed window exactly — and cannot confirm the
receiver's half, a window that closes, without the "explicit-read" mode the same note
names. Not a defect; a boundary on what "confirmed" means for this feature, and the first
flow-control check of the next pass.

### 8. Nothing else is contradicted at level 2, and little at level 3

Within nine features and seventeen passing statements, on one topology and without loss, the
model does what RFC 9293 describes on the normal path. Pass 1 stopped at level 2 partial with
flow control undeclared; pass 2 declares it and passes it.

Pass 3 took the same model off the normal path, with a corrupt segment, a segment outside the
window, two crafted resets, a window that moved backward and two ICMP reports. Twenty-seven
statements now carry a PASS and four a declared FAIL. Of the three gaps this pass found, one
is a choice of default, one is a `should not` beside a `must` that holds, and one is a
missing case in a module of another layer. None of them is a failure of the connection logic
itself, which is what these eleven checks were built to attack.

## What this document does not establish

- It says nothing about the documents outside the in-scope set. The model claims 15 RFCs;
  this pass checks against one.
- It says nothing about the areas the catalog still puts out of scope: retransmission,
  congestion control, the options, urgent data, silly window avoidance, the simultaneous
  cases, and the TIME-WAIT duration. Reset handling on a live connection left that list at
  level 3 and is checked now.
- A `confirmed` verdict means the checks of the feature passed. It does not mean the
  feature is complete.
