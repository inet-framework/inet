# The FIN of DumbTcp (T3)

Status: **done** — item T3 of `plan/pending/tcp-open-items.md` (D-101 of `plan/done/tcp-staged-landing.md`); landed on master on 2026-10-15 with T2 and T4, the owner approved it; the statistics repository records the DumbTcp file of D-141.

- Branch `topic/tcp-dumb-fin`, on `topic/tcp-limited-transmit` (T2), because both add an entry to the bug fixes of WHATSNEW.
- The decision numbers continue those of `plan/done/tcp-limited-transmit.md` (D-140 onward).

## 1. The problem

The retransmission timer of DumbTcp restarts at each expiry and calls `retransmitData()`, also when nothing is outstanding. So DumbTcp sends an acknowledged FIN again every 2 s until the connection closes; in FIN_WAIT_2, with a peer that does not close, it goes on to the end of the run. RFC 6298 section 5.2: "When all outstanding data has been acknowledged, turn off the retransmission timer."

## 2. The steps

| Step | Commit | Test |
| --- | --- | --- |
| 1 ✅ | `plan: add: the FIN of DumbTcp` | — |
| 2 ✅ | `tcp: fix: DumbTcp stops its timer when all is acknowledged` | `tcp_dumb_fin_rexmit_2` (new); six fingerprints (D-141) |
| 3 ✅ | `doc: change: the release note of the FIN of DumbTcp` | bug fix entry 3 of WHATSNEW |
| 4 ✅ | `plan: change: record the FIN of DumbTcp` | — |

## 3. The decisions

- **D-140 — DumbTcp follows RFC 6298 section 5.2.** The timer stops when an ACK acknowledges all outstanding data, the FIN included, and an expiry with nothing outstanding sends nothing; new data starts the timer again, as before.
- **D-141 — Six fingerprints and one statistics file move by the timer that stops.** The six runs of `examples/inet/tcpclientserver` `inet-dumb` change their fingerprints. In run 0, the timer of the client no longer expires after the ACK of its FIN (41 REXMIT events instead of 42), so one resent FIN and its ACK are gone: the server receives one segment less (2 to 1 in `rcvSeq:vector:count` of its connection), and the client one ACK less (744 to 743). The other 245 changed scalars of `omnetpp.ini-inet-dumb-#0.sca` are the packet counts of that pair along the path. The commit of the fix records the six fingerprints; the statistics repository records the file at the landing.

## 4. The record

Commits on `topic/tcp-limited-transmit` (T2): `047d925258` (plan), `9636ff7e6c` (the fix with its test and the six fingerprints), `c21950841c` (the release note), and this record.

| Evidence | Result |
| --- | --- |
| Each commit builds alone; module tests at each commit | 659, 660, 660 PASS; no undefined `inet::` symbol |
| The new test fails before the fix | yes: at master the client sends its acknowledged FIN again at t=3.5, 5.5, 7.5 and 9.5 s |
| CI fingerprint job (podman, clang-16) | before the baselines, the six rows of D-141; with the baselines, OK at the rebuilt fix `9636ff7e6c`; the one ERROR row of `ethernet.csv` is the expected one of master |
| Statistics of the 38 TCP working directories, against statistics `dbd08ed` | 121 KEEP, 4 SKIP (expected), 1 UPDATE: `examples/inet/tcpclientserver/omnetpp.ini-inet-dumb-#0.sca` (D-141) |
| Packetdrill suite of inet-gpl `4a3894d` | 303 MATCH, 12 KERNEL_DRIFT, 3 UNSUPPORTED_FEATURE; each of the 318 verdicts as before |
| INET's TCP protocol suite with inet-gpl | 333 tests: 329 PASS, 4 FAIL (expected) |

