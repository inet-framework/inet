# INET protocol conformance reviews

Each row is one protocol family, or one slice of a large family. A review judges the family
against its standards and reports what is missing, partial, or incorrect. The workflow is the
`inet-protocol-conformance-review` skill.

A report is a living document. When a commit closes a finding, it updates the report's matrix row
in the same commit.

## Status values

| Status | Meaning |
|---|---|
| `not-started` | The family is queued. Nothing has been done. |
| `spec-pack` | The spec set is approved. The documents are being collected. |
| `spec-blocked` | A required document cannot be obtained. The report says what is missing. |
| `in-review` | The scope statement is approved. Enumeration and mapping are running. |
| `reviewed` | The report is complete and signed off. |
| `stale` | The code changed after the review. The report may no longer match. |

## Families

| Family | Directories under `src/inet/` | Specs | Status | Reviewed at | Report |
|---|---|---|---|---|---|
| IPv4 with ARP and ICMP | `networklayer/ipv4`, `networklayer/arp/ipv4` | [manifest](ipv4/spec-manifest.md) | `spec-pack` | — | — |
| IGMP | `networklayer/ipv4` (IGMPv2, IGMPv3) | RFC 2236, RFC 3376 | `not-started` | — | — |
| IPv4 NAT | `networklayer/ipv4` (`Ipv4NatTable`) | RFC 3022 | `not-started` | — | — |
| Ethernet | `linklayer/ethernet` | IEEE 802.3 | `not-started` | — | — |
| OSPFv2 | `routing/ospfv2`, `routing/ospf_common` | RFC 2328 | `not-started` | — | — |
| DHCP | `applications/dhcp` | RFC 2131, RFC 2132 | `not-started` | — | — |
| IEEE 802.11 management | `linklayer/ieee80211/mgmt` | IEEE 802.11 | `not-started` | — | — |

The table grows one row at a time, as a family enters the queue. Do not enumerate the whole source
tree here in advance.

## Staleness

`Reviewed at` holds the commit the review was made against. `Directories` holds the exact path
list it covered. Together they let a later check detect that the code moved under a report, and
mark the family `stale`.

## Ordering

1. User impact first.
2. Spec availability second. A blocked family waits.
3. Implementation age third. Old, rarely touched code hides more.
