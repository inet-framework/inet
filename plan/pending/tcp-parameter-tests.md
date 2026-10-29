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
| `LinuxTcp`, `LegacyTcp` | `tcp/` | 0 new, 39 and 17 default overrides | no test of their own (D6): each value that they set is in a class of its parameter, and the catalog names the preset of each class |
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

- [ ] **Step 0 — worktree.** `git worktree add -b topic/tcp-parameter-tests ../inet-tcp-parameter-tests master`. Pair it with a debug build of `omnetpp-6.x`. Measure the TCP coverage baseline (gap loop, step 1).
- [ ] **Step 1 — the catalog.** Write `doc/project/evidence/model/tcp/parameters.md` from the draft catalog: one row for each class, with the columns parameter, class, value, preset (`LinuxTcp`, `LegacyTcp`, or none), source, observation, network, test file, verdict. Mark the draft rows "draft" until the code pass of their group.
- [ ] **Step 2 — the library extensions E1 to E4**, one commit each, each with its self test and its section in `AUTHORING.md`. Run every protocol suite after each one.
- [ ] **Step 3 — the shared networks (I1) and `TcpTestApp` (I2)** in `tests/protocol/lib`, with a self test for each.
- [ ] **Step 4 — the TCP parts.** The shared ini (I3), the two checks (I4, I5), and the move of `TcpMutations.h` to the suite folder. Check that a command-line assignment `--*.host2.tcp.<parameter>=<value>` takes the place of the ini assignment of the test (I5 needs it).
- [ ] **Step 5 — the pilot: `ack`, 15 tests.** Code pass, tests, run, cross-class check, coverage. Measure the time of the build and of the run of one test, and of the whole group. Count the lambdas that repeat (D3). Report to the user, and change the design where the pilot shows a problem, before the other groups start.
- [ ] **Step 6 — `setup`, 36 tests.**
- [ ] **Step 7 — `send`, 16 tests.**
- [ ] **Step 8 — `rto`, 12 tests.**
- [ ] **Step 9 — `congestion`, 60 tests.** The algorithm classes first, because they give the largest coverage gain.
- [ ] **Step 10 — `recovery`, 27 tests.**
- [ ] **Step 11 — `receive`, 24 tests.**
- [ ] **Step 12 — `close`, 13 tests.**
- [ ] **Step 13 — `ecn`, 18 tests.**
- [ ] **Step 14 — `pmtu`, 11 tests.**
- [ ] **Step 15 — `keepalive`, 7 tests.**
- [ ] **Step 16 — `fastopen`, 29 tests.**
- [ ] **Step 17 — `module` and `spoof`, 14 tests.**
- [ ] **Step 18 — the coverage gap loop** (above), and the final numbers in this plan.
- [ ] **Step 19 — the documents.** A guide `doc/project/guide/derive-tests-from-parameters.md` that any module can follow; a paragraph in `test-anatomy.md` and a row in `AUTHORING.md` §10; `parameter/README.md`.
- [ ] **Step 20 — close.** Move this plan to `plan/done/`.

Each step from 6 to 17 does the same: the code pass of the group (update the catalog), the tests, a run with `opp_run_protocol_tests -m debug -w tcp -f parameter/<group>`, the cross-class check, the verdicts in the catalog, the coverage of the group.

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
