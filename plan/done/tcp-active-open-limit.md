# The limit of an active open (T1)

Status: **done** — item T1 of `plan/pending/tcp-open-items.md`, decision D-129; landed on master on 2026-10-15, the owner approved it.

- Branch `topic/tcp-active-open-limit`, rebased for the landing on master `def7b1cebb` (the T11 commits, documents and plans only).
- The decision numbers continue those of `plan/pending/tcp-open-items.md` (D-135 onward).

## 1. The problem

An active open of INET fails after 75 s (`TCP_TIMEOUT_CONN_ESTAB` in `TcpConnection.h`), independently of `synRetries`. RFC 9293 section 3.8.3 asks for at least 3 minutes of SYN retransmissions (MUST-23). The defaults of `Tcp` would retransmit for about 24 minutes (`synRetries = 12`, `initialRto = 1s`, `maxRexmitTimeout = 240s`), but the timer stops the open at 75 s.

A second defect hides behind the timer. When the SYN retransmissions run out first, the connection aborts, and at an abort the application gets no indication. With `LinuxTcp` (6 retries and 4 linear timeouts, 11 s) this path is the active one: a connect to a silent host never tells its application. For a passive open that does not fork, the abort also closes the listener, where the timeout returns it to LISTEN.

## 2. The steps

| Step | Commit | Test |
| --- | --- | --- |
| 1 ✅ | `plan: add: the limit of an active open` | — |
| 2 ✅ | `tcp: fix: an open that runs out of SYN retransmissions times out` | `tcp_syn_retries_1` (now expects `TCP_I_TIMED_OUT`), `tcp_syn_retries_2` (new: the listener returns to LISTEN), `tcp_socket_synretries_1` and `_2` (the transition of the timeout) |
| 3 ✅ | `tcp: change: an active open lasts as long as its SYN retransmissions` | `tcp_syn_4`, `_6` and `_8` (the setup goes on after 75 s: MUST-23), `tcp_syn_12` (two retransmissions); three fingerprints (D-138) |
| 4 ✅ | `tcp: name: the timer of the deferred SYN of Fast Open` | — (D-136) |
| 5 ✅ | `doc: change: the release note of the limit of an active open` | WHATSNEW entry 50, the migration guide |
| 6 ✅ | `plan: change: record the limit of an active open` | — |

## 3. The decisions

- **D-135 — The end of the SYN retransmissions is the timeout of the setup.** It raises the event `TIMEOUT_CONN_ESTAB` instead of `ABORT`, so it takes the transitions of the old timer: an active open closes and tells the application (`TCP_I_TIMED_OUT`, as `ETIMEDOUT` of Linux), a passive open returns to LISTEN, and a forked connection closes (RFC 9293 section 3.10.7.3).
- **D-136 — The deferred SYN of Fast Open keeps its wait of 75 s, under its own name.** The timer that limited each setup also sent a deferred Fast Open SYN without data when no SEND came. That use stays, with the same 75 s, because no rule of RFC 7413 or of Linux gives another value; Linux waits for the write without a limit. The timer is `deferredSynTimer` now, in a commit of the depth `name`.
- **D-137 — The tests of the 75 s timeout now show MUST-23.** `tcp_syn_4`, `tcp_syn_6` and `tcp_syn_8` delete only the first segments, so their setups now complete at t=93 s, after the old limit, on the active and on the passive side. `tcp_syn_12` tests the timeout of a forked connection, so it sets `synRetries = 2` on both sides.
- **D-138 — Three fingerprints move, each by a later SYN.** In `examples/inet/ipsec` `TCPTraffic` and `TCPTraffic6`, the third connection is outside the IPsec policies, gets no answer, and sends its seventh SYN at t=127.2 s, where it failed at 75.2 s. In `examples/bgpv4/BgpLifecycle`, the BGP opens to a router that is down send a seventh SYN (at t=367 s and t=854 s). Commit 3 records their new values, so that the CI job passes at each commit.

## 4. The record

The evidence below was taken on master `3d3b535fc8`, at `0c5fe92361` (plan), `b80961e69b` (step 2), `ffbd24c647` (step 3), `e60fd7569d` (step 4) and `3402848b34` (step 5). For the landing, the branch was rebased on `def7b1cebb`, whose commits change only documents and plans, so the code of each commit stays the same: the commits are now `df9dd78601`, `903de81bb9`, `55ed59fe28`, `8316ec4dcc` and `577cfb61d7`. The statistics repository records the two IPsec files with the landing.

| Evidence | Result |
| --- | --- |
| Each commit builds alone (debug, omnetpp-6.x) | yes; no undefined `inet::` symbol |
| Module tests at each commit | 657, 658, 658, 658, 658 PASS |
| Each new or changed test fails at the commit before its own | yes: `tcp_syn_retries_1`, `_2` and `tcp_socket_synretries_1` before step 2; `tcp_syn_4`, `_6` and `_8` before step 3 |
| CI fingerprint job (podman, clang-16) | step 2: OK; step 3 before its baselines: the three rows of D-138, and the same three at step 4; with the baselines, OK at step 3 and at the head; the one ERROR row of `ethernet.csv` is the expected one of master |
| Statistics of the 38 TCP working directories at the head, against statistics `8fdf7f5` | 120 KEEP, 4 SKIP (expected), 2 UPDATE: the two IPsec TCP configurations of D-138, with 26 scalars each (the TCP states, the drops of IPsec and the windows of the connection that retries longer); the statistics repository gets their new values at the landing |
| Packetdrill suite of inet-gpl `4a3894d` at the head | 303 MATCH, 12 KERNEL_DRIFT, 3 UNSUPPORTED_FEATURE; each of the 318 verdicts as before |
| INET's TCP protocol suite with inet-gpl | 333 tests: 329 PASS, 4 FAIL (expected) |
| Gates on `master..HEAD` | check-commits, check-classification, check-seals, check-source-seals, check-includes, check-ned-params and check-links pass, with no note |

