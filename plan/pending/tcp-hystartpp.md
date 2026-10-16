# HyStart++ (T7)

Status: **in progress** — item T7 of `plan/pending/tcp-open-items.md`, decision D-130.

- Branch `topic/tcp-hystartpp`, on master `5f909b20f1`.
- The decision numbers continue those of `plan/done/tcp-path-mtu-search.md` (D-157 onward).
- The oracle is the text of RFC 9406 and RFC 9438 section 4.10 (`../standards/RFC/`). Linux has no HyStart++, so no packetdrill script covers it; the tests are module tests.

## 1. The problem

RFC 9438 section 4.10: "CUBIC SHOULD use the HyStart++ slow start algorithm [RFC9406]". INET has only the hybrid slow start of Linux (`hystartEnabled` of `TcpCubic`), which RFC 9438 calls experimental, and which is off in `Tcp`. So `TcpCubic` in `Tcp` overshoots in its first slow start until a loss.

HyStart++ (RFC 9406 section 4.2) runs in the first slow start of any algorithm. Per round of data, it keeps the minimum RTT of at least 8 samples. When the minimum RTT of a round grows by a threshold (an eighth of the last minimum, between 4 and 16 ms), slow start ends, and a conservative slow start (CSS) follows, which grows cwnd four times slower. When the minimum RTT falls below the value at the exit during CSS, the exit was spurious, and slow start resumes. After 5 rounds of CSS, congestion avoidance starts with ssthresh = cwnd. In slow start and CSS, one ACK grows cwnd by at most L = 8 segments, because INET does not pace. When CUBIC leaves the first slow start without a loss, RFC 9438 section 4.10 sets W_max to cwnd and K to 0.

## 2. The steps

| Step | Commit | Test |
| --- | --- | --- |
| 1 | `plan: add: HyStart++` | — |
| 2 | `tcp: change: slowStartExit replaces hystartEnabled` | `tcp_cubic_hystart_1` and `_2` take the new parameter; no behavior changes |
| 3 | `tcp: add: HyStart++ (RFC 9406) by slowStartExit` | `tcp_hystartpp_1` (new): the exit, CSS and congestion avoidance over a bottleneck; `tcp_hystartpp_2` (new): a spurious exit and the return to slow start; `tcp_hystartpp_3` (new): CUBIC after the exit, W_max = cwnd and K = 0 |
| 4 | `tcp: change: TcpCubic uses HyStart++ by default` | the fingerprints and statistics of CUBIC move, each explained |
| 5 | `doc: change: the release note of HyStart++` | WHATSNEW entry 55 |
| 6 | `plan: change: record HyStart++` | — |

## 3. The checks

As for T5: each commit builds and passes the module tests, a new test fails before its commit, the CI fingerprint job and the TCP statistics with the attribution of each moved row, the packetdrill suite of inet-gpl (`LinuxTcp` keeps the hybrid slow start of Linux, so no verdict should change), INET's TCP protocol suite with inet-gpl, and the gates. Steps 2 and 3 move no baseline; step 4 moves the baselines of the examples that use `TcpCubic` in `Tcp`.

## 4. The decisions

- **D-157 — `slowStartExit` in the four algorithms of `TcpClassicAlgorithmBase`.** `TcpReno`, `TcpNewReno`, `DcTcp` and `TcpCubic` get `slowStartExit = "none" | "hystart" | "hystartpp"`; `hystartEnabled` goes, as D-130 decided. `"hystart"` is the hybrid slow start of Linux `tcp_cubic`, which only `TcpCubic` runs; the other three use the slow start of RFC 5681 with it, as Linux does for reno. So the pattern `conn*.algorithm.slowStartExit = default("hystart")` of `LinuxTcp` and `LegacyTcp` works with each algorithm. `TcpTahoe`, `TcpVegas` and `TcpWestwood` keep their own slow starts.
- **D-158 — The constants of RFC 9406 section 4.3.** MIN_RTT_THRESH = 4 ms, MAX_RTT_THRESH = 16 ms, MIN_RTT_DIVISOR = 8, N_RTT_SAMPLE = 8, CSS_GROWTH_DIVISOR = 4, CSS_ROUNDS = 5, and L = 8, because INET does not pace. They are constants: the RFC lets an implementation tune them, but RECOMMENDS these values.
- **D-159 — The RTT samples and the first slow start.** Each ACK of new data gives a sample: the time since the first send of the first newly acknowledged segment, if it was not retransmitted (Karn), as `TcpCubic` already measures for the hybrid slow start and as Linux `tcp_clean_rtx_queue()` measures. A round ends when the ACK reaches the SND.NXT of its start. HyStart++ runs only while ssthresh has its initial value, so a congestion response (a loss recovery, an RTO, an ECN-Echo) ends it, and later slow starts are those of RFC 5681 (section 4.3: "An implementation SHOULD use HyStart++ only for the initial slow start").
- **D-160 — CUBIC after HyStart++.** When `TcpCubic` leaves the first slow start through HyStart++ without a loss, W_max becomes cwnd and K becomes 0 (RFC 9438 section 4.10), so the clamp of Linux for an unknown W_max (`cntClamp`) does not act. The hybrid slow start of Linux keeps W_max unknown, as Linux.

## 5. The record

(after the steps)
