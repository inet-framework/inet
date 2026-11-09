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

Release compilation, the complete before-push gate set, feature-isolated builds, IPv6 runtime
compatibility, fingerprints, diagram rendering and the remaining behavioral plan stages are not
established by this record. No existing baseline was changed.
