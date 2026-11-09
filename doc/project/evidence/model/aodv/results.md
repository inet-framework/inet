# AODV correction execution record

## Submission validation

Final integrated source passes **231/231 protocol cases**, **97/97 module
cases** (93 `Aodv*` plus all four unchanged `AODV*` integration controls), and
the serial-arithmetic unit test. All runners use fresh matching debug artifacts,
General/run 0/seed 0 for the authored deterministic fixtures, and exit 0. The
additional native IPv6 ACK decoding repetition also passes. Debug and release
compilation and the isolated IPv4-only AODV debug feature build succeed.

```sh
make -j4 MODE=debug
make -j4 MODE=release
opp_run_protocol_tests --load . -p inet -m debug -w aodv --no-build --no-concurrent
opp_run_module_tests --load . -p inet -m debug -f 'Aodv.*\.test$' --no-build --no-concurrent
opp_run_module_tests --load . -p inet -m debug -f 'AODV.*\.test$' --no-build --no-concurrent
opp_run_unit_tests --load . -p inet -m debug -f 'AodvSequence\.test$' --no-build --no-concurrent
```

Working directory is `/home/miguel/workspace/inet-aodv`; the feature build is in
`/home/miguel/workspace/inet-aodv-ipv4`, with Ipv6 and its dependent features
disabled and Aodv/Ipv4 enabled. Feature build inputs are the identical final
AODV C++/NED files. Build logs: `/tmp/aodv-final-{debug,release}-build.log`,
`/tmp/aodv-final-ipv4-build.log`; test logs: `/tmp/aodv-final-{protocol,modules,legacy-modules,unit}.log`.

Scoped AODV architecture and source/document seal checks pass. Full-tree
architecture and interface outputs are byte-identical to the pinned upstream
base; naming has the same existing candidates and passes modified NED
declarations. The full documentation link check retains the base's 181 missing
local standards links, with no added failures. No exception ledger change is
needed for these pre-existing results. C++ lint is not verified because
clang-tidy and a compilation database are unavailable. Mermaid rendering is not
verified because no renderer is installed; diagram text and event order were
checked against the fixture assertions. This is an author review, not an
independent review. The local semantic checklist is `/tmp/aodv-semantic-review.md`.
Example fingerprint comparisons are explicitly reported below as mismatches,
not passing correctness evidence or approved baseline changes.

## Final native IPv6 compatibility (stage 12)

Eight identical General/run 0/seed 0 cases ran in each category with fresh
matching debug libraries. Immediate parent `edecd5e6ce` has six passing controls
and two runtime failures per category: Incoming and IncomingRestart receive no
multicast request reply. The correction joins the existing IPv6 control group
on every socket start. Corrected native module cases pass; the full protocol
selection below includes the same eight cases. The three-node Hello fixture
observes actual Ethernet-to-wireless forwarding, retains IP source S and MAC
source N, and measures loss from the last attributable incoming frame, including
native ND. Its final quiet-period assertion is at 0.95 seconds. Initial assertions
that equated final data and final neighbor reception are excluded fixture errors.
The initial discovery/freshness fixture also decodes the native two-byte ACK;
RREQ/RREP/RERR round trips are retained.

```sh
opp_run_protocol_tests --load . -p inet -m debug -w aodv \
  -f 'Rfc3561Ipv6(Incoming|IncomingRestart|RateLimit|DeferredReply|Restart|CrashReady|StopQueued|HelloForwarding).test$' \
  --no-build --no-concurrent
opp_run_module_tests --load . -p inet -m debug \
  -f 'AodvIpv6(Incoming|IncomingRestart|RateLimit|DeferredReply|Restart|CrashReady|StopQueued|HelloForwarding).test$' \
  --no-build --no-concurrent
```

Parent logs: `/tmp/aodv-ipv6-parent-{protocol,modules}-final2.log`. Corrected
combined selections use `/tmp/aodv-final-{protocol,modules}.log` and the four
uppercase legacy controls in `/tmp/aodv-final-legacy-modules.log`. ACK decode
repetition: `/tmp/aodv-final-ipv6-encoding.log`. The per-case assertions and
limitations are in [ipv6-compatibility.md](../../../../../tests/protocol/aodv/ipv6-compatibility.md).

## Final example fingerprint comparison

The dependency inventory maps changed AODV files to `inet.routing.aodv`,
`inet.node.aodv` and the Aodv feature. Five existing examples cover direct and
multihop discovery, lifecycle and external gateway routing. Both sides use debug,
run 0, seed 0, the stored 100-second limit and exactly `tplx`, `~tNl`, `~tND`.
Base `7ec0d4d3f3` passes all 15 comparisons. The corrected source gives two passes
and 13 mismatches, with zero simulation execution errors:

| Configuration | tplx | ~tNl | ~tND |
| --- | --- | --- | --- |
| ShortestPath | mismatch | pass | mismatch |
| SimpleLifecycle | mismatch | mismatch | mismatch |
| SimpleRREQ | mismatch | pass | mismatch |
| SimpleRREQ2 | mismatch | mismatch | mismatch |
| StaticExternal | mismatch | mismatch | mismatch |

Bounded, paired four-second debug traces locate the first actual RREQ at the same
time and with the same wire fields, but dispatched by `aodv-search` instead of
`aodv-send-jitter`. The first destination RREP has the same counter and event/time
on both sides, while its lifetime changes from 6 to 11.2 seconds under the new
minimum `myRouteTimeout=2*pathDiscoveryTime` default. SimpleRREQ's reply is event
50 at 3.202491266542; SimpleRREQ2 and SimpleLifecycle are event 67 at
3.202491396656; ShortestPath is event 106 at 3.203137018857; StaticExternal is
event 46 at 2.00640562485. This directly explains an early event/control-byte
divergence. Later full-100-second trajectories, including the three ~tNl
mismatches, were not fully causally aligned. Direct assertion tests establish
the corrected contracts; these fingerprints remain mismatches for review.

The active `inet.opp` supplies the library/NED/features, stores and ingredient
defaults. For bounded metadata collection, `/tmp/aodv-fingerprint-{base,current}.opp`
are copies with only `root_folder` set to the respective absolute worktree and
`ini_file_folders` restricted to `["examples/aodv"]`. All other settings remain
unchanged. This avoids collecting unrelated project configurations before the
working-directory filter is applied.

```sh
opp_run_fingerprint_tests --load /tmp/aodv-fingerprint-current.opp \
  -p inet -m debug -w '^examples/aodv$' \
  -c '^(SimpleRREQ|SimpleRREQ2|SimpleLifecycle|ShortestPath|StaticExternal)$' \
  -r '^0$' --no-build --no-concurrent \
  --result-file /tmp/aodv-fingerprint-submission.json \
  --log-file /tmp/aodv-fingerprint-submission.log
```

Current exit 1 is the comparison failure, not a simulation error; base exit 0.
The final rerun reproduces the earlier corrected fingerprint values exactly.
Base artifacts: `/tmp/aodv-fingerprint-base.json`,
`/tmp/aodv-fingerprint-base-final{,-runner}.log`. Current artifacts:
`/tmp/aodv-fingerprint-submission{.json,.log,-runner.log}`. Paired traces:
`/tmp/aodv-fingerprint-{base,current}-simple-corrected.out` and
`/tmp/aodv-fingerprint-{base,current}-{SimpleRREQ2,SimpleLifecycle,ShortestPath,StaticExternal}-diagnostic.out`.
The first launcher attempts lacked the worktree-specific `INET_ROOT` and failed
network lookup; they are excluded setup errors. Correct diagnostic runs set
`INET_ROOT` to their own worktree and use `../../bin/inet --debug -u Cmdenv
-c <configuration> -r 0 --sim-time-limit=4s --cmdenv-express-mode=false
--cmdenv-log-level=debug` from `examples/aodv`.

The shared adapter produces `/tmp/aodv-fingerprint-normalized.json`, schema v1,
runner opp_repl, 15 executed comparisons, status FAIL, flaky false, changed result
observed/expected true and baseline approval false. Its input is a mechanically
flattened 15-leaf summary from the runner JSON: the raw nested console summary
otherwise makes this adapter count only the last three-leaf group. The raw JSON
remains authoritative. The resolved runner is
`/home/miguel/omnetpp-6.4.0aipre2/.venv/bin/opp_run_fingerprint_tests`.

No stored expectation or dependency inventory changes. SHA256:
`tests/fingerprint/store.json` = `ff4a94299a5665485527e88d8998e138718dac7c7be232a1fbf6a97349e45587`;
`tests/dependency.json` = `9a827858c2949b68893b14214326cffcf6d6a568994e7e27706aa1b82680a28f`.
Historical tested revisions map to the submitted, source-identical revisions in
[verification-revisions.md](verification-revisions.md).

## Bounded local repair (stage 11)

Immediate parent `497d3cb7e5` and corrected source ran identical General/run 0/seed
0 fixtures with fresh matching debug libraries. The isolated parent's 39 protocol
cases give eight passing controls and 31 runtime failures. Eleven module cases
give eleven failures: two missing parameter rejections and nine runtime failures.
Most enabled IPv4 parents fail first at missing repair ownership. SecondFailure
fails its first-success check. Native IPv6 parents stop earlier because ND cannot
finish while AODV queues the unicast advertisement. These first failures are
separated from later corrected-path observations in
[local-repair.md](../../../../../tests/protocol/aodv/local-repair.md).

Corrected focused scope passes all 39 protocol cases and 12 module cases, including
the initial native IPv6 discovery/freshness control. Fresh debug and release builds
succeed. The isolated IPv4-only feature build also succeeds after copying the
current AODV C++/NED inputs: Aodv and Ipv4 are enabled, Ipv6 is disabled with its
dependent features. Its initial build used the root Makefile to generate features.h;
an earlier direct src-only build without that generated header is excluded.

```sh
make -C src -j4 MODE=debug
make -C src -j4 MODE=release
opp_run_protocol_tests --load . -p inet -m debug -w aodv \
  -f 'Rfc3561LocalRepair.*\.test$' --no-build --no-concurrent
opp_run_module_tests --load . -p inet -m debug \
  -f '(AodvLocalRepair|AodvIpv6Repair|AodvIpv6Compatibility).*\.test$' \
  --no-build --no-concurrent
```

Build logs: `/tmp/aodv-repair-build-first.log`, `/tmp/aodv-repair-nd3-build.log`,
`/tmp/aodv-repair-release.log`, `/tmp/aodv-ipv4-build-corrected.log` and
`/tmp/aodv-ipv4-final-aodv-build.log`. Focused logs:
`/tmp/aodv-repair-all-owners.log`, `/tmp/aodv-repair-modules-owners.log`,
`/tmp/aodv-repair-parent-final.log` and `/tmp/aodv-repair-parent-modules-final.log`.
The 39-case corrected selection uses the final owning IPv4 adapter, requiring
every received transit original destroyed and the native IP hook queue empty.
Source packet counters alone are not claimed as transit-queue disposal proof.

Fixture corrections excluded from reproduction: initially checking a retained
record after ordinary deletion; expecting a jittered notification before its
declared send time; using one script node as both source and local destination
(replaced by a real third source); unbound UDP delivery producing unrelated ICMP
traffic (a sink is now bound); counting a returned control packet as data; allowing
normal idle expiry after successful one-hop recovery; missing hook overrides,
an incorrect unregister signature and colliding Define_Module line expansions.
The module rejection cases use the native stderr diagnostics.

Native IPv6 setup uses the explicit global node configurator, zero boot/DAD delay,
stable one-hop addresses and disabled MANET redirects. The scripted source supplies
its output at local-out so native ND resolves unicast. A probe establishes the failed
neighbor's ND entry before radio loss. Existing IPv6 local-out reinjection can emit
link broadcast; observations are deduplicated by tree identity, while repaired
transit must use the alternate resolved MAC. Native IPv6 initial RREQ/ACK plus
repair RREP/RERR custom encoding checks pass. The first ND-classification attempt
through generic dissection encountered an incomplete chunk; direct typed-header
inspection on the copy avoids treating link control as data demand. Raw bounded
diagnostics are `/tmp/aodv-repair-ipv6-source-diagnostic.out` and
`/tmp/aodv-repair-ipv6-nd-diagnostic.out`.

The final combined selections pass 223/223 protocol and 89/89 module cases, exit
0, including the four unchanged legacy integration controls. Logs are
`/tmp/aodv-repair-all-protocol.log` and `/tmp/aodv-repair-all-modules.log`.
No existing expectation, fingerprint, dependency store or baseline was updated.
Broader
native IPv6 rate/restart/three-node liveness and final submission gates remain
assigned; this checkpoint is not a completion claim for the entire plan.

## Received subnet routes (stage 10)

Immediate parent `9fd380a795` was built in the isolated comparison worktree,
then ran the identical 24-case Subnet selection: 24 runtime assertion failures.
Most fail first because nonzero-prefix replies remain host records. Their later
assertions are corrected-path coverage, not separately reproduced parent defects.
Two additional protocol cases, Downstream and HelloBlock, also fail runtime
assertions on that parent. The native downstream module repetition fails its
missing-prefix assertion. See [subnet-routes.md](../../../../../tests/protocol/aodv/subnet-routes.md)
for each scenario, owner, production path and limit.

Fresh corrected debug/release builds pass. The complete then-current selection
passes 182 protocol cases, followed by passing Downstream and HelloBlock cases:
184 observed protocol passes across these selections. The final module selection
passes all 77 cases, including the unchanged four integration controls. No existing
expectation or fingerprint changes. General, run 0, seed 0; fixed-position IPv4
AckingMac/UnitDisk with zero jitter. The HelloBlock MAC timeout is explicitly 5 ms.

```sh
make -C src -j4 MODE=debug
make -C src -j4 MODE=release
opp_run_protocol_tests --load . -p inet -m debug -w aodv \
  -f 'Rfc3561.*\.test$' --no-build --no-concurrent
opp_run_protocol_tests --load . -p inet -m debug -w aodv \
  -f 'Rfc3561Subnet(Downstream|HelloBlock)\.test$' --no-build --no-concurrent
opp_run_module_tests --load . -p inet -m debug \
  -f '(AODV|Aodv(?!Ipv6)).*\.test$' --no-build --no-concurrent
```

Saved logs: `/tmp/aodv-subnet-final-build.log`, `/tmp/aodv-subnet-release.log`,
`/tmp/aodv-subnet-combined.log`, `/tmp/aodv-subnet-downstream-protocol.log`,
`/tmp/aodv-subnet-hello-block2.log`, `/tmp/aodv-subnet-modules-final.log`,
`/tmp/aodv-subnet-parent-complete.log`, `/tmp/aodv-subnet-parent-last.log` and
`/tmp/aodv-subnet-downstream-parent.log`.

Setup corrections excluded from reproduction: a missing test-only tag include,
cross-module emission without Enter_Method/take (replaced by a public source API),
Cmdenv express-mode output requiring stdout, an incorrect checksum enum spelling,
too-short configured retention relative to activeRouteTimeout, an unscheduled
public placeholder deadline, and an unregistered unused source in the downstream
fixture. Sustained initially allowed quiescence between demands, correctly permitting
deletion; 10 ms demand now covers multiple searches and retention intervals, then
intentional quiescence verifies ordinary deletion. HelloBlock initially checked
before the default 100 ms MAC timeout; its declared 5 ms timeout supplies the
intended failed-transmission event before checking invalidation.

The seeded unknown-host cases are supplemental public table setup. Aggregate
learning, errors, independent counters, queue release, native downstream AODV
and actual next-hop MAC observations use production paths. Native IPv6 fixture
development and the isolated IPv4-only build remain separate final-stage checks.

## Selected interfaces (stage 9)

Parent `14013e9c8a` and corrected source used identical General/run 0/seed 0
fixtures with fresh matching debug libraries. The isolated parent checkout reused
unchanged generated files/objects, then rebuilt AODV and relinked; this was an
incremental build, not a clean build.

The 18 Interfaces protocol cases produced five parent passes and 13 runtime
assertion failures. The 16 Interfaces module cases produced three parent passes
and 13 failures: eleven missing startup rejections, DownStart and Reconfigure.
The final fixtures explicitly configure receiver mode for radio restart and wait
for asynchronous shutdown completion. Setup attempts preceding those corrections
are excluded. See [selected-interfaces.md](../../../../../tests/protocol/aodv/selected-interfaces.md)
for each causal comparison and the saved radio diagnostic.

Commands from the appropriate checkout root:

```sh
make -C src -j4 MODE=debug
make -C src -j4 MODE=release
opp_run_protocol_tests --load . -p inet -m debug -w aodv \
  -f 'Rfc3561Interfaces.*\.test$' --no-build --no-concurrent
opp_run_module_tests --load . -p inet -m debug \
  -f '(AODV(SimpleTest(_2)?|ShortestPath|LifecycleTest)|AodvInterfaces).*\.test$' \
  --no-build --no-concurrent
```

Logs: `/tmp/aodv-interfaces-parent-final-profile.log`,
`/tmp/aodv-interfaces-parent-modules-profile.log`,
`/tmp/aodv-interfaces-neighbor-protocol.log` and
`/tmp/aodv-interfaces-neighbor-modules.log`. Before the final neighbor-observation
ordering correction, the complete selection gave 158 protocol passes and 72/76
module passes. Four unchanged legacy route-event expectations exposed suppression
before the separate previous-hop refresh. Restoring that independent observation
preserves suppression and the original route-event timing. No baseline is edited.

The added profile is IPv4 only. Explicit IPv6 selection rejection is a configuration
check, not native IPv6 runtime evidence. Native IPv6 compatibility remains assigned
to the final shared-state checks. External-interface controls pass on both sources.

Final ordering-corrected selection: 18/18 protocol cases and 20/20 module cases
(16 new cases plus all four unchanged integration controls), exit 0. Fresh debug
and release builds succeed; final release log is
`/tmp/aodv-interfaces-neighbor-release.log`.

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

## Discovery timing and bounds stage, 2026-11-09

Immediate parent: `ec0c7b08be`, rebuilt in the isolated parent checkout. Default feature selection,
debug mode, seed 0. The five new Rfc3561DiscoveryTiming fixtures pass on both revisions. They
provide added coverage for ring TTLs, retained-hop starts, diameter clamping, zero additional
retries, actual-send IDs/sequences and full waits with/without 30 ms jitter. The immediately
preceding ownership commit already supplies this behavior. These are passing parent controls.

Thirty new module cases require configuration/sample rejection and five require acceptance.
All 35 pass on the correction. On the parent, two pass and 33 fail their required diagnostic or
parameter assertion. Classify these failures by cause rather than counting them all as defects:

| Parent outcome | Cases | Interpretation |
| --- | --- | --- |
| Initializes normally despite invalid input | 23 rejection cases | Newly enforced bounds. Duration_activeRouteTimeout uses an independent valid 12-second advertisement override to expose its own missing positivity check. |
| Already rejects through another check | RetriesNegative, BufferNegative, HelloLossZero, Duration_helloInterval, Port, TraversalOverflow | Six diagnostic controls. They do not establish missing invalid-input rejection. |
| Already rejects the sampled jitter correctly | JitterSample | Passing parent control. |
| Wrong derived blacklist default | Default, ZeroRetries, RetainedBound, Boundaries | Four failed public-parameter assertions. Default also checks the corrected destination lifetime after the blacklist assertion. |
| Preserves explicit positive blacklist | Override | Passing parent control. |

The [reviewer document](../../../../../tests/protocol/aodv/discovery-timing.md) lists all 35 module
suffixes, five wire schedules, exact inputs, assertions and limitations. The fatal runtime
JitterSample case checks the diagnostic only: throwing from the IP hook before queue registration
can expose the framework's in-progress datagram as undisposed. This run does not establish
clean stop/crash ownership. Initialization rejection cases prohibit undisposed-object output.

Corrected combined scope: 36 protocol passes and 45 module passes, including all preceding
cases and four existing AODV integration controls. Matching debug and release links pass.
The isolated parent's final module run reports two passes and 33 expected-observation failures;
Duration_activeRouteTimeout is additionally rerun with its final isolated input and reaches normal
initialization. All five final timing fixtures pass on the parent. No baseline changed.

Commands from the corresponding checkout root:

```sh
make -C src -j4 MODE=debug
opp_run_protocol_tests --load . -p inet -m debug -w aodv \
  -f 'Rfc3561DiscoveryTiming.*\.test$' --no-build --no-concurrent \
  --log-file /tmp/aodv-timing-parent-final.log
opp_run_module_tests --load . -p inet -m debug -f 'AodvParameterValidation.*\.test$' \
  --no-build --no-concurrent --log-file /tmp/aodv-parameters-parent-final.log
opp_run_module_tests --load . -p inet -m debug \
  -f 'AodvParameterValidationDuration_activeRouteTimeout\.test$' --no-build --no-concurrent \
  --log-file /tmp/aodv-parameters-parent-active.log

# Corrected checkout:
opp_run_protocol_tests --load . -p inet -m debug -w aodv -f 'Rfc3561.*\.test$' \
  --no-build --no-concurrent --log-file /tmp/aodv-timing-all-protocol.log
opp_run_module_tests --load . -p inet -m debug \
  -f '(AODV(SimpleTest(_2)?|ShortestPath|LifecycleTest)|Aodv(GatewayQueues|TransitFailure|AdvertisementLifetimeInvalid_[1-4]|ParameterValidation.*))\.test$' \
  --no-build --no-concurrent --log-file /tmp/aodv-timing-all-modules.log
make -C src -j4 MODE=release
```

Parent timing exits 0, parent module comparisons exit 1, corrected selections and builds exit 0.
Runner stdout has a `-runner.log` companion for each named log. Build logs:
`/tmp/aodv-timing-parent-build.log`, `/tmp/aodv-timing-build-final.log`, `/tmp/aodv-timing-release.log`.
Generated parent outputs remain under the isolated checkout's tests/protocol and tests/module work
directories. Initial test development caught an integer-parameter conversion error, a suppressed
EV_INFO success marker and duplicate INI entries in timing profiles. Those setup/development
attempts are excluded from the final comparisons; the fixed fixtures retain their intended assertions.

Full link validation/attribution, error propagation, Hello loss, recovery, interfaces, subnet and
repair acceptance cases remain pending. No whole-protocol conformance level is raised.

## Link validation and attribution stage, 2026-11-09

Immediate parent: `f8baeb3233`, freshly rebuilt and linked in the isolated parent checkout.
Default features, debug mode, seed 0. The correction has 21 new protocol fixtures: 12 ReplyAck
variants, seven FailedLink variants and two ControlIngress variants. The exact inputs and causal
comparisons are in [link-validation.md](../../../../../tests/protocol/aodv/link-validation.md).

The 19-case ReplyAck/FailedLink parent scope compiles and reports two passes and 17 intended runtime
assertion failures. Both exact-deadline cases already pass and remain controls. The two additional
ingress cases compile and fail their absent-route/reply assertion on the same parent. The three
module cases compile and fail the corresponding replacement/metadata assertion. These are actual
behavior failures, not fixture build errors. In ExpiredNeighbor the parent preserves D but misses
invalidation of the dependent N route; the other replacement cases catch invalidation of D itself.

Corrected complete scope: **57 protocol passes and 48 module passes**, including the four unchanged
existing integration controls. Fresh matching debug and release links pass. No existing fixture
expectation, simulation fingerprint or baseline is changed.

The DCF case uses independent mode, non-QoS DCF, fixed 1 Mbps, no aggregation and no fragmentation.
It observes a real retry-limit failure and asserts retained InterfaceReq/MacAddressReq, receiver
MAC, Retry and both DS bits on the failed frame. The AckingMac counterpart learns replacement
state through real UDP control while the original packet is in flight. Missing-output, absent
binding, ambiguous MAC and rebinding cases skip peer-specific actions. ExpiredNeighbor verifies
that the stable binding remains usable after both neighbor route records have been deleted.

The no-A reply case exposed a raw-chunk peek using Packet's unspecified-length convention. It
returned an incomplete header, preventing the failed gratuitous reply from being identified.
Bounding the peek by the UDP payload fixes that classification. Port 60654 repeats the real
failure/blacklist observation without a global UDP dissector registration. The saved diagnostic
trace is `/tmp/aodv-classifier-debug-test.out`; it is development evidence, not a passing run.

An initial outgoing policy requested ACKs on every forwarded reply. The expanded integration
selection caught simultaneous-transmission guard failures in three existing simulations. That
policy was removed. The final source retains the established cleared-A forwarded-reply policy,
while acknowledging every received A flag and blacklisting confirmed RREP failures independently
of A. All four existing integration controls then pass unchanged. Initial socket-port collisions,
an incompatible timing-helper signature and the cable-only ACK-order experiment are excluded from
the final comparisons. Their replacement fixtures preserve the intended assertions.

Commands from the corresponding checkout root:

```sh
# Isolated parent:
make -C src -j4 MODE=debug
opp_run_protocol_tests --load . -p inet -m debug -w aodv \
  -f 'Rfc3561(ReplyAck|FailedLink).*\.test$' --no-build --no-concurrent \
  --log-file /tmp/aodv-link-parent-final-with-port.log
opp_run_protocol_tests --load . -p inet -m debug -w aodv \
  -f 'Rfc3561ControlIngress.*\.test$' --no-build --no-concurrent \
  --log-file /tmp/aodv-control-ingress-parent.log
opp_run_module_tests --load . -p inet -m debug \
  -f 'AodvTransitFailure(InFlight|Dcf|Metadata)\.test$' --no-build --no-concurrent \
  --log-file /tmp/aodv-link-modules-parent-final.log

# Corrected checkout:
make -C src -j4 MODE=debug
opp_run_protocol_tests --load . -p inet -m debug -w aodv -f 'Rfc3561.*\.test$' \
  --no-build --no-concurrent --log-file /tmp/aodv-link-all-protocol-guard.log
opp_run_module_tests --load . -p inet -m debug \
  -f '(AODV(SimpleTest(_2)?|ShortestPath|LifecycleTest)|Aodv(GatewayQueues|TransitFailure.*|AdvertisementLifetimeInvalid_[1-4]|ParameterValidation.*))\.test$' \
  --no-build --no-concurrent --log-file /tmp/aodv-link-all-modules-guard.log
make -C src -j4 MODE=release
```

Parent comparisons exit 1 with the explicitly classified outcomes above. Corrected selections
and builds exit 0. Each named log has a `-runner.log` stdout companion. Build logs:
`/tmp/aodv-link-parent-build.log`, `/tmp/aodv-link-build-guard-final.log`,
`/tmp/aodv-link-release-guard.log`. The isolated parent uses the unchanged-object reuse procedure
already recorded in the ownership stage. Generated outputs are transient and runners clear their
work directories, so the named logs retain the comparison record.

This establishes the selected IPv4 link and metadata profiles. Complete precursor/RERR semantics,
Hello observation deadlines, restart cleanup, selected-interface operation, subnet rules, IPv6
compatibility and local repair remain pending. No whole-protocol conformance level is raised.

## Transit errors and precursor delivery, 2026-11-09

Immediate parent: `335030555e`. The same 25 new protocol fixtures compile and execute on that
parent: seven pass as controls and 18 reach the intended runtime assertions. The corrected
selection passes all 25, and the combined protocol selection passes all 82. Three new module
cases give one parent pass (owned forwarding), two parent runtime failures (known transit
retention and foreign-route forwarding), and three corrected passes. The full selected module
scope passes 51 cases, including all four unchanged AODV integration expectations.

| Protocol suffixes, prefix Rfc3561NoDeleteError | Parent observation and correction |
| --- | --- |
| NoDelete | Ordinary invalidation changes the snapshot; preserve every route field for N. |
| NoDeleteStale, NoDeleteHalf, NoDeleteInactive | Missing permitted relay fails the single-notification assertion; process N without ordinary freshness/activity filtering. |
| NoPrecursorNotification | Parent invalidates the snapshot despite N; preserve it without relay. |
| OrdinaryError | Broadcast copies fail the single precursor-delivery assertion; send the qualified precursor copy. |
| TransitKnown, TransitBudget, TransitUnknown, TransitAggregate, TransitBlockedHost | Parent leaves the previous deletion deadline unchanged at PREROUTING; apply case-(ii) mutation and retention before budget admission. These cases additionally check wrap, unknown placeholder preservation, error identity and independent aggregate preservation. |
| TransitAbsent | Broadcast copies fail the intended one-recipient message count; use the attributable immediate sender without creating a route. |
| TransitUnknownHostAggregate, ForeignForward | Parent's second forwarding lookup generates an erroneous RERR; use the already prepared route and preserve independent route state. |
| PrefixAmbiguous, PrefixInactiveWitness | Parent mutates /24 despite the competing /25 identity; resolve ambiguity before freshness/activity. |
| FanoutBudget | Parent broadcasts multiple received copies instead of two intended unicast recipients; deliver every qualified copy for one charge, suppressing the second logical error. |
| TransitBudgetCrossing | Parent charges at arrival and yields only one distinct IP identifier; admit after jitter in each actual send window. |
| StaleError, HalfError, OtherHop, NoPrecursorError, ForeignError, PrefixUnique, OwnedForward | Passing parent controls retained unchanged. |

The fixture corrections preceding this final comparison removed an assumed IP drop signal,
disabled forwarding on the observational peer, and checked the actual two-hop advertised route.
Initial fixture/setup outcomes in `/tmp/aodv-errors-parent-initial.log` are not behavioral evidence.
The final fixture checks local retained state before message counts, making its retention
failure independent of the parent's extra broadcast copies.

Commands from the corresponding checkout root:

```sh
make -C src -j4 MODE=debug
# Isolated parent:
opp_run_protocol_tests --load . -p inet -m debug -w aodv \
  -f 'Rfc3561NoDeleteError.*\.test$' --no-build --no-concurrent \
  --log-file /tmp/aodv-errors-parent-final.log
opp_run_module_tests --load . -p inet -m debug -f 'AodvTransitError.*\.test$' \
  --no-build --no-concurrent --log-file /tmp/aodv-errors-parent-modules.log
# Corrected checkout:
opp_run_protocol_tests --load . -p inet -m debug -w aodv -f 'Rfc3561.*\.test$' \
  --no-build --no-concurrent --log-file /tmp/aodv-errors-all-protocol.log
opp_run_module_tests --load . -p inet -m debug \
  -f '(AODV(SimpleTest(_2)?|ShortestPath|LifecycleTest)|Aodv(GatewayQueues|TransitFailure.*|TransitError.*|AdvertisementLifetimeInvalid_[1-4]|ParameterValidation.*))\.test$' \
  --no-build --no-concurrent --log-file /tmp/aodv-errors-all-modules.log
make -C src -j4 MODE=release
```

Each log has a `-runner.log` companion. Parent comparisons exit 1 with the classified assertions;
corrected selections and builds exit 0. Build logs: `/tmp/aodv-errors-build.log`,
`/tmp/aodv-errors-parent-build.log`, `/tmp/aodv-errors-release.log`. The isolated parent uses the
previously documented unchanged-object reuse procedure. Existing expectations and baselines
are unchanged.

See [transit-errors.md](../../../../../tests/protocol/aodv/transit-errors.md) for the production
path, observations, delayed-message ownership and fixture limits. The prefix cases use public
record setup; real subnet learning is stage 10. RERR partitioning, Hello observation deadlines,
reboot, selected interfaces, IPv6 and local repair remain pending. No conformance level is raised.

A final guard also rejects quarantined inverse bindings when attributing a previous-hop error
recipient. After fresh debug/release builds, the 25 error cases and two ambiguous/rebinding
failed-link controls pass (27 total). Logs: /tmp/aodv-errors-binding-guard-protocol.log and its
-runner.log companion; build logs use the same stem with -build.log and -release.log.

## Route-error capacity, 2026-11-09

Immediate parent: `5e143cc73e`. Rfc3561ErrorCapacityOne and Maximum already pass; Partition and
Budget reach the intended runtime count-octet assertion with a 256-record header. After partitioning,
all four pass. The adjacent NoDeleteErrorFanoutBudget and TransitBudgetCrossing cases also pass.
Both modes build successfully. These are additional focused checks after the stage-6 combined
82-protocol/51-module selection; they do not replace the final full selection.

Commands from the implementation checkout root:

```sh
# Before mutation, with the current debug library:
opp_run_protocol_tests --load . -p inet -m debug -w aodv \
  -f 'Rfc3561ErrorCapacity.*\.test$' --no-build --no-concurrent \
  --log-file /tmp/aodv-capacity-parent-final-guard.log
# After mutation:
make -C src -j4 MODE=debug
opp_run_protocol_tests --load . -p inet -m debug -w aodv \
  -f 'Rfc3561(ErrorCapacity.*|NoDeleteError(FanoutBudget|TransitBudgetCrossing))\.test$' \
  --no-build --no-concurrent --log-file /tmp/aodv-capacity-current.log
make -C src -j4 MODE=release
```

Parent comparison exits 1 with two controls passing and two classified assertion failures.
The corrected six-case selection and builds exit 0. Each test log has a -runner.log companion;
build logs are /tmp/aodv-capacity-build.log and /tmp/aodv-capacity-release.log. Earlier attempts
omitted a suitable jitter bound, overlapped two radios on one medium, or assigned the same subnet
to distinct media. Those setup failures are excluded from the final comparison.

The fixture serializes and decodes actual emitted headers, checks unique destinations and
unchanged counters, and checks all local invalidations even when the second partition is suppressed.
Its public two-interface precursor setup deliberately isolates this concern before stage 9's
selected-interface learning. See [error-capacity.md](../../../../../tests/protocol/aodv/error-capacity.md).
No wire format, existing expectation or baseline is changed. Hello, recovery, selected interfaces,
subnet learning, IPv6 and repair remain pending.

## Hello observations and loss, 2026-11-09

Immediate parent: `381667c9d0`. The same 26 HelloLoss protocol cases give 10 parent passes and
16 intended runtime assertion failures. Both real three-node forwarding module cases fail the
parent's S-inactive assertion. The corrected combined selection passes 112 protocol cases and
53 selected module cases, including the four unchanged integration expectations. Debug and release
libraries were rebuilt after the production changes.

Individual stimuli, parent distinctions and production paths are in
[hello-liveness.md](../../../../../tests/protocol/aodv/hello-liveness.md). Silent, Outgoing,
NoPrecursor, UnknownSender, EthernetSilent, DcfSilent and the two timeout-first cases find the
parent's route still active at 0.35 s. Data, OtherControl, StaleHello, DcfData, EthernetData and
EthernetRefreshFirst find it still active at 0.6 s. Source finds S still active despite its silence.
Validation observes an extra reply after a late ACK, rather than the required loss-induced
blacklist. Metadata failure, no-arm, zero-duration, disabled-Hello and metadata-first exact-tie
cases are passing parent controls.

The 23-case preliminary comparison excluded the two later ingress-metadata controls and validation
case. Final evidence uses the complete 26-case helper. Early attempts omitted nonlocal-source
permission in the scripted replay or the mutable annotation on the test delivery-time parameter.
Those setup outcomes are excluded. The module cases use an actual separate source, an Ethernet
hop to N, then a wireless forwarded hop to the DUT; they assert original IP source S and MAC sender N.

Commands from the corresponding checkout root:

```sh
# Isolated parent:
make -C src -j4 MODE=debug
opp_run_protocol_tests --load . -p inet -m debug -w aodv -f 'Rfc3561HelloLoss.*\.test$' \
  --no-build --no-concurrent --log-file /tmp/aodv-hello-parent-complete.log
opp_run_module_tests --load . -p inet -m debug -f 'AodvHelloForwarded.*\.test$' \
  --no-build --no-concurrent --log-file /tmp/aodv-hello-parent-modules.log
# Corrected checkout:
make -C src -j4 MODE=debug
opp_run_protocol_tests --load . -p inet -m debug -w aodv -f 'Rfc3561.*\.test$' \
  --no-build --no-concurrent --log-file /tmp/aodv-hello-all-protocol.log
opp_run_module_tests --load . -p inet -m debug \
  -f '(AODV(SimpleTest(_2)?|ShortestPath|LifecycleTest)|Aodv(GatewayQueues|TransitFailure.*|TransitError.*|HelloForwarded.*|AdvertisementLifetimeInvalid_[1-4]|ParameterValidation.*))\.test$' \
  --no-build --no-concurrent --log-file /tmp/aodv-hello-all-modules.log
make -C src -j4 MODE=release
```

Parent comparisons exit 1 with the classified runtime assertions; corrected selections and builds
exit 0. Each test log has a -runner.log companion. Build logs: /tmp/aodv-hello-build.log,
 /tmp/aodv-hello-parent-build.log and /tmp/aodv-hello-release.log. The separate parent retains the
previously documented unchanged-object reuse procedure.

The exact-time Ethernet cases use measured serialization and a test adapter that owns incoming
packets before hooks. They select event insertion order without changing a production timer's
priority. Direct AckingMac, independent nonaggregating DCF and Ethernet establish bounded metadata
profiles; there is no concrete-interface whitelist. Restart reset, selected-interface operation,
subnet learning, IPv6 runtime compatibility and repair remain pending. No existing expectation,
baseline or wire layout changes and no conformance level is raised.

## Restart recovery and lifecycle, 2026-11-09

Immediate parent: `05c0a2c2f8`. The same 28 recovery protocol fixtures give four parent passes and
24 runtime failures: 23 intended assertions and the repeated-start UDP port collision. Seven new
module cases give two parent controls and five runtime failures. Corrected final selections pass
140 protocol and 60 module cases, including all four existing integration expectations. Fresh
debug/release builds precede these final executions.

[reboot-recovery.md](../../../../../tests/protocol/aodv/reboot-recovery.md) maps each named fixture
to its stimulus, production path, parent observation and additional corrected assertions. The four
protocol controls are DelayedControl, DelayedError, QueueCrash and DownInit. The two module controls
are QueueCrash and DownInit. Early delivery masks the parent's later reentrant-release path; the
corrected reentrant fixture directly exercises the operational-generation guard after real IP
reinjection and whole-node disposal of the remaining originals.

The first recovery attempts corrected fixture assumptions: no-reply exhaustion needs an explicit
zero additional retry count; a reverse route needs fresh input after it naturally expires during
the hold; enabled Hello requires activeRouteTimeout strictly greater than its loss interval.
A later test-only IPv4Address/L3Address comparison prevented some new fixtures from compiling.
Those setup/build outcomes are excluded from the final behavioral comparison.

A full intermediate run passed 140 protocol cases but found one existing lifecycle expectation
failure: deleting owned routes in reverse order changed the ordered route-recorder output at
shutdown. Preserve the routing table's native deletion order. The final fresh-library rerun passes
140 protocol and 60 module cases with no expectation or baseline modification.

Commands from the corresponding checkout root:

```sh
# Isolated parent:
make -C src -j4 MODE=debug
opp_run_protocol_tests --load . -p inet -m debug -w aodv -f 'Rfc3561RebootRecovery.*\.test$' \
  --no-build --no-concurrent --log-file /tmp/aodv-recovery-parent-corrected.log
opp_run_module_tests --load . -p inet -m debug -f 'AodvRebootRecovery.*\.test$' \
  --no-build --no-concurrent --log-file /tmp/aodv-recovery-parent-modules-corrected.log
# Corrected checkout:
make -C src -j4 MODE=debug
opp_run_protocol_tests --load . -p inet -m debug -w aodv -f 'Rfc3561.*\.test$' \
  --no-build --no-concurrent --log-file /tmp/aodv-recovery-all-protocol-order.log
opp_run_module_tests --load . -p inet -m debug \
  -f '(AODV(SimpleTest(_2)?|ShortestPath|LifecycleTest)|Aodv(GatewayQueues|TransitFailure.*|TransitError.*|HelloForwarded.*|RebootRecovery.*|AdvertisementLifetimeInvalid_[1-4]|ParameterValidation.*))\.test$' \
  --no-build --no-concurrent --log-file /tmp/aodv-recovery-all-modules-order.log
make -C src -j4 MODE=release
```

Parent comparisons exit 1 with the classified runtime outcomes. Final corrected selections and
builds exit 0. Each test log has a -runner.log companion. Final build logs:
 /tmp/aodv-recovery-order-build.log, /tmp/aodv-recovery-order-release.log and
 /tmp/aodv-recovery-parent-build.log. The isolated parent uses the unchanged-object reuse procedure
recorded earlier.

This establishes the selected IPv4 lifecycle profiles, including both exact extension/wake-up
orders, reset attribution/Hello/validation state, down-at-initialization, zero-time restart and
reentrant packet disposal. Explicit multi-interface operation, subnet learning, bounded IPv6
runtime compatibility and repair remain pending. No whole-protocol conformance level is raised.
