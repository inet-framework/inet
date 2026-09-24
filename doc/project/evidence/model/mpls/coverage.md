# MPLS — coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/mpls/standards.md), [conformance.md](conformance.md)

The single place that holds the changing state of the MPLS workflow. Every other artifact of
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
[`standards.md`](../../protocol/mpls/standards.md#target-level).

The exit criterion of level 1 has three parts, and all three hold:

| Part of the criterion | State | Evidence |
| --- | --- | --- |
| The standards map pins an in-scope set | holds | RFC 3031 and RFC 3032, amended by RFC 3443 and RFC 5462, chosen from the register: [`standards.md`](../../protocol/mpls/standards.md#in-scope-set) |
| Every model claim is mapped onto the map | holds | each of the six places searched, and the one field comment that carries a fact without a number, has a row in [`conformance.md`](conformance.md#part-1--the-claims) |
| Every obsolete claim is named | holds (vacuously) | no document of the family is named anywhere in the model, so no claim can name an obsolete one; see the finding of the survey in [`conformance.md`](conformance.md#part-1--the-claims) |

| Level | State | What it needs |
| --- | --- | --- |
| 1, Survey | **reached** | — |
| 2, Core | not started | the catalogs of RFC 3031 and RFC 3032 (as amended by RFC 3443 and RFC 5462), the feature map, the checks, and two or three LSRs with a static LIB; the TTL fact of `conformance.md` fact 1 is the first candidate defect a level 2 check will hit |
| 3, Edge | not started | the invalid-incoming-label and no-outgoing-label cases of RFC 3031 §3.18 and §3.22, and the ICMP report of RFC 3032 §2.3 |
| 4, Dynamics | not started | RFC 3031 states no timer of its own; this level stays empty unless a later pass brings LDP or RSVP-TE dynamics in as a companion |
| 5, Complete | not started | RFC 3031 §3.20, §3.26, §3.27, and RFC 3270, RFC 5129, RFC 5332, RFC 5586, RFC 6790, RFC 7274 |

## Pass log

| Pass | Date | Level | Scope | Result |
| --- | --- | --- | --- | --- |
| 1 | 2026-09-23 | **1, reached** | RFC 3031, RFC 3032, RFC 3443, RFC 5462 downloaded; the standards map; the claims of the model | no run; zero claims of any kind, and one verified TTL defect found while checking the claim question |
