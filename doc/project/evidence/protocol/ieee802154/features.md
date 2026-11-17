# IEEE 802.15.4 — selected feature map

> **Kind:** what · **Status:** draft · **Seal:** none · **Owns:** IEEE802154-F-* · **Stands on:** [standards.md](standards.md), [catalog.md](../../standard/ieee802154/catalog.md)

This map covers the current bounded catalog. Requirement levels apply only when the listed
catalog predicates hold; they do not require every optional operating mode on every device.
There is no implementation-support verdict in this document.

## Index

| Feature | Level and reason | Catalog areas | Procedure |
| --- | --- | --- | --- |
| [IEEE802154-F-WIRE](#ieee802154-f-wire) | mandatory (conditional keyword/only path) | ADDRESS, WIRE | [IEEE802154-C-WIRE](checks.md#ieee802154-c-wire) |
| [IEEE802154-F-ACCESS](#ieee802154-f-access) | mandatory (conditional keyword/only path) | ACCESS | [IEEE802154-C-ACCESS](checks.md#ieee802154-c-access) |
| [IEEE802154-F-SEQUENCE](#ieee802154-f-sequence) | mandatory (conditional keyword/only path) | SEQUENCE | [IEEE802154-C-SEQUENCE](checks.md#ieee802154-c-sequence) |
| [IEEE802154-F-RECEIVE](#ieee802154-f-receive) | mandatory (conditional keyword/only path) | RECEIVE | [IEEE802154-C-RECEIVE](checks.md#ieee802154-c-receive) |
| [IEEE802154-F-ACK](#ieee802154-f-ack) | mandatory (conditional keyword/only path) | ACK | [IEEE802154-C-ACK](checks.md#ieee802154-c-ack) |
| [IEEE802154-F-SECURITY](#ieee802154-f-security) | mandatory (conditional keyword/only path) | SECURITY | [IEEE802154-C-SECURITY](checks.md#ieee802154-c-security) |
| [IEEE802154-F-PHY](#ieee802154-f-phy) | mandatory (conditional keyword/only path) | PHY | [IEEE802154-C-PHY](checks.md#ieee802154-c-phy) |
| [IEEE802154-F-SERVICE](#ieee802154-f-service) | mandatory (conditional keyword/only path) | SERVICE, PIB | [IEEE802154-C-SERVICE](checks.md#ieee802154-c-service) |
| [IEEE802154-F-SCAN](#ieee802154-f-scan) | mandatory (conditional keyword/only path) | SCAN | [IEEE802154-C-SCAN](checks.md#ieee802154-c-scan) |

## IEEE802154-F-WIRE

Imported definitions: [IEEE802-ADDRESS-1](../../standard/ieee802/catalog.md#ieee802-address-1), [IEEE802-ADDRESS-2](../../standard/ieee802/catalog.md#ieee802-address-2).

**Native address and legacy data/ACK encoding.**

Level: **mandatory under the catalog conditions**. At least one core source uses “shall”; exact excerpts and conditional predicates are in the catalog. Governing source: IEEE Std 802.15.4-2024, no overrides.

Core statements: [IEEE802154-ADDRESS-1](../../standard/ieee802154/catalog.md#ieee802154-address-1), [IEEE802154-ADDRESS-2](../../standard/ieee802154/catalog.md#ieee802154-address-2), [IEEE802154-WIRE-1](../../standard/ieee802154/catalog.md#ieee802154-wire-1), [IEEE802154-WIRE-2](../../standard/ieee802154/catalog.md#ieee802154-wire-2), [IEEE802154-WIRE-3](../../standard/ieee802154/catalog.md#ieee802154-wire-3), [IEEE802154-WIRE-4](../../standard/ieee802154/catalog.md#ieee802154-wire-4), [IEEE802154-WIRE-5](../../standard/ieee802154/catalog.md#ieee802154-wire-5), [IEEE802154-WIRE-6](../../standard/ieee802154/catalog.md#ieee802154-wire-6).

Additional core statements under their stated conditions: [IEEE802154-WIRE-7](../../standard/ieee802154/catalog.md#ieee802154-wire-7), [IEEE802154-WIRE-8](../../standard/ieee802154/catalog.md#ieee802154-wire-8), [IEEE802154-WIRE-9](../../standard/ieee802154/catalog.md#ieee802154-wire-9), [IEEE802154-WIRE-10](../../standard/ieee802154/catalog.md#ieee802154-wire-10), [IEEE802154-WIRE-11](../../standard/ieee802154/catalog.md#ieee802154-wire-11), [IEEE802154-WIRE-12](../../standard/ieee802154/catalog.md#ieee802154-wire-12), [IEEE802154-WIRE-13](../../standard/ieee802154/catalog.md#ieee802154-wire-13), [IEEE802154-WIRE-14](../../standard/ieee802154/catalog.md#ieee802154-wire-14), [IEEE802154-WIRE-15](../../standard/ieee802154/catalog.md#ieee802154-wire-15), [IEEE802154-WIRE-16](../../standard/ieee802154/catalog.md#ieee802154-wire-16).

Procedure: [IEEE802154-C-WIRE](checks.md#ieee802154-c-wire).

Additional core statement: [IEEE802154-WIRE-17](../../standard/ieee802154/catalog.md#ieee802154-wire-17).

## IEEE802154-F-ACCESS

Interframe spacing: [IEEE802154-ACCESS-3](../../standard/ieee802154/catalog.md#ieee802154-access-3).

**Unslotted channel access.**

Level: **mandatory under the catalog conditions**. At least one core source uses “shall”; exact excerpts and conditional predicates are in the catalog. Governing source: IEEE Std 802.15.4-2024, no overrides.

Core statements: [IEEE802154-ACCESS-1](../../standard/ieee802154/catalog.md#ieee802154-access-1), [IEEE802154-ACCESS-2](../../standard/ieee802154/catalog.md#ieee802154-access-2).

Procedures: [IEEE802154-C-ACCESS](checks.md#ieee802154-c-access), plus the spacing variant in
[IEEE802154-C-ACK](checks.md#ieee802154-c-ack).

## IEEE802154-F-SEQUENCE

**Device-wide sequence allocation and wrap.**

Level: **mandatory under the catalog conditions**. At least one core source uses “shall”; exact excerpts and conditional predicates are in the catalog. Governing source: IEEE Std 802.15.4-2024, no overrides.

Core statements: [IEEE802154-SEQUENCE-1](../../standard/ieee802154/catalog.md#ieee802154-sequence-1), [IEEE802154-SEQUENCE-2](../../standard/ieee802154/catalog.md#ieee802154-sequence-2).

Procedure: [IEEE802154-C-SEQUENCE](checks.md#ieee802154-c-sequence).

## IEEE802154-F-RECEIVE

**Filtering, immediate ACK eligibility and data indication.**

Level: **mandatory under the catalog conditions**. At least one core source uses “shall”; exact excerpts and conditional predicates are in the catalog. Governing source: IEEE Std 802.15.4-2024, no overrides.

Core statements: [IEEE802154-RECEIVE-1](../../standard/ieee802154/catalog.md#ieee802154-receive-1), [IEEE802154-RECEIVE-2](../../standard/ieee802154/catalog.md#ieee802154-receive-2), [IEEE802154-RECEIVE-3](../../standard/ieee802154/catalog.md#ieee802154-receive-3), [IEEE802154-RECEIVE-4](../../standard/ieee802154/catalog.md#ieee802154-receive-4).

Additional core statements under their stated conditions: [IEEE802154-RECEIVE-5](../../standard/ieee802154/catalog.md#ieee802154-receive-5), [IEEE802154-RECEIVE-6](../../standard/ieee802154/catalog.md#ieee802154-receive-6), [IEEE802154-RECEIVE-7](../../standard/ieee802154/catalog.md#ieee802154-receive-7), [IEEE802154-RECEIVE-8](../../standard/ieee802154/catalog.md#ieee802154-receive-8), [IEEE802154-RECEIVE-9](../../standard/ieee802154/catalog.md#ieee802154-receive-9), [IEEE802154-RECEIVE-10](../../standard/ieee802154/catalog.md#ieee802154-receive-10), [IEEE802154-RECEIVE-11](../../standard/ieee802154/catalog.md#ieee802154-receive-11), [IEEE802154-RECEIVE-12](../../standard/ieee802154/catalog.md#ieee802154-receive-12), [IEEE802154-RECEIVE-13](../../standard/ieee802154/catalog.md#ieee802154-receive-13), [IEEE802154-RECEIVE-14](../../standard/ieee802154/catalog.md#ieee802154-receive-14), [IEEE802154-RECEIVE-15](../../standard/ieee802154/catalog.md#ieee802154-receive-15).

Procedure: [IEEE802154-C-RECEIVE](checks.md#ieee802154-c-receive).

Additional core statement: [IEEE802154-RECEIVE-16](../../standard/ieee802154/catalog.md#ieee802154-receive-16).

## IEEE802154-F-ACK

**Acknowledgment and direct/indirect retry distinction.**

Level: **mandatory under the catalog conditions**. At least one core source uses “shall”; exact excerpts and conditional predicates are in the catalog. Governing source: IEEE Std 802.15.4-2024, no overrides.

Core statements: [IEEE802154-ACK-1](../../standard/ieee802154/catalog.md#ieee802154-ack-1), [IEEE802154-ACK-2](../../standard/ieee802154/catalog.md#ieee802154-ack-2), [IEEE802154-ACK-3](../../standard/ieee802154/catalog.md#ieee802154-ack-3), [IEEE802154-ACK-4](../../standard/ieee802154/catalog.md#ieee802154-ack-4), [IEEE802154-ACK-5](../../standard/ieee802154/catalog.md#ieee802154-ack-5).

Procedure: [IEEE802154-C-ACK](checks.md#ieee802154-c-ack).

## IEEE802154-F-SECURITY

Service routing: [IEEE802154-SERVICE-8](../../standard/ieee802154/catalog.md#ieee802154-service-8).

**Unsecured-profile security outcomes.**

Level: **mandatory under the catalog conditions**. At least one core source uses “shall”; exact excerpts and conditional predicates are in the catalog. Governing source: IEEE Std 802.15.4-2024, no overrides.

Core statements: [IEEE802154-SECURITY-1](../../standard/ieee802154/catalog.md#ieee802154-security-1), [IEEE802154-SECURITY-2](../../standard/ieee802154/catalog.md#ieee802154-security-2), [IEEE802154-SECURITY-3](../../standard/ieee802154/catalog.md#ieee802154-security-3), [IEEE802154-SECURITY-4](../../standard/ieee802154/catalog.md#ieee802154-security-4), [IEEE802154-SECURITY-5](../../standard/ieee802154/catalog.md#ieee802154-security-5).

Procedure: [IEEE802154-C-SECURITY](checks.md#ieee802154-c-security).

## IEEE802154-F-PHY

**O-QPSK timing, CCA, ED and LQI.**

Level: **mandatory under the catalog conditions**. At least one core source uses “shall”; exact excerpts and conditional predicates are in the catalog. Governing source: IEEE Std 802.15.4-2024, no overrides.

Core statements: [IEEE802154-PHY-1](../../standard/ieee802154/catalog.md#ieee802154-phy-1), [IEEE802154-PHY-2](../../standard/ieee802154/catalog.md#ieee802154-phy-2), [IEEE802154-PHY-3](../../standard/ieee802154/catalog.md#ieee802154-phy-3), [IEEE802154-PHY-4](../../standard/ieee802154/catalog.md#ieee802154-phy-4), [IEEE802154-PHY-5](../../standard/ieee802154/catalog.md#ieee802154-phy-5), [IEEE802154-PHY-6](../../standard/ieee802154/catalog.md#ieee802154-phy-6), [IEEE802154-PHY-7](../../standard/ieee802154/catalog.md#ieee802154-phy-7), [IEEE802154-PHY-8](../../standard/ieee802154/catalog.md#ieee802154-phy-8).

Additional core statements under their stated conditions: [IEEE802154-PHY-9](../../standard/ieee802154/catalog.md#ieee802154-phy-9), [IEEE802154-PHY-10](../../standard/ieee802154/catalog.md#ieee802154-phy-10), [IEEE802154-PHY-11](../../standard/ieee802154/catalog.md#ieee802154-phy-11), [IEEE802154-PHY-12](../../standard/ieee802154/catalog.md#ieee802154-phy-12), [IEEE802154-PHY-13](../../standard/ieee802154/catalog.md#ieee802154-phy-13), [IEEE802154-PHY-14](../../standard/ieee802154/catalog.md#ieee802154-phy-14), [IEEE802154-PHY-15](../../standard/ieee802154/catalog.md#ieee802154-phy-15).

Procedure: [IEEE802154-C-PHY](checks.md#ieee802154-c-phy).

## IEEE802154-F-SERVICE

Parameter support: [IEEE802154-SERVICE-9](../../standard/ieee802154/catalog.md#ieee802154-service-9).

**PIB access, reset and request capacity.**

Level: **mandatory under the catalog conditions**. Required service outcomes; the oversize-frame statement uses “shall”. Governing source: IEEE Std 802.15.4-2024, no overrides.

Core statements: [IEEE802154-SERVICE-1](../../standard/ieee802154/catalog.md#ieee802154-service-1), [IEEE802154-PIB-1](../../standard/ieee802154/catalog.md#ieee802154-pib-1), [IEEE802154-PIB-2](../../standard/ieee802154/catalog.md#ieee802154-pib-2), [IEEE802154-PIB-3](../../standard/ieee802154/catalog.md#ieee802154-pib-3).

Additional core statements under their stated conditions: [IEEE802154-PIB-4](../../standard/ieee802154/catalog.md#ieee802154-pib-4), [IEEE802154-PIB-5](../../standard/ieee802154/catalog.md#ieee802154-pib-5), [IEEE802154-PIB-6](../../standard/ieee802154/catalog.md#ieee802154-pib-6), [IEEE802154-PIB-7](../../standard/ieee802154/catalog.md#ieee802154-pib-7), [IEEE802154-PIB-8](../../standard/ieee802154/catalog.md#ieee802154-pib-8), [IEEE802154-PIB-9](../../standard/ieee802154/catalog.md#ieee802154-pib-9), [IEEE802154-SERVICE-2](../../standard/ieee802154/catalog.md#ieee802154-service-2), [IEEE802154-SERVICE-3](../../standard/ieee802154/catalog.md#ieee802154-service-3), [IEEE802154-SERVICE-4](../../standard/ieee802154/catalog.md#ieee802154-service-4), [IEEE802154-SERVICE-5](../../standard/ieee802154/catalog.md#ieee802154-service-5), [IEEE802154-SERVICE-6](../../standard/ieee802154/catalog.md#ieee802154-service-6), [IEEE802154-SERVICE-7](../../standard/ieee802154/catalog.md#ieee802154-service-7).

Procedure: [IEEE802154-C-SERVICE](checks.md#ieee802154-c-service).

## IEEE802154-F-SCAN

**Passive scan obligation and channel order.**

Level: **mandatory under the catalog conditions**. At least one core source uses “shall”; exact excerpts and conditional predicates are in the catalog. Governing source: IEEE Std 802.15.4-2024, no overrides.

Core statements: [IEEE802154-SCAN-1](../../standard/ieee802154/catalog.md#ieee802154-scan-1), [IEEE802154-SCAN-2](../../standard/ieee802154/catalog.md#ieee802154-scan-2).

Procedure: [IEEE802154-C-SCAN](checks.md#ieee802154-c-scan).

## Boundary

Every current catalog area appears above. This map does not exhaust the base standard; the
[catalog extraction boundary](../../standard/ieee802154/catalog.md#extraction-boundary) defines its limits.
