# Mobile IPv6 — coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/mipv6/standards.md), [conformance.md](conformance.md)

The single place that holds the changing state of the Mobile IPv6 workflow. Every other
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
[`standards.md`](../../protocol/mipv6/standards.md#target-level).

The exit criterion of level 1 has three parts, and all three hold:

| Part of the criterion | State | Evidence |
| --- | --- | --- |
| The standards map pins an in-scope set | holds | RFC 6275, chosen from the register: [`standards.md`](../../protocol/mipv6/standards.md#in-scope-set) |
| Every model claim is mapped onto the map | holds | each of the 13 places in the model that name a document of the family has a row in [`conformance.md`](conformance.md#part-1--the-claims) |
| Every obsolete claim is named | holds | nine places name RFC 3775, obsoleted by RFC 6275: the class doc, the NED doc, both data-store modules, the home-agent node type, five comments in `Mipv6.cc` counted as one row, and both users-guide citations |

| Level | State | What it needs |
| --- | --- | --- |
| 1, Survey | **reached** | — |
| 2, Core | not started | the catalog of RFC 6275, the feature map, the checks, and a mockup with a mobile node, a home agent and a correspondent node |
| 3, Edge | not started | crafted Binding Updates, from the same document |
| 4, Dynamics | not started | the nonce/key lifetime and retransmission timers of RFC 6275 §5.2 and §11.8, with a tolerance |
| 5, Complete | not started | Dynamic Home Agent Address Discovery, Mobile Prefix Discovery, the full mobility-option set, and IPsec-protected signaling (RFC 4877, declined by the model so far — see [`conformance.md`](conformance.md#part-1--the-claims)) |

## Pass log

| Pass | Date | Level | Scope | Result |
| --- | --- | --- | --- | --- |
| 1 | 2026-09-23 | **1, reached** | RFC 6275 downloaded; the standards map; the claims of the model | no run; nine places claim the obsolete RFC 3775 while the serializer already claims RFC 6275; RFC 4877 and RFC 5095 are out of scope by the register and unclaimed |
