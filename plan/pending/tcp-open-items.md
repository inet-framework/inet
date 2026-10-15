# The open TCP items

Status: **in progress** — the owner approved the plan and decided the questions on 2026-10-15 (section 6); T10 is closed.

- Branch `topic/tcp-open-items`, on master `bba1db4c81`. This branch holds only the plan; each item gets a branch of its own (section 4).
- The decision numbers continue those of `plan/done/tcp-parameter-groups.md` (D-129 onward).

## 1. The problem

The staged landing of the TCP branch, the packetdrill repairs, the receive memory repairs and the parameter groups are on master. The TCP model still has known gaps, and five plans and the documentation of `Tcp` name them, each at its own place. Nobody can see the whole list, or decide an order, without a search through about 4000 lines of plans.

This plan collects the open TCP items in one list. For each item it gives the evidence, the reference (RFC or Linux), the effect on the baselines and the size, and it proposes an order of branches. The owner decides which items to do and in which order.

## 2. The items

| ID | Item | Kind | Source |
| --- | --- | --- | --- |
| T1 | An active open fails after 75 s | defect: RFC 9293 MUST-23 | `Tcp.ned` (the comment of `synRetries`), S20 record of `plan/done/tcp-staged-landing.md` |
| T2 | Limited Transmit retransmits in the `dupthresh` mode (finding G) | defect: RFC 6675 step (3.3), RFC 3042 | section 5 of `plan/done/tcp-packetdrill-repairs.md` |
| T3 | DumbTcp sends an acknowledged FIN again | defect of a teaching flavour | D-101 of `plan/done/tcp-staged-landing.md` |
| T4 | The SYN data of Fast Open after a SYN-ACK that acknowledges only the SYN | difference from Linux, to check | S18 record of `plan/done/tcp-staged-landing.md` |
| T5 | The path MTU search (RFC 4821) | a SHOULD deviation and two missing Linux behaviors | D-95 of `plan/done/tcp-staged-landing.md`, `Tcp.ned` |
| T6 | The receive memory of the Linux mode | differences from Linux | section 5 of `plan/done/tcp-receive-memory-repairs.md` |
| T7 | HyStart++ (RFC 9406) | missing SHOULD of RFC 9438 | `Tcp.ned` ("Remaining gaps") |
| T8 | Urgent data | missing MUSTs of RFC 9293 | `Tcp.ned` ("Remaining gaps", the deviations of RFC 9293) |
| T9 | The default checksum mode (gap 2) | defect with a blocked repair: RFC 9293 MUST-2 | `doc/project/evidence/model/tcp/results.md` |
| T10 | The timer parameters in a common base type of the algorithms | structure | D-118 of `plan/done/tcp-parameter-groups.md` |
| T11 | The packetdrill wrappers: steps 7 and 8, and the new scripts | open plan | `plan/done/packetdrill-tcp-protocol-tests.md` |
| T12 | The data of a SYN without a Fast Open option | difference from Linux | found in T4 (`plan/done/tcp-fastopen-syn-data.md`, D-143) |

### T1 — An active open fails after 75 s

`TcpConnection.h` defines `TCP_TIMEOUT_CONN_ESTAB` as 75 s, and the connection-establishment timer ends each active open after that time, independently of `synRetries`. RFC 9293 section 3.8.3 says: "R2 for a SYN segment MUST be set large enough to provide retransmission of the segment for at least 3 minutes" (MUST-23). With the defaults of `Tcp` (`initialRto = 1s`, `synRetries = 12`, `maxRexmitTimeout = 240s`), the retransmissions of the SYN would go on for more than 3 minutes, but the timer stops the open at 75 s. Linux has no such timer: the SYN retransmissions end the open. With `tcp_syn_retries = 6` and `tcp_syn_linear_timeouts = 4`, the SYN timeouts are 1, 1, 1, 1, 1, 2 and 4 s, so an open of Linux fails after 11 s, also below the 3 minutes of MUST-23.

Proposal: the SYN retransmissions alone end an active open, as in Linux. `LegacyTcp` keeps 75 s through a new parameter, because its scenarios were written for it (question Q1). Effect: only scenarios in which a SYN gets no answer for more than 75 s change; the module tests of the SYN timeouts show which ones. Size: small.

### T2 — Limited Transmit retransmits in the `dupthresh` mode

At the first duplicate ACK with SACK, step (3.3) of RFC 6675 sends up to one segment of new data. `Rfc6675Recovery` takes the segment from `nextSeg()`, whose rule 3 returns an old unSACKed segment when no new data waits. In the `rack` mode, the code stops there (`lossDetectionMode == 1`); in the `dupthresh` mode it retransmits the first hole at the first duplicate ACK, and again when the recovery starts, and a D-SACK follows. RFC 3042 and step (3.3) send only new data.

Proposal: the `dupthresh` mode also sends only new data in this step. Effect: `LegacyTcp` uses `dupthresh`, so the module tests that turn SACK on with the legacy defaults change their dumps; the CI fingerprints of such scenarios can move. Size: small code, some expectations to update.

### T3 — DumbTcp sends an acknowledged FIN again

D-101 repaired the lost FIN without data of DumbTcp. DumbTcp still sends an acknowledged FIN again at each expiry of its timer, until the connection closes. Proposal: the timer of DumbTcp stops when all is acknowledged. Effect: `tcp_algorithm_dumb` and the three configurations that select DumbTcp. Size: small.

### T4 — The SYN data of Fast Open after a SYN-ACK that acknowledges only the SYN

The S18 record says: after such a SYN-ACK, INET retransmits one segment of the SYN data, and the rest waits for the retransmission timeout; Linux retransmits all of it (`tcp_rcv_fastopen_synack()`). The code marks all the unacknowledged SYN data lost and sends it with the ACK, and the packetdrill script `fastopen/client/syn-data-only-syn-acked` matches; but that script sends less than one segment. Proposal: first a test with SYN data of more than one segment of the MSS of the SYN-ACK (for example a SYN-ACK with a smaller MSS); then a repair, or the item closes. Size: small.

### T5 — The path MTU search (RFC 4821)

`mtuProbing = 2` runs the search of Linux `tcp_mtu_probe()`. Three parts differ:

- (a) A retransmission into the range of a probe ends it as lost, with the congestion response of the algorithm. RFC 4821 section 7.6.2 says that the loss of a probe alone SHOULD NOT count as congestion, and Linux repairs the probe with `tcp_simple_retransmit()`, without a congestion response. A repair needs a hook in the recovery of each flavour.
- (b) The search does not run again after the probe interval (`tcp_probe_interval`, 600 s).
- (c) No black-hole detection exists, so `mtuProbing = 1` is the same as off (`tcp_mtu_probing = 1` of Linux starts the search after repeated timeouts).

Effect: the search is off by default in `Tcp`, `LinuxTcp` and `LegacyTcp`, so no baseline moves; the tests of the search change. Size: medium, in three branches or three steps.

### T6 — The receive memory of the Linux mode

With `windowAutoTuning`, INET follows `tcp_rcvbuf_grow()` and the truesize of Linux, but four parts are missing:

- DRS (`tcp_rcv_space_adjust()`): the buffer grows with the data that the application reads in one receive RTT, which needs the receive RTT (`tcp_rcv_rtt_measure()`).
- The upper bound `tcp_rmem[2]` (6 MB by default).
- The collapse of `tcp_prune_queue()` (`tcp_collapse()`) when the memory overflows.
- The pad and the slab classes of the truesize of a small packet.

Effect: only `LinuxTcp` and the configurations that turn `windowAutoTuning` on; long transfers get other windows, so their statistics can move. The packetdrill suite does not reach these parts; the oracle is the Linux code, and new packetdrill scripts if the Linux selftests have some. Size: large; DRS first.

### T7 — HyStart++ (RFC 9406)

RFC 9438 section 4.10 says that CUBIC SHOULD use HyStart++ (RFC 9406). INET has only the hybrid slow start of Linux (`hystartEnabled` of `TcpCubic`), which RFC 9438 calls experimental, and which is off in `Tcp`. HyStart++ applies to any slow start: it leaves slow start for a conservative slow start (CSS) when the RTT of a round grows, and it can go back. Linux has no HyStart++, so there is no packetdrill oracle; the tests are module tests against the text of RFC 9406 (`../standards/RFC/rfc9406.txt`). Proposal: a parameter of the slow start, in `TcpCubic` and in the classic flavours (question Q2). Effect: none while it is off. Size: medium to large.

### T8 — Urgent data

RFC 9293 section 3.8.5 asks for urgent data: support (MUST-30), a sequence of urgent data of any length (MUST-31), the urgent pointer at the octet after the urgent data (MUST-62), and the notice to the application (MUST-32, MUST-33). INET carries the URG flag and the urgent pointer in the connection state, but a receiver treats urgent bytes as ordinary data, and the socket has no urgent send. Linux keeps one out-of-band byte (`MSG_OOB`, `SO_OOBINLINE`, `SIOCATMARK`); RFC 6093 describes this difference of the implementations. No script of the packetdrill suite uses urgent data. Proposal: the send and the receive side, the notice to the application in `TcpSocket`, and the out-of-band byte of Linux by a parameter (question Q3). Effect: none unless an application uses it. Size: large (protocol, socket API, `PacketDrillApp` in inet-gpl).

### T9 — The default checksum mode (gap 2)

`checksumMode` defaults to `"declared"`: the segment carries no checksum, against RFC 9293 MUST-2. The computed mode works, but a computed checksum serializes the segment, and the payload that comes back has lost its region tags: `examples/inet/nclients -c lwip__inet` then stops. The TCP evidence records the repair as blocked since 2026-09-17, with UDP gap 1 for the same cause. The cause is in `src/inet/common/packet/`, which is sealed. Proposal: first check whether the repair of `SharingRegionTagSet` (`83396aa655`) changed the facts; then the owner decides (question Q4). Effect of a new default: the CI fingerprint job decides which rows move; the checksum value is not an ingredient of `tplx`, but the serialization can change more than the value.

### T10 — The timer parameters in a common base type of the algorithms

`TcpAlgorithmBase` reads the parameters of the RTO, the persist timer, the delayed ACK and keepalive from `Tcp`. The owner kept them in `Tcp` on 2026-10-15 (D-118). They could move to a NED base type of the algorithms, with `LinuxTcp` and `LegacyTcp` setting them by pattern. Effect: many ini keys change (`**.tcp.minRexmitTimeout` and others), so it needs a release note; no simulation changes. Size: medium. Question Q5.

### T11 — The packetdrill wrappers: steps 7 and 8, and the new scripts

`plan/done/packetdrill-tcp-protocol-tests.md` waited for the TCP branch, which is on master now. Its CI job runs the wrappers: on master `bba1db4c81`, the TCP suite of the protocol job has 333 tests, 329 PASS and 4 FAIL (expected); the job fails only in the IGMP, MLD, ND and RIP suites. Open: steps 7 and 8 of that plan (the evidence and the guide), and wrappers for the scripts that inet-gpl added since (INET has 306 wrappers, inet-gpl has 321 scripts in `tests/protocol/tcp`). Then that plan moves to `plan/done`. Size: small.

### T12 — The data of a SYN without a Fast Open option

The INET server keeps the data of a SYN that carries no Fast Open option, and delivers it after the handshake: RFC 9293 section 3.10.7.2 allows this ("any other control or text should be queued for processing later"). A Linux server drops such data (`tcp_conn_request()` keeps no data without an accepted Fast Open cookie), and the SYN-ACK acknowledges only the SYN, so the client sends the data again. T4 found it, because its module test needed a tester command that removes the data of a SYN to play a Linux server. RFC 7413 section 4.2.2 also asks a Fast Open server to drop the data of a SYN whose cookie is not valid; the step first checks what INET does then.

Proposal, by the rule of section 6: `Tcp` keeps the data, as RFC 9293 allows; a new parameter makes `LinuxTcp` drop it, as Linux; the data of a SYN with an invalid cookie is dropped in each mode if INET keeps it now. Effect: only `LinuxTcp` and clients that send data on a SYN. Size: small.

## 3. Out of scope

- The KERNEL_DRIFT rows of the packetdrill suite (12): Linux 7.1.3 itself does not pass these scripts.
- The UNSUPPORTED_FEATURE rows (3): the scripts change a `tc qdisc` in the middle of the run, and INET does not model the drops of the local queue.
- The deviations of `Tcp.ned` that the model keeps on purpose: exact timers, PSH ignored on arrival, the congestion window in bytes, the timestamp clock of the simulation, and the flow control that `autoRead` turns off.
- The failures of other protocols in the CI protocol job (IGMP, MLD, ND, RIP): other agents own them.

## 4. The method for each item

Each item is a branch of its own, `topic/tcp-<item>`, in a worktree of the same name, as the repairs before. An item with more than one step gets its own plan in `plan/pending`. A test comes first and fails before the repair; each commit builds alone and passes the module tests; the CI fingerprint job runs at each code commit; a moved statistic gets its attribution; an item of the Linux mode runs the packetdrill suite of inet-gpl, and inet-gpl follows a change of semantics. Commit messages follow the Before/After rules, with one source line for each paragraph (PR-MSG-WRAP). A moved or renamed parameter loses its old name, and WHATSNEW names the new key.

## 5. The order

| Order | Item | Note |
| --- | --- | --- |
| 1 | T11 | small; it gives the later items the packetdrill wrappers in CI |
| 2 | T9, step 1 | the check of the blocker decides the size of T9 |
| 3 | T1, T2, T3, T4 | small defects; T4 starts with a test |
| 4 | T12 | small; found in T4, added by the owner on 2026-10-15 |
| 5 | T6 | DRS first: it changes long transfers of `LinuxTcp` |
| 6 | T5 | (a), then (b) and (c); the search is off by default |
| 7 | T7 | it changes a default of `Tcp`, so the CUBIC baselines move |
| 8 | T8 | large, with few users |
| 9 | T9, steps 2 and 3 | the repair of the packet API, then the default |
| — | T10 | closed (D-133) |

## 6. The decisions

The owner approved the plan and these answers on 2026-10-15, with one rule for all of them: `Tcp` has the behavior of the RFCs by default, `LinuxTcp` sets the behavior of Linux by parameters, and backward compatibility counts little; `LegacyTcp` gets no new parameters to keep old behavior.

- **D-129 — Only the SYN retransmissions end an active open (T1).** The constant of 75 s goes, without a new parameter. RFC 9293 section 3.8.3 allows R2 as a count, and `TcpSocket::setSynRetries()` sets it for one connection (MUST-21, as `TCP_SYNCNT`). `Tcp` keeps `synRetries = 12`, which the staged landing chose for MUST-23 (about 24 minutes); `LinuxTcp` keeps 6 retries with 4 linear timeouts (11 s, as Linux); `LegacyTcp` loses the 75 s.
- **D-130 — HyStart++ is a choice of the slow start of each algorithm (T7).** One parameter of the algorithm types, `slowStartExit = "none" | "hystart" | "hystartpp"`, replaces `hystartEnabled`, which was never in a release; `"hystart"` exists only in `TcpCubic`, as in Linux. `TcpCubic` defaults to `"hystartpp"` (RFC 9438 section 4.10, SHOULD); the classic algorithms default to `"none"` (RFC 5681); `LinuxTcp` and `LegacyTcp` set `"hystart"` by their pattern, because Linux has no HyStart++. HyStart++ runs only in the first slow start (RFC 9406).
- **D-131 — Urgent data on both sides, with the socket API (T8).** A packet tag of the send sets URG and the urgent pointer at the octet after the urgent data (MUST-62). `Tcp` keeps the urgent bytes in the stream (RFC 6093) and tells the application the position of the urgent mark (MUST-32, MUST-33). The parameter `urgentOutOfBand` of `LinuxTcp` takes the last urgent byte out of the stream, as `MSG_OOB` of Linux, and a socket option puts it back in the stream for one connection (`SO_OOBINLINE`). `PacketDrillApp` of inet-gpl gets `MSG_OOB`, `SO_OOBINLINE` and `SIOCATMARK`.
- **D-132 — The default checksum mode becomes `"computed"` (T9).** Step 1 checks the blocker on master. If it stays, step 2 repairs the region tags of a serialized payload on a branch of its own to master: the owner permitted the change of the sealed `src/inet/common/packet/` for this repair on 2026-10-15. Step 3 sets `checksumMode = "computed"` in `Tcp` (MUST-2) and keeps `"declared"` as an option for speed; the run time of the serialization is measured and recorded. The default of UDP stays for its owner.
- **D-133 — The timer parameters stay in `Tcp`; T10 closes.** The RTO, the persist timer, the delayed ACK and keepalive belong to the connection, not to a congestion control algorithm; Linux keeps them out of `tcp_congestion_ops`, and all algorithms but DumbTcp share them. A later refactor can move the RTO estimator from `TcpAlgorithmBase` into the connection, without a change of an ini key.
- **D-134 — The items without a question follow the RFC where Linux agrees.** T2: only new data in step (3.3), in both modes, without a parameter. T5: (a) a lost probe gets no congestion response, as RFC 4821 and Linux; (b) the new parameter `mtuProbeInterval`, 600 s as Linux; (c) black-hole detection makes `mtuProbing = 1` work as in Linux. T6: the parts of Linux under `windowAutoTuning`, with a new parameter for the bound of `tcp_rmem[2]`.

## 7. The steps of this plan

| Step | Commit | What |
| --- | --- | --- |
| 1 ✅ | `plan: add: the open TCP items` | this plan |
| 2 | `plan: change: the decisions of the owner on the open TCP items` | section 6 |
| 3 | — | each item on its own branch, in the order of section 5; this plan records the branches and their results |

## 8. The record

| Item | Branch | Result |
| --- | --- | --- |
| T11 | `topic/tcp-packetdrill-wrappers` | done 2026-10-15: step 8 (the guide, `2dfb5bba68`) and step 7 (the evidence, `c5d99b242e`) of `packetdrill-tcp-protocol-tests.md`, which moves to `plan/done`. No wrapper was missing: `suite.py gen-wrappers --check` finds the 306 wrappers up to date, and the 15 scripts without one are 12 of kernel drift and 3 that need IPv6, by design. Section 2 counted them wrong. |
| T9, step 1 | — (the CI copy, a temporary change) | done 2026-10-15: the blocker stays. With `checksumMode = "computed"` as the default of `Tcp` at master `3d3b535fc8`, the CI fingerprint job gives one ERROR, `examples/inet/nclients -c lwip__inet`: "Received 1 B of TCP data without a GenericAppMsgReq region tag at its front", and two FAILED rows, `examples/inet/ipsec` `TCPTraffic` and `TCPTraffic6`, whose fingerprints change; every other row stays, because many rows already run with `**.checksumMode="computed"`. The run of the job took 194 s, against 208 s and 232 s at master: no cost that this job can measure. Step 2 (the repair of the packet API) stays at its place in the order. |
| T1 | `topic/tcp-active-open-limit` | done, landed 2026-10-15 (plan `plan/done/tcp-active-open-limit.md`): the SYN retransmissions alone limit a setup, and an open that runs out of them times out with `TCP_I_TIMED_OUT` instead of an abort without an indication (a second defect, which hit every `LinuxTcp` connect to a silent host). Three fingerprints and two IPsec statistics move, each by a SYN after 75 s. |
| T2 | `topic/tcp-limited-transmit` | done, landed 2026-10-15 (plan `plan/done/tcp-limited-transmit.md`): Limited Transmit sends only new data, from HighData+1, in both loss detection modes; no baseline moves. |
| T3 | `topic/tcp-dumb-fin` | done, landed 2026-10-15 (plan `plan/done/tcp-dumb-fin.md`): the timer of DumbTcp stops when all is acknowledged (RFC 6298 section 5.2); six fingerprints and one statistics file move by the FIN resend that is gone. |
| T4 | `topic/tcp-fastopen-syn-data` | done, landed 2026-10-15 (plan `plan/done/tcp-fastopen-syn-data.md`): `LinuxTcp` already sends all the SYN data at once, as Linux; the real defect was Nagle, which held a retransmitted partial segment after a go-back until the next timeout. 27 fingerprints and 11 statistics files move. |
| T12 | `topic/tcp-syn-data-drop` | done, landed 2026-10-15 (plan `plan/done/tcp-syn-data-drop.md`): the parameter `dropSynData` drops the data of a SYN without Fast Open, as Linux; `LinuxTcp` sets it, `Tcp` keeps the data (RFC 9293); the case of an invalid cookie was already right. No baseline moves. |
