# The SYN data of Fast Open, and Nagle after a go-back (T4)

Status: **done** — item T4 of `plan/pending/tcp-open-items.md` (the S18 record of `plan/done/tcp-staged-landing.md`); landed on master on 2026-10-15 with T2 and T3, the owner approved it; the statistics repository records the 11 files of D-144.

- Branch `topic/tcp-fastopen-syn-data`, on `topic/tcp-dumb-fin` (T3), because all three items add entries to the bug fixes of WHATSNEW.
- The decision numbers continue those of `plan/done/tcp-dumb-fin.md` (D-142 onward).

## 1. The problem

The S18 record says: after a SYN-ACK that acknowledges only the SYN, INET retransmits one segment of the data of a Fast Open SYN, and the rest waits for the retransmission timeout; Linux retransmits all of it (`tcp_rcv_fastopen_synack()`). The packetdrill script `fastopen/client/syn-data-only-syn-acked` matches, but its SYN data fits one segment.

A module test with SYN data of three segments shows two facts. With `LinuxTcp` (an initial window of ten segments), the client sends all three segments at once, as Linux: the item, as written, no longer holds. With the legacy defaults (one segment, Nagle on), the client sends one segment for each ACK, and the last, partial segment waits for the retransmission timeout. The cause is general: Nagle counts data as unacknowledged up to `snd_max`. After a go-back, after a retransmission timeout or such a SYN-ACK, the partial segment at `snd_nxt` is a retransmission below `snd_max`, and Nagle holds it until the next timeout. RFC 9293 section 3.7.4 tests SND.NXT > SND.UNA, and Linux never sends a retransmission through Nagle.

The module tester cannot remove the data of a segment, which a server that drops the data of a SYN does (Linux without a valid cookie).

## 2. The steps

| Step | Commit | Test |
| --- | --- | --- |
| 1 ✅ | `plan: add: the SYN data of Fast Open and Nagle after a go-back` | — |
| 2 ✅ | `tests: add: the module tester removes the payload of a segment` | — |
| 3 ✅ | `tcp: fix: Nagle does not hold a retransmission` | `tcp_fastopen_syn_data_1`, `tcp_nagle_rexmit_1` (new); `tcp_stresstest_3` (ends earlier); 27 fingerprints (D-144) |
| 4 ✅ | `doc: change: the release note of Nagle after a go-back` | bug fix entry 4 of WHATSNEW |
| 5 ✅ | `plan: change: record the SYN data of Fast Open and Nagle after a go-back` | — |

## 3. The decisions

- **D-142 — Nagle holds only new data.** The partial segment at `snd_nxt` goes when `snd_nxt` is below `snd_max`, that is, when it is a retransmission. The cork and the avoidance of the silly window syndrome stay as they are.
- **D-143 — The tester gets the command `nodata`.** It removes the payload of a segment, as a server that drops the data of a SYN. The INET server keeps the data of a SYN without a Fast Open option, as RFC 9293 section 3.10.7.2 allows; a Linux server drops it. That difference of the server is not part of this item.
- **D-144 — 27 fingerprints and 11 statistics files move, each by a retransmission that Nagle no longer holds.** The rows are in `examples/ethernet/arptest2`, `examples/inet/bulktransfer` (three), `examples/inet/nclients` `Filetransfer`, `examples/inet/tcp_eth_reconnect` (six) and the Tahoe, Vegas and Westwood runs of `examples/inet/tcpclientserver` (sixteen). In `bulktransfer` `inet__inet`, the five Nagle holds of the run were all holds of retransmissions (5 before, 0 after); in `tcp_eth_reconnect` `with_EtherMACFullDuplex`, 32 of 64. The commit of the fix records the fingerprints; the statistics repository records the 11 files of the same configurations at the landing. The packetdrill suite does not change: `LinuxTcp` uses the Nagle variant of Minshall.

## 4. The record

Commits on `topic/tcp-dumb-fin` (T3): `7178cae98b` (plan), `dda767b13d` (the tester), `07722bc21b` (the fix with its tests and the 27 fingerprints), `3e52446bb1` (the release note), and this record.

| Evidence | Result |
| --- | --- |
| Each commit builds alone; module tests at each commit | 660, 660, 662 PASS; no undefined `inet::` symbol |
| The new tests fail before the fix | the Fast Open test: the last segment at t=1.913 instead of t=0.709; the go-back test: the partial segment waits for the next timeout |
| CI fingerprint job (podman, clang-16) | before the baselines, the 27 rows of D-144; with them, OK at the fix; the one ERROR row of `ethernet.csv` is the expected one of master |
| Statistics of the 38 TCP working directories, against statistics `dbd08ed` | 110 KEEP, 4 SKIP (expected), 12 UPDATE: the 11 files of D-144, and the DumbTcp file of T3 with the same results as at T3 |
| Packetdrill suite of inet-gpl `4a3894d` | 303 MATCH, 12 KERNEL_DRIFT, 3 UNSUPPORTED_FEATURE; each of the 318 verdicts as before |
| INET's TCP protocol suite with inet-gpl | 333 tests: 329 PASS, 4 FAIL (expected) |

