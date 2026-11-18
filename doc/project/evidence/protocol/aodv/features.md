# AODV — selected protocol features

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `AODV-F-*` · **Stands on:** [RFC 3561 catalog](../../standard/rfc3561/catalog.md)

| Feature | Requirements | Behavior |
| --- | --- | --- |
| [AODV-F-FRESHNESS](#aodv-f-freshness) | RFC3561-SEQUENCE, RFC3561-REVERSE, RFC3561-REPLY | Message-specific route selection preserves destination freshness. |
| [AODV-F-LIFETIME](#aodv-f-lifetime) | RFC3561-ROUTE, RFC3561-CACHED-REPLY, RFC3561-GRATUITOUS | Refresh and advertise usable route durations while retaining invalid route history. |
| [AODV-F-DISCOVERY](#aodv-f-discovery) | RFC3561-DISCOVERY | Demand-driven expanding-ring discovery makes bounded progress for finite workload. |
| [AODV-F-LINK](#aodv-f-link) | RFC3561-ACK, RFC3561-HELLO | Correlate unidirectional validation and connectivity observations with the affected neighbor. |
| [AODV-F-EXTENSION](#aodv-f-extension) | RFC3561-EXTENSION | Parse appended extensions without silently skipping mandatory information. |
| [AODV-F-HELLO-INTERVAL](#aodv-f-hello-interval) | RFC3561-HELLO-INTERVAL | Advertise a Hello interval so neighboring routers can use different transmission intervals. |
| [AODV-F-ERROR](#aodv-f-error) | RFC3561-ERROR | Propagate ordinary errors and nondeleting notifications to affected precursors. |
| [AODV-F-RECOVERY](#aodv-f-recovery) | RFC3561-REBOOT | Suppress premature forwarding and advertisements after sequence loss. |
| [AODV-F-INTERFACE](#aodv-f-interface) | RFC3561-INTERFACE | Operate across selected ad hoc links using one originator identity. |
| [AODV-F-SUBNET](#aodv-f-subnet) | RFC3561-SUBNET | Receive, select and forward subnet information without conflating counter domains. |
| [AODV-F-REPAIR](#aodv-f-repair) | RFC3561-REPAIR | Optionally repair a broken path before reporting final failure. |

These are protocol features. Their implementation status and evidence are recorded in the [model coverage ledger](../../model/aodv/coverage.md). This selection does not establish a conformance level. Model-specific cooperation mechanisms are described in the [model claims](../../model/aodv/conformance.md), rather than treated as additional requirements of the RFC.

## AODV-F-FRESHNESS

Message-specific route selection preserves destination freshness.

Source: RFC3561-SEQUENCE, RFC3561-REVERSE, RFC3561-REPLY.

## AODV-F-LIFETIME

Refresh and advertise usable route durations while retaining invalid route history.

Source: RFC3561-ROUTE, RFC3561-CACHED-REPLY, RFC3561-GRATUITOUS.

## AODV-F-DISCOVERY

Demand-driven expanding-ring discovery makes bounded progress for finite workload.

Source: RFC3561-DISCOVERY.

## AODV-F-LINK

Correlate unidirectional validation and connectivity observations with the affected neighbor.

Source: RFC3561-ACK, RFC3561-HELLO.

## AODV-F-EXTENSION

Parse appended extensions without silently skipping mandatory information.

Source: [RFC3561-EXTENSION](../../standard/rfc3561/catalog.md#rfc3561-extension).

## AODV-F-HELLO-INTERVAL

Advertise a Hello interval so neighboring routers can use different transmission intervals.

Source: [RFC3561-HELLO-INTERVAL](../../standard/rfc3561/catalog.md#rfc3561-hello-interval).

## AODV-F-ERROR

Propagate ordinary errors and nondeleting notifications to affected precursors.

Source: RFC3561-ERROR.

## AODV-F-RECOVERY

Suppress premature forwarding and advertisements after sequence loss.

Source: RFC3561-REBOOT.

## AODV-F-INTERFACE

Operate across selected ad hoc links using one originator identity.

Source: RFC3561-INTERFACE.

## AODV-F-SUBNET

Receive, select and forward subnet information without conflating counter domains.

Source: RFC3561-SUBNET.

## AODV-F-REPAIR

Optionally repair a broken path before reporting final failure.

Source: RFC3561-REPAIR.
