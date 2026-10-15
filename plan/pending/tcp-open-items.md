# The open TCP items

Status: **planned** — the owner chooses the items and their order (section 6).

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
| T11 | The packetdrill wrappers: steps 7 and 8, and the new scripts | open plan | `plan/pending/packetdrill-tcp-protocol-tests.md` |

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

`checksumMode` defaults to `"declared"`: the segment carries no checksum, against RFC 9293 MUST-2. The computed mode works, but a computed checksum serializes the segment, and the payload that comes back has lost its region tags: `examples/inet/nclients -c lwip__inet` then stops. The TCP evidence records the repair as blocked since 2026-09-17, with UDP gap 1 for the same cause. The cause is in `src/inet/common/packet/`, which is sealed. Proposal: first check whether the repair of `SharingRegionTagSet` (`83396aa655`) changed the facts; then the owner decides (question Q4). Effect of a new default: every TCP and UDP fingerprint moves.

### T10 — The timer parameters in a common base type of the algorithms

`TcpAlgorithmBase` reads the parameters of the RTO, the persist timer, the delayed ACK and keepalive from `Tcp`. The owner kept them in `Tcp` on 2026-10-15 (D-118). They could move to a NED base type of the algorithms, with `LinuxTcp` and `LegacyTcp` setting them by pattern. Effect: many ini keys change (`**.tcp.minRexmitTimeout` and others), so it needs a release note; no simulation changes. Size: medium. Question Q5.

### T11 — The packetdrill wrappers: steps 7 and 8, and the new scripts

`plan/pending/packetdrill-tcp-protocol-tests.md` waited for the TCP branch, which is on master now. Its CI job runs the wrappers: on master `bba1db4c81`, the TCP suite of the protocol job has 333 tests, 329 PASS and 4 FAIL (expected); the job fails only in the IGMP, MLD, ND and RIP suites. Open: steps 7 and 8 of that plan (the evidence and the guide), and wrappers for the scripts that inet-gpl added since (INET has 306 wrappers, inet-gpl has 321 scripts in `tests/protocol/tcp`). Then that plan moves to `plan/done`. Size: small.

## 3. Out of scope

- The KERNEL_DRIFT rows of the packetdrill suite (12): Linux 7.1.3 itself does not pass these scripts.
- The UNSUPPORTED_FEATURE rows (3): the scripts change a `tc qdisc` in the middle of the run, and INET does not model the drops of the local queue.
- The deviations of `Tcp.ned` that the model keeps on purpose: exact timers, PSH ignored on arrival, the congestion window in bytes, the timestamp clock of the simulation, and the flow control that `autoRead` turns off.
- The failures of other protocols in the CI protocol job (IGMP, MLD, ND, RIP): other agents own them.

## 4. The method for each item

Each item is a branch of its own, `topic/tcp-<item>`, in a worktree of the same name, as the repairs before. An item with more than one step gets its own plan in `plan/pending`. A test comes first and fails before the repair; each commit builds alone and passes the module tests; the CI fingerprint job runs at each code commit; a moved statistic gets its attribution; an item of the Linux mode runs the packetdrill suite of inet-gpl, and inet-gpl follows a change of semantics. Commit messages follow the Before/After rules, with one source line for each paragraph (PR-MSG-WRAP). A moved or renamed parameter loses its old name, and WHATSNEW names the new key.

## 5. The proposed order

| Wave | Items | Why |
| --- | --- | --- |
| 1 | T11, T1, T2, T3, T4 | small; T1 and T2 break a MUST or a step of an RFC; T11 closes an open plan |
| 2 | T5 (a, then b and c), T6 (DRS first) | the Linux mode, with Linux as the oracle |
| 3 | T7, T8 | new features with an API and a design |
| — | T9, T10 | the owner decides first |

## 6. Questions for the owner

- **Q1 (T1):** Do the SYN retransmissions alone end an active open in `Tcp` and `LinuxTcp`, with 75 s kept only in `LegacyTcp` through a new parameter?
- **Q2 (T7):** Does HyStart++ go into the slow start of all the flavours, or only into `TcpCubic`? And does `Tcp` turn it on by default, as RFC 9438 asks?
- **Q3 (T8):** Urgent data on both sides with the socket API, or the receive side only? And the out-of-band byte of Linux by a parameter of `LinuxTcp`?
- **Q4 (T9):** Who repairs the region tags of a serialized payload in the sealed `src/inet/common/packet/`, and may the default of `checksumMode` move after that?
- **Q5 (T10):** Do the timer parameters move now, or later?
- **Q6:** Is the order of section 5 right?

## 7. The steps of this plan

| Step | Commit | What |
| --- | --- | --- |
| 1 | `plan: add: the open TCP items` | this plan |
| 2 | — | the owner answers section 6; then each item starts on its own branch, and this plan records the branches and their results |
