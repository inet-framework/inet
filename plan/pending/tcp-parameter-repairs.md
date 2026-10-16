# The repairs of the TCP parameters

Status: **proposed** — written on 2026-10-16 from the review of all TCP parameters; waits for the owner.

- Branch `topic/tcp-parameter-repairs`, on master `675655fc35`. This branch holds only the plan; each item gets a branch of its own (section 4).
- The decision numbers continue those of `plan/done/tcp-open-items.md` (D-170 onward).
- The review: `audit/tcp-parameter-review/` (git-ignored) holds the nine group reports with file:line evidence and a suggested fix for each finding, and `verified.md`, the check of each medium and high finding in the code.

## 1. The problem

On 2026-10-16, nine review agents checked the 109 parameters of `Tcp`, `TcpCubic`, `TcpVegas` and `DcTcp`, each against its NED doc and the expectation of a user. 55 work as documented, 45 have defects (2 high, 19 medium, 24 low), 9 need only a doc fix, and none is dead.

The packetdrill suite of inet-gpl does not see these defects: its harness maps few sysctls to parameters and runs the defaults of `LinuxTcp`, and each of its 318 verdicts stays MATCH. The module tests miss them too, because their configurations avoid the faulty paths (for example, each classic ECN test turns delayed ACKs off).

The goal follows the rule of the owner: INET gets close to Linux and follows the RFCs where `Tcp` claims them, but it does not copy Linux. A parameter that exists must work as its doc says. Where a full repair would add much code for little use, the item asks the owner to choose between the repair and a documented limit (section 6).

## 2. The items

### Wave 1: defects that the defaults of `Tcp` or `LinuxTcp` reach

| ID | Parameter | Reached by | Defect | Repair |
| --- | --- | --- | --- | --- |
| P1 | `lossDetectionMode` = "rack" | `Tcp` | RACK detects reordering only with `adaptiveReorderingEnabled`, which `Tcp` turns off, so RACK retransmits reordered data and halves cwnd (RFC 8985 section 6.2 step 3 is mandatory) | run the reordering detection always; keep only the growth of `reordering` behind `adaptiveReorderingEnabled` |
| P2 | `hystartEnabled` (TcpCubic) | `LinuxTcp` | the first HyStart round never starts when `snd_una` is 2^31 or more, that is, for half of the random ISS values | reset the HyStart round at the start of the connection, as Linux `cubictcp_init()` |
| P3 | `beta` (TcpCubic) | `LinuxTcp` | at a second RTO in a row, ssthresh falls to 2 MSS, because TcpCubic takes the bytes in flight | compute ssthresh only at the first RTO of an episode, as RFC 5681 and Linux `tcp_enter_loss()` |
| P4 | `synLinearTimeouts` | `LinuxTcp` | an active open gives up after 11 s; Linux gives up after about 131 s, because it limits by time ((2^(n+1) − 1) × RTO), not by count | see Q1 |
| P5 | `initialWindow` = "rfc6928", `initialCwnd` | `LinuxTcp` | after one lost SYN, cwnd falls to 1 segment; RFC 6928 section 2 and Linux keep the initial window unless the SYN or SYN-ACK went more than once | for "rfc6928" and `initialCwnd`, reduce only after more than one retransmission |
| P6 | `lossUndoEnabled` | `LinuxTcp` | the undo of a spurious RTO restores cwnd, but go-back-N goes on and sends data that arrived | at an RTO undo, also clear the lost marks and `afterRto`, as Linux `tcp_try_undo_loss()` and RFC 4015 |
| P7 | `frtoEnabled` | `LinuxTcp` | F-RTO decides at the first ACK; RFC 5682 steps 2.b and 3 (send new data, decide at the second ACK) are missing | see Q2 |
| P8 | `adaptiveReorderingEnabled` | `LinuxTcp` | the learned reordering degree does not delay the start of recovery, which still comes at the third duplicate ACK | use the learned degree in step (1) and in `updateLost()`, as Linux `tcp_time_to_recover()` |
| P9 | `tcpEcnMode` (classic ECN) | `Tcp`, `LinuxTcp` with ECN | a delayed ACK loses the echo of a CE mark when an unmarked segment comes first; ACKs on data and FIN segments carry no ECE | enter the echo state when the CE segment arrives, and set ECE on each segment that acknowledges, as RFC 3168 section 6.1.3 |
| P10 | `beta` (TcpCubic) with ECN | `LinuxTcp` with ECN | at an ECN echo, CUBIC halves cwnd with the generic code; beta, W_max and the epoch stay | TcpCubic reacts to ECE with its own decrease, as RFC 9438 section 4.6 |

### Wave 2: defects of documented options

| ID | Parameter | Defect | Repair |
| --- | --- | --- | --- |
| P11 | TcpVegas `alpha`, `beta` | in congestion avoidance, cwnd changes by one segment on each ACK, not once per RTT | apply the step once per RTT in congestion avoidance; keep the step per ACK in slow start |
| P12 | `advertisedWindow` > 65535 | a retransmitted SYN-ACK writes the window into the 16-bit field without a limit (131072 becomes 0) | write min(window, 65535) into the SYN and the SYN-ACK, and keep `rcv_adv` with it |
| P13 | `windowShrinkAllowed` (and locked buffers, auto-tuning) | duplicate data is charged to the buffer and never freed | charge only the bytes that the arrival added to the receive queue |
| P14 | `delayedAckFrameCount` | counts a segment as full-sized only at the receiver's own send MSS | count by bytes: an ACK when the unacknowledged data reach `delayedAckFrameCount` × RMSS (RFC 9293 SHLD-19) |
| P15 | `keepAliveProbeCount` | answered probes count when the interval is at least the idle time plus the RTT, so a live connection aborts | reset the probe count when a segment arrives after the last probe |
| P16 | `pmtudTimeout` | the MSS goes back to a stored value that can be 0 (ASSERT) or already reduced | record the MSS when it is settled, and bound the restore by `TCP_MAXSEG` and the route MTU |
| P17 | `pathMtu` | the path MTU search does not use it as its upper end and stalls below it; `setPathMtu()` before the open does nothing without `mtuProbing = 2` | bound the midpoint by `pathMtu`; apply the socket option at the open |
| P18 | `abortOnDataAfterClose` | the RST rule works only in FIN_WAIT_1/2: a CLOSE behind unsent data accepts new data, and a CLOSE with unread data sends FIN | see Q3 |
| P19 | `fastopenServerEnabled` | after a lost SYN-ACK, the server keeps its full initial window | at the SYN-ACK timeout, set cwnd to 1 SMSS (RFC 7413 section 4.2.2 with RFC 5681) |
| P20 | `fastopenCookieCacheSize` | 0 makes the first cookie erase from an empty map (crash) | reject values below 1 |

### Wave 3: low defects and doc fixes

| ID | Content |
| --- | --- |
| P21 | Range checks with a clear error: `mss` ≥ 1 (and not tiny), `baseMss` ≥ 1, `dupthresh` ≥ 1, `maxReordering` ≥ `dupthresh`, `delayedAckFrameCount` ≥ 1, `maxPersistTimeout` > 0, DcTcp `g` in (0, 1], the 12 TcpCubic parameters (`beta` in (0, 1), `c` > 0, times ≥ 0, `hystartDelayMin` ≤ `hystartDelayMax`, `hystartDetect` in 0..3), `initialSsthresh` > 0. |
| P22 | Small behavior repairs: `maxPersistTimeout` caps the first probe interval; `minshallNagle` starts `snd_sml` at the ISS; `notsentLowat` set on a listener reaches its connections and a change is checked at once; the SWS rule uses the receive buffer, so a buffer below one MSS opens; `windowAutoTuning` without `receiveBufferSize` gives an error; `alignOptions = false` pads SACK without NOPs; `syncookiesAlways` quantizes the MSS of the peer; TcpVegas and TcpWestwood read `initialSsthresh`; `keepAliveEnabled` starts with TcpNoCongestionControl; the RTO loss window uses `snd_effmss`. |
| P23 | Small ECN, close and Fast Open repairs: the bare TIME_WAIT ACK of AccECN; soft ICMP errors of the types Time Exceeded and Parameter Problem; ECT on the RST+ACK of DCTCP; TS.Recent only after the RFC 5961 ACK check; the strict cookie check takes only the configured length; the pending Fast Open limit counts per listener. |
| P24 | Doc fixes: `pushSegmentsOnWriteBoundary`, `sackSupport` (and `tcpAlgorithmClass`, see Q4), `limitedTransmitEnabled`, `tcpFriendliness`, `cntClamp`, `delta`, `mtuProbing` (needs `pmtudEnabled` on IPv4), `baseMss`, `accEcnOptionPolicy`, `advertisedWindow`, `dupthresh`, `lossUndoEnabled` (needs SACK), `fastopenBlackholeTimeout` (see Q5), and the stale comments in inet-gpl (`parser.y`, `sysctls.yaml`). |

Two items of the review stay out of this plan, because they are not defects of a parameter: the idle restart of `LinuxTcp` follows RFC 5681 and not Linux `tcp_cwnd_restart()`, and `PacketDrillApp` does not map `TCP_CONGESTION`.

## 3. Out of scope

- New features without a parameter, and an exact copy of Linux (decision of the owner, 2026-10-16).
- The congestion algorithms and options that the review found correct.
- Items of other protocols.

## 4. The method for each item

Each item is a branch of its own, `topic/tcp-<item>`, in a worktree of the same name; small items of one area may share a branch. A module test comes first and fails before the repair; it covers the configuration that the review found faulty, which the current tests avoid. Each commit builds alone and passes the module tests; the CI fingerprint job runs at each code commit; a moved statistic gets its attribution; the packetdrill suite of inet-gpl and INET's TCP protocol suite run at each item, and inet-gpl follows a change of semantics. Commit messages follow the Before/After rules, with one source line for each paragraph (PR-MSG-WRAP). A renamed parameter loses its old name, and WHATSNEW names the change.

Wave 1 changes the behavior of the defaults, so baselines can move: P1 for `Tcp` with reordering, P2, P3 and P10 for `TcpCubic`, P9 for ECN with delayed ACKs. Each moved row needs its cause.

## 5. The order

| Order | Items | Note |
| --- | --- | --- |
| 1 | P1 | `Tcp` default; small |
| 2 | P2, P3 | `LinuxTcp` default (CUBIC); small; one branch |
| 3 | P9, P10 | ECN; P10 after P9, as both touch the ECE path |
| 4 | P5, P6, P8 | `LinuxTcp` recovery; small to medium |
| 5 | P4, P7 | by the answers to Q1 and Q2 |
| 6 | P11 to P20 | options; P11 first (high) |
| 7 | P21 to P24 | one or two branches |

## 6. Questions for the owner, with recommendations

- **Q1 — P4, `synLinearTimeouts`.** (a) Model the time limit of Linux: with linear timeouts, the open fails when the time since the first SYN reaches the doubling schedule of `synRetries` (Linux `tcp_model_timeout()`), so `LinuxTcp` gives up after about 131 s as Linux. (b) Keep the count limit and document that the linear timeouts shorten the setup. **Recommendation: (a)**; it is a small change inside the SYN timer, and the setup time of `LinuxTcp` then matches Linux.
- **Q2 — P7, `frtoEnabled`.** (a) Add steps 2.b and 3 of RFC 5682: after the first new ACK, send new data and decide at the second ACK. (b) Document that F-RTO decides at the first ACK and misses a delay spike with one ACK per segment. **Recommendation: (a)**; `LinuxTcp` turns F-RTO on, and the missing steps are the part that detects the common case. The size is medium.
- **Q3 — P18, `abortOnDataAfterClose`.** (a) Extend the rule: a CLOSE with unread data sends RST (RFC 1122 section 4.2.2.13, Linux), and new data after a CLOSE that waits behind unsent data also gets RST. (b) Document that only FIN_WAIT_1/2 is covered. **Recommendation: (a)**, under the parameter, so `Tcp` keeps its behavior.
- **Q4 — `sackSupport` with an algorithm without SACK recovery** (TcpTahoe, TcpVegas, TcpWestwood, TcpNoCongestionControl, DumbTcp). (a) Keep SACK as a receive capability: negotiate it and send SACK blocks; only the sending algorithm ignores them, as in Linux. (b) Document that such an algorithm turns SACK off in both directions. **Recommendation: (b)**; (a) moves the fingerprints of these algorithms for little use.
- **Q5 — `fastopenBlackholeTimeout`.** (a) Detect the black hole at the third SYN timeout and at the give-up, as Linux. (b) Remove the "as Linux" claims and keep the second timeout. **Recommendation: (a)**; it is a one-line change of the count, with the test.
- **Q6 — The order and the scope.** Do wave 1 first, then wave 2, then wave 3, each item on its own branch as in section 4. **Recommendation: yes.**

## 7. The steps of this plan

| Step | Commit | What |
| --- | --- | --- |
| 1 ✅ | `plan: add: the repairs of the TCP parameters` | this plan |
| 2 | `plan: change: the decisions of the owner on the TCP parameter repairs` | section 6 |
| 3 | — | each item on its own branch, in the order of section 5; this plan records the branches and their results |

## 8. The record

| Item | Branch | Result |
| --- | --- | --- |
