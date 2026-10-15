# The SYN data of Fast Open, and Nagle after a go-back (T4)

Status: **in progress** — item T4 of `plan/pending/tcp-open-items.md` (the S18 record of `plan/done/tcp-staged-landing.md`).

- Branch `topic/tcp-fastopen-syn-data`, on `topic/tcp-dumb-fin` (T3), because all three items add entries to the bug fixes of WHATSNEW.
- The decision numbers continue those of `plan/pending/tcp-dumb-fin.md` (D-142 onward).

## 1. The problem

The S18 record says: after a SYN-ACK that acknowledges only the SYN, INET retransmits one segment of the data of a Fast Open SYN, and the rest waits for the retransmission timeout; Linux retransmits all of it (`tcp_rcv_fastopen_synack()`). The packetdrill script `fastopen/client/syn-data-only-syn-acked` matches, but its SYN data fits one segment.

A module test with SYN data of three segments shows two facts. With `LinuxTcp` (an initial window of ten segments), the client sends all three segments at once, as Linux: the item, as written, no longer holds. With the legacy defaults (one segment, Nagle on), the client sends one segment for each ACK, and the last, partial segment waits for the retransmission timeout. The cause is general: Nagle counts data as unacknowledged up to `snd_max`. After a go-back, after a retransmission timeout or such a SYN-ACK, the partial segment at `snd_nxt` is a retransmission below `snd_max`, and Nagle holds it until the next timeout. RFC 9293 section 3.7.4 tests SND.NXT > SND.UNA, and Linux never sends a retransmission through Nagle.

The module tester cannot remove the data of a segment, which a server that drops the data of a SYN does (Linux without a valid cookie).

## 2. The steps

| Step | Commit | Test |
| --- | --- | --- |
| 1 | `plan: add: the SYN data of Fast Open and Nagle after a go-back` | — |
| 2 | `tests: add: the module tester removes the payload of a segment` | — |
| 3 | `tcp: fix: Nagle does not hold a retransmission` | `tcp_fastopen_syn_data_1`, `tcp_nagle_rexmit_1` (new); `tcp_stresstest_3` (ends earlier) |
| 4 | `doc: change: the release note of Nagle after a go-back` | — |
| 5 | `plan: change: record the SYN data of Fast Open and Nagle after a go-back` | — |

## 3. The decisions

- **D-142 — Nagle holds only new data.** The partial segment at `snd_nxt` goes when `snd_nxt` is below `snd_max`, that is, when it is a retransmission. The cork and the avoidance of the silly window syndrome stay as they are.
- **D-143 — The tester gets the command `nodata`.** It removes the payload of a segment, as a server that drops the data of a SYN. The INET server keeps the data of a SYN without a Fast Open option, as RFC 9293 section 3.10.7.2 allows; a Linux server drops it. That difference of the server is not part of this item.
