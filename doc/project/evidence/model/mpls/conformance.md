# MPLS — model claims

> **Kind:** report · **Status:** snapshot 2026-09-23 · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/mpls/standards.md), [coverage.md](coverage.md)

Step 8 artifact of the standards test workflow, part 1 only. A level 1 pass records what the
model intends: which standards it claims to implement, mapped onto the standards map. Part 2,
the conformance matrix, needs the feature map and the verdicts of a level 2 pass.

Read record — no build, no run:

- Date: 2026-09-23
- INET: branch `topic/standards-tests-wave0`, commit `e360ca980e`, tree clean
- Trees: src `16dc528e10`, tests/protocol `6f0a6bdb05`

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
