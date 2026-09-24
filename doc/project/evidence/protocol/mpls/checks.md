# MPLS — English check procedures

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rfc3031/catalog.md](../../standard/rfc3031/catalog.md), [rfc3032/catalog.md](../../standard/rfc3032/catalog.md), [rfc3443/catalog.md](../../standard/rfc3443/catalog.md), [rfc5462/catalog.md](../../standard/rfc5462/catalog.md), [features.md](features.md)

Step 5 artifact of the standards test workflow. The procedures come from the specification
only. They name no simulation model and no code. This file holds the common mockups, the
rules every check obeys, the index, and the statements that no check of this pass reaches. The
checks themselves live in one file per feature group under [`checks/`](checks); the section
names are the anchors that the coverage ledger links to.

## Common mockups

Every link is a PPP link, except where a check names an Ethernet link. A and B are hosts; R1,
R2 and R3 are LSRs, which are also IPv4 routers. The label bindings are configured by hand
before the start; no label distribution protocol runs. Every node has IPv4 routes to every
prefix of the path.

| Link | IPv4 prefix | Addresses |
| --- | --- | --- |
| L1 | 10.0.1.0/24 | A 10.0.1.10, R1 10.0.1.1 |
| L2 | 10.0.12.0/24 | R1 10.0.12.1, R2 10.0.12.2 |
| L3 | 10.0.23.0/24 | R2 10.0.23.2, R3 10.0.23.3 |
| L4 | 10.0.2.0/24 | R3 10.0.2.1, B 10.0.2.10 |

**The path.** Host A on L1, the LSRs R1, R2 and R3 in a line, host B on L4.

```
  A         R1         R2         R3         B
  |       |    |     |    |     |    |       |
  +--L1---+    +-L2--+    +-L3--+    +--L4---+
```

**The LSP.** One LSP for the FEC "IPv4 datagrams to B" (10.0.2.10). R1 is the LSP Ingress, R2
the intermediate LSR, and R3 the LSP Egress. Unless a check says otherwise:

- the FTN of R1 maps the FEC to an NHLFE that pushes the label 100, next hop R2;
- the ILM of R2 maps the label 100 from L2 to an NHLFE that replaces it with 200, next hop R3;
- the ILM of R3 maps the label 200 from L3 to an NHLFE that pops the stack, next hop B.

**The flows.** A flow is a stream of UDP datagrams from an application of A to an application
of B. "Flow 2000" goes from port 1000 of A to port 2000 of B; "flow 2001" from port 1001 to port
2001, and so on. Unless a check says otherwise, a flow carries 100 octets of payload in each
datagram, one datagram each second from 1 second to 5 seconds, with the IPv4 TTL 32 and the DF
bit clear, and observation lasts 10 seconds. B has an application on the port of each flow,
which counts what it receives.

## Rules every check obeys

- **An observation names the link and the sender.** "On L2, from R1" means the frames that R1
  sends onto L2, as they leave R1. The label stack entries and the IPv4 header are read from the
  frame, and the fields of an entry from its 4 octets, in the order of transmission.
- **Delivered and discarded are seen at the receiving application.** A datagram is delivered
  when the application of B on its port receives it, with the payload that A sent. A datagram
  that the path discards never reaches that application; the check reads the count of the
  application at the end of the observation.
- **One observation confirms the stimulus.** Each procedure has an observation that shows the
  situation the rule needs — the labeled packet on the link, the label value, the too-big
  datagram — before the observation that checks the rule. A configuration that voids the
  stimulus then fails the check instead of passing it.
- **The fields of a header come last** in a check that also observes delivery, so that a wrong
  field does not keep the delivery from a verdict; a check never puts two rules into one
  observation when one could hide the other.
- **A binding is configured by hand.** Where a rule depends on label distribution — the request
  of an egress for penultimate hop popping, or the Explicit NULL or Implicit NULL label that an
  egress distributes — the check writes the result into the NHLFE of the upstream LSR, as a
  label distribution protocol would, and says so.
- **The TTL follows the Uniform Model** of RFC 3443 §3.1, the treatment of RFC 3032 §2.4. The
  LSRs are also IPv4 routers: an LSR that forwards a datagram by its IPv4 header decrements the
  IPv4 TTL, as RFC 1812 §5.3.1 says, and the ingress does so before it labels the datagram.

## Index

| Check | File | Statements |
| --- | --- | --- |
| [The label stack entry](checks/encoding.md#the-label-stack-entry) | `checks/encoding.md` | RFC3032-ENC-1, ENC-2, ENC-3, ENC-5, ENC-6, ENC-7, ENC-9, ENC-10, RFC5462-TC-1, RFC3031-TTL-3; covers RFC3032-ENC-4, RFC3031-LBL-3, LBL-4 |
| [A label stack of two entries](checks/encoding.md#a-label-stack-of-two-entries) | `checks/encoding.md` | RFC3032-ENC-8, ENC-10, RFC3031-NHLFE-4, STK-1, NHLFE-7; covers RFC3031-STK-2, LSP-7 |
| [Label switching along an LSP](checks/forwarding.md#label-switching-along-an-lsp) | `checks/forwarding.md` | RFC3031-FTN-1, SWAP-2, LSP-1, ILM-1, SWAP-1, NHLFE-1, NHLFE-2, LSP-4, NHLFE-3, LSP-6, RFC3032-NLP-1, NLP-4, NLP-5, NLP-2, NLP-3; covers RFC3031-LSP-2, LSP-3, SWAP-4, LBL-2, NHLFE-5, NHLFE-6 |
| [Penultimate hop popping](checks/forwarding.md#penultimate-hop-popping) | `checks/forwarding.md` | RFC3031-PHP-1, PHP-2, PHP-3, PHP-6; covers RFC3031-PHP-4, PHP-5, NULL-3 |
| [The IPv4 Explicit NULL label](checks/reserved-labels.md#the-ipv4-explicit-null-label) | `checks/reserved-labels.md` | RFC3032-ENC-11 |
| [The Implicit NULL label](checks/reserved-labels.md#the-implicit-null-label) | `checks/reserved-labels.md` | RFC3032-ENC-15, RFC3031-NULL-1 |
| [The TTL of the first label](checks/ttl.md#the-ttl-of-the-first-label) | `checks/ttl.md` | RFC3032-TTL-9, RFC3031-TTL-4, RFC3443-PUSH-1, MOD-1; covers RFC3032-TTL-8, RFC3443-ITTL-1 |
| [The TTL at each LSR](checks/ttl.md#the-ttl-at-each-lsr) | `checks/ttl.md` | RFC3032-TTL-1, TTL-2, RFC3443-TERM-2, ITTL-2, OTTL-2, RFC3032-TTL-6, RFC3443-ITTL-5, RFC3032-TTL-5, RFC3031-TTL-5, TTL-2; covers RFC3032-TTL-7, RFC3443-TERM-1 |
| [The TTL after the last label](checks/ttl.md#the-ttl-after-the-last-label) | `checks/ttl.md` | RFC3032-TTL-10, RFC3031-TTL-6, TTL-1, RFC3443-OTTL-1, ITTL-4, MOD-1, OTTL-5; covers RFC3443-OTTL-3 |
| [A TTL that reaches zero](checks/ttl.md#a-ttl-that-reaches-zero) | `checks/ttl.md` | RFC3032-TTL-3, RFC3443-TERM-3; covers RFC3032-TTL-4, RFC3443-TERM-4, IMPL-2 |
| [A labeled datagram too big for the next link](checks/fragmentation.md#a-labeled-datagram-too-big-for-the-next-link) | `checks/fragmentation.md` | RFC3032-FRAG-8, FRAG-9, FRAG-11, FRAG-12; covers RFC3032-FRAG-1, FRAG-7, FRAG-10, PPP-15 |
| [A too-big labeled datagram that may not be fragmented](checks/fragmentation.md#a-too-big-labeled-datagram-that-may-not-be-fragmented) | `checks/fragmentation.md` | RFC3032-FRAG-13, FRAG-14, FRAG-15 |
| [Labeled packets on a PPP link](checks/links.md#labeled-packets-on-a-ppp-link) | `checks/links.md` | RFC3032-PPP-13, PPP-12, PPP-16 |
| [The MPLS Control Protocol](checks/links.md#the-mpls-control-protocol) | `checks/links.md` | RFC3032-PPP-1, PPP-11, PPP-5 |
| [Labeled packets on an Ethernet link](checks/links.md#labeled-packets-on-an-ethernet-link) | `checks/links.md` | RFC3032-LAN-3, LAN-1, LAN-2 |

## Statements this pass wrote no check for

Level 2 asks for a core check of every mandatory feature. The checks above give one to every
mandatory feature except one: the discard of a packet whose label has no binding, which the
standards map puts at level 3. The statements below have no check in this pass. This section
says **what a check would need**. It says nothing about the simulation model: whether the
absence of a check is acceptable is a judgment about what the model claims, and that judgment
lives in the coverage ledger.

**A packet that no binding covers**, the toolset of level 3: an incoming label that has no
binding, or a binding with no NHLFE, a pop of an unlabeled packet, and a packet whose network
layer protocol the LSR cannot find; each needs a label stack that no configured LSP makes —
RFC3031-SWAP-3, INV-1, NOL-1, RFC3032-NLP-7.

**An ICMP message about a labeled packet**, level 3 by the standards map: the conditions for an
ICMP message, the label stack that the message copies from the packet, the TTL of that stack, and
the errors that an intermediate LSR reports — RFC3032-NLP-6, ICMP-1 to ICMP-6.

**The Router Alert label**, a later level 2 pass: local software in the LSR that takes a packet
with the label 1 at the top, and a rule for what that software does with the packet; RFC 3032
names neither, so a check needs them from another document — RFC3032-ENC-12, ENC-13.

**More than one NHLFE for a label or a FEC**, a larger mockup with two paths: the choice of
exactly one NHLFE for each packet — RFC3031-ILM-2, FTN-2.

**Label distribution**, a protocol of its own (LDP, RSVP-TE), out of the in-scope set: the labels
that an LSR assigns and distributes, the reserved values it must not assign, the uniqueness of a
label for its upstream peers, the request for penultimate hop popping, and the Implicit NULL
binding of an egress — RFC3031-LBL-1, LBL-2, PHP-7, PHP-8, NULL-2, RFC3032-ENC-16.

**A link without a TTL field**, a link type out of the in-scope set (ATM, Frame Relay): an LSP
segment whose link encoding has no TTL, and the TTL that a packet gets after it — RFC3031-TTL-7,
TTL-8.

**A labeled IPv6 datagram**, a later level 2 pass with an IPv6 mockup: the IPv6 Explicit NULL
label, and the Packet Too Big message and the fragments of a too-big labeled IPv6 datagram —
RFC3032-ENC-14, FRAG-16 to FRAG-19.

**The Maximum Initially Labeled IP Datagram Size**, a later level 2 pass: a configuration
parameter of the ingress, and the fragmentation before the first label that it causes —
RFC3032-FRAG-2 to FRAG-6.

**A tunnel**, level 5 by the standards map (RFC 3031 §3.27): the tunnel MTU, and the ICMP message
of the transmitting end of a tunnel — RFC3032-FRAG-20, FRAG-21.

**The negotiation of the MPLS Control Protocol**, level 3 for a wrong packet, level 4 for a
timer: an MPLS Control Protocol packet before the Network-Layer Protocol phase, the Code values
and the Code-Reject, the options of a Configure-Request, and the timeouts of a Configure-Ack —
RFC3032-PPP-2 to PPP-4, PPP-6 to PPP-10.

**Multicast and other LAN encapsulations**, an optional feature (RFC 5332): the PPP Protocol 0283
hex and the ethertype 8848 hex of a labeled multicast packet, and the LLC/SNAP encapsulation —
RFC3032-PPP-14, LAN-4, LAN-5.

**The Pipe and the Short Pipe Models**, an optional TTL treatment of RFC 3443: an LSP whose
ingress sets the TTL of the pushed label to a value of the operator, and whose LSRs keep the TTL
of the tunneled packet — RFC3443-MOD-2 to MOD-4, ITTL-3, OTTL-4, PUSH-2, PUSH-3, IMPL-3.

**The use of the Traffic Class field**, level 5 by the standards map (RFC 3270, RFC 5129): a QoS
or an ECN function that sets or copies the field — RFC5462-TC-2, TC-3.

**A permission that no observation can fail**, at no level: a system that is not an LSR between
two LSRs, and a decrement by more than one — RFC3031-LSP-5, RFC3443-IMPL-1.
