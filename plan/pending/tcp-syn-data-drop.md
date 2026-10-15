# The data of a SYN without Fast Open (T12)

Status: **ready for review** — item T12 of `plan/pending/tcp-open-items.md`, added by the owner on 2026-10-15.

- Branch `topic/tcp-syn-data-drop`, on master `b389fdfd50`.
- The decision numbers continue those of `plan/done/tcp-fastopen-syn-data.md` (D-145 onward).

## 1. The problem

The INET server keeps the data of a SYN without a Fast Open option in its receive queue and delivers it after the handshake, as RFC 9293 section 3.10.7.2 allows ("any other control or text should be queued for processing later"). A Linux server drops such data: `tcp_conn_request()` keeps no data of a SYN that Fast Open does not accept, the SYN-ACK acknowledges only the SYN, and the client sends the data again. A SYN with a Fast Open option and an invalid cookie is already right: INET drops its data, as RFC 7413 section 4.2.2 asks.

## 2. The steps

| Step | Commit | Test |
| --- | --- | --- |
| 1 ✅ | `plan: add: the data of a SYN without Fast Open` | — |
| 2 ✅ | `tcp: add: dropSynData, the SYN data of Linux, by a parameter` | `tcp_syn_data_drop_1` (new) |
| 3 ✅ | `tcp: change: LinuxTcp drops the data of a SYN without Fast Open` | `tcp_syn_data_drop_2` (new) |
| 4 ✅ | `doc: change: the release note of the SYN data of Linux` | WHATSNEW entry 51 |
| 5 ✅ | `plan: change: record the data of a SYN without Fast Open` | — |

## 3. The decisions

- **D-145 — A parameter selects the behavior of Linux.** `dropSynData` drops the data of a SYN that Fast Open does not accept; `Tcp` keeps the data (`false`), as RFC 9293 allows, and `LinuxTcp` drops it (`true`), as Linux. The parameter belongs to the connection setup, under the behaviors of Linux.

## 4. The record

Commits on master `b389fdfd50`: `69a8b1a974` (plan), `d07da52539` (the parameter), `58a5bb594b` (the default of `LinuxTcp`), `cc1b09e9a0` (the release note), and this record.

| Evidence | Result |
| --- | --- |
| Each commit builds alone; module tests at each commit | 662, 663, 664 PASS; no undefined `inet::` symbol |
| The new tests fail before their commits | yes: `tcp_syn_data_drop_1` before the parameter (the server acknowledges all 1400 bytes at the first segment), `tcp_syn_data_drop_2` before the default of `LinuxTcp` |
| CI fingerprint job (podman, clang-16) at both code commits | OK; the one ERROR row of `ethernet.csv` is the expected one of master |
| Statistics of the 38 TCP working directories, against statistics `ebda488` | 122 KEEP, 4 SKIP (expected), no file changes |
| Packetdrill suite of inet-gpl `4a3894d` | 303 MATCH, 12 KERNEL_DRIFT, 3 UNSUPPORTED_FEATURE; each of the 318 verdicts as before: no script of the suite sends data on a SYN without Fast Open to INET |
| INET's TCP protocol suite with inet-gpl | 333 tests: 329 PASS, 4 FAIL (expected) |

