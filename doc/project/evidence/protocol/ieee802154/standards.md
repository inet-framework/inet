# IEEE 802.15.4 — standards family and extraction scope

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

## In-scope set

The in-scope set pins **IEEE Std 802.15.4-2024, IEEE Standard for Low-Rate Wireless
Networks**, without later amendments. Its scope includes the MAC and the PHY variants defined
by that edition. The family-wide survey is complemented by a bounded legacy-data/O-QPSK requirement extraction. The [source record](../../standard/ieee802154/source.md) identifies the exact PDF.
The versionless document folder follows the naming convention in the derivation guide.

The extraction targets normal data/ACK operation plus negative-input and timing requirements for
nonperiodic-beacon O-QPSK operation. This is preparation toward Levels 2–4 for those mechanisms,
not a complete Level 2 catalog of the base standard. The [catalog](../../standard/ieee802154/catalog.md),
[features](features.md) and [checks](checks.md) state the current extraction boundaries. Achieved
levels and execution results remain in the model ledger; the wider survey scope is retained.

## Family

IEEE SA register metadata retrieved 2026-09-18; the 2024 PDF was downloaded on that date.
Only the 2024 text is locally available for this survey. Other entries below were inspected
as metadata, not downloaded as full texts.

| Document | Relationship and date | Scope decision and source |
| --- | --- | --- |
| IEEE 802.15.4-2024 | Base; approved 2024-09-26, published 2024-12-12; supersedes 2020 | In scope; [IEEE SA register](https://standards.ieee.org/ieee/802.15.4/11041/) |
| IEEE 802.15.4-2020 | Previous base, superseded by 2024 | Historical comparison only; [register](https://standards.ieee.org/ieee/802.15.4/7029/) |
| IEEE 802.15.4-2006 | Earlier base edition | Historical comparison only; [register](https://standards.ieee.org/ieee/802.15.4/3582/) |
| IEEE 802.15.4a-2007 | Amendment 1: Add Alternate PHYs; superseded | Historical PHY context only; [register](https://standards.ieee.org/ieee/802.15.4a/3571/) |
| IEEE 802.15.4ae-2026 | Amendment 1: Ascon Cryptographic Algorithms | Outside the pinned base-only set; [register](https://standards.ieee.org/ieee/802.15.4ae/11836/) |
| IEEE 802.15.4ac-2026 | Amendment 2: Privacy Enhancements | Outside the pinned base-only set; [register](https://standards.ieee.org/ieee/802.15.4ac/11272/) |
| P802.15.4-2024/Cor 1 | Proposed corrigendum, active PAR | Not an adopted override in this set; [register](https://standards.ieee.org/ieee/802.15.4-2024_Cor_1/11994/) |
| P802.15.4ad | Proposed SUN PHY extension | Outside scope; [register](https://standards.ieee.org/ieee/802.15.4ad/11593/) |
| P802.15.4ab | Proposed enhanced UWB amendment listed by IEEE SA | Outside scope; [2024 register](https://standards.ieee.org/ieee/802.15.4/11041/) |

The informative introduction to the 2024 edition (physical PDF pages 18–19) describes the
2003, 2006, 2011, 2015, and 2020 lineage and the integration of amendments into revisions.
It is historical context, not a substitute for the normative text of those editions.
This is a bounded family map for the pinned survey, not an exhaustive catalog of every
historical amendment or normative reference in clause 2.

## Clause mapping and overrides

No amendment is applied to the pinned 2024 text; there are no clause-level overrides in this
set. Clauses 6 and 7 cover MAC behavior and formats; clause 8 covers MAC services (including
constants and PIB attributes in 8.4), clause 9 security, clause 10 optional features, clause 11 general PHY
requirements, clause 12 PHY services, clause 13 O-QPSK, and clause 16 HRP UWB. Other PHYs
and optional mechanisms remain part of the base document, not automatically part of any
implementation claim.

No clause-equivalence assertion between 2006/2007 and 2024 is made without the older texts.
The model-side survey is [conformance.md](../../model/ieee802154/conformance.md).

## Imported addressing reference

IEEE Std 802-2024 supplies the group-address definition referenced by base clause 2 and
6.6.2. Its [source identity](../../standard/ieee802/source.md) and
[address catalog](../../standard/ieee802/catalog.md) are separate from the IEEE 802.15.4
catalog. This resolves the bounded address-definition dependency without changing the selected
IEEE 802.15.4 edition or importing an unrelated PHY/MAC conformance claim.
