# ARP — English check procedures

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rfc826/catalog.md](../../standard/rfc826/catalog.md), [rfc1122/catalog.md](../../standard/rfc1122/catalog.md), [rfc5494/catalog.md](../../standard/rfc5494/catalog.md), [features.md](features.md)

Step 5 artifact of the standards test workflow. The procedures come from the specification
only. They name no simulation model and no code. This file holds the common mockups, the
rules every check obeys, and the index. The checks themselves live in one file per feature
under [`checks/`](checks); the section names are the anchors that the coverage ledger links
to.

## Index

| Check | File | Statements |
| --- | --- | --- |
| [Address resolution](checks/resolution.md#address-resolution) | `checks/resolution.md` | RFC826-REQ-1, REQ-4, REQ-6, RFC1122-AUSE-1; covers FMT-3, FMT-5, TABLE-2 |
| [The cached mapping](checks/resolution.md#the-cached-mapping) | `checks/resolution.md` | RFC826-REQ-2 |
| [The cache flush](checks/resolution.md#the-cache-flush) | `checks/resolution.md` | RFC1122-ACACHE-1, RFC1122-ACACHE-2 |
| [Reply field values](checks/reply.md#reply-field-values) | `checks/reply.md` | RFC826-RECV-8, RECV-9; covers FMT-6, TABLE-2 |
| [Learning from a request](checks/reply.md#learning-from-a-request) | `checks/reply.md` | RFC826-RECV-6, RECV-7 |
| [Packet layout on the wire](checks/packet-format.md#packet-layout-on-the-wire) | `checks/packet-format.md` | RFC826-FMT-1, FMT-2, FMT-4, FMT-7, RECV-11, RFC5494-NUM-1, RFC5494-NUM-4 |
| [A request for a third station](checks/input-validation.md#a-request-for-a-third-station) | `checks/input-validation.md` | RFC826-RECV-5; covers RECV-1 |
| [An unsolicited reply](checks/input-validation.md#an-unsolicited-reply) | `checks/input-validation.md` | RFC826-RECV-10; covers RECV-1 |
| [An experimental opcode](checks/input-validation.md#an-experimental-opcode) | `checks/input-validation.md` | RFC826-RECV-10, RFC5494-NUM-3 |
| [An experimental hardware space](checks/input-validation.md#an-experimental-hardware-space) | `checks/input-validation.md` | RFC826-RECV-2, RFC5494-NUM-2 |
| [An unknown protocol space](checks/input-validation.md#an-unknown-protocol-space) | `checks/input-validation.md` | RFC826-RECV-3 |
| [A newer hardware address](checks/cache.md#a-newer-hardware-address) | `checks/cache.md` | RFC826-TABLE-1, RECV-4 |
| [A reply fills the table](checks/cache.md#a-reply-fills-the-table) | `checks/cache.md` | RFC826-RECV-7, RECV-6 |
| [The waiting datagram](checks/queue.md#the-waiting-datagram) | `checks/queue.md` | RFC1122-AQUEUE-1; covers RFC826-REQ-3 |
| [The request rate](checks/flood-prevention.md#the-request-rate) | `checks/flood-prevention.md` | RFC1122-AFLOOD-1 |
| [No destination unreachable](checks/no-error-report.md#no-destination-unreachable) | `checks/no-error-report.md` | RFC1122-ANOERR-1 |

Sixteen checks. Ten of them need observation alone and belong to level 2. Six need a
crafted packet that no program can send, and those are the level 3 half: the five checks of
[`input-validation.md`](checks/input-validation.md) after the first one, and the two of
[`cache.md`](checks/cache.md).

## Statements that no check carries

Four statements of the in-scope set have no check, and each one has its own reason.

| Statement | What it demands | Why no check |
| --- | --- | --- |
| RFC826-REQ-5 | the target hardware address of a request means nothing, and may be the hardware broadcast address | a permission about a field whose value the standard declares meaningless. A check could read the value and assert nothing about it. What matters is that a receiver does not depend on the field, and that is [Reply field values](checks/reply.md#reply-field-values): the reply is right whatever the field held |
| RFC826-TABLE-3 | table aging is outside the scope of RFC 826 | overridden. RFC 1122 §2.3.2.1 governs, and [The cache flush](checks/resolution.md#the-cache-flush) checks that entry |
| RFC826-GEN-1 | the protocol space field should name the protocol being resolved on other hardware | the in-scope set has one hardware type, the Ethernet. The standards map files the other types at level 5. The one instance an Ethernet link shows is checked: RFC826-FMT-2, in [Packet layout on the wire](checks/packet-format.md#packet-layout-on-the-wire) |
| RFC5494-PROC-1 | new values follow named allocation policies | the statement binds IANA. No behaviour of a host follows from it |

The coverage ledger records each of these as `no check`, with this reason.

## Common mockups

Three mockups serve the sixteen checks.

**The pair.** Two hosts on one Ethernet link. Host A sends datagrams to host B.

```
   host A  ------------  host B
             link 1
```

**The shared link.** Three hosts on one Ethernet segment, joined by a switch S. A broadcast
from any host reaches both others; a frame addressed to one hardware address reaches that
host alone.

```
   host A  ----\
                +---  switch S  ---+
   host B  ----/                    \---  host C
```

- The switch is a station of the link layer only. It has no protocol address, it answers no
  ARP packet, and it forwards every frame. It stands in for the cable of RFC 826, which
  knew no switch: on a cable every station receives a broadcast, and the switch gives the
  same effect for three hosts.
- Host C exists so that a third protocol address exists on the link. In some checks it
  sends and receives nothing.

**The absent neighbour.** The pair, and host A sends to a protocol address on the link that
no station owns.

```
   host A  ------------  host B          (10.0.0.99: nobody)
             link 1
```

- The address is a real address of the link, so routing sends the datagram out of link 1
  and asks for a resolution. No station answers, ever.

## Rules every check obeys

- The hosts are joined by an Ethernet link. Every ARP packet on it therefore has hardware
  space 1 and hardware length 6 (RFC826-FMT-2), and the protocol resolved is IPv4, so the
  protocol length is 4.
- The addresses come from the scenario, not from the check. A check names a host, and reads
  the addresses of that host from the host itself. This keeps a check independent of the
  numbering the run chose.
- ARP has no error message of any kind, so every discard check is an absence: the receiver
  puts no ARP packet on the link, and it hands nothing up. An absence check must also show
  that its stimulus arrived; otherwise a scenario that sends nothing passes it.
- Every check makes one observation confirm the stimulus. For an injected packet that is
  the packet at the interface of the receiving host. For a normal exchange it is the request
  on the link.
- The traffic is UDP over IPv4, 100 octets of data, from port 4000 to port 5000. Nothing in
  ARP depends on the transport, the port or the size; one shape serves every check so that
  a reader can compare two checks line by line.
- Timing: a sender starts at 0.1 s. Every expected event of a level 2 check occurs within
  1 second, and observation stops after 1 second. The two checks that measure a time — the
  cache flush and the request rate — say what their own limit is.
- The address resolution of the hosts is the one under test. No check may replace it with a
  table that the scenario fills in advance, because that table is the thing being checked.
  The only exception is a check that watches the traffic of a switch, and none of these
  sixteen does.

## Why a crafted packet is needed at all

Ten of the sixteen checks watch a normal exchange. The other six ask what a host does with
a packet it cannot use, and a program cannot produce such a packet: the address resolution
module builds every ARP packet itself, from its own addresses and from the address that
routing gave it. There is no interface through which anything else reaches the field values.

The six checks therefore put a packet on the receiving host's interface as if it had come
from the link. Four of them craft a field that no conforming station on this link would
send — an opcode reserved for experiments, a hardware space reserved for experiments, a
protocol space of another protocol — and two craft a well-formed packet from a station that
is not there, which is how a table entry can be made to change without a second host.
