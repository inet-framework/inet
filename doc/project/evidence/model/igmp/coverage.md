# IGMP — coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/igmp/standards.md), [conformance.md](conformance.md)

The single place that holds the changing state of the IGMP workflow. Every other artifact of
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
[`standards.md`](../../protocol/igmp/standards.md#target-level).

The exit criterion of level 1 has three parts, and all three hold:

| Part of the criterion | State | Evidence |
| --- | --- | --- |
| The standards map pins an in-scope set | holds | RFC 9776 §4-§8, RFC 2236 §6-§7, chosen from the register: [`standards.md`](../../protocol/igmp/standards.md#in-scope-set) |
| Every model claim is mapped onto the map | holds | each of the 11 places in the model that name a document has a row in [`conformance.md`](conformance.md#part-1--the-claims) |
| Every obsolete claim is named | holds | one: `Igmpv3.ned:13` names RFC 3376, which RFC 9776 obsoleted in March 2025 |

| Level | State | What it needs |
| --- | --- | --- |
| 1, Survey | **reached** | — |
| 2, Core | not started | the catalogs of RFC 9776 (§4-§7) and RFC 2236 (§6-§7); the feature map; the checks; a querier and one or two hosts on one LAN |
| 3, Edge | not started | the unrecognized-message-type rule (RFC 9776:482) against a crafted type; the model currently crashes instead of discarding |
| 4, Dynamics | not started | RFC 9776 §8 in full, including the two formulas that changed from RFC 2236/RFC 3376 (Group Membership Interval, Older Version Querier Present Interval), each with a tolerance |
| 5, Complete | not started | RFC 1112 (no module exists), RFC 4604 (SSM-aware behavior, unclaimed) |

## Pass log

| Pass | Date | Level | Scope | Result |
| --- | --- | --- | --- | --- |
| 1 | 2026-09-23 | **1, reached** | RFC 9776 and RFC 2236 downloaded; the standards map; the claims of the model | no run; one obsolete claim (RFC 3376, claimed at `Igmpv3.ned:13`); two RFC 9776 formula changes found relative to the model's defaults (Group Membership Interval, Older Version Querier Present Interval); the unrecognized-message-type rule went from a lower-case "should" to a formal MUST between RFC 2236 and RFC 9776, against code that already crashes on the case |
