# Resolve Wi-Fi protocol audit follow-ups

Status: **planned**. Created 2026-09-17.
Starting branch: `fix/protocol-tests-wireless`, HEAD `270014a45b`, rebased onto
`master` at `4548adeb04`. Production implementation has not started under this plan.

Subsequent maintainer decision: restore expected FAIL for the 12 failures in the earlier
18-test focused run because their repairs are nontrivial and deferred. See
[the decision and exact scope](../../doc/project/evidence/model/wifi/results.md#deferred-repair-expectations).
The five explicit testability gaps are included by explicit exception; their assertions need
implementation alongside the missing observations. The maintainer subsequently included `Ac_Ldpc` for the same reason; the scope is now 13 tests.
Focused verification after restoration: all 12 selected tests report expected FAIL;
aggregate CLI exit 1 is documented in the report. Debug build succeeded.
Current declaration inventory: 30; historical outcomes reconcile to 45 PASS, 30 expected FAIL,
and 0 unexpected FAIL. All W1–W9 repair and evidence obligations remain open.

## Objective and evidence boundary

Resolve the actionable findings in [the Wi-Fi audit report](../../doc/project/evidence/model/wifi/results.md)
through bounded production fixes, faithful observations, and explicit dispositions for work that
requires a larger feature implementation. Preserve the six repaired behavior tests.

The current source inventory is 75 tests: 45 default PASS expectations and 30 declared FAIL
expectations (13 deferred repairs plus 17 retained limitations/probes). Historical full-suite
observations reconcile to 45 PASS, 30 expected FAIL, and zero unexpected FAIL under those
declarations. A fresh focused run verified all 13 deferred tests as expected failures on
committed execution inputs; it was not a fresh full-suite run. W0 establishes a full baseline
before production follow-up work. A declaration, helper object, or passing presence probe does
not establish full feature support.

Use [the contribution workflow](../../doc/project/guide/contribute-a-change.md),
[testing rules](../../doc/project/rule/testing.md),
[test categories](../../doc/project/design/test-anatomy.md), and
[the gate procedure](../../doc/project/guide/run-the-gates.md).
Before editing production files, resolve their current status in the
[seal registry](../../doc/project/audit/seal-list.md) and apply the architecture and IEEE 802.11 rules.
Owner names below identify implementation responsibilities, not assigned people.

## Execution order and boundaries

1. **W0:** establish the rebased baseline and review the current classification rule.
2. **W1–W3:** resolve delayed Block Ack, mixed-BSS association, and mode-selection failures.
   These have independent owners and can be separate topic changes.
3. **W4:** integrate A-MPDU with an explicit supported contract. Review Block Ack overlap with W1.
4. **W5–W8:** deliver bounded designs and observations for Multi-TID, LDPC, 80+80,
   reverse direction, and MU-MIMO before undertaking complete feature implementations.
5. **W9:** sharpen retained probes as their prerequisites become available.
6. **W10:** reconcile results and regression evidence after each landed task.

Each implementation change carries its directly related tests and any approved expectation change.
Keep unrelated mechanisms in separate commits/PRs. This plan is not a commitment to implement
all optional Wi-Fi features in one branch. A design-stage task may end with a named blocker and
a separately scoped implementation plan, but that does not mean its feature works or its test passes.

## W0 — establish current evidence and dispositions

Owner: protocol-test maintenance. Dependencies: none.

- [ ] Record branch, full commit, working-tree diff, compiler, OMNeT++ version, library mode,
  exact commands, seed, configuration, timings, and preserved artifact directory.
- [ ] Rebuild INET in debug mode: the rebase includes production header and test-build changes.
  Run all 75 Wi-Fi protocol tests once, preserving logs outside generated `work/`.
- [ ] Compare every observed verdict and first failing step with the audit report. Investigate
  any new build/setup failure before attributing it to protocol behavior.
- [ ] Review the diagnostic classes and repair dependencies of the 13 deferred expected failures
  against the current
  [six-class rule](../../doc/project/guide/derive-tests-from-a-standard.md#the-class-of-a-failure-and-when-to-declare-it-expected).
  Keep detailed labels such as selection/integration/setup as explanatory subcategories.
- [ ] For a proposed **defect with a blocked repair**, identify the faithful failing assertion,
  concrete long-term limitation, affected owner, evidence, and condition that removes the block.
  Work size, a bare TODO, or a missing observation alone is insufficient. Untestable claims remain
  visible failures. Follow the expectation-change procedure for any concrete marker proposal;
  the later maintainer decision authorizes the 12 restorations and subsequent `Ac_Ldpc` declaration recorded above;
  additional declarations require their own applicable evidence and authorization.

Acceptance: a current per-test baseline, exact selected count, and evidence-backed dispositions.
No predetermined marker count or green-run target. Record any changed classification as a new
pass; do not rewrite the historical audit as if its run used the new rule.

## W1 — honor the accepted delayed Block Ack policy

Owner: Block Ack agreement and recipient/originator coordination.
Test: `common/WifiQosDelayedBa.test`.
Entry points: `RecipientQosAckPolicy`, agreement builders, recipient response procedure,
originator BAR/BA timeout handling, and TXOP/channel-access ownership.

- [ ] Reproduce accepted delayed ADDBA followed by the first addressed response to BAR being BA.
  Preserve the negotiated policy and exchange addresses, TID, sequence context, and timestamps.
- [ ] Identify the applicable legacy standard revision and exact delayed-BA exchange before
  specifying timing. Derive timing from the modeled PHY/procedure; do not hard-code SIFS+PIFS.
- [ ] Trace who sends the immediate acknowledgement, queues the delayed BA, acquires access,
  waits for acknowledgement, and cleans up on timeout/agreement teardown.
- [ ] Implement the negotiated procedure through those owners. If support is deliberately
  declined instead, treat refusal as an explicit contract change with rationale and its own test;
  do not weaken the existing successful-negotiation assertion to conceal the discrepancy.
- [ ] Test immediate and delayed policies, a lost response/timeout, late completion, and two
  peers or TIDs where state could leak. Preserve the first-response assertion.

Acceptance: accepted delayed negotiation reaches the standards-backed ACK/BA exchange and
completion; immediate BA still works; no stale pending response after teardown. Pair frame
observations with timer/state evidence where captures cannot establish the cause.

## W2 — make mixed-BSS association reach the ERP observation

Owner: management/rate compatibility and PHY reception.
Test: `11g/G_ErpProtection.test`.
Entry points: the test configuration, AP/STA management, supported/basic rate selection,
and radio receive-mode compatibility.

- [ ] Resolve effective INI values and prove the intended b-only STA and mixed-mode AP are active.
- [ ] Trace scan, beacon reception, authentication, and association to the first missing or rejected
  transition. Inspect frame rates, basic-rate membership, channel, and reception/drop reasons.
- [ ] Repair the smallest demonstrated configuration or production defect. Retain an explicit
  successful-association prerequisite; do not bypass it or force ERP state in the fixture.
- [ ] Only then assess ERP: establish the applicable protection advertisement and actual protected
  exchange with a non-ERP member, plus a no-non-ERP-member control. Reclassify a newly reached failure.

Acceptance: mixed-BSS association completes through production management; the report separately
states association and ERP verdicts. No protection verdict is inferred from the current early failure.

## W3 — expose supported PHY mode selection

Owner: PHY mode sets, mode cache, and transmitter/receiver mode selection.
Tests: `11b/B_Pbcc.test`, `11b/B_ShortPreamble.test`, `11n/N_Greenfield.test`.
Entry points: `Ieee80211ModeSet`, `Ieee80211HrDsssMode`, `Ieee80211HtMode` and callers.

- [ ] For each test, trace object construction, cache identity, configured selection, transmitted
  mode, and receiver support. Identify precisely where the existing object becomes unreachable.
- [ ] For greenfield, check whether cache identity distinguishes mixed/greenfield format.
  For short preamble, check supported rate/peer combinations. For PBCC, distinguish mode identity
  and timing support from an implemented coding/decoding path.
- [ ] Choose an existing configuration/API if sufficient; otherwise define the smallest explicit
  selection contract. Do not globally change default mode sets merely to satisfy these tests.
- [ ] Add focused mode/cache tests for identity and relevant timing plus a production transmission
  and reception case. Keep CCK/long-preamble/mixed-format controls and unsupported-choice handling.

Acceptance: each supported choice is observable at transmission and accepted by a compatible
receiver, with legacy defaults preserved unless explicitly changed. A helper-only result closes
only the helper obligation. Document unsupported coding or receive-path work separately.

## W4 — integrate A-MPDU into the production QoS data service

Owner: QoS data service, aggregation policy, Block Ack, and receive reordering.
Tests: `11n/N_Ampdu.test`, `11ac/Ac_Ampdu.test`.
Entry points: `OriginatorQosMacDataService`, `MpduAggregation`, selection policies and receive path.
Dependencies: W0; coordinate shared BA state/contracts with W1, without assuming delayed BA is required.

- [ ] Define eligibility from negotiated capabilities, recipient/TID, agreement state, size limits,
  and current transmission opportunity. Identify what currently prevents the aggregate-builder call.
- [ ] Connect selection, ownership transfer, queue removal, transmission, deaggregation, and BA
  progress. Uncommenting a builder call alone is not an integration repair.
- [ ] Exercise at least two eligible MPDUs through production queues for HT and VHT separately.
  Verify actual aggregate contents, delivery identities, and BA/retry progress.
- [ ] Cover aggregation disabled, ineligible peer/TID, size boundary, partial acknowledgement,
  retransmission, and agreement cleanup as applicable to the implemented path.

Acceptance: both protocol tests observe real transmitted aggregates and exactly-once delivery;
selective loss makes progress without discarding unacknowledged data or mixing recipient state.
Map affected fingerprints only after direct correctness evidence exists.

## W5 — define the supported Multi-TID Block Ack exchange

Owner: Block Ack frame/serializer, agreements, originator and recipient procedures.
Test: `11ac/Ac_MultiTidBa.test`.
Entry points: `Ieee80211Frame.msg`, serializers and existing Multi-TID rejection/declaration sites.

- [ ] Verify the exact standard revision, frame variant, and prerequisites behind this VHT-labeled
  test. Correct a specification/feature-association error before proposing implementation.
- [ ] Inventory missing per-TID request/response fields, serialization, agreement lookup, bitmap
  interpretation, and acknowledgement bookkeeping. Preserve the existing two-flow/TID prerequisites.
- [ ] Write a bounded implementation design covering absent/invalid TIDs, lengths, sequence wrap,
  and partial acknowledgement; identify shared state changes with W4.
- [ ] Implement representation/serializer checks before production exchange integration. Verify
  both TIDs in the response and independent progress; ordinary single-TID BA remains a control.

Acceptance: a justified variant and design first; closure requires an actual multi-TID exchange
with correct per-TID results. Seeing traffic on two TIDs or constructing a frame in isolation is insufficient.

## W6 — separate LDPC negotiation, code selection, and coding behavior

Owner: HT/VHT capability negotiation and PHY coding/mode API.
Tests: `11n/N_LdpcCap.test`, `11ac/Ac_Ldpc.test`.
Entry points: HT/VHT code and mode classes, capability producers/consumers, PHY transmission metadata.

- [ ] State three distinct claims: advertised/negotiated capability, selected transmit code,
  and actual encoding/decoding or error-model behavior. Trace support for each independently.
- [ ] Define a typed observable code choice at its production owner; do not invent a wire chunk
  or a constant test-only label. Verify that peer capability constrains the selection.
- [ ] Add negotiation/selection checks in the matching module/protocol category, with BCC-only
  peer and LDPC-disabled controls. Use algorithm tests for coding if a coding implementation exists.
- [ ] Replace the two `TESTABILITY GAP` assertions only when the matching observation is real.
  If coding is absent, file its bounded implementation plan separately from capability plumbing.

Acceptance: every passing assertion names the layer it proves. STBC remains a separate limitation;
a capability bit never serves as evidence of LDPC coding correctness.

## W7 — define noncontiguous VHT channel observation

Owner: VHT channel representation, transmitter, medium and receiver.
Test: `11ac/Ac_80p80.test`.

- [ ] Inventory whether two segment centers, occupied bandwidths, and primary-channel identity
  can travel from configuration through transmission to reception.
- [ ] Specify a minimal representation/API and affected spectrum, reception, and serialization
  contracts before changing fields. Reuse existing channel abstractions where sufficient.
- [ ] Design a module/PHY test distinguishing noncontiguous 80+80 from contiguous 160 MHz;
  include a compatible receiver and one mismatch or segment-specific interference case.
- [ ] Replace the explicit gap only after real segment observations exist. If medium/receiver
  support requires a larger change, record that dependency and a separate implementation plan.

Acceptance: two distinct occupied segments and correct receiver behavior are observed.
A scalar total bandwidth of 160 MHz cannot close this task.

## W8 — specify reverse-direction and MU-MIMO observation prerequisites

These are separate feature tasks; neither depends on the other.

| Task / owner | Concrete first deliverable | Evidence required for implementation closure |
| --- | --- | --- |
| `N_ReverseDirection` / MAC frame and TXOP procedures | Identify applicable RDG grant/response rules; map HT Control representation, serializer, grant owner, recipient eligibility, TXOP accounting and termination. Design typed fields and a deterministic grant trigger. | Field encode/decode checks plus a production grant and reverse exchange within its valid opportunity; no-grant and expired-grant controls; no duplicated TXOP ownership. |
| `Ac_MuMimo` / VHT MU scheduling and PHY reception | Map group membership, user allocation, peer capabilities and sounding dependencies. Specify how a transmitted PPDU exposes multiple users and how receivers select their own payloads. Coordinate `Ac_GroupIdMgmt` and `Ac_Beamforming` prerequisites in W9. | At least two intended users in a real PPDU, correct per-user delivery, and a non-member/single-user control. A group ID or multiple flows alone is insufficient. |

- [ ] Produce each bounded design and source map before implementation.
- [ ] Select serializer/unit, module/PHY, and protocol checks according to the claim.
- [ ] Keep explicit failing diagnostics until their production observations and stimulus exist.

## W9 — deepen the 17 retained probes when their prerequisites exist

Owner: the feature owner named below, with protocol-test maintenance.
First deliverable for every row: a check description naming revision, role, negotiated conditions,
trigger, payload/state observation, negative control, and matching category. If the producer is
absent, record the dependency; do not claim a complete procedure from a presence probe.

| Tests | Owner / required stimulus and acceptance |
| --- | --- |
| `WifiRrmMeasurement`, `WifiTpc` | Measurement management: issue a concrete request, match token/peer and requested measurement/report content; handle refusal/timeout. |
| `WifiCountryIe`, `WifiDfsChannelSwitch` | Regulatory/channel management: configure a domain; verify Country contents. Separately inject a supported radar/channel-switch trigger and verify announcement channel/count plus actual transition. |
| `WifiRsn4way`, `WifiPmf` | Security/key management: establish credentials and negotiation, parse EAPOL-Key stages/replay state; then verify protected robust management and rejection of an invalid/unprotected frame under the negotiated policy. |
| `WifiQosAddts`, `WifiQosUapsd` | QoS/power save: request TSPEC admission and check acceptance/refusal. Separately negotiate trigger/delivery ACs, buffer traffic, trigger a service period, verify delivery and EOSP with no-trigger control. |
| `N_Smps`, `N_2040Coex` | HT management: trigger a power-mode change and observe its operational consequence; separately create overlap/intolerance conditions and verify the required coexistence exchange/channel response. |
| `Ac_GroupIdMgmt`, `Ac_Beamforming` | VHT MU/sounding: assign group membership and validate payload/state; trigger sounding and validate feedback, including a non-member or missing-feedback control. Coordinate W8. |
| `Ac_OpModeNotification`, `Ac_VhtIe` | VHT management: change receive width/NSS and check notification and peer adaptation. Split VHT Capabilities from Operation; check actual advertised values and eligible/ineligible peer behavior. |
| `N_StbcCap`, `Ac_LdpcStbcCap` | PHY/STBC: preserve the explicit limitation until support is deliberately added. Any support work needs capability, transmit selection, and receive behavior checks; the historical VHT filename covers STBC only. |
| `Legacy_Pcf` | Legacy coordination: retain the explicit omission; any implementation needs an applicable older standard, point-coordinator configuration, a contention-free period, polling and response checks. |

- [ ] Account for every retained test in a prerequisite ledger; mark which can be deepened now.
- [ ] Add faithful stimuli only through actual feature contracts; tests must not implement the
  absent feature themselves. Review declarations again when a producer becomes available.

## W10 — verification, evidence, and completion

Use one deterministic seed (initially seed-set 0) for causal exchange checks. When a fix changes
random access, mobility, loss, or spectrum behavior, define a finite additional parameter/seed
campaign with its dimensions and failure rule before running it. Do not rerun until green.

Example commands, from the repository root with the normal environment sourced:

```bash
make MODE=debug -j$(nproc)
inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/wifi$' \
  -f '/WifiQosDelayedBa\.test$' --log-file /tmp/wifi-followups-delayed-ba.log
inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/wifi$' \
  -f '/(B_Pbcc|B_ShortPreamble|N_Greenfield)\.test$' \
  --log-file /tmp/wifi-followups-mode-selection.log
inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/wifi$' \
  --log-file /tmp/wifi-followups-full.log
```

These are planned commands, not executed evidence. Select additional affected cases from the changed
owner, not only these examples. The test filter matches full `.test` paths; zero selected cases is
NOT_RUN. Use the category-specific runner for algorithm/module/serializer tests. Avoid overlapping
runner invocations sharing generated directories. Preserve artifacts before subsequent runs overwrite them.

- [ ] For each fix, capture the failing pre-fix assertion and passing post-fix assertion, effective
  configuration and production path; include a meaningful negative/control case.
- [ ] Protect the six repaired tests: HT Capabilities, HT Operation, four-message authentication,
  deauthentication, reassociation, and deterministic retransmission. Also run existing neighboring
  immediate-BA, nonaggregated, and legacy-mode checks where their owners changed.
- [ ] After focused checks, run the full Wi-Fi suite once for interaction coverage. Inspect failures
  by stage and compare inventory with W0; add/split/category-moved tests must reconcile explicitly.
- [ ] Update `wifi/results.md` with fresh runs, per-test verdicts, six-class dispositions, owner paths,
  and unresolved blockers. Create category/coverage artifacts only from actual defined checks;
  do not invent catalog IDs or a feature support claim from these historical test names.
- [ ] Run applicable architecture/seal checks for changed source paths and review the general and
  IEEE 802.11 checklists. Before a push, run the documented project-wide gates and both build modes.
  Treat fingerprints/statistical expectations through their separate evidence/update procedures.
- [ ] Record each follow-up's commit or successor plan and remaining acceptance gaps.

A task is closed when its stated observation is verified or its design-stage deliverable explicitly
hands off the unresolved implementation with a concrete owner, blocker, and next action. The plan
is complete when all 13 deferred repair tasks and all 17 retained-probe tasks have such a disposition,
implemented repairs are verified, and the evidence agrees with the actual suite. The existing
expected-failure declarations alone do not close those tasks. Open feature work
must stay visible in successor plans; a disposition is not a passing test. Move this plan to
`plan/done/` when those deliverables land.

## Review correction record

The declaration commit now includes this plan in its tree. The suite README records
75 tests with 45 PASS and 30 FAIL expectations and delegates observed findings to the
tracked audit report. A fresh 13-test run against committed execution inputs produced
13 expected failures; the report contains the exact command, input digest, and run record.
Production follow-ups remain planned.
