# Limited Transmit in the dupthresh mode (T2)

Status: **in progress** — item T2 of `plan/pending/tcp-open-items.md` (finding G of `plan/done/tcp-packetdrill-repairs.md`), decision D-134.

- Branch `topic/tcp-limited-transmit`, on master `d020d88ba8`.
- The decision numbers continue those of `plan/done/tcp-active-open-limit.md` (D-139 onward).

## 1. The problem

At a duplicate ACK before a recovery, step (3.3) of RFC 6675 sends up to one segment of previously unsent data, from HighData+1, as Limited Transmit of RFC 3042 does. `Rfc6675Recovery` takes the segment from `nextSeg()`. When no new data waits, rule 3 of `nextSeg()` returns an old unSACKed segment, so in the `dupthresh` mode the first duplicate ACK retransmits the first hole, and the recovery sends it again; a D-SACK follows. The `rack` mode, the default of `Tcp` and `LinuxTcp`, stopped the loop at an old segment, so only `lossDetectionMode = "dupthresh"` with SACK and Limited Transmit shows the defect. No module test reached it.

## 2. The steps

| Step | Commit | Test |
| --- | --- | --- |
| 1 | `plan: add: Limited Transmit in the dupthresh mode` | — |
| 2 | `tcp: fix: Limited Transmit sends only new data` | `tcp_limited_transmit_sack_1` (new) |
| 3 | `doc: change: the release note of Limited Transmit` | — |
| 4 | `plan: change: record Limited Transmit in the dupthresh mode` | — |

## 3. The decisions

- **D-139 — Step (3.3) sends from HighData+1, without `nextSeg()`.** RFC 6675 step (3.3) and RFC 3042 send only previously unsent data, and `nextSeg()` is not part of the step. The step now sends from `snd_max` in both loss detection modes, as Linux sends only new data before a recovery. In the `rack` mode, the old loop also stopped when `nextSeg()` returned a lost segment before new data; now new data goes out there too, as the RFC says.
