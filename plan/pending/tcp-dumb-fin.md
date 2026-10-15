# The FIN of DumbTcp (T3)

Status: **in progress** — item T3 of `plan/pending/tcp-open-items.md` (D-101 of `plan/done/tcp-staged-landing.md`).

- Branch `topic/tcp-dumb-fin`, on `topic/tcp-limited-transmit` (T2), because both add an entry to the bug fixes of WHATSNEW.
- The decision numbers continue those of `plan/pending/tcp-limited-transmit.md` (D-140 onward).

## 1. The problem

The retransmission timer of DumbTcp restarts at each expiry and calls `retransmitData()`, also when nothing is outstanding. So DumbTcp sends an acknowledged FIN again every 2 s until the connection closes; in FIN_WAIT_2, with a peer that does not close, it goes on to the end of the run. RFC 6298 section 5.2: "When all outstanding data has been acknowledged, turn off the retransmission timer."

## 2. The steps

| Step | Commit | Test |
| --- | --- | --- |
| 1 | `plan: add: the FIN of DumbTcp` | — |
| 2 | `tcp: fix: DumbTcp stops its timer when all is acknowledged` | `tcp_dumb_fin_rexmit_2` (new) |
| 3 | `doc: change: the release note of the FIN of DumbTcp` | — |
| 4 | `plan: change: record the FIN of DumbTcp` | — |

## 3. The decisions

- **D-140 — DumbTcp follows RFC 6298 section 5.2.** The timer stops when an ACK acknowledges all outstanding data, the FIN included, and an expiry with nothing outstanding sends nothing; new data starts the timer again, as before.
