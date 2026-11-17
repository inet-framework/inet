# Selected MAC PIB mutation contract

> **Kind:** design · **Status:** implemented; standalone evidence · **Seal:** none · **Owns:** — · **Stands on:** [applicability.md](applicability.md), [catalog.md](../../standard/ieee802154/catalog.md#pib-field-domains)

Package 1b defines the store used by the future MAC service provider. This is not an operational
MLME implementation. The store owns validated values; the provider owns admission, request IDs,
active operations, notifications and application of changes to radio/access procedures. PHY PIB
attributes remain with the PHY owner. No existing MAC reads this store yet.

## Representation and attribute selection

The API reuses `Ieee802154MacSetRequest` for startup overrides and
`Ieee802154MacGetConfirm` for GET results, echoing the attribute name. The store produces only
IEEE status alternatives; local admission and abort outcomes belong to the provider.

The selected set contains 20 base MAC attributes, `macPromiscuousMode`, and the 22 functional
flags listed in the catalog. GET preserves the exact requested name. Integer values use the
`uint64_t` variant; SET accepts that representation or nonnegative `int64_t`, never an implicit
Boolean conversion. Boolean values require the Boolean variant. Extended addresses use the
native address variant. Short addresses and PAN identifiers remain integer PIB values.

| Attributes | Access and accepted values | Default/reset value |
| --- | --- | --- |
| macExtendedAddress | Upper-layer read-only; construction requires an individual extended address | Supplied device identity, retained across reset |
| macCoordExtendedAddress | Writable individual extended address | Native NONE denotes unspecified coordinator context; this local sentinel is not an address writable through SET |
| macShortAddress, macCoordShortAddress, macPanId | Writable integers 0–65535, including sentinel values | 65535 |
| macDsn | Writable integer 0–255 | Owner-supplied random draw at construction and each default reset |
| macMinBe, macMaxBe | Writable integers 0–current maximum and 3–8; both must remain consistent | 3, 5 |
| macMaxCsmaBackoffs, macMaxFrameRetries | Writable integers 0–5 and 0–7 | 4, 3 |
| macSifsPeriod, macLifsPeriod | Upper-layer read-only, in symbols | 12, 40 for selected O-QPSK |
| macUnitBackoffPeriod | Writable positive integer symbols, locally representable through UINT32_MAX | 12 + ceil(CCA duration in microseconds / 16) |
| macRxOnWhenIdle, macImplicitBroadcast, macGroupRxMode, macPromiscuousMode | Writable Boolean | false |
| macSecurityEnabled | Writable, selected profile accepts false only | false |
| macBeaconOrder | Writable, selected profile accepts 15 only | 15 |
| macTimestampSupported | Upper-layer read-only | false |
| macSyncSymbolOffset | Upper-layer read-only, symbols | Local unused value 0 while timestamps are unsupported |
| Table 8-37 capability flags | Upper-layer read-only | false for this implementation |
| Table 8-37 enabled flags, macLeHsEnabled, macTrleRelayingMode | Writable, selected profile accepts false only | false |

GET of unspecified coordinator context returns SUCCESS with native NONE as a local API
representation, not an IEEE-defined extended-address value or sentinel.

The source leaves several defaults unspecified. NONE, zero sync offset, and false capability
values above are declared implementation choices, not IEEE defaults. The backoff upper bound is
a local representation limit, not a range supplied by Table 8-36. SIFS/LIFS values follow clause
11.1.4, physical PDF page 593, `ieee802154-2024@2363122:2364053`.

Deferred base attributes (including scan/beacon, indirect and ranging state), security tables,
and PHY names are absent from this MAC-only surface. Their requests return UNSUPPORTED_ATTRIBUTE.
This does not resolve their deferred device-profile obligations or claim them inapplicable.

## Initialization and reset

Construction receives the immutable extended identity, an injected DSN default, and the selected
PHY's CCA duration in integer microseconds (1–1000000, default 128). O-QPSK uses 16 microseconds
per symbol and 12 turnaround symbols. Integer ceiling precedes the addition. CCA duration is
initialization context, not a duplicate mutable PHY PIB. No PHY mutation API is introduced here;
package 1c must coordinate any later profile change. An explicit backoff write is not silently
recomputed during another MAC write.

Startup overrides use the same value/access checks as SET. They are validated as a complete
candidate, including the final BE pair, independent of override order. Duplicate names and
invalid candidates fail construction with a configuration error. Future NED owners read their
parameters once into these inputs; no second runtime parameter copy is authoritative.

Reset with SetDefaultPib=false retains every stored value (Table 8-12, physical PDF page 123,
`ieee802154-2024@530664:531472`). Reset with true restores the table's
standard/profile defaults, using a fresh owner-supplied DSN; it does not replay startup overrides.
Device identity and immutable PHY context remain supplied defaults. The owner controls RNG stream
and draw timing; the store never draws randomness. PHY reset and aborting outstanding requests
are provider responsibilities, outside this store's reset operation.

## Mutation, failure and observability

All writable selected attributes can change at the store boundary after initialization. There
is no public batch SET; no current supported procedure requires one. Each SET first checks the
exact name, then read-only access, then type/domain/profile restrictions, then cross-attribute
invariants. These precedence rules for simultaneous errors are local API choices. Failure retains
all values. A successful same-value SET is still SUCCESS.

The store is synchronous and emits no signals or callbacks. The future provider must admit and
coordinate an operationally sensitive change before committing it, and confirm SUCCESS only
after its owning procedure has applied it. It compares previous and resulting values to emit
notifications for effective changes, after commit. Operations cannot observe partially validated
state. Until that provider exists, direct store tests establish only validation/default behavior,
not busy handling, radio effects, lifecycle barriers or operational MLME support.

Validation and independent review are recorded in [results.md](results.md#step-1b-selected-mac-pib).
