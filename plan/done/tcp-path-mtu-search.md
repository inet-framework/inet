# The path MTU search of RFC 4821 (T5)

Status: **reverted** — item T5 of `plan/pending/tcp-open-items.md`, decision D-134; landed on master on 2026-10-16, the owner approved it; reverted on 2026-10-16 by decision D-167 of the same plan (`4f0b96c9a7`).

- Branch `topic/tcp-path-mtu-search`, on master `36952dc48a`.
- The decision numbers continue those of `plan/done/tcp-linux-receive-buffer.md` (D-153 onward).
- The oracle is the code of Linux 7.1 (`/var/tmp/claude-staged/linux-src/`: `tcp_output.c`, `tcp_input.c`, `tcp_timer.c`) and RFC 4821. The packetdrill suite has one script of the search, `mtu_probe/basic-v4` (a probe that succeeds), and no script sets `tcp_mtu_probing = 1`.

## 1. The problem

`mtuProbing = 2` runs the search of Linux `tcp_mtu_probe()`. The documentation of `Tcp` lists three deviations, and a check of the code of Linux finds a fourth:

- **(a) The loss of a probe.** INET lowers `search_high` at any retransmission into the range of the probe, after the algorithm made its congestion response. RFC 4821 section 7.6.2: "When only the probe is lost, ... the loss SHOULD NOT be interpreted as congestion signal." Linux decides at the start of a loss recovery (`tcp_fastretrans_alert()`): when the probe is the first unacknowledged segment, the probe failed (`tcp_mtup_probe_failed()`), and `tcp_simple_retransmit()` sends it again as segments of the MSS, without a reduction of the congestion window.
- **(a') An inconclusive probe.** In Linux, any other retransmission while a probe is in flight ends the probe without a change of the bounds (`__tcp_retransmit_skb()`: "Inconclusive MTU probe"), and F-RTO does not start at a timeout with a probe in flight (`tcp_enter_loss()`). INET lowers `search_high` when the retransmission reaches the range of the probe, and ignores other retransmissions.
- **(b) No new search.** When the search converges, INET never searches again. RFC 4821 section 7.2: "When probing has converged, a timer SHOULD be set. When the timer expires, search_high should be reset to its initial value", at least 5 minutes, 10 recommended. Linux `tcp_mtu_check_reprobe()` starts again after `tcp_probe_interval`, 600 s.
- **(c) No black-hole detection.** `mtuProbing = 1` is the same as off. RFC 4821 section 7.7 treats a full-stop timeout, the R1 threshold of RFC 1122, as a black hole. Linux `tcp_write_timeout()` calls `tcp_mtu_probing()` when the retransmissions last longer than `tcp_retries1` (3) retransmissions at the minimum RTO: with `tcp_mtu_probing = 1` the search starts and the MSS falls to the base MSS; when the search already runs, its lower bound halves.

## 2. The steps

| Step | Commit | Test |
| --- | --- | --- |
| 1 ✅ | `plan: add: the path MTU search of RFC 4821` | — |
| 2 ✅ | `tcp: change: a lost MTU probe gets no congestion response` | `tcp_mtu_probe_2` and `tcp_mtu_probe_5` change; `tcp_mtu_probe_7`, `_8` and `_9` (new): the scenario of `tcp_mtu_probe_2` without SACK, with `TcpNewReno`, `TcpReno` and `TcpTahoe` |
| 3 ✅ | `tcp: change: another retransmission ends an MTU probe without a result` | `tcp_mtu_probe_10` (new): a loss before the probe; `tcp_mtu_probe_11` (new): a timeout with the probe in flight and F-RTO on |
| 4 ✅ | `tcp: add: mtuProbeInterval, a new path MTU search after a time` | `tcp_mtu_probe_12` (new) |
| 5 ✅ | `tcp: add: the black-hole detection of the path MTU search` | `tcp_mtu_probe_13` (new, `mtuProbing = 1`), `tcp_mtu_probe_14` (new, `mtuProbing = 2`) |
| 6 ✅ | `doc: change: the release note of the path MTU search` | WHATSNEW entry 54 |
| 7 ✅ | `plan: change: record the path MTU search` | — |

Each code step also updates the deviations of RFC 4821 in the documentation of `Tcp`.

## 3. The checks

As for T6 and T15: each commit builds and passes the module tests, a new test fails before its commit, the CI fingerprint job and the TCP statistics, the packetdrill suite of inet-gpl (`mtu_probe/basic-v4` must stay MATCH), INET's TCP protocol suite with inet-gpl, and the gates. The search is off by default in `Tcp`, `LinuxTcp` and `LegacyTcp`, and no example turns it on, so no baseline should move.

## 4. The decisions

- **D-153 — A lost probe gets no congestion response (a).** When an algorithm would start a loss recovery (the fast retransmit of the recoveries of RFC 5681, RFC 6582 and RFC 6675 with its RACK timer, which `TcpReno`, `TcpNewReno`, `TcpCubic` and `DcTcp` share, and of `TcpTahoe`), outside a reduction of the window, and the probe is the first unacknowledged segment, only the probe is lost, as Linux `tcp_fastretrans_alert()` decides. The search continues below the probe, the probe goes again at once as segments of the MSS, and cwnd stays. ssthresh rises to at least 3/4 of cwnd, as Linux `tcp_non_congestion_loss_retransmit()` sets it; that reduces nothing, and `TCP_INFO` shows the value of Linux. Until the ACK passes the probe, no loss recovery starts. Linux stays in its state Loss until the ACK passes all data sent before the repair, so it also repairs another loss of that data without a congestion response; INET responds to such a loss as to congestion, as RFC 4821 asks only for the loss of the probe alone. `TcpVegas` and `TcpWestwood` keep their own recoveries, which count the duplicate ACKs as their state and inflate and deflate cwnd by them, so they still treat a lost probe as congestion; the documentation of `Tcp` lists them. Linux has one recovery for all its algorithms. Linux takes one packet off cwnd when it sends a probe, and gives it back here; INET counts cwnd in bytes, and the probe uses its own bytes, so neither step exists in INET.
- **D-154 — Another retransmission ends a probe without a result (a').** While a probe is in flight, any other retransmission ends the probe and keeps the bounds of the search, as Linux `__tcp_retransmit_skb()`; F-RTO does not start at a timeout with a probe in flight, as Linux `tcp_enter_loss()`.
- **D-155 — `mtuProbeInterval` (b).** A new parameter, 600 s as Linux `tcp_probe_interval` and the 10 minutes of RFC 4821 section 7.2. When the search converged (less than 8 bytes, or the next probe not above the MSS) and the interval passed since the search started, the search starts again, from the MTU of the MSS in use up to the MSS of the peer, as Linux `tcp_mtu_check_reprobe()`.
- **D-156 — Black-hole detection (c).** With `mtuProbing = 1` or `2`, at an RTO after an earlier RTO retransmission, when the time since the first send of the oldest unacknowledged segment reaches that of 3 retransmissions at the minimum RTO, as Linux `tcp_write_timeout()` with `tcp_retries1 = 3` and `tcp_model_timeout()` (3 s with the 200 ms of `LinuxTcp`, 15 s with the 1 s of `Tcp`): with `mtuProbing = 1` the search starts; when it runs already, its lower bound falls to the MTU of the smaller of `baseMss` and half its MSS, at least 48 bytes (Linux `tcp_mtu_probe_floor` and `tcp_min_snd_mss`). The MSS falls to the lower bound, before the retransmission. RFC 4821 section 7.7 counts R1 as 3 retransmissions at the current RTO, and first sets the MTU to `search_low` before it halves; INET follows Linux, because `mtuProbing` is the switch of Linux, and the documentation of `Tcp` lists the difference.

## 5. The record

Commits on master `36952dc48a`: `f39017be81` (plan), `e0c409260c` (the lost probe), `c7b70e59aa` (the probe without a result), `d1bf93d126` (`mtuProbeInterval`), `c2105b15a5` (the black-hole detection), `0dc9e104dd` (the release note), and this record.

Facts found during the work:

- The repair of a lost probe (`TcpAlgorithmBase::repairLostMtuProbe()`) and the block of a new recovery until the ACK passes the probe (`isMtuProbeRepairPending()`) sit at the start of a loss recovery in the RFC 5681 and RFC 6582 recoveries, at the top of step (4) of the RFC 6675 recovery (so the RACK timer is covered), and in `TcpTahoe`. During the block, each duplicate ACK calls `sendData()`, whose bytes in flight follow Linux `tcp_packets_in_flight()`, so new data goes out as in the state Loss of Linux. A timeout ends the block.
- With SACK and without it, the duplicate ACKs do not mark the head as SACKed (`addInferredSack()` skips it), so the lost mark of the probe (`markLost()`) takes it out of the bytes in flight, as `tcp_mark_skb_lost()`.
- Because the window stays after a lost probe, the search goes on at once: in `tcp_mtu_probe_2`, a second probe (MTU 1144) follows and is lost too. Before, the reduction of the window stopped the probes (they need 11 segments of cwnd).
- Linux measures `mtuProbeInterval` from the start of the search, not from the convergence. When the search converges later than the interval, it starts again at once; `tcp_mtu_probe_12` lets it converge in a first burst so that the interval shows.
- The black-hole detection uses the first send time of the head region of the retransmission queue as the `retrans_stamp` of Linux. With `LinuxTcp` and a minimum RTO of 200 ms, the timeouts come after 0.204, 0.408, 0.816 and 1.632 s; the fourth one comes 3.07 s after the first send, so the detection acts there, as in Linux.

| Evidence | Result |
| --- | --- |
| Each commit builds alone; module tests at each commit | 671, 674, 676, 677, 679, 679 PASS; no undefined `inet::` symbol |
| The new and changed tests fail before their commits | yes, each of `tcp_mtu_probe_2`, `_5`, `_7` to `_14` |
| CI fingerprint job (podman, clang-16) at the four code commits | OK at each; the one ERROR row of `ethernet.csv` is the expected one of master |
| Statistics of the 38 TCP working directories, against statistics `ebda488` | 122 KEEP, 4 SKIP (expected), no file changes: no example sets `mtuProbing` |
| Packetdrill suite of inet-gpl `0dfbcae` | 303 MATCH, 12 KERNEL_DRIFT, 3 UNSUPPORTED_FEATURE; each of the 318 verdicts as before; `mtu_probe/basic-v4` stays MATCH |
| INET's TCP protocol suite with inet-gpl | 333 tests: 329 PASS, 4 FAIL (expected) |
