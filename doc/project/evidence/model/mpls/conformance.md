# MPLS — model claims and conformance

> **Kind:** report · **Status:** snapshot 2026-09-24 · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/mpls/standards.md), [features.md](../../protocol/mpls/features.md), [coverage.md](coverage.md)

Step 8 artifact of the standards test workflow, in two parts. Part 1 records what the model
intends: which standards it claims to implement, mapped onto the standards map; the level 1 pass
wrote it, and the level 2 pass adds what its run found for the facts that part 1 named. Part 2,
the conformance matrix, combines the claims with the feature support of the level 2 run.

Read record of part 1 — no build, no run:

- Date: 2026-09-23
- INET: branch `topic/standards-tests-wave0`, commit `e360ca980e`, tree clean
- Trees: src `16dc528e10`, tests/protocol `6f0a6bdb05`

The src tree of the level 2 run, `5c4f41c600`, is a later one; the MPLS, RSVP-TE and PPP sources
and the MPLS chapter of the User's Guide are the same in it.

## Part 1 — the claims

Every place in the model that names a standard of the MPLS family, found with
`grep -rn -E 'RFC ?[0-9]{3,4}|draft-' src/inet/networklayer/mpls --include=*.ned --include=*.cc --include=*.h --include=*.msg`
and the same search over `doc/src/users-guide/ch-mpls.rst`:

| Claim | Where | What it says |
| --- | --- | --- |
| none | [`Mpls.ned:14`](../../../../../src/inet/networklayer/mpls/Mpls.ned) | "Implements the MPLS protocol." The documentation comment of the module, which the module reference publishes, names no document. |
| none | [`Mpls.h:27-28`](../../../../../src/inet/networklayer/mpls/Mpls.h) | "Implements the MPLS protocol; see the NED file for more info." Repeats the NED comment, no document. |
| none | [`LibTable.ned:12-13`](../../../../../src/inet/networklayer/mpls/LibTable.ned) | "Stores the LIB (Label Information Base), accessed by ~Mpls and its associated control protocols (~RsvpTe, ~Ldp)..." Names the two signaling protocols by acronym, no document for either. |
| none | [`IIngressClassifier.ned:9`](../../../../../src/inet/networklayer/mpls/IIngressClassifier.ned) | "Module interface for ingress packet classifiers for MPLS." |
| none | [`MplsPacket.msg`](../../../../../src/inet/networklayer/mpls/MplsPacket.msg) | The `MplsHeader` class carries no comment above it and no field comment names a document (see the note below on the `tc` field). |
| none | [`ch-mpls.rst`](../../../../../doc/src/users-guide/ch-mpls.rst) | The user's guide chapter on MPLS, `Ldp`, `RsvpTe`, `RsvpClassifier`, `LinkStateRouting`, `Ted` and the preassembled router models — a full chapter, and no RFC number appears in it. |

The whole `src/inet/networklayer/mpls/` directory — five `.ned` files, four `.h` files, five
`.cc` files, one `.msg` file — has no hit for the pattern above. The one non-empty match in
the wider MPLS family is a disclaimer inside `RsvpPacket.msg`, which names RFC 2205 and RFC
3209 to say that the RSVP-TE model's wire format is not faithful to them; that file belongs
to the RSVP-TE protocol, out of scope of this document.

One field comment is worth recording although it names no RFC number.
[`MplsPacket.msg:17`](../../../../../src/inet/networklayer/mpls/MplsPacket.msg) reads:
"Traffic Class field for QoS (quality of service) priority and ECN (Explicit Congestion
Notification). Prior to 2009 this field was called EXP." RFC 5462, which renames the field
from "EXP" to "Traffic Class", was published in February 2009. The comment states the fact
that document establishes, and the header field is named `tc`, not `exp` — the model tracks
the 2009 rename in substance, in the one field where the family has a post-base-document
update, and still never cites the document by number.

Mapped onto the standards map, [`standards.md`](../../protocol/mpls/standards.md#document-list):

| Document | In the in-scope set | Claimed | Note |
| --- | --- | --- | --- |
| RFC 3031 | yes, `base` | **no** | Nothing in the model names it. |
| RFC 3032 | yes, `base` | **no**, by number; **yes**, in substance for one field | The label stack entry field width and order (label 20 bits, `tc` 3 bits, `s` 1 bit, `ttl` 8 bits, `MplsPacket.msg:16-19`) match RFC 3032 §2.1 exactly, and label value 1 is documented as the router alert label (`MplsPacket.msg:16`), which is RFC 3032's own reserved value. No comment cites the document. |
| RFC 3443 | yes, `updates` | no | Nothing names the Pipe or Short Pipe Model, and see the TTL fact below. |
| RFC 5462 | yes, `updates` | no, by number; yes, in the field name and the one comment above | — |

**The finding of the survey.** The model implements the MPLS protocol and never names either
base document, anywhere: not the module reference, not the header comment, not the packet
definition, not the user's guide chapter. This is a starker case than the other protocols of
this pass, which cite a document and get its version wrong; MPLS cites no document at all,
correct or not. The one field comment that mentions a date (`MplsPacket.msg:17`, "Prior to
2009") shows the claim gap is not a sign of an old or careless model: the code was kept
current with a document it does not name.

For the level 2 pass, facts from the code (the guide permits this look to decide the claim
question, and the pass must check each one):

1. **The `ttl` field of `MplsHeader` is never computed.** `MplsHeader.ttl` defaults to 0
   ([`MplsPacket_m.h:65`](../../../../../src/inet/networklayer/mpls/MplsPacket_m.h), generated
   from the `@bit(8)` field of [`MplsPacket.msg:19`](../../../../../src/inet/networklayer/mpls/MplsPacket.msg)).
   `Mpls::pushLabel` and `Mpls::swapLabel`
   ([`Mpls.cc:139-153`](../../../../../src/inet/networklayer/mpls/Mpls.cc)) construct a new
   `MplsHeader` and set its label and its `s` flag, and never call `setTtl`. At the ingress,
   `Mpls::tryLabelAndForwardIpv4Datagram`
   ([`Mpls.cc:96-99`](../../../../../src/inet/networklayer/mpls/Mpls.cc)) reads the IPv4
   header only to mark it `(void)ipv4Header; // unused variable` (`Mpls.cc:99`) — the datagram's
   Time-to-Live is never read. Every labeled packet the model produces therefore carries
   `ttl = 0` on the wire (`MplsPacketSerializer.cc:23` writes the field as given). RFC 3032
   §2.4 (`rfc3032.txt:460-461`) states the outgoing TTL rule with a MUST. The `ttl` field
   exists, is serialized, and is printed (`MplsPacket.cc:15`); by the guide's rule, a field
   that is present and carries a placeholder is a claim, so a check of the TTL rule is a
   **defect**, not an unimplemented feature.
2. **No document claim exists to test against for RFC 3031 or RFC 3032**, so the level 2 pass
   states the base-document claim itself as a documentation gap, the same way it states a
   feature gap: the module reference of `Mpls` names no standard for a protocol whose wire
   format the model otherwise follows.
3. **LDP and RSVP-TE are out of scope of this document** (see
   [`standards.md`](../../protocol/mpls/standards.md#in-scope-set)), but `LibTable.ned:13`
   and the user's guide name them as `Mpls`'s signaling partners; a later pass on either
   protocol reuses this fact, not this document.

### What the level 2 run found for these facts

1. **The TTL field is never computed**: confirmed, and repaired on 2026-09-25 with gaps 2 and 3.
   Every entry that the model sent had TTL 0 (`Rfc3032FirstLabelTtl`), and no LSR changed it; a
   defect, as fact 1 predicted
   ([gap 1](results.md#gap-1-defect--the-ttl-field-of-every-label-stack-entry-is-0)). The run found
   two more TTL defects behind it: the pop does not write the IPv4 TTL, and neither end of an LSP
   counts as a hop ([gap 2](results.md#gap-2-defect--the-lsp-does-not-count-its-hops-in-the-ipv4-ttl));
   and no LSR discards a packet whose TTL reaches zero
   ([gap 3](results.md#gap-3-defect--a-labeled-packet-whose-ttl-reaches-zero-goes-on)).
2. **No document is named**: unchanged. Part 2 reads the name "the MPLS protocol" as a claim of
   the base documents. The documentation of `Mpls` also names Frame Relay and ATM as link layers
   (`Mpls.ned:39-40`), which INET does not have, and Ethernet, on which an MPLS router stopped the
   run until the repair of 2026-09-25
   ([gap 4](results.md#gap-4-defect--an-mpls-router-on-an-ethernet-link-stops-the-run)).
3. **LDP and RSVP-TE stay out of scope.** The tests bind the labels by hand in the LIB and in the
   classifier of `RsvpMplsRouter`, with the RSVP hello timers off; no signaling runs.

## Part 2 — the conformance matrix

Run record of the verdicts behind the support values:

- Date: 2026-09-29 18:34 +0200
- INET: branch `topic/standards-tests-mpls-level2-fixes`, commit `db2fd89622`, tree clean
- Trees: src `8006b0e093`, tests/protocol `31f57dca8a`
- OMNeT++: 6.4.0, debug build from this commit, Ubuntu clang 23.0.0, Ubuntu 26.04.1 LTS
- Command: `inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/mpls$'`

A feature is `claimed` when the claims of part 1 cover its governing source document. The model
names no document, but it claims "the MPLS protocol" (`Mpls.ned:14`), the label operations and
the link layers of its documentation, and the fields of `MplsHeader`. This matrix reads that as a
claim of the two base documents, RFC 3031 and RFC 3032, which define the protocol that the model
names, and of the field rename of RFC 5462, which the field comment states. It does not read it
as a claim of RFC 3443, whose Pipe and Short Pipe Models are a choice that the model never names;
the Uniform Model of that document is the TTL rule of RFC 3032 §2.4, and belongs to MPLS-F-TTL.
The support comes from [`coverage.md`](coverage.md#feature-support).

| Feature | Level | Claimed | Support | Verdict |
| --- | --- | --- | --- | --- |
| [MPLS-F-LABEL-STACK-ENCODING](../../protocol/mpls/features.md#mpls-f-label-stack-encoding) | mandatory | yes | supported | `confirmed` |
| [MPLS-F-RESERVED-LABELS](../../protocol/mpls/features.md#mpls-f-reserved-labels) | mandatory | yes | partial | `partial` — every core statement that has a check passes; the IPv6 Explicit NULL label is not claimed, and the reserved values 4 to 15 belong to label distribution |
| [MPLS-F-LABEL-FORWARDING](../../protocol/mpls/features.md#mpls-f-label-forwarding) | mandatory | yes | supported | `confirmed` |
| [MPLS-F-INGRESS-LABELING](../../protocol/mpls/features.md#mpls-f-ingress-labeling) | mandatory | yes | supported | `confirmed` |
| [MPLS-F-EGRESS-DECAPSULATION](../../protocol/mpls/features.md#mpls-f-egress-decapsulation) | mandatory | yes | supported | `confirmed` |
| [MPLS-F-PENULTIMATE-HOP-POPPING](../../protocol/mpls/features.md#mpls-f-penultimate-hop-popping) | mandatory | yes | supported | `confirmed` |
| [MPLS-F-TTL](../../protocol/mpls/features.md#mpls-f-ttl) | mandatory | yes | supported | `confirmed` |
| [MPLS-F-PIPE-MODELS](../../protocol/mpls/features.md#mpls-f-pipe-models) | optional | no | untested | `out of claim` — the model names no TTL model, and RFC 3443 makes these two a choice |
| [MPLS-F-TRAFFIC-CLASS](../../protocol/mpls/features.md#mpls-f-traffic-class) | optional | yes | untested | `unverified` — every core statement is a permission; the model never sets the field |
| [MPLS-F-INVALID-LABEL](../../protocol/mpls/features.md#mpls-f-invalid-label) | mandatory | yes | untested | `unverified` — level 3 by the standards map; the model discards a label without a binding (`Mpls.cc:310-316`), which a level 3 check will confirm |
| [MPLS-F-ICMP](../../protocol/mpls/features.md#mpls-f-icmp) | optional | yes | untested | `unverified` — level 3 by the standards map; the model sends an ICMP message only about a too-big datagram with the DF bit |
| [MPLS-F-FRAGMENTATION](../../protocol/mpls/features.md#mpls-f-fragmentation) | mandatory | yes | partial | `partial` — every core statement that has a check passes; the IPv6 rules FRAG-18 and FRAG-19 are not claimed |
| [MPLS-F-PPP](../../protocol/mpls/features.md#mpls-f-ppp) | mandatory | yes | partial | `partial` — RFC3032-PPP-1, PPP-11: no MPLS Control Protocol, [gap 7](results.md#gap-7-unimplemented-feature--no-mpls-control-protocol); the encapsulation itself passes |
| [MPLS-F-LAN](../../protocol/mpls/features.md#mpls-f-lan) | mandatory | yes | supported | `confirmed` |

### How to read the matrix

- **No feature is `defect` since the repairs of 2026-09-25.** The level 2 run had two: the
  Ethernet link (gap 4) and the reserved labels (gap 5), and the TTL defects of gaps 1 to 3 sat
  inside a `partial` MPLS-F-TTL. All three features are now `confirmed` or `partial` by statements
  without a check.
- **Three features are `partial`.** The reserved labels and fragmentation have core statements
  that the model does not claim (IPv6) or that belong to label distribution; every checked one
  passes. PPP lacks the MPLS Control Protocol (gap 7), which has a plan of its own.
- **Three features are `unverified`.** The discard of a label without a binding, which is
  mandatory, and the ICMP message are level 3 by the standards map; the mandatory one is a
  headline for the next pass. The Traffic Class field is `unverified` because its core statements
  are permissions that no observation can fail.
- **The one `out of claim` feature, the Pipe and Short Pipe Models, is optional**, and the model
  names no TTL model.
