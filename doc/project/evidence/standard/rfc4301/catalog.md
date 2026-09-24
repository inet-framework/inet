# RFC 4301 (Security Architecture for the Internet Protocol) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC4301-*` · **Stands on:** [standards.md](../../protocol/ipsec/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow for RFC 4301, Security Architecture for the Internet Protocol, of
December 2005, a document of the in-scope set of IPsec. The catalog comes from the RFC text
only. It contains no simulation model names and no code references.

Source, in the `standards` project beside the INET tree:

- [`standards/RFC/rfc4301.txt`](../../../../../../standards/RFC/rfc4301.txt) —
  Security Architecture for the Internet Protocol, December 2005. Downloaded 2026-09-23 from
  <https://www.rfc-editor.org/rfc/rfc4301.txt>.

The two companion documents of the in-scope set have catalogs of their own:
[`rfc4302/catalog.md`](../rfc4302/catalog.md) (the Authentication Header) and
[`rfc4303/catalog.md`](../rfc4303/catalog.md) (the Encapsulating Security Payload).

The in-scope sections are §3.1 and §3.2, §4 to §4.4.2, §5.1, §5.2 and §10. §4.4.3 (the Peer
Authorization Database) and §4.5 (key management) belong to the key management protocol, which
enters at level 5.

The family of documents and the in-scope set are in
[`standards.md`](../../protocol/ipsec/standards.md). The features these statements build are in
[`features.md`](../../protocol/ipsec/features.md).

The state of the workflow — which statement a check targets, which test carries it, and
what the run said — is **not** in this document. It lives in the coverage ledger,
[`ipsec/coverage.md`](../../model/ipsec/coverage.md). Keeping it out is deliberate: this catalog
states what the standard says, so a new test or a new run must never force an edit here.

Quotes are verbatim. A reference such as `rfc4301.txt:1203-1205` points to lines of that
file. Where a quote crosses a page break of the RFC, the reference names both line groups.

The entries were drafted section by section and merged; the sections are the areas below, in
the order of the document.

## How to read an entry

- **ID** — `RFC4301-AREA-n`. The ID is stable forever. New entries go at the end of an area.
- **Quote** — the verbatim sentence, with a line reference into the text.
- **Strength** — the word the document uses. RFC 4301 uses the keywords of RFC 2119 in capitals;
  the entry records `must`, `must not`, `should`, `should not` or `may` for them (`must` for
  REQUIRED), and `description` for a normative sentence with no keyword. Where the text writes a
  keyword in lower case, the entry records the word and says so, for example `must (lower
  case)`. An entry with two keywords names what each one covers. A step of a numbered procedure
  that states an action is an entry of its own.
- **Class** — how a test can observe the statement: `wire` (fields of a header on a link, or
  the presence or absence of a packet), `end-to-end` (what an implementation accepts, delivers
  or discards, seen in what it sends or delivers afterwards), `error-signal` (an ICMP error
  report), `internal` (state or a database inside an implementation, an audit log), `encoding`
  (the exact bit layout, sizes and alignment).
- **Check idea** — one or two sentences, still without implementation names.

## Index

| ID | Statement |
| --- | --- |
| [RFC4301-OVW-1](#rfc4301-ovw-1) | At the IPsec boundary, access controls let a packet cross unimpeded, receive AH or ESP security services, or be discarded. |
| [RFC4301-OVW-2](#rfc4301-ovw-2) | A host implementation must support connectivity between two hosts and between a security gateway and a host. |
| [RFC4301-OVW-3](#rfc4301-ovw-3) | A security gateway must support all three connectivity paths: host to host, gateway to gateway, and gateway to host. |
| [RFC4301-OVW-4](#rfc4301-ovw-4) | An IPsec implementation may support more than one interface on either side of the boundary, or on both. |
| [RFC4301-OVW-5](#rfc4301-ovw-5) | An IPsec implementation must support ESP, and may support AH. |
| [RFC4301-OVW-6](#rfc4301-ovw-6) | AH offers integrity and data origin authentication for the traffic it protects. |
| [RFC4301-OVW-7](#rfc4301-ovw-7) | A receiver may turn on the anti-replay feature of AH at its own discretion. |
| [RFC4301-OVW-8](#rfc4301-ovw-8) | ESP offers the same services as AH, and also offers confidentiality. |
| [RFC4301-OVW-9](#rfc4301-ovw-9) | An IPsec implementation should not use ESP to provide confidentiality without integrity. |
| [RFC4301-OVW-10](#rfc4301-ovw-10) | AH and ESP provide access control, enforced by cryptographic key distribution and by the traffic-flow management the Security Policy Database dictates. |
| [RFC4301-OVW-11](#rfc4301-ovw-11) | An IPsec implementation supports both manual and automated distribution of keys. |
| [RFC4301-OVW-12](#rfc4301-ovw-12) | An implementation may use an automated key distribution technique other than IKEv2. |
| [RFC4301-SA-1](#rfc4301-sa-1) | Every AH or ESP implementation must support the concept of a security association. |
| [RFC4301-SA-2](#rfc4301-sa-2) | A security association is simplex: it applies its security service in one direction only. |
| [RFC4301-SA-3](#rfc4301-sa-3) | A single security association uses either AH or ESP, never both together. |
| [RFC4301-SA-4](#rfc4301-sa-4) | When a traffic stream needs both AH and ESP protection, an implementation creates two SAs and coordinates them to apply the protocols in turn. |
| [RFC4301-SA-5](#rfc4301-sa-5) | Bidirectional communication between two IPsec-enabled systems needs a pair of SAs, one for each direction. |
| [RFC4301-SA-6](#rfc4301-sa-6) | For a security association that carries unicast traffic, the Security Parameters Index alone identifies the SA. |
| [RFC4301-SA-7](#rfc4301-sa-7) | As a local matter, an implementation may use the SPI together with the IPsec protocol type to identify a security association. |
| [RFC4301-SA-8](#rfc4301-sa-8) | An IPsec implementation that supports multicast must support multicast SAs, with the mapping algorithm this section gives for inbound IPsec datagrams. |
| [RFC4301-SA-9](#rfc4301-sa-9) | An implementation that supports only unicast traffic does not need the multicast de-multiplexing algorithm. |
| [RFC4301-SA-10](#rfc4301-sa-10) | A multicast-capable IPsec implementation must de-multiplex inbound traffic correctly even when a GSA and a unicast SA share one SPI. |
| [RFC4301-SA-11](#rfc4301-sa-11) | Each SAD entry states whether its SA look-up uses the destination address alone, or the destination and source addresses together with the SPI. |
| [RFC4301-SA-12](#rfc4301-sa-12) | For a multicast SA, the SAD look-up does not use the protocol field. |
| [RFC4301-SA-13](#rfc4301-sa-13) | For an inbound IPsec-protected packet, an implementation searches the SAD for the entry that matches the longest SA identifier. |
| [RFC4301-SA-14](#rfc4301-sa-14) | The entry that also matches the destination address, or the destination and source addresses, is the "longest" match among SPI matches. |
| [RFC4301-SA-15](#rfc4301-sa-15) | An implementation first searches the SAD for an entry that matches the SPI, destination address, and source address together. |
| [RFC4301-SA-16](#rfc4301-sa-16) | When no three-part match exists, an implementation searches the SAD for an entry that matches both SPI and destination address. |
| [RFC4301-SA-17](#rfc4301-sa-17) | As a last step, an implementation matches on SPI alone with one shared AH/ESP SPI space, or on SPI and protocol with separate spaces. |
| [RFC4301-SA-18](#rfc4301-sa-18) | When no SAD entry matches at any of the three steps, an implementation discards the packet. |
| [RFC4301-SA-19](#rfc4301-sa-19) | When no SAD entry matches at any of the three steps, an implementation logs an auditable event. |
| [RFC4301-SA-20](#rfc4301-sa-20) | An implementation may use any SAD search method, or none, as long as its externally visible behavior matches the three-step search order. |
| [RFC4301-SA-21](#rfc4301-sa-21) | An implementation sets its indication of source-and-destination address matching either by manual SA configuration, or by an SA management protocol. |
| [RFC4301-SA-22](#rfc4301-sa-22) | A Source-Specific Multicast group SA typically uses a three-part identifier: SPI, destination multicast address, and source address. |
| [RFC4301-SA-23](#rfc4301-sa-23) | An Any-Source Multicast group SA needs only an SPI and a destination multicast address as its identifier. |
| [RFC4301-SA-24](#rfc4301-sa-24) | A sender should put traffic of different classes, with the same selector values, on different SAs, to support Quality of Service. |
| [RFC4301-SA-25](#rfc4301-sa-25) | An IPsec implementation must let a sender establish and keep multiple SAs, with the same selectors, between one sender and receiver. |
| [RFC4301-SA-26](#rfc4301-sa-26) | A receiver must process packets from different parallel SAs without prejudice. |
| [RFC4301-SA-27](#rfc4301-sa-27) | In tunnel mode, the DSCP value for QoS classification appears in the inner IP header. |
| [RFC4301-SA-28](#rfc4301-sa-28) | In transport mode, an implementation must not check the DSCP value as part of SA or packet validation. |
| [RFC4301-SA-29](#rfc4301-sa-29) | Both SAs of an SA pair are the same mode: both transport mode, or both tunnel mode. |
| [RFC4301-SA-30](#rfc4301-sa-30) | A transport mode SA is typically used between a pair of hosts, to protect their end-to-end traffic. |
| [RFC4301-SA-31](#rfc4301-sa-31) | Transport mode may secure one segment of a path: between two security gateways, or between a security gateway and a host. |
| [RFC4301-SA-32](#rfc4301-sa-32) | When used between security gateways, or a gateway and a host, transport mode may carry an in-IP tunnel, such as IP-in-IP or GRE, or a dynamic routing protocol. |
| [RFC4301-SA-33](#rfc4301-sa-33) | An intermediate system, such as a security gateway, uses transport mode only for its own outbound source address or its own inbound destination address. |
| [RFC4301-SA-34](#rfc4301-sa-34) | In IPv4, a transport mode security protocol header sits right after the IP header and any options, and before the next layer protocol. |
| [RFC4301-SA-35](#rfc4301-sa-35) | In IPv6, the security protocol header of a transport mode SA sits after the base IP header and any extension headers ordered ahead of it. |
| [RFC4301-SA-36](#rfc4301-sa-36) | In IPv6, the transport mode security protocol header may sit before or after the destination options extension header. |
| [RFC4301-SA-37](#rfc4301-sa-37) | In IPv6, the transport mode security protocol header must sit before the next layer protocol header. |
| [RFC4301-SA-38](#rfc4301-sa-38) | In transport mode, an ESP SA protects only the next layer protocols, not the IP header or the extension headers ahead of the ESP header. |
| [RFC4301-SA-39](#rfc4301-sa-39) | In transport mode, an AH SA also protects selected parts of the IP header ahead of it, selected extension header parts, and selected IP options. |
| [RFC4301-SA-40](#rfc4301-sa-40) | A tunnel mode SA is applied to an IP tunnel, with access control applied to the headers of the traffic inside the tunnel. |
| [RFC4301-SA-41](#rfc4301-sa-41) | Two hosts may establish a tunnel mode SA between themselves. |
| [RFC4301-SA-42](#rfc4301-sa-42) | Except for two exceptions, an SA must be tunnel mode whenever either end is a security gateway. |
| [RFC4301-SA-43](#rfc4301-sa-43) | When traffic is destined for a security gateway itself, such as SNMP management commands, the gateway acts as a host and may use transport mode. |
| [RFC4301-SA-44](#rfc4301-sa-44) | A packet that becomes fragmented en route has all its fragments delivered to the same IPsec instance, for reassembly before cryptographic processing. |
| [RFC4301-SA-45](#rfc4301-sa-45) | AH and ESP do not apply transport mode to an IPv4 packet that is a fragment; only tunnel mode applies to it. |
| [RFC4301-SA-46](#rfc4301-sa-46) | For IPv6 too, an implementation does not use transport mode AH or ESP on a packet that is a fragment. |
| [RFC4301-SA-47](#rfc4301-sa-47) | In tunnel mode, an outer IP header carries the IPsec addresses, an inner IP header carries the packet's ultimate addresses, and the security protocol header sits between them. |
| [RFC4301-SA-48](#rfc4301-sa-48) | In tunnel mode, AH protects selected parts of the outer IP header, plus the whole tunneled packet: inner header and next layer protocols. |
| [RFC4301-SA-49](#rfc4301-sa-49) | In tunnel mode, ESP protects only the tunneled inner packet, not the outer IP header. |
| [RFC4301-SA-50](#rfc4301-sa-50) | A host implementation of IPsec must support both transport mode and tunnel mode, whether native, BITS, or BITW. |
| [RFC4301-SA-51](#rfc4301-sa-51) | A security gateway must support tunnel mode, and may support transport mode. |
| [RFC4301-FUNC-1](#rfc4301-func-1) | The set of security services an SA offers depends on the protocol chosen, the SA mode, its endpoints, and the optional services selected. |
| [RFC4301-FUNC-2](#rfc4301-func-2) | AH and ESP both offer integrity and authentication, but their coverage differs between the protocols and between transport and tunnel mode. |
| [RFC4301-FUNC-3](#rfc4301-func-3) | AH can protect the integrity of an IPv4 option or an IPv6 extension header en route, except for a field that may change unpredictably. |
| [RFC4301-FUNC-4](#rfc4301-func-4) | In some contexts, an ESP tunnel mode SA can give the same security as AH. |
| [RFC4301-FUNC-5](#rfc4301-func-5) | The access control granularity an SA gives depends on the selectors chosen to define that SA. |
| [RFC4301-FUNC-6](#rfc4301-func-6) | When confidentiality is selected, a tunnel mode ESP SA between two security gateways can give partial traffic flow confidentiality. |
| [RFC4301-FUNC-7](#rfc4301-func-7) | Tunnel mode lets an implementation encrypt the inner IP header, to hide the identities of the ultimate source and destination. |
| [RFC4301-FUNC-8](#rfc4301-func-8) | ESP payload padding can hide the size of a packet, to conceal more of the traffic's outer characteristics. |
| [RFC4301-FUNC-9](#rfc4301-func-9) | A compliant implementation must not let an ESP SA use both NULL encryption and no integrity algorithm. |
| [RFC4301-FUNC-10](#rfc4301-func-10) | An attempt to negotiate an ESP SA with NULL encryption and no integrity algorithm is an auditable event, at both the initiator and the responder. |
| [RFC4301-FUNC-11](#rfc4301-func-11) | The audit log entry for this event should include the date and time, the local IKE IP address, and the remote IKE IP address. |
| [RFC4301-FUNC-12](#rfc4301-func-12) | The initiator should record the relevant SPD entry in this audit log entry. |
| [RFC4301-COMB-1](#rfc4301-comb-1) | RFC 4301 does not require an implementation to support nested security associations or SA bundles. |
| [RFC4301-COMB-2](#rfc4301-comb-2) | An implementation that supports nested SAs should give a management interface to express the nesting need, and then build the matching SPD and forwarding table entries. |
| [RFC4301-DB-1](#rfc4301-db-1) | An implementation may structure its databases differently from this model, but its outside behavior must match this model's observable traits. |
| [RFC4301-DB-2](#rfc4301-db-2) | The Security Policy Database sets the disposition of all IP traffic, inbound or outbound, for a host or security gateway. |
| [RFC4301-DB-3](#rfc4301-db-3) | The Security Association Database holds the parameters for each established, keyed SA. |
| [RFC4301-DB-4](#rfc4301-db-4) | The Peer Authorization Database links an SA management protocol, such as IKE, to the Security Policy Database. |
| [RFC4301-DB-5](#rfc4301-db-5) | A security gateway that serves several subscribers may run several separate IPsec contexts. |
| [RFC4301-DB-6](#rfc4301-db-6) | Each IPsec context may have, and may use, fully independent identities, policies, and SAs. |
| [RFC4301-DB-7](#rfc4301-db-7) | A gateway with several IPsec contexts has a means to match an inbound SA proposal to the right local context. |
| [RFC4301-DB-8](#rfc4301-db-8) | IPsec assumes that outbound and inbound traffic which has passed through IPsec processing is forwarded to fit the context where IPsec runs. |
| [RFC4301-DB-9](#rfc4301-db-9) | A non-initial fragment lacks one or more selector values access control needs, such as the next layer protocol or the ports. |
| [RFC4301-DB-10](#rfc4301-db-10) | An initial fragment carries every selector value access control needs. |
| [RFC4301-DB-11](#rfc4301-db-11) | For IPv6, the fragment that carries the next layer protocol and ports depends on the extension headers present, so the initial fragment may not be the first fragment. |
| [RFC4301-SPD-1](#rfc4301-spd-1) | An IPsec implementation consults the SPD, or its caches, for all traffic that crosses the IPsec boundary, including traffic not protected by IPsec and IKE traffic. |
| [RFC4301-SPD-2](#rfc4301-spd-2) | An IPsec implementation must have at least one SPD. |
| [RFC4301-SPD-3](#rfc4301-spd-3) | An IPsec implementation may support multiple SPDs, if appropriate for the context in which it operates. |
| [RFC4301-SPD-4](#rfc4301-spd-4) | An IPsec implementation that supports multiple SPDs must include an explicit SPD selection function to select the SPD for outbound traffic processing. |
| [RFC4301-SPD-5](#rfc4301-spd-5) | The SPD selection function takes the outbound packet and local metadata as input and returns an SPD identifier (SPD-ID). |
| [RFC4301-SPD-6](#rfc4301-spd-6) | The SPD must let a user or administrator order the entries to express the desired access control policy. |
| [RFC4301-SPD-7](#rfc4301-spd-7) | The SPD distinguishes traffic that receives IPsec protection from traffic allowed to bypass IPsec, both at the sender and at the receiver. |
| [RFC4301-SPD-8](#rfc4301-spd-8) | For every outbound or inbound datagram, the SPD chooses one of three processing choices: DISCARD, BYPASS IPsec, or PROTECT using IPsec. |
| [RFC4301-SPD-9](#rfc4301-spd-9) | DISCARD means traffic that is not allowed to cross the IPsec boundary in the given direction. |
| [RFC4301-SPD-10](#rfc4301-spd-10) | BYPASS means traffic that is allowed to cross the IPsec boundary without IPsec protection. |
| [RFC4301-SPD-11](#rfc4301-spd-11) | PROTECT means traffic that receives IPsec protection, and for it the SPD states the security protocol, mode, security service options, and cryptographic algorithms to use. |
| [RFC4301-SPD-12](#rfc4301-spd-12) | The SPD is divided into three parts: SPD-S for traffic subject to IPsec protection, SPD-O for outbound traffic to bypass or discard, and SPD-I for inbound traffic to bypass or discard. |
| [RFC4301-SPD-13](#rfc4301-spd-13) | When a packet looked up in SPD-I matches no entry, the IPsec implementation must discard the packet. |
| [RFC4301-SPD-14](#rfc4301-spd-14) | For outbound traffic, when no match is found in SPD-S the implementation checks SPD-O for a bypass match, and when no match is found in SPD-O first, it checks SPD-S. |
| [RFC4301-SPD-15](#rfc4301-spd-15) | In an ordered, non-decorrelated SPD, the SPD-S, SPD-I, and SPD-O entries are interleaved into one list, searched in a single lookup. |
| [RFC4301-SPD-16](#rfc4301-spd-16) | Each SPD entry states a packet disposition of BYPASS, DISCARD, or PROTECT, and is keyed by a list of one or more selectors. |
| [RFC4301-SPD-17](#rfc4301-spd-17) | Every SPD should have a final, nominal entry that matches any otherwise-unmatched traffic and discards it. |
| [RFC4301-SPD-18](#rfc4301-spd-18) | The SPD must let a user or administrator specify SPD-I entries, keyed by selector values, for inbound traffic to bypass or discard. |
| [RFC4301-SPD-19](#rfc4301-spd-19) | The SPD must let a user or administrator specify SPD-O entries, keyed by selector values, for outbound traffic to bypass or discard. |
| [RFC4301-SPD-20](#rfc4301-spd-20) | The SPD must let a user or administrator specify SPD-S entries, with the selector values, the SA-creation controls, and the parameters needed to protect the matching traffic with AH or ESP. |
| [RFC4301-SPD-21](#rfc4301-spd-21) | An SPD-S entry also contains a populate-from-packet (PFP) flag and bits stating whether the SA lookup uses the local and remote IP addresses in addition to the SPI. |
| [RFC4301-SPD-22](#rfc4301-spd-22) | For traffic protected by IPsec, an SPD entry swaps the Local and Remote address and ports to represent directionality, matching IKE conventions. |
| [RFC4301-SPD-23](#rfc4301-spd-23) | SPD entries for ICMP are specified in the same way as SPD entries for other protocols, even though ICMP often has no bidirectional authorization requirement. |
| [RFC4301-SPD-24](#rfc4301-spd-24) | ICMP, Mobility Header, and non-initial-fragment packets have no port fields, so SPD entries express access control for them through the ICMP message type and code or the Mobility Header type instead of ports. |
| [RFC4301-SPD-25](#rfc4301-spd-25) | For bypassed or discarded traffic, the SPD supports separate inbound and outbound entries, so a unidirectional flow can be permitted. |
| [RFC4301-SPD-26](#rfc4301-spd-26) | The special selector value ANY matches any value in the corresponding packet field, or a field that is absent or obscured. |
| [RFC4301-SPD-27](#rfc4301-spd-27) | The special selector value OPAQUE means the corresponding field is not available for examination, because it may be absent from a fragment, absent for the Next Layer Protocol, or already encrypted by IPsec. |
| [RFC4301-SPD-28](#rfc4301-spd-28) | The ANY selector value encompasses the OPAQUE value, so OPAQUE is needed only to distinguish an allowed-any-value field from an absent or unavailable field. |
| [RFC4301-SPD-29](#rfc4301-spd-29) | Each selector of an SPD entry states how to derive the matching value for a new SAD entry from the SPD entry and the packet. |
| [RFC4301-SPD-30](#rfc4301-spd-30) | When IPsec processing is specified for an entry, a populate-from-packet (PFP) flag may be asserted for one or more of the Local address, Remote address, Next Layer Protocol, and port, ICMP type/code, or Mobility Header type selectors. |
| [RFC4301-SPD-31](#rfc4301-spd-31) | When the PFP flag is set for a selector, the new SA takes its value for that selector from the packet; otherwise, the SA takes its value from the SPD entry. |
| [RFC4301-SPD-32](#rfc4301-spd-32) | Every IPsec implementation must have a management interface that lets a user or system administrator manage the SPD. |
| [RFC4301-SPD-33](#rfc4301-spd-33) | The management interface lets the user or administrator specify the security processing applied to every packet that crosses the IPsec boundary. |
| [RFC4301-SPD-34](#rfc4301-spd-34) | The management interface for the SPD must allow creation of entries consistent with the defined selectors. |
| [RFC4301-SPD-35](#rfc4301-spd-35) | The management interface for the SPD must support total ordering of the entries as seen through that interface. |
| [RFC4301-SPD-36](#rfc4301-spd-36) | In host systems, applications may be allowed to create SPD entries. |
| [RFC4301-SPD-37](#rfc4301-spd-37) | The system administrator must be able to specify whether a user or application can override the default system policies. |
| [RFC4301-SPD-38](#rfc4301-spd-38) | All IPsec implementations must support the standard set of SPD elements that this document specifies. |
| [RFC4301-SPD-39](#rfc4301-spd-39) | When an SPD entry is decorrelated, the IPsec implementation must link all the resulting entries together, so they can be placed into the caches and the SAD together. |
| [RFC4301-SPD-40](#rfc4301-spd-40) | When a responder uses a decorrelated SPD, it should use the decorrelated entries to match the initiator's traffic selector proposals. |
| [RFC4301-SPD-41](#rfc4301-spd-41) | When a responder has a correlated SPD, it should match the proposals against the correlated entries. |
| [RFC4301-SPD-42](#rfc4301-spd-42) | When the SPD is not decorrelated, caching is not allowed, and the implementation must perform an ordered search of the SPD to verify that inbound traffic arriving on an SA is consistent with the access control policy. |
| [RFC4301-SPD-43](#rfc4301-spd-43) | When a decorrelated SPD is available, its decorrelated entries are used to populate the SPD-S cache. |
| [RFC4301-SPD-44](#rfc4301-spd-44) | When the SPD changes while the system is running, an implementation should check the effect of the change on existing SAs. |
| [RFC4301-SPD-45](#rfc4301-spd-45) | An implementation should give a user or administrator a mechanism to configure what action to take on an SA affected by an SPD change, such as deleting it or leaving it unchanged. |
| [RFC4301-SEL-1](#rfc4301-sel-1) | All IPsec implementations must support the selector parameters listed in this section, to control the granularity of an SA. |
| [RFC4301-SEL-2](#rfc4301-sel-2) | The Local and Remote addresses of a selector are both IPv4 or both IPv6, never a mix of address types. |
| [RFC4301-SEL-3](#rfc4301-sel-3) | The Local and Remote port selectors, the ICMP message type and code, and the Mobility Header type may be labeled OPAQUE when a fragment makes these fields inaccessible. |
| [RFC4301-SEL-4](#rfc4301-sel-4) | The Remote IP Address selector is a list of ranges of IPv4 or IPv6 addresses, expressing a single address, a list of addresses, a range, or a list of ranges. |
| [RFC4301-SEL-5](#rfc4301-sel-5) | The Local IP Address selector is a list of ranges of IPv4 or IPv6 addresses, expressing a single address, a list of addresses, a range, or a list of ranges, naming the address or addresses this implementation protects. |
| [RFC4301-SEL-6](#rfc4301-sel-6) | The SPD does not support multicast address entries; multicast SAs use a separate Group SPD (GSPD) instead. |
| [RFC4301-SEL-7](#rfc4301-sel-7) | The Next Layer Protocol selector comes from the IPv4 Protocol field or the IPv6 Next Header field, and its value is an individual protocol number, ANY, or, for IPv6 only, OPAQUE. |
| [RFC4301-SEL-8](#rfc4301-sel-8) | The Next Layer Protocol is whatever protocol comes after any IP extension headers present in the packet. |
| [RFC4301-SEL-9](#rfc4301-sel-9) | An IPsec implementation should provide a mechanism for configuring which IPv6 extension headers to skip when it locates the Next Layer Protocol. |
| [RFC4301-SEL-10](#rfc4301-sel-10) | The default configuration for headers to skip when locating the Next Layer Protocol should include Hop-by-Hop Options (0), Routing Header (43), Fragmentation Header (44), and Destination Options (60). |
| [RFC4301-SEL-11](#rfc4301-sel-11) | The default skip list does not include AH (51) or ESP (50); IPsec treats AH and ESP as Next Layer Protocols for selector lookup. |
| [RFC4301-SEL-12](#rfc4301-sel-12) | When the Next Layer Protocol uses two ports, such as TCP, UDP, or SCTP, the SPD entry has Local and Remote Port selectors, each a list of ranges of values. |
| [RFC4301-SEL-13](#rfc4301-sel-13) | A port selector must support the value OPAQUE, because the port fields may be unavailable in a fragment or encrypted by IPsec. |
| [RFC4301-SEL-14](#rfc4301-sel-14) | A port selector set to a value other than ANY or OPAQUE cannot match a packet that is a non-initial fragment, because port values are unavailable there. |
| [RFC4301-SEL-15](#rfc4301-sel-15) | When the SA requires a port value other than ANY or OPAQUE, an implementation must discard an arriving fragment that has no port fields. |
| [RFC4301-SEL-16](#rfc4301-sel-16) | When the Next Layer Protocol is a Mobility Header, the SPD entry has a selector for the IPv6 Mobility Header message type (MH type), an 8-bit value identifying the mobility message. |
| [RFC4301-SEL-17](#rfc4301-sel-17) | The Mobility Header message type may not be available when a fragmented packet is received. |
| [RFC4301-SEL-18](#rfc4301-sel-18) | When the Next Layer Protocol is ICMP, the SPD entry has a 16-bit selector for the ICMP message type and code: an 8-bit message type, or ANY, and an 8-bit code. |
| [RFC4301-SEL-19](#rfc4301-sel-19) | The ICMP type/code selector can express a single type with a range of codes, a single type with any code, or any type with any code. |
| [RFC4301-SEL-20](#rfc4301-sel-20) | Given a policy entry with a Type range T-start to T-end and a Code range C-start to C-end, an implementation must test whether an ICMP packet's Type t and Code c satisfy (T-start*256)+C-start <= (t*256)+c <= (T-end*256)+C-end to decide a match. |
| [RFC4301-SEL-21](#rfc4301-sel-21) | The ICMP message type and code may not be available when a fragmented packet is received. |
| [RFC4301-SEL-22](#rfc4301-sel-22) | The Name selector, unlike the other selectors, is not taken from a packet; it is a symbolic identifier for an IPsec Local or Remote address. |
| [RFC4301-SEL-23](#rfc4301-sel-23) | A responder uses a named SPD entry for access control when an IP address is not appropriate for the Remote IP address selector, such as for a road warrior; the initiator's source IP address is then bound to the Remote IP address of the SAD entry the IKE negotiation creates, overriding the SPD entry's Remote IP address value. |
| [RFC4301-SEL-24](#rfc4301-sel-24) | All IPsec implementations must support the responder's use of named SPD entries. |
| [RFC4301-SEL-25](#rfc4301-sel-25) | An initiator may use a named SPD entry to identify a user for whom an SA is created or traffic is bypassed; the initiator's IP source address then replaces the local address in the SPD cache entry and the outbound SAD entry, and the remote address in the inbound SAD entry. |
| [RFC4301-SEL-26](#rfc4301-sel-26) | Support for the initiator's use of named SPD entries is optional for multi-user, native host implementations and does not apply to other implementations. |
| [RFC4301-SEL-27](#rfc4301-sel-27) | An SPD entry can contain both a name, or a list of names, and values for the Local or Remote IP address at the same time. |
| [RFC4301-SEL-28](#rfc4301-sel-28) | For the responder's use, a named SPD entry's identifier is one of four types: a fully qualified user name (email), a fully qualified DNS name, an X.500 distinguished name, or a byte string. |
| [RFC4301-SEL-29](#rfc4301-sel-29) | For the initiator's use, a named SPD entry's identifier is a byte string, of only local significance, and it is never transmitted. |
| [RFC4301-SPDE-1](#rfc4301-spde-1) | Users must not include multiple selector sets in a single SPD entry unless the access control intent matches the IKE mix-and-match semantics. |
| [RFC4301-SPDE-2](#rfc4301-spde-2) | An implementation may warn a user who creates an SPD entry with multiple selector sets whose syntax indicates a possible conflict with the IKE semantics. |
| [RFC4301-SPDE-3](#rfc4301-spde-3) | The Remote and Local labels apply only to IP addresses and ports, not to the ICMP message type/code or the Mobility Header type. |
| [RFC4301-SPDE-4](#rfc4301-spde-4) | When a selector's value list uses the reserved value OPAQUE or ANY, that value is the only value in the list, and it appears only once. |
| [RFC4301-SPDE-5](#rfc4301-spde-5) | ANY and OPAQUE are local syntax conventions; IKEv2 negotiates them as the ranges start=0/ end=<max> for ANY and start=<max>/end=0 for OPAQUE. |
| [RFC4301-SPDE-6](#rfc4301-spde-6) | The Name field of an SPD entry is an optional list of IDs, a quasi-selector. |
| [RFC4301-SPDE-7](#rfc4301-spde-7) | An IPsec implementation must support the name forms for the Name field that are described in the Selectors section. |
| [RFC4301-SPDE-8](#rfc4301-spde-8) | The PFP flags field of an SPD entry has one flag per traffic selector, and each flag applies to that selector across every selector set in the entry. |
| [RFC4301-SPDE-9](#rfc4301-spde-9) | When an SA is created, each PFP flag states whether that selector's value comes from the packet that triggered the SA or from the SPD entry. |
| [RFC4301-SPDE-10](#rfc4301-spde-10) | An SPD entry has a PFP flag each for Local Address, Remote Address, Next Layer Protocol, the Local Port or ICMP type/code or Mobility Header type, and the Remote Port or ICMP type/code or Mobility Header type. |
| [RFC4301-SPDE-11](#rfc4301-spde-11) | An SPD entry has one to N selector sets, the conditions for applying its IPsec action, each with a Local Address, a Remote Address, a Next Layer Protocol, a Local Port or ICMP type/code or Mobility Header type, and a Remote Port or ICMP type/code or Mobility Header type. |
| [RFC4301-SPDE-12](#rfc4301-spde-12) | The Next Layer Protocol selector is a single value in each selector set, unlike the address selectors, so an SPD entry can associate several protocols and ports with one SA by listing several selector sets. |
| [RFC4301-SPDE-13](#rfc4301-spde-13) | The Processing info field of an SPD entry states one action, PROTECT, BYPASS, or DISCARD, that applies to all the entry's selector sets together, not a separate action per set. |
| [RFC4301-SPDE-14](#rfc4301-spde-14) | For a PROTECT entry, the IPsec mode field states tunnel or transport mode. |
| [RFC4301-SPDE-15](#rfc4301-spde-15) | For a tunnel-mode PROTECT entry on a non-mobile host with multiple interfaces, the local tunnel address must be statically configured; on a mobile host, it is set outside IPsec. |
| [RFC4301-SPDE-16](#rfc4301-spde-16) | For a tunnel-mode PROTECT entry, the entry holds a remote tunnel address, for which this document defines no standard way to determine the value. |
| [RFC4301-SPDE-17](#rfc4301-spde-17) | A PROTECT entry has an Extended Sequence Number field that states whether the SA uses extended sequence numbers. |
| [RFC4301-SPDE-18](#rfc4301-spde-18) | A PROTECT entry has a stateful fragment checking field that states whether the SA uses stateful fragment checking. |
| [RFC4301-SPDE-19](#rfc4301-spde-19) | A tunnel-mode PROTECT entry has a Bypass DF bit field, true or false. |
| [RFC4301-SPDE-20](#rfc4301-spde-20) | A tunnel-mode PROTECT entry has a field that either bypasses the DSCP value or maps it to an array of unprotected DSCP values, to restrict which DSCP values bypass. |
| [RFC4301-SPDE-21](#rfc4301-spde-21) | A PROTECT entry has an IPsec protocol field that states AH or ESP. |
| [RFC4301-SPDE-22](#rfc4301-spde-22) | A PROTECT entry has an algorithms field that lists, in decreasing priority order, which algorithms to use for AH, for ESP, and for combined mode. |
| [RFC4301-NLP-1](#rfc4301-nlp-1) | The SPD entry sets the Local and Remote port selectors to OPAQUE when a Next Layer Protocol has no port selector. |
| [RFC4301-NLP-2](#rfc4301-nlp-2) | The SPD entry sets both the Local and Remote port selectors to the same specified value when a system is willing to send and receive traffic with that port value. |
| [RFC4301-NLP-3](#rfc4301-nlp-3) | The SPD entry sets the Local port selector to a specified value and the Remote port selector to OPAQUE when a system is willing to send, but not receive, traffic with that port value. |
| [RFC4301-NLP-4](#rfc4301-nlp-4) | The SPD entry sets the Local port selector to OPAQUE and the Remote port selector to a specified value when a system is willing to receive, but not send, traffic with that port value. |
| [RFC4301-NLP-5](#rfc4301-nlp-5) | The SPD entry sets the Local port selector to a specific value and the Remote port selector to OPAQUE when a system is willing to send, but not receive, traffic with that particular port value, for a Next Layer Protocol with two port selectors. |
| [RFC4301-NLP-6](#rfc4301-nlp-6) | The SPD entry sets the Local port selector to OPAQUE and the Remote port selector to a specific value when a system is willing to receive, but not send, traffic with that particular port value. |
| [RFC4301-SAD-1](#rfc4301-sad-1) | Each IPsec implementation keeps a Security Association Database, with one entry for each SA. |
| [RFC4301-SAD-2](#rfc4301-sad-2) | For outbound processing, an entry in the SPD-S part of the SPD cache points to the SAD entry for the SA. |
| [RFC4301-SAD-3](#rfc4301-sad-3) | For inbound processing of a unicast SA, the receiver looks up the SA by the SPI alone, or by the SPI together with the IPsec protocol type. |
| [RFC4301-SAD-4](#rfc4301-sad-4) | For inbound processing of a multicast SA, the receiver looks up the SA by the SPI together with the destination address, or by the SPI together with the destination and source addresses. |
| [RFC4301-SAD-5](#rfc4301-sad-5) | The receiver initially populates a SAD entry for an inbound SA with the selector value or values negotiated when the SA was created. |
| [RFC4301-SAD-6](#rfc4301-sad-6) | The receiver uses the negotiated selector values in the SAD entry to check that the header fields of an inbound packet match the SA's selectors, so verifying that the packet is consistent with the SA's policy. |
| [RFC4301-SAD-7](#rfc4301-sad-7) | The SAD can hold an entry for an SA with no corresponding SPD entry, because this document does not require the SAD to be cleared when the SPD changes, so an SAD entry can outlive the SPD entry that created it. |
| [RFC4301-SAD-8](#rfc4301-sad-8) | A manually keyed SA can have a SAD entry with no corresponding SPD entry. |
| [RFC4301-SAD-9](#rfc4301-sad-9) | The SAD supports a multicast SA when the SA is manually configured. |
| [RFC4301-SAD-10](#rfc4301-sad-10) | An outbound multicast SA has the same structure as a unicast SA, with the source address of the sender and the destination address of the multicast group. |
| [RFC4301-SAD-11](#rfc4301-sad-11) | An inbound multicast SA is configured with the source address of each peer authorized to transmit to that multicast SA. |
| [RFC4301-SAD-12](#rfc4301-sad-12) | A multicast group controller, not the receiver, provides the SPI value for a multicast SA. |
| [RFC4301-SAD-13](#rfc4301-sad-13) | A SAD entry for an inbound multicast SA is created only through manual configuration. |
| [RFC4301-SADI-1](#rfc4301-sadi-1) | Each SAD entry contains a Security Parameter Index that the receiving end of the SA selected to uniquely identify the SA. |
| [RFC4301-SADI-2](#rfc4301-sadi-2) | Each SAD entry contains a Sequence Number Counter used to generate the Sequence Number field of AH or ESP headers, 64 bits wide by default or 32 bits wide when negotiated. |
| [RFC4301-SADI-3](#rfc4301-sadi-3) | Each SAD entry contains a Sequence Counter Overflow flag that states whether counter overflow generates an auditable event and blocks further transmission on the SA, or whether the counter is allowed to roll over. |
| [RFC4301-SADI-4](#rfc4301-sadi-4) | The audit log entry for a Sequence Number Counter overflow event should include the SPI value, the current date and time, the Local Address, the Remote Address, and the selectors of the SAD entry. |
| [RFC4301-SADI-5](#rfc4301-sadi-5) | Each SAD entry contains an Anti-Replay Window, a 64-bit counter and bit-map used to detect a replayed inbound AH or ESP packet, ignored for an SA on which anti-replay has been disabled. |
| [RFC4301-SADI-6](#rfc4301-sadi-6) | A SAD entry contains an AH Authentication algorithm and key only when AH is supported. |
| [RFC4301-SADI-7](#rfc4301-sadi-7) | A SAD entry contains an ESP Encryption algorithm, key, mode and IV, unless a combined mode algorithm is used, in which case these fields do not apply. |
| [RFC4301-SADI-8](#rfc4301-sadi-8) | A SAD entry contains an ESP integrity algorithm and keys, unless the integrity service is not selected or a combined mode algorithm is used, in which case these fields do not apply. |
| [RFC4301-SADI-9](#rfc4301-sadi-9) | A SAD entry contains an ESP combined mode algorithm and keys only when a combined mode algorithm is used with ESP on that SA. |
| [RFC4301-SADI-10](#rfc4301-sadi-10) | Each SAD entry contains the Lifetime of the SA, a time interval, byte count, or both, after which the SA must be replaced or terminated, with the first to expire taking precedence when both are used. |
| [RFC4301-SADI-11](#rfc4301-sadi-11) | A compliant implementation supports both a time-based and a byte-count-based SA lifetime, and supports the simultaneous use of both on one SA. |
| [RFC4301-SADI-12](#rfc4301-sadi-12) | When byte count is used for the SA lifetime, the implementation should count the bytes to which the IPsec cryptographic algorithm is applied, including pad bytes. |
| [RFC4301-SADI-13](#rfc4301-sadi-13) | An implementation must be able to handle the byte counters at the two ends of an SA becoming out of synch with each other. |
| [RFC4301-SADI-14](#rfc4301-sadi-14) | An SA lifetime should have both a soft lifetime, which warns the implementation to start setting up a replacement SA, and a hard lifetime, at which the SA ends and is destroyed. |
| [RFC4301-SADI-15](#rfc4301-sadi-15) | A packet not fully delivered before its SA's lifetime ends should be discarded. |
| [RFC4301-SADI-16](#rfc4301-sadi-16) | Each SAD entry contains the IPsec protocol mode, tunnel or transport, that states which mode of AH or ESP applies to traffic on the SA. |
| [RFC4301-SADI-17](#rfc4301-sadi-17) | Each SAD entry contains a Stateful fragment checking flag that states whether stateful fragment checking applies to the SA. |
| [RFC4301-SADI-18](#rfc4301-sadi-18) | A tunnel-mode SAD entry with both inner and outer headers in IPv4 contains a Bypass DF bit. |
| [RFC4301-SADI-19](#rfc4301-sadi-19) | A SAD entry's DSCP values, when one or more are specified, select the SA among several that otherwise match an outbound packet's traffic selectors; when none are specified, no DSCP-specific filtering applies. |
| [RFC4301-SADI-20](#rfc4301-sadi-20) | A SAD entry's DSCP values are not checked against inbound traffic arriving on the SA. |
| [RFC4301-SADI-21](#rfc4301-sadi-21) | A tunnel-mode SAD entry contains a Bypass DSCP flag, or a map from inner-header DSCP values to outer-header DSCP values, to restrict which DSCP values bypass mapping. |
| [RFC4301-SADI-22](#rfc4301-sadi-22) | Each SAD entry contains a Path MTU data item that holds any observed path MTU and its aging variables. |
| [RFC4301-SADI-23](#rfc4301-sadi-23) | A tunnel-mode SAD entry contains a Tunnel header IP source and destination address, both of the same IP version. |
| [RFC4301-PFP-1](#rfc4301-pfp-1) | When the PFP flag is 0, the resulting local-address selector in the SAD entry equals the SPD entry's local-address selector. |
| [RFC4301-PFP-2](#rfc4301-pfp-2) | When the PFP flag is 1, the resulting local-address selector in the SAD entry equals the source address of the packet that triggered the SA. |
| [RFC4301-PFP-3](#rfc4301-pfp-3) | When the PFP flag is 0, the resulting remote-address selector in the SAD entry equals the SPD entry's remote-address selector. |
| [RFC4301-PFP-4](#rfc4301-pfp-4) | When the PFP flag is 1, the resulting remote-address selector in the SAD entry equals the destination address of the packet that triggered the SA. |
| [RFC4301-PFP-5](#rfc4301-pfp-5) | When the PFP flag is 0 and the triggering packet has a protocol value, the resulting protocol selector in the SAD entry equals the SPD entry's protocol selector. |
| [RFC4301-PFP-6](#rfc4301-pfp-6) | When the PFP flag is 0, the SPD entry's protocol selector is a list of protocols, and the triggering packet has no available protocol value, the packet is discarded. |
| [RFC4301-PFP-7](#rfc4301-pfp-7) | When the PFP flag is 0, the SPD entry's protocol selector is ANY or OPAQUE, and the triggering packet has no available protocol value, the resulting protocol selector in the SAD entry keeps the SPD entry's value and the packet is not discarded on this basis. |
| [RFC4301-PFP-8](#rfc4301-pfp-8) | When the PFP flag is 1, the SPD entry's protocol selector is a list of protocols or ANY, and the triggering packet has a protocol value, the resulting protocol selector in the SAD entry narrows to that packet's protocol value. |
| [RFC4301-PFP-9](#rfc4301-pfp-9) | The protocol selector value OPAQUE applies only to IPv6; the protocol field cannot be OPAQUE in IPv4. |
| [RFC4301-PFP-10](#rfc4301-pfp-10) | When the PFP flag is 1, the SPD entry's protocol selector is a list of protocols or ANY, and the triggering packet has no available protocol value, the packet is discarded. |
| [RFC4301-PFP-11](#rfc4301-pfp-11) | Setting the PFP flag to 1 together with an OPAQUE selector value is an error, and an IPsec implementation should prohibit it. |
| [RFC4301-PFP-12](#rfc4301-pfp-12) | When the PFP flag is 0 and the triggering packet has a source port value, the resulting local-port selector in the SAD entry equals the SPD entry's local-port selector. |
| [RFC4301-PFP-13](#rfc4301-pfp-13) | When the PFP flag is 0, the SPD entry's local-port selector is a list of ranges, and the triggering packet has no available source port value, the packet is discarded. |
| [RFC4301-PFP-14](#rfc4301-pfp-14) | When the PFP flag is 0, the SPD entry's local-port selector is ANY or OPAQUE, and the triggering packet has no available source port value, the resulting local-port selector in the SAD entry keeps the SPD entry's value. |
| [RFC4301-PFP-15](#rfc4301-pfp-15) | When the PFP flag is 1, the SPD entry's local-port selector is a list of ranges or ANY, and the triggering packet has a source port value, the resulting local-port selector in the SAD entry narrows to that packet's source port value. |
| [RFC4301-PFP-16](#rfc4301-pfp-16) | When the PFP flag is 1, the SPD entry's local-port selector is a list of ranges or ANY, and the triggering packet has no available source port value, the packet is discarded. |
| [RFC4301-PFP-17](#rfc4301-pfp-17) | When the PFP flag is 0 and the triggering packet has a destination port value, the resulting remote-port selector in the SAD entry equals the SPD entry's remote-port selector. |
| [RFC4301-PFP-18](#rfc4301-pfp-18) | When the PFP flag is 0, the SPD entry's remote-port selector is a list of ranges, and the triggering packet has no available destination port value, the packet is discarded. |
| [RFC4301-PFP-19](#rfc4301-pfp-19) | When the PFP flag is 0, the SPD entry's remote-port selector is ANY or OPAQUE, and the triggering packet has no available destination port value, the resulting remote-port selector in the SAD entry keeps the SPD entry's value. |
| [RFC4301-PFP-20](#rfc4301-pfp-20) | When the PFP flag is 1, the SPD entry's remote-port selector is a list of ranges or ANY, and the triggering packet has a destination port value, the resulting remote-port selector in the SAD entry narrows to that packet's destination port value. |
| [RFC4301-PFP-21](#rfc4301-pfp-21) | When the PFP flag is 1, the SPD entry's remote-port selector is a list of ranges or ANY, and the triggering packet has no available destination port value, the packet is discarded. |
| [RFC4301-PFP-22](#rfc4301-pfp-22) | When the PFP flag is 0 and the triggering packet has a Mobility Header type value, the resulting mh-type selector in the SAD entry equals the SPD entry's mh-type selector. |
| [RFC4301-PFP-23](#rfc4301-pfp-23) | When the PFP flag is 0, the SPD entry's mh-type selector is a list of ranges, and the triggering packet has no available Mobility Header type value, the packet is discarded. |
| [RFC4301-PFP-24](#rfc4301-pfp-24) | When the PFP flag is 0, the SPD entry's mh-type selector is ANY or OPAQUE, and the triggering packet has no available Mobility Header type value, the resulting mh-type selector in the SAD entry keeps the SPD entry's value. |
| [RFC4301-PFP-25](#rfc4301-pfp-25) | When the PFP flag is 1, the SPD entry's mh-type selector is a list of ranges or ANY, and the triggering packet has a Mobility Header type value, the resulting mh-type selector in the SAD entry narrows to that packet's Mobility Header type value. |
| [RFC4301-PFP-26](#rfc4301-pfp-26) | When the PFP flag is 1, the SPD entry's mh-type selector is a list of ranges or ANY, and the triggering packet has no available Mobility Header type value, the packet is discarded. |
| [RFC4301-PFP-27](#rfc4301-pfp-27) | The ICMP type-and-code selector is a 16-bit value in which the code applies to the particular type it is bound to, and it can hold a single type with a range of codes, a single type with ANY code, or ANY type with ANY code. |
| [RFC4301-PFP-28](#rfc4301-pfp-28) | When the PFP flag is 0 and the triggering packet has an ICMP type and code value, the resulting ICMP type-and-code selector in the SAD entry equals the SPD entry's selector. |
| [RFC4301-PFP-29](#rfc4301-pfp-29) | When the PFP flag is 0, the SPD entry's ICMP selector names a single type, and the triggering packet has no available ICMP type and code, the packet is discarded. |
| [RFC4301-PFP-30](#rfc4301-pfp-30) | When the PFP flag is 0, the SPD entry's ICMP selector is ANY type with ANY code, or OPAQUE, and the triggering packet has no available ICMP type and code, the resulting ICMP selector in the SAD entry keeps the SPD entry's value. |
| [RFC4301-PFP-31](#rfc4301-pfp-31) | When the PFP flag is 1, the SPD entry's ICMP selector names a single type or ANY type, and the triggering packet has an ICMP type and code, the resulting ICMP selector in the SAD entry narrows to that packet's type and code. |
| [RFC4301-PFP-32](#rfc4301-pfp-32) | When the PFP flag is 1, the SPD entry's ICMP selector names a single type or ANY type, and the triggering packet has no available ICMP type and code, the packet is discarded. |
| [RFC4301-PFP-33](#rfc4301-pfp-33) | For the name selector, the SPD entry holds a list of user or system names, and the PFP flag, the triggering packet's value, and the resulting SAD entry do not apply. |
| [RFC4301-PROC-1](#rfc4301-proc-1) | An IPsec implementation must consult the SPD, or its associated caches, for all traffic that crosses the IPsec protection boundary, including IPsec management traffic. |
| [RFC4301-PROC-2](#rfc4301-proc-2) | An IPsec implementation must discard a packet, inbound or outbound, when no SPD policy matches it. |
| [RFC4301-PROC-3](#rfc4301-proc-3) | A cached SPD entry indicates whether matching traffic is bypassed or discarded. |
| [RFC4301-OUT-1](#rfc4301-out-1) | On an outbound packet, the implementation invokes the SPD selection function to obtain the SPD-ID that chooses the SPD to use. |
| [RFC4301-OUT-2](#rfc4301-out-2) | The implementation matches an outbound packet's headers against the SPD-O/SPD-S cache for the SPD-ID chosen for it. |
| [RFC4301-OUT-3](#rfc4301-out-3) | On a cache match, the implementation processes the outbound packet as the matching cache entry specifies: BYPASS, DISCARD, or PROTECT using AH or ESP. |
| [RFC4301-OUT-4](#rfc4301-out-4) | When IPsec processing applies to an outbound packet, the SPD cache entry links to the SAD entry that supplies the mode, cryptographic algorithms, keys, SPI, and PMTU used to protect the packet. |
| [RFC4301-OUT-5](#rfc4301-out-5) | The SA's PMTU value, the stateful fragment-checking flag, and the packet's DF bit together decide whether an outbound packet is fragmented before or after IPsec processing, or discarded with an ICMP PMTU message sent. |
| [RFC4301-OUT-6](#rfc4301-out-6) | On a cache miss, the implementation searches the SPD-S and SPD-O parts of the SPD identified by the SPD-ID. |
| [RFC4301-OUT-7](#rfc4301-out-7) | When the matching SPD entry calls for BYPASS or DISCARD, the implementation creates new outbound SPD cache entries, and for BYPASS also new inbound SPD cache entries. |
| [RFC4301-OUT-8](#rfc4301-out-8) | When the matching SPD entry calls for PROTECT, the implementation invokes the key management mechanism to create the SA. |
| [RFC4301-OUT-9](#rfc4301-out-9) | When SA creation for a PROTECT match succeeds, the implementation creates a new outbound SPD-S cache entry together with outbound and inbound SAD entries. |
| [RFC4301-OUT-10](#rfc4301-out-10) | When SA creation for a PROTECT match fails, the implementation discards the packet. |
| [RFC4301-OUT-11](#rfc4301-out-11) | A packet that triggers an SPD lookup may be discarded, or may instead be processed against the newly created cache entry if one was created. |
| [RFC4301-OUT-12](#rfc4301-out-12) | The inbound SAD entry created alongside an outbound SA holds the selector values, derived from the SPD entry and, where PFP flags were set, the packet, used to check inbound traffic on that SA. |
| [RFC4301-OUT-13](#rfc4301-out-13) | The implementation passes the outbound packet to the outbound forwarding function, outside the IPsec implementation, to select the exit interface. |
| [RFC4301-OUT-14](#rfc4301-out-14) | The outbound forwarding function may loop a packet back across the IPsec boundary for further IPsec processing, for example to support nested SAs. |
| [RFC4301-OUT-15](#rfc4301-out-15) | When the forwarding function loops a packet back for nested-SA processing, the SPD-I database must have an entry permitting inbound bypass of the packet, or the packet is discarded. |
| [RFC4301-OUT-16](#rfc4301-out-16) | When more than one SPD-I exists, traffic looped back across the IPsec boundary may be tagged as coming from the internal interface. |
| [RFC4301-OUT-17](#rfc4301-out-17) | Outside IPv4 and IPv6 transport mode, an SG, BITS, or BITW implementation may fragment a packet before applying IPsec. |
| [RFC4301-OUT-18](#rfc4301-out-18) | The device should have a configuration setting to disable fragmentation before IPsec processing. |
| [RFC4301-OUT-19](#rfc4301-out-19) | Fragments produced before IPsec processing are evaluated against the SPD the same way as any other packet. |
| [RFC4301-OUT-20](#rfc4301-out-20) | A fragment carrying no port numbers, ICMP type and code, or Mobility Header type matches only an SPD rule whose corresponding selector is OPAQUE or ANY. |
| [RFC4301-OUT-21](#rfc4301-out-21) | The IPsec system must determine and enforce an SA's PMTU following the steps of Section 8.2. |
| [RFC4301-DISC-1](#rfc4301-disc-1) | When an IPsec system must discard an outbound packet, it should be capable of generating and sending an ICMP message telling the packet's sender that it was discarded. |
| [RFC4301-DISC-2](#rfc4301-disc-2) | The reason for discarding an outbound packet should be recorded in the audit log. |
| [RFC4301-DISC-3](#rfc4301-disc-3) | The audit log entry for a discarded outbound packet should include the reason, the current date and time, and the packet's selector values. |
| [RFC4301-DISC-4](#rfc4301-disc-4) | When an outbound packet is discarded because its selectors matched an SPD entry requiring discard, an IPv4 system reports ICMP type 3, code 13. |
| [RFC4301-DISC-5](#rfc4301-disc-5) | When an outbound packet is discarded because its selectors matched an SPD entry requiring discard, an IPv6 system reports ICMP type 1, code 1. |
| [RFC4301-DISC-6](#rfc4301-disc-6) | When an outbound packet is discarded because the IPsec system reached the remote peer but could not negotiate the required SA, an IPv4 system reports ICMP type 3, code 13. |
| [RFC4301-DISC-7](#rfc4301-disc-7) | When an outbound packet is discarded because the IPsec system reached the remote peer but could not negotiate the required SA, an IPv6 system reports ICMP type 1, code 1. |
| [RFC4301-DISC-8](#rfc4301-disc-8) | When an outbound packet is discarded because the IPsec peer could not be contacted, an IPv4 system reports ICMP type 3, code 1. |
| [RFC4301-DISC-9](#rfc4301-disc-9) | When an outbound packet is discarded because the IPsec peer could not be contacted, an IPv6 system reports ICMP type 1, code 3. |
| [RFC4301-DISC-10](#rfc4301-disc-10) | A security gateway should have a management control letting an administrator configure whether it sends these ICMP messages reporting discarded outbound packets. |
| [RFC4301-DISC-11](#rfc4301-disc-11) | When a security gateway is configured to send these ICMP messages, it should rate-limit their transmission. |
| [RFC4301-TUN-1](#rfc4301-tun-1) | In tunnel mode, the outer IP header's source and destination addresses identify the tunnel endpoints, while the inner header's source and destination addresses identify the datagram's original sender and recipient. |
| [RFC4301-TUN-2](#rfc4301-tun-2) | In tunnel mode, the inner IP header stays unchanged during delivery to the tunnel exit point, except for the fields noted for TTL, or Hop Limit, and the DS/ECN fields. |
| [RFC4301-TUN-3](#rfc4301-tun-3) | In tunnel mode, no IP options or extension headers in the inner header change during delivery through the tunnel. |
| [RFC4301-TUN-4](#rfc4301-tun-4) | An IPsec implementation may be configurable in how it processes the outer DS field of a transmitted tunnel-mode packet. |
| [RFC4301-TUN-5](#rfc4301-tun-5) | The outer DS field of a transmitted tunnel-mode packet may instead be mapped to a fixed value, and that mapping may be set on a per-SA basis. |
| [RFC4301-TUN-6](#rfc4301-tun-6) | By default, an implementation does not let the outer DS value propagate from an unprotected domain into a protected domain. |
| [RFC4301-TUN-7](#rfc4301-tun-7) | An IPsec implementation may be configurable in how it processes the outer DS field of a received tunnel-mode packet. |
| [RFC4301-TUN-8](#rfc4301-tun-8) | A received tunnel-mode packet's outer DS field may be configured to be discarded, the default, or to overwrite the inner DS field. |
| [RFC4301-TUN-9](#rfc4301-tun-9) | Where discard-versus-overwrite of the outer DS field is offered, it may be configured on a per-SA basis. |
| [RFC4301-TUN-10](#rfc4301-tun-10) | In tunnel mode, the encapsulating header's IP version is allowed to differ from the inner header's IP version. |
| [RFC4301-TUN-11](#rfc4301-tun-11) | In tunnel mode, the encapsulator sets the outer header's IP version, 4 for IPv4 or 6 for IPv6, which may differ from the inner header's version, and the decapsulator leaves the inner header's version unchanged. |
| [RFC4301-TUN-12](#rfc4301-tun-12) | In tunnel mode, the encapsulator copies the outer header's DS field from the inner header. |
| [RFC4301-TUN-13](#rfc4301-tun-13) | When a tunnel-mode packet is about to enter a domain for which the outer DSCP value is not appropriate, that value must be mapped to a value appropriate for the domain. |
| [RFC4301-TUN-14](#rfc4301-tun-14) | In tunnel mode, the encapsulator copies the outer header's ECN field from the inner header, and the decapsulator constructs the inner header's ECN field from the outer and inner values. |
| [RFC4301-TUN-15](#rfc4301-tun-15) | The decapsulator sets the inner header's ECN field to Congestion Experienced when the inner field was ECT(0) or ECT(1) and the outer field is Congestion Experienced; otherwise it leaves the inner ECN field unchanged. |
| [RFC4301-TUN-16](#rfc4301-tun-16) | The encapsulator decrements the inner header's TTL, or Hop Limit, before forwarding the packet into the tunnel. |
| [RFC4301-TUN-17](#rfc4301-tun-17) | The decapsulator decrements the inner header's TTL, or Hop Limit, only if it forwards the packet after removing the tunnel header. |
| [RFC4301-TUN-18](#rfc4301-tun-18) | In IPv4 tunnel mode, the encapsulator constructs the outer header checksum, and the decapsulator reconstructs the inner header checksum whenever the TTL or ECN field changes. |
| [RFC4301-TUN-19](#rfc4301-tun-19) | In tunnel mode, the encapsulator constructs the outer header's source and destination addresses from the SA: the SA's remote address in turn determines the local address, or interface, used to forward the packet. |
| [RFC4301-TUN-20](#rfc4301-tun-20) | For multicast tunnel-mode traffic, the encapsulator keeps the outer source address consistent over the life of the SA, matching the address negotiated when the SA was established, except that a mobile IPsec implementation updates its source address as it moves. |
| [RFC4301-TUN-21](#rfc4301-tun-21) | In IPv4 tunnel mode, the encapsulator constructs the outer header's header-length field independently of the inner header. |
| [RFC4301-TUN-22](#rfc4301-tun-22) | In IPv4 tunnel mode, the encapsulator constructs the outer header's total-length field independently of the inner header. |
| [RFC4301-TUN-23](#rfc4301-tun-23) | In IPv4 tunnel mode, the encapsulator constructs the outer header's identification field independently of the inner header. |
| [RFC4301-TUN-24](#rfc4301-tun-24) | In IPv4 tunnel mode, the encapsulator constructs the outer header's flags field, and configuration decides whether the DF bit is copied from the inner header, cleared, or set. |
| [RFC4301-TUN-25](#rfc4301-tun-25) | In IPv4 tunnel mode, the encapsulator constructs the outer header's fragment-offset field independently of the inner header. |
| [RFC4301-TUN-26](#rfc4301-tun-26) | In IPv4 tunnel mode, the encapsulator sets the outer header's protocol field to AH or ESP. |
| [RFC4301-TUN-27](#rfc4301-tun-27) | In IPv4 tunnel mode, the encapsulator never copies IP options from the inner header into the outer header and does not itself construct outer-header options, though code outside IPsec processing may insert or construct them. |
| [RFC4301-TUN-28](#rfc4301-tun-28) | In IPv6 tunnel mode, the encapsulator sets the outer header's flow label by copying it from the inner header or from configuration. |
| [RFC4301-TUN-29](#rfc4301-tun-29) | Copying the flow label from the inner header to the outer header is acceptable only for end systems, not for security gateways, because a security gateway that did so risks flow-label collisions. |
| [RFC4301-TUN-30](#rfc4301-tun-30) | In IPv6 tunnel mode, the encapsulator constructs the outer header's payload-length field independently of the inner header. |
| [RFC4301-TUN-31](#rfc4301-tun-31) | In IPv6 tunnel mode, the encapsulator sets the outer header's next-header field to AH, ESP, or a routing header. |
| [RFC4301-TUN-32](#rfc4301-tun-32) | In IPv6 tunnel mode, the encapsulator never copies extension headers from the inner header into the outer header and does not itself construct outer extension headers, though code outside IPsec processing may insert or construct them. |
| [RFC4301-TUN-33](#rfc4301-tun-33) | An IPv6 tunnel-mode implementation may offer a per-SA facility to pass the outer header's DS value into the inner header of a received packet, but by default it does not. |
| [RFC4301-IN-1](#rfc4301-in-1) | The inbound SPD cache, SPD-I, is applied only to traffic that is bypassed or discarded, not to IPsec-protected traffic. |
| [RFC4301-IN-2](#rfc4301-in-2) | A packet that fails to match any entry of an SPD cache is then referred to the corresponding SPD. |
| [RFC4301-IN-3](#rfc4301-in-3) | Every SPD should have a nominal, final entry that catches and discards anything otherwise unmatched. |
| [RFC4301-IN-4](#rfc4301-in-4) | Non-IPsec-protected traffic that arrives and matches no SPD-I entry is discarded. |
| [RFC4301-IN-5](#rfc4301-in-5) | Any IP fragments arriving via the unprotected interface are reassembled before AH or ESP processing. |
| [RFC4301-IN-6](#rfc4301-in-6) | An inbound IP datagram needing IPsec processing is identified by the appearance of the AH or ESP value in the IP Next Protocol field, or as a next-layer protocol in the IPv6 context. |
| [RFC4301-IN-7](#rfc4301-in-7) | On arrival, a packet may be tagged with the ID of the interface it arrived on, when needed to support multiple SPDs and their SPD-I caches, and that interface ID maps to an SPD-ID. |
| [RFC4301-IN-8](#rfc4301-in-8) | If an inbound packet appears IPsec protected and is addressed to the device, the implementation attempts to map it to an active SA via the SAD. |
| [RFC4301-IN-9](#rfc4301-in-9) | A device may have multiple IP addresses that may be used in the SAD lookup, for example for protocols such as SCTP. |
| [RFC4301-IN-10](#rfc4301-in-10) | Traffic not addressed to the device, or addressed to it but not AH or ESP, is directed to SPD-I lookup. |
| [RFC4301-IN-11](#rfc4301-in-11) | IKE traffic must have an explicit BYPASS entry in the SPD. |
| [RFC4301-IN-12](#rfc4301-in-12) | When multiple SPDs are employed, the interface tag assigned on arrival selects the appropriate SPD-I, and its cache, to search. |
| [RFC4301-IN-13](#rfc4301-in-13) | SPD-I lookup determines whether the action for the packet is DISCARD or BYPASS. |
| [RFC4301-IN-14](#rfc4301-in-14) | A packet addressed to the IPsec device with AH or ESP as its protocol is looked up in the SAD. |
| [RFC4301-IN-15](#rfc4301-in-15) | For unicast traffic, the SAD lookup uses only the SPI, or the SPI plus protocol. |
| [RFC4301-IN-16](#rfc4301-in-16) | For multicast traffic, the SAD lookup uses the SPI plus the destination address, or the SPI plus the destination and source addresses, as Section 4.1 specifies. |
| [RFC4301-IN-17](#rfc4301-in-17) | When a SAD lookup, unicast or multicast, finds no match, the traffic is discarded. |
| [RFC4301-IN-18](#rfc4301-in-18) | A SAD lookup that finds no match for an inbound AH or ESP packet is an auditable event. |
| [RFC4301-IN-19](#rfc4301-in-19) | The audit log entry for a failed SAD lookup should include the current date and time, the SPI, the source and destination of the packet, the IPsec protocol, and any other available selector values. |
| [RFC4301-IN-20](#rfc4301-in-20) | When the packet is found in the SAD, it is processed accordingly, applying AH or ESP processing as step 4 specifies. |
| [RFC4301-IN-21](#rfc4301-in-21) | A packet not addressed to the device, or addressed to it but not AH or ESP, has its header looked up in the appropriate SPD-I cache. |
| [RFC4301-IN-22](#rfc4301-in-22) | On an SPD-I cache match, the packet is discarded or bypassed as the entry says. |
| [RFC4301-IN-23](#rfc4301-in-23) | On an SPD-I cache miss, the packet is looked up in the corresponding SPD-I and a cache entry is created as appropriate. |
| [RFC4301-IN-24](#rfc4301-in-24) | No SA is created in response to an inbound packet that requires IPsec protection; only BYPASS or DISCARD cache entries can be created this way. |
| [RFC4301-IN-25](#rfc4301-in-25) | When an inbound packet matches no SPD-I entry, it is discarded. |
| [RFC4301-IN-26](#rfc4301-in-26) | An inbound packet discarded because it matched no SPD-I entry is an auditable event. |
| [RFC4301-IN-27](#rfc4301-in-27) | The audit log entry for an SPD-I no-match discard should include the current date and time, the SPI if available, the IPsec protocol if available, the source and destination of the packet, and any other available selector values. |
| [RFC4301-IN-28](#rfc4301-in-28) | Processing of ICMP messages is assumed to take place on the unprotected side of the IPsec boundary. |
| [RFC4301-IN-29](#rfc4301-in-29) | Unprotected ICMP messages are examined, and local policy decides whether to accept or reject them and, when accepted, what action to take. |
| [RFC4301-IN-30](#rfc4301-in-30) | On receipt of an ICMP unreachable message, the implementation decides whether to act on it, reject it, or act on it with constraints. |
| [RFC4301-IN-31](#rfc4301-in-31) | Step 4 applies AH or ESP processing using the SAD entry selected in step 3a. |
| [RFC4301-IN-32](#rfc4301-in-32) | Step 4 matches the packet against the inbound selectors identified by the SAD entry, to verify that the received packet is appropriate for the SA on which it arrived. |
| [RFC4301-IN-33](#rfc4301-in-33) | When a received packet's header fields are not consistent with the selectors for the SA it arrived on, the implementation must discard the packet. |
| [RFC4301-IN-34](#rfc4301-in-34) | Discarding an inbound packet for failing its SA's selector check is an auditable event. |
| [RFC4301-IN-35](#rfc4301-in-35) | The audit log entry for a selector-check discard should include the current date and time, the SPI, the IPsec protocol(s), the source and destination of the packet, any other available selector values of the packet, and the selector values from the relevant SAD entry. |
| [RFC4301-IN-36](#rfc4301-in-36) | The system should also be capable of sending an IKE notification of INVALID_SELECTORS to the sending IPsec peer, indicating the packet was discarded for failing selector checks. |
| [RFC4301-IN-37](#rfc4301-in-37) | The IPsec system should include a management control letting an administrator configure whether it sends the INVALID_SELECTORS IKE notification. |
| [RFC4301-IN-38](#rfc4301-in-38) | When the INVALID_SELECTORS notification facility is selected, the system should rate-limit the transmission of such notifications. |
| [RFC4301-IN-39](#rfc4301-in-39) | After traffic is bypassed or processed through IPsec, it is handed to the inbound forwarding function for disposition. |
| [RFC4301-IN-40](#rfc4301-in-40) | The inbound forwarding function may send a packet back out across the IPsec boundary for additional inbound processing, for example to support nested SAs. |
| [RFC4301-IN-41](#rfc4301-in-41) | When inbound forwarding loops a packet back across the IPsec boundary, the packet must be matched against an SPD-O entry, as with all outbound traffic to be bypassed. |
| [RFC4301-CONF-1](#rfc4301-conf-1) | All IPv4 IPsec implementations must comply with all requirements of this document. |
| [RFC4301-CONF-2](#rfc4301-conf-2) | All IPv6 IPsec implementations must comply with all requirements of this document. |

## OVW: What IPsec does and how it works

### RFC4301-OVW-1

**At the IPsec boundary, access controls let a packet cross unimpeded, receive AH or ESP security services, or be discarded.**

> "These controls indicate whether packets cross the boundary unimpeded, are afforded security services via AH or ESP, or are discarded." — §3.1, `rfc4301.txt:389-390`

- Strength: description. Class: end-to-end.
- Check idea: Send packets that match each of the three policy outcomes across the IPsec boundary and observe that each packet crosses unmodified, arrives protected by AH or ESP, or does not arrive.

### RFC4301-OVW-2

**A host implementation must support connectivity between two hosts and between a security gateway and a host.**

> "A compliant host implementation MUST support (a) and (c)" — §3.1, `rfc4301.txt:403-404`

- Strength: must. Class: internal.
- Check idea: Configure a host implementation to communicate with another host, and with a security gateway, and confirm the implementation supports both paths.

### RFC4301-OVW-3

**A security gateway must support all three connectivity paths: host to host, gateway to gateway, and gateway to host.**

> "a compliant security gateway must support all three of these forms of connectivity" — §3.1, `rfc4301.txt:404-405`

- Strength: must (lower case). Class: internal.
- Check idea: Configure a security gateway to communicate host to host, gateway to gateway, and gateway to host, and confirm it supports all three paths.

### RFC4301-OVW-4

**An IPsec implementation may support more than one interface on either side of the boundary, or on both.**

> "An IPsec implementation may support more than one interface on either or both sides of the boundary." — §3.1, `rfc4301.txt:441-443`

- Strength: may (lower case). Class: internal.
- Check idea: Configure the implementation with more than one interface on the protected side, the unprotected side, or both, and confirm it accepts the configuration and classifies traffic on each interface as inbound or outbound.

### RFC4301-OVW-5

**An IPsec implementation must support ESP, and may support AH.**

> "IPsec implementations MUST support ESP and MAY support AH." — §3.2, `rfc4301.txt:470-471`

- Strength: must (ESP), may (AH). Class: internal.
- Check idea: Attempt to configure the implementation to establish an ESP SA and confirm it succeeds; attempt to configure an AH SA and observe whether the implementation accepts or rejects AH support.

### RFC4301-OVW-6

**AH offers integrity and data origin authentication for the traffic it protects.**

> "The IP Authentication Header (AH) [Ken05b] offers integrity and data origin authentication" — §3.2, `rfc4301.txt:477-478`

- Strength: description. Class: end-to-end.
- Check idea: Establish an AH security association between two IPsec implementations and confirm the traffic sent over it is protected against modification and against a false origin.

### RFC4301-OVW-7

**A receiver may turn on the anti-replay feature of AH at its own discretion.**

> "with optional (at the discretion of the receiver) anti-replay features" — §3.2, `rfc4301.txt:478-479`

- Strength: may. Class: internal.
- Check idea: Configure the receiving side of an AH security association with the anti-replay feature on, or off, and confirm the implementation accepts either choice.

### RFC4301-OVW-8

**ESP offers the same services as AH, and also offers confidentiality.**

> "The Encapsulating Security Payload (ESP) protocol [Ken05a] offers the same set of services, and also offers confidentiality." — §3.2, `rfc4301.txt:481-482`

- Strength: description. Class: end-to-end.
- Check idea: Establish an ESP security association with confidentiality and integrity both on, and confirm the traffic it carries gets integrity, data origin authentication, and confidentiality.

### RFC4301-OVW-9

**An IPsec implementation should not use ESP to provide confidentiality without integrity.**

> "Use of ESP to provide confidentiality without integrity is NOT RECOMMENDED." — §3.2, `rfc4301.txt:482-484`

- Strength: should not. Class: internal.
- Check idea: Configure an ESP SA with confidentiality on and integrity off, and confirm this is not the implementation's default or recommended configuration.

### RFC4301-OVW-10

**AH and ESP provide access control, enforced by cryptographic key distribution and by the traffic-flow management the Security Policy Database dictates.**

> "Both AH and ESP offer access control, enforced through the distribution of cryptographic keys and the management of traffic flows as dictated by the Security Policy Database (SPD, Section 4.4.1)." — §3.2, `rfc4301.txt:491-494`

- Strength: description. Class: end-to-end.
- Check idea: Configure SPD entries that permit, protect, or discard traffic, and confirm that AH and ESP security associations enforce exactly those access-control decisions.

### RFC4301-OVW-11

**An IPsec implementation supports both manual and automated distribution of keys.**

> "This document requires support for both manual and automated distribution of keys." — §3.2, `rfc4301.txt:532-533`

- Strength: description. Class: internal.
- Check idea: Configure a security association with manual keys and, separately, with an automated key management protocol, and confirm the implementation accepts both.

### RFC4301-OVW-12

**An implementation may use an automated key distribution technique other than IKEv2.**

> "other automated key distribution techniques MAY be used." — §3.2, `rfc4301.txt:535-536`

- Strength: may. Class: internal.
- Check idea: Configure the implementation with an automated key management protocol other than IKEv2 and confirm it accepts the resulting security association.

## SA: Security associations and their modes

### RFC4301-SA-1

**Every AH or ESP implementation must support the concept of a security association.**

> "All implementations of AH or ESP MUST support the concept of an SA as described below." — §4, `rfc4301.txt:612-613`

- Strength: must. Class: internal.
- Check idea: Inspect how the implementation creates, looks up, and deletes AH or ESP security associations, and confirm it maintains an SA construct as the specification describes.

### RFC4301-SA-2

**A security association is simplex: it applies its security service in one direction only.**

> "An SA is a simplex "connection" that affords security services to the traffic carried by it." — §4.1, `rfc4301.txt:629-630`

- Strength: description. Class: internal.
- Check idea: Establish one security association and confirm it protects traffic in only one direction; the return traffic needs a separate SA.

### RFC4301-SA-3

**A single security association uses either AH or ESP, never both together.**

> "Security services are afforded to an SA by the use of AH, or ESP, but not both." — §4.1, `rfc4301.txt:630-631`

- Strength: description. Class: internal.
- Check idea: Attempt to configure one security association with both AH and ESP protection at once, and confirm the implementation requires two separate SAs instead.

### RFC4301-SA-4

**When a traffic stream needs both AH and ESP protection, an implementation creates two SAs and coordinates them to apply the protocols in turn.**

> "If both AH and ESP protection are applied to a traffic stream, then two SAs must be created and coordinated to effect protection through iterated application of the security protocols." — §4.1, `rfc4301.txt:631-634`

- Strength: must (lower case). Class: internal.
- Check idea: Configure a traffic stream that needs both AH and ESP protection, and confirm the implementation applies two separate, coordinated SAs one after the other.

### RFC4301-SA-5

**Bidirectional communication between two IPsec-enabled systems needs a pair of SAs, one for each direction.**

> "To secure typical, bi-directional communication between two IPsec-enabled systems, a pair of SAs (one in each direction) is required." — §4.1, `rfc4301.txt:634-636`

- Strength: description. Class: internal.
- Check idea: Set up two-way traffic between two IPsec-enabled systems and confirm the implementation creates two SAs, one for each direction.

### RFC4301-SA-6

**For a security association that carries unicast traffic, the Security Parameters Index alone identifies the SA.**

> "For an SA used to carry unicast traffic, the Security Parameters Index (SPI) by itself suffices to specify an SA." — §4.1, `rfc4301.txt:639-640`

- Strength: description. Class: internal.
- Check idea: Configure a unicast SA and confirm the implementation looks it up by the SPI value alone, without the destination or source address.

### RFC4301-SA-7

**As a local matter, an implementation may use the SPI together with the IPsec protocol type to identify a security association.**

> "as a local matter, an implementation may choose to use the SPI in conjunction with the IPsec protocol type (AH or ESP) for SA identification." — §4.1, `rfc4301.txt:642-644`

- Strength: may (lower case). Class: internal.
- Check idea: Configure two SAs that share one SPI value but use different protocols, AH and ESP, and confirm the implementation can tell them apart at look-up.

### RFC4301-SA-8

**An IPsec implementation that supports multicast must support multicast SAs, with the mapping algorithm this section gives for inbound IPsec datagrams.**

> "If an IPsec implementation supports multicast, then it MUST support multicast SAs using the algorithm below for mapping inbound IPsec datagrams to SAs." — §4.1, `rfc4301.txt:644-646`

- Strength: must. Class: internal.
- Check idea: On a multicast-capable implementation, send an inbound multicast IPsec datagram and confirm the implementation maps it to the correct SA with the specified algorithm.

### RFC4301-SA-9

**An implementation that supports only unicast traffic does not need the multicast de-multiplexing algorithm.**

> "Implementations that support only unicast traffic need not implement this de- multiplexing algorithm." — §4.1, `rfc4301.txt:646-648`

- Strength: description. Class: internal.
- Check idea: Confirm that a unicast-only implementation omits the multicast SA look-up algorithm and is still compliant.

### RFC4301-SA-10

**A multicast-capable IPsec implementation must de-multiplex inbound traffic correctly even when a GSA and a unicast SA share one SPI.**

> "A multicast-capable IPsec implementation MUST correctly de-multiplex inbound traffic even in the context of SPI collisions." — §4.1, `rfc4301.txt:656-658`

- Strength: must. Class: internal.
- Check idea: Configure a GSA and a unicast SA that use the same SPI value, send inbound traffic for each, and confirm the implementation delivers each packet to the correct SA.

### RFC4301-SA-11

**Each SAD entry states whether its SA look-up uses the destination address alone, or the destination and source addresses together with the SPI.**

> "Each entry in the SA Database (SAD) (Section 4.4.2) must indicate whether the SA lookup makes use of the destination IP address, or the destination and source IP addresses, in addition to the SPI." — §4.1, `rfc4301.txt:660-662`

- Strength: must (lower case). Class: internal.
- Check idea: Inspect a SAD entry and confirm it records whether its SA look-up uses destination-only or destination-and-source addressing, along with the SPI.

### RFC4301-SA-12

**For a multicast SA, the SAD look-up does not use the protocol field.**

> "For multicast SAs, the protocol field is not employed for SA lookups." — §4.1, `rfc4301.txt:662-663`

- Strength: description. Class: internal.
- Check idea: Look up a multicast SA in the SAD and confirm the search ignores the protocol field.

### RFC4301-SA-13

**For an inbound IPsec-protected packet, an implementation searches the SAD for the entry that matches the longest SA identifier.**

> "For each inbound, IPsec-protected packet, an implementation must conduct its search of the SAD such that it finds the entry that matches the "longest" SA identifier." — §4.1, `rfc4301.txt:664-666`

- Strength: must (lower case). Class: internal.
- Check idea: Configure two SAD entries that could both match an inbound packet's SPI, one with a longer identifier that also matches the address fields, and confirm the implementation picks the longer match.

### RFC4301-SA-14

**The entry that also matches the destination address, or the destination and source addresses, is the "longest" match among SPI matches.**

> "if two or more SAD entries match based on the SPI value, then the entry that also matches based on destination address, or destination and source address (as indicated in the SAD entry) is the "longest" match." — §4.1, `rfc4301.txt:666-669`

- Strength: description. Class: internal.
- Check idea: Configure several SAD entries that share one SPI but differ in which address fields they also match, and confirm the implementation ranks an address-matching entry above an SPI-only match.

### RFC4301-SA-15

**An implementation first searches the SAD for an entry that matches the SPI, destination address, and source address together.**

> "Search the SAD for a match on the combination of SPI, destination address, and source address. If an SAD entry matches, then process the inbound packet with that matching SAD entry." — §4.1, `rfc4301.txt:679-682`

- Strength: description. Class: internal.
- Check idea: Configure a SAD entry with a full SPI, destination, and source match for an inbound packet, and confirm the implementation selects that entry first.

### RFC4301-SA-16

**When no three-part match exists, an implementation searches the SAD for an entry that matches both SPI and destination address.**

> "Search the SAD for a match on both SPI and destination address. If the SAD entry matches, then process the inbound packet with that matching SAD entry." — §4.1, `rfc4301.txt:684-686`

- Strength: description. Class: internal.
- Check idea: Configure a SAD with no three-part match, but with an entry matching SPI and destination address, and confirm the implementation selects that entry.

### RFC4301-SA-17

**As a last step, an implementation matches on SPI alone with one shared AH/ESP SPI space, or on SPI and protocol with separate spaces.**

> "Search the SAD for a match on only SPI if the receiver has chosen to maintain a single SPI space for AH and ESP, and on both SPI and protocol, otherwise. If an SAD entry matches, then process the inbound packet with that matching SAD entry." — §4.1, `rfc4301.txt:688-691`

- Strength: description. Class: internal.
- Check idea: Configure the receiver first with one shared SPI space, then with separate AH and ESP SPI spaces, and confirm the implementation matches accordingly in each case.

### RFC4301-SA-18

**When no SAD entry matches at any of the three steps, an implementation discards the packet.**

> "Otherwise, discard the packet and log an auditable event." — §4.1, `rfc4301.txt:692`

- Strength: description. Class: end-to-end.
- Check idea: Present an inbound packet that matches no SAD entry at any of the three search steps, and confirm the implementation drops it.

### RFC4301-SA-19

**When no SAD entry matches at any of the three steps, an implementation logs an auditable event.**

> "Otherwise, discard the packet and log an auditable event." — §4.1, `rfc4301.txt:692`

- Strength: description. Class: internal.
- Check idea: Present an inbound packet that matches no SAD entry at any step, and confirm the implementation records an auditable log entry for it.

### RFC4301-SA-20

**An implementation may use any SAD search method, or none, as long as its externally visible behavior matches the three-step search order.**

> "an implementation may choose any method (or none at all) to accelerate this search, although its externally visible behavior MUST be functionally equivalent to having searched the SAD in the above order." — §4.1, `rfc4301.txt:694-697`

- Strength: may (the search method), must (the externally visible behavior). Class: end-to-end.
- Check idea: Configure SAD entries with several candidate matches, and confirm that, for a range of inbound packets, the implementation always picks the entry the three-step order would pick.

### RFC4301-SA-21

**An implementation sets its indication of source-and-destination address matching either by manual SA configuration, or by an SA management protocol.**

> "The indication of whether source and destination address matching is required to map inbound IPsec traffic to SAs MUST be set either as a side effect of manual SA configuration or via negotiation using an SA management protocol" — §4.1, `rfc4301.txt:707-710`

- Strength: must. Class: internal.
- Check idea: Create an SA by manual configuration and, separately, through an SA management protocol, and confirm each stores an indication of whether inbound look-up needs address matching.

### RFC4301-SA-22

**A Source-Specific Multicast group SA typically uses a three-part identifier: SPI, destination multicast address, and source address.**

> "Typically, Source-Specific Multicast (SSM) [HC03] groups use a 3-tuple SA identifier composed of an SPI, a destination multicast address, and source address." — §4.1, `rfc4301.txt:711-713`

- Strength: description. Class: internal.
- Check idea: Configure an SSM group SA and confirm the SAD identifies it by the combination of SPI, destination multicast address, and source address.

### RFC4301-SA-23

**An Any-Source Multicast group SA needs only an SPI and a destination multicast address as its identifier.**

> "An Any-Source Multicast group SA requires only an SPI and a destination multicast address as an identifier." — §4.1, `rfc4301.txt:713-715`

- Strength: description. Class: internal.
- Check idea: Configure an Any-Source Multicast group SA and confirm the SAD identifies it by the SPI and destination multicast address alone, with no source address.

### RFC4301-SA-24

**A sender should put traffic of different classes, with the same selector values, on different SAs, to support Quality of Service.**

> "a sender SHOULD put traffic of different classes, but with the same selector values, on different SAs to support Quality of Service (QoS) appropriately." — §4.1, `rfc4301.txt:722-724`

- Strength: should. Class: internal.
- Check idea: Configure traffic of two classes that share the same selector values, and confirm the sender places them on separate SAs, not one shared SA.

### RFC4301-SA-25

**An IPsec implementation must let a sender establish and keep multiple SAs, with the same selectors, between one sender and receiver.**

> "To permit this, the IPsec implementation MUST permit establishment and maintenance of multiple SAs between a given sender and receiver, with the same selectors." — §4.1, `rfc4301.txt:725-726`, `rfc4301.txt:735`

- Strength: must. Class: internal.
- Check idea: Configure two SAs between the same sender and receiver, with identical selector values, one for each traffic class, and confirm the implementation allows both at once.

### RFC4301-SA-26

**A receiver must process packets from different parallel SAs without prejudice.**

> "The receiver MUST process the packets from the different SAs without prejudice." — §4.1, `rfc4301.txt:737-738`

- Strength: must. Class: end-to-end.
- Check idea: Send traffic to a receiver over two parallel SAs with the same selectors, and confirm the receiver processes packets from both without treating either as invalid.

### RFC4301-SA-27

**In tunnel mode, the DSCP value for QoS classification appears in the inner IP header.**

> "In the case of tunnel mode SAs, the DSCP values in question appear in the inner IP header." — §4.1, `rfc4301.txt:739-740`

- Strength: description. Class: wire.
- Check idea: Capture a tunnel mode packet and confirm the DSCP bits used for QoS classification are the ones in the inner IP header, not the outer one.

### RFC4301-SA-28

**In transport mode, an implementation must not check the DSCP value as part of SA or packet validation.**

> "the value is not employed for SA selection and MUST NOT be checked as part of SA/packet validation." — §4.1, `rfc4301.txt:742-744`

- Strength: must not. Class: internal.
- Check idea: Send transport mode packets whose DSCP value changes en route, and confirm the receiver still validates and delivers them without a DSCP check.

### RFC4301-SA-29

**Both SAs of an SA pair are the same mode: both transport mode, or both tunnel mode.**

> "we choose to require that both SAs in a pair be of the same mode, transport or tunnel." — §4.1, `rfc4301.txt:756-758`

- Strength: description. Class: internal.
- Check idea: Establish an SA pair and confirm both SAs of the pair use the same mode, either both transport or both tunnel.

### RFC4301-SA-30

**A transport mode SA is typically used between a pair of hosts, to protect their end-to-end traffic.**

> "A transport mode SA is an SA typically employed between a pair of hosts to provide end-to-end security services." — §4.1, `rfc4301.txt:760-761`

- Strength: description. Class: internal.
- Check idea: Establish a transport mode SA between two hosts and confirm it protects the traffic between them end to end.

### RFC4301-SA-31

**Transport mode may secure one segment of a path: between two security gateways, or between a security gateway and a host.**

> "When security is desired between two intermediate systems along a path (vs. end-to-end use of IPsec), transport mode MAY be used between security gateways or between a security gateway and a host." — §4.1, `rfc4301.txt:761-764`

- Strength: may. Class: internal.
- Check idea: Configure a transport mode SA between two security gateways, or between a gateway and a host, for one segment of a path, and confirm the implementation accepts it.

### RFC4301-SA-32

**When used between security gateways, or a gateway and a host, transport mode may carry an in-IP tunnel, such as IP-in-IP or GRE, or a dynamic routing protocol.**

> "transport mode may be used to support in-IP tunneling (e.g., IP-in-IP [Per96] or Generic Routing Encapsulation (GRE) tunneling [FaLiHaMeTr00] or dynamic routing [ToEgWa04]) over transport mode SAs." — §4.1, `rfc4301.txt:765-769`

- Strength: may (lower case). Class: internal.
- Check idea: Configure an in-IP tunnel, such as IP-in-IP or GRE, or a dynamic routing protocol, to run over a transport mode SA between two security gateways, and confirm the implementation carries it.

### RFC4301-SA-33

**An intermediate system, such as a security gateway, uses transport mode only for its own outbound source address or its own inbound destination address.**

> "the use of transport mode by an intermediate system (e.g., a security gateway) is permitted only when applied to packets whose source address (for outbound packets) or destination address (for inbound packets) is an address belonging to the intermediate system itself." — §4.1, `rfc4301.txt:769-773`

- Strength: description. Class: end-to-end.
- Check idea: Send a security gateway outbound traffic whose source address is its own, and then is not, and confirm transport mode applies only in the first case.

### RFC4301-SA-34

**In IPv4, a transport mode security protocol header sits right after the IP header and any options, and before the next layer protocol.**

> "In IPv4, a transport mode security protocol header appears immediately after the IP header and any options, and before any next layer protocols (e.g., TCP or UDP)." — §4.1, `rfc4301.txt:791-793`

- Strength: description. Class: wire.
- Check idea: Capture an IPv4 transport mode packet and confirm the AH or ESP header sits directly after the IP header and any options, and before the next layer protocol header.

### RFC4301-SA-35

**In IPv6, the security protocol header of a transport mode SA sits after the base IP header and any extension headers ordered ahead of it.**

> "In IPv6, the security protocol header appears after the base IP header and selected extension headers" — §4.1, `rfc4301.txt:793-795`

- Strength: description. Class: wire.
- Check idea: Capture an IPv6 transport mode packet and confirm the AH or ESP header follows the base IP header and any extension headers placed ahead of it.

### RFC4301-SA-36

**In IPv6, the transport mode security protocol header may sit before or after the destination options extension header.**

> "but may appear before or after destination options" — §4.1, `rfc4301.txt:794-795`

- Strength: may (lower case). Class: wire.
- Check idea: Build IPv6 transport mode packets both ways, and confirm the implementation accepts the security protocol header placed either before or after a destination options header.

### RFC4301-SA-37

**In IPv6, the transport mode security protocol header must sit before the next layer protocol header.**

> "it MUST appear before next layer protocols (e.g., TCP, UDP, Stream Control Transmission Protocol (SCTP))." — §4.1, `rfc4301.txt:795-797`

- Strength: must. Class: wire.
- Check idea: Capture an IPv6 transport mode packet and confirm the AH or ESP header comes before the next layer protocol header, such as TCP, UDP, or SCTP.

### RFC4301-SA-38

**In transport mode, an ESP SA protects only the next layer protocols, not the IP header or the extension headers ahead of the ESP header.**

> "In the case of ESP, a transport mode SA provides security services only for these next layer protocols, not for the IP header or any extension headers preceding the ESP header." — §4.1, `rfc4301.txt:797-800`

- Strength: description. Class: end-to-end.
- Check idea: Establish a transport mode ESP SA and confirm its protection covers the next layer protocol data, but not the IP header or the extension headers ahead of it.

### RFC4301-SA-39

**In transport mode, an AH SA also protects selected parts of the IP header ahead of it, selected extension header parts, and selected IP options.**

> "In the case of AH, the protection is also extended to selected portions of the IP header preceding it, selected portions of extension headers, and selected options (contained in the IPv4 header, IPv6 Hop-by-Hop extension header, or IPv6 Destination extension headers)." — §4.1, `rfc4301.txt:800-804`

- Strength: description. Class: end-to-end.
- Check idea: Establish a transport mode AH SA and confirm its coverage extends to the selected immutable parts of the header ahead of it, in addition to the next layer protocol.

### RFC4301-SA-40

**A tunnel mode SA is applied to an IP tunnel, with access control applied to the headers of the traffic inside the tunnel.**

> "A tunnel mode SA is essentially an SA applied to an IP tunnel, with the access controls applied to the headers of the traffic inside the tunnel." — §4.1, `rfc4301.txt:807-809`

- Strength: description. Class: internal.
- Check idea: Establish a tunnel mode SA and confirm the implementation applies its access-control selectors to the headers of the inner, tunneled traffic.

### RFC4301-SA-41

**Two hosts may establish a tunnel mode SA between themselves.**

> "Two hosts MAY establish a tunnel mode SA between themselves." — §4.1, `rfc4301.txt:809`

- Strength: may. Class: internal.
- Check idea: Configure a tunnel mode SA directly between two hosts and confirm the implementation accepts it.

### RFC4301-SA-42

**Except for two exceptions, an SA must be tunnel mode whenever either end is a security gateway.**

> "Aside from the two exceptions below, whenever either end of a security association is a security gateway, the SA MUST be tunnel mode." — §4.1, `rfc4301.txt:810-812`

- Strength: must. Class: internal.
- Check idea: Configure an SA with a security gateway at one end, outside the two exception cases, and confirm the implementation creates it as tunnel mode.

### RFC4301-SA-43

**When traffic is destined for a security gateway itself, such as SNMP management commands, the gateway acts as a host and may use transport mode.**

> "Where traffic is destined for a security gateway, e.g., Simple Network Management Protocol (SNMP) commands, the security gateway is acting as a host and transport mode is allowed." — §4.1, `rfc4301.txt:816-818`

- Strength: may. Class: internal.
- Check idea: Send SNMP management traffic addressed to a security gateway's own address, and confirm the implementation allows a transport mode SA to protect it.

### RFC4301-SA-44

**A packet that becomes fragmented en route has all its fragments delivered to the same IPsec instance, for reassembly before cryptographic processing.**

> "a packet that might be fragmented en route must have all the fragments delivered to the same IPsec instance for reassembly prior to cryptographic processing." — §4.1, `rfc4301.txt:831-834`

- Strength: must (lower case). Class: internal.
- Check idea: Force a packet to fragment en route toward a destination reachable through several IPsec instances, and confirm all fragments arrive at the same instance before cryptographic processing.

### RFC4301-SA-45

**AH and ESP do not apply transport mode to an IPv4 packet that is a fragment; only tunnel mode applies to it.**

> "AH and ESP cannot be applied using transport mode to IPv4 packets that are fragments. Only tunnel mode can be employed in such cases." — §4.1, `rfc4301.txt:852-854`

- Strength: must not. Class: internal.
- Check idea: Try to configure a transport mode AH or ESP SA to process an IPv4 fragment, and confirm the implementation refuses it, or applies tunnel mode instead.

### RFC4301-SA-46

**For IPv6 too, an implementation does not use transport mode AH or ESP on a packet that is a fragment.**

> "for simplicity, this restriction also applies to IPv6 packets." — §4.1, `rfc4301.txt:855-856`

- Strength: must not. Class: internal.
- Check idea: Try to configure a transport mode AH or ESP SA to process an IPv6 fragment, and confirm the implementation applies the same restriction as for IPv4.

### RFC4301-SA-47

**In tunnel mode, an outer IP header carries the IPsec addresses, an inner IP header carries the packet's ultimate addresses, and the security protocol header sits between them.**

> "For a tunnel mode SA, there is an "outer" IP header that specifies the IPsec processing source and destination, plus an "inner" IP header that specifies the (apparently) ultimate source and destination for the packet. The security protocol header appears after the outer IP header, and before the inner IP header." — §4.1, `rfc4301.txt:860-864`

- Strength: description. Class: wire.
- Check idea: Capture a tunnel mode packet and confirm it carries an outer IP header, then the security protocol header, then an inner IP header that names the ultimate source and destination.

### RFC4301-SA-48

**In tunnel mode, AH protects selected parts of the outer IP header, plus the whole tunneled packet: inner header and next layer protocols.**

> "If AH is employed in tunnel mode, portions of the outer IP header are afforded protection (as above), as well as all of the tunneled IP packet (i.e., all of the inner IP header is protected, as well as next layer protocols)." — §4.1, `rfc4301.txt:864-868`

- Strength: description. Class: end-to-end.
- Check idea: Establish a tunnel mode AH SA and confirm its coverage includes the selected outer header parts and the whole inner packet, inner header and next layer protocols included.

### RFC4301-SA-49

**In tunnel mode, ESP protects only the tunneled inner packet, not the outer IP header.**

> "If ESP is employed, the protection is afforded only to the tunneled packet, not to the outer header." — §4.1, `rfc4301.txt:868-869`

- Strength: description. Class: end-to-end.
- Check idea: Establish a tunnel mode ESP SA and confirm its protection covers only the inner packet, and that the outer IP header is left unprotected.

### RFC4301-SA-50

**A host implementation of IPsec must support both transport mode and tunnel mode, whether native, BITS, or BITW.**

> "A host implementation of IPsec MUST support both transport and tunnel mode. This is true for native, BITS, and BITW implementations for hosts." — §4.1, `rfc4301.txt:873-875`

- Strength: must. Class: internal.
- Check idea: On a native, BITS, or BITW host implementation, configure a transport mode SA and a tunnel mode SA in turn, and confirm both are accepted.

### RFC4301-SA-51

**A security gateway must support tunnel mode, and may support transport mode.**

> "A security gateway MUST support tunnel mode and MAY support transport mode." — §4.1, `rfc4301.txt:877-878`

- Strength: must (tunnel mode), may (transport mode). Class: internal.
- Check idea: On a security gateway, configure a tunnel mode SA and confirm it is accepted; then attempt a transport mode SA and observe whether the implementation supports it.

## FUNC: SA functionality

### RFC4301-FUNC-1

**The set of security services an SA offers depends on the protocol chosen, the SA mode, its endpoints, and the optional services selected.**

> "The set of security services offered by an SA depends on the security protocol selected, the SA mode, the endpoints of the SA, and the election of optional services within the protocol." — §4.2, `rfc4301.txt:885-887`

- Strength: description. Class: internal.
- Check idea: Configure SAs that vary the protocol, the mode, the endpoints, and the optional services, and confirm each combination changes the resulting set of security services.

### RFC4301-FUNC-2

**AH and ESP both offer integrity and authentication, but their coverage differs between the protocols and between transport and tunnel mode.**

> "both AH and ESP offer integrity and authentication services, but the coverage differs for each protocol and differs for transport vs. tunnel mode." — §4.2, `rfc4301.txt:889-891`

- Strength: description. Class: end-to-end.
- Check idea: Compare the coverage of an AH SA and an ESP SA, in both transport mode and tunnel mode, and confirm the protected scope differs across the four cases.

### RFC4301-FUNC-3

**AH can protect the integrity of an IPv4 option or an IPv6 extension header en route, except for a field that may change unpredictably.**

> "AH can provide this service, except for IP or extension headers that may change in a fashion not predictable by the sender." — §4.2, `rfc4301.txt:893-894`

- Strength: may (lower case). Class: end-to-end.
- Check idea: Establish an AH SA that covers an IPv4 option or an IPv6 extension header, and confirm AH protects it, except for a field the sender cannot predict.

### RFC4301-FUNC-4

**In some contexts, an ESP tunnel mode SA can give the same security as AH.**

> "the same security may be achieved in some contexts by applying ESP to a tunnel carrying a packet." — §4.2, `rfc4301.txt:903-904`

- Strength: may (lower case). Class: end-to-end.
- Check idea: In a context that needs header-field integrity, establish an ESP tunnel mode SA instead of AH, and confirm it protects the whole original packet.

### RFC4301-FUNC-5

**The access control granularity an SA gives depends on the selectors chosen to define that SA.**

> "The granularity of access control provided is determined by the choice of the selectors that define each SA." — §4.2, `rfc4301.txt:906-907`

- Strength: description. Class: internal.
- Check idea: Define two SAs with selectors of different specificity, such as one host pair versus an address range, and confirm the access-control granularity differs to match.

### RFC4301-FUNC-6

**When confidentiality is selected, a tunnel mode ESP SA between two security gateways can give partial traffic flow confidentiality.**

> "If confidentiality is selected, then an ESP (tunnel mode) SA between two security gateways can offer partial traffic flow confidentiality." — §4.2, `rfc4301.txt:912-913`

- Strength: description. Class: end-to-end.
- Check idea: Establish a tunnel mode ESP SA with confidentiality between two security gateways, and confirm an observer on the outer path cannot see some traffic characteristics of the inner traffic.

### RFC4301-FUNC-7

**Tunnel mode lets an implementation encrypt the inner IP header, to hide the identities of the ultimate source and destination.**

> "The use of tunnel mode allows the inner IP headers to be encrypted, concealing the identities of the (ultimate) traffic source and destination." — §4.2, `rfc4301.txt:914-916`

- Strength: description. Class: wire.
- Check idea: Capture a tunnel mode ESP packet with confidentiality on, and confirm the inner IP header, including the ultimate source and destination addresses, is encrypted.

### RFC4301-FUNC-8

**ESP payload padding can hide the size of a packet, to conceal more of the traffic's outer characteristics.**

> "ESP payload padding also can be invoked to hide the size of the packets, further concealing the external characteristics of the traffic." — §4.2, `rfc4301.txt:916-918`

- Strength: description. Class: wire.
- Check idea: Establish an ESP SA with confidentiality and padding on, and confirm the sender can add padding so the packet's on-wire length no longer reveals the payload size.

### RFC4301-FUNC-9

**A compliant implementation must not let an ESP SA use both NULL encryption and no integrity algorithm.**

> "A compliant implementation MUST NOT allow instantiation of an ESP SA that employs both NULL encryption and no integrity algorithm." — §4.2, `rfc4301.txt:926-927`

- Strength: must not. Class: internal.
- Check idea: Try to negotiate or configure an ESP SA with NULL encryption and no integrity algorithm, and confirm the implementation refuses to create it.

### RFC4301-FUNC-10

**An attempt to negotiate an ESP SA with NULL encryption and no integrity algorithm is an auditable event, at both the initiator and the responder.**

> "An attempt to negotiate such an SA is an auditable event by both initiator and responder." — §4.2, `rfc4301.txt:928-929`

- Strength: description. Class: internal.
- Check idea: Attempt this negotiation at both the initiator and the responder, and confirm each one records an audit log entry for the attempt.

### RFC4301-FUNC-11

**The audit log entry for this event should include the date and time, the local IKE IP address, and the remote IKE IP address.**

> "The audit log entry for this event SHOULD include the current date/time, local IKE IP address, and remote IKE IP address." — §4.2, `rfc4301.txt:929-931`

- Strength: should. Class: internal.
- Check idea: Trigger this audit event and inspect the log entry for the date and time, and for the local and remote IKE IP addresses.

### RFC4301-FUNC-12

**The initiator should record the relevant SPD entry in this audit log entry.**

> "The initiator SHOULD record the relevant SPD entry." — §4.2, `rfc4301.txt:931`

- Strength: should. Class: internal.
- Check idea: Trigger this audit event at the initiator and inspect its log entry for a record of the SPD entry that led to the rejected negotiation.

## COMB: Combining SAs

### RFC4301-COMB-1

**RFC 4301 does not require an implementation to support nested security associations or SA bundles.**

> "This document does not require support for nested security associations or for what RFC 2401 [RFC2401] called "SA bundles"." — §4.3, `rfc4301.txt:935-936`

- Strength: description. Class: internal.
- Check idea: Configure an implementation with no nested-SA or SA-bundle support, and confirm this alone does not make it non-compliant.

### RFC4301-COMB-2

**An implementation that supports nested SAs should give a management interface to express the nesting need, and then build the matching SPD and forwarding table entries.**

> "An implementation that provides support for nested SAs SHOULD provide a management interface that enables a user or administrator to express the nesting requirement, and then create the appropriate SPD entries and forwarding table entries to effect the requisite processing." — §4.3, `rfc4301.txt:942-946`

- Strength: should. Class: internal.
- Check idea: On an implementation that supports nested SAs, use its management interface to express a nesting need, and confirm it creates the matching SPD entries and forwarding table entries.

## DB: The IPsec databases

### RFC4301-DB-1

**An implementation may structure its databases differently from this model, but its outside behavior must match this model's observable traits.**

> "implementations need not match details of this model as presented, but the external behavior of implementations MUST correspond to the externally observable characteristics of this model in order to be compliant." — §4.4, `rfc4301.txt:969-972`

- Strength: description (internal structure), must (external behavior). Class: end-to-end.
- Check idea: Build an implementation whose internal database structure differs from the nominal model, then observe its outside traffic handling, and confirm it matches what the nominal model would produce.

### RFC4301-DB-2

**The Security Policy Database sets the disposition of all IP traffic, inbound or outbound, for a host or security gateway.**

> "The first specifies the policies that determine the disposition of all IP traffic inbound or outbound from a host or security gateway (Section 4.4.1)." — §4.4, `rfc4301.txt:976-978`

- Strength: description. Class: internal.
- Check idea: Inspect the Security Policy Database and confirm its entries set the disposition, protect, bypass, or discard, of both inbound and outbound traffic.

### RFC4301-DB-3

**The Security Association Database holds the parameters for each established, keyed SA.**

> "The second database contains parameters that are associated with each established (keyed) SA (Section 4.4.2)." — §4.4, `rfc4301.txt:978-980`

- Strength: description. Class: internal.
- Check idea: Establish a keyed SA and confirm the Security Association Database holds an entry with that SA's parameters.

### RFC4301-DB-4

**The Peer Authorization Database links an SA management protocol, such as IKE, to the Security Policy Database.**

> "The third database, the PAD, provides a link between an SA management protocol (such as IKE) and the SPD (Section 4.4.3)." — §4.4, `rfc4301.txt:980-982`

- Strength: description. Class: internal.
- Check idea: Trace an SA created through IKE negotiation back to the SPD entry and the peer authorization that permitted it.

### RFC4301-DB-5

**A security gateway that serves several subscribers may run several separate IPsec contexts.**

> "If an IPsec implementation acts as a security gateway for multiple subscribers, it MAY implement multiple separate IPsec contexts." — §4.4, `rfc4301.txt:986-987`

- Strength: may. Class: internal.
- Check idea: Configure a security gateway for several subscribers, with a separate IPsec context per subscriber, and confirm the implementation accepts it.

### RFC4301-DB-6

**Each IPsec context may have, and may use, fully independent identities, policies, and SAs.**

> "Each context MAY have and MAY use completely independent identities, policies, key management SAs, and/or IPsec SAs." — §4.4, `rfc4301.txt:988-989`

- Strength: may. Class: internal.
- Check idea: Configure two IPsec contexts on one gateway with different identities, policies, and SAs, and confirm each context works on its own.

### RFC4301-DB-7

**A gateway with several IPsec contexts has a means to match an inbound SA proposal to the right local context.**

> "a means for associating inbound (SA) proposals with local contexts is required." — §4.4, `rfc4301.txt:990-992`

- Strength: description. Class: internal.
- Check idea: Send an inbound SA proposal addressed to one of several IPsec contexts on the gateway, and confirm the implementation matches it to the correct context.

### RFC4301-DB-8

**IPsec assumes that outbound and inbound traffic which has passed through IPsec processing is forwarded to fit the context where IPsec runs.**

> "IPsec assumes only that outbound and inbound traffic that has passed through IPsec processing is forwarded in a fashion consistent with the context in which IPsec is implemented." — §4.4, `rfc4301.txt:1015-1018`

- Strength: description. Class: internal.
- Check idea: After IPsec forwards or delivers a packet, confirm the surrounding forwarding function routes it the same way it would route any other packet in that deployment.

### RFC4301-DB-9

**A non-initial fragment lacks one or more selector values access control needs, such as the next layer protocol or the ports.**

> "the phrase "non-initial fragments" is used to mean fragments that do not contain all of the selector values that may be needed for access control (e.g., they might not contain Next Layer Protocol, source and destination ports, ICMP message type/code, Mobility Header type)." — §4.4, `rfc4301.txt:1035-1039`

- Strength: may (lower case). Class: wire.
- Check idea: Capture the non-initial fragments of a fragmented packet, and confirm they lack a selector field, such as the next layer protocol or the ports, that access control needs.

### RFC4301-DB-10

**An initial fragment carries every selector value access control needs.**

> "the phrase "initial fragment" is used to mean a fragment that contains all the selector values needed for access control." — §4.4, `rfc4301.txt:1039-1041`

- Strength: description. Class: wire.
- Check idea: Capture the initial fragment of a fragmented packet, and confirm it carries every selector field access control needs, such as the next layer protocol and the ports.

### RFC4301-DB-11

**For IPv6, the fragment that carries the next layer protocol and ports depends on the extension headers present, so the initial fragment may not be the first fragment.**

> "for IPv6, which fragment contains the Next Layer Protocol and ports (or ICMP message type/code or Mobility Header type [Mobip]) will depend on the kind and number of extension headers present. The "initial fragment" might not be the first fragment, in this context." — §4.4, `rfc4301.txt:1041-1046`

- Strength: description. Class: wire.
- Check idea: Fragment an IPv6 packet with several extension headers so the selector data lands in a later fragment, and confirm the implementation treats that later fragment as the initial fragment.

## The Security Policy Database

### RFC4301-SPD-1

**An IPsec implementation consults the SPD, or its caches, for all traffic that crosses the IPsec boundary, including traffic not protected by IPsec and IKE traffic.**

> "The SPD, or relevant caches, must be consulted during the processing of all traffic
> (inbound and outbound), including traffic not protected by IPsec, that traverses the IPsec
> boundary. This includes IPsec management traffic such as IKE." — §4.4.1, `rfc4301.txt:1059-1062`

- Strength: must (lower case). Class: internal.
- Check idea: send both IPsec-protected traffic and traffic not protected by IPsec, including
  IKE traffic, across the IPsec boundary and check that the SPD or its cache is consulted for
  each packet.

### RFC4301-SPD-2

**An IPsec implementation must have at least one SPD.**

> "An IPsec implementation MUST have at least one SPD" — §4.4.1, `rfc4301.txt:1062`,
> `rfc4301.txt:1071`

- Strength: must. Class: internal.
- Check idea: inspect an IPsec implementation and check that it maintains at least one
  Security Policy Database.

### RFC4301-SPD-3

**An IPsec implementation may support multiple SPDs, if appropriate for the context in which it operates.**

> "it MAY support multiple SPDs, if appropriate for the context in which the IPsec
> implementation operates." — §4.4.1, `rfc4301.txt:1071-1073`

- Strength: may. Class: internal.
- Check idea: inspect an IPsec implementation configured with more than one SPD and check that
  this is an allowed configuration.

### RFC4301-SPD-4

**An IPsec implementation that supports multiple SPDs must include an explicit SPD selection function to select the SPD for outbound traffic processing.**

> "if an implementation supports multiple SPDs, then it MUST include an explicit SPD selection
> function that is invoked to select the appropriate SPD for outbound traffic processing." —
> §4.4.1, `rfc4301.txt:1075-1077`

- Strength: must. Class: internal.
- Check idea: configure an IPsec implementation with multiple SPDs and check that it applies an
  explicit selection function that picks one SPD for outbound traffic.

### RFC4301-SPD-5

**The SPD selection function takes the outbound packet and local metadata as input and returns an SPD identifier (SPD-ID).**

> "The inputs to this function are the outbound packet and any local metadata (e.g., the
> interface via which the packet arrived) required to effect the SPD selection function." and
> "The output of the function is an SPD identifier (SPD-ID)." — §4.4.1, `rfc4301.txt:1077-1080`,
> `rfc4301.txt:1080-1081`

- Strength: description. Class: internal.
- Check idea: invoke the SPD selection function with an outbound packet and its local metadata,
  such as the arrival interface, and check that it returns an SPD identifier.

### RFC4301-SPD-6

**The SPD must let a user or administrator order the entries to express the desired access control policy.**

> "a user or administrator MUST be able to order the entries to express a desired access
> control policy." — §4.4.1, `rfc4301.txt:1087-1088`

- Strength: must. Class: internal.
- Check idea: have a user or administrator place SPD entries in a chosen order and check that
  the SPD keeps that order.

### RFC4301-SPD-7

**The SPD distinguishes traffic that receives IPsec protection from traffic allowed to bypass IPsec, both at the sender and at the receiver.**

> "An SPD must discriminate among traffic that is afforded IPsec protection and traffic that is
> allowed to bypass IPsec." and "This applies to the IPsec protection to be applied by a sender
> and to the IPsec protection that must be present at the receiver." — §4.4.1,
> `rfc4301.txt:1095-1097`, `rfc4301.txt:1097-1099`

- Strength: must (lower case). Class: internal.
- Check idea: present the SPD with traffic destined for IPsec protection and traffic destined to
  bypass IPsec, at a sender and at a receiver, and check that the SPD produces the correct
  choice for each.

### RFC4301-SPD-8

**For every outbound or inbound datagram, the SPD chooses one of three processing choices: DISCARD, BYPASS IPsec, or PROTECT using IPsec.**

> "any outbound or inbound datagram, three processing choices are possible: DISCARD, BYPASS
> IPsec, or PROTECT using IPsec." — §4.4.1, `rfc4301.txt:1099-1100`

- Strength: description. Class: internal.
- Check idea: submit an outbound datagram and an inbound datagram to the SPD and check that the
  decision returned is one of DISCARD, BYPASS, or PROTECT.

### RFC4301-SPD-9

**DISCARD means traffic that is not allowed to cross the IPsec boundary in the given direction.**

> "The first choice refers to traffic that is not allowed to traverse the IPsec boundary (in
> the specified direction)." — §4.4.1, `rfc4301.txt:1100-1102`

- Strength: description. Class: end-to-end.
- Check idea: send traffic that matches a DISCARD entry toward the IPsec boundary and check that
  the traffic does not cross it.

### RFC4301-SPD-10

**BYPASS means traffic that is allowed to cross the IPsec boundary without IPsec protection.**

> "The second choice refers to traffic that is allowed to cross the IPsec boundary without
> IPsec protection." — §4.4.1, `rfc4301.txt:1102-1104`

- Strength: description. Class: wire.
- Check idea: send traffic that matches a BYPASS entry across the IPsec boundary and check that
  it crosses unmodified, without IPsec protection.

### RFC4301-SPD-11

**PROTECT means traffic that receives IPsec protection, and for it the SPD states the security protocol, mode, security service options, and cryptographic algorithms to use.**

> "The third choice refers to traffic that is afforded IPsec protection, and for such traffic
> the SPD must specify the security protocols to be employed, their mode, security service
> options, and the cryptographic algorithms to be used." — §4.4.1, `rfc4301.txt:1104-1108`

- Strength: must (lower case). Class: internal.
- Check idea: inspect an SPD entry with the PROTECT action and check that it states the security
  protocol, mode, security service options, and cryptographic algorithms for the matching
  traffic.

### RFC4301-SPD-12

**The SPD is divided into three parts: SPD-S for traffic subject to IPsec protection, SPD-O for outbound traffic to bypass or discard, and SPD-I for inbound traffic to bypass or discard.**

> "The SPD-S (secure traffic) contains entries for all traffic subject to IPsec protection."
> and "SPD-O (outbound) contains entries for all outbound traffic that is to be bypassed or
> discarded." and "SPD-I (inbound) is applied to inbound traffic that will be bypassed or
> discarded." — §4.4.1, `rfc4301.txt:1112-1114`, `rfc4301.txt:1114-1115`, `rfc4301.txt:1115-1116`

- Strength: description. Class: internal.
- Check idea: inspect the SPD of an IPsec implementation and check that its entries fall into
  the SPD-S, SPD-O, and SPD-I parts as defined.

### RFC4301-SPD-13

**When a packet looked up in SPD-I matches no entry, the IPsec implementation must discard the packet.**

> "if a packet that is looked up in the SPD-I cannot be matched to an entry there, then the
> packet MUST be discarded." — §4.4.1, `rfc4301.txt:1132-1134`

- Strength: must. Class: end-to-end.
- Check idea: send inbound traffic that is checked against SPD-I and matches no SPD-I entry, and
  check that the implementation discards it.

### RFC4301-SPD-14

**For outbound traffic, when no match is found in SPD-S the implementation checks SPD-O for a bypass match, and when no match is found in SPD-O first, it checks SPD-S.**

> "for outbound traffic, if a match is not found in SPD-S, then SPD-O must be checked to see if
> the traffic should be bypassed." and "if SPD-O is checked first and no match is found, then
> SPD-S must be checked." — §4.4.1, `rfc4301.txt:1135-1137`, `rfc4301.txt:1137-1138`

- Strength: must, should (lower case). Class: internal.
- Check idea: for outbound traffic, look up SPD-S and SPD-O in each order and check that the
  implementation performs the second lookup whenever the first lookup finds no match.

### RFC4301-SPD-15

**In an ordered, non-decorrelated SPD, the SPD-S, SPD-I, and SPD-O entries are interleaved into one list, searched in a single lookup.**

> "In an ordered, non-decorrelated SPD, the entries for the SPD-S, SPD-I, and SPD-O are
> interleaved." and "So there is one lookup in the SPD." — §4.4.1, `rfc4301.txt:1138-1139`,
> `rfc4301.txt:1139-1140`

- Strength: description. Class: internal.
- Check idea: inspect a non-decorrelated, ordered SPD and check that its SPD-S, SPD-I, and SPD-O
  entries sit in one interleaved, ordered list searched by a single lookup.

### RFC4301-SPD-16

**Each SPD entry states a packet disposition of BYPASS, DISCARD, or PROTECT, and is keyed by a list of one or more selectors.**

> "Each SPD entry specifies packet disposition as BYPASS, DISCARD, or PROTECT." and "The entry
> is keyed by a list of one or more selectors." — §4.4.1, `rfc4301.txt:1144-1145`

- Strength: description. Class: internal.
- Check idea: inspect an SPD entry and check that it names one of BYPASS, DISCARD, or PROTECT
  and lists the selectors that key it.

### RFC4301-SPD-17

**Every SPD should have a final, nominal entry that matches any otherwise-unmatched traffic and discards it.**

> "Every SPD SHOULD have a nominal, final entry that matches anything that is otherwise
> unmatched, and discards it." — §4.4.1, `rfc4301.txt:1151-1152`

- Strength: should. Class: internal.
- Check idea: send a packet that matches no explicit SPD entry and check that the SPD's final
  entry discards it.

### RFC4301-SPD-18

**The SPD must let a user or administrator specify SPD-I entries, keyed by selector values, for inbound traffic to bypass or discard.**

> "The SPD MUST permit a user or administrator to specify policy entries as follows:" and
> "SPD-I: For inbound traffic that is to be bypassed or discarded, the entry consists of the
> values of the selectors that apply to the traffic to be bypassed or discarded." — §4.4.1,
> `rfc4301.txt:1154-1155`, `rfc4301.txt:1157-1159`

- Strength: must. Class: internal.
- Check idea: ask a user or administrator to create an SPD-I entry for inbound traffic to be
  bypassed or discarded and check that the SPD accepts an entry built from selector values.

### RFC4301-SPD-19

**The SPD must let a user or administrator specify SPD-O entries, keyed by selector values, for outbound traffic to bypass or discard.**

> "The SPD MUST permit a user or administrator to specify policy entries as follows:" and
> "SPD-O: For outbound traffic that is to be bypassed or discarded, the entry consists of the
> values of the selectors that apply to the traffic to be bypassed or discarded." — §4.4.1,
> `rfc4301.txt:1154-1155`, `rfc4301.txt:1161-1163`

- Strength: must. Class: internal.
- Check idea: ask a user or administrator to create an SPD-O entry for outbound traffic to be
  bypassed or discarded and check that the SPD accepts an entry built from selector values.

### RFC4301-SPD-20

**The SPD must let a user or administrator specify SPD-S entries, with the selector values, the SA-creation controls, and the parameters needed to protect the matching traffic with AH or ESP.**

> "The SPD MUST permit a user or administrator to specify policy entries as follows:" and
> "SPD-S: For traffic that is to be protected using IPsec, the entry consists of the values of
> the selectors that apply to the traffic to be protected via AH or ESP, controls on how to
> create SAs based on these selectors, and the parameters needed to effect this protection
> (e.g., algorithms, modes, etc.)." — §4.4.1, `rfc4301.txt:1154-1155`, `rfc4301.txt:1165-1169`

- Strength: must. Class: internal.
- Check idea: ask a user or administrator to create an SPD-S entry for traffic to be protected
  and check that the SPD accepts the selector values, the SA-creation controls, and the
  protection parameters.

### RFC4301-SPD-21

**An SPD-S entry also contains a populate-from-packet (PFP) flag and bits stating whether the SA lookup uses the local and remote IP addresses in addition to the SPI.**

> "an SPD-S entry also contains information such as "populate from packet" (PFP) flag (see
> paragraphs below on "How To Derive the Values for an SAD entry") and bits indicating whether
> the SA lookup makes use of the local and remote IP addresses in addition to the SPI (see AH
> [Ken05b] or ESP [Ken05a] specifications)." — §4.4.1, `rfc4301.txt:1170-1172`,
> `rfc4301.txt:1183-1185`

- Strength: description. Class: internal.
- Check idea: inspect an SPD-S entry and check that it carries a PFP flag and bits stating
  whether the SA lookup uses the local and remote IP addresses together with the SPI.

### RFC4301-SPD-22

**For traffic protected by IPsec, an SPD entry swaps the Local and Remote address and ports to represent directionality, matching IKE conventions.**

> "For traffic protected by IPsec, the Local and Remote address and ports in an SPD entry are
> swapped to represent directionality, consistent with IKE conventions." — §4.4.1,
> `rfc4301.txt:1189-1191`

- Strength: description. Class: internal.
- Check idea: compare the outbound and inbound SPD-S entries for the same flow and check that
  the Local and Remote address and port values are swapped between them.

### RFC4301-SPD-23

**SPD entries for ICMP are specified in the same way as SPD entries for other protocols, even though ICMP often has no bidirectional authorization requirement.**

> "for ICMP, there is often no such bi-directional authorization requirement." and "for the sake
> of uniformity and simplicity, SPD entries for ICMP are specified in the same way as for other
> protocols." — §4.4.1, `rfc4301.txt:1193-1194`, `rfc4301.txt:1194-1197`

- Strength: description. Class: internal.
- Check idea: inspect an SPD entry for ICMP traffic and check that it follows the same
  Local/Remote and selector structure used for other protocols.

### RFC4301-SPD-24

**ICMP, Mobility Header, and non-initial-fragment packets have no port fields, so SPD entries express access control for them through the ICMP message type and code or the Mobility Header type instead of ports.**

> "for ICMP, Mobility Header, and non-initial fragments, there are no port fields in these
> packets." and "SPD entries have provisions for expressing access controls appropriate for
> these protocols, in lieu of the normal port field controls." — §4.4.1, `rfc4301.txt:1197-1198`,
> `rfc4301.txt:1200-1202`

- Strength: description. Class: internal.
- Check idea: inspect an SPD entry that matches ICMP or Mobility Header traffic and check that it
  expresses access control through message type/code or Mobility Header type instead of port
  values.

### RFC4301-SPD-25

**For bypassed or discarded traffic, the SPD supports separate inbound and outbound entries, so a unidirectional flow can be permitted.**

> "For bypassed or discarded traffic, separate inbound and outbound entries are supported, e.g.,
> to permit unidirectional flows if required." — §4.4.1, `rfc4301.txt:1202-1204`

- Strength: description. Class: internal.
- Check idea: configure separate SPD-I and SPD-O entries that permit traffic in only one
  direction and check that the SPD allows the unidirectional flow while blocking the reverse
  direction.

### RFC4301-SPD-26

**The special selector value ANY matches any value in the corresponding packet field, or a field that is absent or obscured.**

> "ANY is a wildcard that matches any value in the corresponding field of the packet, or that
> matches packets where that field is not present or is obscured." — §4.4.1,
> `rfc4301.txt:1210-1212`

- Strength: description. Class: internal.
- Check idea: create an SPD entry with a selector set to ANY and check that it matches packets
  with any field value and packets where the field is absent or obscured.

### RFC4301-SPD-27

**The special selector value OPAQUE means the corresponding field is not available for examination, because it may be absent from a fragment, absent for the Next Layer Protocol, or already encrypted by IPsec.**

> "OPAQUE indicates that the corresponding selector field is not available for examination
> because it may not be present in a fragment, it does not exist for the given Next Layer
> Protocol, or prior application of IPsec may have encrypted the value." — §4.4.1,
> `rfc4301.txt:1212-1216`

- Strength: may (lower case). Class: internal.
- Check idea: create an SPD entry with a selector set to OPAQUE and check that it matches
  packets whose corresponding field cannot be examined.

### RFC4301-SPD-28

**The ANY selector value encompasses the OPAQUE value, so OPAQUE is needed only to distinguish an allowed-any-value field from an absent or unavailable field.**

> "The ANY value encompasses the OPAQUE value." and "OPAQUE need be used only when it is
> necessary to distinguish between the case of any allowed value for a field, vs. the absence or
> unavailability (e.g., due to encryption) of the field." — §4.4.1, `rfc4301.txt:1216-1217`,
> `rfc4301.txt:1217-1220`

- Strength: description. Class: internal.
- Check idea: compare an SPD entry that uses ANY against one that uses OPAQUE for the same
  selector and check that ANY also matches the packets that OPAQUE matches.

### RFC4301-SPD-29

**Each selector of an SPD entry states how to derive the matching value for a new SAD entry from the SPD entry and the packet.**

> "For each selector in an SPD entry, the entry specifies how to derive the corresponding values
> for a new SA Database (SAD, see Section 4.4.2) entry from those in the SPD and the packet." —
> §4.4.1, `rfc4301.txt:1224-1226`

- Strength: description. Class: internal.
- Check idea: inspect an SPD entry's selectors and check that each one states whether the new
  SAD entry takes its value from the packet or from the SPD entry.

### RFC4301-SPD-30

**When IPsec processing is specified for an entry, a populate-from-packet (PFP) flag may be asserted for one or more of the Local address, Remote address, Next Layer Protocol, and port, ICMP type/code, or Mobility Header type selectors.**

> "If IPsec processing is specified for an entry, a "populate from packet" (PFP) flag may be
> asserted for one or more of the selectors in the SPD entry (Local IP address; Remote IP
> address; Next Layer Protocol; and, depending on Next Layer Protocol, Local port and Remote
> port, or ICMP type/code, or Mobility Header type)." — §4.4.1, `rfc4301.txt:1241-1246`

- Strength: may (lower case). Class: internal.
- Check idea: configure an SPD entry with IPsec processing and set the PFP flag on one of its
  selectors and check that the SPD entry accepts the flag for that selector.

### RFC4301-SPD-31

**When the PFP flag is set for a selector, the new SA takes its value for that selector from the packet; otherwise, the SA takes its value from the SPD entry.**

> "If asserted for a given selector X, the flag indicates that the SA to be created should take
> its value for X from the value in the packet." and "Otherwise, the SA should take its
> value(s) for X from the value(s) in the SPD entry." — §4.4.1, `rfc4301.txt:1246-1248`,
> `rfc4301.txt:1248-1249`

- Strength: should (lower case). Class: internal.
- Check idea: create SAs from SPD entries with the PFP flag set and with it cleared for a
  selector, and check that the SA's value comes from the packet when the flag is set and from
  the SPD entry when it is cleared.

### RFC4301-SPD-32

**Every IPsec implementation must have a management interface that lets a user or system administrator manage the SPD.**

> "For every IPsec implementation, there MUST be a management interface that allows a user or
> system administrator to manage the SPD." — §4.4.1, `rfc4301.txt:1282-1284`

- Strength: must. Class: internal.
- Check idea: inspect an IPsec implementation and check that it offers a management interface
  through which a user or administrator can manage the SPD.

### RFC4301-SPD-33

**The management interface lets the user or administrator specify the security processing applied to every packet that crosses the IPsec boundary.**

> "The interface must allow the user (or administrator) to specify the security processing to
> be applied to every packet that traverses the IPsec boundary." — §4.4.1,
> `rfc4301.txt:1284-1286`

- Strength: must (lower case). Class: internal.
- Check idea: through the management interface, specify the security processing for a class of
  traffic and check that packets crossing the IPsec boundary receive that processing.

### RFC4301-SPD-34

**The management interface for the SPD must allow creation of entries consistent with the defined selectors.**

> "The management interface for the SPD MUST allow creation of entries consistent with the
> selectors defined in Section 4.4.1.1" — §4.4.1, `rfc4301.txt:1297-1299`

- Strength: must. Class: internal.
- Check idea: through the management interface, create an SPD entry using the defined selector
  types and check that the SPD accepts it.

### RFC4301-SPD-35

**The management interface for the SPD must support total ordering of the entries as seen through that interface.**

> "MUST support (total) ordering of these entries, as seen via this interface." — §4.4.1,
> `rfc4301.txt:1299-1300`

- Strength: must. Class: internal.
- Check idea: through the management interface, set a total order on the SPD entries and check
  that the interface reports that same order back.

### RFC4301-SPD-36

**In host systems, applications may be allowed to create SPD entries.**

> "In host systems, applications MAY be allowed to create SPD entries." — §4.4.1,
> `rfc4301.txt:1305-1306`

- Strength: may. Class: internal.
- Check idea: in a host system that allows application-created policy, have an application
  request an SPD entry and check that the SPD accepts it.

### RFC4301-SPD-37

**The system administrator must be able to specify whether a user or application can override the default system policies.**

> "the system administrator MUST be able to specify whether or not a user or application can
> override (default) system policies." — §4.4.1, `rfc4301.txt:1308-1309`

- Strength: must. Class: internal.
- Check idea: as the system administrator, set whether a user or application may override the
  default system policy and check that the setting is honored when a user or application
  attempts an override.

### RFC4301-SPD-38

**All IPsec implementations must support the standard set of SPD elements that this document specifies.**

> "this document does specify a standard set of SPD elements that all IPsec implementations
> MUST support." — §4.4.1, `rfc4301.txt:1313-1314`

- Strength: must. Class: internal.
- Check idea: inspect an IPsec implementation's SPD and check that it supports every SPD element
  this document defines.

### RFC4301-SPD-39

**When an SPD entry is decorrelated, the IPsec implementation must link all the resulting entries together, so they can be placed into the caches and the SAD together.**

> "when an SPD entry is decorrelated all the resulting entries MUST be linked together, so that
> all members of the group derived from an individual, SPD entry (prior to decorrelation) can
> all be placed into caches and into the SAD at the same time." — §4.4.1,
> `rfc4301.txt:1334-1338`

- Strength: must. Class: internal.
- Check idea: decorrelate an SPD entry into several resulting entries and check that they are
  linked together and placed into the caches and the SAD as one group when one of them triggers
  an SA.

### RFC4301-SPD-40

**When a responder uses a decorrelated SPD, it should use the decorrelated entries to match the initiator's traffic selector proposals.**

> "If the responder employs a decorrelated SPD, it SHOULD use the decorrelated SPD entries for
> matching, as this will generally result in creation of SAs that are more likely to match the
> intent of both peers." — §4.4.1, `rfc4301.txt:1387-1390`

- Strength: should. Class: internal.
- Check idea: give a responder with a decorrelated SPD a set of traffic selector proposals and
  check that it matches them against the decorrelated entries.

### RFC4301-SPD-41

**When a responder has a correlated SPD, it should match the proposals against the correlated entries.**

> "If the responder has a correlated SPD, then it SHOULD match the proposals against the
> correlated entries." — §4.4.1, `rfc4301.txt:1390-1391`

- Strength: should. Class: internal.
- Check idea: give a responder with a correlated SPD a set of traffic selector proposals and
  check that it matches them against the correlated entries.

### RFC4301-SPD-42

**When the SPD is not decorrelated, caching is not allowed, and the implementation must perform an ordered search of the SPD to verify that inbound traffic arriving on an SA is consistent with the access control policy.**

> "If the SPD is not decorrelated, caching is not allowed and an ordered search of SPD MUST be
> performed to verify that inbound traffic arriving on an SA is consistent with the access
> control policy expressed in the SPD." — §4.4.1, `rfc4301.txt:1396-1398`, `rfc4301.txt:1407-1409`

- Strength: must. Class: internal.
- Check idea: with a non-decorrelated SPD, present inbound traffic arriving on an SA and check
  that the implementation performs an ordered search of the SPD, instead of using a cache, to
  verify consistency with the access control policy.

### RFC4301-SPD-43

**When a decorrelated SPD is available, its decorrelated entries are used to populate the SPD-S cache.**

> "when a decorrelated SPD is available, the decorrelated entries are used to populate the
> SPD-S cache." — §4.4.1, `rfc4301.txt:1395-1396`

- Strength: description. Class: internal.
- Check idea: provide an implementation with a decorrelated SPD and check that its SPD-S cache
  is populated from the decorrelated entries.

### RFC4301-SPD-44

**When the SPD changes while the system is running, an implementation should check the effect of the change on existing SAs.**

> "If a change is made to the SPD while the system is running, a check SHOULD be made of the
> effect of this change on extant SAs." and "An implementation SHOULD check the impact of an SPD
> change on extant SAs" — §4.4.1, `rfc4301.txt:1413-1414`, `rfc4301.txt:1415-1416`

- Strength: should. Class: internal.
- Check idea: change the SPD while the system is running, with existing SAs present, and check
  that the implementation evaluates the effect of the change on those SAs.

### RFC4301-SPD-45

**An implementation should give a user or administrator a mechanism to configure what action to take on an SA affected by an SPD change, such as deleting it or leaving it unchanged.**

> "SHOULD provide a user/administrator with a mechanism for configuring what actions to take,
> e.g., delete an affected SA, allow an affected SA to continue unchanged, etc." — §4.4.1,
> `rfc4301.txt:1416-1418`

- Strength: should. Class: internal.
- Check idea: as a user or administrator, configure the action to take on an SA affected by an
  SPD change and check that the implementation applies that configured action, such as deleting
  the SA or leaving it unchanged.

## Selectors

### RFC4301-SEL-1

**All IPsec implementations must support the selector parameters listed in this section, to control the granularity of an SA.**

> "The following selector parameters MUST be supported by all IPsec implementations to
> facilitate control of SA granularity." — §4.4.1.1, `rfc4301.txt:1431-1433`

- Strength: must. Class: internal.
- Check idea: inspect an IPsec implementation and check that it supports every selector
  parameter this section defines.

### RFC4301-SEL-2

**The Local and Remote addresses of a selector are both IPv4 or both IPv6, never a mix of address types.**

> "Note that both Local and Remote addresses should either be IPv4 or IPv6, but not a mix of
> address types." — §4.4.1.1, `rfc4301.txt:1433-1435`

- Strength: should (lower case). Class: internal.
- Check idea: inspect a selector's Local and Remote address values and check that both use the
  same IP version.

### RFC4301-SEL-3

**The Local and Remote port selectors, the ICMP message type and code, and the Mobility Header type may be labeled OPAQUE when a fragment makes these fields inaccessible.**

> "the Local/Remote port selectors (and ICMP message type and code, and Mobility Header type)
> may be labeled as OPAQUE to accommodate situations where these fields are inaccessible due to
> packet fragmentation." — §4.4.1.1, `rfc4301.txt:1435-1438`

- Strength: may (lower case). Class: internal.
- Check idea: present a fragmented packet whose port, ICMP type/code, or Mobility Header type
  field is inaccessible and check that the matching selector can be labeled OPAQUE.

### RFC4301-SEL-4

**The Remote IP Address selector is a list of ranges of IPv4 or IPv6 addresses, expressing a single address, a list of addresses, a range, or a list of ranges.**

> "Remote IP Address(es) (IPv4 or IPv6): This is a list of ranges of IP addresses (unicast,
> broadcast (IPv4 only))." and "This structure allows expression of a single IP address (via a
> trivial range), or a list of addresses (each a trivial range), or a range of addresses (low
> and high values, inclusive), as well as the most generic form of a list of ranges." — §4.4.1.1,
> `rfc4301.txt:1440-1441`, `rfc4301.txt:1441-1445`

- Strength: description. Class: internal.
- Check idea: configure the Remote IP Address selector as a single address, as a list of
  addresses, and as a range of addresses, and check that the SPD entry accepts each form.

### RFC4301-SEL-5

**The Local IP Address selector is a list of ranges of IPv4 or IPv6 addresses, expressing a single address, a list of addresses, a range, or a list of ranges, naming the address or addresses this implementation protects.**

> "Local IP Address(es) (IPv4 or IPv6): This is a list of ranges of IP addresses (unicast,
> broadcast (IPv4 only))." and "This structure allows expression of a single IP address (via a
> trivial range), or a list of addresses (each a trivial range), or a range of addresses (low
> and high values, inclusive), as well as the most generic form of a list of ranges." and "Local
> refers to the address(es) being protected by this implementation (or policy entry)." —
> §4.4.1.1, `rfc4301.txt:1449-1450`, `rfc4301.txt:1450-1454`, `rfc4301.txt:1464-1465`

- Strength: description. Class: internal.
- Check idea: configure the Local IP Address selector as a single address, as a list of
  addresses, and as a range of addresses, and check that the SPD entry accepts each form and
  treats it as the address this implementation protects.

### RFC4301-SEL-6

**The SPD does not support multicast address entries; multicast SAs use a separate Group SPD (GSPD) instead.**

> "The SPD does not include support for multicast address entries." and "To support multicast
> SAs, an implementation should make use of a Group SPD (GSPD) as defined in [RFC3740]." —
> §4.4.1.1, `rfc4301.txt:1467-1469`

- Strength: should (lower case). Class: internal.
- Check idea: attempt to configure an SPD entry with a multicast Local or Remote address and
  check that the SPD does not accept it as an ordinary unicast selector entry.

### RFC4301-SEL-7

**The Next Layer Protocol selector comes from the IPv4 Protocol field or the IPv6 Next Header field, and its value is an individual protocol number, ANY, or, for IPv6 only, OPAQUE.**

> "Next Layer Protocol: Obtained from the IPv4 "Protocol" or the IPv6 "Next Header" fields." and
> "This is an individual protocol number, ANY, or for IPv6 only, OPAQUE." — §4.4.1.1,
> `rfc4301.txt:1477-1479`

- Strength: description. Class: internal.
- Check idea: inspect the Next Layer Protocol selector of an SPD entry and check that its value
  is a single protocol number, ANY, or, on an IPv6 entry, OPAQUE.

### RFC4301-SEL-8

**The Next Layer Protocol is whatever protocol comes after any IP extension headers present in the packet.**

> "The Next Layer Protocol is whatever comes after any IP extension headers that are present." —
> §4.4.1.1, `rfc4301.txt:1479-1481`

- Strength: description. Class: internal.
- Check idea: send a packet with one or more IP extension headers and check that the
  implementation identifies the Next Layer Protocol as the protocol following the last
  extension header.

### RFC4301-SEL-9

**An IPsec implementation should provide a mechanism for configuring which IPv6 extension headers to skip when it locates the Next Layer Protocol.**

> "To simplify locating the Next Layer Protocol, there SHOULD be a mechanism for configuring
> which IPv6 extension headers to skip." — §4.4.1.1, `rfc4301.txt:1481-1483`

- Strength: should. Class: internal.
- Check idea: look for a configuration mechanism that selects which IPv6 extension headers to
  skip and check that it is present and lets the administrator change the skip list.

### RFC4301-SEL-10

**The default configuration for headers to skip when locating the Next Layer Protocol should include Hop-by-Hop Options (0), Routing Header (43), Fragmentation Header (44), and Destination Options (60).**

> "The default configuration for which protocols to skip SHOULD include the following
> protocols: 0 (Hop-by-hop options), 43 (Routing Header), 44 (Fragmentation Header), and 60
> (Destination Options)." — §4.4.1.1, `rfc4301.txt:1483-1486`

- Strength: should. Class: internal.
- Check idea: inspect the default extension-header skip list of a newly configured
  implementation and check that it includes protocol numbers 0, 43, 44, and 60.

### RFC4301-SEL-11

**The default skip list does not include AH (51) or ESP (50); IPsec treats AH and ESP as Next Layer Protocols for selector lookup.**

> "The default list does NOT include 51 (AH) or 50 (ESP)." and "From a selector lookup point of
> view, IPsec treats AH and ESP as Next Layer Protocols." — §4.4.1.1, `rfc4301.txt:1486-1488`

- Strength: description. Class: internal.
- Check idea: inspect the default extension-header skip list and check that it excludes
  protocol numbers 50 and 51, and check that a selector lookup can match on AH or ESP as the
  Next Layer Protocol.

### RFC4301-SEL-12

**When the Next Layer Protocol uses two ports, such as TCP, UDP, or SCTP, the SPD entry has Local and Remote Port selectors, each a list of ranges of values.**

> "If the Next Layer Protocol uses two ports (as do TCP, UDP, SCTP, and others), then there are
> selectors for Local and Remote Ports." and "Each of these selectors has a list of ranges of
> values." — §4.4.1.1, `rfc4301.txt:1493-1494`, `rfc4301.txt:1494-1496`

- Strength: description. Class: internal.
- Check idea: configure an SPD entry for a two-port protocol and check that it accepts Local and
  Remote Port selectors, each as a list of value ranges.

### RFC4301-SEL-13

**A port selector must support the value OPAQUE, because the port fields may be unavailable in a fragment or encrypted by IPsec.**

> "the Local and Remote ports may not be available in the case of receipt of a fragmented packet
> or if the port fields have been protected by IPsec (encrypted); thus, a value of OPAQUE also
> MUST be supported." — §4.4.1.1, `rfc4301.txt:1496-1499`

- Strength: must. Class: internal.
- Check idea: configure a port selector with the value OPAQUE and check that the SPD entry
  accepts it.

### RFC4301-SEL-14

**A port selector set to a value other than ANY or OPAQUE cannot match a packet that is a non-initial fragment, because port values are unavailable there.**

> "In a non-initial fragment, port values will not be available." and "If a port selector
> specifies a value other than ANY or OPAQUE, it cannot match packets that are non-initial
> fragments." — §4.4.1.1, `rfc4301.txt:1499-1500`, `rfc4301.txt:1500-1502`

- Strength: description. Class: internal.
- Check idea: send a non-initial fragment against an SPD entry whose port selector is a specific
  value and check that it does not match.

### RFC4301-SEL-15

**When the SA requires a port value other than ANY or OPAQUE, an implementation must discard an arriving fragment that has no port fields.**

> "If the SA requires a port value other than ANY or OPAQUE, an arriving fragment without ports
> MUST be discarded." — §4.4.1.1, `rfc4301.txt:1502-1504`

- Strength: must. Class: end-to-end.
- Check idea: send a fragment without port fields toward an SA that requires a specific port
  value and check that the implementation discards the fragment.

### RFC4301-SEL-16

**When the Next Layer Protocol is a Mobility Header, the SPD entry has a selector for the IPv6 Mobility Header message type (MH type), an 8-bit value identifying the mobility message.**

> "If the Next Layer Protocol is a Mobility Header, then there is a selector for IPv6 Mobility
> Header message type (MH type) [Mobip]." and "This is an 8-bit value that identifies a
> particular mobility message." — §4.4.1.1, `rfc4301.txt:1507-1509`, `rfc4301.txt:1509-1510`

- Strength: description. Class: internal.
- Check idea: configure an SPD entry whose Next Layer Protocol is the Mobility Header and check
  that it accepts an 8-bit MH type selector.

### RFC4301-SEL-17

**The Mobility Header message type may not be available when a fragmented packet is received.**

> "the MH type may not be available" and "in the case of receipt of a fragmented packet." —
> §4.4.1.1, `rfc4301.txt:1510`, `rfc4301.txt:1519`

- Strength: may (lower case). Class: internal.
- Check idea: send a fragmented Mobility Header packet and check that the implementation treats
  the MH type selector as unavailable.

### RFC4301-SEL-18

**When the Next Layer Protocol is ICMP, the SPD entry has a 16-bit selector for the ICMP message type and code: an 8-bit message type, or ANY, and an 8-bit code.**

> "If the Next Layer Protocol value is ICMP, then there is a 16-bit selector for the ICMP
> message type and code." and "The message type is a single 8-bit value, which defines the type
> of an ICMP message, or ANY." and "The ICMP code is a single 8-bit value that defines a
> specific subtype for an ICMP message." — §4.4.1.1, `rfc4301.txt:1524-1528`

- Strength: description. Class: internal.
- Check idea: configure an SPD entry whose Next Layer Protocol is ICMP and check that it accepts
  an 8-bit message type value, or ANY, and an 8-bit code value.

### RFC4301-SEL-19

**The ICMP type/code selector can express a single type with a range of codes, a single type with any code, or any type with any code.**

> "This 16-bit selector can contain a single type and a range of codes, a single type and ANY
> code, and ANY type and ANY code." — §4.4.1.1, `rfc4301.txt:1531-1533`

- Strength: description. Class: internal.
- Check idea: configure ICMP type/code selectors in each of the three forms and check that the
  SPD entry accepts a single type with a code range, a single type with ANY code, and ANY type
  with ANY code.

### RFC4301-SEL-20

**Given a policy entry with a Type range T-start to T-end and a Code range C-start to C-end, an implementation must test whether an ICMP packet's Type t and Code c satisfy (T-start*256)+C-start <= (t*256)+c <= (T-end*256)+C-end to decide a match.**

> "Given a policy entry with a range of Types (T-start to T-end) and a range of Codes (C-start
> to C-end), and an ICMP packet with Type t and Code c, an implementation MUST test for a match
> using" and "(T-start*256) + C-start <= (t*256) + c <= (T-end*256) + C-end" — §4.4.1.1,
> `rfc4301.txt:1533-1536`, `rfc4301.txt:1538-1539`

- Strength: must. Class: internal.
- Check idea: configure a policy entry with Type and Code ranges and send ICMP packets with Type
  and Code values inside and outside the computed bound, and check that the implementation
  matches exactly the packets that satisfy the formula.

### RFC4301-SEL-21

**The ICMP message type and code may not be available when a fragmented packet is received.**

> "the ICMP message type and code may not be available in the case of receipt of a fragmented
> packet." — §4.4.1.1, `rfc4301.txt:1541-1542`

- Strength: may (lower case). Class: internal.
- Check idea: send a fragmented ICMP packet and check that the implementation treats the message
  type and code selector as unavailable.

### RFC4301-SEL-22

**The Name selector, unlike the other selectors, is not taken from a packet; it is a symbolic identifier for an IPsec Local or Remote address.**

> "This is not a selector like the others above." and "It is not acquired from a packet." and
> "A name may be used as a symbolic identifier for an IPsec Local or Remote address." —
> §4.4.1.1, `rfc4301.txt:1545-1547`

- Strength: may (lower case). Class: internal.
- Check idea: inspect a named SPD entry and check that its Name value is a symbolic identifier
  configured locally, not extracted from a packet field.

### RFC4301-SEL-23

**A responder uses a named SPD entry for access control when an IP address is not appropriate for the Remote IP address selector, such as for a road warrior; the initiator's source IP address is then bound to the Remote IP address of the SAD entry the IKE negotiation creates, overriding the SPD entry's Remote IP address value.**

> "A named SPD entry is used by a responder (not an initiator) in support of access control when
> an IP address would not be appropriate for the Remote IP address selector" and "the
> initiator's Source IP address (inner IP header in tunnel mode) is bound to the Remote IP
> address in the SAD entry created by the IKE negotiation." and "This address overrides the
> Remote IP address value in the SPD, when the SPD entry is selected in this fashion." —
> §4.4.1.1, `rfc4301.txt:1550-1553`, `rfc4301.txt:1555-1557`, `rfc4301.txt:1557-1559`

- Strength: description. Class: internal.
- Check idea: select a named SPD entry as a responder and check that the created SAD entry's
  Remote IP address is the initiator's source address, overriding any address value in the SPD
  entry.

### RFC4301-SEL-24

**All IPsec implementations must support the responder's use of named SPD entries.**

> "All IPsec implementations MUST support this use of names." — §4.4.1.1,
> `rfc4301.txt:1559-1560`

- Strength: must. Class: internal.
- Check idea: inspect an IPsec implementation acting as a responder and check that it supports
  named SPD entries of the four identifier types.

### RFC4301-SEL-25

**An initiator may use a named SPD entry to identify a user for whom an SA is created or traffic is bypassed; the initiator's IP source address then replaces the local address in the SPD cache entry and the outbound SAD entry, and the remote address in the inbound SAD entry.**

> "A named SPD entry may be used by an initiator to identify a user for whom an IPsec SA will be
> created (or for whom traffic may be bypassed)." and "The initiator's IP source address (from
> inner IP header in tunnel mode) is used to replace the following if and when they are
> created:" and "local address in the SPD cache entry" and "local address in the outbound SAD
> entry" and "remote address in the inbound SAD entry" — §4.4.1.1, `rfc4301.txt:1562-1564`,
> `rfc4301.txt:1564-1566`, `rfc4301.txt:1575`, `rfc4301.txt:1576`, `rfc4301.txt:1577`

- Strength: may (lower case). Class: internal.
- Check idea: select a named SPD entry as an initiator and check that the created SPD cache
  entry and outbound SAD entry take the local address from the initiator's IP source address,
  and that the inbound SAD entry takes the remote address from it.

### RFC4301-SEL-26

**Support for the initiator's use of named SPD entries is optional for multi-user, native host implementations and does not apply to other implementations.**

> "Support for this use is optional for multi-user, native host implementations and not
> applicable to other implementations." — §4.4.1.1, `rfc4301.txt:1579-1580`

- Strength: description. Class: internal.
- Check idea: inspect a multi-user, native host implementation and check whether it offers the
  initiator's named-SPD-entry substitution as an optional feature, and check that a non-host
  implementation does not offer it.

### RFC4301-SEL-27

**An SPD entry can contain both a name, or a list of names, and values for the Local or Remote IP address at the same time.**

> "An SPD entry can contain both a name (or a list of names) and also values for the Local or
> Remote IP address." — §4.4.1.1, `rfc4301.txt:1586-1587`

- Strength: description. Class: internal.
- Check idea: configure an SPD entry with both a name and Local or Remote IP address values and
  check that the SPD accepts both together.

### RFC4301-SEL-28

**For the responder's use, a named SPD entry's identifier is one of four types: a fully qualified user name (email), a fully qualified DNS name, an X.500 distinguished name, or a byte string.**

> "For case 1, responder, the identifiers employed in named SPD entries are one of the following
> four types:" — §4.4.1.1, `rfc4301.txt:1589-1590`

- Strength: description. Class: internal.
- Check idea: configure a responder's named SPD entry with each of the four identifier types and
  check that the SPD accepts each one.

### RFC4301-SEL-29

**For the initiator's use, a named SPD entry's identifier is a byte string, of only local significance, and it is never transmitted.**

> "For case 2, initiator, the identifiers employed in named SPD entries are of type byte
> string." and "this identifier is only of local concern and is not transmitted." — §4.4.1.1,
> `rfc4301.txt:1609-1610`, `rfc4301.txt:1612-1613`

- Strength: description. Class: internal.
- Check idea: configure an initiator's named SPD entry with a byte-string identifier and check
  that the identifier never appears in transmitted traffic.

## The structure of an SPD entry

### RFC4301-SPDE-1

**Users must not include multiple selector sets in a single SPD entry unless the access control intent matches the IKE mix-and-match semantics.**

> "users MUST NOT include multiple selector sets in a single SPD entry unless the access control
> intent aligns with the IKE "mix and match" semantics." — §4.4.1.2, `rfc4301.txt:1649-1651`

- Strength: must not. Class: internal.
- Check idea: create an SPD entry with multiple selector sets whose intent does not match the
  IKE mix-and-match semantics and check that the implementation rejects it or forbids it.

### RFC4301-SPDE-2

**An implementation may warn a user who creates an SPD entry with multiple selector sets whose syntax indicates a possible conflict with the IKE semantics.**

> "An implementation MAY warn users, to alert them to this problem if users create SPD entries
> with multiple selector sets, the syntax of which indicates possible conflicts with current IKE
> semantics." — §4.4.1.2, `rfc4301.txt:1651-1654`

- Strength: may. Class: internal.
- Check idea: create an SPD entry with multiple selector sets whose syntax conflicts with the
  IKE semantics and check whether the implementation issues a warning.

### RFC4301-SPDE-3

**The Remote and Local labels apply only to IP addresses and ports, not to the ICMP message type/code or the Mobility Header type.**

> "Remote/Local apply only to IP addresses and ports, not to ICMP message type/code or Mobility
> Header type." — §4.4.1.2, `rfc4301.txt:1660-1661`

- Strength: description. Class: internal.
- Check idea: inspect an SPD entry selector for ICMP type/code or Mobility Header type and check
  that it carries no Remote/Local label.

### RFC4301-SPDE-4

**When a selector's value list uses the reserved value OPAQUE or ANY, that value is the only value in the list, and it appears only once.**

> "if the reserved, symbolic selector value OPAQUE or ANY is employed for a given selector type,
> only that value may appear in the list for that selector, and it must appear only once in the
> list for that selector." — §4.4.1.2, `rfc4301.txt:1662-1665`

- Strength: may, must (lower case). Class: internal.
- Check idea: configure a selector's value list with OPAQUE or ANY together with another value,
  or repeated more than once, and check that the SPD entry rejects the list.

### RFC4301-SPDE-5

**ANY and OPAQUE are local syntax conventions; IKEv2 negotiates them as the ranges start=0/ end=<max> for ANY and start=<max>/end=0 for OPAQUE.**

> "ANY and OPAQUE are local syntax conventions -- IKEv2 negotiates these values via the ranges
> indicated below:" and "ANY: start = 0 end = <max>" and "OPAQUE: start = <max> end = 0" —
> §4.4.1.2, `rfc4301.txt:1665-1666`, `rfc4301.txt:1668`, `rfc4301.txt:1669`

- Strength: description. Class: encoding.
- Check idea: observe the range values an implementation sends via IKEv2 for a selector
  configured as ANY or OPAQUE, and check that ANY appears as the range 0 to max and OPAQUE as
  the range max to 0.

### RFC4301-SPDE-6

**The Name field of an SPD entry is an optional list of IDs, a quasi-selector.**

> "Name -- a list of IDs. This quasi-selector is optional." — §4.4.1.2, `rfc4301.txt:1674`

- Strength: description. Class: internal.
- Check idea: inspect an SPD entry with no Name value and check that the SPD entry is still
  valid, since the Name field is optional.

### RFC4301-SPDE-7

**An IPsec implementation must support the name forms for the Name field that are described in the Selectors section.**

> "The forms that MUST be supported are described above in Section 4.4.1.1 under "Name"." —
> §4.4.1.2, `rfc4301.txt:1675-1676`

- Strength: must. Class: internal.
- Check idea: configure an SPD entry's Name field with each name form described for the
  responder and the initiator cases and check that the implementation accepts them.

### RFC4301-SPDE-8

**The PFP flags field of an SPD entry has one flag per traffic selector, and each flag applies to that selector across every selector set in the entry.**

> "PFP flags -- one per traffic selector." and "A given flag, e.g., for Next Layer Protocol,
> applies to the relevant selector across all "selector sets" (see below) contained in an SPD
> entry." — §4.4.1.2, `rfc4301.txt:1687-1690`

- Strength: description. Class: internal.
- Check idea: inspect the PFP flags of an SPD entry with several selector sets and check that
  one Next Layer Protocol flag governs that selector across all its selector sets.

### RFC4301-SPDE-9

**When an SA is created, each PFP flag states whether that selector's value comes from the packet that triggered the SA or from the SPD entry.**

> "When creating an SA, each flag specifies for the corresponding traffic selector whether to
> instantiate the selector from the corresponding field in the packet that triggered the
> creation of the SA or from the value(s) in the corresponding SPD entry" — §4.4.1.2,
> `rfc4301.txt:1690-1694`

- Strength: description. Class: internal.
- Check idea: create an SA from an SPD entry with a PFP flag set and with it cleared, and check
  that the SA's selector value comes from the packet in the first case and from the SPD entry in
  the second.

### RFC4301-SPDE-10

**An SPD entry has a PFP flag each for Local Address, Remote Address, Next Layer Protocol, the Local Port or ICMP type/code or Mobility Header type, and the Remote Port or ICMP type/code or Mobility Header type.**

> "There are PFP flags for:" and "Local Address" and "Remote Address" and "Next Layer Protocol"
> and "Local Port, or ICMP message type/code or Mobility Header type (depending on the next
> layer protocol)" and "Remote Port, or ICMP message type/code or Mobility Header type (depending
> on the next layer protocol)" — §4.4.1.2, `rfc4301.txt:1698-1705`

- Strength: description. Class: internal.
- Check idea: inspect an SPD entry and check that it carries a separate PFP flag for each of the
  five listed selectors.

### RFC4301-SPDE-11

**An SPD entry has one to N selector sets, the conditions for applying its IPsec action, each with a Local Address, a Remote Address, a Next Layer Protocol, a Local Port or ICMP type/code or Mobility Header type, and a Remote Port or ICMP type/code or Mobility Header type.**

> "One to N selector sets that correspond to the "condition" for applying a particular IPsec
> action." and "Each selector set contains:" and "Local Address" and "Remote Address" and "Next
> Layer Protocol" and "Local Port, or ICMP message type/code or Mobility Header type (depending
> on the next layer protocol)" and "Remote Port, or ICMP message type/code or Mobility Header
> type (depending on the next layer protocol)" — §4.4.1.2, `rfc4301.txt:1707-1716`

- Strength: description. Class: internal.
- Check idea: inspect an SPD entry's selector sets and check that each one carries the five
  listed selector fields.

### RFC4301-SPDE-12

**The Next Layer Protocol selector is a single value in each selector set, unlike the address selectors, so an SPD entry can associate several protocols and ports with one SA by listing several selector sets.**

> "The "next protocol" selector is an individual value (unlike the local and remote IP
> addresses) in a selector set entry." and "It is possible to associate multiple protocols (and
> ports) with a single SA by specifying multiple selector sets for that SA." — §4.4.1.2,
> `rfc4301.txt:1718-1720`, `rfc4301.txt:1723-1725`

- Strength: description. Class: internal.
- Check idea: configure an SPD entry with several selector sets naming different Next Layer
  Protocol values and check that the resulting SA covers traffic of each protocol.

### RFC4301-SPDE-13

**The Processing info field of an SPD entry states one action, PROTECT, BYPASS, or DISCARD, that applies to all the entry's selector sets together, not a separate action per set.**

> "Processing info -- which action is required -- PROTECT, BYPASS, or DISCARD." and "There is
> just one action that goes with all the selector sets, not a separate action for each set." —
> §4.4.1.2, `rfc4301.txt:1727-1730`

- Strength: description. Class: internal.
- Check idea: inspect an SPD entry with several selector sets and check that it carries one
  processing action shared by all of them.

### RFC4301-SPDE-14

**For a PROTECT entry, the IPsec mode field states tunnel or transport mode.**

> "IPsec mode -- tunnel or transport" — §4.4.1.2, `rfc4301.txt:1732`

- Strength: description. Class: internal.
- Check idea: inspect a PROTECT SPD entry and check that its IPsec mode field states tunnel or
  transport.

### RFC4301-SPDE-15

**For a tunnel-mode PROTECT entry on a non-mobile host with multiple interfaces, the local tunnel address must be statically configured; on a mobile host, it is set outside IPsec.**

> "(if tunnel mode) local tunnel address -- For a non-mobile host, if there is just one
> interface, this is straightforward; if there are multiple interfaces, this must be statically
> configured." and "For a mobile host, the specification of the local address is handled
> externally to IPsec." — §4.4.1.2, `rfc4301.txt:1743-1746`, `rfc4301.txt:1746-1748`

- Strength: must (lower case). Class: internal.
- Check idea: configure a tunnel-mode PROTECT entry on a non-mobile host with multiple
  interfaces and check that the local tunnel address is a statically configured value.

### RFC4301-SPDE-16

**For a tunnel-mode PROTECT entry, the entry holds a remote tunnel address, for which this document defines no standard way to determine the value.**

> "(if tunnel mode) remote tunnel address -- There is no standard way to determine this." —
> §4.4.1.2, `rfc4301.txt:1749-1750`

- Strength: description. Class: internal.
- Check idea: inspect a tunnel-mode PROTECT entry and check that it carries a remote tunnel
  address value.

### RFC4301-SPDE-17

**A PROTECT entry has an Extended Sequence Number field that states whether the SA uses extended sequence numbers.**

> "Extended Sequence Number -- Is this SA using extended sequence numbers?" — §4.4.1.2,
> `rfc4301.txt:1752-1753`

- Strength: description. Class: internal.
- Check idea: inspect a PROTECT entry's SA and check that its Extended Sequence Number field
  states whether extended sequence numbers are in use.

### RFC4301-SPDE-18

**A PROTECT entry has a stateful fragment checking field that states whether the SA uses stateful fragment checking.**

> "stateful fragment checking -- Is this SA using stateful fragment checking?" — §4.4.1.2,
> `rfc4301.txt:1754-1755`

- Strength: description. Class: internal.
- Check idea: inspect a PROTECT entry's SA and check that its stateful fragment checking field
  states whether that checking is in use.

### RFC4301-SPDE-19

**A tunnel-mode PROTECT entry has a Bypass DF bit field, true or false.**

> "Bypass DF bit (T/F) -- applicable to tunnel mode SAs" — §4.4.1.2, `rfc4301.txt:1757`

- Strength: description. Class: internal.
- Check idea: inspect a tunnel-mode PROTECT entry and check that its Bypass DF bit field is set
  to true or false.

### RFC4301-SPDE-20

**A tunnel-mode PROTECT entry has a field that either bypasses the DSCP value or maps it to an array of unprotected DSCP values, to restrict which DSCP values bypass.**

> "Bypass DSCP (T/F) or map to unprotected DSCP values (array) if needed to restrict bypass of
> DSCP values -- applicable to tunnel mode SAs" — §4.4.1.2, `rfc4301.txt:1758-1760`

- Strength: description. Class: internal.
- Check idea: inspect a tunnel-mode PROTECT entry and check that its DSCP field either bypasses
  the DSCP value or maps it through an array of unprotected values.

### RFC4301-SPDE-21

**A PROTECT entry has an IPsec protocol field that states AH or ESP.**

> "IPsec protocol -- AH or ESP" — §4.4.1.2, `rfc4301.txt:1761`

- Strength: description. Class: internal.
- Check idea: inspect a PROTECT entry and check that its IPsec protocol field states AH or ESP.

### RFC4301-SPDE-22

**A PROTECT entry has an algorithms field that lists, in decreasing priority order, which algorithms to use for AH, for ESP, and for combined mode.**

> "algorithms -- which ones to use for AH, which ones to use for ESP, which ones to use for
> combined mode, ordered by decreasing priority" — §4.4.1.2, `rfc4301.txt:1762-1764`

- Strength: description. Class: internal.
- Check idea: inspect a PROTECT entry's algorithms field and check that it lists the AH, ESP, and
  combined-mode algorithms in decreasing priority order.

## NLP: Fields of the next layer protocols

### RFC4301-NLP-1

**The SPD entry sets the Local and Remote port selectors to OPAQUE when a Next Layer Protocol has no port selector.**

> "If a Next Layer Protocol has no "port" selectors, then the Local and Remote "port" selectors are set to OPAQUE in the relevant SPD entry" — §4.4.1.3, `rfc4301.txt:1782-1784`

- Strength: description. Class: internal.
- Check idea: Configure an SPD entry for a Next Layer Protocol that has no port-type selector. Confirm both the Local and Remote port selectors of the entry hold OPAQUE.

### RFC4301-NLP-2

**The SPD entry sets both the Local and Remote port selectors to the same specified value when a system is willing to send and receive traffic with that port value.**

> "if Mobility Headers of a specified type are allowed to be sent and received via an SA, then the relevant SPD entry would be set as follows" — §4.4.1.3, `rfc4301.txt:1807-1809`, and "next layer protocol = Mobility Header "port" selector = Mobility Header message type" — §4.4.1.3, `rfc4301.txt:1812-1813`, `rfc4301.txt:1816-1817`

- Strength: description. Class: internal.
- Check idea: Configure an SA that is allowed to send and receive traffic with a specified port value of a Next Layer Protocol that has one port selector. Confirm the SPD entry's Local and Remote port selectors both hold that value.

### RFC4301-NLP-3

**The SPD entry sets the Local port selector to a specified value and the Remote port selector to OPAQUE when a system is willing to send, but not receive, traffic with that port value.**

> "If Mobility Headers of a specified type are allowed to be sent but NOT received via an SA, then the relevant SPD entry would be set as follows" — §4.4.1.3, `rfc4301.txt:1819-1821`, and "next layer protocol = Mobility Header "port" selector = Mobility Header message type" — §4.4.1.3, `rfc4301.txt:1823-1825`, and "next layer protocol = Mobility Header "port" selector = OPAQUE" — §4.4.1.3, `rfc4301.txt:1827-1829`

- Strength: description. Class: internal.
- Check idea: Configure an SA that is allowed to send, but not receive, traffic with a specified port value of a Next Layer Protocol that has one port selector. Confirm the SPD entry's Local port selector holds that value and the Remote port selector holds OPAQUE.

### RFC4301-NLP-4

**The SPD entry sets the Local port selector to OPAQUE and the Remote port selector to a specified value when a system is willing to receive, but not send, traffic with that port value.**

> "If Mobility Headers of a specified type are allowed to be received but NOT sent via an SA, then the relevant SPD entry would be set as follows" — §4.4.1.3, `rfc4301.txt:1831-1833`, and "next layer protocol = Mobility Header "port" selector = OPAQUE" — §4.4.1.3, `rfc4301.txt:1835-1837`, and "next layer protocol = Mobility Header "port" selector = Mobility Header message type" — §4.4.1.3, `rfc4301.txt:1839-1841`

- Strength: description. Class: internal.
- Check idea: Configure an SA that is allowed to receive, but not send, traffic with a specified port value of a Next Layer Protocol that has one port selector. Confirm the SPD entry's Local port selector holds OPAQUE and the Remote port selector holds that value.

### RFC4301-NLP-5

**The SPD entry sets the Local port selector to a specific value and the Remote port selector to OPAQUE when a system is willing to send, but not receive, traffic with that particular port value, for a Next Layer Protocol with two port selectors.**

> "If a system is willing to send traffic with a particular "port" value but NOT receive traffic with that kind of port value, the system's traffic selectors are set as follows in the relevant SPD entry" — §4.4.1.3, `rfc4301.txt:1843-1846`, and "next layer protocol = ICMP "port" selector = <specific ICMP type & code>" — §4.4.1.3, `rfc4301.txt:1855-1857`, and "next layer protocol = ICMP "port" selector = OPAQUE" — §4.4.1.3, `rfc4301.txt:1859-1861`

- Strength: description. Class: internal.
- Check idea: Configure an SA for a Next Layer Protocol with two port selectors, such as ICMP type and code, that is willing to send but not receive traffic with a particular port value. Confirm the SPD entry's Local port selector holds that specific value and the Remote port selector holds OPAQUE.

### RFC4301-NLP-6

**The SPD entry sets the Local port selector to OPAQUE and the Remote port selector to a specific value when a system is willing to receive, but not send, traffic with that particular port value.**

> "To indicate that a system is willing to receive traffic with a particular "port" value but NOT send that kind of traffic, the system's traffic selectors are set as follows in the relevant SPD entry" — §4.4.1.3, `rfc4301.txt:1863-1866`, and "next layer protocol = ICMP "port" selector = OPAQUE" — §4.4.1.3, `rfc4301.txt:1868-1870`, and "next layer protocol = ICMP "port" selector = <specific ICMP type & code>" — §4.4.1.3, `rfc4301.txt:1872-1874`

- Strength: description. Class: internal.
- Check idea: Configure an SA that is willing to receive, but not send, traffic with a particular port value. Confirm the SPD entry's Local port selector holds OPAQUE and the Remote port selector holds that specific value.

## SAD: The Security Association Database

### RFC4301-SAD-1

**Each IPsec implementation keeps a Security Association Database, with one entry for each SA.**

> "In each IPsec implementation, there is a nominal Security Association Database (SAD), in which each entry defines the parameters associated with one SA. Each SA has an entry in the SAD." — §4.4.2, `rfc4301.txt:1892-1894`

- Strength: description. Class: internal.
- Check idea: Create an SA on an IPsec implementation and confirm a corresponding entry appears in its Security Association Database.

### RFC4301-SAD-2

**For outbound processing, an entry in the SPD-S part of the SPD cache points to the SAD entry for the SA.**

> "For outbound processing, each SAD entry is pointed to by entries in the SPD-S part of the SPD cache." — §4.4.2, `rfc4301.txt:1894-1896`

- Strength: description. Class: internal.
- Check idea: Trigger outbound traffic that matches an SPD-S entry and confirm processing follows the SPD-S entry's pointer to the correct SAD entry.

### RFC4301-SAD-3

**For inbound processing of a unicast SA, the receiver looks up the SA by the SPI alone, or by the SPI together with the IPsec protocol type.**

> "For inbound processing, for unicast SAs, the SPI is used either alone to look up an SA or in conjunction with the IPsec protocol type." — §4.4.2, `rfc4301.txt:1896-1898`

- Strength: description. Class: internal.
- Check idea: Send an inbound unicast AH or ESP packet and confirm the receiver selects the SA using the packet's SPI, alone or together with the IPsec protocol type.

### RFC4301-SAD-4

**For inbound processing of a multicast SA, the receiver looks up the SA by the SPI together with the destination address, or by the SPI together with the destination and source addresses.**

> "If an IPsec implementation supports multicast, the SPI plus destination address, or SPI plus destination and source addresses are used to look up the SA." — §4.4.2, `rfc4301.txt:1898-1900`

- Strength: description. Class: internal.
- Check idea: Send an inbound multicast AH or ESP packet to an implementation that supports multicast and confirm the receiver selects the SA using the SPI plus destination address, or SPI plus destination and source addresses.

### RFC4301-SAD-5

**The receiver initially populates a SAD entry for an inbound SA with the selector value or values negotiated when the SA was created.**

> "For each of the selectors defined in Section 4.4.1.1, the entry for an inbound SA in the SAD MUST be initially populated with the value or values negotiated at the time the SA was created." — §4.4.2, `rfc4301.txt:1916-1919`

- Strength: must. Class: internal.
- Check idea: Negotiate an inbound SA with specific selector values and confirm the resulting SAD entry is populated with those negotiated values.

### RFC4301-SAD-6

**The receiver uses the negotiated selector values in the SAD entry to check that the header fields of an inbound packet match the SA's selectors, so verifying that the packet is consistent with the SA's policy.**

> "For a receiver, these values are used to check that the header fields of an inbound packet (after IPsec processing) match the selector values negotiated for the SA. Thus, the SAD acts as a cache for checking the selectors of inbound traffic arriving on SAs. For the receiver, this is part of verifying that a packet arriving on an SA is consistent with the policy for the SA." — §4.4.2, `rfc4301.txt:1921-1926`

- Strength: description. Class: end-to-end.
- Check idea: Send an inbound packet on an SA whose header fields do not match the SA's negotiated selector values and confirm the receiver detects the mismatch and does not deliver the packet as consistent with the SA's policy.

### RFC4301-SAD-7

**The SAD can hold an entry for an SA with no corresponding SPD entry, because this document does not require the SAD to be cleared when the SPD changes, so an SAD entry can outlive the SPD entry that created it.**

> "Note also that there are a couple of situations in which the SAD can have entries for SAs that do not have corresponding entries in the SPD. Since this document does not mandate that the SAD be selectively cleared when the SPD is changed, SAD entries can remain when the SPD entries that created them are changed or deleted." — §4.4.2, `rfc4301.txt:1929-1933`

- Strength: description. Class: internal.
- Check idea: Change or delete an SPD entry that created an existing SA and confirm the corresponding SAD entry can remain present afterward.

### RFC4301-SAD-8

**A manually keyed SA can have a SAD entry with no corresponding SPD entry.**

> "Also, if a manually keyed SA is created, there could be an SAD entry for this SA that does not correspond to any SPD entry." — §4.4.2, `rfc4301.txt:1934-1935`

- Strength: description. Class: internal.
- Check idea: Manually create an SA with no matching SPD entry and confirm a SAD entry for it exists.

### RFC4301-SAD-9

**The SAD supports a multicast SA when the SA is manually configured.**

> "The SAD can support multicast SAs, if manually configured." — §4.4.2, `rfc4301.txt:1937`

- Strength: description. Class: internal.
- Check idea: Manually configure a multicast SA and confirm the SAD holds an entry for it.

### RFC4301-SAD-10

**An outbound multicast SA has the same structure as a unicast SA, with the source address of the sender and the destination address of the multicast group.**

> "An outbound multicast SA has the same structure as a unicast SA. The source address is that of the sender, and the destination address is the multicast group address." — §4.4.2, `rfc4301.txt:1937-1940`

- Strength: description. Class: wire.
- Check idea: Send traffic outbound on a multicast SA and confirm its source address is the sender's address and its destination address is the multicast group address.

### RFC4301-SAD-11

**An inbound multicast SA is configured with the source address of each peer authorized to transmit to that multicast SA.**

> "An inbound, multicast SA must be configured with the source addresses of each peer authorized to transmit to the multicast SA in question." — §4.4.2, `rfc4301.txt:1940-1942`

- Strength: must (lower case). Class: internal.
- Check idea: Configure an inbound multicast SA and confirm it lists the source address of each peer authorized to transmit on it.

### RFC4301-SAD-12

**A multicast group controller, not the receiver, provides the SPI value for a multicast SA.**

> "The SPI value for a multicast SA is provided by a multicast group controller, not by the receiver, as for a unicast SA." — §4.4.2, `rfc4301.txt:1942-1944`

- Strength: description. Class: internal.
- Check idea: Create a multicast SA and confirm its SPI value comes from the multicast group controller rather than being chosen by the receiver.

### RFC4301-SAD-13

**A SAD entry for an inbound multicast SA is created only through manual configuration.**

> "this document does not specify an automated way to create an SAD entry for a multicast, inbound SA. Only manually configured SAD entries can be created to accommodate inbound, multicast traffic." — §4.4.2, `rfc4301.txt:1949-1952`

- Strength: description. Class: internal.
- Check idea: Attempt to establish an inbound multicast SA without manual configuration and confirm no SAD entry results; confirm one results only from manual configuration.

## SADI: Data items in the SAD

### RFC4301-SADI-1

**Each SAD entry contains a Security Parameter Index that the receiving end of the SA selected to uniquely identify the SA.**

> "The following data items MUST be in the SAD:" — §4.4.2.1, `rfc4301.txt:1981`, and "Security Parameter Index (SPI): a 32-bit value selected by the receiving end of an SA to uniquely identify the SA. In an SAD entry for an outbound SA, the SPI is used to construct the packet's AH or ESP header. In an SAD entry for an inbound SA, the SPI is used to map traffic to the appropriate SA" — §4.4.2.1, `rfc4301.txt:1983-1987`

- Strength: must. Class: internal.
- Check idea: Inspect the SAD entry of an SA and confirm it holds a 32-bit SPI; confirm outbound packets on the SA carry this SPI in their AH or ESP header, and inbound traffic is mapped to the SA by this SPI.

### RFC4301-SADI-2

**Each SAD entry contains a Sequence Number Counter used to generate the Sequence Number field of AH or ESP headers, 64 bits wide by default or 32 bits wide when negotiated.**

> "Sequence Number Counter: a 64-bit counter used to generate the Sequence Number field in AH or ESP headers. 64-bit sequence numbers are the default, but 32-bit sequence numbers are also supported if negotiated." — §4.4.2.1, `rfc4301.txt:1990-1993`

- Strength: must. Class: internal.
- Check idea: Inspect the SAD entry of an SA and confirm it holds a Sequence Number Counter, 64 bits wide by default and 32 bits wide when 32-bit sequence numbers were negotiated for the SA.

### RFC4301-SADI-3

**Each SAD entry contains a Sequence Counter Overflow flag that states whether counter overflow generates an auditable event and blocks further transmission on the SA, or whether the counter is allowed to roll over.**

> "Sequence Counter Overflow: a flag indicating whether overflow of the sequence number counter should generate an auditable event and prevent transmission of additional packets on the SA, or whether rollover is permitted." — §4.4.2.1, `rfc4301.txt:1995-1998`

- Strength: must. Class: internal.
- Check idea: Inspect the SAD entry of an SA and confirm it holds a Sequence Counter Overflow flag; drive the sequence number counter to overflow and confirm the implementation follows the flag's setting, either blocking further transmission or letting the counter roll over.

### RFC4301-SADI-4

**The audit log entry for a Sequence Number Counter overflow event should include the SPI value, the current date and time, the Local Address, the Remote Address, and the selectors of the SAD entry.**

> "The audit log entry for this event SHOULD include the SPI value, current date/time, Local Address, Remote Address, and the selectors from the relevant SAD entry." — §4.4.2.1, `rfc4301.txt:1998-2000`

- Strength: should. Class: internal.
- Check idea: Drive the Sequence Number Counter of an SA to overflow with auditing enabled and confirm the resulting audit log entry includes the SPI value, current date and time, Local Address, Remote Address, and the SA's selectors.

### RFC4301-SADI-5

**Each SAD entry contains an Anti-Replay Window, a 64-bit counter and bit-map used to detect a replayed inbound AH or ESP packet, ignored for an SA on which anti-replay has been disabled.**

> "Anti-Replay Window: a 64-bit counter and a bit-map (or equivalent) used to determine whether an inbound AH or ESP packet is a replay." — §4.4.2.1, `rfc4301.txt:2002-2003`, and "If anti-replay has been disabled by the receiver for an SA, e.g., in the case of a manually keyed SA, then the Anti-Replay Window is ignored for the SA in question." — §4.4.2.1, `rfc4301.txt:2005-2007`

- Strength: must. Class: internal.
- Check idea: Inspect the SAD entry of an SA for an Anti-Replay Window; replay an inbound AH or ESP packet on the SA and confirm it is detected, except on an SA where anti-replay has been disabled, where a replayed packet is not rejected on that basis.

### RFC4301-SADI-6

**A SAD entry contains an AH Authentication algorithm and key only when AH is supported.**

> "AH Authentication algorithm, key, etc. This is required only if AH is supported." — §4.4.2.1, `rfc4301.txt:2011-2012`

- Strength: description. Class: internal.
- Check idea: Inspect the SAD entry of an SA that uses AH and confirm it holds an AH Authentication algorithm and key; confirm an SA that does not support AH has no such data item.

### RFC4301-SADI-7

**A SAD entry contains an ESP Encryption algorithm, key, mode and IV, unless a combined mode algorithm is used, in which case these fields do not apply.**

> "ESP Encryption algorithm, key, mode, IV, etc. If a combined mode algorithm is used, these fields will not be applicable." — §4.4.2.1, `rfc4301.txt:2023-2024`

- Strength: description. Class: internal.
- Check idea: Inspect the SAD entry of an SA that uses ESP with a separate encryption algorithm and confirm it holds an encryption algorithm, key, mode, and IV; confirm an SA that uses an ESP combined mode algorithm has no such data item.

### RFC4301-SADI-8

**A SAD entry contains an ESP integrity algorithm and keys, unless the integrity service is not selected or a combined mode algorithm is used, in which case these fields do not apply.**

> "ESP integrity algorithm, keys, etc. If the integrity service is not selected, these fields will not be applicable. If a combined mode algorithm is used, these fields will not be applicable." — §4.4.2.1, `rfc4301.txt:2026-2028`

- Strength: description. Class: internal.
- Check idea: Inspect the SAD entry of an SA that selects the ESP integrity service with a separate integrity algorithm and confirm it holds an integrity algorithm and keys; confirm an SA without the integrity service, or one using a combined mode algorithm, has no such data item.

### RFC4301-SADI-9

**A SAD entry contains an ESP combined mode algorithm and keys only when a combined mode algorithm is used with ESP on that SA.**

> "ESP combined mode algorithms, key(s), etc. This data is used when a combined mode (encryption and integrity) algorithm is used with ESP. If a combined mode algorithm is not used, these fields are not applicable." — §4.4.2.1, `rfc4301.txt:2030-2033`

- Strength: description. Class: internal.
- Check idea: Inspect the SAD entry of an SA that uses an ESP combined mode algorithm and confirm it holds the combined mode algorithm and keys; confirm an SA that does not use a combined mode algorithm has no such data item.

### RFC4301-SADI-10

**Each SAD entry contains the Lifetime of the SA, a time interval, byte count, or both, after which the SA must be replaced or terminated, with the first to expire taking precedence when both are used.**

> "Lifetime of this SA: a time interval after which an SA must be replaced with a new SA (and new SPI) or terminated, plus an indication of which of these actions should occur. This may be expressed as a time or byte count, or a simultaneous use of both with the first lifetime to expire taking precedence." — §4.4.2.1, `rfc4301.txt:2035-2039`

- Strength: must (lower case). Class: internal.
- Check idea: Inspect the SAD entry of an SA and confirm it holds a Lifetime, as a time interval, a byte count, or both; let the SA reach its lifetime and confirm it is replaced or terminated as indicated, with the first of a time and byte limit to expire taking precedence.

### RFC4301-SADI-11

**A compliant implementation supports both a time-based and a byte-count-based SA lifetime, and supports the simultaneous use of both on one SA.**

> "A compliant implementation MUST support both types of lifetimes, and MUST support a simultaneous use of both." — §4.4.2.1, `rfc4301.txt:2039-2041`

- Strength: must. Class: internal.
- Check idea: Configure an SA with a time-based lifetime, with a byte-count-based lifetime, and with both simultaneously, and confirm the implementation enforces each configuration.

### RFC4301-SADI-12

**When byte count is used for the SA lifetime, the implementation should count the bytes to which the IPsec cryptographic algorithm is applied, including pad bytes.**

> "If byte count is used, then the implementation SHOULD count the number of bytes to which the IPsec cryptographic algorithm is applied. For ESP, this is the encryption algorithm (including Null encryption) and for AH, this is the authentication algorithm. This includes pad bytes, etc." — §4.4.2.1, `rfc4301.txt:2051-2055`

- Strength: should. Class: internal.
- Check idea: Configure an SA with a byte-count lifetime and send traffic that includes pad bytes; confirm the byte count used against the lifetime reflects the bytes processed by the SA's encryption algorithm for ESP or authentication algorithm for AH, pad bytes included.

### RFC4301-SADI-13

**An implementation must be able to handle the byte counters at the two ends of an SA becoming out of synch with each other.**

> "Note that implementations MUST be able to handle having the counters at the ends of an SA get out of synch, e.g., because of packet loss or because the implementations at each end of the SA aren't doing things the same way." — §4.4.2.1, `rfc4301.txt:2055-2059`

- Strength: must. Class: internal.
- Check idea: Cause the byte counters at the two ends of an SA to diverge, for example through packet loss, and confirm the implementation continues to operate without error.

### RFC4301-SADI-14

**An SA lifetime should have both a soft lifetime, which warns the implementation to start setting up a replacement SA, and a hard lifetime, at which the SA ends and is destroyed.**

> "There SHOULD be two kinds of lifetime -- a soft lifetime that warns the implementation to initiate action such as setting up a replacement SA, and a hard lifetime when the current SA ends and is destroyed." — §4.4.2.1, `rfc4301.txt:2061-2064`

- Strength: should. Class: internal.
- Check idea: Configure an SA lifetime and confirm the implementation acts on a soft-lifetime expiry by beginning to set up a replacement SA, and acts on a hard-lifetime expiry by ending and destroying the current SA.

### RFC4301-SADI-15

**A packet not fully delivered before its SA's lifetime ends should be discarded.**

> "If the entire packet does not get delivered during the SA's lifetime, the packet SHOULD be discarded." — §4.4.2.1, `rfc4301.txt:2066-2067`

- Strength: should. Class: end-to-end.
- Check idea: Send a packet on an SA whose lifetime ends before delivery completes and confirm the packet is discarded.

### RFC4301-SADI-16

**Each SAD entry contains the IPsec protocol mode, tunnel or transport, that states which mode of AH or ESP applies to traffic on the SA.**

> "IPsec protocol mode: tunnel or transport. Indicates which mode of AH or ESP is applied to traffic on this SA." — §4.4.2.1, `rfc4301.txt:2069-2070`

- Strength: must. Class: internal.
- Check idea: Inspect the SAD entry of an SA and confirm it holds a protocol mode of tunnel or transport, and confirm traffic on the SA is processed with AH or ESP in that mode.

### RFC4301-SADI-17

**Each SAD entry contains a Stateful fragment checking flag that states whether stateful fragment checking applies to the SA.**

> "Stateful fragment checking flag. Indicates whether or not stateful fragment checking applies to this SA." — §4.4.2.1, `rfc4301.txt:2079-2080`

- Strength: must. Class: internal.
- Check idea: Inspect the SAD entry of an SA and confirm it holds a Stateful fragment checking flag; send fragmented traffic on the SA and confirm stateful fragment checking is applied only when the flag is set.

### RFC4301-SADI-18

**A tunnel-mode SAD entry with both inner and outer headers in IPv4 contains a Bypass DF bit.**

> "Bypass DF bit (T/F) -- applicable to tunnel mode SAs where both inner and outer headers are IPv4." — §4.4.2.1, `rfc4301.txt:2082-2083`

- Strength: description. Class: internal.
- Check idea: Inspect the SAD entry of a tunnel-mode SA whose inner and outer headers are both IPv4 and confirm it holds a Bypass DF bit.

### RFC4301-SADI-19

**A SAD entry's DSCP values, when one or more are specified, select the SA among several that otherwise match an outbound packet's traffic selectors; when none are specified, no DSCP-specific filtering applies.**

> "DSCP values -- the set of DSCP values allowed for packets carried over this SA. If no values are specified, no DSCP-specific filtering is applied. If one or more values are specified, these are used to select one SA among several that match the traffic selectors for an outbound packet." — §4.4.2.1, `rfc4301.txt:2085-2089`

- Strength: description. Class: internal.
- Check idea: Configure two SAs that match the same outbound traffic selectors but different DSCP values, and confirm an outbound packet's DSCP value selects between them; confirm an SA with no DSCP values specified applies no DSCP-specific filtering.

### RFC4301-SADI-20

**A SAD entry's DSCP values are not checked against inbound traffic arriving on the SA.**

> "Note that these values are NOT checked against inbound traffic arriving on the SA." — §4.4.2.1, `rfc4301.txt:2089-2090`

- Strength: description. Class: end-to-end.
- Check idea: Send inbound traffic on an SA whose DSCP values do not match the packet's DSCP value and confirm the packet is not rejected on that basis.

### RFC4301-SADI-21

**A tunnel-mode SAD entry contains a Bypass DSCP flag, or a map from inner-header DSCP values to outer-header DSCP values, to restrict which DSCP values bypass mapping.**

> "Bypass DSCP (T/F) or map to unprotected DSCP values (array) if needed to restrict bypass of DSCP values -- applicable to tunnel mode SAs. This feature maps DSCP values from an inner header to values in an outer header, e.g., to address covert channel signaling concerns." — §4.4.2.1, `rfc4301.txt:2092-2096`

- Strength: description. Class: internal.
- Check idea: Configure a tunnel-mode SA with a Bypass DSCP setting or a DSCP mapping array, send a packet whose inner header carries a DSCP value, and confirm the outer header's DSCP value follows the configured bypass or mapping.

### RFC4301-SADI-22

**Each SAD entry contains a Path MTU data item that holds any observed path MTU and its aging variables.**

> "Path MTU: any observed path MTU and aging variables." — §4.4.2.1, `rfc4301.txt:2098`

- Strength: must. Class: internal.
- Check idea: Cause a path MTU to be observed for an SA and confirm the SAD entry holds the observed value together with its aging variables.

### RFC4301-SADI-23

**A tunnel-mode SAD entry contains a Tunnel header IP source and destination address, both of the same IP version.**

> "Tunnel header IP source and destination address -- both addresses must be either IPv4 or IPv6 addresses. The version implies the type of IP header to be used. Only used when the IPsec protocol mode is tunnel." — §4.4.2.1, `rfc4301.txt:2100-2103`

- Strength: must (lower case). Class: internal.
- Check idea: Inspect the SAD entry of a tunnel-mode SA and confirm it holds a tunnel header source and destination address that are both IPv4 or both IPv6, and that the outer header built for the SA uses that IP version; confirm a transport-mode SA has no such data item.

## PFP: The SPD, the PFP flag, the packet and the SAD

### RFC4301-PFP-1

**When the PFP flag is 0, the resulting local-address selector in the SAD entry equals the SPD entry's local-address selector.**

> "loc addr list of ranges 0 IP addr "S" list of ranges ANY 0 IP addr "S" ANY" — §4.4.2.2, `rfc4301.txt:2139-2140`

- Strength: description. Class: internal.
- Check idea: Create an SA from an SPD entry whose local-address selector is a list of ranges, and from one whose local-address selector is ANY, both with the PFP flag 0, and confirm each resulting SAD entry's local-address selector equals the SPD entry's value.

### RFC4301-PFP-2

**When the PFP flag is 1, the resulting local-address selector in the SAD entry equals the source address of the packet that triggered the SA.**

> "list of ranges 1 IP addr "S" "S" ANY 1 IP addr "S" "S"" — §4.4.2.2, `rfc4301.txt:2141-2142`

- Strength: description. Class: internal.
- Check idea: Create an SA from an SPD entry whose local-address selector is a list of ranges, and from one whose local-address selector is ANY, both with the PFP flag 1, triggered by a packet with source address "S", and confirm each resulting SAD entry's local-address selector equals "S".

### RFC4301-PFP-3

**When the PFP flag is 0, the resulting remote-address selector in the SAD entry equals the SPD entry's remote-address selector.**

> "rem addr list of ranges 0 IP addr "D" list of ranges ANY 0 IP addr "D" ANY" — §4.4.2.2, `rfc4301.txt:2144-2145`

- Strength: description. Class: internal.
- Check idea: Create an SA from an SPD entry whose remote-address selector is a list of ranges, and from one whose remote-address selector is ANY, both with the PFP flag 0, and confirm each resulting SAD entry's remote-address selector equals the SPD entry's value.

### RFC4301-PFP-4

**When the PFP flag is 1, the resulting remote-address selector in the SAD entry equals the destination address of the packet that triggered the SA.**

> "list of ranges 1 IP addr "D" "D" ANY 1 IP addr "D" "D"" — §4.4.2.2, `rfc4301.txt:2146-2147`

- Strength: description. Class: internal.
- Check idea: Create an SA from an SPD entry whose remote-address selector is a list of ranges, and from one whose remote-address selector is ANY, both with the PFP flag 1, triggered by a packet with destination address "D", and confirm each resulting SAD entry's remote-address selector equals "D".

### RFC4301-PFP-5

**When the PFP flag is 0 and the triggering packet has a protocol value, the resulting protocol selector in the SAD entry equals the SPD entry's protocol selector.**

> "protocol list of prot's* 0 prot. "P" list of prot's* ANY** 0 prot. "P" ANY OPAQUE**** 0 prot. "P" OPAQUE" — §4.4.2.2, `rfc4301.txt:2149-2151`

- Strength: description. Class: internal.
- Check idea: Create an SA from an SPD entry whose protocol selector is a list of protocols, ANY, or OPAQUE, with the PFP flag 0, triggered by a packet with an available protocol value, and confirm each resulting SAD entry's protocol selector equals the SPD entry's value.

### RFC4301-PFP-6

**When the PFP flag is 0, the SPD entry's protocol selector is a list of protocols, and the triggering packet has no available protocol value, the packet is discarded.**

> "list of prot's* 0 not avail. discard packet" — §4.4.2.2, `rfc4301.txt:2153`

- Strength: description. Class: end-to-end.
- Check idea: Trigger an SA lookup with the PFP flag 0 against an SPD entry whose protocol selector is a list of protocols, using a packet with no available protocol value, and confirm the packet is discarded.

### RFC4301-PFP-7

**When the PFP flag is 0, the SPD entry's protocol selector is ANY or OPAQUE, and the triggering packet has no available protocol value, the resulting protocol selector in the SAD entry keeps the SPD entry's value and the packet is not discarded on this basis.**

> "ANY** 0 not avail. ANY OPAQUE**** 0 not avail. OPAQUE" — §4.4.2.2, `rfc4301.txt:2154-2155`

- Strength: description. Class: internal.
- Check idea: Trigger an SA lookup with the PFP flag 0 against an SPD entry whose protocol selector is ANY, and against one whose protocol selector is OPAQUE, using a packet with no available protocol value, and confirm the resulting SAD entry's protocol selector keeps ANY or OPAQUE and the packet is not discarded for this reason.

### RFC4301-PFP-8

**When the PFP flag is 1, the SPD entry's protocol selector is a list of protocols or ANY, and the triggering packet has a protocol value, the resulting protocol selector in the SAD entry narrows to that packet's protocol value.**

> "list of prot's* 1 prot. "P" "P" ANY** 1 prot. "P" "P"" — §4.4.2.2, `rfc4301.txt:2157-2158`

- Strength: description. Class: internal.
- Check idea: Create an SA from an SPD entry whose protocol selector is a list of protocols, and from one whose protocol selector is ANY, both with the PFP flag 1, triggered by a packet with protocol value "P", and confirm each resulting SAD entry's protocol selector equals "P".

### RFC4301-PFP-9

**The protocol selector value OPAQUE applies only to IPv6; the protocol field cannot be OPAQUE in IPv4.**

> "The protocol field cannot be OPAQUE in IPv4. This table entry applies only to IPv6." — §4.4.2.2, `rfc4301.txt:2375-2376`

- Strength: description. Class: encoding.
- Check idea: Attempt to configure an SPD entry with an OPAQUE protocol selector for IPv4 traffic and for IPv6 traffic, and confirm OPAQUE is accepted for the protocol selector only for IPv6.

### RFC4301-PFP-10

**When the PFP flag is 1, the SPD entry's protocol selector is a list of protocols or ANY, and the triggering packet has no available protocol value, the packet is discarded.**

> "list of prot's* 1 not avail. discard packet ANY** 1 not avail. discard packet" — §4.4.2.2, `rfc4301.txt:2161-2162`

- Strength: description. Class: end-to-end.
- Check idea: Trigger an SA lookup with the PFP flag 1 against an SPD entry whose protocol selector is a list of protocols, and against one whose protocol selector is ANY, using a packet with no available protocol value, and confirm the packet is discarded in each case.

### RFC4301-PFP-11

**Setting the PFP flag to 1 together with an OPAQUE selector value is an error, and an IPsec implementation should prohibit it.**

> "Use of PFP=1 with an OPAQUE value is an error and SHOULD be prohibited by an IPsec implementation." — §4.4.2.2, `rfc4301.txt:2373-2374`

- Strength: should. Class: internal.
- Check idea: Attempt to configure an SPD entry with an OPAQUE selector value, for the protocol, port, mobility header type, or ICMP type-and-code selector, together with the PFP flag set to 1, and confirm the implementation rejects the combination.

### RFC4301-PFP-12

**When the PFP flag is 0 and the triggering packet has a source port value, the resulting local-port selector in the SAD entry equals the SPD entry's local-port selector.**

> "loc port list of ranges 0 src port "s" list of ranges ANY 0 src port "s" ANY OPAQUE 0 src port "s" OPAQUE" — §4.4.2.2, `rfc4301.txt:2198-2200`

- Strength: description. Class: internal.
- Check idea: Create an SA from an SPD entry whose local-port selector is a list of ranges, ANY, or OPAQUE, with the PFP flag 0, triggered by a packet with an available source port, and confirm each resulting SAD entry's local-port selector equals the SPD entry's value.

### RFC4301-PFP-13

**When the PFP flag is 0, the SPD entry's local-port selector is a list of ranges, and the triggering packet has no available source port value, the packet is discarded.**

> "list of ranges 0 not avail. discard packet" — §4.4.2.2, `rfc4301.txt:2202`

- Strength: description. Class: end-to-end.
- Check idea: Trigger an SA lookup with the PFP flag 0 against an SPD entry whose local-port selector is a list of ranges, using a packet with no available source port value, and confirm the packet is discarded.

### RFC4301-PFP-14

**When the PFP flag is 0, the SPD entry's local-port selector is ANY or OPAQUE, and the triggering packet has no available source port value, the resulting local-port selector in the SAD entry keeps the SPD entry's value.**

> "ANY 0 not avail. ANY OPAQUE 0 not avail. OPAQUE" — §4.4.2.2, `rfc4301.txt:2203-2204`

- Strength: description. Class: internal.
- Check idea: Trigger an SA lookup with the PFP flag 0 against an SPD entry whose local-port selector is ANY, and against one whose local-port selector is OPAQUE, using a packet with no available source port value, and confirm the resulting SAD entry's local-port selector keeps ANY or OPAQUE.

### RFC4301-PFP-15

**When the PFP flag is 1, the SPD entry's local-port selector is a list of ranges or ANY, and the triggering packet has a source port value, the resulting local-port selector in the SAD entry narrows to that packet's source port value.**

> "list of ranges 1 src port "s" "s" ANY 1 src port "s" "s"" — §4.4.2.2, `rfc4301.txt:2206-2207`

- Strength: description. Class: internal.
- Check idea: Create an SA from an SPD entry whose local-port selector is a list of ranges, and from one whose local-port selector is ANY, both with the PFP flag 1, triggered by a packet with source port "s", and confirm each resulting SAD entry's local-port selector equals "s".

### RFC4301-PFP-16

**When the PFP flag is 1, the SPD entry's local-port selector is a list of ranges or ANY, and the triggering packet has no available source port value, the packet is discarded.**

> "list of ranges 1 not avail. discard packet ANY 1 not avail. discard packet" — §4.4.2.2, `rfc4301.txt:2210-2211`

- Strength: description. Class: end-to-end.
- Check idea: Trigger an SA lookup with the PFP flag 1 against an SPD entry whose local-port selector is a list of ranges, and against one whose local-port selector is ANY, using a packet with no available source port value, and confirm the packet is discarded in each case.

### RFC4301-PFP-17

**When the PFP flag is 0 and the triggering packet has a destination port value, the resulting remote-port selector in the SAD entry equals the SPD entry's remote-port selector.**

> "rem port list of ranges 0 dst port "d" list of ranges ANY 0 dst port "d" ANY OPAQUE 0 dst port "d" OPAQUE" — §4.4.2.2, `rfc4301.txt:2215-2217`

- Strength: description. Class: internal.
- Check idea: Create an SA from an SPD entry whose remote-port selector is a list of ranges, ANY, or OPAQUE, with the PFP flag 0, triggered by a packet with an available destination port, and confirm each resulting SAD entry's remote-port selector equals the SPD entry's value.

### RFC4301-PFP-18

**When the PFP flag is 0, the SPD entry's remote-port selector is a list of ranges, and the triggering packet has no available destination port value, the packet is discarded.**

> "list of ranges 0 not avail. discard packet" — §4.4.2.2, `rfc4301.txt:2219`

- Strength: description. Class: end-to-end.
- Check idea: Trigger an SA lookup with the PFP flag 0 against an SPD entry whose remote-port selector is a list of ranges, using a packet with no available destination port value, and confirm the packet is discarded.

### RFC4301-PFP-19

**When the PFP flag is 0, the SPD entry's remote-port selector is ANY or OPAQUE, and the triggering packet has no available destination port value, the resulting remote-port selector in the SAD entry keeps the SPD entry's value.**

> "ANY 0 not avail. ANY OPAQUE 0 not avail. OPAQUE" — §4.4.2.2, `rfc4301.txt:2220-2221`

- Strength: description. Class: internal.
- Check idea: Trigger an SA lookup with the PFP flag 0 against an SPD entry whose remote-port selector is ANY, and against one whose remote-port selector is OPAQUE, using a packet with no available destination port value, and confirm the resulting SAD entry's remote-port selector keeps ANY or OPAQUE.

### RFC4301-PFP-20

**When the PFP flag is 1, the SPD entry's remote-port selector is a list of ranges or ANY, and the triggering packet has a destination port value, the resulting remote-port selector in the SAD entry narrows to that packet's destination port value.**

> "list of ranges 1 dst port "d" "d" ANY 1 dst port "d" "d"" — §4.4.2.2, `rfc4301.txt:2223-2224`

- Strength: description. Class: internal.
- Check idea: Create an SA from an SPD entry whose remote-port selector is a list of ranges, and from one whose remote-port selector is ANY, both with the PFP flag 1, triggered by a packet with destination port "d", and confirm each resulting SAD entry's remote-port selector equals "d".

### RFC4301-PFP-21

**When the PFP flag is 1, the SPD entry's remote-port selector is a list of ranges or ANY, and the triggering packet has no available destination port value, the packet is discarded.**

> "list of ranges 1 not avail. discard packet ANY 1 not avail. discard packet" — §4.4.2.2, `rfc4301.txt:2227-2228`

- Strength: description. Class: end-to-end.
- Check idea: Trigger an SA lookup with the PFP flag 1 against an SPD entry whose remote-port selector is a list of ranges, and against one whose remote-port selector is ANY, using a packet with no available destination port value, and confirm the packet is discarded in each case.

### RFC4301-PFP-22

**When the PFP flag is 0 and the triggering packet has a Mobility Header type value, the resulting mh-type selector in the SAD entry equals the SPD entry's mh-type selector.**

> "mh type list of ranges 0 mh type "T" list of ranges ANY 0 mh type "T" ANY OPAQUE 0 mh type "T" OPAQUE" — §4.4.2.2, `rfc4301.txt:2254-2256`

- Strength: description. Class: internal.
- Check idea: Create an SA from an SPD entry whose mh-type selector is a list of ranges, ANY, or OPAQUE, with the PFP flag 0, triggered by a packet with an available Mobility Header type, and confirm each resulting SAD entry's mh-type selector equals the SPD entry's value.

### RFC4301-PFP-23

**When the PFP flag is 0, the SPD entry's mh-type selector is a list of ranges, and the triggering packet has no available Mobility Header type value, the packet is discarded.**

> "list of ranges 0 not avail. discard packet" — §4.4.2.2, `rfc4301.txt:2258`

- Strength: description. Class: end-to-end.
- Check idea: Trigger an SA lookup with the PFP flag 0 against an SPD entry whose mh-type selector is a list of ranges, using a packet with no available Mobility Header type value, and confirm the packet is discarded.

### RFC4301-PFP-24

**When the PFP flag is 0, the SPD entry's mh-type selector is ANY or OPAQUE, and the triggering packet has no available Mobility Header type value, the resulting mh-type selector in the SAD entry keeps the SPD entry's value.**

> "ANY 0 not avail. ANY OPAQUE 0 not avail. OPAQUE" — §4.4.2.2, `rfc4301.txt:2259-2260`

- Strength: description. Class: internal.
- Check idea: Trigger an SA lookup with the PFP flag 0 against an SPD entry whose mh-type selector is ANY, and against one whose mh-type selector is OPAQUE, using a packet with no available Mobility Header type value, and confirm the resulting SAD entry's mh-type selector keeps ANY or OPAQUE.

### RFC4301-PFP-25

**When the PFP flag is 1, the SPD entry's mh-type selector is a list of ranges or ANY, and the triggering packet has a Mobility Header type value, the resulting mh-type selector in the SAD entry narrows to that packet's Mobility Header type value.**

> "list of ranges 1 mh type "T" "T" ANY 1 mh type "T" "T"" — §4.4.2.2, `rfc4301.txt:2262-2263`

- Strength: description. Class: internal.
- Check idea: Create an SA from an SPD entry whose mh-type selector is a list of ranges, and from one whose mh-type selector is ANY, both with the PFP flag 1, triggered by a packet with Mobility Header type "T", and confirm each resulting SAD entry's mh-type selector equals "T".

### RFC4301-PFP-26

**When the PFP flag is 1, the SPD entry's mh-type selector is a list of ranges or ANY, and the triggering packet has no available Mobility Header type value, the packet is discarded.**

> "list of ranges 1 not avail. discard packet ANY 1 not avail. discard packet" — §4.4.2.2, `rfc4301.txt:2266-2267`

- Strength: description. Class: end-to-end.
- Check idea: Trigger an SA lookup with the PFP flag 1 against an SPD entry whose mh-type selector is a list of ranges, and against one whose mh-type selector is ANY, using a packet with no available Mobility Header type value, and confirm the packet is discarded in each case.

### RFC4301-PFP-27

**The ICMP type-and-code selector is a 16-bit value in which the code applies to the particular type it is bound to, and it can hold a single type with a range of codes, a single type with ANY code, or ANY type with ANY code.**

> "there will be a 16-bit selector for ICMP type and ICMP code. Note that the type and code are bound to each other, i.e., the codes apply to the particular type. This 16-bit selector can contain a single type and a range of codes, a single type and ANY code, and ANY type and ANY code." — §4.4.2.2, `rfc4301.txt:2303-2307`

- Strength: description. Class: encoding.
- Check idea: Configure an SPD entry's ICMP selector as a single type with a range of codes, as a single type with ANY code, and as ANY type with ANY code, and confirm the implementation accepts all three forms as one 16-bit selector in which the code is bound to its type.

### RFC4301-PFP-28

**When the PFP flag is 0 and the triggering packet has an ICMP type and code value, the resulting ICMP type-and-code selector in the SAD entry equals the SPD entry's selector.**

> "ICMP type a single type & 0 type "t" & single type & and code range of codes code "c" range of codes a single type & 0 type "t" & single type & ANY code code "c" ANY code ANY type & ANY 0 type "t" & ANY type & code code "c" ANY code OPAQUE 0 type "t" & OPAQUE code "c"" — §4.4.2.2, `rfc4301.txt:2313-2320`

- Strength: description. Class: internal.
- Check idea: Create an SA from an SPD entry whose ICMP selector is a single type with a range of codes, a single type with ANY code, ANY type with ANY code, or OPAQUE, with the PFP flag 0, triggered by a packet with an available ICMP type and code, and confirm each resulting SAD entry's ICMP selector equals the SPD entry's value.

### RFC4301-PFP-29

**When the PFP flag is 0, the SPD entry's ICMP selector names a single type, and the triggering packet has no available ICMP type and code, the packet is discarded.**

> "a single type & 0 not avail. discard packet range of codes a single type & 0 not avail. discard packet ANY code" — §4.4.2.2, `rfc4301.txt:2322-2325`

- Strength: description. Class: end-to-end.
- Check idea: Trigger an SA lookup with the PFP flag 0 against an SPD entry whose ICMP selector names a single type with a range of codes, and against one that names a single type with ANY code, using a packet with no available ICMP type and code, and confirm the packet is discarded in each case.

### RFC4301-PFP-30

**When the PFP flag is 0, the SPD entry's ICMP selector is ANY type with ANY code, or OPAQUE, and the triggering packet has no available ICMP type and code, the resulting ICMP selector in the SAD entry keeps the SPD entry's value.**

> "ANY type & 0 not avail. ANY type & ANY code ANY code OPAQUE 0 not avail. OPAQUE" — §4.4.2.2, `rfc4301.txt:2326-2328`

- Strength: description. Class: internal.
- Check idea: Trigger an SA lookup with the PFP flag 0 against an SPD entry whose ICMP selector is ANY type with ANY code, and against one whose ICMP selector is OPAQUE, using a packet with no available ICMP type and code, and confirm the resulting SAD entry's ICMP selector keeps the SPD entry's value.

### RFC4301-PFP-31

**When the PFP flag is 1, the SPD entry's ICMP selector names a single type or ANY type, and the triggering packet has an ICMP type and code, the resulting ICMP selector in the SAD entry narrows to that packet's type and code.**

> "a single type & 1 type "t" & "t" and "c" range of codes code "c" a single type & 1 type "t" & "t" and "c" ANY code code "c" ANY type & 1 type "t" & "t" and "c" ANY code code "c"" — §4.4.2.2, `rfc4301.txt:2330-2335`

- Strength: description. Class: internal.
- Check idea: Create an SA from an SPD entry whose ICMP selector names a single type with a range of codes, a single type with ANY code, or ANY type with ANY code, with the PFP flag 1, triggered by a packet with ICMP type "t" and code "c", and confirm each resulting SAD entry's ICMP selector equals "t" and "c".

### RFC4301-PFP-32

**When the PFP flag is 1, the SPD entry's ICMP selector names a single type or ANY type, and the triggering packet has no available ICMP type and code, the packet is discarded.**

> "a single type & 1 not avail. discard packet range of codes a single type & 1 not avail. discard packet ANY code ANY type & 1 not avail. discard packet ANY code" — §4.4.2.2, `rfc4301.txt:2339-2344`

- Strength: description. Class: end-to-end.
- Check idea: Trigger an SA lookup with the PFP flag 1 against an SPD entry whose ICMP selector names a single type with a range of codes, a single type with ANY code, or ANY type with ANY code, using a packet with no available ICMP type and code, and confirm the packet is discarded in each case.

### RFC4301-PFP-33

**For the name selector, the SPD entry holds a list of user or system names, and the PFP flag, the triggering packet's value, and the resulting SAD entry do not apply.**

> "Selector SPD Entry PFP Packet Entry" — §4.4.2.2, `rfc4301.txt:2363`, and "name list of user or N/A N/A N/A system names" — §4.4.2.2, `rfc4301.txt:2365-2366`

- Strength: description. Class: internal.
- Check idea: Configure an SPD entry with a name selector holding a list of user or system names, and confirm the resulting SA is not governed by a PFP flag or by a per-packet triggering value for that selector.

## IP traffic processing

### RFC4301-PROC-1

**An IPsec implementation must consult the SPD, or its associated caches, for all traffic that crosses the IPsec protection boundary, including IPsec management traffic.**

> "the SPD (or associated caches) MUST be consulted during the processing of all traffic that crosses the IPsec protection boundary, including IPsec management traffic." — §5, `rfc4301.txt:2793-2795`

- Strength: must. Class: end-to-end.
- Check idea: send ordinary traffic and IPsec management traffic across the protection boundary and check that each is processed according to a matching SPD entry.

### RFC4301-PROC-2

**An IPsec implementation must discard a packet, inbound or outbound, when no SPD policy matches it.**

> "If no policy is found in the SPD that matches a packet (for either inbound or outbound traffic), the packet MUST be discarded." — §5, `rfc4301.txt:2795-2797`

- Strength: must. Class: end-to-end.
- Check idea: send a packet, in each direction, whose selectors match no SPD entry and check that the implementation drops it.

### RFC4301-PROC-3

**A cached SPD entry indicates whether matching traffic is bypassed or discarded.**

> "Each cached entry will indicate that matching traffic should be bypassed or discarded, appropriately." — §5, `rfc4301.txt:2828-2830`

- Strength: should (lower case). Class: internal.
- Check idea: after a cache entry is created from a decorrelated SPD entry, inspect what disposition it records and check that it matches the source SPD entry's action.

## Outbound IP traffic processing

### RFC4301-OUT-1

**On an outbound packet, the implementation invokes the SPD selection function to obtain the SPD-ID that chooses the SPD to use.**

> "When a packet arrives from the subscriber (protected) interface, invoke the SPD selection function to obtain the SPD-ID needed to choose the appropriate SPD." — §5.1, `rfc4301.txt:2922-2924`

- Strength: must. Class: internal.
- Check idea: send an outbound packet from the protected interface and check that an SPD-ID selection step runs and yields the SPD to use for the packet.

### RFC4301-OUT-2

**The implementation matches an outbound packet's headers against the SPD-O/SPD-S cache for the SPD-ID chosen for it.**

> "Match the packet headers against the cache for the SPD specified by the SPD-ID from step 1.  Note that this cache contains entries from SPD-O and SPD-S." — §5.1, `rfc4301.txt:2927-2929`

- Strength: must. Class: internal.
- Check idea: send an outbound packet and check that its header fields are compared against the SPD-O/SPD-S cache entries for the chosen SPD before any other action is taken.

### RFC4301-OUT-3

**On a cache match, the implementation processes the outbound packet as the matching cache entry specifies: BYPASS, DISCARD, or PROTECT using AH or ESP.**

> "If there is a match, then process the packet as specified by the matching cache entry, i.e., BYPASS, DISCARD, or PROTECT using AH or ESP." — §5.1, `rfc4301.txt:2931-2933`

- Strength: must. Class: end-to-end.
- Check idea: prime the SPD-O/SPD-S cache with a BYPASS, a DISCARD, and a PROTECT entry in turn, and check that a matching outbound packet is forwarded unchanged, dropped, or given AH/ESP treatment respectively.

### RFC4301-OUT-4

**When IPsec processing applies to an outbound packet, the SPD cache entry links to the SAD entry that supplies the mode, cryptographic algorithms, keys, SPI, and PMTU used to protect the packet.**

> "If IPsec processing is applied, there is a link from the SPD cache entry to the relevant SAD entry (specifying the mode, cryptographic algorithms, keys, SPI, PMTU, etc.)." — §5.1, `rfc4301.txt:2933-2935`

- Strength: description. Class: internal.
- Check idea: cause a PROTECT match and check that the resulting protected packet carries the SPI, mode, and algorithm output that the linked SAD entry specifies.

### RFC4301-OUT-5

**The SA's PMTU value, the stateful fragment-checking flag, and the packet's DF bit together decide whether an outbound packet is fragmented before or after IPsec processing, or discarded with an ICMP PMTU message sent.**

> "the SA PMTU value, plus the value of the stateful fragment checking flag (and the DF bit in the IP header of the outbound packet) determine whether the packet can (must) be fragmented prior to or after IPsec processing, or if it must be discarded and an ICMP PMTU message is sent." — §5.1, `rfc4301.txt:2938-2942`

- Strength: must (lower case). Class: end-to-end.
- Check idea: send an outbound packet larger than the SA's PMTU under each combination of the DF bit and the fragment-checking flag, and check whether it is fragmented before protection, fragmented after protection, or discarded with an ICMP PMTU message.

### RFC4301-OUT-6

**On a cache miss, the implementation searches the SPD-S and SPD-O parts of the SPD identified by the SPD-ID.**

> "If no match is found in the cache, search the SPD (SPD-S and SPD-O parts) specified by SPD-ID." — §5.1, `rfc4301.txt:2944-2945`

- Strength: must. Class: internal.
- Check idea: send an outbound packet that misses the SPD cache and check that the implementation then searches the SPD-S and SPD-O parts of the SPD before deciding the packet's disposition.

### RFC4301-OUT-7

**When the matching SPD entry calls for BYPASS or DISCARD, the implementation creates new outbound SPD cache entries, and for BYPASS also new inbound SPD cache entries.**

> "If the SPD entry calls for BYPASS or DISCARD, create one or more new outbound SPD cache entries and if BYPASS, create one or more new inbound SPD cache entries." — §5.1, `rfc4301.txt:2945-2948`

- Strength: description. Class: internal.
- Check idea: send an outbound packet that hits a BYPASS SPD entry and one that hits a DISCARD SPD entry, and check that a matching outbound cache entry appears afterward, with an inbound cache entry appearing only in the BYPASS case.

### RFC4301-OUT-8

**When the matching SPD entry calls for PROTECT, the implementation invokes the key management mechanism to create the SA.**

> "If the SPD entry calls for PROTECT, i.e., creation of an SA, the key management mechanism (e.g., IKEv2) is invoked to create the SA." — §5.1, `rfc4301.txt:2950-2952`

- Strength: description. Class: internal.
- Check idea: send an outbound packet that matches a PROTECT SPD entry with no existing SA and check that SA establishment begins.

### RFC4301-OUT-9

**When SA creation for a PROTECT match succeeds, the implementation creates a new outbound SPD-S cache entry together with outbound and inbound SAD entries.**

> "If SA creation succeeds, a new outbound (SPD-S) cache entry is created, along with outbound and inbound SAD entries" — §5.1, `rfc4301.txt:2953-2954`

- Strength: description. Class: internal.
- Check idea: let SA negotiation for a PROTECT match succeed and check that an outbound SPD-S cache entry and both an outbound and an inbound SAD entry now exist.

### RFC4301-OUT-10

**When SA creation for a PROTECT match fails, the implementation discards the packet.**

> "otherwise the packet is discarded." — §5.1, `rfc4301.txt:2954-2955`

- Strength: description. Class: end-to-end.
- Check idea: let SA negotiation for a PROTECT match fail and check that the triggering packet is dropped.

### RFC4301-OUT-11

**A packet that triggers an SPD lookup may be discarded, or may instead be processed against the newly created cache entry if one was created.**

> "A packet that triggers an SPD lookup MAY be discarded by the implementation, or it MAY be processed against the newly created cache entry, if one is created." — §5.1, `rfc4301.txt:2955-2957`

- Strength: may. Class: end-to-end.
- Check idea: send a packet that triggers SPD lookup and SA creation, and check that the implementation either drops that first packet or forwards it protected using the freshly created cache entry.

### RFC4301-OUT-12

**The inbound SAD entry created alongside an outbound SA holds the selector values, derived from the SPD entry and, where PFP flags were set, the packet, used to check inbound traffic on that SA.**

> "Since SAs are created in pairs, an SAD entry for the corresponding inbound SA also is created, and it contains the selector values derived from the SPD entry (and packet, if any PFP flags were "true") used to create the inbound SA, for use in checking inbound traffic delivered via the SA." — §5.1, `rfc4301.txt:2957-2962`

- Strength: description. Class: internal.
- Check idea: trigger SA-pair creation from an SPD entry with some PFP flags true, and check that the inbound SAD entry's selectors combine the fixed SPD values with the corresponding fields of the triggering packet.

### RFC4301-OUT-13

**The implementation passes the outbound packet to the outbound forwarding function, outside the IPsec implementation, to select the exit interface.**

> "The packet is passed to the outbound forwarding function (operating outside of the IPsec implementation), to select the interface to which the packet will be directed." — §5.1, `rfc4301.txt:2964-2966`

- Strength: must. Class: internal.
- Check idea: send an outbound packet through BYPASS or PROTECT processing and check that it reaches ordinary IP forwarding for interface selection afterward.

### RFC4301-OUT-14

**The outbound forwarding function may loop a packet back across the IPsec boundary for further IPsec processing, for example to support nested SAs.**

> "This function" and "may cause the packet to be passed back across the IPsec boundary, for additional IPsec processing, e.g., in support of nested SAs." — §5.1, `rfc4301.txt:2966`, `rfc4301.txt:2975-2976`

- Strength: may (lower case). Class: internal.
- Check idea: configure nested SAs so that forwarding routes a once-protected packet back across the IPsec boundary, and check that it receives a second round of IPsec processing.

### RFC4301-OUT-15

**When the forwarding function loops a packet back for nested-SA processing, the SPD-I database must have an entry permitting inbound bypass of the packet, or the packet is discarded.**

> "If so, there MUST be an entry in SPD-I database that permits inbound bypassing of the packet, otherwise the packet will be discarded." — §5.1, `rfc4301.txt:2977-2979`

- Strength: must. Class: end-to-end.
- Check idea: loop a packet back for nested-SA processing with no permitting SPD-I entry and check that it is discarded; add a permitting entry and check that the loop-back now succeeds.

### RFC4301-OUT-16

**When more than one SPD-I exists, traffic looped back across the IPsec boundary may be tagged as coming from the internal interface.**

> "the traffic being looped back MAY be tagged as coming from this internal interface." — §5.1, `rfc4301.txt:2980-2981`

- Strength: may. Class: internal.
- Check idea: with multiple SPD-Is configured, loop a packet back and check whether it carries a tag identifying the internal interface, distinguishing it from externally arriving traffic.

### RFC4301-OUT-17

**Outside IPv4 and IPv6 transport mode, an SG, BITS, or BITW implementation may fragment a packet before applying IPsec.**

> "With the exception of IPv4 and IPv6 transport mode, an SG, BITS, or BITW implementation MAY fragment packets before applying IPsec." — §5.1, `rfc4301.txt:2984-2986`

- Strength: may. Class: wire.
- Check idea: send an oversized packet to an SG, BITS, or BITW implementation for tunnel-mode protection and check whether it fragments the packet before applying AH or ESP.

### RFC4301-OUT-18

**The device should have a configuration setting to disable fragmentation before IPsec processing.**

> "The device SHOULD have a configuration setting to disable this." — §5.1, `rfc4301.txt:2987-2988`

- Strength: should. Class: internal.
- Check idea: look for a configuration setting that turns off fragmentation before IPsec processing and check that enabling it stops such fragmentation.

### RFC4301-OUT-19

**Fragments produced before IPsec processing are evaluated against the SPD the same way as any other packet.**

> "The resulting fragments are evaluated against the SPD in the normal manner." — §5.1, `rfc4301.txt:2988-2989`

- Strength: description. Class: internal.
- Check idea: pre-fragment an outbound packet and check that each fragment goes through ordinary SPD, or cache, lookup on its own.

### RFC4301-OUT-20

**A fragment carrying no port numbers, ICMP type and code, or Mobility Header type matches only an SPD rule whose corresponding selector is OPAQUE or ANY.**

> "fragments not containing port numbers (or ICMP message type and code, or Mobility Header type) will only match rules having port (or ICMP message type and code, or MH type) selectors of OPAQUE or ANY." — §5.1, `rfc4301.txt:2989-2992`

- Strength: description. Class: end-to-end.
- Check idea: send a non-initial fragment carrying no port, ICMP type/code, or Mobility Header field, and check that it matches only SPD entries whose relevant selector is OPAQUE or ANY.

### RFC4301-OUT-21

**The IPsec system must determine and enforce an SA's PMTU following the steps of Section 8.2.**

> "With regard to determining and enforcing the PMTU of an SA, the IPsec system MUST follow the steps described in Section 8.2." — §5.1, `rfc4301.txt:2995-2996`

- Strength: must. Class: internal.
- Check idea: exercise a scenario needing PMTU determination and enforcement and check that the implementation's behavior matches the Section 8.2 procedure.

## An outbound packet that must be discarded

### RFC4301-DISC-1

**When an IPsec system must discard an outbound packet, it should be capable of generating and sending an ICMP message telling the packet's sender that it was discarded.**

> "If an IPsec system receives an outbound packet that it finds it must discard, it SHOULD be capable of generating and sending an ICMP message to indicate to the sender of the outbound packet that the packet was discarded." — §5.1.1, `rfc4301.txt:3000-3003`

- Strength: should. Class: error-signal.
- Check idea: cause an outbound packet to be discarded and check whether an ICMP message reporting the discard is sent back toward its sender.

### RFC4301-DISC-2

**The reason for discarding an outbound packet should be recorded in the audit log.**

> "The reason SHOULD be recorded in the audit log." — §5.1.1, `rfc4301.txt:3005`

- Strength: should. Class: internal.
- Check idea: cause an outbound packet to be discarded and check that the audit log records the reason for the discard.

### RFC4301-DISC-3

**The audit log entry for a discarded outbound packet should include the reason, the current date and time, and the packet's selector values.**

> "The audit log entry for this event SHOULD include the reason, current date/time, and the selector values from the packet." — §5.1.1, `rfc4301.txt:3005-3007`

- Strength: should. Class: internal.
- Check idea: cause an outbound packet to be discarded and check that the resulting audit log entry contains the reason, a timestamp, and the packet's selector values.

### RFC4301-DISC-4

**When an outbound packet is discarded because its selectors matched an SPD entry requiring discard, an IPv4 system reports ICMP type 3, code 13.**

> "The selectors of the packet matched an SPD entry requiring the packet to be discarded." and "IPv4 Type = 3 (destination unreachable) Code = 13" and "(Communication Administratively Prohibited)" — §5.1.1, `rfc4301.txt:3009-3010`, `rfc4301.txt:3012`, `rfc4301.txt:3013`

- Strength: description. Class: error-signal.
- Check idea: send an IPv4 packet matching a DISCARD SPD entry and check that the ICMP message sent back is type 3, code 13.

### RFC4301-DISC-5

**When an outbound packet is discarded because its selectors matched an SPD entry requiring discard, an IPv6 system reports ICMP type 1, code 1.**

> "The selectors of the packet matched an SPD entry requiring the packet to be discarded." and "IPv6 Type = 1 (destination unreachable) Code = 1" and "(Communication with destination administratively" and "prohibited)" — §5.1.1, `rfc4301.txt:3009-3010`, `rfc4301.txt:3015`, `rfc4301.txt:3016`, `rfc4301.txt:3017`

- Strength: description. Class: error-signal.
- Check idea: send an IPv6 packet matching a DISCARD SPD entry and check that the ICMPv6 message sent back is type 1, code 1.

### RFC4301-DISC-6

**When an outbound packet is discarded because the IPsec system reached the remote peer but could not negotiate the required SA, an IPv4 system reports ICMP type 3, code 13.**

> "The IPsec system successfully reached the remote peer but was unable to negotiate the SA required by the SPD entry matching the packet because, for example, the remote peer is administratively prohibited from communicating with the initiator, the initiating" and "peer was unable to authenticate itself to the remote peer, the remote peer was unable to authenticate itself to the initiating peer, or the SPD at the remote peer did not have a suitable entry." and "IPv4 Type = 3 (destination unreachable) Code = 13" — §5.1.1, `rfc4301.txt:3019-3022`, `rfc4301.txt:3031-3034`, `rfc4301.txt:3036`

- Strength: description. Class: error-signal.
- Check idea: make SA negotiation with a reachable IPv4 peer fail for an administrative or authentication reason and check that the ICMP message sent back is type 3, code 13.

### RFC4301-DISC-7

**When an outbound packet is discarded because the IPsec system reached the remote peer but could not negotiate the required SA, an IPv6 system reports ICMP type 1, code 1.**

> "The IPsec system successfully reached the remote peer but was unable to negotiate the SA required by the SPD entry matching the packet because, for example, the remote peer is administratively prohibited from communicating with the initiator, the initiating" and "peer was unable to authenticate itself to the remote peer, the remote peer was unable to authenticate itself to the initiating peer, or the SPD at the remote peer did not have a suitable entry." and "IPv6 Type = 1 (destination unreachable) Code = 1" — §5.1.1, `rfc4301.txt:3019-3022`, `rfc4301.txt:3031-3034`, `rfc4301.txt:3039`

- Strength: description. Class: error-signal.
- Check idea: make SA negotiation with a reachable IPv6 peer fail for an administrative or authentication reason and check that the ICMPv6 message sent back is type 1, code 1.

### RFC4301-DISC-8

**When an outbound packet is discarded because the IPsec peer could not be contacted, an IPv4 system reports ICMP type 3, code 1.**

> "The IPsec system was unable to set up the SA required by the SPD entry matching the packet because the IPsec peer at the other end of the exchange could not be contacted." and "IPv4 Type = 3 (destination unreachable) Code = 1 (host" and "unreachable)" — §5.1.1, `rfc4301.txt:3043-3045`, `rfc4301.txt:3047`, `rfc4301.txt:3048`

- Strength: description. Class: error-signal.
- Check idea: make SA negotiation with an unreachable IPv4 peer fail and check that the ICMP message sent back is type 3, code 1.

### RFC4301-DISC-9

**When an outbound packet is discarded because the IPsec peer could not be contacted, an IPv6 system reports ICMP type 1, code 3.**

> "The IPsec system was unable to set up the SA required by the SPD entry matching the packet because the IPsec peer at the other end of the exchange could not be contacted." and "IPv6 Type = 1 (destination unreachable) Code = 3 (address" and "unreachable)" — §5.1.1, `rfc4301.txt:3043-3045`, `rfc4301.txt:3050`, `rfc4301.txt:3051`

- Strength: description. Class: error-signal.
- Check idea: make SA negotiation with an unreachable IPv6 peer fail and check that the ICMPv6 message sent back is type 1, code 3.

### RFC4301-DISC-10

**A security gateway should have a management control letting an administrator configure whether it sends these ICMP messages reporting discarded outbound packets.**

> "a security gateway SHOULD include a management control to allow an administrator to configure an IPsec implementation to send or not send the ICMP messages under these circumstances" — §5.1.1, `rfc4301.txt:3057-3060`

- Strength: should. Class: internal.
- Check idea: look for a security-gateway configuration control that enables or disables sending discard-reporting ICMP messages, and check that turning it off stops those ICMP messages.

### RFC4301-DISC-11

**When a security gateway is configured to send these ICMP messages, it should rate-limit their transmission.**

> "if this facility is selected, to rate limit the transmission of such ICMP responses." — §5.1.1, `rfc4301.txt:3060-3061`

- Strength: should. Class: wire.
- Check idea: trigger many discarded outbound packets on a security gateway configured to send ICMP reports and check that the rate of ICMP messages sent is limited rather than one-for-one.

## Header construction for tunnel mode

### RFC4301-TUN-1

**In tunnel mode, the outer IP header's source and destination addresses identify the tunnel endpoints, while the inner header's source and destination addresses identify the datagram's original sender and recipient.**

> "The outer IP header Source Address and Destination Address identify the "endpoints" of the tunnel (the encapsulator and decapsulator)." and "The inner IP header Source Address and Destination Addresses identify the original sender and recipient of the datagram (from the perspective of this tunnel), respectively." — §5.1.2, `rfc4301.txt:3073-3074`, `rfc4301.txt:3075-3077`

- Strength: description. Class: wire.
- Check idea: capture a tunnel-mode packet and check that the outer header addresses are the tunnel endpoints while the inner header addresses are the original sender and recipient.

### RFC4301-TUN-2

**In tunnel mode, the inner IP header stays unchanged during delivery to the tunnel exit point, except for the fields noted for TTL, or Hop Limit, and the DS/ECN fields.**

> "The inner IP header is not changed except as noted below for TTL (or Hop Limit) and the DS/ECN Fields.  The inner IP header otherwise remains unchanged during its delivery to the tunnel exit point." — §5.1.2, `rfc4301.txt:3090-3093`

- Strength: description. Class: wire.
- Check idea: compare the inner header of a tunnel-mode packet as sent and as it arrives at the tunnel exit point, and check that only the TTL/Hop Limit and DS/ECN fields can differ.

### RFC4301-TUN-3

**In tunnel mode, no IP options or extension headers in the inner header change during delivery through the tunnel.**

> "No change to IP options or extension headers in the inner header occurs during delivery of the encapsulated datagram through the tunnel." — §5.1.2, `rfc4301.txt:3095-3097`

- Strength: description. Class: wire.
- Check idea: send a tunnel-mode packet whose inner header carries IP options or extension headers and check that they are identical at the tunnel exit point.

### RFC4301-TUN-4

**An IPsec implementation may be configurable in how it processes the outer DS field of a transmitted tunnel-mode packet.**

> "An IPsec implementation MAY be configurable with regard to how it processes the outer DS field for tunnel mode for transmitted packets." — §5.1.2, `rfc4301.txt:3106-3108`

- Strength: may. Class: internal.
- Check idea: look for a configuration setting controlling how the outer DS field is set on transmitted tunnel-mode packets.

### RFC4301-TUN-5

**The outer DS field of a transmitted tunnel-mode packet may instead be mapped to a fixed value, and that mapping may be set on a per-SA basis.**

> "Another will allow the outer DS field to be mapped to a fixed value, which MAY be configured on a per-SA basis." — §5.1.2, `rfc4301.txt:3111-3113`

- Strength: may. Class: wire.
- Check idea: configure a fixed outer DS value for one SA and check that outbound tunnel-mode packets on that SA carry that fixed value regardless of the inner DS field.

### RFC4301-TUN-6

**By default, an implementation does not let the outer DS value propagate from an unprotected domain into a protected domain.**

> "By default, DS propagation from an unprotected domain to a protected" and "domain is not permitted." — §5.1.2, `rfc4301.txt:3125-3126`, `rfc4301.txt:3127`

- Strength: description. Class: wire.
- Check idea: send a tunnel-mode packet whose outer DS value differs from the inner DS value and check, with default configuration, that the inner DS value is unaffected at the decapsulator.

### RFC4301-TUN-7

**An IPsec implementation may be configurable in how it processes the outer DS field of a received tunnel-mode packet.**

> "an IPsec implementation MAY be configurable in terms of how it processes the outer DS field for tunnel mode for received packets." — §5.1.2, `rfc4301.txt:3130-3132`

- Strength: may. Class: internal.
- Check idea: look for a configuration setting controlling how the decapsulator treats the outer DS field of a received tunnel-mode packet.

### RFC4301-TUN-8

**A received tunnel-mode packet's outer DS field may be configured to be discarded, the default, or to overwrite the inner DS field.**

> "It may" and "be configured to either discard the outer DS value (the default)" and "OR to overwrite the inner DS field with the outer DS field." — §5.1.2, `rfc4301.txt:3132`, `rfc4301.txt:3133`, `rfc4301.txt:3134`

- Strength: may (lower case). Class: wire.
- Check idea: receive a tunnel-mode packet whose outer and inner DS values differ, under each configuration setting, and check whether the decapsulated inner header keeps its own DS value or takes the outer one.

### RFC4301-TUN-9

**Where discard-versus-overwrite of the outer DS field is offered, it may be configured on a per-SA basis.**

> "If" and "offered, the discard vs. overwrite behavior MAY be configured on a" and "per-SA basis." — §5.1.2, `rfc4301.txt:3134`, `rfc4301.txt:3143`, `rfc4301.txt:3144`

- Strength: may. Class: internal.
- Check idea: configure the discard-versus-overwrite DS behavior differently on two inbound SAs and check that each SA's decapsulated packets follow its own setting.

### RFC4301-TUN-10

**In tunnel mode, the encapsulating header's IP version is allowed to differ from the inner header's IP version.**

> "IPsec allows the IP version of the encapsulating header to be different from that of the inner header." — §5.1.2, `rfc4301.txt:3152-3153`

- Strength: description. Class: wire.
- Check idea: build a tunnel with an outer header of one IP version around an inner packet of the other IP version and check that the tunnel carries it.

### RFC4301-TUN-11

**In tunnel mode, the encapsulator sets the outer header's IP version, 4 for IPv4 or 6 for IPv6, which may differ from the inner header's version, and the decapsulator leaves the inner header's version unchanged.**

> "version          4 (1)                        no change" and "version          6 (1)                        no change" and "The IP version in the encapsulating header can be different" and "from the value in the inner header." — §5.1.2.1, §5.1.2.2, `rfc4301.txt:3166`, `rfc4301.txt:3261`, `rfc4301.txt:3183`, `rfc4301.txt:3184`

- Strength: description. Class: wire.
- Check idea: capture the outer and inner headers of a tunnel-mode packet and check that the outer version is 4 or 6 as appropriate and the inner version is unchanged from what was sent.

### RFC4301-TUN-12

**In tunnel mode, the encapsulator copies the outer header's DS field from the inner header.**

> "DS Field         copied from inner hdr (5)    no change" — §5.1.2.1, `rfc4301.txt:3168`

- Strength: description. Class: wire.
- Check idea: send a tunnel-mode packet with a known inner DS value and check that the outer header carries the same DS value.

### RFC4301-TUN-13

**When a tunnel-mode packet is about to enter a domain for which the outer DSCP value is not appropriate, that value must be mapped to a value appropriate for the domain.**

> "If the packet will immediately enter a domain for which the" and "DSCP value in the outer header is not appropriate, that value" and "MUST be mapped to an appropriate value for the domain" — §5.1.2.1, `rfc4301.txt:3227`, `rfc4301.txt:3228`, `rfc4301.txt:3229`

- Strength: must. Class: wire.
- Check idea: send a tunnel-mode packet toward a domain boundary with a DSCP value not valid in the next domain, and check that the outer DSCP is remapped before it enters that domain.

### RFC4301-TUN-14

**In tunnel mode, the encapsulator copies the outer header's ECN field from the inner header, and the decapsulator constructs the inner header's ECN field from the outer and inner values.**

> "ECN Field        copied from inner hdr        constructed (6)" — §5.1.2.1, §5.1.2.2, `rfc4301.txt:3169`, `rfc4301.txt:3263`

- Strength: description. Class: wire.
- Check idea: send a tunnel-mode packet and check that the outer ECN field equals the inner ECN field as sent, and that the decapsulated inner ECN field follows the CE-propagation rule.

### RFC4301-TUN-15

**The decapsulator sets the inner header's ECN field to Congestion Experienced when the inner field was ECT(0) or ECT(1) and the outer field is Congestion Experienced; otherwise it leaves the inner ECN field unchanged.**

> "If the ECN field in the inner header is set to ECT(0) or" and "ECT(1), where ECT is ECN-Capable Transport (ECT), and if the" and "ECN field in the outer header is set to Congestion Experienced" and "(CE), then set the ECN field in the inner header to CE;" and "otherwise, make no change to the ECN field in the inner" and "header." — §5.1.2.1, `rfc4301.txt:3233`, `rfc4301.txt:3234`, `rfc4301.txt:3235`, `rfc4301.txt:3236`, `rfc4301.txt:3237`, `rfc4301.txt:3238`

- Strength: description. Class: wire.
- Check idea: decapsulate a tunnel packet with inner ECN ECT(0) or ECT(1) and outer ECN CE, and check that the delivered inner ECN becomes CE; repeat with a non-CE outer ECN and check that the inner ECN is unchanged.

### RFC4301-TUN-16

**The encapsulator decrements the inner header's TTL, or Hop Limit, before forwarding the packet into the tunnel.**

> "TTL              constructed (2)              decrement (2)" and "hop limit        constructed (2)              decrement (2)" and "The TTL in the inner header is decremented by the encapsulator" and "prior to forwarding" — §5.1.2.1, §5.1.2.2, `rfc4301.txt:3174`, `rfc4301.txt:3267`, `rfc4301.txt:3186`, `rfc4301.txt:3187`

- Strength: description. Class: wire.
- Check idea: send a packet into the tunnel and check that the encapsulated inner header's TTL or Hop Limit is one less than the original value.

### RFC4301-TUN-17

**The decapsulator decrements the inner header's TTL, or Hop Limit, only if it forwards the packet after removing the tunnel header.**

> "and by the decapsulator if it forwards the" and "packet." — §5.1.2.1, `rfc4301.txt:3187`, `rfc4301.txt:3188`

- Strength: description. Class: wire.
- Check idea: decapsulate a tunnel packet that the decapsulator then forwards and check that the inner TTL or Hop Limit is decremented; decapsulate one delivered locally and check that it is not.

### RFC4301-TUN-18

**In IPv4 tunnel mode, the encapsulator constructs the outer header checksum, and the decapsulator reconstructs the inner header checksum whenever the TTL or ECN field changes.**

> "checksum         constructed                  constructed (2)(6)" — §5.1.2.1, `rfc4301.txt:3176`

- Strength: description. Class: wire.
- Check idea: capture the outer header checksum of an encapsulated IPv4 tunnel packet and check that it is valid for the outer header; check that the inner header checksum at the decapsulator is valid after its TTL or ECN field changes.

### RFC4301-TUN-19

**In tunnel mode, the encapsulator constructs the outer header's source and destination addresses from the SA: the SA's remote address in turn determines the local address, or interface, used to forward the packet.**

> "src address      constructed (3)              no change" and "dest address     constructed (3)              no change" and "src address      constructed (3)              no change" and "dest address     constructed (3)              no change" and "Local and Remote addresses depend on the SA, which is used to" and "determine the Remote address, which in turn determines which" and "Local address (net interface) is used to forward the packet." — §5.1.2.1, §5.1.2.2, `rfc4301.txt:3177`, `rfc4301.txt:3178`, `rfc4301.txt:3268`, `rfc4301.txt:3269`, `rfc4301.txt:3210`, `rfc4301.txt:3211`, `rfc4301.txt:3212`

- Strength: description. Class: wire.
- Check idea: capture the outer header of a tunnel-mode packet and check that its source and destination addresses are the SA's local and remote tunnel endpoints, with the local address matching the interface used to forward the packet.

### RFC4301-TUN-20

**For multicast tunnel-mode traffic, the encapsulator keeps the outer source address consistent over the life of the SA, matching the address negotiated when the SA was established, except that a mobile IPsec implementation updates its source address as it moves.**

> "it is important to ensure consistency" and "over the lifetime of the SA by ensuring that the source" and "address that appears in the encapsulating tunnel header is the" and "same as the one that was negotiated during the SA" and "establishment process.  There is an exception to this general" and "rule, i.e., a mobile IPsec implementation will update its" and "source address as it moves." — §5.1.2.1, `rfc4301.txt:3216`, `rfc4301.txt:3217`, `rfc4301.txt:3218`, `rfc4301.txt:3219`, `rfc4301.txt:3220`, `rfc4301.txt:3221`, `rfc4301.txt:3222`

- Strength: description. Class: wire.
- Check idea: send several multicast tunnel-mode packets on the same SA and check that the outer source address stays the one negotiated for the SA, except on a mobile implementation that has since moved.

### RFC4301-TUN-21

**In IPv4 tunnel mode, the encapsulator constructs the outer header's header-length field independently of the inner header.**

> "header length    constructed                  no change" — §5.1.2.1, `rfc4301.txt:3167`

- Strength: description. Class: wire.
- Check idea: capture the outer header of an IPv4 tunnel-mode packet and check that its header-length field matches the outer header's own length, not the inner header's.

### RFC4301-TUN-22

**In IPv4 tunnel mode, the encapsulator constructs the outer header's total-length field independently of the inner header.**

> "total length     constructed                  no change" — §5.1.2.1, `rfc4301.txt:3170`

- Strength: description. Class: wire.
- Check idea: capture the outer header of an IPv4 tunnel-mode packet and check that its total-length field reflects the outer packet's own length.

### RFC4301-TUN-23

**In IPv4 tunnel mode, the encapsulator constructs the outer header's identification field independently of the inner header.**

> "ID               constructed                  no change" — §5.1.2.1, `rfc4301.txt:3171`

- Strength: description. Class: wire.
- Check idea: capture the outer header of an IPv4 tunnel-mode packet and check that its identification field is set by the encapsulator, not copied from the inner header.

### RFC4301-TUN-24

**In IPv4 tunnel mode, the encapsulator constructs the outer header's flags field, and configuration decides whether the DF bit is copied from the inner header, cleared, or set.**

> "flags (DF,MF)    constructed, DF (4)          no change" and "Configuration determines whether to copy from the inner header" and "(IPv4 only), clear, or set the DF." — §5.1.2.1, `rfc4301.txt:3172`, `rfc4301.txt:3224`, `rfc4301.txt:3225`

- Strength: description. Class: wire.
- Check idea: under each DF configuration setting, send an IPv4 tunnel-mode packet and check whether the outer DF bit matches the inner DF bit, is clear, or is set.

### RFC4301-TUN-25

**In IPv4 tunnel mode, the encapsulator constructs the outer header's fragment-offset field independently of the inner header.**

> "fragment offset  constructed                  no change" — §5.1.2.1, `rfc4301.txt:3173`

- Strength: description. Class: wire.
- Check idea: capture the outer header of an IPv4 tunnel-mode packet and check that its fragment offset reflects the outer packet, not the inner header.

### RFC4301-TUN-26

**In IPv4 tunnel mode, the encapsulator sets the outer header's protocol field to AH or ESP.**

> "protocol         AH, ESP                      no change" — §5.1.2.1, `rfc4301.txt:3175`

- Strength: description. Class: wire.
- Check idea: capture the outer header of an IPv4 tunnel-mode packet and check that its protocol field is AH or ESP, matching the applied IPsec protocol.

### RFC4301-TUN-27

**In IPv4 tunnel mode, the encapsulator never copies IP options from the inner header into the outer header and does not itself construct outer-header options, though code outside IPsec processing may insert or construct them.**

> "Options            never copied                 no change" and "IPsec does not copy the options from the inner header into the" and "outer header, nor does IPsec construct the options in the outer" and "header.  However, post-IPsec code MAY insert/construct options for" and "the outer header." — §5.1.2.1, `rfc4301.txt:3179`, `rfc4301.txt:3240`, `rfc4301.txt:3241`, `rfc4301.txt:3242`, `rfc4301.txt:3243`

- Strength: description. Class: wire.
- Check idea: send an IPv4 tunnel-mode packet whose inner header carries IP options and check that the outer header carries no options copied from it.

### RFC4301-TUN-28

**In IPv6 tunnel mode, the encapsulator sets the outer header's flow label by copying it from the inner header or from configuration.**

> "flow label       copied or configured (8)     no change" — §5.1.2.2, `rfc4301.txt:3264`

- Strength: description. Class: wire.
- Check idea: capture the outer header of an IPv6 tunnel-mode packet and check that its flow label equals either the inner header's flow label or a configured value.

### RFC4301-TUN-29

**Copying the flow label from the inner header to the outer header is acceptable only for end systems, not for security gateways, because a security gateway that did so risks flow-label collisions.**

> "Copying is acceptable only for end systems," and "not SGs.  If an SG copied flow labels from the inner header to" and "the outer header, collisions might result." — §5.1.2.2, `rfc4301.txt:3281`, `rfc4301.txt:3282`, `rfc4301.txt:3283`

- Strength: description. Class: wire.
- Check idea: check that a security gateway's IPv6 tunnel-mode implementation does not offer inner-to-outer flow-label copying, unlike an end-system implementation.

### RFC4301-TUN-30

**In IPv6 tunnel mode, the encapsulator constructs the outer header's payload-length field independently of the inner header.**

> "payload length   constructed                  no change" — §5.1.2.2, `rfc4301.txt:3265`

- Strength: description. Class: wire.
- Check idea: capture the outer header of an IPv6 tunnel-mode packet and check that its payload length reflects the outer packet's own payload.

### RFC4301-TUN-31

**In IPv6 tunnel mode, the encapsulator sets the outer header's next-header field to AH, ESP, or a routing header.**

> "next header      AH,ESP,routing hdr           no change" — §5.1.2.2, `rfc4301.txt:3266`

- Strength: description. Class: wire.
- Check idea: capture the outer header of an IPv6 tunnel-mode packet and check that its next-header field is AH, ESP, or a routing header.

### RFC4301-TUN-32

**In IPv6 tunnel mode, the encapsulator never copies extension headers from the inner header into the outer header and does not itself construct outer extension headers, though code outside IPsec processing may insert or construct them.**

> "Extension headers  never copied (7)             no change" and "IPsec does not copy the extension headers from the inner" and "packet into outer headers, nor does IPsec construct extension" and "headers in the outer header.  However, post-IPsec code MAY" and "insert/construct extension headers for the outer header." — §5.1.2.2, `rfc4301.txt:3270`, `rfc4301.txt:3276`, `rfc4301.txt:3277`, `rfc4301.txt:3278`, `rfc4301.txt:3279`

- Strength: description. Class: wire.
- Check idea: send an IPv6 tunnel-mode packet whose inner header carries an extension header and check that the outer header carries no extension header copied from it.

### RFC4301-TUN-33

**An IPv6 tunnel-mode implementation may offer a per-SA facility to pass the outer header's DS value into the inner header of a received packet, but by default it does not.**

> "An implementation MAY choose to provide a facility to pass the" and "DS value from the outer header to the inner header, on a per-" and "SA basis, for received tunnel mode packets." and "Hence the default behavior for IPsec" and "implementations is NOT to permit such copying." — §5.1.2.2, `rfc4301.txt:3285`, `rfc4301.txt:3286`, `rfc4301.txt:3287`, `rfc4301.txt:3295`, `rfc4301.txt:3296`

- Strength: may. Class: wire.
- Check idea: receive an IPv6 tunnel-mode packet whose outer and inner DS values differ, and check that by default the inner DS value is not overwritten unless a per-SA facility has been explicitly configured to do so.

## Inbound IP traffic processing

### RFC4301-IN-1

**The inbound SPD cache, SPD-I, is applied only to traffic that is bypassed or discarded, not to IPsec-protected traffic.**

> "The inbound SPD cache (SPD-I) is applied only to bypassed or" and "discarded traffic." — §5.2, `rfc4301.txt:3302`, `rfc4301.txt:3311`

- Strength: description. Class: internal.
- Check idea: send an inbound IPsec-protected packet and an inbound plain packet, and check that only the plain packet's disposition is decided through the SPD-I cache.

### RFC4301-IN-2

**A packet that fails to match any entry of an SPD cache is then referred to the corresponding SPD.**

> "The intent for any SPD cache is that a packet that fails to match any entry is then referred to the corresponding SPD." — §5.2, `rfc4301.txt:3313-3315`

- Strength: description. Class: internal.
- Check idea: send a packet that misses every entry of an SPD cache and check that it is then looked up in the corresponding SPD.

### RFC4301-IN-3

**Every SPD should have a nominal, final entry that catches and discards anything otherwise unmatched.**

> "Every SPD SHOULD have a nominal, final entry that catches anything that is otherwise unmatched, and discards it." — §5.2, `rfc4301.txt:3315-3316`

- Strength: should. Class: internal.
- Check idea: look for a final, catch-all SPD entry and check that a packet matching no other SPD entry is discarded by it.

### RFC4301-IN-4

**Non-IPsec-protected traffic that arrives and matches no SPD-I entry is discarded.**

> "This ensures that non-IPsec-protected traffic that arrives and does not match any SPD-I entry will be discarded." — §5.2, `rfc4301.txt:3316-3318`

- Strength: description. Class: end-to-end.
- Check idea: send inbound, non-IPsec-protected traffic that matches no SPD-I entry and check that it is discarded.

### RFC4301-IN-5

**Any IP fragments arriving via the unprotected interface are reassembled before AH or ESP processing.**

> "Prior to performing AH or ESP processing, any IP fragments that arrive via the unprotected interface are reassembled (by IP)." — §5.2, `rfc4301.txt:3380-3382`

- Strength: description. Class: internal.
- Check idea: send fragmented inbound traffic via the unprotected interface and check that it is reassembled before AH or ESP processing is applied.

### RFC4301-IN-6

**An inbound IP datagram needing IPsec processing is identified by the appearance of the AH or ESP value in the IP Next Protocol field, or as a next-layer protocol in the IPv6 context.**

> "Each inbound IP datagram to which IPsec processing will be applied is identified by the appearance of the AH or ESP values in the IP Next Protocol field (or of AH or ESP as a next layer protocol in the IPv6 context)." — §5.2, `rfc4301.txt:3381-3385`

- Strength: description. Class: wire.
- Check idea: send an inbound datagram whose Next Protocol field, or IPv6 next-layer protocol, is AH or ESP, and check that it is routed to IPsec processing instead of ordinary delivery.

### RFC4301-IN-7

**On arrival, a packet may be tagged with the ID of the interface it arrived on, when needed to support multiple SPDs and their SPD-I caches, and that interface ID maps to an SPD-ID.**

> "When a packet arrives, it may be tagged with the ID of the" and "interface (physical or virtual) via which it arrived, if" and "necessary, to support multiple SPDs and associated SPD-I caches." and "(The interface ID is mapped to a corresponding SPD-ID.)" — §5.2, `rfc4301.txt:3389`, `rfc4301.txt:3390`, `rfc4301.txt:3391`, `rfc4301.txt:3392`

- Strength: may (lower case). Class: internal.
- Check idea: with multiple SPDs configured, receive a packet on a given interface and check that it is tagged with that interface's ID, which maps to the corresponding SPD-ID.

### RFC4301-IN-8

**If an inbound packet appears IPsec protected and is addressed to the device, the implementation attempts to map it to an active SA via the SAD.**

> "If the packet appears to be IPsec protected and it is addressed" and "to this device, an attempt is made to map it to an active SA" and "via the SAD." — §5.2, `rfc4301.txt:3395`, `rfc4301.txt:3396`, `rfc4301.txt:3397`

- Strength: description. Class: internal.
- Check idea: send an inbound packet that appears IPsec protected and addressed to the device, and check that the implementation attempts to map it to an active SA in the SAD.

### RFC4301-IN-9

**A device may have multiple IP addresses that may be used in the SAD lookup, for example for protocols such as SCTP.**

> "Note that the device may have multiple IP" and "addresses that may be used in the SAD lookup, e.g., in the case" and "of protocols such as SCTP." — §5.2, `rfc4301.txt:3397`, `rfc4301.txt:3398`, `rfc4301.txt:3399`

- Strength: may (lower case). Class: internal.
- Check idea: send an inbound IPsec-protected packet addressed to one of a device's several IP addresses and check that the SAD lookup can use that address.

### RFC4301-IN-10

**Traffic not addressed to the device, or addressed to it but not AH or ESP, is directed to SPD-I lookup.**

> "Traffic not addressed to this device, or addressed to this" and "device and not AH or ESP, is directed to SPD-I lookup." — §5.2, `rfc4301.txt:3400`, `rfc4301.txt:3401`

- Strength: description. Class: internal.
- Check idea: send a packet not addressed to the device, and a packet addressed to it but carrying neither AH nor ESP, and check that both are routed to SPD-I lookup.

### RFC4301-IN-11

**IKE traffic must have an explicit BYPASS entry in the SPD.**

> "(This" and "implies that IKE traffic MUST have an explicit BYPASS entry in" and "the SPD.)" — §5.2, `rfc4301.txt:3401`, `rfc4301.txt:3402`, `rfc4301.txt:3403`

- Strength: must. Class: internal.
- Check idea: check the SPD for an explicit BYPASS entry covering IKE traffic, and check that inbound IKE traffic is bypassed rather than discarded or matched against a different entry.

### RFC4301-IN-12

**When multiple SPDs are employed, the interface tag assigned on arrival selects the appropriate SPD-I, and its cache, to search.**

> "If multiple SPDs are employed, the tag assigned to" and "the packet in step 1 is used to select the appropriate SPD-I" and "(and cache) to search." — §5.2, `rfc4301.txt:3403`, `rfc4301.txt:3404`, `rfc4301.txt:3405`

- Strength: description. Class: internal.
- Check idea: with multiple SPDs configured, receive packets tagged for different interfaces and check that each is searched against the SPD-I selected by its own tag.

### RFC4301-IN-13

**SPD-I lookup determines whether the action for the packet is DISCARD or BYPASS.**

> "SPD-I lookup determines whether the" and "action is DISCARD or BYPASS." — §5.2, `rfc4301.txt:3405`, `rfc4301.txt:3406`

- Strength: description. Class: end-to-end.
- Check idea: send inbound traffic matching an SPD-I entry and check that the packet is either discarded or bypassed according to that entry's action.

### RFC4301-IN-14

**A packet addressed to the IPsec device with AH or ESP as its protocol is looked up in the SAD.**

> "If the packet is addressed to the IPsec device and AH or ESP is" and "specified as the protocol, the packet is looked up in the SAD." — §5.2, `rfc4301.txt:3408`, `rfc4301.txt:3409`

- Strength: description. Class: internal.
- Check idea: send an inbound packet addressed to the IPsec device with AH or ESP as its protocol, and check that it is looked up in the SAD.

### RFC4301-IN-15

**For unicast traffic, the SAD lookup uses only the SPI, or the SPI plus protocol.**

> "For unicast traffic, use only the SPI (or SPI plus protocol)." — §5.2, `rfc4301.txt:3410`

- Strength: description. Class: internal.
- Check idea: send an inbound unicast AH or ESP packet and check that its SAD lookup key is the SPI alone, or the SPI plus protocol.

### RFC4301-IN-16

**For multicast traffic, the SAD lookup uses the SPI plus the destination address, or the SPI plus the destination and source addresses, as Section 4.1 specifies.**

> "For multicast traffic, use the SPI plus the destination or SPI" and "plus destination and source addresses, as specified in Section" and "4.1." — §5.2, `rfc4301.txt:3411`, `rfc4301.txt:3412`, `rfc4301.txt:3413`

- Strength: description. Class: internal.
- Check idea: send an inbound multicast AH or ESP packet and check that its SAD lookup key is the SPI plus destination, or the SPI plus destination and source, as specified for the SA.

### RFC4301-IN-17

**When a SAD lookup, unicast or multicast, finds no match, the traffic is discarded.**

> "In either case (unicast or multicast), if there is no match," and "discard the traffic." — §5.2, `rfc4301.txt:3413`, `rfc4301.txt:3414`

- Strength: description. Class: end-to-end.
- Check idea: send an inbound AH or ESP packet whose SPI matches no SAD entry, in unicast and multicast, and check that the packet is discarded.

### RFC4301-IN-18

**A SAD lookup that finds no match for an inbound AH or ESP packet is an auditable event.**

> "This is an auditable event." — §5.2, `rfc4301.txt:3414`

- Strength: description. Class: internal.
- Check idea: send an inbound AH or ESP packet whose SPI matches no SAD entry and check that the audit log records the event.

### RFC4301-IN-19

**The audit log entry for a failed SAD lookup should include the current date and time, the SPI, the source and destination of the packet, the IPsec protocol, and any other available selector values.**

> "The audit log" and "entry for this event SHOULD include the current date/time, SPI," and "source and destination of the packet, IPsec protocol, and any" and "other selector values of the packet that are available." — §5.2, `rfc4301.txt:3414`, `rfc4301.txt:3423`, `rfc4301.txt:3424`, `rfc4301.txt:3425`

- Strength: should. Class: internal.
- Check idea: cause a SAD lookup to fail for an inbound AH or ESP packet and check that the resulting audit log entry contains the date/time, SPI, source and destination, protocol, and any other available selectors.

### RFC4301-IN-20

**When the packet is found in the SAD, it is processed accordingly, applying AH or ESP processing as step 4 specifies.**

> "If the" and "packet is found in the SAD, process it accordingly (see step 4)." — §5.2, `rfc4301.txt:3425`, `rfc4301.txt:3426`

- Strength: description. Class: internal.
- Check idea: send an inbound AH or ESP packet whose SPI matches a SAD entry and check that it proceeds to AH/ESP processing rather than being discarded.

### RFC4301-IN-21

**A packet not addressed to the device, or addressed to it but not AH or ESP, has its header looked up in the appropriate SPD-I cache.**

> "If the packet is not addressed to the device or is addressed to" and "this device and is not AH or ESP, look up the packet header in" and "the (appropriate) SPD-I cache." — §5.2, `rfc4301.txt:3428`, `rfc4301.txt:3429`, `rfc4301.txt:3430`

- Strength: description. Class: internal.
- Check idea: send a packet not addressed to the device, and one addressed to it but neither AH nor ESP, and check that each is looked up in the appropriate SPD-I cache.

### RFC4301-IN-22

**On an SPD-I cache match, the packet is discarded or bypassed as the entry says.**

> "If there is a match and the" and "packet is to be discarded or bypassed, do so." — §5.2, `rfc4301.txt:3430`, `rfc4301.txt:3431`

- Strength: description. Class: end-to-end.
- Check idea: send a packet matching a DISCARD SPD-I cache entry and one matching a BYPASS entry, and check that each is discarded or bypassed respectively.

### RFC4301-IN-23

**On an SPD-I cache miss, the packet is looked up in the corresponding SPD-I and a cache entry is created as appropriate.**

> "If there is no" and "cache match, look up the packet in the corresponding SPD-I and" and "create a cache entry as appropriate." — §5.2, `rfc4301.txt:3431`, `rfc4301.txt:3432`, `rfc4301.txt:3433`

- Strength: description. Class: internal.
- Check idea: send a packet that misses the SPD-I cache and check that it is then looked up in the SPD-I and that a cache entry is created afterward.

### RFC4301-IN-24

**No SA is created in response to an inbound packet that requires IPsec protection; only BYPASS or DISCARD cache entries can be created this way.**

> "(No SAs are created in" and "response to receipt of a packet that requires IPsec protection;" and "only BYPASS or DISCARD cache entries can be created this way.)" — §5.2, `rfc4301.txt:3433`, `rfc4301.txt:3434`, `rfc4301.txt:3435`

- Strength: description. Class: internal.
- Check idea: send an inbound packet whose SPD-I entry calls for IPsec protection and check that no SA is created for it, unlike a BYPASS or DISCARD match.

### RFC4301-IN-25

**When an inbound packet matches no SPD-I entry, it is discarded.**

> "If" and "there is no match, discard the traffic." — §5.2, `rfc4301.txt:3435`, `rfc4301.txt:3436`

- Strength: description. Class: end-to-end.
- Check idea: send an inbound packet matching no SPD-I entry and check that it is discarded.

### RFC4301-IN-26

**An inbound packet discarded because it matched no SPD-I entry is an auditable event.**

> "This is an auditable" and "event." — §5.2, `rfc4301.txt:3436`, `rfc4301.txt:3437`

- Strength: description. Class: internal.
- Check idea: send an inbound packet matching no SPD-I entry and check that the audit log records the event.

### RFC4301-IN-27

**The audit log entry for an SPD-I no-match discard should include the current date and time, the SPI if available, the IPsec protocol if available, the source and destination of the packet, and any other available selector values.**

> "The audit log entry for this event SHOULD include the" and "current date/time, SPI if available, IPsec protocol if available," and "source and destination of the packet, and any other selector" and "values of the packet that are available." — §5.2, `rfc4301.txt:3437`, `rfc4301.txt:3438`, `rfc4301.txt:3439`, `rfc4301.txt:3440`

- Strength: should. Class: internal.
- Check idea: cause an SPD-I no-match discard and check that the resulting audit log entry contains the date/time, SPI and protocol where available, and the source, destination, and other available selectors.

### RFC4301-IN-28

**Processing of ICMP messages is assumed to take place on the unprotected side of the IPsec boundary.**

> "Processing of ICMP messages is assumed to take place on the" and "unprotected side of the IPsec boundary." — §5.2, `rfc4301.txt:3442`, `rfc4301.txt:3443`

- Strength: description. Class: internal.
- Check idea: check that an unprotected ICMP message received at the device is examined before, not after, IPsec processing.

### RFC4301-IN-29

**Unprotected ICMP messages are examined, and local policy decides whether to accept or reject them and, when accepted, what action to take.**

> "Unprotected ICMP" and "messages are examined and local policy is applied to determine" and "whether to accept or reject these messages and, if accepted, what" and "action to take as a result." — §5.2, `rfc4301.txt:3443`, `rfc4301.txt:3444`, `rfc4301.txt:3445`, `rfc4301.txt:3446`

- Strength: description. Class: internal.
- Check idea: receive an unprotected ICMP message under a policy that accepts it and one that rejects it, and check that the device's reaction follows the configured policy.

### RFC4301-IN-30

**On receipt of an ICMP unreachable message, the implementation decides whether to act on it, reject it, or act on it with constraints.**

> "For example, if an ICMP unreachable" and "message is received, the implementation must decide whether to" and "act on it, reject it, or act on it with constraints." — §5.2, `rfc4301.txt:3446`, `rfc4301.txt:3447`, `rfc4301.txt:3448`

- Strength: must (lower case). Class: internal.
- Check idea: deliver an ICMP unreachable message to the implementation and check that it acts on, rejects, or constrains its response to it, rather than ignoring it outright.

### RFC4301-IN-31

**Step 4 applies AH or ESP processing using the SAD entry selected in step 3a.**

> "Apply AH or ESP processing as specified, using the SAD entry" and "selected in step 3a above." — §5.2, `rfc4301.txt:3451`, `rfc4301.txt:3452`

- Strength: must. Class: internal.
- Check idea: send an inbound AH or ESP packet matched to a SAD entry and check that AH or ESP processing is applied using that same entry.

### RFC4301-IN-32

**Step 4 matches the packet against the inbound selectors identified by the SAD entry, to verify that the received packet is appropriate for the SA on which it arrived.**

> "Then match the packet against the" and "inbound selectors identified by the SAD entry to verify that the" and "received packet is appropriate for the SA via which it was" and "received." — §5.2, `rfc4301.txt:3452`, `rfc4301.txt:3453`, `rfc4301.txt:3454`, `rfc4301.txt:3455`

- Strength: must. Class: end-to-end.
- Check idea: receive an IPsec-protected packet on an SA and check that its header fields are compared against that SA's inbound selectors before delivery.

### RFC4301-IN-33

**When a received packet's header fields are not consistent with the selectors for the SA it arrived on, the implementation must discard the packet.**

> "If an IPsec system receives an inbound packet on an SA and the" and "packet's header fields are not consistent with the selectors for" and "the SA, it MUST discard the packet." — §5.2, `rfc4301.txt:3457`, `rfc4301.txt:3458`, `rfc4301.txt:3459`

- Strength: must. Class: end-to-end.
- Check idea: receive a packet on an SA whose header fields do not match that SA's selectors, and check that the implementation discards it.

### RFC4301-IN-34

**Discarding an inbound packet for failing its SA's selector check is an auditable event.**

> "This is an auditable event." — §5.2, `rfc4301.txt:3459`

- Strength: description. Class: internal.
- Check idea: cause an inbound packet to fail its SA's selector check and check that the audit log records the event.

### RFC4301-IN-35

**The audit log entry for a selector-check discard should include the current date and time, the SPI, the IPsec protocol(s), the source and destination of the packet, any other available selector values of the packet, and the selector values from the relevant SAD entry.**

> "The audit log entry for this event SHOULD include the current" and "date/time, SPI, IPsec protocol(s), source and destination of the" and "packet, any other selector values of the packet that are" and "available, and the selector values from the relevant SAD entry." — §5.2, `rfc4301.txt:3460`, `rfc4301.txt:3461`, `rfc4301.txt:3462`, `rfc4301.txt:3463`

- Strength: should. Class: internal.
- Check idea: cause a selector-check discard and check that the resulting audit log entry contains the date/time, SPI, protocol, source and destination, other available selectors, and the SAD entry's selector values.

### RFC4301-IN-36

**The system should also be capable of sending an IKE notification of INVALID_SELECTORS to the sending IPsec peer, indicating the packet was discarded for failing selector checks.**

> "The system SHOULD also be capable of generating and sending an" and "IKE notification of INVALID_SELECTORS to the sender (IPsec peer)," and "indicating that the received packet was discarded because of" and "failure to pass selector checks." — §5.2, `rfc4301.txt:3464`, `rfc4301.txt:3465`, `rfc4301.txt:3466`, `rfc4301.txt:3467`

- Strength: should. Class: wire.
- Check idea: cause a selector-check discard and check whether an IKE INVALID_SELECTORS notification is sent back to the peer that sent the packet.

### RFC4301-IN-37

**The IPsec system should include a management control letting an administrator configure whether it sends the INVALID_SELECTORS IKE notification.**

> "the" and "IPsec system SHOULD include a management control to allow an" and "administrator to configure the IPsec implementation to send or not" and "send this IKE notification" — §5.2, `rfc4301.txt:3479`, `rfc4301.txt:3480`, `rfc4301.txt:3481`, `rfc4301.txt:3482`

- Strength: should. Class: internal.
- Check idea: look for a configuration control that enables or disables sending the INVALID_SELECTORS IKE notification, and check that turning it off stops those notifications.

### RFC4301-IN-38

**When the INVALID_SELECTORS notification facility is selected, the system should rate-limit the transmission of such notifications.**

> "and if this facility is selected, to rate" and "limit the transmission of such notifications." — §5.2, `rfc4301.txt:3482`, `rfc4301.txt:3483`

- Strength: should. Class: wire.
- Check idea: trigger many selector-check discards on a system configured to send INVALID_SELECTORS notifications and check that the rate of notifications sent is limited rather than one-for-one.

### RFC4301-IN-39

**After traffic is bypassed or processed through IPsec, it is handed to the inbound forwarding function for disposition.**

> "After traffic is bypassed or processed through IPsec, it is handed to" and "the inbound forwarding function for disposition." — §5.2, `rfc4301.txt:3485`, `rfc4301.txt:3486`

- Strength: description. Class: internal.
- Check idea: bypass an inbound packet and protect another through IPsec, and check that both reach the inbound forwarding function afterward.

### RFC4301-IN-40

**The inbound forwarding function may send a packet back out across the IPsec boundary for additional inbound processing, for example to support nested SAs.**

> "This function may" and "cause the packet to be sent (outbound) across the IPsec boundary for" and "additional inbound IPsec processing, e.g., in support of nested SAs." — §5.2, `rfc4301.txt:3486`, `rfc4301.txt:3487`, `rfc4301.txt:3488`

- Strength: may (lower case). Class: internal.
- Check idea: configure nested SAs so that inbound forwarding routes a decapsulated packet back out across the IPsec boundary, and check that it receives additional inbound processing.

### RFC4301-IN-41

**When inbound forwarding loops a packet back across the IPsec boundary, the packet must be matched against an SPD-O entry, as with all outbound traffic to be bypassed.**

> "If so, then as with ALL outbound traffic that is to be bypassed, the" and "packet MUST be matched against an SPD-O entry." — §5.2, `rfc4301.txt:3489`, `rfc4301.txt:3490`

- Strength: must. Class: internal.
- Check idea: loop a decapsulated packet back across the IPsec boundary for bypass and check that it is matched against an SPD-O entry before being forwarded outbound.

## Conformance requirements

### RFC4301-CONF-1

**All IPv4 IPsec implementations must comply with all requirements of this document.**

> "All IPv4 IPsec implementations MUST comply with all requirements of" and "this document." — §10, `rfc4301.txt:3967`, `rfc4301.txt:3968`

- Strength: must. Class: internal.
- Check idea: for an IPv4 IPsec implementation, check it against each requirement cataloged elsewhere for this document and confirm it satisfies all of them.

### RFC4301-CONF-2

**All IPv6 IPsec implementations must comply with all requirements of this document.**

> "All IPv6 implementations MUST comply with all" and "requirements of this document." — §10, `rfc4301.txt:3968`, `rfc4301.txt:3969`

- Strength: must. Class: internal.
- Check idea: for an IPv6 IPsec implementation, check it against each requirement cataloged elsewhere for this document and confirm it satisfies all of them.

## Out of scope in this catalog

The entries above hold every `MUST`, `MUST NOT`, `SHOULD`, `SHOULD NOT` and `MAY` of
§3.1, §3.2, §4 to §4.4.2, §5 to §5.2 and §10, and every field of every header. What the catalog leaves out:

- **The sections outside the in-scope set.** §1 and §2 (introduction, design objectives),
  §3.3 (where IPsec can be implemented), §4.4.3 to §4.6 (the PAD, key management, multicast
  SAs), §6 to §9 (ICMP processing, fragments, path MTU, auditing) and §11 to §14. See
  [`standards.md`](../../protocol/ipsec/standards.md#target-level) for the level at which each
  one enters.

Within the in-scope sections, these parts have no entry, with the reason:

In §3.1 to §4.4:

- `rfc4301.txt:429-440` — definitions of the terms "unprotected"/"protected" interfaces and "inbound"/"outbound"; terminology, not itself a checkable statement.
- `rfc4301.txt:455-458` — a note summarizing Figure 1 (the Discard, BYPASS, and IKE elements); descriptive summary, no new normative content.
- `rfc4301.txt:460-463` — IPsec's optional support for IP compression negotiation; informative, and compression itself is specified elsewhere, out of this range.
- `rfc4301.txt:496-499` — "these protocols may be applied individually or in combination..." and the two-modes intro sentence; restated with full normative detail in the SA area.
- `rfc4301.txt:499-513` — a high-level description of transport mode vs. tunnel mode; the substantive placement and coverage rules are cataloged in the SA area from §4.1.
- `rfc4301.txt:515-528` — an informal overview of SPD-based granularity control, with illustrative examples; the formal SPD requirements are cataloged in the SPD area of a later draft.
- `rfc4301.txt:538-543` — a note pointing to IKEv2-only features (port-range negotiation, multiple SAs with the same selectors); an informative pointer, the substantive requirements are cataloged where those features are specified.
- `rfc4301.txt:607-609` — states which implementations this section's requirements apply to; a scope statement, not itself a requirement.
- `rfc4301.txt:609-612` — "a major function of IKE is the establishment and maintenance of SAs"; describes IKE's own role, not IPsec traffic processing.
- `rfc4301.txt:636-637` — "IKE explicitly creates SA pairs..."; describes IKE's own behavior.
- `rfc4301.txt:650-656` — background on Group Controller/Key Server SPI assignment for GSAs; key-management/GDOI background, informative.
- `rfc4301.txt:697-705` — hash-table and TCAM implementation examples; illustrative, add no new requirement beyond the equivalence MUST already cataloged (RFC4301-SA-20).
- `rfc4301.txt:735-737` — "Distribution of traffic among these parallel SAs...is not negotiated by IKE"; a local/IKE-scope note, no independently checkable requirement.
- `rfc4301.txt:738-739` — "These requirements apply to both transport and tunnel mode SAs"; a scope clarifier already reflected in RFC4301-SA-24 and RFC4301-SA-26.
- `rfc4301.txt:740-742` — "the DSCP value might change en route, but this should not cause problems..."; rationale leading into the MUST NOT already cataloged (RFC4301-SA-28).
- `rfc4301.txt:744-747` — explains a possible side effect (anti-replay discarding) of DSCP-induced reordering; not itself a new requirement, and anti-replay windowing is specified in the AH/ESP documents, out of range.
- `rfc4301.txt:749-753` — a labeled DISCUSSION block on DSCP/ECN vs. selectors and the "classifier" concept; informative.
- `rfc4301.txt:755-756` — "two types of SAs are defined: transport mode and tunnel mode"; restates a point already established.
- `rfc4301.txt:773-779` — explains why access control is limited in this context, and advises administrators to evaluate this use of transport mode carefully; rationale and advisory guidance, not a testable implementation behavior.
- `rfc4301.txt:804-805` — "For more details on the coverage afforded by AH, see the AH specification"; a pointer.
- `rfc4301.txt:812-813` — "Thus, an SA between two security gateways is typically a tunnel mode SA..."; restates RFC4301-SA-42.
- `rfc4301.txt:818-820` — explains why exception 1 applies (the SA terminates at a host-management function); rationale.
- `rfc4301.txt:822-825` — "security gateways MAY support a transport mode SA" between two intermediate systems along a path; restates, as "exception 2," the MAY already cataloged as RFC4301-SA-31.
- `rfc4301.txt:827-831` — motivates tunnel mode by the need to reach the negotiated gateway when multiple paths exist; rationale.
- `rfc4301.txt:834-839`, `rfc4301.txt:847-850` — further motivation (retaining fragmentation state across inner/outer headers) and a parenthetical alternative (IP-in-IP plus transport mode) with its access-control limitation; rationale.
- `rfc4301.txt:854-855` — notes that the IPv6 restriction is adopted for simplicity, not technical necessity; rationale.
- `rfc4301.txt:856-858` — "See Section 7 for more details..."; a pointer.
- `rfc4301.txt:878-881` — summarizes when a security gateway's transport mode support should be used; restates the two exceptions already cataloged (RFC4301-SA-43, RFC4301-SA-31).
- `rfc4301.txt:908-910` — "the authentication means employed by IPsec peers...also affects the granularity..."; describes an IKE/key-management property, not independently testable.
- `rfc4301.txt:918-921` — an illustrative example (a mobile dialup user and a corporate firewall).
- `rfc4301.txt:921-924` — a general security observation (fine-granularity SAs are more vulnerable to traffic analysis); not a requirement on implementation behavior.
- `rfc4301.txt:937-940` — nested/bundled SA support, where offered, is achieved through SPD and forwarding configuration; explicitly outside the IPsec module and the scope of this specification.
- `rfc4301.txt:940-942` — rationale/commentary comparing nested/bundled SA management complexity to the RFC 2401 model.
- `rfc4301.txt:946-947` — "See Appendix E for an example..."; a pointer.
- `rfc4301.txt:961-966` — meta-commentary on why this section standardizes some external, interoperability-relevant aspects while leaving other details local.
- `rfc4301.txt:992-995` — "context identifiers MAY be conveyed...in the signaling messages"; describes optional IKE signaling of context identifiers, not itself a directly IPsec-traffic-testable rule.
- `rfc4301.txt:996-998` — an illustrative example (a VPN service to multiple customers).
- `rfc4301.txt:1002-1006`, `rfc4301.txt:1015` — describes the forwarding/security-decision separation and gives examples of forwarding complexity (trivial vs. sophisticated); not itself a testable requirement.
- `rfc4301.txt:1018-1021` — restates, in the context of forwarding, that nested SA support is optional and needs forwarding/SPD coordination; already cataloged in the COMB area from §4.3 (RFC4301-COMB-1).
- `rfc4301.txt:1023-1032` — defines the terms "Local"/"Remote" versus "source"/"destination"; a definition. The policy rules that use these terms are cataloged in the SPD/selector areas of a later draft.

In §4.4.1 to §4.4.1.2:

- 1050-1058: background on what an SA is and a scope statement for the SPD's form and
  interface; no checkable statement.
- 1073-1074: comparison to the superseded RFC 2401, stating the absence of a per-interface SPD
  requirement; not testable.
- 1083-1086: rationale for the SPD being an ordered database; the operative MUST is cataloged as
  RFC4301-SPD-6.
- 1088-1091: rationale for why no canonical order can be imposed on SPD entries.
- 1127-1131: descriptive elaboration of how a single SPD or partial SPDs compose SPD-S/SPD-O/
  SPD-I; not an independent requirement beyond RFC4301-SPD-12.
- 1249-1252: informative note on IKEv2 negotiating a subset of SPD selector values in the
  non-PFP case; describes IKE, not an SPD requirement.
- 1252-1255: "is a local matter" statement about whether one PFP flag or several cover related
  selectors; not testable.
- 1257-1278: the worked PFP example (192.0.2.1-192.0.2.10 case) and its table; illustrative
  example only.
- 1286-1287, 1295-1297: parenthetical noting a native host socket-interface implementation may
  not need per-packet SPD consultation; an optional relief, not an independent requirement.
- 1300-1303: analogy between SPD selectors and firewall ACLs/packet filters; rationale only.
- 1306-1307: parenthetical placing the signaling of application SPD requests out of scope.
- 1309-1313: statement that the management interface's form is unspecified and may differ by
  implementation; not testable.
- 1318-1329: rationale for decorrelation as an optional performance technique, not required by
  this RFC.
- 1330-1334: note on SPD terminology and a pointer to the Appendix B decorrelation algorithm.
- 1338-1354: the worked decorrelation example (entry A decorrelating into A1/A2/A3).
- 1356-1380: the three options for what an initiator sends a peer via an SA management protocol
  (IKE) when using a decorrelated SPD, and their tradeoffs; describes IKE message content
  choices, not an IPsec-traffic-observable requirement.
- 1382-1386: introductory sentence to responder matching, leading into RFC4301-SPD-40 and
  RFC4301-SPD-41.
- 1392-1393: note that IKEv2 with a decorrelated SPD offers the best chance of a "narrowed"
  response; an informative IKE benefit, not a requirement.
- 1422-1431: background and example on SA granularity (fine- vs coarse-grained SAs).
- 1445-1447: rationale for Remote address ranges supporting a shared SA behind a security
  gateway.
- 1469-1475: elaboration of the GSPD's differing structure and the outbound/inbound multicast
  asymmetry; out of scope of the SPD itself.
- 1520-1522: describes how IKE encodes the Mobility Header type in its own 16-bit port
  selector; an IKE encoding detail, not IPsec traffic behavior.
- 1529-1531: describes how IKE encodes the ICMP type and code in its own 16-bit selector; an IKE
  encoding detail.
- 1553-1554: describes the name being carried in the IKE ID payload during negotiation; an IKE
  protocol detail.
- 1581-1583: note that the initiator's name is local only and not carried by the key management
  protocol, and a cross-reference to name forms used in the initiator context; informative.
- 1592-1607: the four detailed examples of responder identifier types (a-d), with their IKEv2
  ID-payload correspondences; illustrative detail already generalized in RFC4301-SEL-28.
- 1615-1622: background on how the IPsec implementation context (native host socket interface
  vs. BITS/BITW/security gateway) affects how often the SPD is consulted; descriptive.
- 1633-1635: pointer to the Appendix C ASN.1 example of an SPD entry.
- 1637-1648: rationale for why the SPD text is written to map to IKE payloads, and the
  described mismatch between SPD semantics and the concurrently published IKEv2; background for
  RFC4301-SPDE-1.
- 1656-1660: description of optional management-GUI conveniences (address prefixes, symbolic
  names) and a note not to confuse them with the SPD selector "Name"; not testable.
- 1671-1672: lead-in sentence to the SPD entry field list; no independent claim.
- 1695-1698: "is a local matter" statement about whether a single PFP flag or separate flags
  cover related selectors; not testable.
- 1766-1767: "It is a local matter as to what information is kept with regard to handling
  extant SAs when the SPD is changed."; explicitly leaves the behavior unspecified, not
  testable.

In §4.4.1.3 to §4.4.2.2:

- `rfc4301.txt:1771-1780` — introductory framing and terminology for §4.4.1.3 ("port" means a Next Layer Protocol field; a protocol can have zero, one, or two such selectors; AH and ESP have none; the IPv6 Mobility Header has one). Instantiated precisely by NLP-1 through NLP-6.
- `rfc4301.txt:1785-1801` — worked example instantiating rule A with AH's Local and Remote SPD fields; the general rule is NLP-1.
- `rfc4301.txt:1803-1806` — framing sentence introducing rule B (a protocol with a single selector) before its three concrete cases, cataloged as NLP-2 through NLP-4.
- `rfc4301.txt:1876-1888` — worked example of rule D using a security gateway that allows outbound ICMP traceroutes but not inbound ones; the general rule is NLP-6.
- `rfc4301.txt:1900-1902` — cross-reference to the inbound-SA-lookup algorithm required by §4.1; that algorithm's own requirement is defined outside this range.
- `rfc4301.txt:1919-1921` — cross-reference to the §4.4.1 guidance on the effect of SPD changes on extant SAs; no new checkable statement here.
- `rfc4301.txt:1926-1929` — cross-reference to §6 for ICMP-message rules, and to §4.4.1.1 for the forms a selector can take (specific value, range, ANY, OPAQUE); both are catalogued elsewhere.
- `rfc4301.txt:1911-1915` — statement that the SAD data items should all be present except where otherwise noted, and that the list is not a MIB, only a specification of the minimal data items; informative framing for the SADI list, not itself a checkable behavior.
- `rfc4301.txt:1944-1948` — rationale explaining why inbound-multicast source-address support is already present in an IPsec implementation (because unicast SPD entries can already expand to multiple SAD source addresses); an argument, not a checkable statement.
- `rfc4301.txt:1954-1977` — "Implementation Guidance" paragraph: states that this document does not specify how an SPD-S entry refers to its SAD entry, recounts problems some RFC 2401-era implementations had with a (remote tunnel IP address, remote SPI) key, and closes with an unquantified recommendation that implementors avoid such problems. Advisory and historical, with no independently testable condition.
- `rfc4301.txt:2013-2022`, `2074-2078` — page-break furniture (author footer and "RFC 4301" header lines) between SADI bullets, with no body text of their own.
- `rfc4301.txt:2041-2049` — ties the SA lifetime to IKE's use of X.509 certificate validity intervals and CRL NextIssueDate, and assigns responsibility to both IKE peers; describes a key-management interaction, not independently checkable from IPsec traffic, plus an introductory "Note" pointing to the (a)/(b)/(c) list that follows (cataloged as SADI-12, SADI-14, SADI-15).
- `rfc4301.txt:2107-2115` — introductory framing for the selector/PFP/packet/SAD tables, including the aside that an administrative interface may offer syntactic shortcuts such as a single address or prefix instead of a list of ranges; no new checkable statement beyond the tables themselves.
- `rfc4301.txt:2191-2192` — framing sentence noting that a protocol with two ports has selectors for both Local and Remote ports, leading into the loc-port/rem-port tables (PFP-12 through PFP-21).
- `rfc4301.txt:2247-2248` — framing sentence noting that a mobility-header protocol has an mh-type selector, leading into the mh-type table (PFP-22 through PFP-26).
- `rfc4301.txt:2359` — bare lead-in line ("If the name selector is used:") before the name-selector table, folded into PFP-33.
- `rfc4301.txt:2159`, `2163`, `2208`, `2212`, `2225`, `2229`, `2264`, `2268`, `2336-2337`, `2345` — the "***" table rows marking PFP=1 combined with an OPAQUE selector value (protocol, local port, remote port, mh type, and ICMP type-and-code tables); all instantiate the single general rule cataloged once as PFP-11, rather than being repeated per table.
- `rfc4301.txt:2368-2370` — footnote "*": clarifies that "list of protocols" describes the information carried, not how the SPD, SAD, or IKEv2 represent it; a representational note, not a checkable behavior.
- `rfc4301.txt:2371-2372` — footnote "**": states that IKE uses 0 to mean ANY for the protocol; describes IKE's own wire encoding, not a behavior of the IPsec implementation's traffic processing.

In §5 to §5.2, and §10:

- `rfc4301.txt:2798-2823` — background on the notion of an SPD cache (one cache per SPD, the assumption that each cached entry maps to one SA) and the exceptions where a SAD entry has no corresponding SPD entry: informative architecture and assumptions, not a requirement of an implementation.
- `rfc4301.txt:2824-2828`, `rfc4301.txt:2830-2834` — rationale for why SPD entries must be decorrelated before caching, and the documentation convention that later uses of "SPD"/"SPD cache"/"cache" mean the decorrelated SPD: explanatory and a documentation convention, not a requirement.
- `rfc4301.txt:2836-2841` — Note on socket-based host implementations providing implicit caching: an illustrative example of one implementation style, not a general requirement.
- `rfc4301.txt:2843-2847` — Note on starting from a correlated SPD and applying a decorrelation algorithm: explanatory.
- `rfc4301.txt:2849-2851` — "For inbound IPsec traffic, the SAD entry selected by the SPI serves as the cache...": a preview of the rule cataloged precisely as the matching-and-verification steps of the Inbound IP traffic processing area (step 4, RFC4301-IN-31/32); redundant with those entries.
- `rfc4301.txt:2865-2903` — introductory sentence and Figure 2 (the outbound processing diagram) with its caption; the caption's remark that "There is no requirement that an implementation buffer the packet if there is a cache miss" disclaims a requirement rather than stating one, so there is nothing to check.
- `rfc4301.txt:2919-2920`, `rfc4301.txt:3387-3388` — "IPsec MUST perform the following steps...": the umbrella sentence introducing the numbered steps; its MUST is carried by the individual per-step entries below (RFC4301-OUT-1 through 16, RFC4301-IN-7 through 41), not by a separate entry.
- `rfc4301.txt:2925` — "(If the implementation uses only one SPD, this step is a no-op.)": a clarification of RFC4301-OUT-1's scope, not a separately testable claim.
- `rfc4301.txt:2984-2987`, second sentence — "(This applies only to IPv4. For IPv6 packets, only the originator is allowed to fragment them.)": restates the general IPv6 fragmentation rule of RFC 8200, not an IPsec-specific requirement.
- `rfc4301.txt:3003-3005` — "The type and code of the ICMP message will depend on the reason for discarding the packet, as specified below.": a lead-in to items a, b1, and b2, which are cataloged individually (RFC4301-DISC-4 through 9).
- `rfc4301.txt:3053-3057` — the DoS/spoofed-source-address rationale for why a security gateway should control ICMP generation: explains the reason for RFC4301-DISC-10/11, not itself a requirement.
- `rfc4301.txt:3065-3071` — introductory description of what §5.1.2 covers, modeled after RFC 2003: informative overview, no independent checkable claim.
- `rfc4301.txt:3099-3101` — "Note: IPsec tunnel mode is different from IP-in-IP tunneling... in several ways:": a lead-in to the bulleted differences, which are cataloged individually.
- `rfc4301.txt:3102-3106`, `rfc4301.txt:3113-3116` — covert-channel rationale for the outer-DS-field configurability of RFC4301-TUN-4/5: explanatory, not itself a requirement.
- `rfc4301.txt:3144-3150` — rationale for per-SA discard-versus-overwrite configurability and a pointer to RFC 2983 for further information: explanatory and bibliographic.
- `rfc4301.txt:3155-3158` — lead-in defining what "constructed" means in the tables that follow: definitional, not itself a requirement.
- `rfc4301.txt:3181-3182`, `rfc4301.txt:3272-3275` — the "Notes:" headers and the "(1) - (6) See Section 5.1.2.1." cross-reference: structural pointers; the footnotes themselves are cataloged where they add content.
- `rfc4301.txt:3199-3208` — extended note that decrementing TTL is a normal part of forwarding, that a packet originating at the encapsulator is not decremented since it is not being forwarded, and that TTL processing can prevent loops: clarifies the scope of RFC4301-TUN-16/17 but states no new field-value rule.
- `rfc4301.txt:3230-3232` — "[NiBlBaBL98]. See RFC 2475 [BBCDWW98] for further information.": bibliographic pointer following RFC4301-TUN-13.
- `rfc4301.txt:3311-3313` — "If an arriving packet appears to be an IPsec fragment from an unprotected interface, reassembly is performed prior to IPsec processing.": a preview restated precisely at `rfc4301.txt:3380-3382` (cataloged as RFC4301-IN-5); redundant.
- `rfc4301.txt:3320-3378` — Figure 3 (the inbound processing diagram) and its legend notes (*), (**), (***): diagram illustration cross-referencing the numbered steps already cataloged; the legend's remark about no buffering requirement on a cache miss again disclaims a requirement rather than stating one.
- `rfc4301.txt:3491-3492` — "Ultimately, the packet should be forwarded to the destination host or process for disposition.": a lowercase "should" describing the ordinary outcome of IP delivery in general, not an IPsec-specific requirement.
