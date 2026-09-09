# IPv6 — English check procedures

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rfc8200/catalog.md](../../standard/rfc8200/catalog.md), [rfc4443/catalog.md](../../standard/rfc4443/catalog.md), [features.md](features.md)

Step 5 artifact of the standards test workflow (see
[derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)).
The procedures come from the specification only. They name no simulation model and no code.
This file holds the common mockup, the rules every check obeys, and the index. The checks
themselves live in one file per feature under [`checks/`](checks); the section names are
the anchors that the coverage ledger links to. The catalog entries are in
[`rfc8200/catalog.md`](../../standard/rfc8200/catalog.md),
[`rfc4443/catalog.md`](../../standard/rfc4443/catalog.md) and
[`rfc8504/catalog.md`](../../standard/rfc8504/catalog.md).

## Index

| Check | File | Statements |
| --- | --- | --- |
| [Datagram delivery](checks/delivery.md#datagram-delivery) | `checks/delivery.md` | RFC8200-HDR-1, HDR-2, HDR-3, HDR-4, DLV-1, HL-1, CKSUM-1 |
| [Hop limit expiry](checks/hop-limit.md#hop-limit-expiry) | `checks/hop-limit.md` | RFC8200-HL-2, RFC4443-TE-1, RFC4443-ERR-2 |
| [Hop limit 1 at the destination](checks/hop-limit.md#hop-limit-1-at-the-destination) | `checks/hop-limit.md` | RFC8200-HL-3; covers HL-1 |
| [Hop limit 0 at the destination](checks/hop-limit.md#hop-limit-0-at-the-destination) | `checks/hop-limit.md` | RFC8200-HL-3 (the exact case) |
| [Source fragmentation and reassembly](checks/fragmentation.md#source-fragmentation-and-reassembly) | `checks/fragmentation.md` | RFC8200-FRAG-3, FRAG-4, FRAG-5 (the structure), FRAG-6, EXT-1, REASM-1, REASM-2, MTU-4; covers HDR-3, RFC8504-NR-2, NR-3 (sender), NR-4 (sender), NR-8 (sender) |
| [Fragment payload length](checks/fragmentation.md#fragment-payload-length) | `checks/fragmentation.md` | RFC8200-FRAG-4, FRAG-5 (the payload length), HDR-2 |
| [Fragment identification](checks/fragmentation.md#fragment-identification) | `checks/fragmentation.md` | RFC8200-FRAG-2; notes RFC8504-NR-5 |
| [Atomic fragment](checks/fragmentation.md#atomic-fragment) | `checks/fragmentation.md` | RFC8504-NR-4 (receiver; governs RFC8200-REASM-6) |
| [Overlapping fragments](checks/fragmentation.md#overlapping-fragments) | `checks/fragmentation.md` | RFC8504-NR-3 (receiver; governs RFC8200-REASM-5) |
| [Short fragment](checks/fragmentation.md#short-fragment) | `checks/fragmentation.md` | RFC8200-REASM-4 |
| [Oversized fragment offset](checks/fragmentation.md#oversized-fragment-offset) | `checks/fragmentation.md` | RFC8200-REASM-8 |
| [Packet too big](checks/packet-too-big.md#packet-too-big) | `checks/packet-too-big.md` | RFC8200-FRAG-1, RFC4443-PTB-1, RFC4443-ERR-2; covers RFC8504-NR-11 |
| [Packet too big, the MTU field](checks/packet-too-big.md#packet-too-big-the-mtu-field) | `checks/packet-too-big.md` | RFC4443-PTB-2 |
| [Link MTU packet](checks/packet-size.md#link-mtu-packet) | `checks/packet-size.md` | RFC8200-MTU-2 |
| [Host error report](checks/error-report.md#host-error-report) | `checks/error-report.md` | RFC4443-DU-4, ERR-1, ERR-2, SRC-1 |
| [Report source address](checks/error-report.md#report-source-address) | `checks/error-report.md` | RFC4443-SRC-2 |
| [No error about an error](checks/error-report.md#no-error-about-an-error) | `checks/error-report.md` | RFC4443-MPR-4 |
| [No error for a multicast destination](checks/error-report.md#no-error-for-a-multicast-destination) | `checks/error-report.md` | RFC4443-MPR-6 |
| [No error for a link-layer multicast](checks/error-report.md#no-error-for-a-link-layer-multicast) | `checks/error-report.md` | RFC4443-MPR-7 |
| [No error for a link-layer broadcast](checks/error-report.md#no-error-for-a-link-layer-broadcast) | `checks/error-report.md` | RFC4443-MPR-8 |
| [No error for an unspecified source](checks/error-report.md#no-error-for-an-unspecified-source) | `checks/error-report.md` | RFC4443-MPR-9 |
| [Zero UDP checksum](checks/input-validation.md#zero-udp-checksum) | `checks/input-validation.md` | RFC8200-CKSUM-1 (the discard) |
| [Unrecognized next header](checks/input-validation.md#unrecognized-next-header) | `checks/input-validation.md` | RFC8504-NR-6 (governs RFC8200-EXT-3) |
| [Unassigned next header](checks/input-validation.md#unassigned-next-header) | `checks/input-validation.md` | RFC8504-NR-6 (governs RFC8200-EXT-3), for an unassigned value |
| [Error for an unknown protocol](checks/error-report.md#error-for-an-unknown-protocol) | `checks/error-report.md` | RFC4443-MPR-3, MPR-4 |
| [Unknown ICMPv6 error type](checks/input-validation.md#unknown-icmpv6-error-type) | `checks/input-validation.md` | RFC4443-MPR-4 (the silence about it); notes MPR-1 |
| [Unknown ICMPv6 informational type](checks/input-validation.md#unknown-icmpv6-informational-type) | `checks/input-validation.md` | RFC4443-MPR-2 |

Twenty-seven checks: nine from the level 2 pass, eighteen added at level 3.

## Common mockup

Three nodes in a row. Host A is the source. Node R is a router. Host B is the destination.

```
A ----------- R ----------- B
    link 1        link 2
```

- The links are Ethernet with an MTU of 1500 octets unless a check says otherwise. No
  check sets an MTU below 1280 octets on any link (RFC8200-MTU-1).
- Every node has one IPv6 address per link from one flat address plan, and the router
  knows the route to each host. Router discovery, address configuration and neighbor
  discovery run before the first datagram: their messages appear on the wire in every
  scenario, and no check judges them.
- A small application on host A sends one UDP datagram to host B. Timing: host A sends
  5 seconds after the start, when the addresses of every node are usable. All expected
  events occur within 1 second of that send. Observation stops after 10 seconds.
- Each check states its own scenario constants: data size, sender hop limit, and the MTU
  of a link. Unless a check says otherwise, the sender hop limit is 64.
- Every node computes the UDP checksum of the datagrams it sends. A simulation model may
  offer a mode that assumes checksums correct without computing them; the checks run with
  that mode off, because one observation reads the value.

### Variants for the level 3 checks

**With a relay.** A transparent relay T is spliced into link 2 next to host B. The relay
forwards every frame unchanged unless the check tells it to rewrite one field of one
packet, or to shorten one packet. IPv6 has no header checksum, so a rewrite of a header
field needs no recompute; a rewrite inside UDP or ICMPv6 is stated with its checksum
consequence. "On link 2" means before the relay; "at host B's interface" means after it.

```
A ----------- R ------ T ------ B
    link 1          link 2
```

**On one link.** Host A and host B share one link, without a router. The multicast check
uses it, because a link-local multicast does not cross a router.

```
A ----------- B
    link 1
```

### Rules every check obeys

- **One observation confirms the stimulus.** A packet with the crafted field is observed
  where it enters the node under test.
- **A discard is observed where the decision is made.** The router of this mockup reports
  every discard it makes with an ICMPv6 message, so the origination of that message at the
  router is the record of the decision. A host that discards silently, as the level 3 rules
  require, leaves no record at all; there the arrival of the crafted packet at the host's
  interface is the anchor, which precedes the decision, and the absence watch opens at
  that moment.
- **Silence has two halves.** "Silently discard" means that nothing goes upward and nothing
  goes back. Every silent-discard check watches both.
- **A `may` or a `should` is not a violation when absent.** Where a check asserts the
  cooperative behavior of a should, its notes say so, and the ledger keeps the strengths
  apart.
