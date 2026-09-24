# IPv6 tunneling — coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/ipv6tunnel/standards.md), [conformance.md](conformance.md)

The single place that holds the changing state of the IPv6 tunneling workflow. Every other
artifact of this protocol states what a standard says and never changes again unless the
standard changes. This one changes on every pass.

State of the ledger, from a level 1 pass — no build, no run:

- Date: 2026-09-23
- INET: branch `topic/standards-tests-wave0`, commit `e360ca980e`, tree clean
- Trees: src `16dc528e10`, tests/protocol `6f0a6bdb05`
- Target level: 1

No catalog, no feature map and no test exist yet, so the ledger has no statement table and no
feature table.

## Achieved level

**Level 1, reached.** Target: level 1, from
[`standards.md`](../../protocol/ipv6tunnel/standards.md#target-level).

The exit criterion of level 1 has three parts, and all three hold:

| Part of the criterion | State | Evidence |
| --- | --- | --- |
| The standards map pins an in-scope set | holds | RFC 2473, the register's only document for this family: [`standards.md`](../../protocol/ipv6tunnel/standards.md#in-scope-set) |
| Every model claim is mapped onto the map | holds | 8 places in this protocol's own files and the generic IPv6 module have a row in [`conformance.md`](conformance.md#part-1--the-claims); 2 more, in Mobile IPv6's own copy of the tunnel logic, are named there and belong to the Mobile IPv6 pass's own files |
| Every obsolete claim is named | holds, vacuously | none exists. The register gives RFC 2473 no relative, so no citation can be stale against one |

| Level | State | What it needs |
| --- | --- | --- |
| 1, Survey | **reached** | — |
| 2, Core | not started | the catalog of RFC 2473, the feature map, the checks, and a mockup with a tunnel entry point, an exit point and one intermediate hop |
| 3, Edge | not started | nested and oversized tunnel packets, from the same document |
| 4, Dynamics | not started | none identified; RFC 2473 defines no timer of its own (see [`standards.md`](../../protocol/ipv6tunnel/standards.md#target-level)) |
| 5, Complete | not started | none identified; every mechanism of RFC 2473 is reachable at level 2 or 3 |

## Pass log

| Pass | Date | Level | Scope | Result |
| --- | --- | --- | --- | --- |
| 1 | 2026-09-23 | **1, reached** | RFC 2473 downloaded; the standards map; the claims of the model | no run; no obsolete claim; the claim reaches beyond this protocol's own directory into the generic IPv6 module and into Mobile IPv6's own copy of the same mechanism |
