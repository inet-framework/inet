# MLD — coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/mld/standards.md), [conformance.md](conformance.md)

The single place that holds the changing state of the MLD workflow. Every other artifact of
this protocol states what a standard says and never changes again unless the standard
changes. This one changes on every pass.

State of the ledger, from a level 1 pass — no build, no run:

- Date: 2026-09-23
- INET: branch `topic/standards-tests-wave0`, commit `e360ca980e`, tree clean
- Trees: src `16dc528e10`, tests/protocol `6f0a6bdb05`
- Target level: 1

No catalog, no feature map and no test exist yet, so the ledger has no statement table and no
feature table.

## Achieved level

**Level 1, reached.** Target: level 1, from
[`standards.md`](../../protocol/mld/standards.md#target-level).

The exit criterion of level 1 has three parts, and all three hold:

| Part of the criterion | State | Evidence |
| --- | --- | --- |
| The standards map pins an in-scope set | holds | RFC 9777 §5-§9, RFC 2710 §5-§7, chosen from the register: [`standards.md`](../../protocol/mld/standards.md#in-scope-set) |
| Every model claim is mapped onto the map | holds | each of the 13 places in the model that name a document has a row in [`conformance.md`](conformance.md#part-1--the-claims) |
| Every obsolete claim is named | holds | one: `Mldv2.ned:12` names RFC 3810, which RFC 9777 obsoleted in March 2025 |

| Level | State | What it needs |
| --- | --- | --- |
| 1, Survey | **reached** | — |
| 2, Core | not started | the catalogs of RFC 9777 (§5-§8) and RFC 2710 (§5-§7); the feature map; the checks; one shared link, a querier and one or two listener hosts |
| 3, Edge | not started | the unrecognized-message-type rule (RFC 9777:765) against a crafted type; `Mldv1` crashes on this case today, `Mldv2` already discards it |
| 4, Dynamics | not started | RFC 9777 §9 in full, including the one formula that changed from RFC 2710/RFC 3810 (Multicast Address Listening Interval), with a tolerance |
| 5, Complete | not started | RFC 4604 (SSM-aware behavior, unclaimed), RFC 3590 (source-address selection, unclaimed), querier election and MLD snooping (both stated as absent by the model) |

## Pass log

| Pass | Date | Level | Scope | Result |
| --- | --- | --- | --- | --- |
| 1 | 2026-09-23 | **1, reached** | RFC 9777 and RFC 2710 downloaded; the standards map; the claims of the model | no run; one obsolete claim (RFC 3810, claimed at `Mldv2.ned:12`); one RFC 9777 formula change found relative to the model's default (Multicast Address Listening Interval, shared with the same gap in IGMP); the `Mldv2.ned` "Parity gaps" comment found stale on all three of its own points, not the one already known; `Mldv1` still crashes on an unrecognized message type where `Mldv2` already discards it silently |
