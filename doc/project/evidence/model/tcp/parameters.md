# TCP — the catalog of the parameter tests

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [Tcp.ned](../../../../../src/inet/transportlayer/tcp/Tcp.ned), [the plan](../../../../../plan/pending/tcp-parameter-tests.md)

The classes of values of each NED parameter of the TCP modules, and the parameter test of each class. A class is a set of values that take the same code path and give the same documented behavior. The tests are in [`tests/protocol/tcp/parameter/`](../../../../../tests/protocol/tcp/parameter); each test sets its parameter to the value of its row, and the cross-class check makes sure that it fails with the value of each other row of its parameter.

This document is a ledger: each group of parameters writes its rows when its tests are written, and the verdict column changes with each run. [`check_catalog.py`](../../../../../tests/protocol/tcp/parameter/check_catalog.py) compares the rows with the NED files and the test files, and lists the parameters that have no rows yet.

## How to read a row

- **Class**: `<Module>.<parameter>:<class>`. A class with more than one test has one row for each facet: `linux-quickack`, `linux-ato`, `linux-pingpong` are three facets of the value `true`.
- **Value**: the NED expression text that the test sets, the representative value of the class.
- **Preset**: `LinuxTcp` or `LegacyTcp` when that module sets its default to a value of this class; — when no preset does. A preset has no tests of its own: each of its defaults falls into one class.
- **Claim**: the behavior that separates the class from the other classes of the parameter.
- **Source**: where the class comes from: "NED" (the comment of the parameter), "code" (a condition in the code, with the function), or both.
- **Obs**: how the test observes the class: W the wire, A the application, S a signal, I the internal state (`assertModule`), E the error at the open.
- **Net**: P `TapPairNetwork`, R `TapRouterNetwork`, F `TapSwitchNetwork`; T a tap rule, K a socket call of `TcpTestApp`, M a change of a parameter during the run.
- **Verdict**: the result of the last run: PASS, FAIL, or FAIL (expected) with its reason in the findings.

## Findings

Defects and gaps that the parameter tests found. This pass changes nothing in `src/`; a later plan repairs them.

| Id | Finding | Where | Found by |
| --- | --- | --- | --- |
| PF-1 | `TcpSessionApp` closes the connection at once after its last send when `tClose = -1`; its NED says that -1 issues no CLOSE. The tests use a close time after the end of the run. | `TcpSessionApp::handleTimer()`, the branch after the last command | the first test of the group `ack` |
| PF-2 | `Tcp.delayedAckTimeout` has no check: a negative value reaches the scheduler, and the run stops with its error instead of a message about the parameter. No class tests it, because the NED does not name such a value. | `TcpAlgorithm::initialize()` | the code pass of the group `ack` |
| PF-3 | `Tcp.windowScalingFactor`: the NED does not say that a factor whose largest window is below `advertisedWindow` stops the run with an error. | `Tcp.ned` | the code pass of the group `setup` |
| PF-4 | `Tcp.windowScalingFactor = -1` can select a factor that is one too small, so the connection cannot offer its whole `advertisedWindow`: the selection shifts the window down with truncation (131071 bytes get the factor 1, whose largest window is 131070), and it starts from one byte less than the window (262144 bytes get 2, whose largest window is 262140; Linux `tcp_select_initial_window()` gives 3). The NED says "the smallest shift count that represents the largest window". | `TcpConnection::writeHeaderOptions()`, the Window Scale option | `TcpWindowScalingFactorAuto` |
| PF-5 | `Tcp.nagleEnabled`: TCP_NODELAY of a socket takes the place of the parameter for its connection, and the NED does not say so, as it does for `notsentLowat` and `keepAliveEnabled`. | `Tcp.ned`; `TcpConnection::initConnection()` | the code pass of the group `send` |

## The classes

### ack — acknowledgments

Run on 2026-10-29, worktree `inet-tcp-parameter-tests`, debug mode: 13 PASS.

| Class | Value | Preset | Claim | Source | Obs | Net | Test | Verdict |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `Tcp.delayedAcksEnabled:on` | `true` | — | a lone segment gets no ACK at once, and its ACK comes before 0.5 s | NED; code: `TcpAlgorithmBase::receiveSeqChanged()` | W | P | [TcpDelayedAcksEnabledOn](../../../../../tests/protocol/tcp/parameter/ack/TcpDelayedAcksEnabledOn.test) | PASS |
| `Tcp.delayedAcksEnabled:off` | `false` | LegacyTcp | each segment gets its ACK at once | NED; code: the same | W | P | [TcpDelayedAcksEnabledOff](../../../../../tests/protocol/tcp/parameter/ack/TcpDelayedAcksEnabledOff.test) | PASS |
| `Tcp.delayedAckTimeout:value` | `100ms` | — | a lone segment gets its ACK after this time | NED; code: `receiveSeqChanged()` | W | P | [TcpDelayedAckTimeoutValue](../../../../../tests/protocol/tcp/parameter/ack/TcpDelayedAckTimeoutValue.test) | PASS |
| `Tcp.delayedAckTimeout:zero` | `0s` | — | a lone segment gets its ACK at once | NED | W | P | [TcpDelayedAckTimeoutZero](../../../../../tests/protocol/tcp/parameter/ack/TcpDelayedAckTimeoutZero.test) | PASS |
| `Tcp.delayedAckFrameCount:one` | `1` | — | each data segment gets its own ACK | NED; code: `receiveSeqChanged()` | W | P | [TcpDelayedAckFrameCountOne](../../../../../tests/protocol/tcp/parameter/ack/TcpDelayedAckFrameCountOne.test) | PASS |
| `Tcp.delayedAckFrameCount:many` | `2` | — | one ACK for each second full-sized segment (2 is the default) | NED; code: the same | W | P | [TcpDelayedAckFrameCountMany](../../../../../tests/protocol/tcp/parameter/ack/TcpDelayedAckFrameCountMany.test) | PASS |
| `Tcp.delayedAckFrameCount:invalid` | `0` | — | the error "delayedAckFrameCount must be 1 or more" | NED; code: `TcpConnection::initConnection()` | E | P | [TcpDelayedAckFrameCountInvalid](../../../../../tests/protocol/tcp/parameter/ack/TcpDelayedAckFrameCountInvalid.test) | PASS |
| `Tcp.dsackEnabled:on` | `true` | — | the ACK of a duplicate segment starts its SACK option with a D-SACK block | NED; code: `addSacks()` | W | P, T | [TcpDsackEnabledOn](../../../../../tests/protocol/tcp/parameter/ack/TcpDsackEnabledOn.test) | PASS |
| `Tcp.dsackEnabled:off` | `false` | — | the ACK of a duplicate segment has no D-SACK block | NED; code: the same | W | P, T | [TcpDsackEnabledOff](../../../../../tests/protocol/tcp/parameter/ack/TcpDsackEnabledOff.test) | PASS |
| `Tcp.adaptiveDelayedAcks:rfc` | `false` | — | the first data of a connection waits for the delayed ACK | NED | W | P | [TcpAdaptiveDelayedAcksRfc](../../../../../tests/protocol/tcp/parameter/ack/TcpAdaptiveDelayedAcksRfc.test) | PASS |
| `Tcp.adaptiveDelayedAcks:linux-quickack` | `true` | LinuxTcp | the quickack mode acknowledges the first data of a connection at once | NED; code: `dataArrivedAtoUpdate()` | W | P | [TcpAdaptiveDelayedAcksLinuxQuickack](../../../../../tests/protocol/tcp/parameter/ack/TcpAdaptiveDelayedAcksLinuxQuickack.test) | PASS |
| `Tcp.adaptiveDelayedAcks:linux-ato` | `true` | LinuxTcp | after the quickack budget, a lone segment waits for the ATO (40 ms), not for `delayedAckTimeout` | NED; code: `scheduleDelayedAck()` | W | P | [TcpAdaptiveDelayedAcksLinuxAto](../../../../../tests/protocol/tcp/parameter/ack/TcpAdaptiveDelayedAcksLinuxAto.test) | PASS |
| `Tcp.adaptiveDelayedAcks:linux-pingpong` | `true` | LinuxTcp | after a reply, the ACK of the next request goes with the next reply | NED; code: `dataSent()`, `inQuickackMode()` | W | P | [TcpAdaptiveDelayedAcksLinuxPingpong](../../../../../tests/protocol/tcp/parameter/ack/TcpAdaptiveDelayedAcksLinuxPingpong.test) | PASS |

### setup — connection setup and header options

Run on 2026-10-29, worktree `inet-tcp-parameter-tests`, debug mode: 33 PASS, 1 FAIL (PF-4). An algorithm without SACK recovery turns `sackSupport` off; on the wire that is the class `off`, so the tests of `tcpAlgorithmClass` check it, not a class of `sackSupport`.

| Class | Value | Preset | Claim | Source | Obs | Net | Test | Verdict |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `Tcp.initialSendSequenceNumber:clock` | `-1` | — | the ISS of the SYN is its time times 250000, modulo 2^32 | NED; code: `selectInitialSeqNum()` | W | P | [TcpInitialSendSequenceNumberClock](../../../../../tests/protocol/tcp/parameter/setup/TcpInitialSendSequenceNumberClock.test) | PASS |
| `Tcp.initialSendSequenceNumber:fixed` | `1000` | — | the ISS of each connection is the value | NED; code: the same | W | P | [TcpInitialSendSequenceNumberFixed](../../../../../tests/protocol/tcp/parameter/setup/TcpInitialSendSequenceNumberFixed.test) | PASS |
| `Tcp.initialSendSequenceNumber:random` | `intuniform(0, 1000000000)` | — | two connections that open at the same time have different ISSs | NED (volatile) | W | P | [TcpInitialSendSequenceNumberRandom](../../../../../tests/protocol/tcp/parameter/setup/TcpInitialSendSequenceNumberRandom.test) | PASS |
| `Tcp.initialSendSequenceNumber:invalid` | `-2` | — | the error "initialSendSequenceNumber must be -1 or 0 to 4294967295" | code: the same | E | P | [TcpInitialSendSequenceNumberInvalid](../../../../../tests/protocol/tcp/parameter/setup/TcpInitialSendSequenceNumberInvalid.test) | PASS |
| `Tcp.mss:auto-ipv4` | `-1` | — | an IPv4 host announces the MSS 1460 | NED; code: `resolveAddressFamilyMss()` | W | P | [TcpMssAutoIpv4](../../../../../tests/protocol/tcp/parameter/setup/TcpMssAutoIpv4.test) | PASS |
| `Tcp.mss:auto-ipv6` | `-1` | — | an IPv6 host announces the MSS 1440 | NED; code: the same | W | P6 | [TcpMssAutoIpv6](../../../../../tests/protocol/tcp/parameter/setup/TcpMssAutoIpv6.test) | PASS |
| `Tcp.mss:explicit` | `536` | LegacyTcp | the host announces the value | NED; code: `initConnection()` | W | P | [TcpMssExplicit](../../../../../tests/protocol/tcp/parameter/setup/TcpMssExplicit.test) | PASS |
| `Tcp.mss:invalid` | `87` | — | the error "mss must be -1 or at least 88" | NED; code: the same | E | P | [TcpMssInvalid](../../../../../tests/protocol/tcp/parameter/setup/TcpMssInvalid.test) | PASS |
| `Tcp.sendMssOption:on` | `true` | — | the SYN has an MSS option | NED | W | P | [TcpSendMssOptionOn](../../../../../tests/protocol/tcp/parameter/setup/TcpSendMssOptionOn.test) | PASS |
| `Tcp.sendMssOption:off` | `false` | — | the SYN has no MSS option | NED | W | P | [TcpSendMssOptionOff](../../../../../tests/protocol/tcp/parameter/setup/TcpSendMssOptionOff.test) | PASS |
| `Tcp.alignOptions:aligned` | `true` | — | NOP options align the options of the SYN | NED | W | P | [TcpAlignOptionsAligned](../../../../../tests/protocol/tcp/parameter/setup/TcpAlignOptionsAligned.test) | PASS |
| `Tcp.alignOptions:packed` | `false` | — | the SYN has no NOP option | NED | W | P | [TcpAlignOptionsPacked](../../../../../tests/protocol/tcp/parameter/setup/TcpAlignOptionsPacked.test) | PASS |
| `Tcp.windowScalingSupport:on` | `true` | — | the SYN has a Window Scale option | NED | W | P | [TcpWindowScalingSupportOn](../../../../../tests/protocol/tcp/parameter/setup/TcpWindowScalingSupportOn.test) | PASS |
| `Tcp.windowScalingSupport:off` | `false` | LegacyTcp | the SYN has no Window Scale option | NED | W | P | [TcpWindowScalingSupportOff](../../../../../tests/protocol/tcp/parameter/setup/TcpWindowScalingSupportOff.test) | PASS |
| `Tcp.windowScalingFactor:auto` | `-1` | — | the shift count is the smallest that represents advertisedWindow: 3 for 262144 | NED; code: the options of the SYN | W | P | [TcpWindowScalingFactorAuto](../../../../../tests/protocol/tcp/parameter/setup/TcpWindowScalingFactorAuto.test) | FAIL (PF-4) |
| `Tcp.windowScalingFactor:explicit` | `7` | — | the shift count is the value | NED | W | P | [TcpWindowScalingFactorExplicit](../../../../../tests/protocol/tcp/parameter/setup/TcpWindowScalingFactorExplicit.test) | PASS |
| `Tcp.windowScalingFactor:invalid` | `15` | — | the error "Invalid parameter value windowScalingFactor=15" | NED; code: `configureStateVariables()` | E | P | [TcpWindowScalingFactorInvalid](../../../../../tests/protocol/tcp/parameter/setup/TcpWindowScalingFactorInvalid.test) | PASS |
| `Tcp.windowScalingFactor:too-small` | `0` | — | a factor that cannot represent advertisedWindow stops the run with an error | code: the same (PF-3) | E | P | [TcpWindowScalingFactorTooSmall](../../../../../tests/protocol/tcp/parameter/setup/TcpWindowScalingFactorTooSmall.test) | PASS |
| `Tcp.timestampSupport:on` | `true` | — | the SYN has a Timestamps option | NED | W | P | [TcpTimestampSupportOn](../../../../../tests/protocol/tcp/parameter/setup/TcpTimestampSupportOn.test) | PASS |
| `Tcp.timestampSupport:off` | `false` | LegacyTcp | the SYN has no Timestamps option | NED | W | P | [TcpTimestampSupportOff](../../../../../tests/protocol/tcp/parameter/setup/TcpTimestampSupportOff.test) | PASS |
| `Tcp.timestampSupport:changed` | `true` | — | a change during the run applies to the next connection | NED (@mutable) | W | P, M | [TcpTimestampSupportChanged](../../../../../tests/protocol/tcp/parameter/setup/TcpTimestampSupportChanged.test) | PASS |
| `Tcp.sackSupport:on` | `true` | — | the SYN has a SACK-Permitted option | NED | W | P | [TcpSackSupportOn](../../../../../tests/protocol/tcp/parameter/setup/TcpSackSupportOn.test) | PASS |
| `Tcp.sackSupport:off` | `false` | LegacyTcp | the SYN has no SACK-Permitted option | NED | W | P | [TcpSackSupportOff](../../../../../tests/protocol/tcp/parameter/setup/TcpSackSupportOff.test) | PASS |
| `Tcp.synRetries:active` | `2` | LinuxTcp | the SYN goes this many times again, then the open fails | NED; code: `process_TIMEOUT_SYN_REXMIT()` | W, A | P, T, K | [TcpSynRetriesActive](../../../../../tests/protocol/tcp/parameter/setup/TcpSynRetriesActive.test) | PASS |
| `Tcp.synRetries:passive` | `2` | LinuxTcp | the SYN-ACK goes this many times again | NED; code: the same | W | P, T, K | [TcpSynRetriesPassive](../../../../../tests/protocol/tcp/parameter/setup/TcpSynRetriesPassive.test) | PASS |
| `Tcp.synRetries:zero` | `0` | — | the SYN goes once, and the open fails at the first timeout | code: the same | W, A | P, T, K | [TcpSynRetriesZero](../../../../../tests/protocol/tcp/parameter/setup/TcpSynRetriesZero.test) | PASS |
| `Tcp.sendDataWithFirstAck:on` | `true` | — | data that waits during the handshake goes with the ACK of the SYN-ACK | NED; code: `TcpAlgorithmBase::established()` | W | P, K | [TcpSendDataWithFirstAckOn](../../../../../tests/protocol/tcp/parameter/setup/TcpSendDataWithFirstAckOn.test) | PASS |
| `Tcp.sendDataWithFirstAck:off` | `false` | — | a bare ACK ends the handshake, and the data follows | NED; code: the same | W | P, K | [TcpSendDataWithFirstAckOff](../../../../../tests/protocol/tcp/parameter/setup/TcpSendDataWithFirstAckOff.test) | PASS |
| `Tcp.synLinearTimeouts:doubling` | `0` | — | the timeout of a SYN doubles at each expiry | NED; code: `process_TIMEOUT_SYN_REXMIT()` | W | P, T | [TcpSynLinearTimeoutsDoubling](../../../../../tests/protocol/tcp/parameter/setup/TcpSynLinearTimeoutsDoubling.test) | PASS |
| `Tcp.synLinearTimeouts:linear` | `2` | LinuxTcp | the first retransmissions of a SYN keep the initial timeout | NED; code: the same | W | P, T | [TcpSynLinearTimeoutsLinear](../../../../../tests/protocol/tcp/parameter/setup/TcpSynLinearTimeoutsLinear.test) | PASS |
| `Tcp.syncookiesAlways:off` | `false` | — | the server sends segments of the MSS of the peer | NED | W | P | [TcpSyncookiesAlwaysOff](../../../../../tests/protocol/tcp/parameter/setup/TcpSyncookiesAlwaysOff.test) | PASS |
| `Tcp.syncookiesAlways:on` | `true` | — | the cookie quantizes the MSS of the peer down to {536, 1300, 1440, 1460} | NED; code: `processSegmentInListen()` | W | P | [TcpSyncookiesAlwaysOn](../../../../../tests/protocol/tcp/parameter/setup/TcpSyncookiesAlwaysOn.test) | PASS |
| `Tcp.dropSynData:keep` | `false` | — | the data of a SYN waits in the receive queue, and the end of the handshake processes it | NED; code: `processSegmentInSynRcvd()` | W | P, T, K | [TcpDropSynDataKeep](../../../../../tests/protocol/tcp/parameter/setup/TcpDropSynDataKeep.test) | PASS |
| `Tcp.dropSynData:drop` | `true` | LinuxTcp | the data of a SYN is dropped, and the client must send it again | NED; code: `processSegmentInListen()` | W | P, T, K | [TcpDropSynDataDrop](../../../../../tests/protocol/tcp/parameter/setup/TcpDropSynDataDrop.test) | PASS |

### send — sending data

Run on 2026-10-29, worktree `inet-tcp-parameter-tests`, debug mode: 17 PASS. The zero-window tests use a `TcpTestApp` at host2 that never reads. A class whose name ends with `-override` claims that a socket option takes the place of the parameter; its test passes with each value of the parameter, so the cross-class check skips it.

| Class | Value | Preset | Claim | Source | Obs | Net | Test | Verdict |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `Tcp.nagleEnabled:on` | `true` | — | a small segment waits while sent data is not acknowledged | NED; code: `sendData()` | W | P | [TcpNagleEnabledOn](../../../../../tests/protocol/tcp/parameter/send/TcpNagleEnabledOn.test) | PASS |
| `Tcp.nagleEnabled:off` | `false` | — | a small segment goes at once | NED; code: the same | W | P | [TcpNagleEnabledOff](../../../../../tests/protocol/tcp/parameter/send/TcpNagleEnabledOff.test) | PASS |
| `Tcp.pushLastBufferedSegment:on` | `true` | — | the last buffered segment carries PSH | NED; code: `sendSegment()` | W | P | [TcpPushLastBufferedSegmentOn](../../../../../tests/protocol/tcp/parameter/send/TcpPushLastBufferedSegmentOn.test) | PASS |
| `Tcp.pushLastBufferedSegment:off` | `false` | LegacyTcp | no segment carries PSH | NED | W | P | [TcpPushLastBufferedSegmentOff](../../../../../tests/protocol/tcp/parameter/send/TcpPushLastBufferedSegmentOff.test) | PASS |
| `Tcp.zeroWindowProbe:rfc` | `"rfc"` | — | the probe of a zero window carries the next octet of new data | NED; code: `sendProbe()` | W | P, K | [TcpZeroWindowProbeRfc](../../../../../tests/protocol/tcp/parameter/send/TcpZeroWindowProbeRfc.test) | PASS |
| `Tcp.zeroWindowProbe:linux` | `"linux"` | LinuxTcp | the probe has no data and the sequence number SND.UNA - 1 | NED; code: the same | W | P, K | [TcpZeroWindowProbeLinux](../../../../../tests/protocol/tcp/parameter/send/TcpZeroWindowProbeLinux.test) | PASS |
| `Tcp.maxPersistTimeout:caps-growth` | `4s` | LinuxTcp | the probe interval starts at the RTO, doubles, and stops at the value | NED; code: `processPersistTimer()` | W | P, K | [TcpMaxPersistTimeoutCapsGrowth](../../../../../tests/protocol/tcp/parameter/send/TcpMaxPersistTimeoutCapsGrowth.test) | PASS |
| `Tcp.maxPersistTimeout:caps-first` | `0.5s` | — | a value below the RTO is the first probe interval too | NED; code: `startPersistTimer()` | W | P, K | [TcpMaxPersistTimeoutCapsFirst](../../../../../tests/protocol/tcp/parameter/send/TcpMaxPersistTimeoutCapsFirst.test) | PASS |
| `Tcp.maxPersistTimeout:invalid` | `0s` | — | the error "maxPersistTimeout must be more than 0" | NED; code: `TcpAlgorithm::initialize()` | E | P | [TcpMaxPersistTimeoutInvalid](../../../../../tests/protocol/tcp/parameter/send/TcpMaxPersistTimeoutInvalid.test) | PASS |
| `Tcp.minshallNagle:off` | `false` | — | the short end of a write waits while full-sized segments are not acknowledged | NED; code: `sendData()` | W | P | [TcpMinshallNagleOff](../../../../../tests/protocol/tcp/parameter/send/TcpMinshallNagleOff.test) | PASS |
| `Tcp.minshallNagle:on` | `true` | LinuxTcp | full-sized segments in flight do not hold the short end of a write | NED; code: the same | W | P | [TcpMinshallNagleOn](../../../../../tests/protocol/tcp/parameter/send/TcpMinshallNagleOn.test) | PASS |
| `Tcp.pushSegmentsOnWriteBoundary:off` | `false` | — | the end of a write that is not the end of the buffer has no PSH | NED; code: `sendSegment()` | W | P, K | [TcpPushSegmentsOnWriteBoundaryOff](../../../../../tests/protocol/tcp/parameter/send/TcpPushSegmentsOnWriteBoundaryOff.test) | PASS |
| `Tcp.pushSegmentsOnWriteBoundary:write-end` | `true` | LinuxTcp | the segment that ends a write has PSH | NED; code: the same | W | P, K | [TcpPushSegmentsOnWriteBoundaryWriteEnd](../../../../../tests/protocol/tcp/parameter/send/TcpPushSegmentsOnWriteBoundaryWriteEnd.test) | PASS |
| `Tcp.pushSegmentsOnWriteBoundary:long-write` | `true` | LinuxTcp | a write longer than half the largest window has PSH inside it | NED; code: the marks of a SEND | W | P, K | [TcpPushSegmentsOnWriteBoundaryLongWrite](../../../../../tests/protocol/tcp/parameter/send/TcpPushSegmentsOnWriteBoundaryLongWrite.test) | PASS |
| `Tcp.notsentLowat:off` | `-1B` | — | the application gets no send-ready indication | NED; code: `sendData()` | A | P, K | [TcpNotsentLowatOff](../../../../../tests/protocol/tcp/parameter/send/TcpNotsentLowatOff.test) | PASS |
| `Tcp.notsentLowat:mark` | `4096B` | — | when the unsent bytes fall to the mark, the application gets one send-ready indication | NED; code: the same | A | P, K | [TcpNotsentLowatMark](../../../../../tests/protocol/tcp/parameter/send/TcpNotsentLowatMark.test) | PASS |
| `Tcp.notsentLowat:socket-override` | `-1B` | — | TCP_NOTSENT_LOWAT of the socket takes the place of the parameter | NED; code: `initConnection()` | A | P, K | [TcpNotsentLowatSocketOverride](../../../../../tests/protocol/tcp/parameter/send/TcpNotsentLowatSocketOverride.test) | PASS |
