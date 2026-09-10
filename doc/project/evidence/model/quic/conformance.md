# QUIC — model claims and conformance matrix

> **Kind:** report · **Status:** snapshot 2026-09-10 · **Seal:** none · **Owns:** — · **Stands on:** [features.md](../../protocol/quic/features.md), [coverage.md](coverage.md), [standards.md](../../protocol/quic/standards.md)

Step 8 artifact of the standards test workflow. The tests tell what the model does. This
document adds what the model says it intends to do, and compares the two at the level of
features, never at the level of a single test.

- Claim scan: 2026-09-08, source identical to `master`.
  The claim scan did not run again on 2026-09-10. Every source file that part 1 cites is
  identical to the file at the commit above, so the claims still hold.
- Support values: [`coverage.md`](coverage.md#feature-support), from the run of 2026-09-10
  on `master`, commit `0868c36c88`. Every verdict of that run repeats the verdict of the
  earlier pass.

This is the one document of the workflow whose first part reads the model documentation on
purpose. The claims must not travel back into the catalog, the feature map, or the check
descriptions.

## Part 1 — what the model claims

### The claim, and the deviations declared with it

The QUIC model states its standards and, unusually, states what it does not implement. The
claim:

> "Implementation is based on the following RFCs:
>  - RFC 5681 - TCP Congestion Control
>  - RFC 8899 - Packetization Layer Path MTU Discovery for Datagram Transports
>  - RFC 9000 - QUIC: A UDP-Based Multiplexed and Secure Transport
>  - RFC 9002 - QUIC Loss Detection and Congestion Control"
> — [Quic.ned:135-139](../../../../../src/inet/transportlayer/quic/Quic.ned#L135-L139)

The heading above it is **"Standards and Deviations"**
([Quic.ned:133](../../../../../src/inet/transportlayer/quic/Quic.ned#L133)), and the section
that follows is what makes this model different from the other three in this tree:

> "Missing bits:" — [Quic.ned:141](../../../../../src/inet/transportlayer/quic/Quic.ned#L141)

Twelve deviations follow, each naming what the standard requires and what the module does
instead: no encryption (RFC 9001), no pacing (RFC 9002), no ECN, no connection migration, no
path validation, no amplification-attack mitigation, no client address validation and no
Retry packet, no stream types, one flow-control parameter where the standard has three, no
stream termination, a fixed connection identifier 0, and no version negotiation. The module
also cites the paper that describes it.

Two of the twelve land directly on the catalog of this pass:

> "Stream types: QUIC distinguish streams depending on who opened the stream (client or
> server) and whether the stream is unidirectional or bidirectional. This module does not
> distinguish that. All streams can be used bidirectional."
> — [Quic.ned:163-166](../../../../../src/inet/transportlayer/quic/Quic.ned#L163-L166)

> "Exchange of Connection IDs: ... This module always uses the fixed connection ID 0."
> — [Quic.ned:175-178](../../../../../src/inet/transportlayer/quic/Quic.ned#L175-L178)

RFC 8999, RFC 9114 and RFC 9369 appear nowhere in the model.

### How this pass reads the claim

The claim is explicit and current: RFC 9000 is named by number, it is the governing
document, and it has never been updated. There is no generation gap here, unlike TCP, whose
model claims a document that RFC 9293 replaced in 2022. The claim on RFC 9000 is bounded by
the twelve declared deviations, and the matrix reads it that way: a feature the deviations
exclude is `claimed, with a declared exception`, not simply `claimed`.

## Part 2 — conformance matrix

The verdict combines the claim on the governing source document, the support value of the
ledger, and the level of the feature, by the table of step 8.

| Feature | Level | Claimed | Support | Verdict |
| --- | --- | --- | --- | --- |
| QUIC-F-PACKET | mandatory | yes, RFC 9000 by number; the connection identifier is a declared exception | supported | **confirmed**, with the identifier degenerate |
| QUIC-F-INITIAL-SIZE | mandatory | yes, and no deviation is declared for it | partial | **defect** for the server half, finding 3; **confirmed** for the client half |
| QUIC-F-STREAMS | mandatory | the ordered stream, yes; the identifier encoding is a **declared exception** | partial | **declined** for the encoding, **confirmed** for the rest, including its buffering half |
| QUIC-F-FLOW-CONTROL | mandatory | yes, with a declared exception for the three stream-type parameters | supported | **confirmed** for the stream limit |
| QUIC-F-ACKNOWLEDGE | mandatory | yes | supported | **confirmed** |
| QUIC-F-CLOSE | mandatory | yes | supported | **confirmed** |
| QUIC-F-VERSION-NEGOTIATION | mandatory | yes, RFC 9000 by number, and no deviation is declared for it | not supported | **defect** — finding 4 |
| QUIC-F-ADDRESS-VALIDATION | mandatory | the amplification limit is a **declared exception** ("Missing bits") | supported | **undocumented** — the model does more than it claims, finding 5 |
| QUIC-F-FRAME-VALIDATION | mandatory | yes, and no deviation is declared for it | not supported | **defect** — finding 6 |

Five confirmed, one split between a declined half and a confirmed half, three `defect`, one
`undocumented`.

Pass 1 recorded no `defect` and named one candidate for the next pass: the server's Initial
datagram. That candidate is now a `defect`, because a check covers it. Two more joined it,
and one feature turned out better than the model claims.

A `defect` is the matrix's word for a feature the model claims and does not do. It is a
strong word, and it is the right one here: RFC 9000 is claimed by number, none of these three
requirements is in the model's own list of missing bits, and each has a check that fails.

## Findings

### 1. The best-documented model in this tree

`Quic.ned` names its standards by number and then lists twelve things it does not do, each
one naming the standard it departs from. Nothing in the IPv4, UDP or TCP models comes near
this. It changes what the conformance step can say: a deviation that the model declares is
a `declined`, not a `defect`, because the model never claimed the behavior. The value of
the workflow against such a model is not in catching it out — it is in confirming that the
declared list is complete.

It is not quite complete. Finding 3.

### 2. The stream identifier encoding: declared, and therefore declined

RFC9000-STR-1 is mandatory, and the model does not implement it: an application's chosen
stream number reaches the wire unchanged. The model says so plainly. The verdict is
therefore `declined` and not `defect` — the reading the matrix was given for exactly this
case.

Two things follow that the verdict alone does not carry. First, the passing check
establishes nothing here, and the ledger says so; the decisive check was attempted and
withdrawn, so this is a coverage gap as well as a declared deviation. Second, a `declined`
mandatory requirement is still a limit on what the model can be used for: a study of stream
multiplexing, of unidirectional streams, or of anything where the peer's view of who may
send on a stream matters, is outside what this model represents.

### 3. The first undeclared deviation: the server's Initial datagram

RFC 9000 §14.1 requires **both** endpoints to expand datagrams carrying Initial packets to
1200 octets — the client always, the server whenever the datagram is ack-eliciting. The
client side is implemented at
[PacketBuilder.cc:476](../../../../../src/inet/transportlayer/quic/PacketBuilder.cc#L476).
The server side is not: `buildServerInitialPacket` has no padding.

The "Missing bits" list does not name it. It names amplification-attack mitigation, which is
the neighbouring requirement of §8 and a different rule. Pass 1 called this the single
undeclared gap in an otherwise exemplary declaration. Pass 2 found two more, in findings 4 and
6, so the sentence needs correcting: there are three, and this is the smallest of them.

**Pass 2 wrote that check.** `Rfc9000ServerInitialSize.test` finds the server sending an
Initial packet with a CRYPTO frame in a datagram of 27 octets. The verdict for the feature is
now `defect` for the server half: a requirement the model claims, does not declare an
exception to, and does not meet.

### 4. Version negotiation is absent, and it is not declared

RFC 9000 §6.1 gives a server one answer to a version it does not accept. The model gives
none: it ignores the version field, and a client claiming 0x0A0A0A0A completes a handshake and
transfers data. The "Missing bits" list does not name version negotiation.

This is the second undeclared departure, and it is larger than the first. The version field is
what lets QUIC change versions at all, and a deployment of this model would tell a future peer
nothing about what it speaks. See gap 2 of [`results.md`](results.md).

### 5. The anti-amplification limit holds, and the model says it does not

The "Missing bits" list names amplification-attack mitigation among the things the model does
not implement. The check finds the bound respected: the server never sends more than three
times what it received before the address is validated.

The matrix calls that `undocumented` — the model does more than it claims. The reason is worth
knowing, because it is luck rather than design: the client pads its Initial datagram to 1200
octets, which gives the server a 3600-octet budget from the first packet, and the server's
whole handshake is a hundred octets. A model that implemented the server's own 1200-octet
padding (finding 3) would spend more of that budget, and a model with a smaller client floor
would have less of it. The bound is not enforced anywhere; it simply is not reached.

### 6. An unknown frame type stops the run, and it is not declared

RFC 9000 §12.4 requires an endpoint to treat a frame of unknown type as a connection error of
type FRAME_ENCODING_ERROR. The model throws instead, from the default branch of the frame
switch in `ConnectionState.cc`. The "Missing bits" list does not name it.

This is the third undeclared departure, and the one with the widest reach: frame types come
from a registry that grows, so any peer of a later version can stop this endpoint. See gap 3
of [`results.md`](results.md).

### 7. A documentation gap outside the module

The showcase at `showcases/quic/linksharing/doc/index.rst` describes QUIC's TLS 1.3
encryption and its connection migration as properties of QUIC, without saying that this
model implements neither. The module's own documentation is careful; the showcase does not
inherit that care, and a reader of the showcase alone would be misled. A documentation task
for the model, not a test task.

## What this document does not establish

- It says nothing about RFC 9001 or RFC 9002, which supply the handshake and the recovery
  that RFC 9000 defers, and which the model claims in part.
- It says nothing about the areas the catalog puts out of scope: errors, resets, version
  negotiation, migration, and every frame type beyond the four this pass observes.
- A `confirmed` verdict means the checks of the feature passed. For QUIC it means less than
  it did for the earlier protocols, because the framework cannot dissect QUIC packets and
  every check reads chunk types instead; [`results.md`](results.md) records that.
