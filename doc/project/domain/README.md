# Domain rules

> **Kind:** reference · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rule/architecture.md](../rule/architecture.md)

One document per protocol family that carries extra risk. A domain document holds the rules that
apply **in addition to** everything in [rule/](../rule/architecture.md), inside a named set of paths.

## The documents

| Document | Prefix | Covers |
| --- | --- | --- |
| [ieee80211.md](ieee80211.md) | `AR-WLAN-*` | `src/inet/linklayer/ieee80211/`, `src/inet/physicallayer/wireless/ieee80211/` |

## What a domain document holds

1. **The paths it covers.** A rule with no scope is a rule nobody can apply.
2. **The rules that apply there beyond the general ones**, with identifiers `AR-<DOMAIN>-<AREA>-<NAME>`.
3. **A link to its review checklist** under [enforcement/checklist/](../enforcement/README.md).
4. **What it does *not* restate.** Where a general rule already covers a point, the domain rule says
   what *more* is required — it never repeats ([DR-CITE-DONT-REPEAT](../rule/documentation.md#dr-cite-dont-repeat)).

## When a family earns one

A domain document is a cost: another place to look, and another set of identifiers. A family earns
one when it concentrates a risk that the general rules cannot express. IEEE 802.11 is the first
because four risks meet there — dense normative standards text, shared-medium timing measured in
microseconds, a large space of PHY and MAC variants across amendments, and protocol state whose
corruption produces plausible-looking but wrong results.

The likely next ones are TSN, for the same standards-density reason, and the physical layer, for the
fidelity-level reason. Neither exists yet, and neither should until a review keeps finding the same
class of defect that no general rule catches.
