# PIM — coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/pim/standards.md), [conformance.md](conformance.md)

The single place that holds the changing state of the PIM workflow. Every other artifact of
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
[`standards.md`](../../protocol/pim/standards.md#target-level).

The exit criterion of level 1 has three parts, and all three hold:

| Part of the criterion | State | Evidence |
| --- | --- | --- |
| The standards map pins an in-scope set | holds | RFC 7761 (partial) and RFC 3973 (partial) for the two modes, plus RFC 3956 and RFC 4607, chosen from the register: [`standards.md`](../../protocol/pim/standards.md#in-scope-set) |
| Every model claim is mapped onto the map | holds | each of the 14 places in the model that name a document has a row in [`conformance.md`](conformance.md#part-1--the-claims) |
| Every obsolete claim is named | holds | two: `PimSm.ned`/`PimSm.h`/`Pim.h` name RFC 4601, obsoleted by RFC 7761; `PimPacket.msg`/`Pim.cc` name RFC 2460, obsoleted by RFC 8200 |

| Level | State | What it needs |
| --- | --- | --- |
| 1, Survey | **reached** | — |
| 2, Core | not started | the catalogs of RFC 7761 (§3, §4.1-4.6, static configuration of §4.7, §4.8, §4.9.1-4.9.6), RFC 3973 (§3, §4.1-4.7), RFC 3956 and RFC 4607; the feature map; the checks; a small network for each mode |
| 3, Edge | not started | RFC 9436's flag bits and type-space extension, and the base documents' own validity checks |
| 4, Dynamics | not started | the timers of RFC 7761 §4.10-4.11 and RFC 3973 §4.8 |
| 5, Complete | not started | RFC 5059 (declared absent by the model) |

## Pass log

| Pass | Date | Level | Scope | Result |
| --- | --- | --- | --- | --- |
| 1 | 2026-09-23 | **1, reached** | RFC 7761, RFC 3973, RFC 3956 and RFC 4607 downloaded; the standards map; the claims of the model | no run; two obsolete claims (RFC 4601 in the published module documentation of PIM-SM, RFC 2460 in the checksum comments of both modes); three declared limitations in `PimSm.ned`, not two; the Bootstrap and Candidate-RP-Advertisement message types have a reserved code and a discard-only receive path but no message class |
