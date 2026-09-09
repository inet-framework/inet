# UDP — English check procedures

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rfc768/catalog.md](../../standard/rfc768/catalog.md), [rfc792/catalog.md](../../standard/rfc792/catalog.md), [rfc1122/catalog.md](../../standard/rfc1122/catalog.md), [features.md](features.md)

Step 5 artifact of the standards test workflow. The procedures come from the specification
only. They name no simulation model and no code. This file holds the common mockups, the
rules every check obeys, and the index. The checks themselves live in one file per feature
under [`checks/`](checks); the section names are the anchors that the coverage ledger links
to.

## Index

| Check | File | Statements |
| --- | --- | --- |
| [Datagram delivery](checks/delivery.md#datagram-delivery) | `checks/delivery.md` | RFC768-HDR-2, HDR-3, PROTO-1, UI-1; covers HDR-1 |
| [Valid source address](checks/delivery.md#valid-source-address) | `checks/delivery.md` | RFC1122-UADDR-2 |
| [Empty datagram](checks/delivery.md#empty-datagram) | `checks/delivery.md` | RFC768-HDR-3 (the minimum) |
| [Checksum presence and absence](checks/checksum.md#checksum-presence-and-absence) | `checks/checksum.md` | RFC768-CKSUM-2; covers CKSUM-1 (presence) |
| [Checksum by default](checks/checksum.md#checksum-by-default) | `checks/checksum.md` | RFC1122-UCK-3; covers UCK-1, RFC768-CKSUM-1 |
| [Checksum discard](checks/checksum.md#checksum-discard) | `checks/checksum.md` | RFC1122-UCK-4 |
| [Checksum covers the data](checks/checksum.md#checksum-covers-the-data) | `checks/checksum.md` | RFC768-CKSUM-1, RFC1122-UCK-4 |
| [Checksum covers the pseudo header](checks/checksum.md#checksum-covers-the-pseudo-header) | `checks/checksum.md` | RFC768-CKSUM-1, RFC1122-UCK-4 |
| [Zero checksum accepted](checks/checksum.md#zero-checksum-accepted) | `checks/checksum.md` | RFC768-CKSUM-2, RFC1122-UCK-5 |
| [Port unreachable](checks/port-unreachable.md#port-unreachable) | `checks/port-unreachable.md` | RFC1122-UPORT-1 (governs RFC792-DU-3); covers RFC768-HDR-2 |
| [Multicast source address](checks/input-validation.md#multicast-source-address) | `checks/input-validation.md` | RFC1122-UADDR-1 |
| [TTL and TOS from the program](checks/app-interface.md#ttl-and-tos-from-the-program) | `checks/app-interface.md` | RFC1122-UAPI-1 |
| [Source address from the program](checks/app-interface.md#source-address-from-the-program) | `checks/app-interface.md` | RFC1122-UMH-2 |

Thirteen checks: three from the level 2 pass, ten added at level 3.

## Statements that no check carries

Nine statements of the in-scope set have no check, and each one has the same reason: the
observation is at the interface between UDP and a program, not on a link between two nodes.
A check of this kind can watch what two nodes send each other; it cannot watch what a module
hands to the program above it.

| Statement | What it demands | Why no check |
| --- | --- | --- |
| RFC1122-UERR-1 | UDP passes every ICMP error up to the program | the handoff to the program is not traffic on a link |
| RFC1122-UOPT-1 | a received IP option reaches the program | the same |
| RFC1122-UOPT-2 | the program can name the IP options of a sent datagram | needs an interface for options in the sending program |
| RFC1122-UMH-1 | the specific destination address reaches the program | the same as UERR-1 |
| RFC1122-UMH-3 | the program can learn which source address was chosen | the same |
| RFC1122-UAPI-2 | UDP may pass the received TOS to the program | the same; and a `may` |
| RFC1122-UAPI-3 | the interface gives the full service of §3.4 | §3.4 is an interface catalog, not a behaviour on a link |
| RFC1122-UCK-2 | the program may control whether a checksum is generated | a permission for an interface; its effect on the wire is the subject of Checksum by default |
| RFC1122-UCK-6 | a computed checksum of zero goes out as all ones | needs data whose sum is exactly zero, which depends on addresses and ports that the run assigns |

The coverage ledger records each of these as `no check`, with this reason. They are listed
here and not forgotten: a later level that adds a module test, and not a test of two nodes
on a link, can reach the first seven.

## Common mockups

Three mockups serve the twelve checks. The first two come from the level 2 pass. The third
is the one the level 3 checks need, because a datagram can only be changed while it travels.

**The plain mockup.** Two hosts on one link. For the checksum check a third host joins, on
a second link of host B, so that two senders with different checksum settings reach one
receiver.

```
   host A  ------------  host B  ------------  host C
            link 1                link 2
```

**The mockup with a relay.** A relay T sits on the path between the two hosts and holds each
frame for a moment. A gateway R stands before it, so that the datagram is a normal one when
the relay gets it.

```
   host A  ------  gateway R  ------  relay T  ------  host B
           link 1             link 2            link 2
```

- The relay changes one datagram and passes every other frame on without a change. It is
  not a node of the protocol: it has no address and it answers nothing.
- What the relay does is what a fault on the path would do. No program can produce it, and
  that is why these checks are level 3.

**The mockup with two links at the sender.** Host A has a second interface on a link of its
own, so that host A owns two addresses.

```
   host D  ------  host A  ------  host B
           link 3          link 1
```

- Host D is there to give link 3 a far end. It sends nothing and receives nothing.

- Host A and host C send. Host B receives; it runs one program per open port.
- Timing: the senders start shortly after the start. Every expected event occurs within 1
  second; observation stops after 1 second.
- The data is carried by UDP over IPv4 with a 20-octet IP header and no options. A UDP
  datagram with N octets of data is 8 + N octets long and rides in an IP datagram of
  28 + N octets.
- The programs on host B are receivers that open one port each. No program on host B opens
  port 6000.
- Where a check needs real checksums, it says so, and then every host generates and checks
  the value that RFC 768 defines. A check that needs the default state says that too, and
  then nothing in the scenario mentions the checksum at all.
- Where a check tells two datagrams apart at the receiver, it uses their sizes: 100 octets
  and 200 octets. Nothing in UDP depends on those sizes.
