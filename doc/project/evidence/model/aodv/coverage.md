# AODV — coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/aodv/standards.md), [conformance.md](conformance.md)

The single place that holds the changing state of the AODV workflow. Every other artifact of
this protocol states what a standard says and never changes again unless the standard changes.
This one changes on every pass.

State of the ledger, from a level 1 pass — no build, no run:

- Date: 2026-09-23
- INET: branch `topic/standards-tests-wave0`, commit `e360ca980e`, tree clean
- Trees: src `16dc528e10`, tests/protocol `6f0a6bdb05`
- Target level: 1

No catalog, no feature map and no test exist yet, so the ledger has no statement table and no
feature table.

## Achieved level

**Level 1, reached.** Target: level 1, from
[`standards.md`](../../protocol/aodv/standards.md#target-level).

The exit criterion of level 1 has three parts, and all three hold:

| Part of the criterion | State | Evidence |
| --- | --- | --- |
| The standards map pins an in-scope set | holds | RFC 3561 §5, §6.1, §6.3 to §6.7 and §6.11, and RFC 5148 §5, chosen from the register: [`standards.md`](../../protocol/aodv/standards.md#in-scope-set) |
| Every model claim is mapped onto the map | holds | each of the 11 places in the model that name a document has a row in [`conformance.md`](conformance.md#part-1--the-claims) |
| Every obsolete claim is named | holds, vacuously | neither RFC 3561 nor RFC 5148 has an `obsoleted_by` or `updated_by` relation in the register, so no citation of the model can be obsolete; see [`conformance.md`](conformance.md#part-1--the-claims) |

| Level | State | What it needs |
| --- | --- | --- |
| 1, Survey | **reached** | — |
| 2, Core | not started | the catalogs of RFC 3561 (§5, §6.1, §6.3 to §6.7, §6.11) and RFC 5148 (§5), the feature map, the checks, and a two- or three-hop wireless chain |
| 3, Edge | not started | the request-validation and duplicate-suppression rules of §6.4 and §6.5, and a unidirectional link (§6.8) |
| 4, Dynamics | not started | the Hello message interval and local-connectivity check (§6.9, §6.10), the ring-search backoff (§6.3), and the RFC 5148 jitter bound, with a tolerance |
| 5, Complete | not started | local repair and actions after reboot (both claimed by the model, see [`conformance.md`](conformance.md#part-1--the-claims)), multiple interfaces, subnet aggregation, and the AODV-to-wired gateway |

## Pass log

| Pass | Date | Level | Scope | Result |
| --- | --- | --- | --- | --- |
| 1 | 2026-09-23 | **1, reached** | RFC 3561 and RFC 5148 downloaded; the standards map; the claims of the model | no run; no obsolete claim; one claim (local repair) reads as a refusal but is a claim by the guide's own rule, because the switch already exists |
