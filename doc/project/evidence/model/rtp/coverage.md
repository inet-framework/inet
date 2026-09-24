# RTP — coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/rtp/standards.md), [conformance.md](conformance.md)

The single place that holds the changing state of the RTP workflow. Every other artifact of
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
[`standards.md`](../../protocol/rtp/standards.md#target-level).

The exit criterion of level 1 has three parts, and all three hold:

| Part of the criterion | State | Evidence |
| --- | --- | --- |
| The standards map pins an in-scope set | holds | RFC 3550 §5, §6.1, §6.4, §6.5 and §6.6, and RFC 3551 §6 (payload type 10), chosen from the register: [`standards.md`](../../protocol/rtp/standards.md#in-scope-set) |
| Every model claim is mapped onto the map | holds | each of the 5 places in the model that name a document has a row in [`conformance.md`](conformance.md#part-1--the-claims) |
| Every obsolete claim is named | holds | three of the five claims name RFC 1889 or RFC 1890, both obsoleted in 2003; see [`conformance.md`](conformance.md#part-1--the-claims) |

| Level | State | What it needs |
| --- | --- | --- |
| 1, Survey | **reached** | — |
| 2, Core | not started | the catalogs of RFC 3550 (§5, §6.1, §6.4, §6.5, §6.6) and RFC 3551 (§6, payload type 10), the feature map, the checks, and two RTP endpoints; payload type 10's sender and receiver classes are excluded from the build today (see [`conformance.md`](conformance.md#part-1--the-claims)) and need re-enabling first |
| 3, Edge | not started | a crafted or truncated RTCP packet, and an induced loss for the fraction-lost computation of §6.4.4 |
| 4, Dynamics | not started | the RTCP transmission-interval algorithm and the SSRC timeout of §6.2 and §6.3, with a tolerance |
| 5, Complete | not started | APP packets, translators and mixers, every document that updates RFC 3550 or RFC 3551, and RFC 2250 — already known, from `tests/serializer/REMAINING_GAPS.md`, to be a defect and not merely untested |

## Pass log

| Pass | Date | Level | Scope | Result |
| --- | --- | --- | --- | --- |
| 1 | 2026-09-23 | **1, reached** | RFC 3550 and RFC 3551 downloaded; the standards map; the claims of the model | no run; three of five claims name a document obsoleted in 2003 (RFC 1889, RFC 1890); one names RFC 2250, current but out of scope; the one in-scope current-document claim is a single comment inside a formula, not a documentation comment; the claimed payload type 10 is excluded from the build |
