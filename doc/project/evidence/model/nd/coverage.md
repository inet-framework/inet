# ND — coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/nd/standards.md), [conformance.md](conformance.md)

The single place that holds the changing state of the ND workflow. Every other artifact of
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
[`standards.md`](../../protocol/nd/standards.md#target-level).

The exit criterion of level 1 has three parts, and all three hold:

| Part of the criterion | State | Evidence |
| --- | --- | --- |
| The standards map pins an in-scope set | holds | RFC 4861 §4, §6, §7.2, §8, RFC 4862 §5, RFC 5942 §6 and RFC 6980 §5, chosen from the register: [`standards.md`](../../protocol/nd/standards.md#in-scope-set) |
| Every model claim is mapped onto the map | holds | each of the 19 places in the model that name a document, or decline one, has a row in [`conformance.md`](conformance.md#part-1--the-claims) |
| Every obsolete claim is named | holds | two: RFC 2461 (the larger claim by line count) and RFC 2462, both in [`conformance.md`](conformance.md#part-1--the-claims) |

| Level | State | What it needs |
| --- | --- | --- |
| 1, Survey | **reached** | — |
| 2, Core | not started | the catalogs of RFC 4861 (§4, §6, §7.2, §8), RFC 4862 (§5), RFC 5942 (§6) and RFC 6980 (§5); the feature map; the checks; a two-node link with the default RA/RS intervals overridden in the ini |
| 3, Edge | not started | the validation clauses of RFC 4861 §6.1.2, §7.1.1, §7.1.2 against a crafted RA, NS or NA; no separate host-requirements companion exists for ND |
| 4, Dynamics | not started | RFC 4861 §6.2.6 and §6.3.4 (RA jitter, reachable-time randomization), RFC 7048, RFC 7559, RFC 8319, all with a tolerance |
| 5, Complete | not started | RFC 4429, RFC 7527, RFC 8028, RFC 9131, RFC 9762, RFC 9685, RFC 9926, and the model's own admitted anycast and multi-prefix gaps |

## Pass log

| Pass | Date | Level | Scope | Result |
| --- | --- | --- | --- | --- |
| 1 | 2026-09-23 | **1, reached** | RFC 4861, RFC 4862, RFC 5942 and RFC 6980 downloaded; the standards map; the claims of the model | no run; two obsolete claims (RFC 2461, RFC 2462), one claim outside the in-scope set (RFC 4429), one declined document with a stated reason (RFC 7527) |
