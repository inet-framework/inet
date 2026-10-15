# The data of a SYN without Fast Open (T12)

Status: **in progress** — item T12 of `plan/pending/tcp-open-items.md`, added by the owner on 2026-10-15.

- Branch `topic/tcp-syn-data-drop`, on master `b389fdfd50`.
- The decision numbers continue those of `plan/done/tcp-fastopen-syn-data.md` (D-145 onward).

## 1. The problem

The INET server keeps the data of a SYN without a Fast Open option in its receive queue and delivers it after the handshake, as RFC 9293 section 3.10.7.2 allows ("any other control or text should be queued for processing later"). A Linux server drops such data: `tcp_conn_request()` keeps no data of a SYN that Fast Open does not accept, the SYN-ACK acknowledges only the SYN, and the client sends the data again. A SYN with a Fast Open option and an invalid cookie is already right: INET drops its data, as RFC 7413 section 4.2.2 asks.

## 2. The steps

| Step | Commit | Test |
| --- | --- | --- |
| 1 | `plan: add: the data of a SYN without Fast Open` | — |
| 2 | `tcp: add: dropSynData, the SYN data of Linux, by a parameter` | `tcp_syn_data_drop_1` (new) |
| 3 | `tcp: change: LinuxTcp drops the data of a SYN without Fast Open` | `tcp_syn_data_drop_2` (new) |
| 4 | `doc: change: the release note of the SYN data of Linux` | — |
| 5 | `plan: change: record the data of a SYN without Fast Open` | — |

## 3. The decisions

- **D-145 — A parameter selects the behavior of Linux.** `dropSynData` drops the data of a SYN that Fast Open does not accept; `Tcp` keeps the data (`false`), as RFC 9293 allows, and `LinuxTcp` drops it (`true`), as Linux. The parameter belongs to the connection setup, under the behaviors of Linux.
