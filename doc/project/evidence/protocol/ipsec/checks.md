# IPsec — English check procedures

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rfc4301/catalog.md](../../standard/rfc4301/catalog.md), [rfc4302/catalog.md](../../standard/rfc4302/catalog.md), [rfc4303/catalog.md](../../standard/rfc4303/catalog.md), [features.md](features.md)

Step 5 artifact of the standards test workflow. The procedures come from the specification
only. They name no simulation model and no code. This file holds the common mockups, the
rules every check obeys, the index, and the statements that no check of this pass reaches. The
checks themselves live in one file per feature group under [`checks/`](checks); the section
names are the anchors that the coverage ledger links to.

## Common mockups

Every link is an Ethernet link. Every host is an IPsec host in the sense of RFC 4301: a native
implementation, with one SPD and one SAD. The SAs are keyed manually before the start
(RFC 4301 §4.5.1); no key management protocol runs.

| Link | IPv4 prefix | IPv6 prefix | Addresses |
| --- | --- | --- | --- |
| L1 | 10.0.1.0/24 | 2001:db8:1::/64 | R 10.0.1.1 or ::1, A 10.0.1.10 or ::10, B 10.0.1.11 or ::11, C 10.0.1.12 or ::12 |
| L2 | 10.0.2.0/24 | 2001:db8:2::/64 | R 10.0.2.1 or ::1, B 10.0.2.11 or ::11 |

**The link.** Hosts A, B and C on L1, through a switch. A sends, B receives; C only receives,
where a check names it.

```
  A      B      C
  |      |      |
 -+------+------+- L1
```

**The path.** Host A on L1, router R between L1 and L2, host B on L2. R is not an IPsec
implementation: it forwards the packets of A and B as a plain router. Where a check gives L2 a
smaller MTU, it says so.

```
  A          R          B
  |        |   |        |
 -+--------+-  -+-------+-
     L1             L2
```

**The flows.** A flow is a stream of UDP datagrams from an application of A to an application
of B. "Flow 2000" goes from port 1000 of A to port 2000 of B; "flow 2001" from port 1001 to port
2001, and so on. Unless a check says otherwise, a flow carries 100 octets of payload in each
datagram, one datagram each second from 1 second to 5 seconds, and observation lasts 10 seconds.
B has an application on the port of each flow, which counts what it receives.

**The SAs.** A check names an SA by its SPI: "SA 101" is the SA with SPI 101. An SA appears at
both ends: outbound at A, inbound at B, with the same SPI, the same protocol and the same
algorithms. Unless a check says otherwise:

- an ESP SA uses AES-CBC with a 128-bit key (a 16-octet IV and a 16-octet block) and
  HMAC-SHA-256-128 (a 16-octet ICV), the integrity algorithm that RFC 8221 makes mandatory;
- an AH SA uses HMAC-SHA-256-128 (a 16-octet ICV);
- the SA is in transport mode, and its anti-replay service is off, as RFC 4302 §5 and RFC 4303
  §5 advise for a manually keyed SA.

**The policies.** A check gives the SPD entries of each host in the order of the search. An
entry names its selectors (local and remote address, next layer protocol, local and remote
port, or ICMP type and code; a selector that the entry does not name is ANY), its direction and
its action. A PROTECT entry names its SAs. At the end of each SPD, after the entries of the
check, one entry discards everything else, as RFC4301-SPD-17 advises. In the IPv6 mockups each
SPD also begins with a BYPASS entry for ICMPv6 in both directions, so that Neighbor Discovery
works.

## Rules every check obeys

- **An observation names the link and the sender.** "On L1, from A" means the packets that A
  sends onto L1, as they leave A. The IP header, the AH or ESP header and the ESP trailer are
  read from the packet. The contents of an encrypted ESP payload are read with the keys of the
  SA, which the observer knows because the SAs are keyed manually.
- **Delivered and discarded are seen at the receiving application.** A datagram is delivered
  when the application of B on its port receives it, with the payload that A sent. A datagram
  that B discards never reaches that application; the check reads the count of the application
  at the end of the observation.
- **One observation confirms the stimulus.** Each procedure has an observation that shows the
  situation the rule needs — the protected packet on the link, the fragment, the SA in use —
  before the observation that checks the rule. A configuration that voids the stimulus then
  fails the check instead of passing it.
- **The fields of a header come last** in a check that also observes delivery, so that a wrong
  field does not keep the delivery from a verdict; a check never puts two rules into one
  observation when one could hide the other.
- **A rule of IPv4 or of IPv6 is checked in that version.** Where a rule holds for both versions
  and the header layout differs, the check runs once on each version.
- **An SA is keyed by hand.** Where a rule depends on key management — the creation of an SA
  when a packet needs one, the negotiation of an option — the check uses the case that the
  standard gives for an implementation without it, and says so.

## Index

| Check | File | Statements |
| --- | --- | --- |
| [Outbound dispositions](checks/policy.md#outbound-dispositions) | `checks/policy.md` | RFC4301-PROC-1, PROC-2, SPD-19, SPD-20, OUT-3, SPD-8, SPD-9, SPD-10, SPD-11; covers RFC4301-SPD-2, OUT-1, OUT-2, OUT-6, OUT-13, SPD-32, SPD-7, SPD-12, SPD-16, SPDE-13, SPDE-21, OVW-1, OUT-7, SPD-14, PROC-3 |
| [Inbound dispositions](checks/policy.md#inbound-dispositions) | `checks/policy.md` | RFC4301-IN-4, IN-13, IN-22, IN-25, SPD-13, SPD-18; covers RFC4301-IN-1, IN-2, IN-10, IN-21, IN-23, IN-24, IN-3 |
| [The first matching entry decides](checks/policy.md#the-first-matching-entry-decides) | `checks/policy.md` | RFC4301-SPD-6, SPD-35, SPD-42; covers RFC4301-SPD-15 |
| [Selectors](checks/policy.md#selectors) | `checks/policy.md` | RFC4301-SEL-1, SEL-20, SPD-34, SPD-38, SEL-4, SEL-7, SEL-12, SEL-18, SEL-19, SPD-26, SPDE-11, SPD-17; covers RFC4301-SEL-2, SEL-5, SEL-8, SPD-22, SPD-23, SPDE-3, SPDE-12, NLP-2, SPDE-4 |
| [One SA for each direction](checks/security-associations.md#one-sa-for-each-direction) | `checks/security-associations.md` | RFC4301-SA-1, SA-2, SA-5; covers RFC4301-SA-3, SAD-1, RFC4303-SPI-1, SPI-3, RFC4301-SADI-1 |
| [Inbound SA lookup](checks/security-associations.md#inbound-sa-lookup) | `checks/security-associations.md` | RFC4301-SA-6, SA-18, SAD-3, IN-14, IN-15, IN-17, RFC4302-SPI-2, SPI-15, ISA-1, ISA-2, RFC4303-SPI-13, ISA-1, ISA-2, RFC4301-SA-13, RFC4302-SPI-10, RFC4303-SPI-10, RFC4302-SPI-5, ISA-6, RFC4303-SPI-4, ISA-6; covers RFC4301-SA-7, RFC4302-SPI-3, RFC4301-SA-14, SA-15, SA-16, SA-17, IN-6, IN-8, IN-20, RFC4302-SPI-1, SPI-11, SPI-12, SPI-13, SPI-14, ISA-4, ISA-5, RFC4303-SPI-11, SPI-12, ISA-5, RFC4301-SA-20, RFC4302-SPI-4, SPI-16, RFC4303-SPI-14, ISA-4 |
| [Inbound selector check](checks/security-associations.md#inbound-selector-check) | `checks/security-associations.md` | RFC4301-IN-31, IN-32, IN-33, SAD-6; covers RFC4301-SAD-5, IN-39 |
| [A PROTECT entry without an SA](checks/security-associations.md#a-protect-entry-without-an-sa) | `checks/security-associations.md` | RFC4301-OUT-10; covers RFC4301-OUT-8, OUT-9 |
| [The lifetime of an SA](checks/security-associations.md#the-lifetime-of-an-sa) | `checks/security-associations.md` | RFC4301-SADI-10, SADI-11; covers RFC4301-SADI-14, SADI-15 |
| [Parallel SAs for classes of traffic](checks/security-associations.md#parallel-sas-for-classes-of-traffic) | `checks/security-associations.md` | RFC4301-SA-25, SA-26; covers RFC4301-SA-24, SA-28, SADI-19, SADI-20 |
| [Sequence numbers of an SA](checks/sequence-numbers.md#sequence-numbers-of-an-sa) | `checks/sequence-numbers.md` | RFC4301-SADI-2, RFC4302-SEQ-2, RFC4303-SEQ-2, RFC4302-SEQ-1, SEQ-8, OSEQ-1, OSEQ-2, RFC4303-SEQ-1, SEQ-7, OSEQ-1; covers RFC4302-SEQ-6, OSEQ-9, RFC4303-OSEQ-2, OSEQ-10, SEQ-6 |
| [ESP packet format in transport mode](checks/esp.md#esp-packet-format-in-transport-mode) | `checks/esp.md` | RFC4303-FMT-1, SEQ-5, PAY-4, PAY-5, CONF-1, CONF-2, RFC4301-SA-37, OVW-2, CONF-1, CONF-2, RFC4303-FMT-2, FMT-3, SPI-2, PAY-2, PADL-1, PADL-3, NH-1, ICV-3, ICV-4, LOC-2, LOC-3, OSA-1, OENC-1, IICV-2, IICV-8, IICV-14, RFC4301-SA-34, SA-35, RFC4303-LOC-6, RFC4301-OVW-5; covers RFC4303-PAY-1, PAY-3, FMT-7, FMT-13, FMT-14, FMT-15, FMT-16, OENC-4, OENC-9, IICV-1, RFC4301-SA-38, SA-29, SA-30, SA-50, DB-1 |
| [ESP padding](checks/esp.md#esp-padding) | `checks/esp.md` | RFC4303-PAD-1, PAD-6, PAD-8, PAD-2, PAD-5, PAD-9; covers RFC4303-PAD-4, OENC-3, IICV-11, RFC4301-FUNC-8 |
| [ESP services](checks/esp.md#esp-services) | `checks/esp.md` | RFC4303-ALG-5, ALG-6, CONF-8, CONF-10; covers RFC4303-ALG-3, ALG-7, ALG-9, CONF-9, FMT-5, ICV-1, RFC4301-OVW-6, OVW-8, RFC4303-FMT-6, FMT-8, RFC4301-OVW-9 |
| [An ESP SA without encryption and without integrity](checks/esp.md#an-esp-sa-without-encryption-and-without-integrity) | `checks/esp.md` | RFC4303-ALG-2, CONF-11, RFC4301-FUNC-9 |
| [Traffic flow confidentiality padding](checks/esp.md#traffic-flow-confidentiality-padding) | `checks/esp.md` | RFC4303-TFC-2, TFC-5; covers RFC4303-TFC-3, TFC-4, TFC-6, IICV-17, FMT-11, PADL-2 |
| [Dummy packets](checks/esp.md#dummy-packets) | `checks/esp.md` | RFC4303-NH-2, NH-3, NH-4, NH-5; covers RFC4303-NH-6 |
| [AH format in transport mode](checks/ah.md#ah-format-in-transport-mode) | `checks/ah.md` | RFC4302-FMT-1, RSV-2, SEQ-5, SEQ-7, ICV-4, FMT-2, NH-2, LEN-1, ICV-1, LOC-2, LOC-3, LOC-7, OSA-1, IICV-2, LEN-2, ICV-2, LOC-5; covers RFC4302-FMT-3, NH-1, RSV-1, OICV-25, OICV-26, OICV-27, RFC4301-SA-39, RFC4302-ICV-3, LOC-1, OICV-28, CONF-1, CONF-2 |
| [AH across a router](checks/ah.md#ah-across-a-router) | `checks/ah.md` | RFC4302-OICV-1, OICV-13, OICV-19, OICV-5; covers RFC4302-OICV-2, OICV-3, OICV-6, OICV-7, OICV-17, IICV-1, IICV-5, IICV-9, ALG-1, OICV-11, IICV-6 |
| [Tunnel mode between two hosts](checks/modes.md#tunnel-mode-between-two-hosts) | `checks/modes.md` | RFC4301-SA-47, TUN-1, TUN-19, TUN-26, RFC4303-LOC-9, LOC-11, OENC-2, IICV-15, RFC4302-LOC-9, LOC-11, LOC-13, RFC4301-SA-50; covers RFC4301-SA-41, TUN-2, TUN-3, TUN-11, TUN-22, TUN-23, RFC4302-LOC-12, LOC-14, RFC4303-LOC-12, LOC-13 |
| [AH and ESP on one packet](checks/modes.md#ah-and-esp-on-one-packet) | `checks/modes.md` | RFC4301-SA-4, RFC4303-LOC-4; covers RFC4301-SA-3, COMB-1, RFC4302-REAS-1 |
| [Fragments of a protected datagram](checks/fragmentation.md#fragments-of-a-protected-datagram) | `checks/fragmentation.md` | RFC4301-IN-5, RFC4302-FRAG-1, REAS-2, RFC4303-FRAG-1, FRAG-2, REAS-1, RFC4301-SA-44, RFC4302-FRAG-2, RFC4301-SA-45, SA-46, RFC4303-FRAG-3; covers RFC4302-FRAG-3, OICV-14, REAS-5, RFC4303-REAS-5 |
| [Path MTU of a protected flow](checks/fragmentation.md#path-mtu-of-a-protected-flow) | `checks/fragmentation.md` | RFC4301-OUT-21, RFC4302-FRAG-10, RFC4303-FRAG-9; covers RFC4301-OUT-5, SADI-22, RFC4302-FRAG-9, RFC4303-FRAG-8 |

## Statements this pass wrote no check for

Level 2 asks for a core check of every mandatory feature. The checks above give one to every
mandatory feature except two: the named SPD entries need a key management protocol, and the
anti-replay service needs a packet that arrives twice. The statements below have no check in
this pass. This section says **what a check would need**. It says nothing about the simulation
model: whether the absence of a check is acceptable is a judgment about what the model claims,
and that judgment lives in the coverage ledger.

**A crafted, corrupted or replayed packet**, the toolset of level 3: an ICV that fails, a
fragment that reaches AH or ESP, a non-zero Reserved field, a reserved SPI value, wrong padding,
a packet with Next Header 59 that no sender makes, and every rule of the anti-replay service,
which needs a packet that arrives twice; the size of the window is level 4 — RFC4301-OVW-7,
SADI-5, RFC4302-RSV-3, RSV-4, SPI-20, SPI-21, REAS-3, ISEQ-1, ISEQ-2, ISEQ-6 to ISEQ-15, ISEQ-20,
ISEQ-21, ISEQ-23 to ISEQ-29, IICV-3, CONF-4, CONF-5, RFC4303-SPI-18, SEQ-8, PAD-10, REAS-2,
ISEQ-1, ISEQ-2, ISEQ-5, ISEQ-6, ISEQ-8 to ISEQ-17, ISEQ-20 to ISEQ-23, ISEQ-26 to ISEQ-31,
IICV-3, IICV-12, IICV-13, IICV-18, IICV-22, CONF-4, CONF-5.

**A key management protocol**, level 5 by the standards map: the creation of an SA when a packet
needs one, the populate-from-packet flags, the named SPD entries of a responder, the Peer
Authorization Database, the IKE traffic that bypasses IPsec, the INVALID_SELECTORS notification,
the byte counters of an SA lifetime that two peers keep, and every option that the two peers
negotiate — RFC4301-OVW-11, OVW-12, SA-21, DB-4, SPD-21, SPD-29 to SPD-31, SPD-40, SPD-41, SEL-22
to SEL-29, SPDE-1, SPDE-2, SPDE-5 to SPDE-10, SADI-13, PFP-1 to PFP-33, OUT-11, OUT-12, IN-11,
IN-36 to IN-38, RFC4302-FMT-4, SPI-17, ISA-8, RFC4303-FMT-18, SPI-15, TFC-7, ISEQ-7.

**Multicast**, an optional feature, for a later level 2 pass with a multicast mockup: group SAs,
their lookup by destination and source, SPI collisions between a group SA and a unicast SA, and
the sequence numbers of an SA with several senders — RFC4301-SA-8 to SA-12, SA-22, SA-23, SEL-6,
SAD-4, SAD-9 to SAD-13, IN-16, RFC4302-SPI-6 to SPI-9, SPI-18, SPI-19, SEQ-3, SEQ-4, OSEQ-10,
ISA-3, ISEQ-3 to ISEQ-5, CONF-3, RFC4303-SPI-5 to SPI-9, SPI-16, SPI-17, SEQ-3, SEQ-4, OSEQ-11,
ISA-3, ISEQ-3, ISEQ-4, CONF-3.

**A security gateway, or a bump-in-the-stack or bump-in-the-wire implementation**, a larger
mockup: the rules of a gateway and of tunnel mode beyond one SA between two hosts: the outer
header fields of §5.1.2 (DS, ECN, TTL, DF, flow label, options), an IPv6 tunnel, the separate
contexts of a gateway, transport mode at a gateway, fragments on the protected side and the
OPAQUE selector values that they need, and the fragmentation before IPsec that a gateway may do —
RFC4301-OVW-3, SA-27, SA-31 to SA-33, SA-40, SA-42, SA-43, SA-48, SA-49, SA-51, FUNC-6, FUNC-7,
DB-5 to DB-11, SPD-27, SPD-28, SEL-3, SEL-13 to SEL-15, SEL-17, SEL-21, SPDE-14 to SPDE-16,
SPDE-18 to SPDE-20, SADI-16 to SADI-18, SADI-21, SADI-23, OUT-17 to OUT-20, TUN-4 to TUN-10,
TUN-12 to TUN-18, TUN-20, TUN-21, TUN-24, TUN-25, TUN-27 to TUN-33, RFC4302-LOC-8, LOC-10,
LOC-15, OSA-2, FRAG-4 to FRAG-7, RFC4303-FMT-12, LOC-8, LOC-10, FRAG-4 to FRAG-6.

**IPv4 options and IPv6 extension headers**, a larger mockup: a packet that carries them, to see
where AH or ESP goes among them, how the AH ICV treats each one, and which headers the selectors
skip to find the next layer protocol — RFC4301-SA-36, FUNC-3, SEL-9 to SEL-11, SEL-16,
RFC4302-LOC-6, OICV-8 to OICV-10, OICV-12, OICV-15, OICV-16, OICV-18, OICV-20 to OICV-24,
RFC4303-LOC-7.

**The algorithm itself**, level 5 by the standards map (RFC 8221 and the algorithm documents):
the transform of the cipher and of the integrity algorithm, their keys and IV, the implicit
padding of the ICV computation, the order of the operations inside the sender and the receiver,
and what the ciphertext and the ICV cover byte by byte, which a test can only see through an ICV
that fails — RFC4301-OVW-10, FUNC-1, FUNC-2, FUNC-4, FUNC-5, SADI-6 to SADI-9, RFC4302-ICV-5,
LOC-4, ALG-2, OICV-4, OICV-31 to OICV-37, IICV-8, CONF-6, CONF-7, RFC4303-FMT-4, FMT-9, FMT-10,
FMT-17, PAD-7, ICV-2, LOC-5, ALG-1, ALG-4, ALG-8, ALG-10 to ALG-12, OENC-5 to OENC-8, OENC-11 to
OENC-18, IICV-6, IICV-7, IICV-9, IICV-10, IICV-16, IICV-19 to IICV-21, IICV-25 to IICV-27,
CONF-6, CONF-7.

**Extended sequence numbers**, an optional feature that key management negotiates: the 64-bit
counter, its high-order bits in the ICV, and the reconstruction of those bits at the receiver —
RFC4301-SPDE-17, RFC4302-SEQ-11 to SEQ-15, OSEQ-11, OSEQ-12, OICV-29, OICV-30, ISEQ-16 to
ISEQ-19, IICV-7, RFC4303-SEQ-10 to SEQ-16, OENC-10, OSEQ-12, OSEQ-13, ISEQ-18, ISEQ-19.

**A sequence counter near 2^32**, the toolset of level 4: the rule that the counter never cycles
while anti-replay is on, the new SA before it does, and the roll-over when anti-replay is off —
RFC4301-SADI-3, RFC4302-SEQ-9, SEQ-10, OSEQ-3, OSEQ-4, OSEQ-7, OSEQ-8, RFC4303-SEQ-9, OSEQ-3,
OSEQ-6 to OSEQ-9.

**State inside a node, or an audit log**, the toolset of level 4: the structure of the SPD, its
caches and the SAD, the decorrelation of the SPD, and every auditable event with the fields of
its log entry — RFC4301-SA-19, FUNC-10 to FUNC-12, DB-2, DB-3, SPD-1, SPD-39, SPD-43, SPDE-22,
SAD-2, SAD-7, SAD-8, SADI-4, SADI-12, OUT-4, DISC-2, DISC-3, IN-18, IN-19, IN-26, IN-27, IN-34,
IN-35, RFC4302-OSA-3, OSEQ-5, OSEQ-6, REAS-4, ISA-7, ISEQ-22, IICV-4, RFC4303-OSA-2, OSEQ-4,
OSEQ-5, REAS-3, REAS-4, ISA-7, ISA-8, ISEQ-24, ISEQ-25, IICV-4, IICV-5, IICV-23, IICV-24.

**The ICMP report of an outbound discard**, an optional feature, for a later level 2 pass: a
check that reads the ICMP message and its code for each reason of a discard, and the control and
the rate limit of a gateway — RFC4301-DISC-1, DISC-4 to DISC-11.

**Nested SAs**, an optional feature, for a later level 2 pass: a packet that crosses the IPsec
boundary twice, with the SPD entries that let it through the second time — RFC4301-COMB-2, OUT-14
to OUT-16, IN-40, IN-41.

**ICMP error messages at the IPsec boundary**, RFC 4301 §6, outside the in-scope set: the
processing of an ICMP error that reaches the unprotected side — RFC4301-IN-28 to IN-30.

**A host with more than one SPD, interface or address**, a larger mockup: the SPD selection
function, the interface tag of an inbound packet, and a SAD lookup with several local addresses —
RFC4301-OVW-4, SPD-3 to SPD-5, IN-7, IN-9, IN-12.

**A change of the SPD while the system runs**, a later level 2 pass: an SPD entry that changes
after an SA exists, and the action that the administrator chose for such an SA — RFC4301-SPD-44,
SPD-45.

**A permission or a description that no observation can fail**, no check: a may that leaves the
choice to the implementation, a local control, and a sentence that describes the protocol without
a behavior to observe; the checks above use several of them in their mockups — RFC4301-SPD-24,
SPD-25, SPD-33, SPD-36, SPD-37, NLP-1, NLP-3 to NLP-6, RFC4302-FRAG-8, RFC4303-PAD-3, TFC-1,
TFC-8, LOC-1, FRAG-7.
