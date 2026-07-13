# Phase 0.3 Smoke Test Baseline Summary

Branch: `topic/infrastructure` (inet-infrastructure worktree)
OMNeT++: `topic/inet-infrastructure` (omnetpp worktree)
Date: 2026-07-13

## Suite Results

| suite | invocation | total | pass | fail | error/skip | failing tests (first 20, truncated at 20) | log file |
|-------|-----------|-------|------|------|------------|-------------------------------------------|----------|
| tests/unit | `opp_test run -p <testname>/<testname>_dbg -w work <testfile>` (per-test, debug mode, lib: `tests/unit/lib/libtest_dbg.so`) | 76 | 59 | 17 | 0/0 | Clock_Granularity_2, Clock_Linear_3, Clock_Linear_4, Clock_SettableGranularity_2, Clock_SettableGranularity_5, Clock_SettableGranularity_6, Clock_SettableGranularity_7, Clock_SettableGranularity_8, Clock_SettableLinear_3, Clock_SettableLinear_4, Clock_SettableLinear_5, Clock_SettableLinear_6, Clock_SettableLinear_7, Oscillator_10, Oscillator_4, Oscillator_5, Oscillator_9 | smoke-unit.log |
| tests/packet | `./packet_test_dbg -u Cmdenv -c UnitTest -n .:../../src` (debug, single binary) | 1 run (42 internal tests via assert) | 1 (exit 0) | 0 | 0/0 | — | smoke-packet.log |
| tests/queueing | `opp_test run -p <testname>/<testname>_dbg -w work <testfile>` (per-test, debug mode) | 57 | 0 | 57 | 0/0 | ActiveSourceActiveSink_1, ActiveSourceActiveSink_2, ActiveSourceFullSink_1, ActiveSourcePassiveSink_1, ActiveSourcePassiveSink_2, ActiveSourcePassiveSink_3, Buffer_1, Burst_1, Burst_2, Classifier_1, Classifier_2, Complex_1, CompoundQueue_1, Delayer_1, Demultiplexer_1, Duplicator_1, EmptySourceActiveSink_1, EmptySourcePassiveSink_1, Filter_1, Filter_2, … (all 57 fail) | smoke-queueing.log |
| tests/protocol | `opp_test run -p <testname>/<testname>_dbg -w work <testfile>` (per-test, debug mode) | 13 | 0 | 13 | 0/0 | Checksum_1, Checksum_2, Cutthrough_1, FragmentNumberHeaderBasedFragmentation_1, FragmentTagAndFragmentNumberHeaderBasedFragmentation_1, FragmentTagBasedFragmentation_1, Serialization_1, Streaming_1, Streaming_2, SubpacketLengthHeaderBasedAggregation_1, Transceiver_1, Transceiver_2, Transceiver_3 | smoke-protocol.log |
| tests/module | `opp_test gen -w work *.test` + per-test `opp_makemake`+`make MODE=debug` + `opp_test run -p <testname>/<testname>_dbg -w work <testfile>` (debug, lib: `tests/module/lib/libtest_dbg.so` built fresh) | 268 | 73 | 195 | 0/0 | AntennaOrientation_1, AntennaOrientation_10, AntennaOrientation_11, AntennaOrientation_12, AntennaOrientation_2, AntennaOrientation_3, AntennaOrientation_4, AntennaOrientation_5, AntennaOrientation_6, AntennaOrientation_7, AntennaOrientation_8, AntennaOrientation_9, AODVLifecycleTest, AODVShortestPath, AODVSimpleTest, AODVSimpleTest_2, DHCP_1, DHCP_2, DHCP_lifecycle_1, DHCP_lifecycle_2 … (195 total fail) | smoke-module.log |

## Notes

### tests/unit — 17 failures

All 17 failures are in the Clock/Oscillator subsystem:
- **Clock_\* failures (13 tests)**: "stdout fails %contains rule" or "test program returned exit code 1" — consistently in `Clock_Granularity_2`, `Clock_Linear_3/4`, `Clock_SettableGranularity_2/5/6/7/8`, `Clock_SettableLinear_3/4/5/6/7`.
- **Oscillator_\* failures (4 tests)**: "stdout fails %contains-regex rule" — in `Oscillator_4`, `Oscillator_5`, `Oscillator_9`, `Oscillator_10`.

### tests/packet — PASS

Single invocation with exit code 0. Output shows 3 "undisposed object" warnings for `inet::Packet` at teardown — these are printed to stdout but do not cause a test failure.

### tests/queueing — 57/57 fail

All 57 tests fail. The dominant failure mode is "test program returned exit code 1" (53 tests) with a few "stderr fails %contains rule" (4 tests). The `test.err` output from representative failing tests shows:

```
<!> Error: Submodule producer: Cannot resolve module type 'ActivePacketSource'
(not in the loaded NED files?), at .../test.ned:8
```

The test run script uses `-n ../../../../src:.` which does not appear to resolve the queueing module NED types from `inet.queueing.*`. This is a systematic baseline failure across the entire suite.

### tests/protocol — 13/13 fail

All 13 tests fail with "test program returned exit code 1". Same NED-path / module-resolution pattern as queueing (tests use `inet.queueing.*` and `inet.protocolelement.*` types).

### tests/module — 195/268 fail

268 tests total (.test.off and .test.fail files were excluded). The module test lib (`libtest_dbg.so`) and all 268 per-test binaries were built fresh for this run. Failure breakdown:
- 171 "test program returned exit code 1"
- 7 "stderr fails %contains rule"
- 6 "postrun-command(1).out fails %contains rule"
- 4 "stdout fails %contains rule"
- 3 "stdout fails %contains-regex rule"
- others (4 total)

73 passing tests include: ConvolutionalCoder12/34, diffserv_*, eth_*, ExternalProcess_1/2/4, Ieee80211BitDomain, Ieee80211SymbolDomain, IGMPv3_*, lifecycle_AccessPoint_*, lifecycle_IdealRadio_AP_*, Ieee8021d-Rstp, Ieee8021d-Stp, and others.

### Tool/Environment Note

`opp_test --help` crashes under Python 3.14 (argparse `_format_actions_usage` was renamed to `_get_actions_usage_parts`). The tool works normally when given actual arguments — only the `--help` path is broken. All tests were run successfully by invoking with explicit arguments.

### Excluded tests

- `tests/module/*.test.off` — 6 files (IdealRadio_1.test.off, IdealRadio_1e.test.off, IdealRadio_2.test.off, IdealRadio_2e.test.off, sctp_flowcontrol.test.off, sctp_nat_peer_to_peer.test.off)
- `tests/module/*.test.fail` — 2 files (ospf_backbone_and_3_areas_VirtualLink_HostInterface.test.fail, ospf_fig6_simple.test.fail)
