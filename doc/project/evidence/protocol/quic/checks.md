# QUIC — English check procedures

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [catalog.md](../../standard/rfc9000/catalog.md), [features.md](features.md)

Step 5 artifact of the standards test workflow. This file holds the common mockups, the
scenario constants, the rules every check obeys, and the index. The checks themselves live in
one file per feature under [`checks/`](checks); the section names are the anchors that the
coverage ledger links to. The procedures come from the specification only. They name no
simulation model and no code.

## Index

| Check | File | Statements |
| --- | --- | --- |
| [Connection establishment](checks/establishment.md#connection-establishment) | `checks/establishment.md` | RFC9000-PKT-1, PKT-2, SIZE-1 (the client half) |
| [Server Initial datagram size](checks/establishment.md#server-initial-datagram-size) | `checks/establishment.md` | RFC9000-SIZE-1 (the server half) |
| [Stream data transfer](checks/streams.md#stream-data-transfer) | `checks/streams.md` | RFC9000-STR-2, STR-3, PKT-3 |
| [Stream identifiers](checks/streams.md#stream-identifiers) | `checks/streams.md` | RFC9000-STR-1 |
| [Stream identifier assignment](checks/streams.md#stream-identifier-assignment) | `checks/streams.md` | RFC9000-STR-1, the decisive form |
| [Ordered delivery under reordering](checks/streams.md#ordered-delivery-under-reordering) | `checks/streams.md` | RFC9000-STR-2, at its edge |
| [Flow control](checks/flow-control.md#flow-control) | `checks/flow-control.md` | RFC9000-FC-1 |
| [Acknowledgment](checks/acknowledgment.md#acknowledgment) | `checks/acknowledgment.md` | RFC9000-ACK-1 |
| [Connection close](checks/close.md#connection-close) | `checks/close.md` | RFC9000-CLOSE-1 |
| [Version negotiation](checks/version-negotiation.md#version-negotiation) | `checks/version-negotiation.md` | RFC9000-VER-1 |
| [Anti-amplification limit](checks/address-validation.md#anti-amplification-limit) | `checks/address-validation.md` | RFC9000-AMP-1 |
| [Unknown frame type](checks/frame-validation.md#unknown-frame-type) | `checks/frame-validation.md` | RFC9000-ERR-1 |

Twelve checks: seven from the level 2 pass, five added at level 3.

## Statements that no check carries

| Statement | What it demands | Why no check |
| --- | --- | --- |
| RFC9000-VER-2 | no endpoint answers a Version Negotiation packet with another | needs such a packet delivered to an endpoint; the relay could make one, and this pass did not |

## Common mockups

Two mockups serve the twelve checks. The first is the one the level 2 pass used. The second
is what the level 3 checks need, because a packet can only be changed or held while it
travels.

### The plain mockup

Two hosts and a path between them. Host A opens a QUIC connection to host B, sends a block
of application data on one stream, and closes.

```
   host A  ------------  host B
            path P
   (client)              (server)
```

Scenario constants, shared unless a check says otherwise:

| Constant | Value | Why |
| --- | --- | --- |
| Path | one link, no loss, no reordering | level 2 observes the normal path; loss belongs to level 3 and level 4 |
| Data block | 4000 octets on one stream | several packets' worth, so packet numbers and ordering are visible |
| Observation limit | 5 s | generous; nothing in the scenario needs more |

Two properties of QUIC shape every check below, and neither has a counterpart in the
protocols already in this tree:

- **QUIC rides on UDP.** Every QUIC packet is the payload of a UDP datagram. An observation
  of "a packet on the path" is therefore an observation of a UDP datagram, and its size is
  the size the UDP datagram carries.
- **QUIC packets are normally protected.** A real endpoint encrypts everything after the
  header. A check that reads a field of a frame is possible only where the observer has the
  keys, or where packet protection is absent. Each check below states which fields it
  needs; a reader can then tell which checks survive on a protected connection.

### The mockup with a relay

A relay T sits on the link and holds each datagram for a moment.

```
   client  ------  relay T  ------  server
```

- The relay changes or holds one datagram and passes every other one on unchanged. It is not
  an endpoint of the protocol: it has no address of its own and it answers nothing.
- A relay holds one rule at a time. A check that needs two changes needs two relays in
  series.
- What the relay does is what a network or a third party would do: reorder a packet, write a
  version nobody speaks, or set a frame type from a registry entry that does not exist. No
  application can produce any of it, and that is why these checks are level 3.
- A relay that changes a field of a QUIC packet rebuilds the packet from its parts. The
  content of a QUIC packet is a list of chunks — the packet header, then each frame's header
  and its data — and a change means replacing one of them and putting the list back.

