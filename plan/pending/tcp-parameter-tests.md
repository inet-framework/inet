# TCP parameter tests — one test for each value class of each NED parameter of TCP

**Status:** pending, design only. Written 2026-10-29 from `master` at `3e51de989f`. No test exists yet. The user settled D1 to D7 on 2026-10-29 (section "Decisions").

## The goal

Start a new class of tests: **parameter tests**. A parameter test checks that one NED parameter of one module operates as its documentation says, for one class of values. The first collection covers every parameter of every module in `src/inet/transportlayer/tcp/`.

The second goal is coverage. The TCP protocol tests of INET alone cover 47.2 % of the TCP source lines, and 73.3 % together with the packetdrill wrappers of inet-gpl (local report of 2026-10-27, `/home/levy/workspace/tcp-coverage/report-*.txt`). Six algorithm classes have 0 %: `DcTcp`, `DumbTcp`, `TcpTahoe`, `TcpVegas`, `TcpWestwood`, `TcpNoCongestionControl`. Most of the missed lines are behind a parameter value that no test sets. A test for each value class reaches these lines by construction.

## What a parameter test is

- **The subject** is one value class of one parameter of one module. A value class is a set of values that take the same code path and give the same documented behavior. Example: `Tcp.delayedAckFrameCount` has four classes: `one` (1), `two` (2, the default), `many` (3 and more), `invalid` (0 and less).
- **The claim** is the NED documentation of the parameter. A behavior that the code has and the documentation does not state is an implicit claim (the third principle of [derive-tests-from-a-standard.md](../../doc/project/guide/derive-tests-from-a-standard.md#principle-a-claimed-feature-gets-a-test)). The test then states the behavior from the code comment or the cited standard, and the catalog records a documentation gap.
- **The assertion discriminates.** The test must fail when the parameter has a value of another class. A test that passes for every value adds coverage but no evidence. The cross-class check (I5) runs each test with the values of the other classes and makes sure that it fails.
- **One parameter changes.** The test sets the parameter under test explicitly, also when the class holds the default. It also sets the preconditions that the class needs (for example `windowAutoTuning = true` for `receiveBufferLowRtt`), and nothing else. The other parameters keep the defaults of `Tcp`, not of `LinuxTcp` or `LegacyTcp`.
- **The host under test** is named in the description. The parameter is set only on that host. The peer keeps the defaults, unless the class needs a peer setting, which the description then names.

### The observation, in the order of preference

| Level | Channel | Example |
| --- | --- | --- |
| 1, black box | the segments on the wire, at `hostN.tcp` or on the tap | the MSS option of the SYN; the time of the first retransmission |
| 1, black box | the application: the delivered bytes, the indications | the urgent byte is not delivered |
| 2, published interface | the declared signals of `TcpConnection` (`cwnd`, `ssthresh`, `rto`, `srtt`, `rcvWnd`, `state`, `numRtos`, `load`, ...) | `ssthresh` after a loss with `TcpCubic.beta = 0.5` |
| 3, white box | the internal state of the connection, read by its class descriptor (E3), or in a lambda where no descriptor field exists | the entries of the Fast Open cookie cache |
| — | the error at the open of a connection | `mss = 87`: "mss must be -1 or at least 88" |

Use the first level that can show the class. Use level 3 only when levels 1 and 2 cannot show it, and give the reason in the description. An `invalid` class is an error test: the simulation must stop with the documented message.

### The relation to the existing test categories

A TCP parameter test uses the protocol test library, so its category under [TR-CAT-MATCH](../../doc/project/rule/testing.md#tr-cat-match) stays `protocol`. "Parameter test" names the derivation, as "standards test" does: the standards tests start from a standard, the parameter tests start from the NED parameters. For a module that is not a protocol, the same derivation can put its tests in `tests/module/<subject>/parameter/`. [test-anatomy.md](../../doc/project/design/test-anatomy.md) therefore gets no thirteenth category, only a paragraph about the derivation (step 19).

## The scope

| Module | Folder | Parameters | Note |
| --- | --- | --- | --- |
| `Tcp` | `tcp/` | 93 | in 13 groups, which `Tcp.ned` already has (`// acknowledgments`, ...) |
| `TcpCubic` | `tcp/flavours/` | 12 | |
| `TcpVegas` | `tcp/flavours/` | 3 | |
| `DcTcp` | `tcp/flavours/` | 1 | |
| `TcpNewReno`, `TcpReno`, `TcpTahoe`, `TcpWestwood`, `TcpNoCongestionControl`, `DumbTcp` | `tcp/flavours/` | 0 | each is a class of `Tcp.tcpAlgorithmClass` |
| `TcpConnection`, `TcpAlgorithm` | `tcp/`, `tcp/flavours/` | 0 | no test |
| `LinuxTcp`, `LegacyTcp` | `tcp/` | 0 new, 40 and 17 default overrides | no test of their own (D6): each value that they set is in a class of its parameter, and the catalog names the preset of each class |
| `TcpSpoof` | `tcp_common/` | 7 | not in `tcp/`, but in the scope (D2) |

The C++ code reads exactly the 115 parameter names that these NED files declare (`par("...")` in `tcp/` and `tcp_common/`); there is no hidden parameter.

Out of the scope: `tcp_lwip/`, the TCP applications, and the socket API of `TcpSocket`. A socket option that overrides a parameter (`setKeepAlive`, `setNotsentLowat`, the algorithm of the open command) is a class of that parameter, so its tests are in the scope.

## Where the tests go

The tests go in a subfolder of the TCP suite, `tests/protocol/tcp/parameter/` (D1). The runner finds it with no change (every test below `tests/protocol/tcp` belongs to the `tcp` suite), the protocol workflow runs it on each push, and the weekly coverage workflow includes it.

```
tests/protocol/lib/                   the library of all suites (on the NED path of each test)
    TapPairNetwork.ned, ...           new: the shared networks (section "The shared networks")
    TcpTestApp.{ned,h,cc}             new: the owner of the TCP sockets of a test (section "Socket calls")
tests/protocol/tcp/
    TcpMutations.h                    moved from rfc/: the typed rewrites that stay C++ (TCP options)
    parameter/
        README.md                     the class, the rules, how to run, a link to the catalog
        check_catalog.py              the completeness check (I4)
        cross_class.py                the cross-class check (I5)
        ini/_base.ini, _bulk.ini, ...  shared ini fragments, as tests/protocol/wifi/ini
        module/                       the module: checksumMode, stopOperation*
        setup/                        connection setup and header options
        receive/                      receive window and buffer
        ack/                          acknowledgments
        send/                         sending data
        congestion/                   congestion control, tcpAlgorithmClass, TcpCubic, TcpVegas, DcTcp
        recovery/                     loss detection and recovery
        rto/                          retransmission timeout
        ecn/                          ECN and AccECN
        pmtu/                         path MTU
        keepalive/                    keepalive
        fastopen/                     TCP Fast Open
        close/                        connection close, the checks of arriving segments and ICMP errors
        spoof/                        TcpSpoof (D2)
doc/project/evidence/model/tcp/parameters.md   the catalog and ledger (step 1)
```

The folders follow the parameter groups of `Tcp.ned`. A folder holds 6 to 60 tests, and `-f parameter/ack` runs one group.

**The file name** is `<Module><Parameter><Class>.test` in PascalCase ([NR-TEST](../../doc/project/rule/naming.md#nr-test)): `TcpDelayedAckFrameCountOne.test`, `TcpCubicBetaInvalid.test`. When the parameter name starts with the module name, the module is not written two times: `TcpAlgorithmClassVegas.test`, `TcpEcnModeAccecn.test`. The name must be unique in the whole `tcp` suite, because the runner builds every test of a suite in `tests/protocol/tcp/work/<file name>/`. The names `Rfc*` and `tcp_*` of the other folders do not collide with this scheme.

**The class id** is `<Module>.<parameter>:<class>`, for example `Tcp.delayedAckFrameCount:one`. The test program has the name of the file, so the verdict line is `PROTOCOLTEST TcpDelayedAckFrameCountOne: PASS`.

**The metadata** is in `%#` lines, which opp_test ignores, as `%# expected-result:` already is:

```
%# parameter: Tcp.delayedAckFrameCount
%# class: one
%# value: 1
%# expected-result: PASS
```

## The anatomy of a parameter test

```
%description:

Tcp.delayedAckFrameCount:one: with the value 1, the receiver acknowledges each full-sized
segment at once.

The claim (Tcp.ned): "with delayedAcksEnabled, the ACK goes at once when this number of
full-sized segments waits for it, or when the unacknowledged data reach this number times
RMSS [...]; at least 1".

The classes: one (1), two (2, the default), many (4), invalid (0). This test separates "one"
from "two" and "many": with 2 or 4, the first segment of the burst gets no ACK of its own.

Host under test: host2, the receiver. Peer: host1, a bulk sender with the defaults of Tcp.
Observation: level 1, the ACKs that host2 sends.

%# parameter: Tcp.delayedAckFrameCount
%# class: one
%# value: 1
%# expected-result: PASS

%includes:
#include "ProtocolTest.h"

%global:
// the program: an ACK from host2 for the first full-sized segment, within 1 ms of its arrival

%activity:
EV << "compiled\n";

%inifile: omnetpp.ini
[General]
network = inet.protocoltest.TapPairNetwork
include ../../parameter/ini/_base.ini
include ../../parameter/ini/_bulk.ini
*.host2.tcp.delayedAckFrameCount = 1        # the parameter under test

%contains: stdout
PROTOCOLTEST TcpDelayedAckFrameCountOne: PASS
```

An error test has no program. It opens one connection and checks the message:

```
%# parameter: Tcp.delayedAckFrameCount
%# class: invalid
%# value: 0
%exitcode: 1
%contains-regex: stderr
delayedAckFrameCount must be 1 or more, but it is 0
```

## How to find the value classes of a parameter

Do these steps for each parameter, one group at a time, just before the tests of the group are written (the code pass).

1. Read the NED comment. Write down each value or range that it names: the special values (`-1`, `0`, `""`), the enum values, the bounds, the errors, the default expression, the preconditions ("requires sackSupport"), and a socket option that overrides it.
2. Read the code that reads the parameter: the `par()` call, and each use of the member that keeps its value. Each condition on the value is a boundary. The values between two boundaries are one class. A validation that throws gives the class `invalid`.
3. A `@mutable` parameter gets the class `changed`: an action step (E4) changes the value during the run, and the next connection uses the new value while the open connection keeps the old one (the NED comments say "each connection reads it at its open").
4. A `volatile` parameter gets a class for a distribution, if the code reads it more than one time.
5. A class whose behavior has more than one independent part gets one test for each part (a facet), for example the quickack, ATO and pingpong parts of `adaptiveDelayedAcks = true`. The file names add the facet: `TcpAdaptiveDelayedAcksQuickack.test`.
6. A combination of values of the peer is not a class. Select the peer that makes the class visible. The standards tests check the negotiation.
7. Choose one representative value for each class: the default when it is in the class, else a typical value; a boundary value only when the code treats the boundary specially.
8. Write the classes into the catalog with their source: "NED", or "code" with the file and line. A class with source "code" and no NED statement is a documentation gap.

## The shared infrastructure

The first version of this plan had a TCP helper header with segment builders, ICMP builders, observers and state accessors. The review of 2026-10-29 replaced it with real network behavior and with generic constructs of the protocol test library. Each construct below serves every suite, not only TCP.

### The findings of the review

- **No shared network for TCP.** Each test of `tcp/rfc/` carries its network in its own `%file: test.ned`: the 8 copies of `Rfc9293TapNet` are byte-identical, and `Rfc9293TcpNet` has 6 copies. The other suites do the same (`NdLink` 25 copies, `MldLink` 24, `IpsecLink` 23, ...). Only the wifi suite shares a network: `tests/protocol/wifi/ned/WifiInfraNetwork.ned`.
- **Two shapes repeat across the suites.** "host — tap — host" (`Rfc9293TapNet`, `QuicTapNet`) and "host — router — tap — host" (`Rfc768TapNet`, `Rfc791TapNet`, and `Rfc8200TapNet` with the IPv6 node types).
- **The library reads a field by a path** (`evalPacketField`, `PacketField.h`: "tcp.sequenceNo", by dissection and class descriptors), but it cannot write one. So each changed field is a C++ lambda: 9 TCP tests use the typed rewrites of `TcpMutations.h`, and about 40 tests of the other suites have a `mutate` lambda of their own.
- **The library has no aggregate.** About 100 tests keep a static counter and update it in a `filterEvent` lambda (for example the sum of the bytes in flight of `Rfc5681InitialWindow`).
- **The library cannot run an action at a time**, other than to inject a packet. So it cannot call a socket, change a parameter, or stop a node.
- **The TCP state has a descriptor.** `TcpConnection` has a read-only class descriptor with the fields `state`, `fsmState`, the addresses and the queues (`TcpConnection.msg`), and `TcpStateVariables` describes about 300 fields (`TcpConnectionState.msg`). A white-box read can use the same descriptor path as the packet fields.
- **Real ICMP errors need no builder.** `Rfc9293SoftIcmpError` sets the TTL of a segment to 1 in a tap before a router, and the router sends a real Time Exceeded that quotes the segment. A router with a small MTU sends a real Fragmentation Needed.
- **`TcpSocket` already has each call that the tests need:** `connect()` with `fastOpen`, `setTCPAlgorithmClass()`, `setKeepAlive()`, `setNotsentLowat()`, `setAutoRead()` and `read()`, `send()` with EOR, `setCork()`, `close()`, `abort()`. No change in `src/` is necessary (D3, D4).

### The shared networks (I1)

Two generic networks go to `tests/protocol/lib/`, package `inet.protocoltest`. The runner puts the library folder on the NED path of every suite. Each network selects its node and configurator types by parameters (`like INetworkNode`, `like IL3NetworkConfigurator`), so one network serves IPv4 and IPv6. The links use a channel with the datarate and the delay from parameters.

| Network | Shape | TCP use |
| --- | --- | --- |
| `TapPairNetwork` | host1 — tap — host2 | nearly every test |
| `TapRouterNetwork` | host1 — tap — router — host2; the MTU, the datarate and the queue of the router's second interface from the ini | PMTU, ICMP errors, ECN marking, HyStart, Vegas, DCTCP |
| `TapSwitchNetwork` | host1 — tap — switch — host[n] | Fast Open cookie cache, pending Fast Open requests |

The tests of the other suites keep their copies. A later cleanup can move them to these networks; this plan does not.

### The extensions of the protocol test library (E1 to E4)

Each extension gets a description in `AUTHORING.md` and a test in `tests/protocol/self/`. After each extension, every suite runs again, because each suite uses the library.

| Id | Construct | Replaces | Example |
| --- | --- | --- | --- |
| E1 | **A field writer.** `setPacketField()`, the inverse of `evalPacketField()`, and a tap action `.set("<protocol.field>", "<expression>")`. The expression can name `{captures}` and the fields of the frame itself. The action repairs the checksums that the change makes wrong (IPv4 header, and ICMP, UDP and TCP in the computed mode). | the `mutate` lambdas that change one field; `rewriteTcpHeader`, `rewriteIpv4Header` | `.intercept(tap("tap").from("host2").filterPacket("tcp.ackBit").nth(3).set("tcp.window", "100"))` |
| E2 | **Aggregate captures.** A step that matches adds to a capture: `.sum("name", "<expression>")`, `.count("name")`, `.max(...)`, `.min(...)`; and `.capture()` of an expression, not only of a field. An assertion reads the result by `{name}`. | the static counters and the `filterEvent` lambdas that only count | the bytes in flight before the first ACK, the right edge of the offered window |
| E3 | **A module field reader.** `module("host1.tcp", "conn-*")` with `.assertField("state.snd_cwnd >= 14600")`, by the class descriptor of the module and the objects it points to. It is level 3 of the observation order. | the C++ accessors of the connection state | `Tcp.receiveBufferMax:capped`, `TcpCubic.delta:settling` |
| E4 | **An action step.** `.act(when(0.5).describe("...").run(fn))` or `.after(d)`: it runs `fn` at a time, or after the previous step, in the context of a named module. The library adds three common actions: set a parameter (the `@mutable` classes), and start and stop a node by its lifecycle. | the ScenarioManager scripts; the socket calls | `.act(when(1.0).setParameter("host1.tcp", "timestampSupport", "false"))` |

A segment that INET's own TCP never sends (beyond the window, with a forged Fast Open cookie, with a bad ACK) comes from a real segment: a tap rule changes it with E1, or `.duplicate()` sends a copy and a `.copiesOnly()` rule changes the copy with E1. The existing `inject` step with a C++ builder stays for a segment that has no real model. A change that E1 cannot express, such as the contents of a TCP option, stays a typed rewrite of `TcpMutations.h`, which moves from `rfc/` to the suite folder. The RFC tests still find it, because the suite folder is on the include path.

An ICMP error comes from a real router of `TapRouterNetwork`: a tap sets the TTL of a segment to 1, and the router answers with Time Exceeded, a soft error. For `Tcp.icmpSequenceCheck`, the same rule also changes the sequence number of the segment, so that the router quotes a stale number.

### Socket calls (I2)

A TCP socket must belong to a module that the `at` dispatcher of the node connects, because TCP sends its indications there. No INET application gives its socket to a test program. Therefore the library gets a small application, `TcpTestApp` (`like IApp`): it owns the sockets of a test, it calls nothing by itself, and it emits each indication as a signal (established, data arrived, send ready, peer closed, closed, failure). The test program calls the socket with C++ in an E4 action:

```cpp
.act(when(0.1).run("host1.app[0]", [](TcpTestApp& app) {
    app.socket().connect(app.addressOf("host2"), 1000, true);   // with Fast Open
}))
.once(on("host1.app[0]").signal("established").within(1.0))
```

When the same lambda repeats in more than about three tests, it becomes a named action of the library (D3: the amount of repetition decides).

### The rest

| Id | Part | Where |
| --- | --- | --- |
| I3 | the shared ini: GlobalArp, `**.tcp.typename = "Tcp"`, seed, time limit; the scenarios bulk transfer, slow reader (`TcpEchoApp` with `autoRead = false`), interactive request and response | `parameter/ini/` |
| I4 | the completeness check: each parameter of the NED files has at least one catalog row, each row has a test file, and the `%#` lines of each file agree with its row | `parameter/check_catalog.py` |
| I5 | the cross-class check: run each behavior test with the representative value of each other class (`--*.hostN.tcp.<parameter>=<value>` on the command line) and expect FAIL; report each test that still passes | `parameter/cross_class.py` |

## The draft catalog

The draft comes from the NED comments and the validations of the code. The code pass of each group refines it, and adds the preset of each class. Columns: **Ch** is the observation (W wire, A application, S signal, I internal state by E3, E error); **Net** is the network and the means (P `TapPairNetwork`, P6 the same with IPv6 nodes, R `TapRouterNetwork`, F `TapSwitchNetwork`, T a tap rule, X a segment that INET's TCP never sends, made from a real one by E1, K a socket call to `TcpTestApp` by E4, M a run-time change by E4). A "?" marks a class that the code pass must confirm.

### module (6 tests)

| Parameter | Classes: value → what separates the class | Ch | Net |
| --- | --- | --- | --- |
| checksumMode | declared: "declared" → no computed checksum, a `corrupt()` frame is discarded · computed: "computed" → the field equals the RFC 9293 checksum, a wrong field is discarded | W | P, T |
| stopOperationTimeout | in-time: 2s, the connections close before it → the stop ends after the FIN exchange · expires: 0.5s, the peer does not answer → the stop ends at the timeout | W, I | P, T, M |
| stopOperationExtraTime | zero: 0s → the stop ends at the last close · extra: 1s → 1 s later | I | P, M |

### setup — connection setup and header options (36 tests)

| Parameter | Classes | Ch | Net |
| --- | --- | --- | --- |
| initialSendSequenceNumber | clock: -1 → ISS = t × 250000 mod 2^32 · fixed: 1000 → ISS 1000 · random: `intuniform(0, 4294967295)`, two connections → two ISS · invalid: -2 | W, E | P |
| mss | auto-ipv4: -1 → option 1460 · auto-ipv6: -1 → 1440 · explicit: 536 → option 536, segments ≤ 536 · minimum: 88 · invalid: 87 | W, E | P, P6 |
| sendMssOption | on: true → MSS option in SYN and SYN-ACK · off: false → no option, the peer sends ≤ 536 | W | P |
| alignOptions | aligned: true → a NOP before each option · packed: false → no NOP, EOL padding | W | P |
| windowScalingSupport | on: true → WS option, scaled window · off: false → no WS option, window ≤ 65535 | W | P |
| windowScalingFactor | auto: -1 → the smallest shift for advertisedWindow · explicit: 7 → shift 7 · invalid: 15 · too-small: 0 with advertisedWindow 131072 → error | W, E | P |
| timestampSupport | on · off · changed (M at 1 s: the second connection has no TS option) | W | P, M |
| sackSupport | on: SACK-Permitted, SACK blocks after a loss · off · forced-off: true with TcpTahoe → no SACK-Permitted | W | P, T |
| synRetries | active: 2 → 2 SYN retransmissions, then the open fails · passive: 2 → 2 SYN-ACK retransmissions · zero: 0 | W, A | P, T |
| sendDataWithFirstAck | on: data queued in the handshake goes with the third segment · off: a bare ACK first | W | P |
| synLinearTimeouts | doubling: 0 → 1, 2, 4 s · linear: 2 → 1, 1, 1, 2 s | W | P, T |
| syncookiesAlways | off: the server sends segments of the peer MSS (1000) · on: the server quantizes it to 536 | W | P |
| dropSynData | keep: false → the SYN data waits for the handshake · drop: true → the SYN-ACK acknowledges only the SYN, the client sends the data again | W, A | P, K |

### receive — receive window and buffer (24 tests)

| Parameter | Classes | Ch | Net |
| --- | --- | --- | --- |
| advertisedWindow | small: 16384 → window 16384 · large: 262144 → the SYN offers 65535, later 262144 scaled · unscalable: 262144 without window scaling → error | W, E | P |
| receiveBufferSize | none: -1B → offer = advertisedWindow − unread · limiting: 8192B, slow reader → offer ≤ free buffer · out-of-order: 16384B, a lost segment → the out-of-order data reduce the offer | W | P, T |
| windowShrinkAllowed | no: the right edge never moves back · yes: first offer = half the buffer, the edge moves back | W | P |
| windowAutoTuning | off: the offer stays · on: the offer grows in a bulk transfer · no-buffer: on without receiveBufferSize → error | W, E | P |
| receiveBufferLowRtt | low-rtt: RTT below → growth in proportion to the RTT · high-rtt: RTT above → doubling | W, I | P |
| receiveBufferMax | capped: the growth stops at the cap · free: the cap is not reached | W | P |
| windowUpdateAfterRead | off: no ACK at a READ · on: an ACK at a READ that doubles the open window | W | P |
| acceptBeyondBufferAtEmptyQueue | drop: false · accept: true → accepted, window 0 | W, A | X |
| urgentOutOfBand | inline: every byte delivered · oob: the last urgent byte not delivered | A | P, T (sets URG) |
| dropBeyondWindowEdge | rfc: trimmed to the window · drop: dropped whole · exception: starts in the window, empty queue → accepted | W | X |

### ack — acknowledgments (15 tests; the pilot)

| Parameter | Classes | Ch | Net |
| --- | --- | --- | --- |
| delayedAcksEnabled | on: one ACK for two full segments, a lone segment after the timeout · off: an ACK for each segment at once | W | P |
| delayedAckTimeout | value: 100ms → a lone segment acknowledged after 100 ms · zero?: 0s | W | P |
| delayedAckFrameCount | one: 1 · two: 2 · many: 4 · invalid: 0 | W, E | P |
| dsackEnabled | on: a duplicate segment → D-SACK block · off: no D-SACK · follows: not set, sackSupport false → no D-SACK | W | P, T (duplicate) |
| adaptiveDelayedAcks | rfc: false → fixed timeout · quickack, ato, pingpong: the three facets of true | W | P |

### send — sending data (16 tests)

| Parameter | Classes | Ch | Net |
| --- | --- | --- | --- |
| nagleEnabled | on: the second small segment waits for the ACK · off: it goes at once | W | P |
| pushLastBufferedSegment | on: PSH on the last segment of the buffer · off: no PSH | W | P |
| zeroWindowProbe | rfc: one octet of new data · linux: no data, SEQ = SND.UNA − 1 | W | P |
| maxPersistTimeout | caps-growth: 4s → probe intervals 1, 2, 4, 4 s · caps-first: 0.5s → first interval 0.5 s | W | P |
| minshallNagle | off · on: a small segment waits only for an earlier small segment | W | P |
| pushSegmentsOnWriteBoundary | off · write-end: PSH at the end of each write · long-write: a write above half the window → forced push | W | P, K |
| notsentLowat | off: -1B · mark: 4096B → no TCP_I_SEND_MSG while more is unsent · socket: `setNotsentLowat()` takes the place of the parameter | A, I | P, K |

### congestion — congestion control and the algorithm modules (60 tests)

| Parameter | Classes | Ch | Net |
| --- | --- | --- | --- |
| tcpAlgorithmClass | newreno, reno, tahoe, cubic, vegas, westwood, dctcp, nocc, dumb: a loss scenario whose response separates the algorithm (9) · qualified: "inet.transportlayer.tcp.flavours.TcpReno" · unknown: "NoSuchTcp" → error · socket: the open command selects TcpTahoe | W, S, E | P, R, T, K |
| initialWindow | rfc2001 · rfc3390 · rfc6928: the data in flight before the first ACK | W | P |
| initialCwnd | zero: 0 → initialWindow decides · segments: 3 · invalid: -1 | W, E | P |
| increasedIWEnabled | off · on: with "rfc2001" → as "rfc3390" · conflict: with "rfc6928" → error | W, E | P |
| initialSsthresh | high: the default → slow start until a loss · low: 8000 → slow start ends at 8000 · invalid: 0 | S, E | P |
| limitedTransmitEnabled | on: new data at the first and second duplicate ACK · off | W | P, T |
| TcpCubic.beta | default: 0.7 → cwnd × 0.7 at a loss · other: 0.5 · invalid: 1.0 | S, E | P, T |
| TcpCubic.c | value: 0.1 → the growth curve after a loss · invalid: 0 | S, E | P, T |
| TcpCubic.fastConvergence | on: W_max is lowered at a loss below the previous W_max · off | S | P, T |
| TcpCubic.tcpFriendliness | on: never slower than the Reno estimate · off | S | P, T |
| TcpCubic.cntClamp | binds: before the first loss, the increment is limited · invalid: -1 | S, E | P |
| TcpCubic.delta | settling: RTT samples of the first delta of an epoch are ignored · invalid: -1s | S, I, E | P, T |
| TcpCubic.hystartEnabled | off: slow start until a loss · on: slow start ends before the loss | S | R |
| TcpCubic.hystartAckDelta | train: the ACK train exit · no-train: a larger spacing → no train exit | S | R |
| TcpCubic.hystartDelayMin | binds: the threshold = the minimum | S | R |
| TcpCubic.hystartDelayMax | binds: the threshold = the maximum · invalid: below hystartDelayMin | S, E | R |
| TcpCubic.hystartDetect | off: 0 · train: 1 · delay: 2 · both: 3 · invalid: 4 | S, E | R |
| TcpCubic.hystartLowWindow | below: no detection under the window · above: detection | S | R |
| TcpVegas.alpha | grows: fewer extra segments than alpha → cwnd + 1 per RTT · invalid: alpha > beta | S, E | R |
| TcpVegas.beta | shrinks: more extra segments than beta → cwnd − 1 per RTT | S | R |
| TcpVegas.gamma | exit: more than gamma in slow start → slow start ends · invalid: -1 | S, E | R |
| DcTcp.g | value: 1/16 → alpha moves with the gain g at a fixed marking fraction · invalid: 0 | S, E | R (ECN marking) |

### recovery — loss detection and recovery (27 tests)

| Parameter | Classes | Ch | Net |
| --- | --- | --- | --- |
| lossDetectionMode | dupthresh: retransmission at the third duplicate ACK · rack: retransmission after the reordering window with fewer · rack-no-sack: rack with sackSupport false → as dupthresh | W | P, T |
| dupthresh | one: 1 · three: 3 · five: 5 · rack: 5 with RACK → the reordering window is 0 at 5 SACKed segments · invalid: 0 | W, E | P, T |
| tlpEnabled | on: a probe about 2 SRTT after the lost tail · off: the RTO · no-sack: on without SACK → the RTO | W | P, T |
| prrEnabled | on: the PRR pattern in recovery · off: the classic halving · no-sack | W, S | P, T |
| frtoEnabled | off: a spurious RTO keeps the reduced cwnd · on: cwnd restored | S, W | P, T (delay) |
| lossUndoEnabled | off · dsack: undo after a D-SACK · eifel: undo after the timestamps | S | P, T |
| rackReoTimerGranularity | exact: 0s · ticks: 4ms → the REO timeout is rounded up, plus one tick | W | P, T |
| adaptiveReorderingEnabled | off: persistent reordering → spurious fast retransmissions · on: none after the learning | W | P, T |
| maxReordering | caps: 4 → the learned degree stops at 4 · invalid: below dupthresh | S, I, E | P, T |
| retainSackAfterRto | clear: SACKed data is sent again after an RTO · retain: it is not | W | P, T |

### rto — retransmission timeout (12 tests)

| Parameter | Classes | Ch | Net |
| --- | --- | --- | --- |
| initialRto | one: 1s → the first SYN retransmission at 1 s · three: 3s | W | P, T |
| minRexmitTimeout | binds: 1s, short RTT → RTO = 1 s · free: 0.2s, long RTT → RTO above it | S, W | P |
| maxRexmitTimeout | caps: 4s → backoff 1, 2, 4, 4 s · free: the default, not reached | W | P, T |
| maxRexmitCount | limit: 3 → the abort after 3 retransmissions · zero?: 0 | W, A | P, T |
| rtoVarianceFloor | rfc: RTO = SRTT + 4 RTTVAR, then the minimum · linux: SRTT + max(4 RTTVAR, minimum) | S | P |
| seedRttFromHandshake | off: the first RTO = initialRto · on: from the handshake RTT | S, W | P |

### ecn — ECN and AccECN (18 tests)

| Parameter | Classes | Ch | Net |
| --- | --- | --- | --- |
| tcpEcnMode | off · passive (answers, does not request) · rfc3168 (requests; CWR after a CE mark) · accecn · accecn-passive · changed | W | P, T (sets CE), M |
| ecnWillingness | off: tcpEcnMode decides · on: as "rfc3168" · conflict: with "accecn" → error | W, E | P |
| accEcnOptionEnabled | on: the option is sent · off: only the ACE field · changed | W | P, M |
| accEcnOptionPolicy | rfc · linux | W | P, T |
| accEcnBareTimeWaitAck | ace: the ACK of TIME_WAIT has the ACE field · bare: it has none | W | P |
| ecnMarkAll | new-data: ECT only on new data · all: ECT also on pure ACKs and retransmissions | W | P |

### pmtu — path MTU (11 tests)

| Parameter | Classes | Ch | Net |
| --- | --- | --- | --- |
| pmtudEnabled | on: DF set, a Fragmentation Needed reduces the MSS · off: DF clear, no reaction | W | R |
| pmtudTimeout | restore: 5s → the original MSS again after 5 s | W | R |
| mtuProbing | off: 0 · detector: 1 → no search, as the NED says · search: 2 → probes above the MSS, MSS raised · invalid: 3 | W, E | R |
| baseMss | start: 512 → the active opener starts at 512 · invalid: 87 | W, E | R |
| pathMtu | negotiated: 0 → the search ends at the negotiated MSS · route: 1400 → it ends at 1400 | W | R |

### keepalive (7 tests)

| Parameter | Classes | Ch | Net |
| --- | --- | --- | --- |
| keepAliveEnabled | off: no probe · on: a probe after the idle time · socket: `setKeepAlive(true)` with the parameter false | W | P, K |
| keepAliveIdleTime | value: 10s → the first probe after 10 s | W | P |
| keepAliveInterval | value: 2s → probes 2 s apart | W | P, T |
| keepAliveProbeCount | abort: 3 probes without an answer → abort · answered: the connection stays | W, A | P, T |

### fastopen — TCP Fast Open (29 tests)

| Parameter | Classes | Ch | Net |
| --- | --- | --- | --- |
| fastopenServerEnabled | off: no cookie · on: a cookie, and SYN data with a valid cookie goes to the application before the handshake ends · changed | W, A | P, K, M |
| fastopenClientEnabled | off: no cookie request · on: a request, then data on the SYN · changed | W | P, K, M |
| fastopenAcceptWithoutCookie | off: SYN data without a cookie is not acknowledged · on: accepted, no cookie | W | P, K |
| fastopenClientNoCookieRequired | off · on: data on the SYN, no option | W | P, K |
| fastopenLenientCookieValidation | strict: a forged cookie is refused · lenient: accepted | W | X |
| fastopenCookieBytes | min: 4 · max: 16 · invalid: 5 | W, E | P, K |
| fastopenCookieCacheSize | fits: 2 servers, size 2 · evicts: 3 servers, size 2 → one server gets a cookie request again · invalid: 0 | W, E | F, K |
| fastopenKey | random: "" · fixed: a key → the cookie of Linux for this key (a known value from a Linux run, not from INET's own function) · invalid: "xyz" · changed: a new key → the old cookie is refused | W, E | P, K, M |
| fastopenMaxPendingRequests | under: the SYN gets Fast Open · over: the SYN gets none | W | F, K |
| fastopenBlackholeTimeout | off: 0s · on: 10s → no Fast Open for 10 s after two SYN timeouts | W | P, K, T |
| fastopenExpOptionEnabled | off: only kind 34 · on: the server answers kind 254, the client tries kind 254 | W | P, K, X |

### close — connection close, the checks of arriving segments and ICMP errors (13 tests)

| Parameter | Classes | Ch | Net |
| --- | --- | --- | --- |
| msl | value: 5s → TIME_WAIT lasts 10 s | W, S | P |
| abortOnDataAfterClose | rfc: data after the CLOSE is delivered · linux: new data in FIN_WAIT → RST | W, A | P, K |
| rfc5961AckCheck | off: an old ACK counts as a duplicate · on: dropped, an ACK in reply | W | X |
| icmpSequenceCheck | off: a stale ICMP error acts · on: ignored | W | R, T (TTL 1 and a stale sequence number) |
| softIcmpErrorsAbortSetup | rfc: a soft error does not abort SYN_SENT · linux: it aborts | W, A | R, T (TTL 1 on the SYN) |
| abortRstWithAck | rfc: the RST has no ACK · linux: RST with ACK and RCV.NXT | W | P, K |
| tsRecentAfterAckCheck | rfc: TS.Recent from a segment with a bad ACK · linux: not from it | W | X |

### spoof (about 8 tests, D2)

| Module | Classes | Ch | Net |
| --- | --- | --- | --- |
| TcpSpoof | srcAddress: empty or given · seqNo: -1 (ISS) or a value · isSYN: true or false · destAddress, srcPort, destPort, t: the values on the wire | W | P |

**The count:** about 274 tests for the modules of `tcp/`, and 8 for `TcpSpoof`. About 28 of them are error tests. `LinuxTcp` and `LegacyTcp` get no tests of their own (D6); a column of the catalog names the preset that uses each class.

## Verdicts and defects

The rules of the standards tests apply ([derive-tests-from-a-standard.md](../../doc/project/guide/derive-tests-from-a-standard.md#principle-a-claimed-feature-gets-a-test), and `AUTHORING.md` §10):

- A test that fails is a finding. Keep it, and do not make it softer.
- Declare `%# expected-result: FAIL` only for a behavior that the model does not claim, or for a known defect whose repair a limitation blocks; the description names the limitation.
- This pass changes no file in `src/` (D4). The catalog records each defect and each documentation gap. A separate, later plan repairs them.

## The coverage measurement and the gap loop

1. Before the first group: run the TCP coverage recipe of `/home/levy/workspace/tcp-coverage/run.sh` with a third pool, `tcp-parameter` (the folder `parameter`), and record the numbers of the three pools in this plan.
2. After each group: run the `tcp-parameter` pool, and record the line coverage of the files that the group reaches.
3. After the last group: read the missed lines of the TCP sources and sort them into four kinds. (a) A line behind a parameter value is a missing class: add it to the catalog and write its test. (b) A line of a socket command or an API path is not a parameter class; record it for a socket API test class. (c) A line that handles a malformed or unexpected segment belongs to the standards tests. (d) A line that no input can reach is a finding.
4. Repeat step 3 until no line of kind (a) is left.

The estimate before the pilot: the INET TCP tests alone rise from 47 % to more than 80 % of the TCP lines, and all TCP tests together from 73 % to about 85 to 90 %. The 0 % algorithm files alone are about 700 lines, 7 % of the TCP lines. The pilot measures the real gain.

## The steps

Do the work in the worktree `/home/levy/workspace/inet-tcp-parameter-tests` on the local branch `topic/tcp-parameter-tests`, from `master`. Make a commit at the end of each step. Push only when the user asks.

- [x] **Step 0 — worktree.** `git worktree add -b topic/tcp-parameter-tests ../inet-tcp-parameter-tests master`, from `18e41c6d2e`. Pair it with a debug build of `omnetpp-6.x`. The coverage baseline moves to step 5, which needs a coverage build anyway.
- [x] **Step 1 — the catalog.** Write `doc/project/evidence/model/tcp/parameters.md`: one row for each class, with the columns class id, value, preset (`LinuxTcp`, `LegacyTcp`, or none), the claim, source, observation, network, test file, verdict. Each group writes its rows in its code pass, so the draft of this plan is not copied twice; the completeness check lists the parameters without rows until the last group. Done: `1e9f747388`.
- [x] **Step 2 — the library extensions E1 to E4**, one commit each, each with its self test and its section in `AUTHORING.md`. Run every protocol suite after each one. Done: `64c9ab1805` (E1), `d43de56157` (E2), `4f1c8ddced` (E3), `1668c6dbba` (E4).
- [x] **Step 3 — the shared networks (I1) and `TcpTestApp` (I2)** in `tests/protocol/lib`, with a self test for each. Done: `321083cb2e` (networks), `4621ab2d0d` (`TcpTestApp`).
- [x] **Step 4 — the TCP parts.** The shared ini (I3), the two checks (I4, I5), and the move of `TcpMutations.h` to the suite folder. Check that a command-line assignment `--*.host2.tcp.<parameter>=<value>` takes the place of the ini assignment of the test (I5 needs it). Done: `a2feecf0e8` (the move), `1e9f747388` (the ini, the checks, the README). The command line beats the ini line.
- [ ] **Step 5 — the pilot: `ack`, 15 tests.** Code pass, tests, run, cross-class check, coverage. Measure the time of the build and of the run of one test, and of the whole group. Count the lambdas that repeat (D3). Report to the user, and change the design where the pilot shows a problem, before the other groups start. The tests are done (`4ae6534722`). The user decided on 2026-10-29: the coverage is measured only in step 18, after all the tests.
- [x] **Step 6 — `setup`, 34 tests.** Done: `4d280c2293`; 33 PASS, 1 FAIL (PF-4).
- [x] **Step 7 — `send`, 17 tests.** Done: `a82f12fdec`; 17 PASS.
- [x] **Step 8 — `rto`, 12 tests.** Done: `caa51a2dcd`; 12 PASS.
- [x] **Step 9 — `congestion`, 73 tests.** The algorithm classes first, because they give the largest coverage gain. Done: `8876619f7f` (the check skips `-alias`), `e6648b83cc` (the initial window and threshold), `2a19ee2456` (the algorithms), `af106bb2be` (TcpCubic and DcTcp), `9717a2cb35` (HyStart), `d815e75754` (TcpVegas); 72 PASS, 1 FAIL (PF-6).
- [x] **Step 10 — `recovery`, 25 tests.** Done: `d41a5f4b8c`; 25 PASS.
- [x] **Step 11 — `receive`, 25 tests.** Done: `dccaa84b57`; 25 PASS.
- [x] **Step 12 — `close`, 14 tests.** Done: `6e2bf3fc72`; 14 PASS.
- [x] **Step 13 — `ecn`, 19 tests.** Done: `3d9eb039d5`; 19 PASS.
- [x] **Step 14 — `pmtu`, 12 tests.** Done: `a87c9acdd3` (the comment of `TapRouterNetwork`), `e9c1ba160d`; 12 PASS.
- [x] **Step 15 — `keepalive`, 10 tests.** Done: `c5486012b5`; 10 PASS.
- [x] **Step 16 — `fastopen`, 33 tests.** Done: `8f631bf2ab` (one helper hides any option), `ac2c9abf8d`; 33 PASS.
- [x] **Step 17 — `module` and `spoof`, 24 tests.** Done: `1fa921b193` (the action word `stopNode`), `e857617919` (`module`), `f021d0de88` (`spoof`), `0082f7a6b1` (the facet `other-segment`); 20 PASS, 4 FAIL (PF-9, PF-11, PF-13, PF-14).
- [x] **Step 18 — the coverage gap loop** (above), and the final numbers in this plan. Done: `33d547ad38` (the facets of the algorithms), `efbb6cd69a` (the other facets), `d63ddf0ff1` (the coverage section of the catalog and PF-15); 19 facets, all PASS; all pools 86.0 % of the TCP lines.
- [ ] **Step 19 — the documents.** A guide `doc/project/guide/derive-tests-from-parameters.md` that any module can follow; a paragraph in `test-anatomy.md` and a row in `AUTHORING.md` §10; `parameter/README.md`.
- [ ] **Step 20 — close.** Move this plan to `plan/done/`.

Each step from 6 to 17 does the same: the code pass of the group (update the catalog), the tests, a run with `opp_run_protocol_tests -m debug -w tcp -f parameter/<group>`, the cross-class check, the verdicts in the catalog, the coverage of the group.

## The record of the implementation

Facts and decisions that the work found, in the order of the steps.

**Step 0.**
- The debug build of the worktree came from the build of `inet-standards-tests-ipv6tunnel-level2` (the `src/` of `3e51de989f`) by hard links and one fresh time, with the 21 changed sources of the Radiotap commits touched after it; `make` compiled 884 files in about 1 minute (ccache).
- `omnetpp-6.x/bin/opp_msgtool` is a coverage build of 2026-10-27 whose library `liboppnedxml_coverage.so` is gone, so every INET build that runs the message compiler fails. This work does not change the shared tree: the build passes `MSGC=/home/levy/workspace/omnetpp-6.x/out/clang-release/src/nedxml/opp_msgtool` (the release tool of 2026-06-03; no source of `nedxml` changed since), and the runner gets `--no-build`, because this pass changes nothing in `src/`.
- The baseline of all protocol suites without the packetdrill wrappers: 406 PASS, 47 expected FAIL, 306 SKIP (the wrappers, without inet-gpl), 67 unexpected FAIL. The 67 are the IGMP, MLD, Neighbor Discovery and RIP tests whose repairs are in the open pull requests. Each later step compares its run with this list.

**Step 2.**
- The words differ from the first design where the reading is clearer: the aggregate words are `captureSum`, `captureCount`, `captureMax` and `captureMin`, because they are captures; the free builder `anyNumberOfTimes(...)` is new, because an aggregate needs a guard that cannot fail; the action step is `act(action().at(t)...)` or `.after(d)`, with the words `setParameter`, `run`, `run<T>` and `assertModule`.
- A guard of `meanwhile(...)` now runs the captures of the events that it matches; no existing test had a capture in a guard.
- A failed assertion quotes its expression with the values of its captures. This came from the first run of the E2 self test, whose reason did not show why it failed.
- The field writer of E1 must sum a computed TCP checksum over the data of the datagram only, not over the Ethernet padding and FCS behind it, so it takes the length from the IPv4 total length or the IPv6 payload length.
- The start and stop of a node are not words of E4 yet. The group `module` (step 17) adds them, or uses `run` with the lifecycle controller, when it needs them.

**Step 3.**
- The networks extend `WiredNetworkBase`, and they turn off `MacForwardingTableConfigurator`, which crashes on a tap (it looks for the `macTable` of each node). With GlobalArp, a switch learns the addresses from the frames.
- In a channel block, a parameter of the network needs `parent.` before its name.
- A test file must not have the name of a C++ class that it uses: opp_test makes a namespace of the file name, so `TcpTestApp.test` collides with `TcpTestApp`.

## Decisions

The user settled these on 2026-10-29.

- **D1 — the place:** `tests/protocol/tcp/parameter/`, a part of the TCP suite, with no change to the runner, CI or the coverage workflow.
- **D2 — `TcpSpoof`:** in the scope (about 8 tests), although it is in `tcp_common/`.
- **D3 — the socket calls:** no new application with a script language. The test code calls `TcpSocket` directly in C++, where that is simple; the library gets the constructs that make it simple (E4 and `TcpTestApp`). The amount of repeated code decides whether more goes into the library. `TcpSocket` itself needs no change.
- **D4 — the defects:** no change in `src/` in this pass. Failing tests stay. A separate, later plan repairs the defects.
- **D5 — the CI time:** decide after the pilot. If the suite adds more than about 10 minutes, the parameter tests run only in the weekly coverage run.
- **D6 — the presets:** no tests of their own. `LinuxTcp` and `LegacyTcp` only set values, and each value falls into a class of its parameter.
- **D7 — the pilot group:** `ack`, 15 tests. It needs only `TapPairNetwork`, and it has each kind of class: on and off, numeric, invalid, a default expression (`dsackEnabled`) and facets (`adaptiveDelayedAcks`).

## Risks and costs

- **The build time.** The runner builds each test separately. With ccache, a build of a TCP test takes less than one second (the work folder of `Rfc5681InitialWindow`); without ccache it is longer. The pilot measures it.
- **The draft classes.** The draft comes from the NED comments; the code pass will add classes and remove some. The count of 284 can change by about 20 %.
- **The library is shared.** E1 to E4 and the networks change `tests/protocol/lib`, which every suite uses. Each extension adds constructs and changes no existing one, and every suite runs again after each extension.
- **The changed segments.** Some classes need segments that INET's TCP never sends. E1 must repair the checksums after a change; the cross-class check finds a rule that makes the test pass for the wrong reason.
- **The Linux cookie.** `Tcp.fastopenKey:fixed` needs a cookie value from a Linux run or from a packetdrill script of inet-gpl, not from INET's own function; else the test only checks INET against itself.

**Steps 1, 4 and 5 (the pilot).**
- The 13 tests of `ack` pass, and the cross-class check passes: each test fails with the value of each other class of its parameter (12 runs, one for each other value). Every other protocol test keeps its verdict.
- The code pass changed the draft: 11 classes and 13 tests, not 15. `dsackEnabled:follows` is gone, because a default expression is not a class of values: its value falls into `on` or `off`. `delayedAckFrameCount` has `one`, `many` (2 and more, with the default 2) and `invalid`, because 2 and 4 take the same rule. `delayedAckTimeout:zero` stays, because its behavior differs on the wire (the ACK goes at once).
- Two findings went into the catalog: PF-1, `TcpSessionApp` closes at once after its last send when `tClose = -1`, against its NED (the shared scenario `_bulk.ini` uses 1000 s); PF-2, `delayedAckTimeout` has no check of a negative value.
- The filters can tell data from a pure ACK with `pk.dataLength > tcp.headerLength`: `PacketFilter` compares bits with bytes. The expression name `length` of the library is the same data length, without the popped headers, so it is right at a receiver too.
- In an ini file the first line that matches a key wins, and an include is no error for a key that the test also sets; so a test writes its own lines first and includes the shared files last.
- The time: a test builds in about 1 s, also without ccache, and runs in about 0.4 s; the 13 tests take 3 s of wall time with 32 cores. About 280 tests take a few minutes on a CI runner with 4 cores, below the limit of D5.
- D3: the group needs no socket call. One lambda (`hasDsackBlock()` of `TcpMutations.h`) is in two tests; nothing repeats enough for a new word.
- The coverage of the pilot is not measured. `omnetpp-6.x` has no coverage libraries since 2026-10-27, and a coverage build of them changes the shared tree, which also holds the broken `opp_msgtool` of step 0. The user decides how to measure.

**Step 6 (`setup`).**
- The user decided on 2026-10-29 to measure the coverage only after all the tests (step 18), and to go on with the groups.
- The code pass changed the draft from 36 to 34 tests. `mss:minimum` is gone: 88 takes the path of any explicit value. `sackSupport:forced-off` is gone: `true` with an algorithm without SACK recovery is the class `off` on the wire, so no test of `sackSupport` can separate them; the tests of `tcpAlgorithmClass` (step 9) check it. `synRetries` has the facets `active` and `passive` of one class and the class `zero`, so that each test has another value to fail with.
- PF-4 is a defect of `windowScalingFactor = -1`: its test fails, as a plain FAIL, because a later plan repairs it. The boundaries show the cause: 131071 gets the factor 1, 262141 to 262144 get 2, 262145 gets 3.
- `dropSynData`: the INET client sends the data of its SYN again with the ACK of the SYN-ACK, and with `keep` the server ignores that copy without an ACK at once; so the two classes look the same on the wire. The tests remove the data from that ACK in the tap (`removeTcpData()`): then only the queue of the server can hold the data.
- The NED function `intuniform()` takes 32-bit integers: `intuniform(0, 4294967295)` gives an error.
- The group has no new library word. Two lambdas repeat: the check of an option of the SYN (`tcpOptionCount()`, `tcpMssOptionOf()`, `tcpWindowScaleOptionOf()` in 18 tests) and the D-SACK check; both are one-line calls of `TcpMutations.h`.

**Step 7 (`send`).**
- The code pass changed the draft from 16 to 17 tests: `maxPersistTimeout` has an `invalid` class, because the code checks "more than 0"; `pushSegmentsOnWriteBoundary` has the facets `write-end` and `long-write` of `true`.
- A socket option that takes the place of a parameter stays a class, as the scope says, but its test passes with each value of the parameter by design. Such a class has a name that ends with `-override`, and the cross-class check skips it (`Tcp.notsentLowat:socket-override`).
- The zero-window tests need a receiver that never reads: a `TcpTestApp` that listens with `setAutoRead(false)`; the forked connection inherits it.
- A step with `notBefore` skips the events before its time, so a test of an interval uses a `never` step for the early part and a `once` step for the window; the generator calls this a gap. With `notBefore` alone, probes 0.5 s apart passed a test of 1, 2 and 4 s.
- A time in an error message prints without its unit ("but it is 0").
- PF-5: TCP_NODELAY of a socket takes the place of `nagleEnabled`, and the NED does not say so.

**Step 8 (`rto`).**
- 12 tests, as the draft said; all pass at the first run, and the cross-class check confirms each one.
- The classes `binds` and `free` of `minRexmitTimeout` and `caps` and `free` of `maxRexmitTimeout` are facets of the path, not of the code: a limit binds or it does not. The tests fix the path (a round trip of 100 ms) and pick a value on each side.
- The signal `rto` comes only with a measurement of the round trip, not at the open; so the tests read its first value after the first ACK of data.
- The tests of the timer of data turn `tlpEnabled` off: a tail loss probe is a copy of the segment on the wire, but not a retransmission of the timer.

**Step 9 (`congestion`).**
- The code pass changed the draft from 60 to 73 tests. Each parameter whose value the code checks got an `invalid` class (14 error tests). The default of a standard or of Linux is a class with the name of its source (`rfc9438`, `rfc8257`, `linux`); another value is `other`. The qualified NED type name of `tcpAlgorithmClass` is the class `qualified-alias`: it passes with the value that it spells, so the cross-class check skips a class that ends with `-alias`, and uses no `-alias` or `-override` class as the alternative of another class.
- The tests of the algorithms turn SACK off on the sender: with SACK, the recovery of RFC 6675 makes TcpNewReno, TcpReno and DcTcp the same.
- PF-6: TcpWestwood takes no RTT sample, so it never computes the threshold from its estimate, and its window wraps to 0 after a fast retransmit. `TcpAlgorithmClassWestwood` fails, as a plain FAIL, because a later plan repairs it.
- TcpVegas: with the delayed ACK of the receiver, Vegas leaves slow start at 4 segments without a queue, because the round trips include the delay of the ACK; the tests turn the delayed ACK off. The code computes the difference of the expected and the actual rate in unsigned arithmetic, so the difference wraps when the actual rate is higher. No test shows an effect of this; the plan of the repairs looks at it.
- The exit of HyStart sets ssthresh but emits no signal, so the tests read `state.hystart_found` and `state.ssthresh` with `assertModule`. No new network was necessary: the router of `TapRouterNetwork` runs a TCP sink (`hasTcp = true`), and each test of `hystartDetect` opens a connection to host2 through the bottleneck (only the delay detector acts, because the delayed ACKs come 2.4 ms apart) and then one to the router (only the ACK train acts: a router window of 260000 bytes, a little more than the path holds, keeps the queue below the threshold of 4 ms). At each time, the newest connection is the one under test.
- The tests of `hystartDelayMin` and `hystartDelayMax` change the delay of the four links at known times, as `TcpCubicDeltaLong` does; a receiver window of 65535 bytes keeps the sender below the rate of the path, so there is no queue and no ACK train. Probes confirmed the clamps: with `hystartDelayMin = 0`, a step of 3 ms at a round trip of 16 ms gives the exit, and with 4 ms it does not.
- The exit of HyStart does not move up monotonically with `hystartLowWindow` below the natural exit point: 100 gave an exit at 117 segments, 16 at 145, because the rounds of HyStart start at other times. The test of `other` uses 400.
- D3: the group needs one socket call (`socket-override`). One file-scope helper repeats: the delay setter of the links is in 4 tests and inline in `TcpCubicDeltaLong`. The generator of the tests got a slot for file-scope helpers. If more groups need the helper, it goes into the library.
- The group runs in 21 s of wall time with 32 cores; the tests with two connections take 18 s each. The regression run has 69 unexpected FAILs: the 67 of the baseline, PF-4 and PF-6.

**Step 10 (`recovery`).**
- The code pass changed the draft from 27 to 25 tests. The classes without SACK (`rack-no-sack`, the `no-sack` classes of `tlpEnabled` and `prrEnabled`) are gone: each parameter of the group needs SACK, and without it the class is the same as `off`, so no test can separate them (the rule of `sackSupport:forced-off` in step 6). `dupthresh` has the classes `rfc5681` (3), `other` (5) and `invalid`, and each value has a facet for each loss detection, because the code reads the value on two paths. `lossUndoEnabled = true` has the facets `dsack` and `eifel`; each test turns the other proof off (no timestamps, or no D-SACK at host2). `rackReoTimerGranularity` has no `invalid` class: the code takes a value below 0 as 0, and the NED names no such value.
- At a tap, `pk.dataLength` is the length of the frame, so `pk.dataLength > tcp.headerLength` does not tell data from a pure ACK: a rule for the first data segment dropped the last ACK of the handshake, which has the same sequence number. Tap rules compare `ipv4.totalLengthField > 100B`, as the earlier groups do; at the TCP module, `pk.dataLength` works.
- RFC 6675 counts an ACK with new SACK information as a duplicate ACK, also when it moves the cumulative ACK. So the first ACK at a hole is a duplicate when the ACK of the segment before the hole was delayed. The tests of `dupthresh` count the ACKs that carry a SACK option (a header longer than 32 bytes).
- With DupThresh 5 and RACK, more than 5 segments must follow the hole in its flight; else the timer of the reordering window acts first. The tests drop the 100th segment.
- The probe timeout before the first RTT sample is the RTO, 1 s. After the ACK of new data, it is 2 * SRTT + 200 ms with one segment in flight, as RFC 8985 says.
- Without PRR, the window does not change during the recovery; the first value after it is above the threshold (13908 against 13756). The test of `off` checks that the window does not change in 30 ms after the threshold.
- The threshold of a timeout is 2 * `snd_mss` = 2920 bytes, also with timestamps, where the segments carry 1448 bytes; RFC 5681 excludes the options from SMSS. No claim of a parameter depends on it; the plan of the repairs looks at it.
- The test of `retainSackAfterRto` needs a peer that reports no SACK blocks after the timeout, because the next ACK marks the data again in both classes. The new helper `hideSackBlocks()` of `TcpMutations.h` replaces the SACK option with NOP options of the same length.
- The group runs in 3 s of wall time with 32 cores. D3: no socket call, and no new library word.

**Step 11 (`receive`).**
- The code pass changed the draft from 24 to 25 tests. `advertisedWindow` got the error class `too-large`, above 65535 << 14 bytes; the NED does not name that limit (PF-7, a documentation gap). `receiveBufferSize` has the classes `none`, `limiting` and `ample` (a buffer above the advertised window, the value of LinuxTcp): the draft class `out-of-order` is gone, because the right edge of the window never moves back, so the out-of-order data never make the offer smaller on the wire. `windowShrinkAllowed = true` has the facets `first` (half the buffer in the SYN-ACK) and `shrink`. The class `exception` of `dropBeyondWindowEdge` is gone: there the result is the same as with false.
- A tap rule that writes 65535 into the window field of host2 makes host1 send beyond the offer. With an offer of 16384 bytes and no reads, the highest ACK of host2 counts the segments that got in: 11 (the buffer is the window), 5 (a buffer of 8192 bytes), 12 (a larger buffer takes the segment that crosses the edge), and 11 again with `dropBeyondWindowEdge`. One scenario separates the classes of two parameters.
- The window of a receiver that does not read does not reach 0: the rule "do not shrink the window" keeps the last 375 bytes open, and the sender does not send so few bytes. The tests wait for a window below one segment.
- `windowUpdateAfterRead` acts only at an explicit READ, so its tests use `TcpTestApp` without auto-read; a delivery with auto-read is no such read.
- `receiveBufferLowRtt` acts only through Dynamic Right-Sizing, once in each receive RTT (40 ms here). Every value below the RTT gives the same growth; above it, the growth is in proportion to the RTT over the value (100 ms gives about 900000 bytes at 0.7 s, 1 s about 440000, below the RTT 1330000). Later, the growth for the overflow of the buffer hides the difference, so the tests read the buffer at 0.7 s.
- The group runs in 4 s of wall time with 32 cores. D3: the socket calls are the listen without auto-read and the read of `TcpTestApp`; no new library word.

**Step 12 (`close`).**
- The code pass changed the draft from 13 to 14 tests: `msl` has the classes `rfc` (120 s) and `other` (5 s; LinuxTcp sets 30 s, which the completeness check found when the first rows forgot it). The code does not check `msl`, and the NED names no invalid value, so it has no class `invalid`.
- The end of TIME_WAIT shows in the signal `state` (CLOSED is 1). The signal `tcpConnectionRemoved` does not come at that time.
- The field writer of a tap rule reaches only the fields of a header, not those of an option. The new helper `addToTsval()` of `TcpMutations.h` changes the TSval of a segment.
- `tsRecentAfterAckCheck`: with the rule of RFC 7323, a forged ACK of unsent data with a TSval far ahead moves TS.Recent, and PAWS then drops each later ACK of the peer, so the sender times out. The check of Linux prevents it. The test observes that timeout, not TS.Recent.
- `icmpSequenceCheck`: the tap sets the TTL of the first SYN to 1 and its sequence number 100 higher, so the Time Exceeded of the router quotes a stale sequence number. The tests turn `softIcmpErrorsAbortSetup` on, because only an error that aborts the setup shows whether it counted.
- The group runs in 2 s of wall time with 32 cores. D3: the socket calls are connect, close, abort, listen and send of `TcpTestApp`; no new library word.

**Step 13 (`ecn`).**
- The code pass changed the draft from 18 to 19 tests: `accEcnOptionPolicy = "linux"` has the facets `kind` (the option of kind 174 on the SYN-ACK) and `duplicate` (a duplicate segment that ends at RCV.NXT counts in the AccECN counters), because the NED claims both.
- The five values of `tcpEcnMode` differ only in the pair of the SYN that a host sends and the SYN-ACK that it sends to an AccECN SYN. Each test therefore lets host1 open a connection and take one from host2, which requests AccECN; every value gives another pair, so one test structure separates each value from the four others.
- The classes `changed` of the two `@mutable` parameters set the new value with `setParameter` at 0.5 s. A second application of host1 opens the second connection; the first connection shows the old value on its data after the change. The value of a string parameter needs its quotes: `"\"off\""`.
- The first data segment of an AccECN client carries the AccECN option, so it holds 1436 bytes, not 1448.
- A full close makes the connection a time-wait socket already in FIN_WAIT_2, so the ACK of the FIN of the peer is the ACK of TIME_WAIT that `accEcnBareTimeWaitAck` changes.
- The group runs in 2 s of wall time with 32 cores. D3: no socket call, and no new library word.

**Step 14 (`pmtu`).**
- The code pass changed the draft from 11 to 12 tests. `pmtudTimeout` has the classes `rfc1191` (600 s; the run lasts 602 s of simulated time, and the idle time costs nothing) and `other`. `mtuProbing` has the classes `off`, `search` and `invalid`: the code turns the search on only for 2, so 1 acts as 0 and falls into `off`, as the NED says. `baseMss` got the class `linux` (1024) beside `other` and `invalid`.
- The comment of `TapRouterNetwork` named the key `*.router.eth[1].mtu`, which the Ethernet interface does not have; the MTU is a parameter of the MAC, `*.router.eth[1].mac.mtu` (corrected in `a87c9acdd3`).
- The restore of `pmtudTimeout` comes at the first segment after the timeout, but the burst of that segment keeps the segment size that the sender computed before; the next burst, after the delayed ACK of the peer, has the MSS of the handshake. The tests look for 0.5 s.
- The search of RFC 4821 stops when its range is narrower than 8 bytes: on a path of 1500 bytes its last probe has an MTU of 1493, so no segment reaches 1448 bytes of data. The tests compare with 1400 bytes of IP.
- The tests measure the frames that leave host1 at its MAC (`host1.eth[0].mac`, `packetSentToLower`) by `ipv4.totalLengthField`, which needs no arithmetic on the lengths of the headers.
- The group runs in 3 s of wall time with 32 cores. D3: no socket call, and no new library word.

**Step 15 (`keepalive`).**
- The code pass changed the draft from 7 to 10 tests. The idle time and the interval each have the class of the default (`rfc` 7200 s, `linux` 75 s; the runs last up to 7202 s of simulated time, and the idle time costs nothing) and `other`. `keepAliveProbeCount` has the classes `linux` (9), `other` (3) and `zero` (0: the abort at the idle time, without a probe); the draft class `answered` is gone, because it is a scenario, not a value. The socket class is `socket-override`, so the cross-class check skips it.
- PF-8: a negative idle time or interval has no check and reaches the scheduler ("Cannot schedule message KEEPALIVE to the past"), as PF-2 for the delayed ACK. The NED names no such value, so no class tests it.
- `_bulk.ini` closes the connection at 1000 s; the test of two hours sets a later close.
- The abort shows in the signal `state` (CLOSED is 1); a signal step can assert the captures with `assertModule()` of the module that emits it.
- The group runs in 1 s of wall time with 32 cores. D3: one socket call (`setKeepAlive()`), no new library word.

**Step 16 (`fastopen`).**
- The code pass changed the draft from 29 to 33 tests. Each parameter that is `@mutable` and acts at the open got the class `changed` (rule 3), also `fastopenAcceptWithoutCookie` and `fastopenClientNoCookieRequired`. `fastopenExpOptionEnabled` has none: it acts only at the SYN-ACK of a cookie request without an answer and at the next open, so no later step can see an old value in an open connection. It has the facets `on-client` and `on-server` instead. `fastopenCookieBytes` has `linux` (8) and `long` (16: a second MAC), `fastopenCookieCacheSize` `ample`, `evicts` and `invalid`, `fastopenMaxPendingRequests` `ample`, `limit` and `zero`, `fastopenBlackholeTimeout` `hour`, `other` and `off`.
- The cookie of `fastopenKey:fixed`: the packetdrill script fastopen/server/simple1.pkt sets its own addresses (`--local_ip=192.168.0.2`, `--remote_ip=10.248.0.2`); with the packetdrill defaults the cookie differs. With the addresses of the script, INET gives the cookie of the Linux run, 00abaaabe8e954c2. `TapRouterNetwork` takes the addresses from an XML configuration of the configurator.
- A test that needs a program and ends with an error waits for the event that the error prevents; else the program ends before the error.
- The helper `hideSackBlocks()` of step 10 is now `hideTcpOption(frame, kind)`, one name for any option (`8f631bf2ab`). New helpers: `tcpFastOpenCookieOf()` and `forgeFastOpenCookie()`.
- With a cookie in the cache, the SYN of a connect() with Fast Open waits for the first SEND; a test that wants the SYN at once sends data.
- The group runs in 3 s of wall time with 32 cores. D3: the socket calls are connect with Fast Open and send of `TcpTestApp`, one application for each connection; no new library word.

**Step 17 (`module` and `spoof`).**
- The code pass changed the draft from 6 + about 8 to 10 + 14 tests. `checksumMode` has `declared`, `computed`, the facet `computed-ipv6` (`Tcp::initialize()` puts the hook into the IPv6 module with its own call) and `invalid` (the `@enum`). The receiver checks the mode that the header carries, so the parameter of the sender decides whether a receiver can find a wrong checksum. The tap writes a wrong value with `corruptTcpChecksum()`: the `corrupt()` of a tap rule gives a bit error, which the MAC discards before TCP sees it.
- The draft of the stop was wrong. `Tcp::handleStopOperation()` deletes all connections at once, without a segment, and waits for nothing (PF-10). The stage of TCP ends at the end of the extra time or at the timeout, whichever comes first. So the timeout has the classes `longer` and `shorter`, both with an extra time of 1 s, and the extra time has `zero`, `other`, the facet `other-segment`, and `negative-alias`. A negative value is a class by rule 2, because `startActiveOperationExtraTimeOrFinish()` has a branch for it: it is another spelling of "no extra time", and it stops the run (PF-9).
- E4 got the word `stopNode(path)` (`1fa921b193`), with the self test `StopNode`. `ProtocolTester` is also the `LifecycleController`, because `moduleOperationStageCompleted()` casts the controller to `cComponent`. The tests see the end of the stage of TCP in the echo replies of host1, because the network layer goes down after the stage of TCP. The signal `pingRxSeq` carries the number of the reply, so a step names the last reply exactly. An application of host1 can delay the stage of the applications (`TcpSinkApp` waits for its connections), so the stop tests give host1 no application.
- `Tcp` does not override `handleMessageWhenDown()`, so a segment that reaches TCP in the extra time stops the run (PF-14). A frame that reaches a node whose network layer is down causes no error.
- `TcpSpoof` is `like ITcp`, so a test sets it as the type of the TCP of a node. A source address that the node does not have stops the run in `Ipv4::encapsulate()`, because `TcpSpoof` does not set `nonLocalSrcAddress`. So `srcAddress` has `empty`, `local` and `invalid`, and its tests run the spoof in the router of `TapRouterNetwork`, the only node with two interfaces. `destAddress` has `host`, with the facet `host-ipv6` (the same value on IPv6 hosts, as `mss:auto-ipv6`), and `empty`: two values of different address families in one network need a dual-stack setup.
- Three defects of `TcpSpoof`: no checksum mode (PF-11), the empty default of `destAddress` (PF-12), and the ISS of `seqNo = -1` (PF-13). Because of PF-11, the tap drops the segment after the wire step in each test except `true-answer`. Without the drop, a cross-class run that waits for a segment ends with the error of host2 and gives no verdict.
- The cross-class check: in `module`, 7 runs, and each gives FAIL as it must; the facet `other-segment` fails with `0s` too, by hand. In `spoof`, 5 tests fail with the value of the other class, and two runs give no verdict, because PF-11 or PF-13 stops the run of the other value. opp_test writes no `run` script for a test that fails, so the check reports `NOT BUILT` for it; such a test runs by hand with the `run` script of a sibling test.
- Each of the 116 parameters of the catalog has rows now: 311 rows and 311 tests. The regression run has 73 unexpected FAILs: the 67 of the baseline, PF-4, PF-6, PF-9, PF-11, PF-13 and PF-14. Each group runs in about 2 s of wall time with 32 cores.

**Step 18 (the coverage gap loop).**
- The coverage tools of 2026-10-27 were gone: the INET copy of `tcp-coverage/`, the runner folder of inet-gpl, and the wrappers of opp_repl. A coverage build in `omnetpp-6.x` writes coverage tools into its `bin/`, which caused the broken `opp_msgtool` of step 0. So the measurement used a private copy of `omnetpp-6.x`, the coverage objects of the worktree (`out/clang-coverage`), and a detached worktree of inet-gpl master, all in `/var/tmp/claude-tcpparam/cov/`, with the scripts `build.sh`, `build-gpl.sh`, `runs.sh`, `gaps.py` and `kinds.py` there (not committed). The builds took 3 minutes (ccache), and the three pools 4 minutes.
- The line coverage of the TCP sources (9458 lines) before the loop: `tcp-inet` 47.2 %, `tcp-parameter` 74.1 %, both 75.0 %, `tcp-packetdrill` 67.6 %, all 83.4 % (1574 lines in the gap list). After it: `tcp-parameter` 76.9 %, both 77.9 %, all 86.0 % (1325 lines). The estimate of the plan holds for all pools (85 to 90 %), but not for the INET tests (above 80 %): the socket API and the segments of the peer are no parameter classes.
- Kind (a) has two conditions: a parameter value decides the line, and its behavior separates the class, as the cross-class check asks. A line that also needs a rare state that no parameter sets, such as an ATO above the RTO, is kind (c). The loop added 19 facets: 4 of DcTcp, one each of TcpVegas, TcpNoCongestionControl, DumbTcp and `initialWindow` "rfc2001", and 11 of other classes. Three blocks behind a value have no facet: the timeout of TcpTahoe and the ACK of DumbTcp for an out-of-order segment do what other classes do, and TcpWestwood waits for PF-6.
- A test must reach the lines of its claim. `TcpWindowAutoTuningOnClamp` passed first through the growth for out-of-order data, and the coverage of the single test showed it; its receiver now reads nothing. `TcpSoftIcmpErrorsAbortSetupLinuxIpv6` passed first through the "no route" error of the Neighbour Discovery of host1.
- IPv6 with a router needs `Ipv6NetworkConfigurator` and `**.ipv6.configurator.networkConfiguratorModule = "configurator"`; the flat configurator gives the hosts no route through the router, and the network configurator without the node configurators gives host2 only its link-local address.
- The cross-class check found 3 problems: DumbTcp also acknowledges each segment at once (`dctcp-receiver` now needs the delayed ACK of unmarked data), DumbTcp signals no window (`nocc-rto` now checks the timing of RFC 6298), and "accecn" falls back to RFC 3168 with a classic peer (`rfc3168-cwr` now has an "accecn-passive" peer). After the fixes: 72 runs and 0 problems.
- DcTcp negotiates ECN only with `tcpEcnMode` "rfc3168" or "accecn"; without it, it sends an ECN-setup SYN and nothing more. The tests of `DcTcp.g` end at the first alpha, so no test reached the reduction of DcTcp before. TcpVegas computes its window with `snd_mss` (1460), and the window before its reduction is the last value of the signal, not the largest.
- The 1325 lines after the loop: 46 wait for PF-6, 245 belong to a test class of the socket API, 760 belong to the standards tests, and 274 are model errors, strings of the inspectors, and dead code (PF-15). The catalog has the table.
- The regression run has 1164 tests and 73 unexpected FAILs, as before.
