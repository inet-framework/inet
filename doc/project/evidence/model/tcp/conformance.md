# TCP — model claims and conformance matrix

> **Kind:** report · **Status:** snapshot 2026-09-10 · **Seal:** none · **Owns:** — · **Stands on:** [features.md](../../protocol/tcp/features.md), [coverage.md](coverage.md), [standards.md](../../protocol/tcp/standards.md)

Step 8 artifact of the standards test workflow. The tests tell what the model does. This
document adds what the model says it intends to do, and compares the two at the level of
features, never at the level of a single test.

- Claim scan: 2026-09-08, source identical to `master`; the scan of the earlier pass was
  repeated and gave the same result.
  The claim scan did not run again on 2026-09-10. Every source file that part 1 cites is
  identical to the file at the commit above, so the claims still hold.
- Support values: [`coverage.md`](coverage.md#feature-support), from the run of 2026-09-10
  on `master`, commit `0868c36c88`. Every verdict of that run repeats the verdict of the
  earlier pass.

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

Every document the claim names in the areas of this pass has since been replaced:

| Claimed | Replaced by | Since | In the model? |
| --- | --- | --- | --- |
| RFC 793, Transmission Control Protocol | **RFC 9293** | August 2022 | **no mention anywhere in `src/`** |
| RFC 2581, TCP Congestion Control | RFC 5681 | September 2009 | 41 mentions of the old one, 1 of the new |
| RFC 1323, TCP Extensions for High Performance | RFC 7323 | September 2014 | 27 mentions of the old one, 0 of the new |
| RFC 2988, Computing TCP's Retransmission Timer | RFC 6298 | June 2011 | 2 mentions of the old one, 0 of the new |
| RFC 3782, NewReno | RFC 6582 | April 2012 | 0 mentions of the new one |

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
| TCP-F-CHECKSUM | mandatory | yes, through RFC 793 | partial | **partial** — the receive half passes; the value the sender writes by default does not, finding 4 |
| TCP-F-HEADER | mandatory | yes, through RFC 793 | supported | **confirmed** |
| TCP-F-RESET | mandatory | yes, through RFC 793 | supported | **confirmed** |
| TCP-F-SEGMENT-ACCEPTANCE | mandatory | yes, through RFC 793 | supported | **confirmed** |
| TCP-F-RESET-VALIDATION | mandatory | yes, through RFC 793 | supported | **confirmed** |
| TCP-F-WINDOW-ROBUSTNESS | mandatory | yes, through RFC 793 | supported; supporting WND-5 failed | **confirmed**, with finding 5 |
| TCP-F-ICMP-HANDLING | mandatory | yes, through RFC 793 | partial | **partial** — a Source Quench stops the run, finding 6 |

Eleven features `confirmed`, two `partial`. No feature reaches `defect` by the rules of the
matrix. Three MUST-level statements are violated all the same, two of them on supporting
statements; findings 3, 4 and 6 say so rather than let the matrix's shape hide it.

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
[`results.md`](results.md#model-gap-1-pass-2-the-psh-bit-is-never-set)). `Rfc9293Push.test`
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

### 5. New data goes past a window edge that moved backward

RFC 9293 §3.8.6 asks two things of a sender whose peer shrinks the window: survive it
(MUST-34), and send no new data past the new edge (SHLD-15). The model does the first and not
the second. With the right edge at 26708 it sent a full segment starting at 27144.

The two are separate tests on purpose, so that the failure of the weaker requirement cannot
hide the verdict on the stronger one. See gap 3 of [`results.md`](results.md).

### 6. A Source Quench stops the simulation

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
