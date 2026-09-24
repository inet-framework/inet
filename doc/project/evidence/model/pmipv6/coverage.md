# Proxy Mobile IPv6 — coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/pmipv6/standards.md), [conformance.md](conformance.md)

The single place that holds the changing state of the Proxy Mobile IPv6 workflow. Every other
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
[`standards.md`](../../protocol/pmipv6/standards.md#target-level).

The exit criterion of level 1 has three parts, and all three hold:

| Part of the criterion | State | Evidence |
| --- | --- | --- |
| The standards map pins an in-scope set | holds | RFC 5213 and RFC 4283, chosen from the register and from RFC 5213's own text: [`standards.md`](../../protocol/pmipv6/standards.md#in-scope-set) |
| Every model claim is mapped onto the map | holds | each of the 10 places in the model that name a document of the family has a row in [`conformance.md`](conformance.md#part-1--the-claims) |
| Every obsolete claim is named | holds, vacuously | none exists. RFC 5213 has no `obsoletes`/`obsoleted by` relation in the register, and every citation in the tree names it by its current number — the opposite finding from Mobile IPv6, see [`conformance.md`](conformance.md#part-1--the-claims) |

| Level | State | What it needs |
| --- | --- | --- |
| 1, Survey | **reached** | — |
| 2, Core | not started | the catalogs of RFC 5213 and RFC 4283, the feature map, the checks, and a mockup with a Local Mobility Anchor, two Mobile Access Gateways and one mobile node |
| 3, Edge | not started | the reserved address of RFC 6543, once it enters scope |
| 4, Dynamics | not started | the retransmission and binding-lifetime timers of RFC 5213 §6.9.4 and §5.3.3/§5.3.4 |
| 5, Complete | not started | flow mobility (RFC 7864), IPv4 support (RFC 5844), multihoming, anchor address discovery and route optimization (RFC 5213 §5.4, §5.7, §5.9 — the binding cache's data structure has no room for the first of these, see [`conformance.md`](conformance.md#part-1--the-claims)) |

## Pass log

| Pass | Date | Level | Scope | Result |
| --- | --- | --- | --- | --- |
| 1 | 2026-09-23 | **1, reached** | RFC 5213 and RFC 4283 downloaded; the standards map; the claims of the model | no run; no obsolete claim, unlike Mobile IPv6's split RFC 3775/RFC 6275 claim; RFC 4283 enters this protocol's in-scope set although it stays out of Mobile IPv6's, because RFC 5213 elevates it to mandatory |
