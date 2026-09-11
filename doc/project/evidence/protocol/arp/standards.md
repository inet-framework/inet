# ARP — standards family and in-scope set

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

Step 2 artifact of the standards test workflow. This document maps the family of standards
around the Address Resolution Protocol, records which document governs each contested
clause, and pins the exact set that the current pass tests against.

The relationships come from the RFC-editor metadata (`https://www.rfc-editor.org/rfc/rfcNNN.json`),
retrieved 2026-09-11.

## Target level

**Level 3 — Edge** (see [the levels of the guide](../../../guide/derive-tests-from-a-standard.md#levels-of-depth)).
ARP had no evidence tree before this pass, so the pass does the work of levels 1, 2 and 3
together. The in-scope set below holds the base document, the host requirements, and the
document that updates the number spaces of the two fields a crafted packet plays with.

| To reach | Add to the in-scope set | Why |
| --- | --- | --- |
| level 2, Core | RFC 826 | the exchange itself: a broadcast request, a direct reply, and the table that both fill |
| level 3, Edge | RFC 1122 §2.3.2, one paragraph of §2.3.3, one of §2.4, and RFC 5494 | in scope now. RFC 1122 states what a host must do that RFC 826 leaves open: flush the table, limit the request rate, keep the waiting packet, and report no error. RFC 5494 makes a crafted opcode a defined case and not an accident |
| level 4, Dynamics | RFC 5227 | the address conflict detection is a control loop with five timers. The retry timer and the cache timeout of level 3 also need a tolerance at level 4 |
| level 5, Complete | RFC 1027, RFC 903, RFC 1868 | proxy ARP over a subnet gateway, the reverse protocol, and the UNARP extension |

What the pass actually reached is not recorded here. It is in
[`model/arp/coverage.md`](../../model/arp/coverage.md#achieved-level).

## Document list

| Document | Title | Date | Status | Relation | Cached |
| --- | --- | --- | --- | --- | --- |
| RFC 826 | An Ethernet Address Resolution Protocol | November 1982 | Internet Standard (STD 37) | `base` | [`standard/rfc826/`](../../standard/rfc826/rfc826.txt), 2026-09-11 |
| RFC 1122 | Requirements for Internet Hosts — Communication Layers | October 1989 | Internet Standard | `companion` (§2.3.2, one paragraph of §2.3.3 and one of §2.4: the ARP host requirements) | [`standard/rfc1122/`](../../standard/rfc1122/rfc1122.txt), 2026-09-09; one copy, shared with IPv4 and UDP |
| RFC 5494 | IANA Allocation Guidelines for the Address Resolution Protocol (ARP) | April 2009 | Proposed Standard | `updates` RFC 826 | [`standard/rfc5494/`](../../standard/rfc5494/rfc5494.txt), 2026-09-11 |
| RFC 5227 | IPv4 Address Conflict Detection | July 2008 | Proposed Standard | `updates` RFC 826 | no |
| RFC 903 | A Reverse Address Resolution Protocol | June 1984 | Internet Standard | `companion`; a separate protocol in the same packet format | no |
| RFC 1027 | Using ARP to Implement Transparent Subnet Gateways | October 1987 | Unknown status | `companion`; proxy ARP | no |
| RFC 1868 | ARP Extension — UNARP | November 1995 | Experimental | `companion`; never a standards-track update | no |

Source of the cached texts:

- `rfc826.txt` in [`evidence/standard/rfc826/`](../../standard/rfc826/) —
  <https://www.rfc-editor.org/rfc/rfc826.txt>, downloaded 2026-09-11.
- `rfc5494.txt` in [`evidence/standard/rfc5494/`](../../standard/rfc5494/) —
  <https://www.rfc-editor.org/rfc/rfc5494.txt>, downloaded 2026-09-11.

The RFC 1122 catalog is the one that IPv4 and UDP use. IPv4 owns the entries of §3 there,
UDP owns the entries of §4.1, whose identifiers start with `U`, and this pass added the
entries of §2.3 and §2.4, whose identifiers start with `A`. The scope statement of that
catalog names all three protocols.

The in-scope set takes three clauses of §2 and not the whole of it, and the boundary is the
subject and not the size. §2.3.2 is the ARP section, so it is in scope in full. §2.3.3 and
§2.4 each hold one paragraph that speaks about address translation and several that speak
about the Ethernet encapsulation and about the interface between IP and the link layer; a
pass on the Ethernet link layer owns those, and this one names only the two paragraphs it
takes. The catalog lists the rest as out of scope.

RFC 826 predates RFC 2119 by eleven years. It carries two `must` words in the whole
document and no other keyword, so nearly every entry of its catalog has the strength
`description`. The level of a feature therefore comes mostly from the "only path" rule of
the guide, and not from a keyword. RFC 1122 is the document that gives ARP its keywords.

## Override table

| Area | Base clause | Later clause | Governs | In scope now |
| --- | --- | --- | --- | --- |
| Aging the translation table | RFC 826, `rfc826.txt:415-416`: "It may be desirable to have table aging and/or timeouts. The implementation of these is outside the scope of this protocol." | RFC 1122 §2.3.2.1, `rfc1122.txt:1286-1289`: an implementation MUST provide a mechanism to flush out-of-date cache entries, and a timeout SHOULD be configurable | RFC 1122 | yes, from level 3 |
| The rate of requests for one address | RFC 826 states no limit; its own answer to the question is that information travels "as it is needed, and only once (probably) per boot", `rfc826.txt:257-258` | RFC 1122 §2.3.2.1, `rfc1122.txt:1291-1305`: a mechanism to prevent ARP flooding MUST be included, and the recommended maximum rate is one request per second per destination | RFC 1122 | yes, from level 3 |
| The datagram that waits for the resolution | RFC 826, `rfc826.txt:172-175`: the module "probably informs the caller that it is throwing the packet away (on the assumption the packet will be retransmitted by a higher network layer)" | RFC 1122 §2.3.2.2, `rfc1122.txt:1375-1378`: the link layer SHOULD save at least the latest packet and transmit it when the address is resolved | RFC 1122 | yes, from level 3 |
| The `ar$hrd` and `ar$op` number spaces | RFC 826, `rfc826.txt:272-274`: "Currently the only defined value is for the 10Mbit Ethernet" | RFC 5494 §2 and §3, `rfc5494.txt:119-183`: the allocation rules, two experimental values in each space, and the values 0 and 65535 reserved in both | RFC 5494 | yes, from level 3 |
| A packet whose sender protocol address is zero | RFC 826 gives the field no special value; the reception algorithm treats it as any other address, `rfc826.txt:209-218` | RFC 5227 §2.1.1: an ARP Probe carries an all-zero sender protocol address on purpose, so that the probe cannot pollute the table of another host | RFC 5227 | no; level 4 |
| Defending an address in use | RFC 826 knows no conflict: "hosts don't transmit information about anyone other than themselves", `rfc826.txt:459-460`, so a second host with the same address is outside its model | RFC 5227 §2.4: what a host does when it sees its own address in the sender field of another host | RFC 5227 | no; level 4 |

The first three rows are the reason RFC 1122 is in the set. RFC 826 is a description of a
mechanism, and RFC 1122 turns three of its open questions into requirements on a host.
The fourth row is what makes a crafted opcode a checkable case: RFC 5494 reserves two
opcode values for experiments, so a packet with opcode 24 is a well-defined packet that
no conforming host answers, and not a corruption.

The two RFC 5227 rows stay out of scope, and the reason is the level and not the subject.
Address conflict detection is a control loop: a host probes an address a set number of
times, waits a set time between probes, announces the address, and defends it at a limited
rate. The guide files timers and control loops at level 4, and every requirement of RFC 5227
§2 carries one of its five time constants. The ARP Probe packet shape belongs to that
document, so this pass does not check it as such; a packet with an unusual sender address
is checked at level 3 through the reception algorithm of RFC 826, which discards a packet
that is not addressed to the receiver whatever its sender field holds.

## In-scope set

| Document | Version | Catalog file |
| --- | --- | --- |
| RFC 826 | November 1982, Internet Standard, no revision of the body since | [`standard/rfc826/catalog.md`](../../standard/rfc826/catalog.md) |
| RFC 1122 | October 1989, Internet Standard; §2.3.2 in full, the address-translation paragraph of §2.3.3 (`rfc1122.txt:1429-1431`), and the third paragraph of §2.4 (`rfc1122.txt:1493-1494`) | [`standard/rfc1122/catalog.md`](../../standard/rfc1122/catalog.md) |
| RFC 5494 | April 2009, Proposed Standard | [`standard/rfc5494/catalog.md`](../../standard/rfc5494/catalog.md) |

Out of scope, with the reason:

| Document | Reason |
| --- | --- |
| RFC 5227 | Level 4. Every requirement of its §2 carries one of five time constants, and the guide files timers and control loops at level 4. |
| RFC 903 | RARP is a separate protocol. It shares the packet format and two opcodes, and it resolves in the other direction. It gets its own protocol folder when it gets a pass. |
| RFC 1027 | Level 5. Proxy ARP lets a gateway answer for an address that is not its own, which is the one case where the target test of RFC 826 does not decide alone. RFC 1122 §2.3.2.1 mentions proxy ARP only as a reason for the cache timeout. |
| RFC 1868 | Experimental, and never a standards-track update of RFC 826. |
| RFC 5342 | The Ethertype space that `ar$pro` shares. It allocates numbers; it states no behaviour of a host. |
| RFC 1122, the rest of §2 | §2.1 and §2.2 introduce the layer. The rest of §2.3.3 states the Ethernet and IEEE 802 encapsulation rules, and the first two paragraphs of §2.4 demand a broadcast flag and a TOS field in the interface between IP and the link layer. None of them is about address translation; a pass on the Ethernet link layer owns them. |
| RFC 1122 §2.3.1 | The trailer encapsulation negotiation uses ARP replies for a second protocol type. The requirements summary of §2.5 puts "Send Trailers by default without negotiation" in the MUST NOT column, `rfc1122.txt:1511`, and the mechanism itself is a level 5 area. |
