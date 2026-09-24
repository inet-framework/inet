# SCTP — coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/sctp/standards.md), [conformance.md](conformance.md)

The single place that holds the changing state of the SCTP workflow. Every other artifact of
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
[`standards.md`](../../protocol/sctp/standards.md#target-level).

The exit criterion of level 1 has three parts, and all three hold:

| Part of the criterion | State | Evidence |
| --- | --- | --- |
| The standards map pins an in-scope set | holds | RFC 9260 §3, §4, §5, §6 and §9, chosen from the register: [`standards.md`](../../protocol/sctp/standards.md#in-scope-set) |
| Every model claim is mapped onto the map | holds | each of the 14 places in the model that name a document, and the 2 places that name none, has a row in [`conformance.md`](conformance.md#part-1--the-claims) |
| Every obsolete claim is named | holds | eight claims name RFC 4960 (obsoleted by RFC 9260 in 2022, including one that is an enum identifier, not a comment) and one names RFC 2960 (obsoleted twice over); see [`conformance.md`](conformance.md#part-1--the-claims) |

| Level | State | What it needs |
| --- | --- | --- |
| 1, Survey | **reached** | — |
| 2, Core | not started | the catalog of RFC 9260 §3, §4, §5, §6 and §9, the feature map, the checks, and a two-endpoint association (the existing `tests/module/sctp_*` scenarios show the shape to reuse) |
| 3, Edge | not started | the fault-management validity checks of §8 and the ICMP companion of §10 |
| 4, Dynamics | not started | the congestion-control loop of §7 and the retransmission timer of §8, with a tolerance |
| 5, Complete | not started | PR-SCTP, ASCONF, AUTH and stream reconfiguration (each claimed by the model, only in the changelog — see [`conformance.md`](conformance.md#part-1--the-claims)), DPLPMTUD, SCTP-over-UDP, and the Internet-Draft extensions NR-SACK and CMT |

## Pass log

| Pass | Date | Level | Scope | Result |
| --- | --- | --- | --- | --- |
| 1 | 2026-09-23 | **1, reached** | RFC 9260 downloaded; the standards map; the claims of the model | no run; RFC 9260 is claimed nowhere; RFC 4960 is claimed pervasively, including as an enum identifier in the parameter surface; RFC 2960 is claimed once, two generations behind |
