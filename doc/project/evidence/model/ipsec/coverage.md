# IPsec — coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/ipsec/standards.md), [conformance.md](conformance.md)

The single place that holds the changing state of the IPsec workflow. Every other artifact of
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
[`standards.md`](../../protocol/ipsec/standards.md#target-level).

The exit criterion of level 1 has three parts, and all three hold:

| Part of the criterion | State | Evidence |
| --- | --- | --- |
| The standards map pins an in-scope set | holds | RFC 4301 §3.1-§3.2, §4.4.1, §4.4.2, §5.1, §5.2; RFC 4302 §2, §3.3, §3.4; RFC 4303 §2, §3.3, §3.4, chosen from the register: [`standards.md`](../../protocol/ipsec/standards.md#in-scope-set) |
| Every model claim is mapped onto the map | holds | each of the nine claim rows, and the five-item limitations list, has a row in [`conformance.md`](conformance.md#part-1--the-claims) |
| Every obsolete claim is named | holds (vacuously) | no obsolete document is claimed anywhere in the model; the claim names the current RFC 4301/4302/4303 throughout, see the finding of the survey in [`conformance.md`](conformance.md#part-1--the-claims) |

| Level | State | What it needs |
| --- | --- | --- |
| 1, Survey | **reached** | — |
| 2, Core | not started | the catalogs of RFC 4301, RFC 4302 and RFC 4303, the feature map, the checks, and two hosts with an SPD/SAD pair each; the ICV-verification TODO of `conformance.md` fact 1 is the first candidate defect a level 2 check will hit |
| 3, Edge | not started | the discard case of RFC 4301 §5.1.1 and the sequence-number and ICV verification failures of RFC 4302 §3.4 and RFC 4303 §3.4 |
| 4, Dynamics | not started | the anti-replay window of RFC 4302 Appendix B and RFC 4303 §3.4.3; declared unsupported by the model (`IPsec.ned:42`), so its checks are declared expected failures from the start |
| 5, Complete | not started | RFC 8221 and the cipher RFCs; the model performs no cryptography, so most checks of this level are untestable claims or out of claim entirely |

## Pass log

| Pass | Date | Level | Scope | Result |
| --- | --- | --- | --- | --- |
| 1 | 2026-09-23 | **1, reached** | RFC 4301, RFC 4302, RFC 4303 downloaded; the standards map; the claims of the model | no run; no obsolete claim; five stated refusals in the model's own words; one bare TODO (ICV verification) that is a claim, not a refusal |
