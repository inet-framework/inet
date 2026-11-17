# 6LoWPAN — standards family and capability scope

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

6LoWPAN is a family of specifications. This map pins the eight documents selected for
the core adaptation layer, optimized Neighbor Discovery, and advanced dispatch and
fragmentation capabilities. It records standards scope; it makes no implementation or
test-coverage claim. No test pass or test-depth target is established by this inventory.

## Capability levels

These cumulative levels describe implementation scope. They are local planning labels,
not IETF conformance profiles and not the workflow's
[levels of test depth](../../../guide/derive-tests-from-a-standard.md#levels-of-depth).

| Capability level | Documents added | Scope |
| --- | --- | --- |
| 1 — Core | RFC 4944 + RFC 6282 | Adaptation-layer dispatch, IPv6 transmission, mesh addressing, fragmentation and reassembly, modern IPv6 and next-header compression |
| 2 — Neighbor Discovery | RFC 6775 + RFC 8505 | Optimized Neighbor Discovery and extended address registration |
| 3 — Advanced | RFC 8025 + RFC 8066 + RFC 8930 + RFC 8931 | Paging and ESC dispatch extensions, fragment forwarding and selective fragment recovery |

For the core, use **LOWPAN_IPHC and LOWPAN_NHC from RFC 6282** for modern compression.
RFC 4944's HC1 mechanism is a legacy format, not the compression baseline of this scope.
RFC 6282 updates RFC 4944; it does not obsolete the adaptation-layer specification as
a whole. Its compression formats are in §§3–4, while RFC 4944 §5 supplies the original
dispatch, mesh and fragmentation framework.

RFC 6775 already introduces address registration. RFC 8505 extends that registration
model; include both when modeling optimized 6LoWPAN Neighbor Discovery. Compression and
fragmentation alone do not establish support for those network-management procedures.

RFC 8930 addresses forwarding fragments across a route-over network using virtual
reassembly buffers. RFC 8931 adds selective recovery and congestion control; it updates
RFC 4944 and does not belong to the basic fragmentation capability by implication.

## Document list and sources

Titles, publication dates and relationships are from the linked RFC Editor records,
consulted 2026-09-18. All eight documents are Proposed Standards. RFC numbers identify
the published versions; later updates are separate documents.

| Document and official record | Title | Published | Relationship within the selected family | Original text |
| --- | --- | --- | --- | --- |
| [RFC 4944](https://www.rfc-editor.org/info/rfc4944) | Transmission of IPv6 Packets over IEEE 802.15.4 Networks | September 2007 | `base` | [Text](https://www.rfc-editor.org/rfc/rfc4944.txt) |
| [RFC 6282](https://www.rfc-editor.org/info/rfc6282) | Compression Format for IPv6 Datagrams over IEEE 802.15.4-Based Networks | September 2011 | `updates` RFC 4944 | [Text](https://www.rfc-editor.org/rfc/rfc6282.txt) |
| [RFC 6775](https://www.rfc-editor.org/info/rfc6775) | Neighbor Discovery Optimization for IPv6 over Low-Power Wireless Personal Area Networks (6LoWPANs) | November 2012 | `updates` RFC 4944; optimized Neighbor Discovery companion | [Text](https://www.rfc-editor.org/rfc/rfc6775.txt) |
| [RFC 8505](https://www.rfc-editor.org/info/rfc8505) | Registration Extensions for IPv6 over Low-Power Wireless Personal Area Network (6LoWPAN) Neighbor Discovery | November 2018 | `updates` RFC 6775 | [Text](https://www.rfc-editor.org/rfc/rfc8505.txt) |
| [RFC 8025](https://www.rfc-editor.org/info/rfc8025) | IPv6 over Low-Power Wireless Personal Area Network (6LoWPAN) Paging Dispatch | November 2016 | `updates` RFC 4944 | [Text](https://www.rfc-editor.org/rfc/rfc8025.txt) |
| [RFC 8066](https://www.rfc-editor.org/info/rfc8066) | IPv6 over Low-Power Wireless Personal Area Network (6LoWPAN) ESC Dispatch Code Points and Guidelines | February 2017 | `updates` RFC 4944 and RFC 6282 | [Text](https://www.rfc-editor.org/rfc/rfc8066.txt) |
| [RFC 8930](https://www.rfc-editor.org/info/rfc8930) | On Forwarding 6LoWPAN Fragments over a Multi-Hop IPv6 Network | November 2020 | `companion`; fragment forwarding using RFC 4944, no formal Updates relationship | [Text](https://www.rfc-editor.org/rfc/rfc8930.txt) |
| [RFC 8931](https://www.rfc-editor.org/info/rfc8931) | IPv6 over Low-Power Wireless Personal Area Network (6LoWPAN) Selective Fragment Recovery | November 2020 | `updates` RFC 4944 | [Text](https://www.rfc-editor.org/rfc/rfc8931.txt) |

**Cache status:** all eight original texts were downloaded from the RFC Editor URLs
above on 2026-09-18 and are preserved unmodified in the per-document folders linked
below. Cite sections and line numbers in these cached files. Each companion catalog
selects checkable statements and lists its exclusions; none claims exhaustive coverage
or an achieved test-depth level. Protocol checks and execution evidence remain separate.

| Document | Cached text | Statement catalog |
| --- | --- | --- |
| RFC 4944 | [rfc4944.txt](../../standard/rfc4944/rfc4944.txt) | [RFC 4944 catalog](../../standard/rfc4944/catalog.md) |
| RFC 6282 | [rfc6282.txt](../../standard/rfc6282/rfc6282.txt) | [RFC 6282 catalog](../../standard/rfc6282/catalog.md) |
| RFC 6775 | [rfc6775.txt](../../standard/rfc6775/rfc6775.txt) | [RFC 6775 catalog](../../standard/rfc6775/catalog.md) |
| RFC 8505 | [rfc8505.txt](../../standard/rfc8505/rfc8505.txt) | [RFC 8505 catalog](../../standard/rfc8505/catalog.md) |
| RFC 8025 | [rfc8025.txt](../../standard/rfc8025/rfc8025.txt) | [RFC 8025 catalog](../../standard/rfc8025/catalog.md) |
| RFC 8066 | [rfc8066.txt](../../standard/rfc8066/rfc8066.txt) | [RFC 8066 catalog](../../standard/rfc8066/catalog.md) |
| RFC 8930 | [rfc8930.txt](../../standard/rfc8930/rfc8930.txt) | [RFC 8930 catalog](../../standard/rfc8930/catalog.md) |
| RFC 8931 | [rfc8931.txt](../../standard/rfc8931/rfc8931.txt) | [RFC 8931 catalog](../../standard/rfc8931/catalog.md) |

## Governing documents

| Area | Original provision | Governing document for the selected capability |
| --- | --- | --- |
| IPv6 and next-header compression | RFC 4944 §10, HC1 and HC2 | RFC 6282 §§3–4, LOWPAN_IPHC and LOWPAN_NHC, for the modern compression baseline |
| Basic dispatch and ESC allocation | RFC 4944 §5.1 | RFC 6282 §5 updates the dispatch allocation; RFC 8066 defines ESC extension code points and usage |
| Dispatch pages | RFC 4944 §5.1 | RFC 8025 supplies paging when that extension is selected |
| Address registration | RFC 6775 | RFC 8505 governs its extended registration procedures |
| Original fragmentation and reassembly | RFC 4944 §5.3 | RFC 4944 remains the core; RFC 8930 adds forwarding guidance, and RFC 8931 governs selective recovery when selected |

The catalog-level override links below retain the original statements and identify the
later governing rules. Extended registration rules apply to extended-capable peers;
RFC 8505 §6 determines behavior with base-only peers.

| Area | Original catalog statement | Later governing statement |
| --- | --- | --- |
| Legacy compression transmission | [RFC4944-LEGACY-1](../../standard/rfc4944/catalog.md#rfc4944-legacy-1), §10.3.1 | [RFC6282-LEGACY-1](../../standard/rfc6282/catalog.md#rfc6282-legacy-1), §2; the old layout remains relevant to optional legacy decoding |
| Replacement ESC definition | [RFC6282-DISPATCH-1](../../standard/rfc6282/catalog.md#rfc6282-dispatch-1), §5 | [RFC8066-ESC-1](../../standard/rfc8066/catalog.md#rfc8066-esc-1), §3 |
| Registered address location | [RFC6775-ARO-1](../../standard/rfc6775/catalog.md#rfc6775-aro-1), §4.1 | [RFC8505-ADDR-2](../../standard/rfc8505/catalog.md#rfc8505-addr-2), §5.5 |
| Registration option length | [RFC6775-ARO-2](../../standard/rfc6775/catalog.md#rfc6775-aro-2), §6.5 | [RFC8505-EARO-3](../../standard/rfc8505/catalog.md#rfc8505-earo-3), §5.1; Length two is the backward-compatible form |
| Registration ownership identifier | [RFC6775-ARO-4](../../standard/rfc6775/catalog.md#rfc6775-aro-4), §6.5.1 | [RFC8505-ROVR-1](../../standard/rfc8505/catalog.md#rfc8505-rovr-1), §5.3; extended recency additionally depends on §5.2 |

The [RFC 4944 size](../../standard/rfc4944/catalog.md#rfc4944-frag-3) and
[offset](../../standard/rfc4944/catalog.md#rfc4944-frag-5) rules remain valid for its
original fragment format. [RFC8931-FRAG-3](../../standard/rfc8931/catalog.md#rfc8931-frag-3)
uses compressed-datagram coordinates for the separate RFRAG format; it does not change
the meaning of those fields in an RFC 4944 frame.

Advanced extensions do not make their entire feature set mandatory for the core scope.
Dispatch allocation updates still matter when interpreting received dispatch values.

## Scope boundary

The eight published documents above are the selected reference set. It is not an
exhaustive dependency closure or a claim to implement every subsequent update. The
[IPv6 standards map](../ipv6/standards.md) supplies the related IPv6 family; general
Neighbor Discovery and address autoconfiguration also require RFC 4861 and RFC 4862.
IEEE 802.15.4 link behavior requires a separately pinned IEEE edition.

Specialized extensions and later updates outside this selection need an explicit scope
decision before a test pass. In particular, the RFC 8505 record lists further updates;
selecting RFC 8505 here does not implicitly select all of them. Before deriving tests,
check applicable updates and errata, pin the clauses under test, extend the catalogs
where the selected scope requires it, and derive protocol checks using the linked workflow.
