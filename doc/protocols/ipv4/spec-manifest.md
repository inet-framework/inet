# IPv4 with ARP and ICMP — specification manifest

The documents this conformance review is judged against. Approved at the P0 checkpoint on
2026-08-25. Every relation below was read from `https://www.rfc-editor.org/rfc/rfc<N>.json`, not
from memory.

The document texts are **not** in this repository. They live in the local cache at
`~/w/ai/spec-cache/ipv4/`, one `rfc<N>.txt` and one `rfc<N>-errata.txt` per entry.

## Documents

| Document | Title | Status | Published | Updated by | Errata (total / significant) |
|---|---|---|---|---|---|
| RFC 791 | Internet Protocol | Internet Standard | September 1981 | RFC1349,RFC2474,RFC6864 | 22 / 17 |
| RFC 792 | Internet Control Message Protocol | Internet Standard | September 1981 | RFC950,RFC4884,RFC6633,RFC6918 | 7 / 7 |
| RFC 826 | An Ethernet Address Resolution Protocol | Internet Standard | November 1982 | RFC5227,RFC5494 | 4 / 1 |
| RFC 950 | Internet Standard Subnetting Procedure | Internet Standard | August 1985 | RFC6918 | 0 / 0 |
| RFC 1071 | Computing the Internet checksum | Informational | September 1988 | RFC1141 | 4 / 4 |
| RFC 1122 | Requirements for Internet Hosts - Communication Layers | Internet Standard | October 1989 | RFC1349,RFC4379,RFC5884,RFC6093,RFC6298,RFC6633,RFC6864,RFC8029,RFC9293 | 13 / 12 |
| RFC 1191 | Path MTU discovery | Draft Standard | November 1990 | — | 1 / 1 |
| RFC 1624 | Computation of the Internet Checksum via Incremental Update | Informational | May 1994 | — | 3 / 3 |
| RFC 1812 | Requirements for IP Version 4 Routers | Proposed Standard | June 1995 | RFC2644,RFC6633 | 5 / 5 |
| RFC 2474 | Definition of the Differentiated Services Field (DS Field) in the IPv4 and IPv6 Headers | Proposed Standard | December 1998 | RFC3168,RFC3260,RFC8436 | 1 / 1 |
| RFC 3168 | The Addition of Explicit Congestion Notification (ECN) to IP | Proposed Standard | September 2001 | RFC4301,RFC6040,RFC8311,RFC9768 | 10 / 6 |
| RFC 5227 | IPv4 Address Conflict Detection | Proposed Standard | July 2008 | — | 1 / 0 |
| RFC 6633 | Deprecation of ICMP Source Quench Messages | Proposed Standard | May 2012 | — | 0 / 0 |
| RFC 6864 | Updated Specification of the IPv4 ID Field | Proposed Standard | February 2013 | — | 0 / 0 |
| RFC 6918 | Formally Deprecating Some ICMPv4 Message Types | Proposed Standard | April 2013 | — | 0 / 0 |

"Significant" errata are those with status `Verified` or `Held for Document Update`. `Rejected`
and `Reported` records are present in the cache files, but carry no weight as evidence.

## Roles

| Document | Role in this review |
|---|---|
| RFC 791 | IP. The base document. |
| RFC 792 | ICMP. The base document. |
| RFC 826 | ARP. The base document. |
| RFC 1122 | Host requirements. The normative source for host-side behavior. |
| RFC 1812 | Router requirements. The normative source for forwarding behavior. |
| RFC 6864 | Updates RFC 791. IPv4 Identification field semantics. |
| RFC 2474 | Updates RFC 791. The DS field replaces the ToS byte. |
| RFC 3168 | Updates RFC 2474. ECN occupies the low two bits of the DS byte. |
| RFC 950 | Updates RFC 792. Subnetting, and the Address Mask messages. |
| RFC 6633 | Updates RFC 792, 1122, 1812. Deprecates Source Quench. |
| RFC 6918 | Updates RFC 792, 950. Formally deprecates several ICMPv4 types. |
| RFC 5227 | Updates RFC 826. IPv4 address conflict detection. |
| RFC 1191 | Path MTU discovery. Depends on ICMP Fragmentation Needed. |
| RFC 1071 | Informational. The Internet checksum algorithm. |
| RFC 1624 | Informational. Incremental checksum update. |

## Excluded, with reason

| Document | Reason |
|---|---|
| RFC 1349 | Obsoleted by RFC 2474. |
| RFC 4884 | Multi-part ICMP. Outside the declared scope of this family. |
| RFC 5494 | IANA allocation guidelines for ARP. No implementation behavior. |
| RFC 3927 | IPv4 link-local addressing. A separate capability. |
| RFC 815 | Reassembly algorithms. Informational, and superseded in practice. |
| RFC 2236, RFC 3376 | IGMP. Excluded from this family at the P0 checkpoint. Queue it separately. |
| RFC 3022 | Traditional NAT. Excluded from this family at the P0 checkpoint. |

## Retrieval

- Texts: `https://www.rfc-editor.org/rfc/rfc<N>.txt`, retrieved 2026-08-25.
- Metadata: `https://www.rfc-editor.org/rfc/rfc<N>.json`, retrieved 2026-08-25.
- Errata: the full dump at `https://www.rfc-editor.org/errata.json`, retrieved 2026-08-25, then
  filtered per document. The per-RFC HTML errata pages do not parse reliably.
