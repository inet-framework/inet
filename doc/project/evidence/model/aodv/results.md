# AODV correction execution record

> **Kind:** report · **Status:** snapshot 2026-11-09 · **Seal:** none · **Owns:** — · **Stands on:** [coverage.md](coverage.md), [correction decisions](conformance.md#selected-correction-decisions-2026-11-09)

This record covers the selected freshness corrections implemented so far. The full correction plan remains
in progress. It is not a claim of complete RFC conformance or completion of the §7 acceptance matrix.

## Environment and revisions

- Checkout: `/home/miguel/workspace/inet-aodv`, branch `fix/aodv-rfc3561`.
- Base: upstream master `7ec0d4d3f3e0e0e1357e2449b0bdcbb6ef0e1ce4`.
- Immediate parent of the freshness correction: preparation commit
  `185bc12f6b80ebc3d61d2cfa271dceda7d34f15d`.
- Parent AODV source tree: `0f2f3b90f4751534e8658cd2af2aacce6abe2972`.
- Corrected AODV source tree: `5d9a9a632488be44ed84a3ca86023b8c3636d7c4`.
- OMNeT++: 6.4.0aipre2, Clang, debug INET library. Default project features include IPv4 and IPv6.
- Protocol fixtures: configuration General, run 0, seed-set 0, zero broadcast and periodic jitter,
  fixed positions, AckingMac and UnitDisk radio.
- Parent and corrected tests used the same fixture inputs in the dedicated checkout, with a fresh
  matching library build before each source state. A second separate parent checkout has not yet
  been built. The plan's additional isolated-worktree and feature-profile checks remain pending.

## Builds and preliminary failures

From the checkout root, `make -j8 MODE=debug` initially reached linking but failed because eight
object files were missing after an overlapping runner-triggered build was stopped. A single
subsequent invocation of the same command regenerated them and linked successfully, exit 0.
Logs: `/tmp/inet-aodv-debug-build.log`, `/tmp/inet-aodv-debug-build-retry.log`.

After the source correction, `make -j8 MODE=debug` rebuilt AODV and linked successfully, exit 0.
Logs: `/tmp/inet-aodv-freshness-build.log`, `/tmp/inet-aodv-freshness-build-final.log`.

Initial fixture attempts exposed a wrong relative NED path, a generated C++ namespace mismatch,
and an unset display-derived mobility position. These were setup failures, not reproductions.
The first known-floor control then reached the originator-validity assertion and failed. That
scenario is covered by `Rfc3561ReverseRoute`; the preparation control uses U set and passes.
No expectation markers hide these failures.

## Parent and corrected results

All test names below are under `tests/protocol/aodv/` and have the prefix `Rfc3561` and suffix `.test`.

| Case | Parent observation | Corrected observation |
| --- | --- | --- |
| Control | PASS, reply sequence 0 and known reverse sequence 1 | PASS |
| SequenceFreshness | FAIL at t=0.25, wrapped reply does not produce the required stored sequence | PASS |
| ReverseRoute | FAIL at t=0.2, originator sequence remains unknown for the known-floor request | PASS |
| DestinationSequence | FAIL at t=0.2, reply does not adopt the several-ahead requested sequence | PASS |
| NeighborSequence | FAIL at t=0.2, stale direct reply replaces retained sequence information | PASS |
| RequestFloors | FAIL at t=0.15, outgoing unknown flag/sequence does not match absent knowledge | PASS |
| HelloAcceptance | FAIL at t=0.15, zero-lifetime Hello creates a route | PASS |
| ErrorFreshness | FAIL at t=0.25, stale error changes the recovered route | PASS |
| ReplyAcceptance | FAIL at t=0.2, stale reply replaces sequence 20 while discovery has queued data | PASS, equal-sequence longer path releases one datagram and cancels discovery |

The first ReplyAcceptance run stopped earlier when the peer attempted to forward the incorrectly
released data and GlobalArp could not resolve its final destination. The fixture now disables peer
forwarding, observes the datagram on real IPv4 reception, and lets IP dispose of it. Repeating this
corrected fixture on the parent reached the intended freshness assertion. Only that latter result
is counted as the reproduction.

Commands, all from the checkout root:

```sh
opp_run_protocol_tests --load . -p inet -m debug -w aodv \
  -f 'Rfc3561Control\.test$' --no-build --no-concurrent \
  --log-file /tmp/inet-aodv-control-parent-runner.log

opp_run_protocol_tests --load . -p inet -m debug -w aodv \
  -f 'Rfc3561(SequenceFreshness|ReverseRoute|DestinationSequence|NeighborSequence|RequestFloors|HelloAcceptance|ErrorFreshness|ReplyAcceptance)\.test$' \
  --no-build --no-concurrent --log-file /tmp/inet-aodv-freshness-parent-runner.log

opp_run_protocol_tests --load . -p inet -m debug -w aodv \
  -f 'Rfc3561ReplyAcceptance\.test$' --no-build --no-concurrent \
  --log-file /tmp/inet-aodv-reply-parent-runner.log

opp_run_protocol_tests --load . -p inet -m debug -w aodv \
  -f 'Rfc3561(Control|SequenceFreshness|ReverseRoute|DestinationSequence|NeighborSequence|RequestFloors|HelloAcceptance|ErrorFreshness|ReplyAcceptance)\.test$' \
  --no-build --no-concurrent --log-file /tmp/inet-aodv-freshness-final-runner.log

opp_run_unit_tests --load . -p inet -m debug -f 'AodvSequence\.test$' \
  --no-build --no-concurrent

opp_run_module_tests --load . -p inet -m debug \
  -f 'AODV(SimpleTest(_2)?|ShortestPath|LifecycleTest)\.test$' \
  --no-build --no-concurrent
```

Parent control: exit 0. Parent regression selection: exit 1, seven intended assertions and the
initial ReplyAcceptance fixture failure. Corrected ReplyAcceptance fixture on parent: exit 1 at
the intended assertion. Corrected protocol selection: exit 0, nine observed passes. Unit selection:
exit 0, one observed pass. The unit test is supplemental arithmetic coverage, not a parent runtime
reproduction. The causal explanation for each protocol test is in
[route-freshness.md](../../../../../tests/protocol/aodv/route-freshness.md).

The four existing AODV module controls also pass, exit 0. An earlier implementation attempt
suppressed a returned self-originated request before learning its separate previous-hop route,
moving recorded route-addition timing and failing these controls. Preserving that independent
neighbor observation before suppression restored all four unchanged expectations. The final
source was rebuilt and all nine protocol checks passed again. No existing expectation was edited.

Parent outputs and fixture snapshot are preserved at `/tmp/inet-aodv-parent-evidence/`.
Corrected outputs are under `tests/protocol/aodv/work/Rfc3561*/`, with runner logs above.
Unit runner output is `/tmp/inet-aodv-sequence-unit.log`. These are local artifacts.
Existing module runner output is `/tmp/inet-aodv-existing-module-final.log`.

## Gates and remaining scope

Scoped architecture, commit-format/classification checks for the preparation commit, release-note
format, seal-index and diff-whitespace checks passed. Scoped naming reports the pre-existing
unchanged `AodvControlPackets.msg` umbrella filename as a mechanical candidate.

Full release compilation subsequently passed on the freshness source, exit 0, with log
`/tmp/inet-aodv-release-build.log`. The complete before-push gate set, feature-isolated builds, IPv6 runtime
compatibility, fingerprints, diagram rendering and the remaining behavioral plan stages are not
established by this record. No existing baseline was changed.

## Lifetime stage

Immediate parent: `67bb7ba7b5cc7768c4c0efef08c990217bf475f2`, AODV source tree
`5d9a9a632488be44ed84a3ca86023b8c3636d7c4`. Corrected source tree:
`0244022aa958decc01702b71e36e73337a4d89e0`. The parent and corrected runs use unchanged
fixture inputs, fresh debug libraries and the same environment as above. The parent source was
restored and rebuilt for a final comparison after fixture corrections. Separate checkout and
feature-profile validation remain pending.

| New protocol case | Parent | Corrected |
| --- | --- | --- |
| Rfc3561ReverseLifetime | FAIL at 100.1 s, no independent reverse expiry extension | PASS, active after old expiry and inactive after the refreshed deadline, sequence retained |
| Rfc3561ReverseLifetimeLong | FAIL at 100.1 s, newer path shortens an existing later deadline | PASS, later deadline retained and eventually observed by expiry processing |
| Rfc3561GratuitousReply | FAIL at 100.15 s, absolute expiry advertised as duration | PASS, remaining whole-millisecond duration and real recipient expiry equal to reception plus duration |
| Rfc3561SubmillisecondReply | FAIL at 100.1 s, zero-lifetime reply emitted | PASS, no reply and normal request forwarding |
| Rfc3561MillisecondReply | PASS | PASS, 1999 us advertises 1 ms, control |
| Rfc3561MaximumLifetimeReply | PASS | PASS, exact UINT32_MAX millisecond field, control |
| Rfc3561OversizedLifetimeReply | FAIL at 100.1 s, unrepresentable advertisement emitted | PASS, no reply and normal request forwarding |

Four new module tests, `AodvAdvertisementLifetimeInvalid_1.test` through `_4.test`, all fail on
the parent because it completes initialization and reaches the 10 ms limit instead of rejecting
the invalid duration. On the corrected library all four pass by observing simulation exit 1,
the field-specific initialization diagnostic and no undisposed-object output. They cover below-one-
millisecond and oversized destination and Hello durations. Their other configured time relationships
are valid. These are initialization rejection tests, not expected-failure markers.

Exact focused commands, from the checkout root:

```sh
# Parent source, after restoring the immediate parent's Aodv.cc and Aodv.h:
make -C src -j8 MODE=debug
opp_run_protocol_tests --load . -p inet -m debug -w aodv \
  -f 'Rfc3561(ReverseLifetime(Long)?|GratuitousReply|SubmillisecondReply|MillisecondReply|MaximumLifetimeReply|OversizedLifetimeReply)\.test$' \
  --no-build --no-concurrent --log-file /tmp/inet-aodv-lifetime-parent-final-runner.log
opp_run_module_tests --load . -p inet -m debug \
  -f 'AodvAdvertisementLifetimeInvalid_[1-4]\.test$' --no-build --no-concurrent

# Corrected source, with fresh matching debug/release libraries:
make -C src -j4 MODE=debug
make -C src -j4 MODE=release
opp_run_protocol_tests --load . -p inet -m debug -w aodv \
  -f 'Rfc3561.*\.test$' --no-build --no-concurrent \
  --log-file /tmp/inet-aodv-lifetime-fixed-runner.log
opp_run_module_tests --load . -p inet -m debug \
  -f '(AODV(SimpleTest(_2)?|ShortestPath|LifecycleTest)|AodvAdvertisementLifetimeInvalid_[1-4])\.test$' \
  --no-build --no-concurrent
```

Parent protocol selection: exit 1, five intended assertion failures and two controls passing.
Parent module selection: exit 1, all four missing required initialization rejection. Corrected
protocol selection: exit 0, 16 passes. Corrected module selection: exit 0, eight passes, including
the four existing integration controls. Debug and release builds pass, exit 0. No baseline changed.

Logs: `/tmp/inet-aodv-lifetime-parent-final-build.log`,
`/tmp/inet-aodv-lifetime-parent-final.log`, `/tmp/inet-aodv-parameter-parent-final.log`,
`/tmp/inet-aodv-lifetime-debug-build-final.log`, `/tmp/inet-aodv-lifetime-release-build-final.log`,
`/tmp/inet-aodv-lifetime-fixed.log`, `/tmp/inet-aodv-lifetime-module-fixed.log`.
Parent outputs and fixture snapshot: `/tmp/inet-aodv-lifetime-parent-evidence-final/`.

Initial boundary-fixture attempts used absolute packet offsets despite a popped MAC header and
hit the radio medium's simultaneous-transmission guard. Those setup failures are excluded from
the table. Corrected fixtures use offsets relative to packet data and an explicit deterministic
medium profile. Exact boundary durations are constructed from integer microseconds.

The [reviewer document](../../../../../tests/protocol/aodv/route-lifetimes.md) explains the packet
replay, route-state setup, each parent comparison and coverage limits. The migration note records
the newly rejected configuration values. Full discovery, attribution, transit, reboot, interface,
subnet and repair coverage remains pending in the plan.

## Discovery ownership stage, 2026-11-09

Immediate parent: `be0ba1067d`. Corrected source is the discovery-ownership commit following that
parent. Default feature selection, debug protocol/module executions, deterministic seed 0.

The 15 new protocol fixtures listed in [discovery-ownership.md](../../../../../tests/protocol/aodv/discovery-ownership.md)
all compile on the parent and fail their intended runtime assertions. The parent emits on the
wrong interface/MAC in each of the six constrained-output positions. RateLimit, DeferredReply,
ReplyDuringJitter, IndependentRoute, NeighborCompletion and GatewayQueues fail required data
delivery. DiscoveryExhaustion observes only one request instead of two. GatewayFailure leaves
originals undestroyed. TransitAdmission observes originating discovery instead of RERR/drop.
Both new module fixtures fail the corresponding runtime assertion on the same parent.

The exact same fixtures pass with the corrected library. Combined protocol scope is 31 passes:
the preceding 16 controls/corrections and 15 new cases. Combined module scope is ten passes:
four existing AODV controls, four advertisement-duration rejections and two new cases.
Fresh matching debug and release links pass. No expected output or fingerprint baseline changed.

The final parent comparison uses `/home/miguel/workspace/inet-aodv-parent`, detached at the immediate
parent, with copied identical fixtures. Its Aodv translation unit was rebuilt from the parent's
tracked source and the library relinked. Unchanged translation-unit objects and generated message
files were copied from the already built checkout to avoid recompiling the rest of INET. The
source difference is restricted to Aodv.cc/Aodv.h, and no other source includes Aodv.h. This is an
incremental build with verified unchanged object reuse, not a clean build. An earlier full parent
build attempt was stopped and its partial objects superseded before the recorded execution.

Commands from each checkout root:

```sh
make -C src -j6 MODE=debug
opp_run_protocol_tests --load . -p inet -m debug -w aodv \
  -f 'Rfc3561(RateLimit|GatewayQueues|ReplyDuringJitter|DiscoveryExhaustion|IndependentRoute|DeferredReply|GatewayFailure|NeighborCompletion|Output.*|TransitAdmission)\.test$' \
  --no-build --no-concurrent --log-file /tmp/aodv-discovery-parent-isolated.log
opp_run_module_tests --load . -p inet -m debug \
  -f 'Aodv(GatewayQueues|TransitFailure)\.test$' --no-build --no-concurrent \
  --log-file /tmp/aodv-discovery-parent-modules.log

# Corrected checkout, after the matching debug build:
opp_run_protocol_tests --load . -p inet -m debug -w aodv -f 'Rfc3561.*\.test$' \
  --no-build --no-concurrent --log-file /tmp/aodv-discovery-final.log
opp_run_module_tests --load . -p inet -m debug \
  -f '(AODV(SimpleTest(_2)?|ShortestPath|LifecycleTest)|Aodv(GatewayQueues|TransitFailure|AdvertisementLifetimeInvalid_[1-4]))\.test$' \
  --no-build --no-concurrent --log-file /tmp/aodv-discovery-modules.log
make -C src -j4 MODE=release
```

Parent commands exit 1 with 15 and two intentional assertion failures. Corrected commands exit 0
with 31 and ten passes. Logs are the named `/tmp` files, with `-runner.log` companions. Build logs:
`/tmp/aodv-discovery-build-final.log`, `/tmp/aodv-discovery-release.log`, and
`/tmp/aodv-discovery-parent-isolated-incremental-build.log`. Generated parent output remains in the
isolated checkout; the earlier eight-case reproduction is saved in `/tmp/aodv-discovery-parent-evidence/`.

Basic ring/backoff progression moved with the search record because the old WaitForRrep retry
state was replaced as one ownership change. The next stage supplies distinguishing timing cases,
checked parameter arithmetic and corrected defaults. Link attribution, detailed RERR semantics,
recovery, interfaces, subnet profiles and local repair remain pending. These runs do not establish
IPv6 compatibility or a higher whole-protocol conformance level.
