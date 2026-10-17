# The repairs of the TCP parameters

Status: **in progress** — written on 2026-10-16 from the review of all TCP parameters; the owner decided the questions on 2026-10-16 (section 6).

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
| P4 | `synLinearTimeouts` | `LinuxTcp` | an active open gives up after 11 s; Linux gives up after about 131 s, because it limits by time ((2^(n+1) − 1) × RTO), not by count | document the count limit (D-170); a doc fix in P24 |
| P5 | `initialWindow` = "rfc6928", `initialCwnd` | `LinuxTcp` | after one lost SYN, cwnd falls to 1 segment; RFC 6928 section 2 and Linux keep the initial window unless the SYN or SYN-ACK went more than once | for "rfc6928" and `initialCwnd`, reduce only after more than one retransmission |
| P6 | `lossUndoEnabled` | `LinuxTcp` | the undo of a spurious RTO restores cwnd, but go-back-N goes on and sends data that arrived | at an RTO undo, also clear the lost marks and `afterRto`, as Linux `tcp_try_undo_loss()` and RFC 4015 |
| P7 | `frtoEnabled` | `LinuxTcp` | F-RTO decides at the first ACK; RFC 5682 steps 2.b and 3 (send new data, decide at the second ACK) are missing | document the limit (D-171); a doc fix in P24 |
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
| P18 | `abortOnDataAfterClose` | the RST rule works only in FIN_WAIT_1/2: a CLOSE behind unsent data accepts new data, and a CLOSE with unread data sends FIN | document that only FIN_WAIT_1/2 is covered (D-172); a doc fix in P24 |
| P19 | `fastopenServerEnabled` | after a lost SYN-ACK, the server keeps its full initial window | at the SYN-ACK timeout, set cwnd to 1 SMSS (RFC 7413 section 4.2.2 with RFC 5681) |
| P20 | `fastopenCookieCacheSize` | 0 makes the first cookie erase from an empty map (crash) | reject values below 1 |

### Wave 3: low defects and doc fixes

| ID | Content |
| --- | --- |
| P21 | Range checks with a clear error: `mss` ≥ 1 (and not tiny), `baseMss` ≥ 1, `dupthresh` ≥ 1, `maxReordering` ≥ `dupthresh`, `delayedAckFrameCount` ≥ 1, `maxPersistTimeout` > 0, DcTcp `g` in (0, 1], the 12 TcpCubic parameters (`beta` in (0, 1), `c` > 0, times ≥ 0, `hystartDelayMin` ≤ `hystartDelayMax`, `hystartDetect` in 0..3), `initialSsthresh` > 0. |
| P22 | Small behavior repairs: `maxPersistTimeout` caps the first probe interval; `minshallNagle` starts `snd_sml` at the ISS; `notsentLowat` set on a listener reaches its connections and a change is checked at once; the SWS rule uses the receive buffer, so a buffer below one MSS opens; `windowAutoTuning` without `receiveBufferSize` gives an error; `alignOptions = false` pads SACK without NOPs; `syncookiesAlways` quantizes the MSS of the peer; TcpVegas and TcpWestwood read `initialSsthresh`; `keepAliveEnabled` starts with TcpNoCongestionControl; the RTO loss window uses `snd_effmss`. |
| P23 | Small ECN, close and Fast Open repairs: the bare TIME_WAIT ACK of AccECN; soft ICMP errors of the types Time Exceeded and Parameter Problem; ECT on the RST+ACK of DCTCP; TS.Recent only after the RFC 5961 ACK check; the strict cookie check takes only the configured length; the pending Fast Open limit counts per listener. |
| P24 | Doc fixes: `synLinearTimeouts` and `synRetries` (P4: the count limit, not the time limit of Linux), `frtoEnabled` (P7: the decision at the first ACK), `abortOnDataAfterClose` (P18: FIN_WAIT_1/2 only), `pushSegmentsOnWriteBoundary`, `sackSupport` (and `tcpAlgorithmClass`: SACK off in both directions, D-173), `limitedTransmitEnabled`, `tcpFriendliness`, `cntClamp`, `delta`, `mtuProbing` (needs `pmtudEnabled` on IPv4), `baseMss`, `accEcnOptionPolicy`, `advertisedWindow`, `dupthresh`, `lossUndoEnabled` (needs SACK), `fastopenBlackholeTimeout` (the second SYN timeout, without the "as Linux" claims, D-174), and the stale comments in inet-gpl (`parser.y`, `sysctls.yaml`). |

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
| 5 | P11 to P20, but P18 | options; P11 first (high) |
| 6 | P21 to P24 | one or two branches; P24 has the docs of P4, P7 and P18 |

## 6. Questions for the owner, with recommendations

- **Q1 — P4, `synLinearTimeouts`.** (a) Model the time limit of Linux: with linear timeouts, the open fails when the time since the first SYN reaches the doubling schedule of `synRetries` (Linux `tcp_model_timeout()`), so `LinuxTcp` gives up after about 131 s as Linux. (b) Keep the count limit and document that the linear timeouts shorten the setup. **Recommendation: (a)**; it is a small change inside the SYN timer, and the setup time of `LinuxTcp` then matches Linux.
- **Q2 — P7, `frtoEnabled`.** (a) Add steps 2.b and 3 of RFC 5682: after the first new ACK, send new data and decide at the second ACK. (b) Document that F-RTO decides at the first ACK and misses a delay spike with one ACK per segment. **Recommendation: (a)**; `LinuxTcp` turns F-RTO on, and the missing steps are the part that detects the common case. The size is medium.
- **Q3 — P18, `abortOnDataAfterClose`.** (a) Extend the rule: a CLOSE with unread data sends RST (RFC 1122 section 4.2.2.13, Linux), and new data after a CLOSE that waits behind unsent data also gets RST. (b) Document that only FIN_WAIT_1/2 is covered. **Recommendation: (a)**, under the parameter, so `Tcp` keeps its behavior.
- **Q4 — `sackSupport` with an algorithm without SACK recovery** (TcpTahoe, TcpVegas, TcpWestwood, TcpNoCongestionControl, DumbTcp). (a) Keep SACK as a receive capability: negotiate it and send SACK blocks; only the sending algorithm ignores them, as in Linux. (b) Document that such an algorithm turns SACK off in both directions. **Recommendation: (b)**; (a) moves the fingerprints of these algorithms for little use.
- **Q5 — `fastopenBlackholeTimeout`.** (a) Detect the black hole at the third SYN timeout and at the give-up, as Linux. (b) Remove the "as Linux" claims and keep the second timeout. **Recommendation: (a)**; it is a one-line change of the count, with the test.
- **Q6 — The order and the scope.** Do wave 1 first, then wave 2, then wave 3, each item on its own branch as in section 4. **Recommendation: yes.**

### The decisions

The owner decided on 2026-10-16: "chose the smaller changes that keep the tests pass, order agreed".

- **D-170 — P4: the count limit stays.** `synLinearTimeouts` keeps the count of `synRetries` as the limit of the setup; the docs of both say that Linux limits by time, so with linear timeouts `LinuxTcp` gives up earlier than Linux (11 s against about 131 s with the defaults).
- **D-171 — P7: F-RTO decides at the first ACK.** The doc of `frtoEnabled` says that steps 2.b and 3 of RFC 5682 are not modeled, so a delay spike with one ACK per segment is not detected.
- **D-172 — P18: only FIN_WAIT_1/2.** The doc of `abortOnDataAfterClose` says that the RST covers new data in FIN_WAIT_1/2 only; a CLOSE with unread data sends FIN.
- **D-173 — SACK with an algorithm without SACK recovery stays off in both directions,** and the docs of `sackSupport` and `tcpAlgorithmClass` say so.
- **D-174 — The Fast Open black hole stays at the second SYN timeout;** the "as Linux" claims go from the code comment, the test description and the doc.
- **D-175 — Each repair is the smaller change that keeps the tests passing,** in the order of section 5. A test changes only where its old expectation was the defect itself, and the commit says so.

### The decisions of the work

- **D-176 — P1: the reordering detection compares with the SACK state before the ACK.** The first run of P1 moved 33 fingerprints of examples on paths that do not reorder. Two ACKs looked like reordering: a delayed ACK that acknowledges in-order data and SACKs later data, and an ACK that newly SACKs two blocks, the higher block first. RFC 8985 step 3 takes the newly acknowledged segments in ascending order, and Linux compares with `prior_fack`, so both checks now compare with the highest SACKed sequence number before the segment (`priorFack`). This repair is a commit of its own before P1, because the defect also reaches `LinuxTcp` through `adaptiveReorderingEnabled`. A D-SACK sets no reordering in RFC 8985, so its two checks keep the condition `adaptiveReorderingEnabled`.

## 7. The steps of this plan

| Step | Commit | What |
| --- | --- | --- |
| 1 ✅ | `plan: add: the repairs of the TCP parameters` | this plan |
| 2 ✅ | `plan: change: the decisions of the owner on the TCP parameter repairs` | section 6 |
| 3 | — | each item on its own branch, in the order of section 5; this plan records the branches and their results (section 8) |

## 8. The record

| Item | Branch | Result |
| --- | --- | --- |
| P1 | `topic/tcp-rack-reordering` | landed: 4e696c711b (the SACK state before the ACK, D-176, `tcp_reordering_2`), 455ce93c55 (RACK detects reordering, `tcp_reordering_3`); each new test fails before its commit; the module tests pass at each commit (672, 673); no fingerprint and no statistics file moves; each of the 318 packetdrill verdicts stays; the TCP protocol suite gives 329 PASS and 4 expected FAIL; the gates are clean |
| P2, P3 | `topic/tcp-cubic-repairs` | landed: 5a43da1af4 (the first HyStart round, `tcp_cubic_hystart_3`), a31e664953 (the second timeout keeps ssthresh, `tcp_cubic_rto_1`); each new test fails before its commit; the module tests pass at each commit (674, 675); no fingerprint moves; P2 moves no statistic; P3 moves only the ssthresh statistics of 6 runs with repeated timeouts, because TcpReno emitted the same value again (statistics 6e671ab); each of the 318 packetdrill verdicts stays; the TCP protocol suite gives 329 PASS and 4 expected FAIL; the gates are clean |
| P9, P10 | `topic/tcp-ecn-repairs` | landed: 41ef11d77a (the classic ECN echo, `tcp_ecn_echo_1`, `tcp_ecn_echo_2`), 02fda053d8 (the CUBIC reaction to an ECN-Echo, `tcp_cubic_ecn_1`); each new test fails before its commit; the module tests pass at each commit (677, 678); no fingerprint and no statistics file moves; each of the 318 packetdrill verdicts stays; the TCP protocol suite gives 329 PASS and 4 expected FAIL; the gates are clean |
| P5, P6, P8 | `topic/tcp-recovery-repairs` | landed: 81b9b98101 (the initial window after one lost SYN, `tcp_iw_5`, and `tcp_iw_6` for the limit), 0fb357a527 (the undo of a timeout ends its go-back-N, `tcp_undo_4`), bfd9157ede (the learned reordering degree starts the recovery, `tcp_reordering_4`); each new test fails before its commit, but `tcp_iw_6`, which keeps the limit of two lost SYNs; the module tests pass at each commit (680, 681, 682); no fingerprint and no statistics file moves; each of the 318 packetdrill verdicts stays; the TCP protocol suite gives 329 PASS and 4 expected FAIL; the gates are clean |
| P11 | `topic/tcp-vegas-steps` | landed: 2db41cd980 (the cwnd step of TcpVegas once per RTT, `tcp_vegas_steps_1`); the new test fails before the commit; the module tests pass (683); the seven TcpVegas runs of `tcpclientserver` change their fingerprints, and they and the TcpVegas run of `tcpthroughput Experiment` change their statistics (statistics 5ca3853); each of the 318 packetdrill verdicts stays; the TCP protocol suite gives 329 PASS and 4 expected FAIL; the gates are clean |
| P12, P13, P14 | `topic/tcp-receive-repairs` | landed: 915420e8bb (the window of a SYN-ACK, `tcp_synack_window_1`), 8a3a406554 (a duplicate takes no buffer space, `tcp_rcvbuf_shrink_3`), addd01f1be (the delayed ACK counts bytes against RMSS, `tcp_delack_rmss_1`); each new test fails before its commit; the module tests pass at each commit (684, 685, 686); P12 moves no fingerprint and only the rcvAdv statistics of `tcpwindowscale WS_enabled`, where the SYN-ACK gave 65436 (statistics ba389a6); P13 moves nothing; P14 moves eight fingerprints and five statistics files of runs where at least 2*RMSS bytes waited with fewer than two full-sized segments (statistics d69d21c); each of the 318 packetdrill verdicts stays; the TCP protocol suite gives 329 PASS and 4 expected FAIL; the gates are clean |
| P15 | `topic/tcp-keepalive-probes` | landed: 31b3ff1ca3 (an answered keepalive probe does not count, `tcp_keepalive_5`); the new test fails before the commit; the module tests pass (687); no fingerprint and no statistics file moves; each of the 318 packetdrill verdicts stays; the TCP protocol suite gives 329 PASS and 4 expected FAIL; the gates are clean |
| P19, P20 | `topic/tcp-fastopen-repairs` | landed: 31bdd4ddf3 (a Fast Open server lowers its initial window at a lost SYN-ACK, `tcp_fastopen_srvcwnd_1`), 83638a9f64 (fastopenCookieCacheSize below 1 is an error, `tcp_fastopen_cache_size_1`); each new test fails before its commit; the module tests pass at each commit (688, 689); no fingerprint and no statistics file moves; each of the 318 packetdrill verdicts stays; the TCP protocol suite gives 329 PASS and 4 expected FAIL; the gates are clean |
| P16, P17 | `topic/tcp-pmtu-repairs` | landed: 2327289943 (pmtudTimeout restores the MSS of the handshake, `tcp_pmtud_restore_1`, `tcp_pmtud_restore_2`), 49c7c3bce0 (pathMtu bounds the path MTU search, also from before the open, `tcp_mtu_probe_7`, `tcp_path_mtu_2`); each new test fails before its commit; the module tests pass at each commit (691, 693); no fingerprint and no statistics file moves; each of the 318 packetdrill verdicts stays; the TCP protocol suite gives 329 PASS and 4 expected FAIL; the gates are clean |
| P21 | `topic/tcp-range-checks` | landed: 1225a0d947 (seven Tcp parameters, `tcp_param_range_1` to `_7`), 0657a3dfb3 (the TcpCubic parameters, `tcp_cubic_param_range_1` to `_9`), bde1b76e3f (the gain g of DcTcp, `tcp_dctcp_param_range_1`); each new test fails before its commit; the module tests pass at each commit (700, 709, 710); no fingerprint and no statistics file moves; each of the 318 packetdrill verdicts stays; the TCP protocol suite gives 329 PASS and 4 expected FAIL; the gates are clean |
| P22 | `topic/tcp-small-repairs` | landed: d46a8815c9 (maxPersistTimeout caps the first probe, `tcp_persist_5`), a6d2bddb9e (minshallNagle after a wrap, `tcp_nagle_minshall_2`), 6b3d2b33c3 (notsentLowatScript of TcpTestClient), f9ba50ca2c (notsentLowat of a listener and a new mark, `tcp_notsent_lowat_3`, `_4`), bc214c359a (a buffer below one MSS opens the window, `tcp_sws_3`), c47beca253 (windowAutoTuning needs a buffer, `tcp_rcvbuf_autotune_3`), 9ff779376a (no NOPs before SACK without alignOptions, `tcp_options_align_2`), e94f6785ca (syncookiesAlways quantizes the MSS of the peer, `tcp_syncookies_2`), db81577ca0 (TcpVegas and TcpWestwood take initialSsthresh, `tcp_vegas_ssthresh_1`, `tcp_westwood_ssthresh_1`), e41083fed3 (keepalive with TcpNoCongestionControl, `tcp_keepalive_6`), cdebfa1e60 (the loss window after a timeout, `tcp_rto_cwnd_1`); each new test fails before its commit, also after the commit of the test feature that it needs; the module tests pass at each commit (711 to 722); db81577ca0 moves the seven TcpWestwood fingerprints of `tcpclientserver` and one statistics file (statistics 186a2b4); cdebfa1e60 moves 13 fingerprints and 13 statistics files of runs with timeouts (statistics f6ff8f0); the others move nothing; each of the 318 packetdrill verdicts stays; the TCP protocol suite gives 329 PASS and 4 expected FAIL; the gates are clean |
| P23 | `topic/tcp-small-ecn-close-repairs` | landed: 284c957c46 (the cookie command of TCPTester and the addresses of the extra connections of TcpTestClient), ac11ae266e (the ACE field on the ACK of a FIN in FIN_WAIT_1, `tcp_accecn_bare_1`, `_2`), 2513813c86 (Time Exceeded and Parameter Problem abort a setup, `tcp_icmp_soft_3`, `_4`, `tcp_icmp6_soft_2`), bda188ebe0 (the RST+ACK of DCTCP is ECT, `tcp_abort_rst_ack_2`), c3b32d7546 (TS.Recent after the RFC 5961 ACK check, `tcp_ts_recent_3`), e72c79ac9e (the strict cookie check takes only its length, `tcp_fastopen_cookie_len_1`), 9041bb30f7 (the pending Fast Open limit counts per listener, `tcp_fastopen_pending_2`); each new test fails before its commit, also after the commit of the test features; the module tests pass at each commit (722 to 731); no fingerprint and no statistics file moves; each of the 318 packetdrill verdicts stays; the TCP protocol suite gives 329 PASS and 4 expected FAIL; the gates are clean |
