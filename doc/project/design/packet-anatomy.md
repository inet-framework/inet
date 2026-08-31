# What a packet is made of

> **Kind:** design · **Status:** current · **Seal:** by section · **Owns:** — · **Stands on:** [decisions.md](decisions.md), [rule/architecture.md](../rule/architecture.md)

The settled form of INET's most-depended-upon value type. How to *use* the API is
`doc/src/developers-guide/ch-packets.rst` and `ch-tags.rst`; this document says what the
representation is, and which decision put each part there.

## The four things that travel

A packet on its way through a node and across a medium is four separate things, and keeping them
apart is the whole design:

| | What it is | Where it stops |
| --- | --- | --- |
| **content** | a tree of typed, immutable, shared chunks — the bytes that a real packet would carry | it crosses the wire |
| **tags** | local metadata: which interface, which socket, which flow | at the transmission boundary |
| **the signal** | the physical transmission, with its analog model | at the receiver's antenna |
| **the region tags** | identity that survives being cut, joined and re-encapsulated | with the bytes they mark |

The partition is by *what the information is*, not by what is convenient
([D-CHUNKS](decisions.md#d-chunks), [D-TAGS](decisions.md#d-tags),
[D-SIGNAL](decisions.md#d-signal)). The property it produces is **evidentiary continuity**: a field
inspected in a model, serialized into a capture, processed by the PHY and attributed to a flow is one
representation throughout, so an analysis tool cannot report something the model never represented.

## The content

A chunk is typed and immutable once it is referenced, and a packet holds a tree of them rather than a
byte array. Three consequences a reader needs:

- **A header is a value with fields**, not an offset. A model that has not decided its encoding does
  not have to invent one.
- **Sharing replaces copying.** One chunk reaches a hundred receivers without a hundred copies.
- **Peek, insert and update** are the operations, where a byte buffer would have one assignment. That
  is the cost, and [REJ-06](rejected-designs.md#rej-06) records why it is worth paying.

## The wire boundary

Every header has a registered serializer, so it has both a field form and a raw-byte form, in both
directions ([D-DUAL](decisions.md#d-dual)). That boundary is what lets a packet leave for a capture
file or a real network and come back.

A header added without its serializer is a gap that shows up only at the emulation or capture
boundary, long after the change that made it —
[AR-PKT-DUAL](../rule/architecture.md#ar-pkt-dual) and
[AR-OBS-INTROSPECTION](../rule/architecture.md#ar-obs-introspection) are what catch it.

## The three tools that read a packet

A protocol registers three things beside its serializer, and each answers a different question:

| Tool | Answers |
| --- | --- |
| dissector | which chunk is which, in this packet |
| printer | how does this chunk read, for a human |
| filter | does this packet match what the user asked for |

A protocol that ships a header without them produces packets that the tooling cannot describe, which
is a silent loss: nothing fails, the user simply sees less than the model represented.

## Errors

A transmission error is representable at more than one level of detail — a flag, a corrupted chunk, a
bit error rate applied to a region — because a large scenario and a focused study need different
answers ([D-FIDELITY](decisions.md#d-fidelity),
[AR-PKT-ERRORS](../rule/architecture.md#ar-pkt-errors)). The level is a configuration choice, not a
model rewrite.
