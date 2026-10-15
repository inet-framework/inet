# The limit of an active open (T1)

Status: **in progress** — item T1 of `plan/pending/tcp-open-items.md`, decision D-129.

- Branch `topic/tcp-active-open-limit`, on master `3d3b535fc8`.
- The decision numbers continue those of `plan/pending/tcp-open-items.md` (D-135 onward).

## 1. The problem

An active open of INET fails after 75 s (`TCP_TIMEOUT_CONN_ESTAB` in `TcpConnection.h`), independently of `synRetries`. RFC 9293 section 3.8.3 asks for at least 3 minutes of SYN retransmissions (MUST-23). The defaults of `Tcp` would retransmit for about 24 minutes (`synRetries = 12`, `initialRto = 1s`, `maxRexmitTimeout = 240s`), but the timer stops the open at 75 s.

A second defect hides behind the timer. When the SYN retransmissions run out first, the connection aborts, and at an abort the application gets no indication. With `LinuxTcp` (6 retries and 4 linear timeouts, 11 s) this path is the active one: a connect to a silent host never tells its application. For a passive open that does not fork, the abort also closes the listener, where the timeout returns it to LISTEN.

## 2. The steps

| Step | Commit | Test |
| --- | --- | --- |
| 1 | `plan: add: the limit of an active open` | — |
| 2 | `tcp: fix: an open that runs out of SYN retransmissions times out` | `tcp_syn_retries_1` (now expects `TCP_I_TIMED_OUT`), `tcp_syn_retries_2` (new: the listener returns to LISTEN) |
| 3 | `tcp: change: an active open lasts as long as its SYN retransmissions` | the tests of the SYN timeouts, and a new test of MUST-23 |
| 4 | `doc: change: the release note of the limit of an active open` | — |
| 5 | `plan: change: record the limit of an active open` | — |

## 3. The decisions

- **D-135 — The end of the SYN retransmissions is the timeout of the setup.** It raises the event `TIMEOUT_CONN_ESTAB` instead of `ABORT`, so it takes the transitions of the old timer: an active open closes and tells the application (`TCP_I_TIMED_OUT`, as `ETIMEDOUT` of Linux), a passive open returns to LISTEN, and a forked connection closes (RFC 9293 section 3.10.7.3).
