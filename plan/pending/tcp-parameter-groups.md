# Group the parameters of Tcp, and decide which parts become submodules

Status: **in progress** — the grouping is done; the submodules wait for the decision of the owner.

- Branch `topic/tcp-parameter-groups`, on master `00e916e3ed`.

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

## 4. Submodules

### The constraints

1. `Tcp` is a simple module, and NED gives a simple module no submodules. A submodule must
   either be created in C++ at initialization, as the `conn-N` modules are, or `Tcp` must become
   a compound module. The second moves the protocol and every connection one level down
   (`tcp.<core>.conn-N`): each result name, chart filter, statistical baseline and fingerprint
   that names a connection changes, and so do the ini keys of the 102 parameters.
2. A flavour (`tcpAlgorithmClass`) is one object per connection, and a socket can choose its own
   flavour (the open command carries `tcpAlgorithmClass`). So the parameters of an algorithm must
   exist for each flavour that a socket can choose, not only for the default of the module.
3. `LinuxTcp` and `LegacyTcp` set defaults in NED. NED cannot set the defaults of a submodule
   that C++ creates: such a submodule takes its defaults from its own NED type and from ini.
   Between `Tcp` and `LinuxTcp`, one algorithm parameter differs: `hystartEnabled`.

### The candidates

| Part | Parameters | Separate? | Verdict |
| --- | --- | --- | --- |
| CUBIC and HyStart | 12 | read only by `TcpCubic`; set at most once in INET | the strong candidate |
| DCTCP | 1 | read only by `DcTcp` | with CUBIC, if the algorithms move |
| Vegas | 0 (alpha 2, beta 4, gamma 1 segments are constants in the code) | would become parameters | with CUBIC, if the algorithms move |
| loss detection and recovery | 10 | no: the connection reads them into its state; the RACK and TLP timers are in `TcpConnection`, PRR in `Rfc6675Recovery`, F-RTO and the undo in the algorithm base; SACK negotiation, not the user, chooses the recovery | a group, not a submodule |
| TCP Fast Open | 11 | partly: a separate feature with host-wide state in `Tcp` (the server key, the cookie cache, the blackhole timer) | a submodule would fit if `Tcp` were compound; as a C++-created one, a moderate refactor |
| keepalive, path MTU, ECN | 4, 5, 6 | small, read into the connection state | groups |

### The options for the algorithms

- **A — Parameter submodules of the algorithm types.** `Tcp` creates `tcp.cubic`, `tcp.dctcp`
  (and `tcp.vegas`) at initialization; the per-connection objects stay as they are and read the
  parameters of their type's submodule. Small C++ change; 13 ini keys move (`**.tcp.cubicBeta`
  becomes `**.tcp.cubic.beta`). Constraint 3: `LinuxTcp` needs another way to turn HyStart on,
  for example a parameter of `Tcp` that names the NED type of the CUBIC submodule.
- **B — The algorithm as a submodule of each connection.** The flavour becomes a module,
  `tcp.conn-N.algorithm`, of the NED type that `tcpAlgorithmClass` names. This is the cleanest
  OMNeT++ model (the algorithm with its own parameters and signals, visible in Qtenv), but the
  largest change: the `TcpAlgorithm` hierarchy becomes `cSimpleModule`, the connection creates
  and forks it, the ini keys become `**.algorithm.beta`, and Linux defaults need NED types such
  as `LinuxTcpCubic`.
- **C — `Tcp` as a compound module.** Rejected by constraint 1.
- **D — No submodules.** The groups of step 2, with the prefixes `cubic*`, `hystart*` and
  `dctcp*` that the parameters already have.

Recommendation: D now (step 2); then A for the algorithms if the owner wants submodules, because
it separates the 13 algorithm parameters at a small cost and keeps the connection code as it is.
B only if the algorithms should become visible modules in their own right.

## 5. The steps

| Step | Commit | What |
| --- | --- | --- |
| 1 | `plan: add: group the parameters of Tcp` | this plan |
| 2 | `tcp: format: group the parameters of Tcp by the part of the protocol` | section 3 |
| 3 | (after the decision) the submodules of section 4 | — |
| 4 | the record | — |
