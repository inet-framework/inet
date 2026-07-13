# Phase 1.5 Final Baseline — smoke4 run

Date: 2026-07-13
OMNeT++ branch: topic/inet-infrastructure-clean
INET branch: topic/infrastructure (bde8051d8b)
Environment: source /home/levy/workspace/omnetpp/setenv -q, then source /home/levy/workspace/inet-infrastructure/setenv -q
**CRITICAL:** setenv must be sourced WITHOUT piping output (i.e. using -q flag, NOT `2>&1 | grep -v`).
Piping `source setenv` through a filter runs in a subshell and INET_ROOT does not propagate.
CWD for all test invocations: /home/levy/workspace/inet-infrastructure
Runner: opp_run_*_tests --load @opp --no-build (opp_repl 0.5.dev)
INET_ROOT verified: /home/levy/workspace/inet-infrastructure (not /home/levy/workspace/inet)

## Key discovery: previous smoke runs (smoke2/smoke3) tested the WRONG binary

The prior smoke runs used `source setenv 2>&1 | grep -v overwriting` which caused `source` to
execute in a subshell (due to the pipe). As a result, INET_ROOT remained `/home/levy/workspace/inet`
(the unmodified baseline) and the test runner loaded `/home/levy/workspace/inet/src/libINET.so`.

smoke4 is the FIRST run to correctly test `/home/levy/workspace/inet-infrastructure/src/libINET_dbg.so`.
The `--load @opp` flag loads the bundled inet.opp which uses `root_folder_environment_variable="INET_ROOT"`,
so once INET_ROOT is correctly set, the runner uses the right binary and test files.

## Release build result

| Component | Mode | Exit code |
|-----------|------|-----------|
| INET (inet-infrastructure) | release | **0** |

Last lines: `Creating shared library: ../out/clang-release/src/libINET.so`

## Test results

| Suite | Invocation | Total | PASS | FAIL | Failing test names | Log file |
|-------|-----------|-------|------|------|--------------------|----------|
| unit | `opp_run_unit_tests --load @opp --no-build` | 76 | **76** | 0 | — | smoke4-unit.log |
| packet | `opp_run_packet_tests --load @opp --no-build` | 1 | **1** | 0 | — | smoke4-packet.log |
| queueing | `opp_run_queueing_tests --load @opp --no-build` | 57 | **57** | 0 | — | smoke4-queueing.log |
| protocol | `opp_run_protocol_tests --load @opp --no-build` | 13 | **13** | 0 | — | smoke4-protocol.log |
| module | `opp_run_module_tests --load @opp --no-build` | 268 | **255** | 13 | see below | smoke4-module.log |

## Module suite failures (13 total)

### SCTP failures (11 tests) — segfault in SctpAssociationUtil.cc:532

All 11 sctp_* tests crash with a segmentation fault:

```
Segmentation fault (Address not mapped to object [(nil)])
#0 SctpAssociationUtil.cc:532 in sendEstabIndicationToApp: callback->handleEstablished(msg)
#1 SctpAssociationBase.cc:1083 in processAppCommand: sendEstabIndicationToApp()
#2 Sctp.cc:343 in handleMessage: assoc->processAppCommand(...)
#3 Sctp.cc:1005 in accept: handleMessage(cmsg)
#4 SctpSocket.cc:225: sctp->accept(socketId)
```

The null `callback` pointer at `sendEstabIndicationToApp` indicates the SCTP fix in commit
fd329a752c ("Sctp: Fixed available indication delivery to use the listening socket callback")
introduced a regression: the server-side association's callback is not initialized before
`sendEstabIndicationToApp()` is called during the accept path.

Failing tests:
- sctp_streamReset, sctp_prsctp_ttl, sctp_prsctp_rtx0, sctp_congestion,
- sctp_addip_setPrimary, sctp_addip_addAddress, sctp_auth, sctp_failover,
- sctp_prsctp_rtx1, sctp_streams, sctp_nat_peer_to_server

### UDPSocket_1 — golden mismatch (log message field name changed)

The test expects:
```
Connecting socket, socketId = 2, address = 10.0.0.1, port = 1000.
Binding socket, socketId = 2, localAddress = <none>, localPort = 100.
```
Actual output:
```
Connecting socket, socketId = 2, addr = 10.0.0.1, port = 1000.
Binding socket, socketId = 2, localAddr = <none>, localPort = 100.
```
Field names changed from `address`/`localAddress` to `addr`/`localAddr` in the UDP log messages.
The test golden in UDPSocket_1.test needs updating.

### ICMPv6_delivery — functional failure (ICMP error not delivered to application)

The simulation runs to completion. The first check passes:
- "Forwarding ICMPv6 error indication to transport protocol udp" IS found in output.

The second check fails:
- "ICMPv6 error received: type=1 code=0 about packet" NOT found in output.

The ICMP error message is forwarded by IPv6 to UDP but not delivered to the application layer.
This is a functional regression — the UDP socket is not delivering the ICMPv6 error indication
to the application callback.

## Flake re-check

Both ICMPv6_delivery and UDPSocket_1 were re-run 3 times in isolation:

| Test | Run 1 | Run 2 | Run 3 | Verdict |
|------|-------|-------|-------|---------|
| ICMPv6_delivery | FAIL | FAIL | FAIL | **NOT a flake — reproducible functional failure** |
| UDPSocket_1 | FAIL | FAIL | FAIL | **NOT a flake — reproducible golden mismatch** |

## Comparison with previous runs (smoke2/smoke3)

smoke2 showed 179/268 module failures. smoke3 (after fixes) showed fewer.
smoke4 is different in two key ways:
1. Correct binary: tests run against inet-infrastructure/src/libINET_dbg.so
2. Far fewer failures: 13 vs 179 — the infrastructure fixes are confirmed effective

The 13 remaining failures represent real open issues, not baseline noise:
- 11 SCTP: regression introduced by the SCTP accept callback fix (fd329a752c)
- 1 UDPSocket_1: golden mismatch (addr vs address field name)
- 1 ICMPv6_delivery: UDP not delivering ICMP errors to application

## Queueing improvement vs smoke2

smoke2 (against inet baseline): Gate_1, Gate_2, Gate_3, PeriodicGate_1 failed (4 failures).
smoke4 (against inet-infrastructure): 57/57 PASS.
The queueing fixes in inet-infrastructure are confirmed working.

## Artifact paths

- /home/levy/workspace/inet-infrastructure/plan/artifacts/phase0/smoke4-unit.log
- /home/levy/workspace/inet-infrastructure/plan/artifacts/phase0/smoke4-packet.log
- /home/levy/workspace/inet-infrastructure/plan/artifacts/phase0/smoke4-queueing.log
- /home/levy/workspace/inet-infrastructure/plan/artifacts/phase0/smoke4-protocol.log
- /home/levy/workspace/inet-infrastructure/plan/artifacts/phase0/smoke4-module.log
- /home/levy/workspace/inet-infrastructure/plan/artifacts/phase0/smoke4-summary.md (this file)
