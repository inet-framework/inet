# OSPFv2 — coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/ospfv2/standards.md), [conformance.md](conformance.md)

The single place that holds the changing state of the OSPFv2 workflow. Every other artifact
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
[`standards.md`](../../protocol/ospfv2/standards.md#target-level).

The exit criterion of level 1 has three parts, and all three hold:

| Part of the criterion | State | Evidence |
| --- | --- | --- |
| The standards map pins an in-scope set | holds | RFC 2328 §5 to §10, §12.1, §12.2, §12.4.1, §12.4.2, §13, §16.1 and Appendix A.3/A.4.1-A.4.3, chosen from the register: [`standards.md`](../../protocol/ospfv2/standards.md#in-scope-set) |
| Every model claim is mapped onto the map | holds | each of the 13 places in the model that name a document has a row in [`conformance.md`](conformance.md#part-1--the-claims) |
| Every obsolete claim is named | holds | RFC 1583, obsoleted by RFC 2178 and then by RFC 2328, is named twice: for the Router-LSA layout (`Ospfv2Packet.msg`) and for the `RFC1583Compatible` flag (`Ospfv2Router.h`) |

A fourth finding, beyond the exit criterion: one claim names a document that is not
obsolete but wrong. `Ospfv2.ned:41,159` cites RFC 3101 — a current Proposed Standard, and a
companion document about the NSSA area type — for the `RFC1583Compatible` parameter, which
RFC 2328 Appendix C.1 and Appendix G actually govern. See
[`conformance.md`](conformance.md#part-1--the-claims) for the two citations side by side.

| Level | State | What it needs |
| --- | --- | --- |
| 1, Survey | **reached** | — |
| 2, Core | not started | the catalog of RFC 2328's level-2 slice, the feature map, the checks, and a network of two routers in one area; the default `helloInterval` of 10 seconds needs a shorter setting or a longer simulated time |
| 3, Edge | not started | crafted Hello, Database Description and LSA packets against the same slice |
| 4, Dynamics | not started | the Hello, retransmission and aging timers of RFC 2328 §9.5, §13.3/§13.6 and §14/§14.1, with a tolerance |
| 5, Complete | not started | more than one area, virtual links, external routes, authentication (a stub today), and the eight RFCs and the companion RFC 3101 that `standards.md` places at level 5 |

## Pass log

| Pass | Date | Level | Scope | Result |
| --- | --- | --- | --- | --- |
| 1 | 2026-09-23 | **1, reached** | RFC 2328 downloaded; the standards map; the claims of the model | no run; one obsolete document named twice (RFC 1583), one wrong citation (RFC 3101 for `RFC1583Compatible`), two stub mechanisms found (authentication, LSA checksum validation) that a later pass must treat as defects, not omissions |
