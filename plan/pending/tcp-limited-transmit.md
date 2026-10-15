# Limited Transmit in the dupthresh mode (T2)

Status: **ready for review** — item T2 of `plan/pending/tcp-open-items.md` (finding G of `plan/done/tcp-packetdrill-repairs.md`), decision D-134; all steps done on 2026-10-15; T3 is stacked on this branch.

- Branch `topic/tcp-limited-transmit`, on master `d020d88ba8`.
- The decision numbers continue those of `plan/done/tcp-active-open-limit.md` (D-139 onward).

## 1. The problem

At a duplicate ACK before a recovery, step (3.3) of RFC 6675 sends up to one segment of previously unsent data, from HighData+1, as Limited Transmit of RFC 3042 does. `Rfc6675Recovery` takes the segment from `nextSeg()`. When no new data waits, rule 3 of `nextSeg()` returns an old unSACKed segment, so in the `dupthresh` mode the first duplicate ACK retransmits the first hole, and the recovery sends it again; a D-SACK follows. The `rack` mode, the default of `Tcp` and `LinuxTcp`, stopped the loop at an old segment, so only `lossDetectionMode = "dupthresh"` with SACK and Limited Transmit shows the defect. No module test reached it.

## 2. The steps

| Step | Commit | Test |
| --- | --- | --- |
| 1 ✅ | `plan: add: Limited Transmit in the dupthresh mode` | — |
| 2 ✅ | `tcp: fix: Limited Transmit sends only new data` | `tcp_limited_transmit_sack_1` (new) |
| 3 ✅ | `doc: change: the release note of Limited Transmit` | bug fix entry 2 of WHATSNEW |
| 4 ✅ | `plan: change: record Limited Transmit in the dupthresh mode` | — |

## 3. The decisions

- **D-139 — Step (3.3) sends from HighData+1, without `nextSeg()`.** RFC 6675 step (3.3) and RFC 3042 send only previously unsent data, and `nextSeg()` is not part of the step. The step now sends from `snd_max` in both loss detection modes, as Linux sends only new data before a recovery. In the `rack` mode, the old loop also stopped when `nextSeg()` returned a lost segment before new data; now new data goes out there too, as the RFC says.

## 4. The record

Commits on master `d020d88ba8`: `16ce3f3582` (plan), `4ce4cb9e07` (the fix with its test), `4f13c8e556` (the release note), and this record. A first version of the branch had the new test in the plan commit, because an earlier temporary commit left it staged; the branch was rebuilt before the landing so that the test is in the commit of the fix, with the same trees.

| Evidence | Result |
| --- | --- |
| Each commit builds alone; module tests at each commit | 658, 659, 659 PASS; no undefined `inet::` symbol |
| The new test fails before the fix | yes: at master the hole goes out twice (A007 and A008 at t=1.005), and the server reports a D-SACK (B006) |
| CI fingerprint job (podman, clang-16) at the fix | OK; the one ERROR row of `ethernet.csv` is the expected one of master |
| Statistics of the 38 TCP working directories, against statistics `dbd08ed` | 122 KEEP, 4 SKIP (expected), no file changes |
| Packetdrill suite of inet-gpl `4a3894d` | 303 MATCH, 12 KERNEL_DRIFT, 3 UNSUPPORTED_FEATURE; each of the 318 verdicts as before |
| INET's TCP protocol suite with inet-gpl | 333 tests: 329 PASS, 4 FAIL (expected) |

