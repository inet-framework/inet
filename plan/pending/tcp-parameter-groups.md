# Group the parameters of Tcp, and make the algorithm of a connection its submodule

Status: **done** — the grouping (step 2) and the algorithm as a submodule of its connection
(steps 3 to 9) landed on master on 2026-10-15; the owner approved both.

- Branch `topic/tcp-parameter-groups`, rebased on master `9b4b22258f` for the landing.
- The decision numbers continue those of `plan/done/tcp-receive-memory-repairs.md` (D-117 onward).

## 1. The problem

`Tcp` has 102 parameters in one flat list, in the order in which they were added. A reader cannot
see which parameters belong to which part of the protocol, or which ones model Linux and which
ones an RFC. Some parameters belong to one algorithm only: the 12 parameters of CUBIC and its
slow start (HyStart) are in the list also when the connection runs NewReno.

## 2. The inventory

- `configureStateVariables()` (`TcpConnectionUtil.cc`) reads 60 of the 102 parameters into the
  state of each connection; `Tcp.cc` reads the host-wide ones (the Fast Open key and cookie cache,
  the AccECN policy, the checksum mode, the lifecycle timeouts).
- Only a few groups are read by one class: CUBIC and HyStart (12) by `TcpCubic`, `dctcpGamma` by
  `DcTcp`, keepalive (4) by `TcpAlgorithmBase`.
- Use in INET (ini and test files): `mss` 140, `advertisedWindow` 99, `tcpAlgorithmClass` 95,
  `sackSupport` 71, `delayedAcksEnabled` 64; the CUBIC, HyStart and DCTCP parameters at most once.
  inet-gpl sets 30 parameters through `tcp.ini`, `sysctls.yaml` and `tcp.py`.

## 3. The groups (step 2)

The parameters come in 15 groups, one for each part of the protocol. In a group, the parameters
of the RFCs come first. The behaviors of Linux that no RFC defines follow under `// Linux
behaviors (no RFC)`; `LinuxTcp` sets most of them. An RFC behavior that `Tcp` leaves off (F-RTO,
the undo, the checks of RFC 5961) stays with the RFC parameters.

| Group | RFC | Linux (no RFC) |
| --- | --- | --- |
| the module | `checksumMode`, `stopOperationTimeout`, `stopOperationExtraTime` | — |
| connection setup and header options | `initialSendSequenceNumber`, `mss`, `sendMssOption`, `alignOptions`, `windowScalingSupport`, `windowScalingFactor`, `timestampSupport`, `sackSupport`, `synRetries`, `sendDataWithFirstAck` | `synLinearTimeouts`, `syncookiesAlways` |
| receive window and buffer | `advertisedWindow`, `receiveBufferSize`, `windowShrinkAllowed` | `windowAutoTuning`, `windowUpdateAfterRead`, `acceptBeyondBufferAtEmptyQueue`, `dropBeyondWindowEdge` |
| acknowledgments | `delayedAcksEnabled`, `delayedAckTimeout`, `delayedAckFrameCount`, `dsackEnabled` | `adaptiveDelayedAcks` |
| sending data | `nagleEnabled`, `pushLastBufferedSegment`, `zeroWindowProbe`, `maxPersistTimeout` | `minshallNagle`, `pushSegmentsOnWriteBoundary`, `notsentLowat` |
| congestion control | `tcpAlgorithmClass`, `initialWindow`, `initialCwnd`, `increasedIWEnabled`, `initialSsthresh`, `limitedTransmitEnabled` | — |
| CUBIC | `cubicBeta`, `cubicC`, `cubicFastConvergence`, `cubicTcpFriendliness` | `cubicCntClamp`, `cubicDelta`, `hystartEnabled` and the 5 HyStart values |
| DCTCP | `dctcpGamma` | — |
| loss detection and recovery | `lossDetectionMode`, `dupthresh`, `tlpEnabled`, `prrEnabled`, `frtoEnabled`, `lossUndoEnabled` | `rackReoTimerGranularity`, `adaptiveReorderingEnabled`, `maxReordering`, `retainSackAfterRto` |
| retransmission timeout | `initialRto`, `minRexmitTimeout`, `maxRexmitTimeout`, `maxRexmitCount` | `rtoVarianceFloor`, `seedRttFromHandshake` |
| ECN and AccECN | `tcpEcnMode`, `ecnWillingness`, `accEcnOptionEnabled` | `accEcnOptionPolicy`, `accEcnBareTimeWaitAck`, `ecnMarkAll` |
| path MTU | `pmtudEnabled`, `pmtudTimeout`, `mtuProbing`, `baseMss`, `pathMtu` | — |
| keepalive | the 4 `keepAlive*` parameters | — |
| TCP Fast Open | 9 `fastopen*` parameters | `fastopenBlackholeTimeout`, `fastopenExpOptionEnabled` |
| connection close and the checks | `msl`, `abortOnDataAfterClose`, `rfc5961AckCheck`, `icmpSequenceCheck`, `softIcmpErrorsAbortSetup` | `abortRstWithAck`, `tsRecentAfterAckCheck` |

The step moves lines and adds headings only: the 102 parameters keep their names, types,
defaults and properties (checked by `opp_nedtool c -x` of the old and the new file).

## 4. The algorithm as a submodule of its connection

### Today

- Each connection is a submodule of `Tcp`: `Tcp::createConnection()` creates `tcp.conn-N` at run
  time from the NED type `TcpConnection`, and a listener forks a new one for each accepted
  connection. `TcpConnection.ned` has no parameters, only signals and statistics.
- The algorithm (`tcpAlgorithmClass`, the default of `Tcp` or the choice of the socket) is a
  `cObject` that the connection creates by its class name. It has no parameters of its own: the
  connection and the algorithm read all 102 parameters from `Tcp` (94 calls of
  `tcpMain->par()`).
- The algorithm creates the state of the connection as its own subclass
  (`createStateVariables()`): `TcpCubicStateVariables` extends
  `TcpClassicAlgorithmBaseStateVariables`, which extends the TCB (`TcpStateVariables`). These
  `.msg` structs have descriptors; Qtenv shows them under `tcp.conn-N`, `state`.
- OMNeT++ applies the pattern assignments of the NED types of all ancestors to a module that C++
  creates (`cNedNetworkBuilder::assignParametersFromPatterns()`). A `simple` type may hold a
  pattern assignment; a hyphen is not allowed in the pattern (`conn*`, not `conn-*`).

### The options

The first version of this plan compared parameter submodules of `Tcp` (`tcp.cubic`, option A),
the algorithm as a submodule of each connection (option B), a compound `Tcp` (rejected: it moves
the connections) and no submodules. The owner chose B: Qtenv then shows the algorithm that a
running connection uses, with its parameters and its state.

### The shape

- `tcp.conn-N.algorithm`: each connection creates its algorithm as a submodule, of the NED type
  that `tcpAlgorithmClass` names, at its open or fork.
- Each algorithm has a NED type with the name of its C++ class, in
  `inet.transportlayer.tcp.flavours`; the parameters of that algorithm move there.
- `Tcp`, `LinuxTcp` and `LegacyTcp` stay `simple`; `LinuxTcp` and `LegacyTcp` set the one
  algorithm default that differs by a pattern assignment.
- The connection keeps its timers, its events and its signals, and calls the algorithm through
  the same C++ calls. Connection paths, statistics and fingerprints stay.

`flavours/TcpAlgorithm.ned`:

```ned
package inet.transportlayer.tcp.flavours;

import inet.common.SimpleModule;

//
// The congestion control and loss recovery algorithm of one TCP connection.
// Each connection of ~Tcp creates one as its submodule `algorithm`, of the
// NED type that the `tcpAlgorithmClass` parameter of ~Tcp names, or that the
// socket chose. The connection calls it from C++; its state is the state of
// the connection, extended by the algorithm, and Qtenv shows it with the
// parameters of the algorithm.
//
simple TcpAlgorithm extends SimpleModule
{
    parameters:
        @class(-);
        @display("i=block/control");
}
```

`flavours/TcpNewReno.ned` (and `TcpReno`, `TcpTahoe`, `TcpWestwood`, `TcpNoCongestionControl`,
`DumbTcp` alike, without parameters):

```ned
package inet.transportlayer.tcp.flavours;

//
// NewReno (RFC 6582), with the SACK recovery of RFC 6675 when SACK is on.
//
simple TcpNewReno extends TcpAlgorithm
{
    parameters:
        @class(TcpNewReno);
}
```

`flavours/TcpCubic.ned`:

```ned
package inet.transportlayer.tcp.flavours;

//
// CUBIC (RFC 9438), with the slow start of Linux (HyStart).
//
simple TcpCubic extends TcpAlgorithm
{
    parameters:
        @class(TcpCubic);
        double beta = default(0.7); // multiplicative-decrease factor ... (RFC 9438)
        double c = default(0.4); // scaling constant C of the cubic window-growth curve
        bool fastConvergence = default(true); // ... (RFC 9438 section 4.7)
        bool tcpFriendliness = default(true); // ... (RFC 9438 section 4.3)
        // Linux behaviors (no RFC)
        int cntClamp = default(20); // ...
        double delta @unit(s) = default(10ms); // ...
        bool hystartEnabled = default(false); // HyStart: leave slow start when the path appears full
        double hystartAckDelta @unit(s) = default(2ms);
        double hystartDelayMax @unit(s) = default(16ms);
        double hystartDelayMin @unit(s) = default(4ms);
        int hystartDetect = default(3);
        int hystartLowWindow = default(16);
}
```

`flavours/DcTcp.ned`:

```ned
simple DcTcp extends TcpAlgorithm
{
    parameters:
        @class(DcTcp);
        double g = default(0.0625); // the estimation gain g of RFC 8257 section 4.2
}
```

`flavours/TcpVegas.ned` (the values of the code today, in segments):

```ned
simple TcpVegas extends TcpAlgorithm
{
    parameters:
        @class(TcpVegas);
        int alpha = default(2); // below this many extra segments in the network, cwnd grows
        int beta = default(4); // above this many, cwnd shrinks
        int gamma = default(1); // in slow start, above this many, Vegas leaves slow start
}
```

`Tcp.ned`: the groups CUBIC and DCTCP go; the group "congestion control" keeps
`tcpAlgorithmClass` and its documentation names the algorithm types. `dctcpGamma` goes (D-120).

`LinuxTcp.ned` (and `LegacyTcp.ned` alike):

```ned
simple LinuxTcp extends Tcp
{
    parameters:
        @class(Tcp);
        tcpAlgorithmClass = default("TcpCubic");
        conn*.algorithm.hystartEnabled = default(true);   // was: hystartEnabled = default(true)
        ...
}
```

ini: `**.tcp.cubicBeta` becomes `**.tcp.conn-*.algorithm.beta` (or `**.algorithm.beta`).

### The C++ shape

- `TcpAlgorithm` derives from `SimpleModule` instead of `cObject`. It keeps `conn`, `state` and
  its interface; the connection creates it with `cModuleType::create("algorithm", this)` (NED type
  from `tcpAlgorithmClass`: a qualified NED name, or a name in `inet.transportlayer.tcp.flavours`),
  and the fork takes the NED type of the listener's algorithm. The module goes with its
  connection; nothing deletes it by hand.
- The method that the connection calls to start the algorithm, `TcpAlgorithm::initialize()`, has
  the name of the module initialization of OMNeT++, which runs in the context of the algorithm
  module: the timers that it creates would belong to the algorithm, and the connection could not
  schedule them. It gets another name, and the connection calls it as today, in its own context.
  The module initialization only reads the parameters of the algorithm.
- The names that `SimpleModule` brings (`cancelEvent()`, `scheduleAfter()`, `scheduleAt()`,
  `getState()`) must not take over calls that go to the connection today; the step checks each
  use.
- The algorithm module watches the state (`WATCH_PTR`), so Qtenv shows its parameters and the
  state together; the connection keeps its watches.

### The decisions

- **D-117 — The algorithm of a connection is its submodule `algorithm`.** The owner chose this
  shape (option B) so that Qtenv shows the algorithm of a running connection with its parameters
  and its state. The NED types have the names of the C++ classes.
- **D-118 — Only the parameters of one algorithm move now.** CUBIC and HyStart (12), DCTCP (1)
  and the thresholds of Vegas (3, constants until now) move to their algorithm. The algorithm base
  also reads the parameters of the RTO, the persist timer, the delayed-ACK timer and keepalive;
  they stay in `Tcp`, because many configurations set them, and a later step can move them to a
  common base type.
- **D-119 — The names lose their prefixes.** CUBIC as RFC 9438 (`beta`, `c`); the gain of DCTCP
  `g`, as RFC 8257; Vegas `alpha`, `beta`, `gamma`, as Brakmo and Peterson. The CUBIC and HyStart
  parameters were never in a release, so they move without a deprecation.
- **D-120 — `dctcpGamma` goes.** It was in INET 4.7, but the owner keeps no parameters for
  backward compatibility: the release note names the new key `algorithm.g`.
- **D-121 — The connection keeps its events.** The timers stay messages of the connection, and
  the signals stay on the connection, so the module paths of the results stay; a log line of the
  initialization of the new module is the only new output.
- **D-122 — An algorithm needs a NED type.** A C++ algorithm class without one (from another
  project) stops with an error that tells to add a NED type; the release note says so.

Decisions of the implementation:

- **D-123 — Step 4 is two commits.** The rename of `initialize()` to `initializeAlgorithm()` and of
  the helper `cancelEvent()` to `cancelTimer()` has the depth `name`, and the module has the depth
  `refactor`; the rule of one depth for each commit separates them. The rename comes first and
  stands alone.
- **D-124 — The connection deletes the timers of the algorithm.** OMNeT++ deletes a module in the
  context of that module, so the destructor of the algorithm deleted the timers of the connection
  in a foreign context: 306 module tests printed "is deleting an object ... it doesn't own" at the
  end of the run, and 10 of them failed. `TcpConnection::deleteTimer()` cancels and deletes a timer
  with `Enter_Method_Silent()`, and the destructors of `TcpAlgorithmBase` and `DumbTcp` call it.
- **D-125 — The algorithm module has one stage of initialization**, as its connection. It only
  watches the state; with the stages of INET, Cmdenv would print one line for each stage at each
  open when the event banners are on.
- **D-126 — The log lines of the algorithm come from its module.** In a cComponent, `EV` takes the
  component as the source of the line, so the prefixes that show the source component (`%x`, `%y`,
  `%z`) and the log level of the module apply to the lines of the algorithm. The module tests do
  not show these prefixes, and the statistical test records no parameters
  (`**.param-recording=false`), so neither moves; D-121 named only the initialization line. The
  scalar file of an ordinary run records the parameters of the algorithm modules that exist at the
  end of the run.
- **D-127 — The moves of steps 5 and 6 have the depth `name`, not `change`.** Each value stays, and
  only the keys move, so the subjects say `name:`. HyStart does not leave slow start in the scenario
  of `tcp_cubic_hystart_1` with the defaults of `LinuxTcp`, also with a larger queue, so
  `tcp_cubic_hystart_2` reads the recorded parameter instead: the `Tcp` client keeps `false`, and
  the connection that the `LinuxTcp` listener forks has `true`.
- **D-128 — The thresholds of Vegas are state variables.** `TcpVegas` copies `alpha`, `beta` and
  `gamma` into its state, where Qtenv shows them, and stops with an error unless
  0 <= alpha <= beta and 0 <= gamma. The state variable of the gain of DCTCP keeps its name,
  `dctcp_gamma`: a rename would change the TCB lines of the log, and the parameter `g` documents
  it.

## 5. The steps

| Step | Commit | What |
| --- | --- | --- |
| 1 ✅ | `plan: add: group the parameters of Tcp` | this plan |
| 2 ✅ | `tcp: format: group the parameters of Tcp by the part of the protocol` | section 3; landed on master on 2026-10-15, the owner approved it |
| 3 ✅ | `plan: change: the algorithm of a connection becomes its submodule` | section 4 |
| 4a ✅ | `tcp: name: the connection starts its algorithm in initializeAlgorithm()` | the two names that clash with a module (D-123) |
| 4b ✅ | `tcp: refactor: the algorithm of a connection is its submodule` | `TcpAlgorithm` a module, the NED types without parameters, the creation, the fork, the watches, `deleteTimer()` (D-124); `tcp_algorithm_submodule_1`, `_2` |
| 5 ✅ | `tcp: name: the parameters of CUBIC and HyStart belong to TcpCubic` | 12 parameters; `LinuxTcp`, `LegacyTcp`; `tcp_cubic_hystart_1`, `tcp_cubic_hystart_2` (D-127) |
| 6 ✅ | `tcp: name: the gain of DCTCP belongs to DcTcp` | `g`; `dctcpGamma` goes (D-120); `tcp_dctcp_gain_1` |
| 7 ✅ | `tcp: add: the thresholds of Vegas as parameters` | `alpha`, `beta`, `gamma` with the values of today (D-128); `tcp_vegas_thresholds_1` |
| 8 ✅ | `doc: change: the release note of the algorithm submodule` | WHATSNEW entry 49, the migration guide |
| 9 ✅ | `plan: change: record the algorithm submodule` | sections 5 to 7 |

## 6. The evidence

Each commit builds alone; the module tests and the TCP protocol tests pass at each commit; unit
and serializer at the head. Step 4 changes no behavior: the CI fingerprint job moves no row, the
statistical results of the 95 TCP configurations stay (the chain helper of the statistics, at the
head), and the packetdrill suite of inet-gpl keeps 303 MATCH. Steps 5 to 7 move no CI row either,
because the defaults keep their values. A module test checks that Qtenv's model is there: the
connection has the submodule `algorithm` of the right type, and `LinuxTcp` turns HyStart on
through the pattern. The six gates and the combined pull-request gate.

## 7. The record

The evidence below was taken on master `2d16cb1614` (the grouping of step 2 is on origin with
it), at the commits `3416078f69` (plan), `89dde4d55b` (4a), `63533d9db0` (4b), `91b55a848e` (5),
`87a8241db9` (6), `1cbfbdb2b8` (7) and `751cca9985` (8).

For the landing, the branch was rebased on origin master `9b4b22258f`, whose 7 new commits change
IEEE 802.11 files, WHATSNEW and the migration guide. That master had two WHATSNEW entries with the
number 47, so the commit `333ea6383f` numbers the entry of the QoS DATA lifetime 48, and the
release note of this plan is entry 49. The rebased commits are `5abb63861b` (plan), `eacc040ed9`
(4a), `ee23386728` (4b), `308cfa76d2` (5), `4ad6c6a90f` (6), `346ade23b5` (7) and `c376be4e85`
(8). The owner asked to land first; the tests were not run again after the rebase.

| Evidence | Result |
| --- | --- |
| Each commit builds alone (debug, omnetpp-6.x) | yes; no undefined `inet::` symbol |
| Module tests at each commit | 641, 641, 643, 644, 645, 646, 646 PASS |
| Unit, serializer, protocol (`tcp`, `self`) at the head | 128, 4, 2 PASS |
| CI fingerprint job (podman, clang-16): master, 4a, 4b, 5, 6, 7 | OK at each; the one ERROR row is the expected 5 Gbps half-duplex row of `ethernet.csv`, the same as on master |
| Statistics of the 38 TCP working directories at the head, against statistics `8fdf7f5` | 122 KEEP, 4 SKIP (expected), 0 files change |
| inet-gpl packetdrill suite (inet-gpl `4a3894d`) at the head | 303 MATCH, 12 KERNEL_DRIFT, 3 UNSUPPORTED_FEATURE; each of the 318 verdicts as before the branch |
| Gates on `master..HEAD` | check-commits, check-classification, check-seals, check-source-seals, check-includes, check-ned-params, check-links and check-pull-request pass |

The new tests: `tcp_algorithm_submodule_1` (the client of `Tcp` has a `TcpNewReno` submodule
`algorithm`, and the connection that the `LinuxTcp` listener forks has `TcpCubic`),
`tcp_algorithm_submodule_2` (the error of D-122), `tcp_cubic_hystart_2` (D-127),
`tcp_dctcp_gain_1` (with g = 1, alpha is 1024 / 21504 = 0.047619 where the default gives
0.00297619) and `tcp_vegas_thresholds_1` (thresholds of 1000 segments let the queue drop packets,
the defaults drop none). Each fails before its commit.

Open, outside this plan: the parameters of the RTO, the persist timer, the delayed ACK and
keepalive, which `TcpAlgorithmBase` reads from `Tcp` (D-118), can move to a common base type later.
