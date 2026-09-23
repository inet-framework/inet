# Repair EDCA management recovery ownership

Status: implemented and verified in the working tree, 2026-09-23.
Base source: `9ccb5c8e59`. See the
[implementation and verification report](../evidence/ieee80211-edca-management-recovery/implementation.md)
for exact commands, 40 passing module runs, unchanged DCF/EDCA fingerprint
controls, debug/release builds, gate results, and retained limitations.
Prepared for commit on `fix/ieee80211-edca-management-recovery`; no push performed.

## Objective and boundary

For a management frame handled by an EDCAF, its recovery procedure must update that
EDCAF's contention window and management retry state. No other AC may change as a
side effect. This applies to transmission failure, successful acknowledgment, retry
exhaustion, and internal collision. An internal collision concerns the losing EDCAF,
which need not be the channel owner.

The bounded repair is to move the existing management/non-QoS recovery submodule
from `Edca` into each `Edcaf`, bind its existing CW callback permanently to that
EDCAF, and route HCF management outcomes through that local procedure. Reuse the
existing class, methods, signals, counters, and frame representation. Introduce no
new production interface, class, message, tag, signal, or timer.

This is an ownership/dispatch correction, **not a complete modernization of EDCA
recovery to IEEE 802.11-2024**. In particular, the current QoS and non-QoS algorithms
have separate station-counter representations and older retry/RTS-threshold rules.
Their unification and a full audit of QSRC/TXOP reset semantics are separate work.
The plan must not claim that relocating the procedure resolves those limitations.
It also does not add non-QoS data support to HCF's currently incomplete dispatch.

## Guidance and normative basis

Follow the [contribution workflow](../../doc/project/guide/contribute-a-change.md),
[developer chapter](../../doc/src/developers-guide/ch-80211.rst),
[implementation anatomy](../../doc/project/design/ieee80211-anatomy.md), and
[model architecture](../../doc/project/design/ieee80211-model-architecture.md).
The chapter's reliability, access, communication, and contracts sections establish
the existing division: HCF coordinates outcomes, recovery decides retry/CW actions,
and channel access owns the actual CW. The chapter's shared management procedure
is the composition being corrected, not a requirement to retain a defective owner.

Applicable project rules are
[AR-WLAN-MAC-QOS and ownership](../../doc/project/domain/ieee80211.md),
[typed direct calls, notifications, initialization, and minimal surface](../../doc/project/rule/architecture.md),
and [focused testing](../../doc/project/rule/testing.md).
Keep algorithm state in the MAC components, not the MIB; signals report results
and never select the target AC or complete the recovery action.

IEEE Std 802.11-2024, document ID `ieee80211-2024`:

| Normative clause | Obligation used here | Corpus node / physical PDF pages |
| --- | --- | --- |
| 10.23.2.1 | Independent EDCAFs represent the ACs | `ieee80211-2024:clause:10.23.2.1`, p. 1998 |
| 10.23.2.2 | CW and backoff recovery belong to the affected AC; internal collision affects the losing EDCAF | `ieee80211-2024:clause:10.23.2.2`, pp. 1999–2001 |
| 10.23.2.12.1 | Retransmit, discard, and internal-collision frame-retry obligations | `ieee80211-2024:clause:10.23.2.12.1`, pp. 2016–2017 |

These clauses were retrieved from the local 2024 base standard (the local filename
is `80211ax-2024.pdf`). No PDF-image inspection was needed for these obligations.
The backoff clause references retransmission and TXOP recovery rules; do not infer
their complete implementation from this ownership repair. Corpus lint reports
ambiguities elsewhere; the three cited clause identities resolve explicitly.
The implementation's 2012 citations must not be relabeled as 2024 compliance.

The seal registry currently protects `common/packet/`, not the source paths below.
Recheck [seals](../../doc/project/audit/seal-list.md) and both exception ledgers at
implementation time. No new architectural exception is proposed.

## Existing failure and coverage gap

`Hcf::processUpperFrame()` places management frames in `AC_VO`.
`Edca.ned` nevertheless configures its shared
`mgmtAndNonQoSRecoveryProcedure.cwCalculatorModule` as `^.edcaf[1]`, the BE EDCAF.
HCF supplies the actual EDCAF's `StationRetryCounters` to this shared procedure.
The procedure therefore combines one AC's counters with another AC's CW.

Affected HCF paths are `handleInternalCollision()`,
`originatorProcessRtsProtectionFailed()`, `originatorProcessFailedFrame()`, and
the ACK branch of `originatorProcessReceivedControlFrame()`. Associated count
queries and retry-limit cleanup must use the same procedure as the initial failure.
CTS and multicast completion currently call the QoS procedure without distinguishing
management; include their dispatch in the terminal-path work below.

`Ieee80211HcfInternalCollision_1.test` asserts VO/BE station counters and the terminal
management callback, but not CW. Its retry limit of one also bypasses a useful
CW-growth observation. `Ieee80211MgmtApHcfRtsTimeout_1.test` exercises the real RTS
timeout path, but subscribes to the shared procedure. Both need stronger assertions,
not merely updated paths.

## Ownership and communication after the repair

| State or responsibility | Owner / access |
| --- | --- |
| CW, CW bounds, pending/in-progress frames, contention | Existing `Edcaf` and its existing parts; recovery invokes `IRecoveryProcedure::ICwCalculator` |
| Management/non-QoS per-frame SRC/LRC maps | That EDCAF's `NonQosRecoveryProcedure` instance; no shared cross-AC map |
| Management/non-QoS station retry counters | Existing `Edcaf::stationRetryCounters` object; only the selected non-QoS recovery procedure performs its existing transitions |
| QoS-data recovery state | Existing per-EDCAF `QosRecoveryProcedure`, unchanged |
| Choice of affected EDCAF | `Hcf`, from the exchange AC or the explicit internally-collided EDCAF |
| ACK bookkeeping, retained frames, terminal management notification | Existing `QosAckHandler`, `InProgressFrames`, and HCF outcome path |

The C++ pointer to the new child instance is borrowed; the module hierarchy owns
the child. `Edcaf` continues to own and delete its existing station-counter object.
No additional writable copy of CW or retry counters is introduced.

Calls remain synchronous typed calls:

```text
Hcf outcome for edcaf
  -> edcaf->getMgmtAndNonQoSRecoveryProcedure()
  -> existing recovery method(..., edcaf->getStationRetryCounters())
  -> IRecoveryProcedure::ICwCalculator on that same edcaf
  -> CW change and existing observation
```

## Interface, class, configuration, and signal inventory

Paths in the next table are relative to `src/inet/linklayer/ieee80211/mac/`.

| Artifact | Disposition | Planned change or use |
| --- | --- | --- |
| `contract/IRecoveryProcedure.h`, nested `ICwCalculator` | Reuse unchanged | `incrementCw()`, `resetCw()`, `getCw()` already express the required callback; no AC argument or new callback interface |
| `originator/NonQosRecoveryProcedure.{h,cc,ned}` | Reuse unchanged | Instantiate once per EDCAF; retain algorithms, API, retry-limit parameters, declarations, and signal payloads |
| `channelaccess/Edcaf.{h,cc}` | Modify | Add a borrowed `NonQosRecoveryProcedure *mgmtAndNonQoSRecoveryProcedure`; resolve the child alongside existing local-stage peer lookups; add `getMgmtAndNonQoSRecoveryProcedure() const` with the existing naming/signature convention |
| `channelaccess/Edcaf.ned` | Modify | Import and instantiate `mgmtAndNonQoSRecoveryProcedure: NonQosRecoveryProcedure`; set `cwCalculatorModule = "^"` and `rtsPolicyModule = "^.^.^.rtsPolicy"` |
| `channelaccess/Edca.{h,cc,ned}` | Modify | Remove the shared child, member, initialization lookup, getter, and now-unused import/include; retain EDCAF composition/classification |
| `coordinationfunction/Hcf.cc` | Modify | Select recovery from the affected EDCAF for every management operation, including count queries and terminal cleanup; correct matching CTS/multicast dispatch as described below |
| `Hcf.h` | Reuse unchanged | No new public operation or member is needed |
| `StationRetryCounters`, `QosRecoveryProcedure`, `QosAckHandler`, `InProgressFrames` | Reuse unchanged | Existing state and APIs; no new counter provider or recovery base class |
| `IRtsPolicy`, `IChannelAccess`, `IContention`, `IEdcaCollisionController` | Reuse unchanged | Existing threshold, channel access, backoff, and collision contracts |
| `IRateControl`, `FrameTransmissionDetails`, `Ieee80211Mac` | Reuse unchanged | Preserve count feedback, terminal status payload, and signal identifier |
| `Dcf`, `Dcaf`, DCF NED composition | Unchanged control | Continue using their existing non-QoS recovery instance and CW target |
| New production types/contracts | None | Only additional instances of an existing NED/C++ type |

No protocol gate, wire field, serializer, capability, feature gate, PHY parameter,
or mode-set change is required. Enablement remains `qosStation = true` and the
existing management-frame path.

### Existing signals

| Signal | Publisher and scope after repair | Contract |
| --- | --- | --- |
| `IRecoveryProcedure::contentionWindowChangedSignal` (`long`) | `edca.edcaf[ac].mgmtAndNonQoSRecoveryProcedure` for non-QoS recovery; existing QoS publisher remains | Existing initialization sample, then existing emissions when CW actually changes. AC is identified by source module. No forwarding/re-emission at `Edca` or HCF |
| `IRecoveryProcedure::retryLimitReachedSignal` (`Packet`) | Same per-AC non-QoS procedure | Existing retry-limit event; packet remains borrowed during the callback |
| `Ieee80211Mac::frameTransmissionOutcomeSignal` | Existing HCF publisher | Preserve `ACKNOWLEDGED`, `RETRY_LIMIT_REACHED`, and `DROPPED_BEFORE_TRANSMISSION` outcomes and exactly-once management completion |
| `packetDropped`, `packetSentToPeer`, `channelOwnershipChanged`, contention observations | Existing publishers | Reuse for drop, on-air attempt, and contention evidence; no new signal |

Keep existing signal IDs and NED declarations. Listeners must filter source/AC;
subscriptions to an ancestor receive several recovery streams. Do not concatenate
them into a single CW time series. Initial samples from the additional instances
are initialization observations, not four retry events. Signal emission does not
transfer packet ownership; fixtures must copy any values they retain.

### Configuration migration

Move `...edca.mgmtAndNonQoSRecoveryProcedure.<parameter>` to
`...edca.edcaf[*].mgmtAndNonQoSRecoveryProcedure.<parameter>` when preserving one
setting for all ACs, or use `[3]` for the existing VO-only management fixtures.
Move signal subscriptions and result-file paths similarly. Preserve defaults
`shortRetryLimit = 7` and `longRetryLimit = 4`.

Remove the old `Edca` getter rather than keeping a compatibility accessor that
silently chooses BE, VO, or the current owner. Document the C++/NED path migration;
do not introduce a second configuration authority just to preserve the old path.
Search source, tests, examples, showcases, and documentation for every old reference.

## Implementation sequence

1. **Establish a failing CW observation.** Extend the internal-collision fixture
   with a nonterminal case using retry limit greater than one and distinct CW
   settings. Read all four `Edcaf::getCw()` values before and after the HCF call.
   Assert VO grows while BE/BK/VI do not. Also retain the existing terminal case.
   Record the pre-fix failure before changing production code.
2. **Relocate composition and lookup.** Apply the `Edca`/`Edcaf` changes above.
   Resolve the child at `INITSTAGE_LOCAL` without invoking its protocol methods.
   Its existing `INITSTAGE_LAST` binding then observes a CW initialized during the
   link-layer stage. Existing `NUM_INIT_STAGES` coverage is sufficient. No static
   dependency is distributed through a signal.
3. **Make every management outcome select the local procedure.** In internal
   collision use the loop's losing `edcaf`, not `getChannelOwner()`. In RTS failure,
   data/management failure, and ACK handling use the EDCAF already identified by
   that production exchange. Use the same local instance for failure, count query,
   retry-limit check, and cleanup. Preserve existing packet ownership, ACK-state
   operations, rate-control feedback, Retry-bit handling, and terminal notification.
4. **Close matching success/reset paths.** For multicast completion, inspect the
   transmitted frame before dropping it: management uses the local non-QoS
   `multicastFrameTransmitted(stationCounters)`, QoS data keeps its existing path.
   For a received CTS, identify the data/management frame protected by the active
   RTS exchange using the existing exchange/in-progress state; the last transmitted
   header is an RTS and cannot itself classify the protected frame. Management
   calls the local non-QoS `ctsFrameReceived(stationCounters)`. Add no cached
   "last recovery owner". Do not use an unrelated pending-queue front as evidence.
5. **Migrate fixtures and document the resulting composition.** Update the two
   existing direct tests and add the bounded coverage below. Update the developer
   chapter's reliability/state/contracts descriptions and remove only the resolved
   shared-CW defect from its gaps section. The anatomy is explicitly a historical
   snapshot: preserve its old facts and add a dated link to the current guide,
   rather than silently claiming the snapshot described the new composition.
6. **Validate and review.** Run the focused build/tests/gates below, check the final
   call inventory, and record remaining limitations. Keep regression expectations
   separate from the implementation until their changes have causal evidence.

Do not alter unrelated threshold inequalities, retry-limit arithmetic, TXOP
predicates, aggregation, or lifecycle behavior while moving the recovery owner.
If a required test exposes one of those issues, report it separately and reassess
scope before claiming broader standards compliance.

## Verification design

Use [module tests](../../doc/project/design/test-anatomy.md): the main claim is
production dispatch and state ownership, not just an arithmetic helper. Use a
single deterministic seed for injected transitions. A synthetic call to the HCF
collision handler establishes dispatch only; it does not prove collision detection
or demonstrate that VO can lose an internal collision with normal four-AC priorities.

| Case | Stimulus and required evidence |
| --- | --- |
| Internal-collision ownership | Extend `Ieee80211HcfInternalCollision_1.test`: nonterminal injected management collision, all four CW/counter snapshots, only the passed EDCAF changes. Retain exactly-once terminal callback case. Inject a different AC in a separate fixture case to reject a hardcoded VO repair; label this as dispatch coverage, not classifier behavior |
| Actual RTS/CTS timeout | Extend `Ieee80211MgmtApHcfRtsTimeout_1.test`: preserve production management enqueue, RTS and missing CTS; assert VO CW growth, unchanged BE, local retry-limit signal, exact attempt/drop/association-notification counts |
| Failed ACK then successful ACK | Proposed `Ieee80211HcfManagementRecovery_1.test`: real HCF management exchange, deterministic ACK suppression then delivery; VO CW grows then resets; precondition BE at a distinct nonminimum CW so an accidental BE reset is detectable |
| CTS success and multicast completion | Same new fixture: management station-counter resets reach the local non-QoS instance; QoS-data counters are not reset through the wrong path; no ACK wait/retry is introduced for group-addressed management |
| CW boundaries and cleanup | Cases with CW below and at CWmax; retry limit 1 and 2; successful completion and terminal discard followed by a new frame. Assert no unexpected cross-AC updates, duplicate terminal signals, or stale frame retry entries |
| Other traffic and DCF controls | Interleave QoS data with management and retain a non-QoS DCF management timeout control. Assert unchanged QoS/DCF paths and intended per-AC isolation; do not claim unified QoS/non-QoS station-counter semantics |

Test-only subclasses/listeners are permitted where the existing module-test pattern
requires them. They may expose protected observations or supply deterministic
stimuli; they must not copy the HCF dispatch under test or directly repair CW.
No new production query exists solely for a fixture: `Edcaf::getCw()` and the
existing station-counter queries already provide the needed observations.

For the on-air timeout/ACK cases, run a bounded seed set `{0, 1, 2}` with the same
deterministic fault rule, short/long frame cases, and the named CW/retry boundaries.
Any invariant failure fails the campaign; do not rerun until green. This checks
ordering sensitivity, not a statistical throughput/fairness claim.

Planned commands from the repository root, after the new fixture exists:

```bash
make MODE=debug -j$(nproc)
inet_run_module_tests -m debug -f 'Ieee80211(HcfInternalCollision|HcfManagementRecovery|MgmtApHcfRtsTimeout)_1\.test$'
inet_run_module_tests -m debug -f 'Ieee80211MgmtAp(HcfQueueDrop|QueueDrop|Timeout)_1\.test$'
doc/project/enforcement/check-architecture.sh src/inet/linklayer/ieee80211
```

The runner and existing named fixtures are present; `HcfManagementRecovery_1` is
explicitly new planned coverage. Implement its parameter/seed matrix in the test
definition or separate explicitly selected cases. Record per-case selection,
configuration, seed, mode, exit status, and artifacts. Zero selected cases means
NOT_RUN. These were the planned commands; the linked report records the exact executed
filters, fixture corrections, outcomes, and artifacts.

Map affected DCF/EDCA scenarios to exact fingerprint cases after the source/test
change is settled; selection is a required implementation deliverable, not an
unfiltered substitute for the assertions above. Diagnose expected management-loss
trajectory changes separately from unaffected controls. Follow
[baseline policy](../../doc/project/guide/change-a-baseline.md) before any baseline
update and [the before-push gates](../../doc/project/guide/run-the-gates.md), including
debug/release compilation. A helper pass or fingerprint match alone cannot close
the ownership claim.

## Alternatives and retained limitations

- Changing `edcaf[1]` to `edcaf[3]` fixes today's normal classification only. It
  leaves one permanently selected AC behind a shared service and fails the
  different-AC/collision dispatch invariant.
- Making `Edca` a CW callback proxy that consults the channel owner fails for a
  losing EDCAF and hides the actual target behind transient global state.
- Rebinding one shared calculator pointer before each call introduces mutable
  routing state and leaves signal provenance ambiguous. Passing the target through
  every recovery method could work, but changes the shared DCF API and observer
  contract unnecessarily for this bounded repair.
- Consolidating management and QoS-data recovery into one revised algorithm per
  EDCAF may be the eventual design. It requires a separate standards audit of
  frame identity, station retry counters, CW reset events, and retry limits; it is
  not required to eliminate cross-AC mutation now.

Ordinary stop/crash/restart reset remains the documented MAC lifecycle gap. This
plan adds no timers and no lifecycle claims; module destruction releases the new
child instances through the normal hierarchy. Recovery-map sequence identity and
wrap behavior retain their current limits. No MIB state or duplicate authority is
added to compensate for those limitations.

## Completion criteria

- [x] No shared management recovery child/getter remains in `Edca`; every EDCAF's
  local procedure resolves its CW callback to that same EDCAF.
- [x] Every management failure, query, cleanup, ACK, CTS, and multicast path uses
  the intended local procedure; losing-AC dispatch does not consult the winner.
- [x] Direct pre-fix failure and post-fix success demonstrate CW isolation; all
  focused cases retain correct packet disposition and exactly-once notifications.
- [x] Configuration/result-path migration and current guide text match the tree;
  historical snapshot wording remains honest.
- [x] No new production class, interface, signal, timer, wire field, or shared state;
  no incidental change to DCF/QoS algorithms or unrelated gaps.
- [x] Build, test, architecture, and selected regression evidence are recorded;
  broader EDCA-counter/lifecycle limitations remain explicit.

Implementation and required scoped evidence are complete. The global gates still
report unrelated pre-existing violations and an empty-range commit-checker error;
they are recorded rather than presented as clean. RTS short-map cleanup remains
a separate pre-existing algorithm issue, as required by the bounded scope. The
ACK/data-failure cleanup cases pass; universal RTS-map cleanup is not claimed.
The original inventory and design above describe the implemented ownership repair.
