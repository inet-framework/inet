# Module Test Suite Census — smoke3

Date: 2026-07-13  
Log: `plan/artifacts/phase0/smoke3-module.log`

## Counts

| Total | Pass | Fail |
|-------|------|------|
| 268   | 249  | 19   |

## Comparison with Previous Census

Previous: 89/268 pass, 179 fail.  
Now: 249/268 pass, 19 fail.  
Net improvement: +160 pass.

Previous failure clusters resolved:
- 134 Udp-Icmp cast — **FIXED** (commit 8360ab4d9a / 075993e404)
- 30 LLC-ethernetmac — **FIXED**
- 7 stale-ID goldens — **FIXED** (this session: 9 tests fixed by %subst)
- 6 ND-ipv6 — partially resolved; 1 ICMPv6_delivery remains
- 1 PassivePacketSink-cast variant — **FIXED**
- 1 tun-echo — **FIXED** (passes now; was environment setup issue)

## Remaining Failures (19)

### Cluster 1: SCTP check_and_cast<IPassivePacketSink*> crash (11 tests)

Signature: Simulation crashes with `check_and_cast<IPassivePacketSink *>(referencedGate->getOwnerModule())` at `ModuleInterfaceLookup.cc:112` when SCTP receives a message on an unexpected gate. The `.sca` results file is empty (finish() not reached).

Tests:
- sctp_streamReset
- sctp_prsctp_ttl
- sctp_prsctp_rtx0
- sctp_prsctp_rtx1
- sctp_congestion
- sctp_addip_setPrimary
- sctp_addip_addAddress
- sctp_auth
- sctp_failover
- sctp_streams
- sctp_nat_peer_to_server

### Cluster 2: tcp_algorithm scalar format 1000000 → 1e+06 (6 tests)

Signature: Golden expects `scalar ClientServer.server.app[0] packetReceived:sum(packetBytes) 1000000` but actual output is `1e+06` (OMNeT++ changed scalar floating-point formatting).

Tests:
- tcp_algorithm_reno
- tcp_algorithm_dumb
- tcp_algorithm_vegas
- tcp_algorithm_tahoe
- tcp_algorithm_westwood
- tcp_algorithm_newreno

### Cluster 3: UDPSocket_1 log message format change (1 test)

Signature: Expected `multicastLoop = 0`, `address = 10.0.0.1`, `localAddress = <none>` but actual has `value = 0`, `addr = 10.0.0.1`, `localAddr = <none>` — parameter name abbreviations changed in Udp socket log messages.

### Cluster 4: ICMPv6_delivery missing log line (1 test)

Signature: Third `%contains: stdout` check for `Ignoring UDP error report` not found in stdout. The simulation completes but the UDP layer no longer logs this message (possible ND/IPv6 cluster remnant).
