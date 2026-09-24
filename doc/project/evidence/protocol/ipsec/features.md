# IPsec — feature map

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `IPSEC-F-*` · **Stands on:** [standards.md](standards.md), [rfc4301/catalog.md](../../standard/rfc4301/catalog.md), [rfc4302/catalog.md](../../standard/rfc4302/catalog.md), [rfc4303/catalog.md](../../standard/rfc4303/catalog.md)

Step 4 artifact of the standards test workflow. The catalogs are flat, fine-grained, and per
document. This document is the high-level view above them: the capabilities that the
in-scope set of [`standards.md`](standards.md#in-scope-set) defines, the requirement level of
each, and the cross reference into the catalogs.

The map reads in two directions:

- **Down** — which checks tell whether a feature works. A `core` check is one the feature
  cannot work without. A `supporting` check is a detail or an edge case.
- **Up** — which features the simulation model supports. Step 7 writes the answer into
  [`coverage.md`](../../model/ipsec/coverage.md), and step 8 compares it with the claims of the
  model in [`conformance.md`](../../model/ipsec/conformance.md).

The feature list comes from the standard texts only. The support of each feature — what the
run actually showed — is **not** in this document. It lives in the coverage ledger,
[`coverage.md`](../../model/ipsec/coverage.md).

## How this map decides the level

The rule of the guide: `mandatory` when any core statement says `must` or `shall`, or when the
mechanism is the only path the document gives to a state or an outcome; `optional` when every
core statement says `may` or `should`; `unstated` otherwise.

One refinement, which the DHCP, RIP, ND and IGMP maps state too: **a conditional keyword does
not raise the level of a feature.** RFC 4301 says that an implementation that supports multicast
"MUST" support multicast SAs; the rule holds only once an implementation supports multicast,
which the in-scope set does not require. The level comes from the statement that decides whether
the mechanism has to exist at all, and each feature names it.

A second refinement, new in this map: **the base document can make a whole companion optional.**
RFC 4301 says that an implementation "MUST support ESP" and "MAY support AH" (RFC4301-OVW-5),
and RFC 4302 makes every rule of AH a must for an implementation that offers AH (RFC4302-CONF-1).
The two AH features are therefore `optional`, and each of their rules is a must once AH is there.

A statement can be in more than one feature, when two features need it: RFC4301-SA-50, the rule
that a host supports both modes, is core to the transport mode and to the tunnel mode.

## Index

| ID | Feature |
| --- | --- |
| [IPSEC-F-ARCHITECTURE](#ipsec-f-architecture) | An IPsec implementation supports ESP, and may support AH, to protect traffic between hosts and security gateways. |
| [IPSEC-F-SECURITY-ASSOCIATION](#ipsec-f-security-association) | A security association is simplex, uses AH or ESP but not both, and is identified at the receiver by its SPI; two-way traffic needs a pair of SAs. |
| [IPSEC-F-SPD-PROCESSING](#ipsec-f-spd-processing) | Every packet that crosses the IPsec boundary is matched against the ordered SPD and gets PROTECT, BYPASS or DISCARD; a packet that matches no entry is discarded. |
| [IPSEC-F-SPD-MANAGEMENT](#ipsec-f-spd-management) | An administrator creates, orders and changes the entries of the SPD through a management interface, with every standard selector and action. |
| [IPSEC-F-SELECTORS](#ipsec-f-selectors) | An SPD entry selects traffic by local and remote address ranges, next layer protocol, local and remote ports, and ICMP type and code, with the values ANY and OPAQUE. |
| [IPSEC-F-NAMED-SPD-ENTRIES](#ipsec-f-named-spd-entries) | An SPD entry can carry a name, a user or system identity, that a responder uses for access control where an address does not fit. |
| [IPSEC-F-SA-CREATION](#ipsec-f-sa-creation) | Key management creates the SAs of a PROTECT entry, with selector values from the entry or from the packet, and replaces each SA at the end of its lifetime. |
| [IPSEC-F-SA-LOOKUP](#ipsec-f-sa-lookup) | The receiver finds the SA of an inbound AH or ESP packet in the SAD by its SPI, and discards a packet for which no SA exists. |
| [IPSEC-F-INBOUND-SELECTOR-CHECK](#ipsec-f-inbound-selector-check) | After AH or ESP processing, the receiver checks the packet against the selectors of the SA it arrived on, and discards a packet that does not match. |
| [IPSEC-F-TRANSPORT-MODE](#ipsec-f-transport-mode) | In transport mode the AH or ESP header sits after the IP header and its options or extension headers, and before the next layer protocol header. |
| [IPSEC-F-TUNNEL-MODE](#ipsec-f-tunnel-mode) | In tunnel mode an outer IP header carries the tunnel endpoints and the AH or ESP header, and the inner IP header keeps the ultimate addresses; hosts and gateways support it. |
| [IPSEC-F-PARALLEL-SAS](#ipsec-f-parallel-sas) | A sender can keep several SAs with the same selectors, for example one for each class of service, and the receiver processes each of them the same way. |
| [IPSEC-F-SA-COMBINATION](#ipsec-f-sa-combination) | Two SAs can protect one packet: in transport mode AH then covers the ESP packet, and a nested SA passes the packet through the IPsec boundary again. |
| [IPSEC-F-FRAGMENTATION](#ipsec-f-fragmentation) | IPsec processes whole IP datagrams: the sender fragments after AH or ESP, and the receiver reassembles before, and discards a fragment that reaches AH or ESP. |
| [IPSEC-F-PATH-MTU](#ipsec-f-path-mtu) | The IPsec system keeps the path MTU of each SA and reports it to the sender of a packet that is too large for the SA. |
| [IPSEC-F-OUTBOUND-DISCARD-REPORT](#ipsec-f-outbound-discard-report) | An IPsec system that discards an outbound packet can tell the sender with an ICMP message whose code gives the reason. |
| [IPSEC-F-AUDIT](#ipsec-f-audit) | The IPsec system records an auditable event, with the SPI, the addresses and the time, for each packet it discards and for each sequence number overflow. |
| [IPSEC-F-AH-FORMAT](#ipsec-f-ah-format) | The AH header carries Next Header, Payload Length, a zero Reserved field, the SPI, the Sequence Number and the ICV, in this order, after protocol number 51. |
| [IPSEC-F-AH-INTEGRITY](#ipsec-f-ah-integrity) | The AH ICV covers the fields of the IP header that do not change in transit, the AH header with a zero ICV field, and the whole payload; the receiver verifies it and discards a packet that fails. |
| [IPSEC-F-ESP-FORMAT](#ipsec-f-esp-format) | An ESP packet carries the SPI, the Sequence Number, the Payload Data, the Padding, the Pad Length, the Next Header and an optional ICV, after protocol number 50. |
| [IPSEC-F-ESP-PADDING](#ipsec-f-esp-padding) | The sender pads the ESP payload to the block size of the cipher, so that Pad Length and Next Header end on a 4-octet boundary, and fills the padding with 1, 2, 3 and so on. |
| [IPSEC-F-ESP-SERVICES](#ipsec-f-esp-services) | An ESP SA gives confidentiality, integrity, or both, with separate algorithms or a combined one; the encryption and the integrity algorithm are never both NULL. |
| [IPSEC-F-ESP-PROCESSING](#ipsec-f-esp-processing) | The ESP sender encrypts and then computes the ICV; the receiver verifies the ICV before it decrypts, discards a packet that fails, and restores the original datagram. |
| [IPSEC-F-SEQUENCE-NUMBERS](#ipsec-f-sequence-numbers) | The sender numbers the packets of each SA from 1, one by one, in a 32-bit field that is always present, and never lets the number cycle while anti-replay is on. |
| [IPSEC-F-ANTI-REPLAY](#ipsec-f-anti-replay) | A receiver can check each Sequence Number against a sliding window of at least 32 packets, and discards a duplicate or a packet left of the window. |
| [IPSEC-F-EXTENDED-SEQUENCE-NUMBERS](#ipsec-f-extended-sequence-numbers) | An SA can use 64-bit sequence numbers, of which only the low-order 32 bits are sent and the high-order 32 bits enter the ICV. |
| [IPSEC-F-ESP-TFC-PADDING](#ipsec-f-esp-tfc-padding) | An ESP sender can add traffic flow confidentiality padding after the payload, when the payload carries its own length, and the receiver drops it. |
| [IPSEC-F-ESP-DUMMY-PACKETS](#ipsec-f-esp-dummy-packets) | An ESP sender can send dummy packets with Next Header 59, and the receiver discards them without an error. |
| [IPSEC-F-MULTICAST](#ipsec-f-multicast) | An implementation that supports multicast keeps group SAs, found by SPI and destination, and also source, and tells them apart from unicast SAs with the same SPI. |

## Summary table

| Feature | Level | Sources | Core checks |
| --- | --- | --- | --- |
| [IPSEC-F-ARCHITECTURE](#ipsec-f-architecture) | mandatory | RFC 4301 §3.1, §3.2, §4.2, §4.4, §10; RFC 4303 §5 | RFC4301-OVW-2, OVW-5, CONF-1, CONF-2, DB-1, RFC4303-CONF-1, CONF-2 |
| [IPSEC-F-SECURITY-ASSOCIATION](#ipsec-f-security-association) | mandatory | RFC 4301 §4.1, §4.4.2; RFC 4302 §2.4; RFC 4303 §2.1 | RFC4301-SA-1, SA-2, SA-3, SA-5, SA-6, SAD-1, SADI-1, RFC4302-SPI-1, SPI-2, SPI-5, RFC4303-SPI-1, SPI-2, SPI-3, SPI-4 |
| [IPSEC-F-SPD-PROCESSING](#ipsec-f-spd-processing) | mandatory | RFC 4301 §4.4.1, §5, §5.1, §5.2 | RFC4301-PROC-1, PROC-2, SPD-6, SPD-8, SPD-9, SPD-10, SPD-11, SPD-13, SPD-42, OUT-3, IN-4, IN-13, IN-22, IN-25 |
| [IPSEC-F-SPD-MANAGEMENT](#ipsec-f-spd-management) | mandatory | RFC 4301 §4.4, §4.4.1, §4.4.1.2 | RFC4301-SPD-2, SPD-18, SPD-19, SPD-20, SPD-32, SPD-34, SPD-35, SPD-38 |
| [IPSEC-F-SELECTORS](#ipsec-f-selectors) | mandatory | RFC 4301 §4.4.1, §4.4.1.1, §4.4.1.2, §4.4.1.3 | RFC4301-SEL-1, SEL-4, SEL-5, SEL-7, SEL-8, SEL-12, SEL-13, SEL-18, SEL-19, SEL-20, SPD-26, SPD-27, SPDE-11 |
| [IPSEC-F-NAMED-SPD-ENTRIES](#ipsec-f-named-spd-entries) | mandatory | RFC 4301 §4.4.1.1, §4.4.1.2 | RFC4301-SEL-24, SPDE-7 |
| [IPSEC-F-SA-CREATION](#ipsec-f-sa-creation) | mandatory | RFC 4301 §4.4.1.2, §4.4.2, §4.4.2.1, §4.4.2.2, §5.1 | RFC4301-OUT-8, OUT-9, OUT-10, SAD-5, SADI-10, SADI-11 |
| [IPSEC-F-SA-LOOKUP](#ipsec-f-sa-lookup) | mandatory | RFC 4301 §4.1, §4.4.2, §5.2; RFC 4302 §2.4, §3.4.2; RFC 4303 §2.1, §3.4.2 | RFC4301-SA-13, SA-18, SAD-3, IN-14, IN-15, IN-17, RFC4302-ISA-1, ISA-2, ISA-6, SPI-10, SPI-15, RFC4303-ISA-1, ISA-2, ISA-6, SPI-10, SPI-13 |
| [IPSEC-F-INBOUND-SELECTOR-CHECK](#ipsec-f-inbound-selector-check) | mandatory | RFC 4301 §4.4.2, §5.2 | RFC4301-IN-31, IN-32, IN-33, SAD-6 |
| [IPSEC-F-TRANSPORT-MODE](#ipsec-f-transport-mode) | mandatory | RFC 4301 §4.1; RFC 4302 §3.1.1, §3.3; RFC 4303 §3.1.1, §3.3, §3.4.4 | RFC4301-SA-34, SA-35, SA-37, SA-50, RFC4302-LOC-2, LOC-3, LOC-5, LOC-7, OSA-1, RFC4303-LOC-2, LOC-3, LOC-6, OSA-1, OENC-1, IICV-14 |
| [IPSEC-F-TUNNEL-MODE](#ipsec-f-tunnel-mode) | mandatory | RFC 4301 §4.1, §5.1.2; RFC 4302 §3.1.2; RFC 4303 §3.1.2, §3.3, §3.4.4 | RFC4301-SA-47, SA-50, SA-51, TUN-1, TUN-11, TUN-19, TUN-26, TUN-31, RFC4302-LOC-9, LOC-11, LOC-12, LOC-13, LOC-14, LOC-15, RFC4303-LOC-9, LOC-11, LOC-12, LOC-13, OENC-2, IICV-15 |
| [IPSEC-F-PARALLEL-SAS](#ipsec-f-parallel-sas) | mandatory | RFC 4301 §4.1, §4.4.2.1 | RFC4301-SA-25, SA-26, SA-28 |
| [IPSEC-F-SA-COMBINATION](#ipsec-f-sa-combination) | optional | RFC 4301 §4.1, §4.3, §5.1, §5.2; RFC 4302 §3.4.1; RFC 4303 §3.1.1 | RFC4301-SA-4, RFC4303-LOC-4 |
| [IPSEC-F-FRAGMENTATION](#ipsec-f-fragmentation) | mandatory | RFC 4301 §4.1, §5.1, §5.2; RFC 4302 §3.3.4, §3.4.1; RFC 4303 §3.3.4, §3.4.1 | RFC4301-IN-5, SA-44, RFC4302-FRAG-1, FRAG-2, REAS-2, REAS-3, RFC4303-FRAG-1, FRAG-2, FRAG-3, REAS-1, REAS-2 |
| [IPSEC-F-PATH-MTU](#ipsec-f-path-mtu) | mandatory | RFC 4301 §4.4.2.1, §5.1; RFC 4302 §3.3.4; RFC 4303 §3.3.4 | RFC4301-OUT-21, RFC4302-FRAG-10, RFC4303-FRAG-9 |
| [IPSEC-F-OUTBOUND-DISCARD-REPORT](#ipsec-f-outbound-discard-report) | optional | RFC 4301 §5.1.1 | RFC4301-DISC-1, DISC-4, DISC-5 |
| [IPSEC-F-AUDIT](#ipsec-f-audit) | unstated | RFC 4301 §4.1, §4.2, §4.4.2.1, §5.1.1, §5.2; RFC 4302 §3.3.2, §3.4; RFC 4303 §3.3.3, §3.4 | RFC4301-SA-19, IN-18, IN-26, IN-34 |
| [IPSEC-F-AH-FORMAT](#ipsec-f-ah-format) | optional | RFC 4302 §2, §3.3.3.2.1, §5 | RFC4302-FMT-1, FMT-2, NH-2, LEN-1, RSV-2, SEQ-5, SEQ-7, ICV-1, ICV-2, ICV-4 |
| [IPSEC-F-AH-INTEGRITY](#ipsec-f-ah-integrity) | optional | RFC 4302 §3.2, §3.3.3, §3.4.4, §5 | RFC4302-OICV-1, OICV-2, OICV-3, OICV-5, OICV-7, IICV-1, IICV-2, IICV-3, ISEQ-21 |
| [IPSEC-F-ESP-FORMAT](#ipsec-f-esp-format) | mandatory | RFC 4303 §2 | RFC4303-FMT-1, FMT-2, FMT-3, SPI-2, SEQ-5, PAY-2, PADL-1, PADL-3, NH-1, ICV-3, ICV-4 |
| [IPSEC-F-ESP-PADDING](#ipsec-f-esp-padding) | mandatory | RFC 4303 §2.4, §3.3.2, §3.4.4 | RFC4303-PAD-1, PAD-2, PAD-5, PAD-6, PAD-8, PAD-9 |
| [IPSEC-F-ESP-SERVICES](#ipsec-f-esp-services) | mandatory | RFC 4301 §3.2, §4.2; RFC 4303 §2, §3.2, §5 | RFC4303-ALG-2, ALG-5, ALG-6, CONF-8, CONF-10, CONF-11, FMT-5, FMT-6, RFC4301-FUNC-9 |
| [IPSEC-F-ESP-PROCESSING](#ipsec-f-esp-processing) | mandatory | RFC 4303 §3.3, §3.4.3, §3.4.4 | RFC4303-OENC-4, OENC-7, OENC-9, IICV-1, IICV-2, IICV-3, IICV-8, IICV-18, ISEQ-23 |
| [IPSEC-F-SEQUENCE-NUMBERS](#ipsec-f-sequence-numbers) | mandatory | RFC 4301 §4.4.2.1; RFC 4302 §2.5, §3.3.2; RFC 4303 §2.2, §3.3.3 | RFC4301-SADI-2, RFC4302-SEQ-1, SEQ-2, SEQ-8, OSEQ-1, OSEQ-2, OSEQ-4, RFC4303-SEQ-1, SEQ-2, SEQ-7, OSEQ-1, OSEQ-3 |
| [IPSEC-F-ANTI-REPLAY](#ipsec-f-anti-replay) | mandatory | RFC 4301 §3.2, §4.4.2.1; RFC 4302 §3.4.3, §5; RFC 4303 §3.4.3, §5 | RFC4301-SADI-5, RFC4302-ISEQ-1, ISEQ-9, ISEQ-10, ISEQ-24, RFC4303-ISEQ-1, ISEQ-2, ISEQ-8, ISEQ-9, ISEQ-28 |
| [IPSEC-F-EXTENDED-SEQUENCE-NUMBERS](#ipsec-f-extended-sequence-numbers) | optional | RFC 4301 §4.4.1.2; RFC 4302 §2.5.1, §3.3.2, §3.3.3.2.2, §3.4.3; RFC 4303 §2.2.1, §3.3.2, §3.3.3, §3.4.3 | RFC4302-SEQ-11, RFC4303-SEQ-10 |
| [IPSEC-F-ESP-TFC-PADDING](#ipsec-f-esp-tfc-padding) | optional | RFC 4303 §2, §2.7, §3.4.4 | RFC4303-TFC-2, TFC-5 |
| [IPSEC-F-ESP-DUMMY-PACKETS](#ipsec-f-esp-dummy-packets) | mandatory | RFC 4303 §2.6, §3.4.4 | RFC4303-NH-2, NH-3, NH-4, NH-5 |
| [IPSEC-F-MULTICAST](#ipsec-f-multicast) | optional | RFC 4301 §4.1, §4.4.1.1, §4.4.2; RFC 4302 §2.4, §3.4.2, §5; RFC 4303 §2.1, §3.4.2, §5 | RFC4301-SA-8, SA-10, RFC4302-SPI-6, SPI-7, RFC4303-SPI-5, SPI-7 |

## IPSEC-F-ARCHITECTURE

**An IPsec implementation supports ESP, and may support AH, to protect traffic between hosts and security gateways.**

- **Sources** — RFC 4301 §3.1, §3.2, §4.2, §4.4, §10; RFC 4303 §5.
- **Level** — mandatory (reason: keyword). RFC4301-OVW-2 has the strength "must".
- **Description** — A host supports host-to-host and gateway-to-host connectivity; a security gateway also supports gateway-to-gateway. AH gives integrity and data origin authentication, ESP also confidentiality. An implementation may structure its databases as it likes, but its outside behavior matches the model of the architecture. Every IPv4 and IPv6 implementation complies with the whole architecture, and an ESP implementation with the whole of RFC 4303.
- **Checks** — core: RFC4301-OVW-2, OVW-5, CONF-1, CONF-2, DB-1, RFC4303-CONF-1, CONF-2. Supporting: RFC4301-OVW-1, OVW-3, OVW-4, OVW-6, OVW-8, OVW-10, DB-2, DB-3, DB-4, FUNC-1, FUNC-2, FUNC-3, FUNC-5.

## IPSEC-F-SECURITY-ASSOCIATION

**A security association is simplex, uses AH or ESP but not both, and is identified at the receiver by its SPI; two-way traffic needs a pair of SAs.**

- **Sources** — RFC 4301 §4.1, §4.4.2; RFC 4302 §2.4; RFC 4303 §2.1.
- **Level** — mandatory (reason: keyword). RFC4301-SA-1 has the strength "must".
- **Description** — The receiver chooses the SPI of a unicast SA, a 32-bit value; the SPI alone, or the SPI and the protocol, identifies the SA. The SAD holds one entry for each SA, with its SPI, its algorithms, its mode and its sequence counter. The SPI values 1 to 255 are reserved, and zero is never sent.
- **Checks** — core: RFC4301-SA-1, SA-2, SA-3, SA-5, SA-6, SAD-1, SADI-1, RFC4302-SPI-1, SPI-2, SPI-5, RFC4303-SPI-1, SPI-2, SPI-3, SPI-4. Supporting: RFC4301-SA-7, SAD-2, SAD-7, SAD-8, SADI-6, SADI-7, SADI-8, SADI-9, SADI-16, SADI-17, RFC4302-SPI-3, SPI-4.

## IPSEC-F-SPD-PROCESSING

**Every packet that crosses the IPsec boundary is matched against the ordered SPD and gets PROTECT, BYPASS or DISCARD; a packet that matches no entry is discarded.**

- **Sources** — RFC 4301 §4.4.1, §5, §5.1, §5.2.
- **Level** — mandatory (reason: keyword). RFC4301-PROC-1 has the strength "must".
- **Description** — The search is ordered and the first matching entry decides. Outbound, the SPD-S and SPD-O parts decide; inbound, unprotected traffic is matched against SPD-I, and IPsec-protected traffic addressed to the node goes to the SAD instead. The SPD should end with an entry that discards everything else. A PROTECT entry names the protocol, the mode and the algorithms of the protection.
- **Checks** — core: RFC4301-PROC-1, PROC-2, SPD-6, SPD-8, SPD-9, SPD-10, SPD-11, SPD-13, SPD-42, OUT-3, IN-4, IN-13, IN-22, IN-25. Supporting: RFC4301-SPD-1, SPD-7, SPD-12, SPD-14, SPD-15, SPD-16, SPD-17, SPD-25, SPDE-13, PROC-3, OUT-1, OUT-2, OUT-4, OUT-6, OUT-7, OUT-13, IN-1, IN-2, IN-3, IN-10, IN-11, IN-21, IN-23, IN-24, IN-28, IN-29, IN-30, RFC4302-OSA-3.

## IPSEC-F-SPD-MANAGEMENT

**An administrator creates, orders and changes the entries of the SPD through a management interface, with every standard selector and action.**

- **Sources** — RFC 4301 §4.4, §4.4.1, §4.4.1.2.
- **Level** — mandatory (reason: keyword). RFC4301-SPD-2 has the strength "must".
- **Description** — Every implementation has at least one SPD and may have several, with a selection function between them. The interface lets an administrator give BYPASS entries for each direction and PROTECT entries with their SA controls, orders all entries, and decides whether a user or an application may add entries. A change of the SPD at run time should be checked against the SAs that exist.
- **Checks** — core: RFC4301-SPD-2, SPD-18, SPD-19, SPD-20, SPD-32, SPD-34, SPD-35, SPD-38. Supporting: RFC4301-SPD-3, SPD-4, SPD-5, SPD-33, SPD-36, SPD-37, SPD-39, SPD-40, SPD-41, SPD-43, SPD-44, SPD-45, SPDE-1, SPDE-2, SPDE-21, SPDE-22, DB-5, DB-6, DB-7, DB-8, IN-7, IN-12.

## IPSEC-F-SELECTORS

**An SPD entry selects traffic by local and remote address ranges, next layer protocol, local and remote ports, and ICMP type and code, with the values ANY and OPAQUE.**

- **Sources** — RFC 4301 §4.4.1, §4.4.1.1, §4.4.1.2, §4.4.1.3.
- **Level** — mandatory (reason: keyword). RFC4301-SEL-1 has the strength "must".
- **Description** — The next layer protocol is the protocol after the extension headers that the implementation skips, and AH and ESP count as next layer protocols. A port selector supports OPAQUE for a fragment or an encrypted header; an entry that names a port cannot match a non-initial fragment. The ICMP selector matches a range of types and a range of codes.
- **Checks** — core: RFC4301-SEL-1, SEL-4, SEL-5, SEL-7, SEL-8, SEL-12, SEL-13, SEL-18, SEL-19, SEL-20, SPD-26, SPD-27, SPDE-11. Supporting: RFC4301-SEL-2, SEL-3, SEL-6, SEL-9, SEL-10, SEL-11, SEL-14, SEL-15, SEL-16, SEL-17, SEL-21, SPD-22, SPD-23, SPD-24, SPD-28, SPDE-3, SPDE-4, SPDE-5, SPDE-12, NLP-1, NLP-2, NLP-3, NLP-4, NLP-5, NLP-6, DB-9, DB-10, DB-11, OUT-19, OUT-20.

## IPSEC-F-NAMED-SPD-ENTRIES

**An SPD entry can carry a name, a user or system identity, that a responder uses for access control where an address does not fit.**

- **Sources** — RFC 4301 §4.4.1.1, §4.4.1.2.
- **Level** — mandatory (reason: keyword). RFC4301-SEL-24 has the strength "must".
- **Description** — The name is not a field of a packet. A responder matches it against the identity that the key management protocol authenticates; an initiator may use it to name the user of an SA.
- **Checks** — core: RFC4301-SEL-24, SPDE-7. Supporting: RFC4301-SEL-22, SEL-23, SEL-25, SEL-26, SEL-27, SEL-28, SEL-29, SPDE-6.

## IPSEC-F-SA-CREATION

**Key management creates the SAs of a PROTECT entry, with selector values from the entry or from the packet, and replaces each SA at the end of its lifetime.**

- **Sources** — RFC 4301 §4.4.1.2, §4.4.2, §4.4.2.1, §4.4.2.2, §5.1.
- **Level** — mandatory (reason: keyword). RFC4301-SAD-5 has the strength "must".
- **Description** — A packet that matches a PROTECT entry without an SA starts key management; the packet is discarded when the SA cannot be created. The populate-from-packet flag of each selector decides whether the SA takes the value of the entry or of the packet. Each SA has a lifetime in time, in bytes, or both, with a soft and a hard limit.
- **Checks** — core: RFC4301-OUT-8, OUT-9, OUT-10, SAD-5, SADI-10, SADI-11. Supporting: RFC4301-OUT-11, OUT-12, SPD-21, SPD-29, SPD-30, SPD-31, SPDE-8, SPDE-9, SPDE-10, PFP-1, PFP-2, PFP-3, PFP-4, PFP-5, PFP-6, PFP-7, PFP-8, PFP-9, PFP-10, PFP-11, PFP-12, PFP-13, PFP-14, PFP-15, PFP-16, PFP-17, PFP-18, PFP-19, PFP-20, PFP-21, PFP-22, PFP-23, PFP-24, PFP-25, PFP-26, PFP-27, PFP-28, PFP-29, PFP-30, PFP-31, PFP-32, PFP-33, SADI-12, SADI-13, SADI-14, SADI-15, OVW-11, OVW-12, SA-21, RFC4302-FMT-4, SPI-17, RFC4303-FMT-18, SPI-15.

## IPSEC-F-SA-LOOKUP

**The receiver finds the SA of an inbound AH or ESP packet in the SAD by its SPI, and discards a packet for which no SA exists.**

- **Sources** — RFC 4301 §4.1, §4.4.2, §5.2; RFC 4302 §2.4, §3.4.2; RFC 4303 §2.1, §3.4.2.
- **Level** — mandatory (reason: keyword). RFC4301-SA-13 has the strength "must (lower case)".
- **Description** — A unicast SA is found by the SPI, or the SPI and the protocol; the search takes the longest match of SPI, destination and source. The SA gives the algorithms, the keys and the sequence number processing of the packet.
- **Checks** — core: RFC4301-SA-13, SA-18, SAD-3, IN-14, IN-15, IN-17, RFC4302-ISA-1, ISA-2, ISA-6, SPI-10, SPI-15, RFC4303-ISA-1, ISA-2, ISA-6, SPI-10, SPI-13. Supporting: RFC4301-SA-14, SA-15, SA-16, SA-17, SA-20, IN-6, IN-8, IN-9, IN-20, RFC4302-SPI-11, SPI-12, SPI-13, SPI-14, SPI-16, ISA-4, ISA-5, ISA-8, RFC4303-SPI-11, SPI-12, SPI-14, ISA-4, ISA-5.

## IPSEC-F-INBOUND-SELECTOR-CHECK

**After AH or ESP processing, the receiver checks the packet against the selectors of the SA it arrived on, and discards a packet that does not match.**

- **Sources** — RFC 4301 §4.4.2, §5.2.
- **Level** — mandatory (reason: keyword). RFC4301-IN-31 has the strength "must".
- **Description** — The SAD entry of an inbound SA holds the selector values that were agreed for it. A packet whose headers are not consistent with them is discarded, and the receiver should be able to tell the sender with an INVALID_SELECTORS notification.
- **Checks** — core: RFC4301-IN-31, IN-32, IN-33, SAD-6. Supporting: RFC4301-IN-36, IN-37, IN-38, IN-39.

## IPSEC-F-TRANSPORT-MODE

**In transport mode the AH or ESP header sits after the IP header and its options or extension headers, and before the next layer protocol header.**

- **Sources** — RFC 4301 §4.1; RFC 4302 §3.1.1, §3.3; RFC 4303 §3.1.1, §3.3, §3.4.4.
- **Level** — mandatory (reason: keyword). RFC4301-SA-37 has the strength "must".
- **Description** — In IPv6 the header comes after the hop-by-hop, routing and fragment headers, and a destination options header may come before or after it. ESP in transport mode protects the next layer protocol only; AH also protects the fields of the IP header that do not change in transit. A host supports transport mode.
- **Checks** — core: RFC4301-SA-34, SA-35, SA-37, SA-50, RFC4302-LOC-2, LOC-3, LOC-5, LOC-7, OSA-1, RFC4303-LOC-2, LOC-3, LOC-6, OSA-1, OENC-1, IICV-14. Supporting: RFC4301-SA-29, SA-30, SA-31, SA-32, SA-33, SA-36, SA-38, SA-39, SA-45, SA-46, RFC4302-LOC-1, LOC-4, LOC-6, LOC-8, RFC4303-LOC-1, LOC-5, LOC-7, LOC-8.

## IPSEC-F-TUNNEL-MODE

**In tunnel mode an outer IP header carries the tunnel endpoints and the AH or ESP header, and the inner IP header keeps the ultimate addresses; hosts and gateways support it.**

- **Sources** — RFC 4301 §4.1, §5.1.2; RFC 4302 §3.1.2; RFC 4303 §3.1.2, §3.3, §3.4.4.
- **Level** — mandatory (reason: keyword). RFC4301-SA-50 has the strength "must".
- **Description** — The encapsulator builds the outer header from the SA: its addresses are the tunnel endpoints, its protocol is AH or ESP, and the inner header is not changed. The outer and inner IP versions can differ. The DS and ECN fields, the TTL and the DF bit follow the table of §5.1.2. An SA that has a security gateway at either end is a tunnel mode SA.
- **Checks** — core: RFC4301-SA-47, SA-50, SA-51, TUN-1, TUN-11, TUN-19, TUN-26, TUN-31, RFC4302-LOC-9, LOC-11, LOC-12, LOC-13, LOC-14, LOC-15, RFC4303-LOC-9, LOC-11, LOC-12, LOC-13, OENC-2, IICV-15. Supporting: RFC4301-SA-40, SA-41, SA-42, SA-43, SA-48, SA-49, TUN-2, TUN-3, TUN-4, TUN-5, TUN-6, TUN-7, TUN-8, TUN-9, TUN-10, TUN-12, TUN-13, TUN-14, TUN-15, TUN-16, TUN-17, TUN-18, TUN-20, TUN-21, TUN-22, TUN-23, TUN-24, TUN-25, TUN-27, TUN-28, TUN-29, TUN-30, TUN-32, TUN-33, SADI-18, SADI-21, SADI-23, SPDE-14, SPDE-15, SPDE-16, SPDE-19, SPDE-20, FUNC-6, FUNC-7, RFC4302-LOC-10, OSA-2, RFC4303-LOC-10.

## IPSEC-F-PARALLEL-SAS

**A sender can keep several SAs with the same selectors, for example one for each class of service, and the receiver processes each of them the same way.**

- **Sources** — RFC 4301 §4.1, §4.4.2.1.
- **Level** — mandatory (reason: keyword). RFC4301-SA-25 has the strength "must".
- **Description** — The DSCP values of an SA choose between SAs that otherwise match an outbound packet; a receiver does not check the DSCP of an inbound packet in transport mode.
- **Checks** — core: RFC4301-SA-25, SA-26, SA-28. Supporting: RFC4301-SA-24, SA-27, SADI-19, SADI-20.

## IPSEC-F-SA-COMBINATION

**Two SAs can protect one packet: in transport mode AH then covers the ESP packet, and a nested SA passes the packet through the IPsec boundary again.**

- **Sources** — RFC 4301 §4.1, §4.3, §5.1, §5.2; RFC 4302 §3.4.1; RFC 4303 §3.1.1.
- **Level** — optional (reason: conditional keyword). The must of RFC4301-SA-4 holds only when a stream needs both AH and ESP, and RFC4301-COMB-1 says that no implementation has to support nested SAs.
- **Description** — A stream that needs AH and ESP gets two SAs, one for each protocol. The processing of one IPsec header ignores the headers that were applied after it. An implementation does not have to support nested SAs; when it does, the SPD lets the looped-back packet through.
- **Checks** — core: RFC4301-SA-4, RFC4303-LOC-4. Supporting: RFC4301-COMB-1, COMB-2, OUT-14, OUT-15, OUT-16, IN-40, IN-41, RFC4302-REAS-1.

## IPSEC-F-FRAGMENTATION

**IPsec processes whole IP datagrams: the sender fragments after AH or ESP, and the receiver reassembles before, and discards a fragment that reaches AH or ESP.**

- **Sources** — RFC 4301 §4.1, §5.1, §5.2; RFC 4302 §3.3.4, §3.4.1; RFC 4303 §3.3.4, §3.4.1.
- **Level** — mandatory (reason: keyword). RFC4301-SA-44 has the strength "must (lower case)".
- **Description** — In transport mode AH and ESP never apply to a fragment. A router may fragment a protected packet, and the receiver reassembles the fragments before the IPsec processing. A packet that still has a fragment offset or the More Fragments flag when it reaches AH or ESP is discarded.
- **Checks** — core: RFC4301-IN-5, SA-44, RFC4302-FRAG-1, FRAG-2, REAS-2, REAS-3, RFC4303-FRAG-1, FRAG-2, FRAG-3, REAS-1, REAS-2. Supporting: RFC4301-OUT-17, OUT-18, SPDE-18, RFC4302-FRAG-3, FRAG-4, FRAG-5, FRAG-6, FRAG-7, FRAG-8, REAS-5, OICV-14, RFC4303-FRAG-4, FRAG-5, FRAG-6, FRAG-7, REAS-5.

## IPSEC-F-PATH-MTU

**The IPsec system keeps the path MTU of each SA and reports it to the sender of a packet that is too large for the SA.**

- **Sources** — RFC 4301 §4.4.2.1, §5.1; RFC 4302 §3.3.4; RFC 4303 §3.3.4.
- **Level** — mandatory (reason: keyword). RFC4301-OUT-21 has the strength "must".
- **Description** — The SA holds the observed path MTU and its aging variables. An AH or ESP implementation can generate ICMP PMTU messages, or the same signal inside a native host, and may set the DF bit on the packets it sends.
- **Checks** — core: RFC4301-OUT-21, RFC4302-FRAG-10, RFC4303-FRAG-9. Supporting: RFC4301-OUT-5, SADI-22, RFC4302-FRAG-9, RFC4303-FRAG-8.

## IPSEC-F-OUTBOUND-DISCARD-REPORT

**An IPsec system that discards an outbound packet can tell the sender with an ICMP message whose code gives the reason.**

- **Sources** — RFC 4301 §5.1.1.
- **Level** — optional (reason: keyword). RFC4301-DISC-1 says should; the codes of RFC4301-DISC-4 and DISC-5 hold once a system sends the message.
- **Description** — The code tells an administrative prohibition by the SPD from a failed negotiation and from an unreachable peer, for IPv4 and for IPv6. A security gateway has a control to turn the messages on and limits their rate.
- **Checks** — core: RFC4301-DISC-1, DISC-4, DISC-5. Supporting: RFC4301-DISC-6, DISC-7, DISC-8, DISC-9, DISC-10, DISC-11.

## IPSEC-F-AUDIT

**The IPsec system records an auditable event, with the SPI, the addresses and the time, for each packet it discards and for each sequence number overflow.**

- **Sources** — RFC 4301 §4.1, §4.2, §4.4.2.1, §5.1.1, §5.2; RFC 4302 §3.3.2, §3.4; RFC 4303 §3.3.3, §3.4.
- **Level** — unstated. The core statements are descriptions or a mix without a must.
- **Description** — The events are: no SA for an inbound packet, no SPD entry for an inbound packet, a failed selector check, a failed ICV check, a fragment offered to AH or ESP, an outbound discard, a sequence number that would cycle, and an attempt to negotiate ESP with NULL encryption and no integrity.
- **Checks** — core: RFC4301-SA-19, IN-18, IN-26, IN-34. Supporting: RFC4301-FUNC-10, FUNC-11, FUNC-12, SADI-3, SADI-4, DISC-2, DISC-3, IN-19, IN-27, IN-35, RFC4302-OSEQ-5, OSEQ-6, REAS-4, ISA-7, ISEQ-22, IICV-4, RFC4303-OSEQ-4, OSEQ-5, REAS-3, REAS-4, ISA-7, ISA-8, ISEQ-24, ISEQ-25, IICV-4, IICV-5, IICV-23, IICV-24.

## IPSEC-F-AH-FORMAT

**The AH header carries Next Header, Payload Length, a zero Reserved field, the SPI, the Sequence Number and the ICV, in this order, after protocol number 51.**

- **Sources** — RFC 4302 §2, §3.3.3.2.1, §5.
- **Level** — optional (reason: base document). RFC4301-OVW-5 lets an implementation leave AH out; once an implementation offers AH, RFC4302-CONF-1 makes the whole of AH a must.
- **Description** — Payload Length is the length of the AH header in 32-bit words minus 2. The ICV field is a multiple of 32 bits, padded to a multiple of 64 bits in IPv6, with no more padding than the alignment needs. Every field is present in every packet.
- **Checks** — core: RFC4302-FMT-1, FMT-2, NH-2, LEN-1, RSV-2, SEQ-5, SEQ-7, ICV-1, ICV-2, ICV-4. Supporting: RFC4302-FMT-3, NH-1, LEN-2, RSV-1, RSV-3, RSV-4, ICV-3, ICV-5, SPI-20, SPI-21, OICV-25, OICV-26, OICV-27, OICV-28, CONF-1, CONF-2.

## IPSEC-F-AH-INTEGRITY

**The AH ICV covers the fields of the IP header that do not change in transit, the AH header with a zero ICV field, and the whole payload; the receiver verifies it and discards a packet that fails.**

- **Sources** — RFC 4302 §3.2, §3.3.3, §3.4.4, §5.
- **Level** — optional (reason: base document). RFC4301-OVW-5 lets an implementation leave AH out; once an implementation offers AH, RFC4302-CONF-1 makes the whole of AH a must.
- **Description** — A field that can change in transit counts as zero, a field whose value at the receiver is known counts with that value. The integrity algorithm of the SA computes the ICV, with implicit zero padding when its block size needs it.
- **Checks** — core: RFC4302-OICV-1, OICV-2, OICV-3, OICV-5, OICV-7, IICV-1, IICV-2, IICV-3, ISEQ-21. Supporting: RFC4302-OICV-4, OICV-6, OICV-8, OICV-9, OICV-10, OICV-11, OICV-12, OICV-13, OICV-15, OICV-16, OICV-17, OICV-18, OICV-19, OICV-20, OICV-21, OICV-22, OICV-23, OICV-24, OICV-31, OICV-32, OICV-33, OICV-34, OICV-35, OICV-36, OICV-37, IICV-5, IICV-6, IICV-8, IICV-9, ALG-1, ALG-2, CONF-6, CONF-7.

## IPSEC-F-ESP-FORMAT

**An ESP packet carries the SPI, the Sequence Number, the Payload Data, the Padding, the Pad Length, the Next Header and an optional ICV, after protocol number 50.**

- **Sources** — RFC 4303 §2.
- **Level** — mandatory (reason: keyword). RFC4303-FMT-1 has the strength "must".
- **Description** — The SPI and the Sequence Number are not encrypted. The Payload Data carries the IV of the cipher when it needs one, and the next layer header starts at an offset that is a multiple of 4 octets in IPv4 and 8 in IPv6. The ICV is present only with the integrity service, and its length comes from the integrity algorithm. The format of an SA stays the same for the life of the SA.
- **Checks** — core: RFC4303-FMT-1, FMT-2, FMT-3, SPI-2, SEQ-5, PAY-2, PADL-1, PADL-3, NH-1, ICV-3, ICV-4. Supporting: RFC4303-FMT-7, FMT-13, FMT-14, FMT-15, FMT-16, FMT-17, PAY-1, PAY-3, PAY-4, PAY-5, SPI-18.

## IPSEC-F-ESP-PADDING

**The sender pads the ESP payload to the block size of the cipher, so that Pad Length and Next Header end on a 4-octet boundary, and fills the padding with 1, 2, 3 and so on.**

- **Sources** — RFC 4303 §2.4, §3.3.2, §3.4.4.
- **Level** — mandatory (reason: keyword). RFC4303-PAD-2 has the strength "must (lower case)".
- **Description** — Every implementation can generate and consume padding, from 0 to 255 octets. The padding counts over the Payload Data without the IV, the Padding, the Pad Length and the Next Header. The receiver should inspect the default padding.
- **Checks** — core: RFC4303-PAD-1, PAD-2, PAD-5, PAD-6, PAD-8, PAD-9. Supporting: RFC4303-PAD-3, PAD-4, PAD-7, PAD-10, IICV-11, IICV-12, OENC-3, RFC4301-FUNC-8.

## IPSEC-F-ESP-SERVICES

**An ESP SA gives confidentiality, integrity, or both, with separate algorithms or a combined one; the encryption and the integrity algorithm are never both NULL.**

- **Sources** — RFC 4301 §3.2, §4.2; RFC 4303 §2, §3.2, §5.
- **Level** — mandatory (reason: keyword). RFC4303-ALG-2 has the strength "must (selecting at least one service); must not (setting both algorithms to NULL at the same time)".
- **Description** — An implementation supports the NULL encryption algorithm. Confidentiality without integrity is optional, and an implementation that offers it also supports the NULL integrity algorithm; RFC 4301 advises against it. With the integrity service the ICV covers the ESP header, the payload and the trailer.
- **Checks** — core: RFC4303-ALG-2, ALG-5, ALG-6, CONF-8, CONF-10, CONF-11, FMT-5, FMT-6, RFC4301-FUNC-9. Supporting: RFC4303-ALG-1, ALG-3, ALG-4, ALG-7, ALG-8, ALG-9, ALG-10, ALG-11, ALG-12, CONF-6, CONF-7, CONF-9, FMT-4, FMT-8, FMT-9, FMT-10, ICV-1, ICV-2, RFC4301-OVW-9, FUNC-4.

## IPSEC-F-ESP-PROCESSING

**The ESP sender encrypts and then computes the ICV; the receiver verifies the ICV before it decrypts, discards a packet that fails, and restores the original datagram.**

- **Sources** — RFC 4303 §3.3, §3.4.3, §3.4.4.
- **Level** — mandatory (reason: keyword). RFC4303-IICV-3 has the strength "must".
- **Description** — The sender applies ESP only to a packet whose SA asks for it. The ICV covers the whole ESP packet except the ICV field. The receiver completes the integrity check before it passes a decrypted packet on, and rebuilds the datagram from the IP header and the decrypted payload.
- **Checks** — core: RFC4303-OENC-4, OENC-7, OENC-9, IICV-1, IICV-2, IICV-3, IICV-8, IICV-18, ISEQ-23. Supporting: RFC4303-OSA-2, OENC-5, OENC-6, OENC-8, OENC-11, OENC-12, OENC-13, OENC-14, OENC-15, OENC-16, OENC-17, OENC-18, IICV-6, IICV-7, IICV-9, IICV-10, IICV-16, IICV-19, IICV-20, IICV-21, IICV-22, IICV-25, IICV-26, IICV-27, ISEQ-21, ISEQ-22.

## IPSEC-F-SEQUENCE-NUMBERS

**The sender numbers the packets of each SA from 1, one by one, in a 32-bit field that is always present, and never lets the number cycle while anti-replay is on.**

- **Sources** — RFC 4301 §4.4.2.1; RFC 4302 §2.5, §3.3.2; RFC 4303 §2.2, §3.3.3.
- **Level** — mandatory (reason: keyword). RFC4301-SADI-2 has the strength "must".
- **Description** — The counter of an SA starts at 0 when the SA is created, so the first packet carries 1. The sender transmits the field even when the receiver does not check it. Before the counter would cycle, a new SA replaces the old one; without anti-replay the counter rolls over.
- **Checks** — core: RFC4301-SADI-2, RFC4302-SEQ-1, SEQ-2, SEQ-8, OSEQ-1, OSEQ-2, OSEQ-4, RFC4303-SEQ-1, SEQ-2, SEQ-7, OSEQ-1, OSEQ-3. Supporting: RFC4302-SEQ-3, SEQ-4, SEQ-6, SEQ-9, SEQ-10, OSEQ-3, OSEQ-7, OSEQ-8, OSEQ-9, OSEQ-10, RFC4303-SEQ-3, SEQ-4, SEQ-6, SEQ-9, OSEQ-2, OSEQ-6, OSEQ-7, OSEQ-8, OSEQ-9, OSEQ-10, OSEQ-11.

## IPSEC-F-ANTI-REPLAY

**A receiver can check each Sequence Number against a sliding window of at least 32 packets, and discards a duplicate or a packet left of the window.**

- **Sources** — RFC 4301 §3.2, §4.4.2.1; RFC 4302 §3.4.3, §5; RFC 4303 §3.4.3, §5.
- **Level** — mandatory (reason: keyword). RFC4301-SADI-5 has the strength "must".
- **Description** — Every implementation supports the service; the receiver turns it on or off for each SA, and only together with the integrity service. The window moves only after a successful ICV check. Both RFC 4302 and RFC 4303 advise against the service on a manually keyed SA.
- **Checks** — core: RFC4301-SADI-5, RFC4302-ISEQ-1, ISEQ-9, ISEQ-10, ISEQ-24, RFC4303-ISEQ-1, ISEQ-2, ISEQ-8, ISEQ-9, ISEQ-28. Supporting: RFC4301-OVW-7, RFC4302-ISEQ-2, ISEQ-3, ISEQ-4, ISEQ-5, ISEQ-6, ISEQ-7, ISEQ-8, ISEQ-11, ISEQ-12, ISEQ-13, ISEQ-14, ISEQ-15, ISEQ-20, ISEQ-23, ISEQ-25, ISEQ-26, ISEQ-27, ISEQ-28, ISEQ-29, CONF-4, CONF-5, RFC4303-SEQ-8, ISEQ-3, ISEQ-4, ISEQ-5, ISEQ-6, ISEQ-7, ISEQ-10, ISEQ-11, ISEQ-12, ISEQ-13, ISEQ-14, ISEQ-15, ISEQ-16, ISEQ-17, ISEQ-20, ISEQ-26, ISEQ-27, ISEQ-29, ISEQ-30, ISEQ-31, CONF-4, CONF-5.

## IPSEC-F-EXTENDED-SEQUENCE-NUMBERS

**An SA can use 64-bit sequence numbers, of which only the low-order 32 bits are sent and the high-order 32 bits enter the ICV.**

- **Sources** — RFC 4301 §4.4.1.2; RFC 4302 §2.5.1, §3.3.2, §3.3.3.2.2, §3.4.3; RFC 4303 §2.2.1, §3.3.2, §3.3.3, §3.4.3.
- **Level** — optional (reason: keyword). Every core statement says should or may.
- **Description** — The SA management protocol negotiates the option. The receiver rebuilds the high-order bits from its window, which tolerates a gap of 2^32 - 1 packets.
- **Checks** — core: RFC4302-SEQ-11, RFC4303-SEQ-10. Supporting: RFC4302-SEQ-12, SEQ-13, SEQ-14, SEQ-15, OSEQ-11, OSEQ-12, ISEQ-16, ISEQ-17, ISEQ-18, ISEQ-19, OICV-29, OICV-30, IICV-7, RFC4303-SEQ-11, SEQ-12, SEQ-13, SEQ-14, SEQ-15, SEQ-16, OSEQ-12, OSEQ-13, ISEQ-18, ISEQ-19, OENC-10, RFC4301-SPDE-17.

## IPSEC-F-ESP-TFC-PADDING

**An ESP sender can add traffic flow confidentiality padding after the payload, when the payload carries its own length, and the receiver drops it.**

- **Sources** — RFC 4303 §2, §2.7, §3.4.4.
- **Level** — optional (reason: conditional keyword). RFC4303-TFC-2 says should; the must not of RFC4303-TFC-5 holds only once a sender adds the padding.
- **Description** — The padding sits after the Payload Data and before the Padding field; the Pad Length does not count it, and the length field of the payload is not changed. The SA management protocol negotiates its use.
- **Checks** — core: RFC4303-TFC-2, TFC-5. Supporting: RFC4303-TFC-1, TFC-3, TFC-4, TFC-6, TFC-7, TFC-8, FMT-11, FMT-12, PADL-2, IICV-17.

## IPSEC-F-ESP-DUMMY-PACKETS

**An ESP sender can send dummy packets with Next Header 59, and the receiver discards them without an error.**

- **Sources** — RFC 4303 §2.6, §3.4.4.
- **Level** — mandatory (reason: keyword). RFC4303-NH-2 has the strength "must".
- **Description** — A dummy packet has every field of the ESP header and trailer. Local controls turn the generation on for each SA.
- **Checks** — core: RFC4303-NH-2, NH-3, NH-4, NH-5. Supporting: RFC4303-NH-6, IICV-13.

## IPSEC-F-MULTICAST

**An implementation that supports multicast keeps group SAs, found by SPI and destination, and also source, and tells them apart from unicast SAs with the same SPI.**

- **Sources** — RFC 4301 §4.1, §4.4.1.1, §4.4.2; RFC 4302 §2.4, §3.4.2, §5; RFC 4303 §2.1, §3.4.2, §5.
- **Level** — optional (reason: conditional keyword). The musts hold only for an implementation that supports multicast; RFC4301-SA-9 frees a unicast-only one.
- **Description** — A group controller chooses the SPI of a group SA. The SAD entry says whether the lookup uses the destination alone or the destination and the source. The SPD does not hold multicast addresses; a group SPD does.
- **Checks** — core: RFC4301-SA-8, SA-10, RFC4302-SPI-6, SPI-7, RFC4303-SPI-5, SPI-7. Supporting: RFC4301-SA-9, SA-11, SA-12, SA-22, SA-23, SAD-4, SAD-9, SAD-10, SAD-11, SAD-12, SAD-13, SEL-6, TUN-20, IN-16, RFC4302-SPI-8, SPI-9, SPI-18, SPI-19, ISA-3, CONF-3, RFC4303-SPI-6, SPI-8, SPI-9, SPI-16, SPI-17, ISA-3, CONF-3.

## Coverage of the catalog entries

Every one of the 797 entries of the three catalogs is in at least one feature above, as core
or as supporting: RFC 4301 with 370, RFC 4302 with
190 and RFC 4303 with
237.
