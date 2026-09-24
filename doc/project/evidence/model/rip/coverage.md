# RIP — coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/rip/standards.md), [conformance.md](conformance.md)

The single place that holds the changing state of the RIP workflow. Every other artifact of
this protocol states what a standard says and never changes again unless the standard changes.
This one changes on every pass.

State of the ledger, from a level 1 pass — no build, no run:

- Date: 2026-09-23
- INET: branch `topic/standards-tests-wave0`, commit `97f4559eee`, tree clean
- Trees: src `16dc528e10`, tests/protocol `6f0a6bdb05`
- Target level: 1

No catalog, no feature map and no test exist yet, so the ledger has no statement table and no
feature table.

## Achieved level

**Level 1, reached.** Target: level 1, from
[`standards.md`](../../protocol/rip/standards.md#target-level).

The exit criterion of level 1 has three parts, and all three hold:

| Part of the criterion | State | Evidence |
| --- | --- | --- |
| The standards map pins an in-scope set | holds | RFC 2453 §3 and §4.2 to §4.6, and RFC 2080 §2, chosen from the register: [`standards.md`](../../protocol/rip/standards.md#in-scope-set) |
| Every model claim is mapped onto the map | holds | each of the 11 places in the model that name a document, or refuse one, has a row in [`conformance.md`](conformance.md#part-1--the-claims) |
| Every obsolete claim is named | holds | one: the serializer names RFC 1058, which is Historic, for the version 2 layout of RFC 2453 |

| Level | State | What it needs |
| --- | --- | --- |
| 1, Survey | **reached** | — |
| 2, Core | not started | the catalogs of RFC 2453 and RFC 2080, the feature map, the checks, and a network of two or three routers; the update interval of 30 seconds needs a shorter setting in the network |
| 3, Edge | not started | crafted responses, from the same two documents |
| 4, Dynamics | not started | the timers of RFC 2453 §3.8 and RFC 2080 §2.3, with a tolerance |
| 5, Complete | not started | authentication (declined by the model), the interaction with RIP version 1, demand circuits |

## Pass log

| Pass | Date | Level | Scope | Result |
| --- | --- | --- | --- | --- |
| 1 | 2026-09-23 | **1, reached** | RFC 2453 and RFC 2080 downloaded; the standards map; the claims of the model | no run; one obsolete claim (RFC 1058), one declined area (authentication) |
