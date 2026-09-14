# Onoe multirate retry contract

Status: design step completed, 2026-09-10. Production implementation pending.
Inspected source baseline: `73d5342260`.

Follow-up 3 of [the alignment plan](onoe-reference-alignment.md).

## Scope and reference boundary

Add an optional legacy normal-ACK retry series without changing recovery retry
limits or Onoe's completed-sample arithmetic. Start with individually addressed
data and management frames whose rate is selected by Onoe. Fixed-rate overrides,
group frames, control responses, Block Ack, aggregation and HT/VHT remain on
their existing selection paths. Introduce `multirateRetry = false` on Onoe so
existing simulations retain their behavior until explicitly enabled.

The pinned [MadWifi reference](https://github.com/proski/madwifi/blob/a7531fd223a1f454d3fd74a975b4581cde5411bb/ath_rate/onoe/onoe.c)
is revision `a7531fd223a1f454d3fd74a975b4581cde5411bb`. The parent assessment
records a 4/2/2/2 series, a lowest-hardware-rate final stage and disabled
redundant stages. This turn could not retrieve that source: both web URLs
failed and curl failed DNS resolution. These details are inherited evidence,
not a new reference verification. Before implementing the builder, inspect
`ath_rate_findrate`, `ath_rate_setupxtxdesc`, and `ath_rate_update`, and record
the exact low-rate predicates and total-tries interpretation in test cases.
Do not substitute four successive slower rates or redistribute disabled tries.

INET has no negotiated legacy peer-rate list in the current controller contract.
Its local slowest legal mode is a model mapping, not automatically MadWifi's
lowest hardware rate. Resolve that mapping explicitly for each supported legacy
mode set; do not claim hardware-status or negotiated-rate equivalence.

## Findings that determine the contract

- `RateSelection::computeMode()` and `QosRateSelection::computeMode()` receive a
  packet, but their adaptive branch calls only `IRateControl::getRate(receiver)`.
  No current argument identifies the attempt or captures a packet's base rate.
- `Dcf::transmitFrame()` and `Hcf::transmitFrame()` select and tag the mode before
  calculating Duration/ID and handing the packet to Tx. These are the actual
  transmission paths that must consume the selected attempt mode.
- `OriginatorProtectionMechanism` and `SingleProtectionMechanism` also query
  `computeMode()` for pending packets, including before RTS transmission.
  Queries must be repeatable and must never consume attempts. The mode reserved
  in RTS duration calculations must match the protected data transmission.
- Recovery SRC/LRC include RTS failures and internal collisions. Neither their
  sum nor the length-selected legacy retry count is a data-attempt ordinal.
  Reusing them would move the series without a data transmission.
- `InProgressFrames` owns the original MPDUs across exchanges, including
  fragments, and retires them through `dropFrame()`/`dropFrames()` and destruction.
  `FrameSequenceHandler` contexts end between exchanges; they cannot alone own
  state that must survive another contention and retry.
- QoS recovery keys include receiver, TID and sequence/fragment. Non-QoS recovery
  currently keys by sequence/fragment. A new series must not inherit that narrower
  key or use one mutable series per receiver.

## Proposed typed interface and ownership

Use protocol-local value types in `mac/common/`, with no packet wire fields or
changes to the sealed packet subsystem:

- `RateRetryStage`: borrowed mode pointer and positive data-transmission count.
- `RateRetrySeries`: bounded ordered stages, with an explicit disabled state;
  an empty series means ordinary dynamic selection, never packet exhaustion.
- `RateRetryContext`: one captured series, mode-set generation, number of
  completed data transmissions, and an optional prepared-attempt mode. It does
  not copy SRC/LRC, own packets, or update Onoe peer statistics.

`IRateControl` declares a pure virtual series request; `RateControlBase` returns
disabled, and Onoe constructs a series from its current peer mode when enabled.
Existing `getRate()` and feedback methods keep their meanings. AARF must keep
querying its live rate per attempt; freezing a one-stage AARF series would change
its feedback-driven behavior. Direct interface implementers need the new method;
document the migration.

`InProgressFrames` owns one context for each original resident MPDU. Prefer a
sidecar keyed by that owned packet, erased before retirement; packet identity is
scoped to residence, not a pointer retained after deletion. Diagnostic identity
includes receiver, frame class, TID where applicable, sequence and fragment.
Transmission duplicates resolve to their original MPDU through an explicit
coordination argument, never through an assumed matching pointer or MAC sequence
number alone. Other components receive const context views or copied values.

Rate selection owns override precedence, applicability and peer compatibility.
Extend both selection contracts to prepare/query the attempt mode with an
explicit context; keep existing ordinary queries available for unsupported
traffic. Coordination supplies the correct in-progress owner and context to
selection and protection. Protection must use the prepared pending-frame mode,
rather than making a second independent series decision. No concrete Onoe
downcast, module-path lookup from protection to recovery, or writable controller
history map is needed.

## Attempt and terminal semantics

1. Prepare the first attempt before its earliest duration reservation (including
   RTS or preceding-fragment lookahead). Capture the series once for that MPDU.
   Repeated preparation returns the same mode. Peer adaptation caused by another
   packet cannot change this already captured series.
2. An actual completed data/management transmission consumes one try. RTS/CTS,
   internal collisions, failed channel access, and duration queries consume none.
   Use the existing transmitted-frame notification for this transition, with an
   explicit prepared/in-flight/completed state to reject duplicate consumption.
   ACK failure allows preparation of the next try; ACK success retires context.
3. For the full reference series, zero-based data-attempt ordinals 0–3 use stage
   0, 4–5 stage 1, 6–7 stage 2, and 8–9 stage 3. Low-rate disabled stages are
   omitted using the verified reference predicates. Test every boundary.
4. Recovery and frame-sequence machinery remain the sole authority for retry
   exhaustion and CW behavior. They can terminate before any series boundary.
   If recovery permits attempts beyond the finite series, hold its last enabled
   mode. This tail is an explicit INET policy; it is not hardware descriptor
   exhaustion equivalence. A series must not silently grant extra MAC retries.
5. Existing success, data-exhaustion, RTS-exhaustion and internal-collision-drop
   callbacks report one terminal completion with current recovery totals before
   cleanup. They do not report one completion per series stage. Abandonment or
   lifecycle cleanup must not invent a success or a transmitted data failure.
6. Erase the sidecar on every original-MPDU retirement and owner destruction.
   Identity reuse starts fresh. A mode-set change invalidates borrowed modes:
   discard the old series and prepare afresh against the new generation before
   another exchange; keep actual recovery counters. If a protected exchange is
   already committed, abort/restart it through the existing exchange machinery
   before using a newly selected mode. Resolve the exact signal/abort ordering
   in the implementation contract before writing that path.

## Change surface and implementation sequence

All named MAC targets are unsealed in the inspected registry. No new architecture
or naming exception is proposed. Follow the current
[contributor route](../../doc/project/guide/contribute-a-change.md),
[WLAN ownership rules](../../doc/project/domain/ieee80211.md), and
[test rules](../../doc/project/rule/testing.md).

1. Verify the reference boundary table and mode mapping. Trace Tx duplication,
   transmitted callbacks, mode-set signal ordering and all original-MPDU removal
   paths. Resolve those details in a source-backed implementation contract.
2. Add value types, pure interface operations and base defaults, then Onoe's
   opt-in builder and its deterministic boundary tests. Expected files:
   `mac/common/RateRetry*.h`, `mac/contract/IRateControl.h`,
   `mac/ratecontrol/{RateControlBase,OnoeRateControl}.{h,cc}`, Onoe NED and the
   rate-control migration documentation. Keep interface headers free of policy.
3. Add resident-MPDU context ownership in `mac/queue/InProgressFrames.{h,cc}`;
   thread it through `IRateSelection.h`, `IQosRateSelection.h`, both selection
   implementations, `Dcf.{h,cc}`, `Hcf.{h,cc}` and both originator protection
   implementations. Check frame-sequence/Tx paths before extending their APIs.
   Recovery arithmetic requires no proposed change.
4. Add real DCF/HCF attempt evidence and cleanup tests; update Onoe documentation
   with enablement, finite-series tail policy and compatibility limits.

## Required verification

New proposed tests: `OnoeRateControlRetrySeries_1.test` for the builder and
`OnoeRateControlMultirateRetry_1.test` for actual exchanges. They do not exist yet.

| Case | Direct assertion |
| --- | --- |
| Full series, MAC limits permitting ten attempts | Actual PHY mode sequence follows 4/2/2/2 |
| Each low starting rate | Exact reference stage disabling and final-stage mapping |
| MAC limit before/at/after a stage boundary | No extra attempt; correct terminal callback |
| MAC limit beyond series capacity | Last enabled mode retained until MAC termination |
| RTS loss or internal collision before data | No data-stage advancement; correct completion total |
| Repeated duration queries and RTS protection | No consumed try; reserved and transmitted data modes match |
| Success at each stage | One completion and no later attempt |
| Peer, TID and fragment interleaving | Independent series and cleanup on identity reuse |
| Another packet changes the peer rate | Captured series remains stable |
| Mode-set reset and owner destruction | No stale mode or resident context |
| Disabled feature, AARF, fixed overrides and excluded traffic | Existing selection behavior retained |

Extend the real-radio fixture pattern in `OnoeRateControlRtsFeedback_1.test` to
record the transmission object's actual PHY mode on every data attempt. A
stored-peer-rate assertion, request tag alone, or datarate-selected signal alone
does not prove that Tx emitted that rate. Check Duration/ID against authoritative
mode durations and existing response selection. Seed 0 is appropriate for
deterministically forced losses; do not claim throughput validation.

After implementation, build fresh debug artifacts and run the explicit filter
`(OnoeRateControl|AarfRateControl|Ieee80211RecoveryRetryTotals|Ieee80211HcfInternalCollision).*\.test`
through `inet_run_module_tests -m debug -f`, plus directly mapped protection and
mode-selection cases discovered from the final diff. Run the MAC architecture
gate and naming gates for added artifacts. Record nonzero case counts, commands,
exit statuses and logs. Baseline changes require their own concrete proposal.

## Design-step validation

Source inspection established ownership, current callers, override precedence,
duration-query reentry and removal paths. The design intentionally separates
data-transmission progress from recovery totals. No production implementation,
behavioral test result, independent review or hardware equivalence is claimed.
Validate this plan with whitespace and local-link checks. Skill-package checks
from AGENTS.md are unavailable in this checkout and unrelated to this artifact.
