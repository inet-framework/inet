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
| `stale` | The code moved under the report. |
| `method-stale` | The workflow gained a pass this report never had. |
| `spec-stale` | A cited document was obsoleted or updated. |

## Families

| Family | Directories under `src/inet/` | Specs | Status | Workflow | Reviewed at | Report |
|---|---|---|---|---|---|---|
| IPv4 with ARP and ICMP | `networklayer/ipv4`, `networklayer/arp/ipv4` | [manifest](ipv4/spec-manifest.md) | `method-stale` | v1 | `7b0a6de5cef3` | [report](ipv4/conformance-review.md) |
| IGMP | `networklayer/ipv4` (IGMPv2, IGMPv3) | RFC 2236, RFC 3376 | `not-started` | — | — |
| IPv4 NAT | `networklayer/ipv4` (`Ipv4NatTable`) | RFC 3022 | `not-started` | — | — |
| Ethernet | `linklayer/ethernet` | IEEE 802.3 | `not-started` | — | — |
| OSPFv2 | `routing/ospfv2`, `routing/ospf_common` | RFC 2328 | `not-started` | — | — |
| DHCP | `applications/dhcp` | RFC 2131, RFC 2132 | `not-started` | — | — |
| IEEE 802.11 management | `linklayer/ieee80211/mgmt` | IEEE 802.11 | `not-started` | — | — |

The table grows one row at a time, as a family enters the queue. Do not enumerate the whole source
tree here in advance.

## Conformance tests

A review that finds a reachable defect leaves an executable test behind, under
`tests/protocol/conformance/`. Each test asserts what the standard requires and carries an
`%expected-failure:` naming its finding id, so it turns green by itself when the defect is fixed.

Note that `inet_run_protocol_tests` does not recurse into that folder yet.

## Staleness

`Reviewed at` holds the commit the review was made against. `Directories` holds the exact path
list it covered. Together they let a later check detect that the code moved under a report, and
mark the family `stale`.

## Ordering

1. User impact first.
2. Spec availability second. A blocked family waits.
3. Implementation age third. Old, rarely touched code hides more.
