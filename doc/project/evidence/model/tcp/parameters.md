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
