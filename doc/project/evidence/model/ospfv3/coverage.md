# OSPFv3 — coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/ospfv3/standards.md), [conformance.md](conformance.md)

The single place that holds the changing state of the OSPFv3 workflow. Every other artifact
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
[`standards.md`](../../protocol/ospfv3/standards.md#target-level).

The exit criterion of level 1 has three parts, and all three hold:

| Part of the criterion | State | Evidence |
| --- | --- | --- |
| The standards map pins an in-scope set | holds | RFC 5340's own list of unchanged and inherited RFC 2328 clauses, chosen from the register: [`standards.md`](../../protocol/ospfv3/standards.md#in-scope-set) |
| Every model claim is mapped onto the map | holds | each of the 17 places in the model that name a document has a row in [`conformance.md`](conformance.md#part-1--the-claims) |
| Every obsolete claim is named | holds | none exists. RFC 2740 (obsoleted by RFC 5340) is never cited anywhere in `src/inet/routing/ospfv3` or `src/inet/node/ospfv3`; both RFC 2328 citations the model carries land on clauses the standards map's inheritance table also lists |

Unlike OSPFv2, no claim in OSPFv3 is obsolete or misattributed — the finding here is what
the model does **not** claim: RFC 5340 was itself updated by five documents and has one
companion, and the model names none of them, even for the NSSA area type and the dual
address-family instances its own code shape already carries out partway. See
[`conformance.md`](conformance.md#part-1--the-claims) for the five fact rows a level 2 pass
needs, three of which mark code that runs (the checksum stub, the active
`calculateInterAreaRoutes` call) against comments that say otherwise.

| Level | State | What it needs |
| --- | --- | --- |
| 1, Survey | **reached** | — |
| 2, Core | not started | the catalogs of RFC 5340's level-2 slice and the RFC 2328 clauses it inherits (shared with [`ospfv2/coverage.md`](../ospfv2/coverage.md)), the feature map, the checks, and a network of two routers over IPv6 in one area |
| 3, Edge | not started | crafted Hello, Database Description and LSA packets against the same slice |
| 4, Dynamics | not started | the Hello, retransmission and aging timers RFC 5340 §4.2.1.1 and the inherited RFC 2328 clauses govern, with a tolerance |
| 5, Complete | not started | more than one area, external and NSSA routes, a second interface to one link, and the five update RFCs and the companion RFC 5838 that `standards.md` places at level 5 |

## Pass log

| Pass | Date | Level | Scope | Result |
| --- | --- | --- | --- | --- |
| 1 | 2026-09-23 | **1, reached** | RFC 5340 downloaded; RFC 2328 reused from `ospfv2`; the standards map; the claims of the model | no run; no obsolete or misattributed claim; two stale/misleading comments found next to live code (`Ospfv3Process.cc:427`'s "multiarea... not supported" above a multi-area loop, and the commented-out step 4/step 5 of the routing-table calculation next to the step 3 call that does run); the LSA checksum stub repeats the OSPFv2 defect in all eight LSA classes |
