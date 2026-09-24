# DiffServ — coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/diffserv/standards.md), [conformance.md](conformance.md)

The single place that holds the changing state of the DiffServ workflow. Every other artifact
of this protocol states what a standard says and never changes again unless the standard
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
[`standards.md`](../../protocol/diffserv/standards.md#target-level).

The exit criterion of level 1 has three parts, and all three hold:

| Part of the criterion | State | Evidence |
| --- | --- | --- |
| The standards map pins an in-scope set | holds | RFC 2474, RFC 2475, RFC 2597, RFC 3246, RFC 2697, RFC 2698 and RFC 3260, chosen from the register: [`standards.md`](../../protocol/diffserv/standards.md#in-scope-set) |
| Every model claim is mapped onto the map | holds | each of the ten claim rows has a row in [`conformance.md`](conformance.md#part-1--the-claims) |
| Every obsolete claim is named | holds | one: `Dscp.msg:36` names RFC 2598, obsoleted by RFC 3246, for the same PHB the model elsewhere (correctly) cites RFC 3246 for |

| Level | State | What it needs |
| --- | --- | --- |
| 1, Survey | **reached** | — |
| 2, Core | not started | the catalogs of RFC 2474, RFC 2475, RFC 2597, RFC 3246, RFC 2697, RFC 2698 and RFC 3260, the feature map, the checks, and one host with a classifier, a marker, a meter and a PHB queue on its interface |
| 3, Edge | not started | the in-profile/out-of-profile case of RFC 2475 §2.3.2 and the unknown-DSCP case of RFC 3260 §6; both already inside the in-scope documents |
| 4, Dynamics | not started | the token-bucket distributions of RFC 2697 §3 and RFC 2698 §3, with a tolerance |
| 5, Complete | not started | RFC 3290 (claimed, out of scope) and RFC 4115 (an alternative marker the model does not implement) |

## Pass log

| Pass | Date | Level | Scope | Result |
| --- | --- | --- | --- | --- |
| 1 | 2026-09-23 | **1, reached** | RFC 2474, RFC 2475, RFC 2597, RFC 3246, RFC 2697, RFC 2698, RFC 3260 downloaded; the standards map; the claims of the model | no run; one obsolete claim (RFC 2598, contradicted by the model's own RFC 3246 claim for the same PHB); one claimed document outside the in-scope set (RFC 3290) |
