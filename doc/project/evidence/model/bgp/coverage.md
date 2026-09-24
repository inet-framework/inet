# BGP-4 — coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/bgp/standards.md), [conformance.md](conformance.md)

The single place that holds the changing state of the BGP-4 workflow. Every other artifact of
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
[`standards.md`](../../protocol/bgp/standards.md#target-level).

The exit criterion of level 1 has three parts, and all three hold:

| Part of the criterion | State | Evidence |
| --- | --- | --- |
| The standards map pins an in-scope set | holds | RFC 4271 §3-§5, §6.8, §8, §9; RFC 4760; RFC 5492; RFC 6793; RFC 8212 — chosen from the register: [`standards.md`](../../protocol/bgp/standards.md#in-scope-set) |
| Every model claim is mapped onto the map | holds | each of the 17 places in the model that name a document, or refuse or stub one, has a row in [`conformance.md`](conformance.md#part-1--the-claims) |
| Every obsolete claim is named | holds | none found; every claim names a current document (RFC 4271, RFC 4760, RFC 5492, RFC 6286, RFC 1997, RFC 7911) |

| Level | State | What it needs |
| --- | --- | --- |
| 1, Survey | **reached** | — |
| 2, Core | not started | the catalogs of RFC 4271 (§3-§5, §6.8, §8, §9), RFC 4760, RFC 5492, RFC 6793 and RFC 8212, the feature map, the checks, and a network of two routers with one eBGP session |
| 3, Edge | not started | the NOTIFICATION message the model cannot build, RFC 6608, RFC 7606, RFC 7607, RFC 9072, RFC 9774 |
| 4, Dynamics | not started | the FSM timers of RFC 4271 §8, the damping timers of §9.2.1 (unimplemented), and RFC 9687 |
| 5, Complete | not started | RFC 4724, RFC 8654, RFC 6286, RFC 1997, RFC 7911, RFC 4456, RFC 5065 |

## Pass log

| Pass | Date | Level | Scope | Result |
| --- | --- | --- | --- | --- |
| 1 | 2026-09-23 | **1, reached** | RFC 4271, RFC 4760, RFC 5492, RFC 6793 and RFC 8212 downloaded; the standards map; the claims of the model | no run; no obsolete claim; two current, normative updates to the normal exchange (RFC 6793, RFC 8212) are unclaimed, and RFC 6793 is architecturally unreachable (a fixed 2-octet AS number field); the `__TODO` file's own limitations table is inaccurate for LOCAL_PREF (claimed missing, actually implemented and used) and for two Mandatory FSM events (ManualStop, Tcp_CR_Acked, both without a handler) |
