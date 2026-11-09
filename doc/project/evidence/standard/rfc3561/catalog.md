# RFC 3561 — selected behavioral requirements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC3561-*` · **Stands on:** [RFC 3561](https://www.rfc-editor.org/rfc/rfc3561.html)

This catalog selects the clauses used by the AODV correction effort. It is not an exhaustive
catalog or a claim of complete conformance. RFC 3561 is Experimental. The requirements below
describe interoperability within its applicability assumptions. RFC 5148 supplies companion jitter
advice. Verified [erratum 7762](https://www.rfc-editor.org/errata/eid7762) corrects wording in §6.7.

| Identifier | Clause | Checkable requirement |
| --- | --- | --- |
| [RFC3561-SEQUENCE](#rfc3561-sequence) | §6.1 | Sequence comparison uses 32-bit serial arithmetic. Unknown sequence fields do not establish freshness. |
| [RFC3561-ROUTE](#rfc3561-route) | §6.2 | Retain destination, sequence validity, next hop, interface, hop count, precursor and expiration state. Only valid routes forward data. |
| [RFC3561-DISCOVERY](#rfc3561-discovery) | §§6.3–6.4 | Buffer originating data, bound request generation, expand TTL rings and back off network-wide retries. |
| [RFC3561-REVERSE](#rfc3561-reverse) | §6.5 | Select the reverse path using originator freshness and hop count. Refresh its minimal lifetime independently. Suppress duplicate requests. |
| [RFC3561-DESTINATION](#rfc3561-destination) | §§6.1, 6.6.1 | A destination replies using its own sequence information and a zero hop count. These clauses contain a sequence-update ambiguity. |
| [RFC3561-CACHED-REPLY](#rfc3561-cached-reply) | §6.6.2 | A sufficiently fresh valid destination route may answer when D is clear. Advertise its remaining lifetime. |
| [RFC3561-GRATUITOUS](#rfc3561-gratuitous) | §6.6.3 | G requests an additional reply toward the destination describing the reverse path, including its remaining lifetime. |
| [RFC3561-REPLY](#rfc3561-reply) | §6.7 | Accept unknown/newer destination sequence information or equal sequence with an inactive/shorter route. Forward a reply only after creating/updating its route. |
| [RFC3561-ACK](#rfc3561-ack) | §6.8 | A-flagged replies elicit one-hop acknowledgments. Failed validation blacklists the affected neighbor. |
| [RFC3561-HELLO](#rfc3561-hello) | §§6.9–6.10 | Hellos advertise direct reachability. Reception observations, rather than outgoing route use, establish continued neighbor connectivity. |
| [RFC3561-ERROR](#rfc3561-error) | §§5.3, 6.11 | Report affected destinations and sequences to precursors. Ordinary errors invalidate routes. N notifications do not delete them. The destination count is one octet. |
| [RFC3561-REPAIR](#rfc3561-repair) | §6.12 | Optional local repair buffers subsequent transit data and notifies precursors on failure or a longer recovered path. |
| [RFC3561-REBOOT](#rfc3561-reboot) | §6.13 | After sequence loss, defer route advertisements and forwarding for DELETE_PERIOD, extended by transit data. |
| [RFC3561-INTERFACE](#rfc3561-interface) | §6.14 | Learn the incoming interface and broadcast discovery across configured interfaces while retaining one originator identity. |
| [RFC3561-SUBNET](#rfc3561-subnet) | §§7–8 | Nonzero reply prefixes describe subnet routes. Sequence and precursor information belongs to that subnet. Member ICMP errors do not invalidate the aggregate. |
| [RFC3561-PARAMETER](#rfc3561-parameter) | §10 | Traversal, retention, retry and advertisement parameters must support the protocol's timing relationships. |

Model interpretations and implementation restrictions belong to
[model conformance](../../model/aodv/conformance.md), rather than to these source requirements.

## RFC3561-SEQUENCE

Sequence comparison uses 32-bit serial arithmetic. Unknown sequence fields do not establish freshness.

Source: §6.1.

## RFC3561-ROUTE

Retain destination, sequence validity, next hop, interface, hop count, precursor and expiration state. Only valid routes forward data.

Source: §6.2.

## RFC3561-DISCOVERY

Buffer originating data, bound request generation, expand TTL rings and back off network-wide retries.

Source: §§6.3–6.4.

## RFC3561-REVERSE

Select the reverse path using originator freshness and hop count. Refresh its minimal lifetime independently. Suppress duplicate requests.

Source: §6.5.

## RFC3561-DESTINATION

A destination replies using its own sequence information and a zero hop count. These clauses contain a sequence-update ambiguity.

Source: §§6.1, 6.6.1.

## RFC3561-CACHED-REPLY

A sufficiently fresh valid destination route may answer when D is clear. Advertise its remaining lifetime.

Source: §6.6.2.

## RFC3561-GRATUITOUS

G requests an additional reply toward the destination describing the reverse path, including its remaining lifetime.

Source: §6.6.3.

## RFC3561-REPLY

Accept unknown/newer destination sequence information or equal sequence with an inactive/shorter route. Forward a reply only after creating/updating its route.

Source: §6.7.

## RFC3561-ACK

A-flagged replies elicit one-hop acknowledgments. Failed validation blacklists the affected neighbor.

Source: §6.8.

## RFC3561-HELLO

Hellos advertise direct reachability. Reception observations, rather than outgoing route use, establish continued neighbor connectivity.

Source: §§6.9–6.10.

## RFC3561-ERROR

Report affected destinations and sequences to precursors. Ordinary errors invalidate routes. N notifications do not delete them. The destination count is one octet.

Source: §§5.3, 6.11.

## RFC3561-REPAIR

Optional local repair buffers subsequent transit data and notifies precursors on failure or a longer recovered path.

Source: §6.12.

## RFC3561-REBOOT

After sequence loss, defer route advertisements and forwarding for DELETE_PERIOD, extended by transit data.

Source: §6.13.

## RFC3561-INTERFACE

Learn the incoming interface and broadcast discovery across configured interfaces while retaining one originator identity.

Source: §6.14.

## RFC3561-SUBNET

Nonzero reply prefixes describe subnet routes. Sequence and precursor information belongs to that subnet. Member ICMP errors do not invalidate the aggregate.

Source: §§7–8.

## RFC3561-PARAMETER

Traversal, retention, retry and advertisement parameters must support the protocol's timing relationships.

Source: §10.
