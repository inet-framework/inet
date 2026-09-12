# Align Onoe's decision algorithm with MadWifi

Status: implemented and focused validation passed, 2026-09-10; pending review.

## Implementation contract

- Invariant and owner: Onoe owns only per-receiver completed-sample statistics,
  credit, mode, and evaluation time. Recovery procedures retain ownership of
  per-packet retry counters. Decision arithmetic follows the pinned MadWifi source.
- Entry and control path: DCF/HCF call `frameTransmitted()` after incrementing
  failures and before clearing terminal retry state. Onoe consumes the terminal
  count once, then evaluates a due sample; `getRate()` only initializes/reads state.
- Affected artifacts: Onoe `.cc/.h/.ned`, the three existing Onoe module tests,
  additional Onoe boundary/feedback tests, and this plan/report. Shared callers,
  interfaces, packets, and AARF behavior are unchanged.
- Siblings and terminals: exercise DCF/HCF normal-ACK success and exhaustion,
  intermediate retries, peer and TID interleaving, idle queries, and the existing
  mode-set signal reset. Feedback borrows packets and does not retain or delete them.
- Boundaries and units: elapsed simulation seconds from first peer observation or
  last evaluation; terminal completion included at equality; integral counters,
  truncated ten-percent threshold, credit 0 through 9, rate floor and ceiling.
- Verification: fresh debug build and `inet_run_module_tests -m debug -f
  'OnoeRateControl.*\.test'`; actual DCF/HCF callbacks establish retry mapping,
  public API tests establish timing and decisions, scoped architecture check
  establishes structural compliance. Existing INI/CSV searches find no configured
  Onoe fingerprint case; report that gap rather than claim fingerprint coverage.

The contract was self-validated against the current sources before editing.
The instruction to execute this plan authorizes the listed behavioral test
expectation changes. Deferred follow-ups remain outside this implementation.

## Assessment and reference scope

The supplied critique is substantially correct. PR #1176 fixes failed-attempt
double counting and failed-only fallback, but retains an approximation of Onoe's
decision algorithm. Its six statistical discrepancies are present in this checkout.
Adopt the MadWifi decision rules and completion-driven sampling together, then
describe the supported feedback scope precisely. Do not claim full MadWifi
compatibility from those changes alone.

Evidence inspected:

- [PR #1176](https://github.com/inet-framework/inet/pull/1176), whose retrieved head
  matches local HEAD `6d7b49c1d51b5d28f0eb8d59d40bd8e3f507e07b`.
- [MadWifi onoe.c](https://github.com/proski/madwifi/blob/a7531fd223a1f454d3fd74a975b4581cde5411bb/ath_rate/onoe/onoe.c):
  `ath_rate_ctl`, `ath_rate_tx_complete`, `ath_rate_update`, and `ath_rate_ctl_start`.
  This is a pinned historical implementation, not proof of the earliest version.
- Local [Onoe implementation](../../src/inet/linklayer/ieee80211/mac/ratecontrol/OnoeRateControl.cc),
  [state](../../src/inet/linklayer/ieee80211/mac/ratecontrol/OnoeRateControl.h),
  [parameters](../../src/inet/linklayer/ieee80211/mac/ratecontrol/OnoeRateControl.ned),
  DCF/HCF feedback callers, recovery counters, and all three `OnoeRateControl*.test` cases.

Onoe is an implementation-defined adaptation algorithm, not an IEEE 802.11
normative algorithm. Use “reference implementation” in documentation.

## Corrections and additions to the critique

1. **Integer rounding matters.** MadWifi uses a strict comparison against an
   integer-truncated percentage. With 19 successes and one retry, its threshold
   is 1 and no credit is awarded, although 1/19 is below 10%. Preserve that
   arithmetic; a floating ratio or cross-multiplication changes behavior.
2. **The floor changes reset behavior.** A failed-only sample below ten packets
   clears credit at the lowest rate but retains statistics because the rate did
   not actually change. Sufficient samples reset even at a rate limit. At the
   ceiling, the tenth qualifying credit resets credit without a rate change.
3. **“About 100 packets” needs traffic assumptions.** One clean completion per
   second produces ten qualifying ten-packet samples before an increase, from
   zero credit and below the ceiling. Exact elapsed time depends on initialization
   and boundary alignment; this is not a universal tenfold timing result.
4. **A callback is not necessarily a completed packet.** INET reports individual
   failures; MadWifi commits retry totals with final success/error. Reordering
   the existing callback alone can still contaminate a sample with an unfinished
   packet's failures, including another HCF access category's packet.
5. **RTS accounting is not established by counting data failures.** DCF's
   `originatorProcessRtsProtectionFailed()` has no rate-control callback, including
   terminal RTS failure. `NonQosRecoveryProcedure::getRetryCount()` and its QoS
   counterpart select a short or long counter by packet length; they do not sum
   both. Therefore the current feedback cannot be asserted equivalent to
   MadWifi's hardware short-plus-long retry status.
6. **MadWifi's retry chain has boundary behavior.** The inspected MadWifi chain
   uses a lowest-hardware-rate final stage and disables redundant lower stages.
   A future retry-series implementation must account for these details.
7. **Attribution needs care.** MadWifi names Atsushi Onoe's algorithm, while the
   inspected file credits Sam Leffler as copyright holder. Correct INET's NED
   description, which currently attributes development to Atheros. Implement
   the behavior in INET style and retain applicable notices if source is copied.

## Recommended scope

Deliver exact decision arithmetic, credit transitions, sample retention, and
completion-triggered evaluation for the currently supported normal-ACK data and
management feedback. Keep the existing `initialRate` parameter, supported mode
navigation, per-receiver state, and default one-second interval. State that INET
starts a peer's interval when first seen; this is an initialization convention.

Defer RTS/CTS feedback completeness, negotiated startup policy, and hardware-style
multirate retry to follow-up work. Aggregation/Block Ack and HT-or-newer fidelity
are not established by this legacy normal-ACK work. The resulting claim should be:

> Onoe's decision rules follow the pinned MadWifi reference for supported
> normal-ACK completion feedback; startup and retry-series behavior use INET's
> existing policies, and RTS/Block Ack equivalence is not claimed.

## Implementation sequence

### 1. Establish completion feedback and timing

Own changes in `OnoeRateControl.cc/.h` and focused module tests. Document the
existing callback contract in `IRateControl.h` only if needed; do not change
shared feedback semantics as part of this step.

- Verify, with real DCF and HCF traces, that terminal `retryCount` contains the
  accumulated data failures for the supported unprotected normal-ACK path:
  clean success = 0; two failures then success = 2; immediate terminal failure
  = 1; two nonterminal failures then terminal failure = 3.
- Use that terminal count once to update the completed sample. Intermediate
  failures neither contribute to the completed sample nor trigger evaluation.
  This avoids maintaining a second MAC retry counter in Onoe and prevents
  interleaved HCF packets from sharing pending retry state. If traces invalidate
  this mapping, resolve the producer contract before implementing this adapter;
  do not substitute one pending counter per receiver.
- On success or give-up, record exactly one completion and its retries first,
  then evaluate if the peer deadline has been reached. The completion at the
  deadline belongs to the evaluated sample. Advance the deadline from that
  evaluation time; do not run synthetic catch-up evaluations after idle time.
- Make `getRate()` return the stored rate after lazy peer initialization. It
  must not evaluate or consume a decision deadline. Keep this local to Onoe.
- Preserve clearing all peer state on mode-set changes. Tests must demonstrate
  that a reset clears counters, credit, rate references, and deadlines.

Exit criterion: completion counts and retry totals match actual callback traces;
rate queries and unfinished retries cannot change the sampled rate or credit.

### 2. Replace the decision block

Use completed successes S, terminal errors E, and their retry total R:

| Decision | Exact condition |
| --- | --- |
| Sufficient sample | S + E >= 10 |
| Down | (E > 0 and S == 0) or (sufficient and S < R) |
| Upward credit | sufficient and E == 0 and R < floor(S * 10 / 100) |
| Neutral | otherwise |

- Down: attempt one lower mode and set credit to zero, including at the floor.
- Neutral: decrement credit only for a sufficient sample and only above zero.
- Upward credit: increment; at ten, attempt one higher mode and reset credit,
  including at the ceiling.
- Clear S/E/R only after an actual rate change or a sufficient sample. Retain
  insufficient samples otherwise, including failed-only samples at the floor.
- Remove `avgRetriesPerFrame` if unused after the change. Use appropriately wide
  integer arithmetic for retained counts and percentage multiplication.
- Emit the existing per-peer signal on actual rate changes; preserve the initial
  peer observation. Suppress false increase notifications at the ceiling, just
  as the current code suppresses decreases at the floor.

Exit criterion: all table cases below match independently derived reference
expectations; credit stays in [0, 9] after each decision.

### 3. Repair and extend tests with the behavior change

The existing tests encode the approximation in more than one place:

- `OnoeRateControlRetryAccounting_1.test`: change four successes/eight failures
  from a decrease to retention. Two successes/two give-ups/eight failures is
  also insufficient and must retain the rate. Replace query-triggered ticks
  with deliberately timed completions and assert retained state or subsequent
  observable behavior.
- `OnoeRateControlFailureInterval_1.test`: replace the one-clean-packet-per-credit
  expectation and the one-success/ten-retries decrease. Rework every evaluation
  trigger for completion timing while retaining peer-isolation and floor checks.
  Increase its simulation limit for the sparse-traffic case or split that case.
- `OnoeRateControlRetryFeedback_1.test`: preserve real DCF/HCF coverage, but make
  it discriminate correct retry accounting. Four successes followed by a query
  will no longer trigger a sufficient evaluation. Include ten successes with
  exactly one failure each: R == S must hold the rate, while double counting
  would incorrectly decrease it. Add one extra failure as the paired down case.

Use public callback/rate APIs for the behavior tests, listeners for rate changes,
and a test subclass where direct credit/sample observation makes a boundary
assertion unambiguous. A helper-only test does not prove DCF/HCF integration.

| Required case | Expected result at evaluation |
| --- | --- |
| S=4, E=0, R=8 | Hold rate and credit; retain sample |
| S=8, E=2, R=9 | Down; credit and sample reset |
| S=10, E=0, R=10 / R=11 | Neutral / down |
| S=10, E=0, R=0 / R=1 | Credit / neutral |
| S=19, E=0, R=1 | Neutral due to integer truncation |
| S=20, E=0, R=1 | Credit |
| S=100, E=1, R=1 | No credit; decay existing positive credit |
| Repeated sufficient neutral samples, credit initially 0 | Credit stays 0 |
| Three clean completions per second | Samples accumulate; first credit at 12 completions |
| One clean completion per second, zero initial credit | First increase after 100 qualifying completions |
| E=1, S=0 below ten completions | Down above floor; clear credit but retain sample at floor |
| Ten qualifying samples at maximum rate | Credit resets; no rate-change signal |
| Completion just before / at / after deadline | No early evaluation; boundary completion included |
| Long idle gap, repeated getRate calls | No evaluation or credit manufactured |
| Unfinished retries across a deadline and HCF AC interleaving | Only completed packets enter sample |
| Two peers and mode-set reset | No state leakage; no stale state after reset |

For each independent table row, identify the pinned reference function in the
test description. Separate single-sample tests from accumulation tests so retained
state does not accidentally alter a later case's premise.

### 4. Validate and document the supported behavior

From the repository root, after implementation:

```sh
make MODE=debug -j$(nproc)
inet_run_module_tests -m debug -f 'OnoeRateControl.*\.test'
doc/project/enforcement/check-architecture.sh src/inet/linklayer/ieee80211/mac/ratecontrol
```

The module runner and scoped architecture script exist in this checkout. Record
the selected case count (zero is not a pass), commands, build mode, seeds, exit
statuses, and artifacts. Run the directly affected AARF cases too if a shared
contract or caller changes. Map Onoe-using configurations to focused fingerprint
checks; explain divergences with behavioral evidence. Before push, follow the
debug/release builds and project-wide gates in
[run-the-gates.md](../../doc/project/guide/run-the-gates.md).

Update the NED description and PR explanation with the reference revision,
completion semantics, changed low-load behavior, and explicit compatibility
limits. Suggested review units are completion accounting/timing with its tests,
then decision rules with their tests and documentation; each unit must pass its
own affected tests. No history rewrite is needed to execute this plan.

## Follow-up work, in order

1. **Complete RTS feedback:** design how data and RTS attempts contribute to one
   terminal outcome without double counting. Cover failed CTS exchanges, RTS
   exhaustion, and mixed RTS/data failures through DCF and HCF. Review all rate
   controllers before changing shared feedback. This is a prerequisite for a
   broader hardware-status equivalence claim. **Completed 2026-09-10:** see the
   [implementation and verification report](onoe-rts-feedback.md). The original
   implementation report below records the scope before this follow-up.
2. **Startup policy:** the pinned MadWifi source starts at the highest negotiated
   11b rate, otherwise the highest negotiated rate no greater than 36 Mbps.
   Decide how this maps to INET's peer capabilities and explicit initial-rate
   configuration. Do not alter shared `RateControlBase` defaults for Onoe alone.
   **Local-mode policy completed 2026-09-10:** automatic legacy startup now
   follows the reference ceiling using local modes; explicit rates take
   precedence and HT/VHT defaults are retained. Negotiated legacy peer rates
   are unavailable through the current contract. See the
   [decision and verification report](onoe-startup-policy.md).
3. **Multirate retry:** design a typed retry-context/series contract between rate
   selection, recovery, and control, respecting existing state ownership. Specify
   stage truncation at low rates, MAC retry-limit precedence, and packet identity
   against the pinned MadWifi reference. Validate transmitted PHY rates for
   every attempt; a stored-peer-rate assertion cannot prove a 4/2/2/2 series.
   **Design step completed 2026-09-10:** see the
   [retry-series contract and verification plan](onoe-multirate-retry.md).
   Implementation is pending reference-boundary verification and the explicit
   Tx/lifecycle integration checks recorded there.

## Project guidance and completion gates

This plan follows [the contributor route](../../doc/project/guide/contribute-a-change.md),
[WLAN ownership and boundaries](../../doc/project/domain/ieee80211.md#ar-wlan-arch-boundaries),
[testing rules](../../doc/project/rule/testing.md), and
[the seal registry](../../doc/project/audit/seal-list.md). The proposed Onoe source
paths are unsealed; no new architecture/naming exception is proposed. The execution
report below records focused validation and self-audit, not an independent audit.

Recorded expectation changes require a concrete scope-and-reason proposal under
[TR-BASELINE-DELIBERATE](../../doc/project/rule/testing.md#tr-baseline-deliberate)
before editing those expectations. The instruction to execute this concrete plan
authorized its listed test changes. No fingerprint baselines were changed.

The supplied AGENTS.md describes skill-source packaging checks, but this active
checkout has no `scripts/validate_skill_suite.py`, `scripts/package_skill_suite.py`,
or `tests/skill-suite`. Those checks cannot run here; no skill/package content is
changed by this plan. Validate this artifact with `git diff --check` and verify
its local links. Implementation is complete only when the behavioral matrix,
production DCF/HCF evidence, focused regression results, and scoped documentation
claim are all satisfied.

## Execution report

### Result and review description

Onoe now bases decisions on completed normal-ACK frames. Four successes with eight
retries retain their sample; ten completed frames with more retries than successes
lower the rate. Upward credit requires a sufficient error-free sample and the
integer-truncated retry threshold. Credit cannot become negative, and small
samples persist between evaluations. At one clean completion per second, the
first increase from zero credit takes 100 completions rather than ten.

Terminal feedback contributes the recovery procedure's retry count once, before
checking the interval. Intermediate failures and rate queries do not evaluate
samples. Rate-change signals are emitted only for actual changes, with the
existing initial peer observation preserved. INET startup and retry-series
policies remain in place; RTS/Block Ack equivalence is not claimed.

This text is prepared as the Onoe explanation for PR #1176. No remote PR edit,
commit, history rewrite, or push was performed.

### Changed paths

- `src/inet/linklayer/ieee80211/mac/ratecontrol/OnoeRateControl.cc`
- `src/inet/linklayer/ieee80211/mac/ratecontrol/OnoeRateControl.h`
- `src/inet/linklayer/ieee80211/mac/ratecontrol/OnoeRateControl.ned`
- `tests/module/OnoeRateControlRetryAccounting_1.test`
- `tests/module/OnoeRateControlFailureInterval_1.test`
- `tests/module/OnoeRateControlRetryFeedback_1.test`
- `tests/module/OnoeRateControlCompletionFeedback_1.test` (new)
- `tests/module/OnoeRateControlSampling_1.test` (new)
- `tests/module/OnoeRateControlInterleavedFeedback_1.test` (new)
- This plan, which is ignored by the checkout's existing Git rules.

The pre-existing AGENTS.md modification was preserved. No shared production
interface or caller changed. The division `successes / 10` implements the pinned
reference's integer-truncated ten-percent threshold without an intermediate
multiplication. Sample counters are now 64-bit. These are implementation choices
within the approved contract, not additional behavior scope.

### Focused evidence

All commands ran from `/home/user/omnetpp_ws/inet-agent-rate-control`.
Every module case used configuration `General`, run 0, `seed-set = 0`, debug mode.
The scenarios use deterministic outcomes; no probabilistic performance claim or
multi-seed campaign is made.

| Command | Exit | Result / artifact |
| --- | --- | --- |
| `make MODE=debug -j$(nproc)` | 0 | Fresh debug library; `/tmp/onoe-alignment/build-after.log` |
| `inet_run_module_tests -m debug -f 'OnoeRateControl.*\.test'` | 0 | Six selected tests, six PASS; `/tmp/onoe-alignment/module-final.log` |
| `doc/project/enforcement/check-architecture.sh src/inet/linklayer/ieee80211/mac/ratecontrol` | 0 | PASS; `/tmp/onoe-alignment/architecture.log` |
| `doc/project/enforcement/check-naming.sh src/inet/linklayer/ieee80211/mac/ratecontrol` | 0 | PASS; `/tmp/onoe-alignment/naming.log` |
| `git diff --check` | 0 | No whitespace errors |

Test-to-claim mapping:

| Test | Direct evidence |
| --- | --- |
| RetryAccounting | Sixteen isolated reference samples: insufficient/sufficient boundary, retries versus successes, errors blocking credit, integer truncation, credit decay, and tenth-credit increase; retained counters checked explicitly |
| FailureInterval | Idle queries, unfinished retries, ten neutral periods without negative credit, 100 sparse clean completions, sufficient high-retry fallback, peer isolation, downward signal suppression at the floor |
| RetryFeedback | Real DCF/HCF paired runs: ten successful packets with ten retries hold 24 Mbps; eleven retries lower to 18 Mbps; completed statistics clear |
| CompletionFeedback | Eight real DCF/HCF scenarios establishing terminal counts 0, 2, 1, and 3 for clean success, retry success, and two retry-exhaustion limits |
| Sampling | Before/equal/after deadlines, small samples across periods, no catch-up after idle, direct interleaved TID feedback, first credit at twelve completions for three per interval, floor retention/reset, ceiling credit reset without signals, and actual mode-set signal clearing both peers |
| InterleavedFeedback | Real HCF AC_BK failure, AC_VO success, then AC_BK retry success for one receiver; completed retry totals observed as 0, 0, then 1 |

Raw simulation output is under
`tests/module/work/<test-name-without-.test>/test.out` and `test.err`.
The interleaving trace is:

```text
feedback 1: tid=1 retries=1 success=0
feedback 2: tid=6 retries=0 success=1
feedback 3: tid=1 retries=1 success=1
HCF interleaved completion feedback verified.
```

During test development, fixture failures identified an unused DCF test-controller
instance on QoS stations, a missing include, an unassigned application parameter,
a legacy sink receiving QoS frames, and an error-model cast that assumed every
frame was data. Each fixture was corrected before the final six-test run; no
production workaround or changed seed was used to obtain a pass.

### Self-audit and remaining scope

- Owner/control path retraced: DCF/HCF notify before terminal retry cleanup;
  Onoe retains only completed statistics, owns no duplicate per-packet counter,
  borrows feedback packets, and uses existing mode-navigation and signal APIs.
- C++ numeric and state checks: strict retry comparisons, truncated percentage,
  zero-bounded credit, reset conditions, and rate limits match the contract.
  State and evaluation time are updated before rate-change notification.
- OMNeT++ checks: the existing initialization-stage coverage is preserved;
  elapsed time remains `simtime_t`; no self-messages, random draws, or new
  subscriptions are introduced. The real mode-set reset path is tested.
- INET/WLAN checks: packet representation, MAC recovery, rate selection, PHY,
  mode definitions, and shared AARF behavior are unchanged. The test evidence
  distinguishes public callback behavior from real DCF/HCF transmission paths.
  No new architecture/naming exception or protected-source change is needed.
- `rg -n -i 'onoe' --glob '*.ini' --glob '*.csv' examples showcases tests/fingerprint`
  returned no matches (exit 1). There is no existing directly configured Onoe
  fingerprint case in those trees; no fingerprint result or baseline update is
  claimed. The six module cases supply the focused behavioral evidence.
- No release build or project-wide before-push campaign was run, because this
  task did not push. Run the existing before-push gates when preparing publication.
  AARF was not retested because no shared caller, interface, or implementation
  changed. No independent reviewer was invoked.
- RTS/CTS completeness, negotiated startup, multirate retry, and aggregation or
  newer-PHY fidelity remain the documented follow-ups. Real feedback tests cover
  data frames; management callback compatibility is source-traced, without a
  separate management-exchange runtime test.
- The skill-suite packaging tools listed in AGENTS.md are absent in this INET
  checkout. No skill package was changed or represented as validated.
