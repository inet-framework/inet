# Phase 0.3 Test Baseline — smoke2 run

Date: 2026-07-13
OMNeT++ branch: topic/inet-infrastructure-clean
INET branch: topic/infrastructure
Environment: source /home/levy/workspace/omnetpp/setenv, then source /home/levy/workspace/inet-infrastructure/setenv
CWD for all test invocations: /home/levy/workspace/inet-infrastructure
Runner: canonical opp_run_* console scripts (opp_repl 0.5.dev)

## Build results

| Component | Mode | Exit code |
|-----------|------|-----------|
| omnetpp | release | 0 |
| omnetpp | debug | 0 |
| INET | release | 0 |
| INET | debug | 0 |

## Test results

| Suite | Invocation + CWD | Total | PASS | FAIL | ERROR/SKIP | Failing test names (≤20) | Log file |
|-------|-----------------|-------|------|------|------------|--------------------------|----------|
| unit | `opp_run_unit_tests --load @opp --no-build` from `/home/levy/workspace/inet-infrastructure` | 76 | 76 | 0 | 0/0 | — | smoke2-unit.log |
| packet | `opp_run_packet_tests --load @opp --no-build` from `/home/levy/workspace/inet-infrastructure` | 1 | 1 | 0 | 0/0 | — | smoke2-packet.log |
| queueing | `opp_run_queueing_tests --load @opp --no-build` from `/home/levy/workspace/inet-infrastructure` | 57 | 53 | 4 | 0/0 | Gate_1, Gate_2, Gate_3, PeriodicGate_1 | smoke2-queueing.log |
| protocol | `opp_run_protocol_tests --load @opp --no-build` from `/home/levy/workspace/inet-infrastructure` | 13 | 13 | 0 | 0/0 | — | smoke2-protocol.log |
| module | `opp_run_module_tests --load @opp --no-build` from `/home/levy/workspace/inet-infrastructure` | 268 | 89 | 179 | 0/0 | AntennaOrientation_1..12, AODVLifecycleTest, AODVShortestPath, AODVSimpleTest, AODVSimpleTest_2, DHCP_1, DHCP_2, DHCP_lifecycle_1, DHCP_lifecycle_2, DHCP_lifecycle_3, ICMPv6_delivery, Ieee80211_1..4, Ieee80211Retransmission1..10, IGMP_basic (179 total — see smoke2-module.log) | smoke2-module.log |

## Comparison with previous (invalid) smoke run

### Clock/Oscillator unit tests (17 failed in smoke run → 0 failed now)

Previous run (smoke-unit.log): 17 failures in Clock/Oscillator tests —
Clock_Granularity_2, Clock_Linear_3, Clock_Linear_4, Clock_SettableGranularity_2/5/6/7/8,
Clock_SettableLinear_3/4/5/6/7, Oscillator_4/5/9/10 — all attributed to time-parameter jitter
in omnetpp (stdout %contains/%contains-regex mismatches due to non-deterministic timing output).

Current run (smoke2-unit.log): **0 failures** across all 76 unit tests including all Clock and
Oscillator tests. The jitter fix on branch topic/inet-infrastructure-clean is confirmed effective.

### Queueing NED-resolution failures (57 of 57 failed in smoke run → 4 of 57 fail now)

Previous run (smoke-queueing.log): ALL 57 queueing tests failed with NED package-resolution errors
(`inet.queueing.*` types not found). Root cause was the wrong invocation (hand-rolled opp_test
without proper -n NED path including src/).

Current run (smoke2-queueing.log): **53 of 57 pass**. The 4 remaining failures are:
Gate_1.test, Gate_2.test, Gate_3.test, PeriodicGate_1.test — these are content failures
(not NED-resolution errors), and represent the real baseline failures for this suite.

## Notes

1. **opp_run_unit/packet/queueing/protocol/module_tests** scripts were not installed in the venv
   at session start; `pip install -e /home/levy/workspace/opp_repl` was required to register them.
   The scripts are now present at `/home/levy/.venv/bin/opp_run_*`.

2. **--load @opp** is required: inet-infrastructure has no *.opp file in its root, so the runner
   cannot auto-detect the project. The bundled inet.opp from opp_repl is loaded via `--load @opp`.

3. **--no-build** was used throughout: INET was already rebuilt in Task 2; skipping redundant
   rebuild saves time and avoids makefile re-invocation noise.

4. **Module tests: 179/268 fail.** This is a substantial number of failures spanning diverse
   subsystems (AODV, DHCP, Ieee80211, IGMP, Interference, IPv4/IPv6, lifecycle, OSPF, ping, power,
   SCTP, TCP, tun, UDP). These failures represent the actual topic/infrastructure branch baseline
   (not artefacts of wrong invocation or jitter). The full failing-test list is in smoke2-module.log.

5. Module test summary line shows "268 concurrent" (thread count) and "268 TOTAL" — the counter
   includes 2 infrastructure lines (header + summary), so actual test count is 268 with 89 PASS
   and 179 FAIL.

## Artifact paths

- /home/levy/workspace/inet-infrastructure/plan/artifacts/phase0/smoke2-omnetpp-build.log
- /home/levy/workspace/inet-infrastructure/plan/artifacts/phase0/smoke2-inet-build-release.log
- /home/levy/workspace/inet-infrastructure/plan/artifacts/phase0/smoke2-inet-build-debug.log
- /home/levy/workspace/inet-infrastructure/plan/artifacts/phase0/smoke2-unit.log
- /home/levy/workspace/inet-infrastructure/plan/artifacts/phase0/smoke2-packet.log
- /home/levy/workspace/inet-infrastructure/plan/artifacts/phase0/smoke2-queueing.log
- /home/levy/workspace/inet-infrastructure/plan/artifacts/phase0/smoke2-protocol.log
- /home/levy/workspace/inet-infrastructure/plan/artifacts/phase0/smoke2-module.log
