# The receive buffer of the Linux mode (T15)

Status: **ready for review** — item T15 of `plan/pending/tcp-open-items.md`, added by the owner on 2026-10-16.

- Branch `topic/tcp-linux-receive-buffer`, on master `e35907827b`.
- The decision numbers continue those of `plan/done/tcp-receive-space.md` (D-151 onward).

## 1. The problem

T6 found that `LinuxTcp` turns `windowAutoTuning` on, but sets no receive buffer (`receiveBufferSize = -1B`). Auto-tuning acts only with a receive buffer, so with the defaults of `LinuxTcp`, neither the growth for out-of-order data (D-114) nor Dynamic Right-Sizing (T6) acts, and the offered window stays at `advertisedWindow`. Linux gives each socket the buffer `tcp_rmem[1]`, 131072 bytes (`tcp_init()` in `net/ipv4/tcp.c`).

A second defect stops the growth even with a receive buffer. With `windowScalingFactor = -1`, INET selects the smallest shift that represents the largest window that the connection can offer at the time of its SYN: with a buffer of 131072 bytes, half the buffer, so a shift of 1. The window field can then never offer more than 131070 bytes. In `tcp_rcv_space_1`, DRS grows the buffer of the server to 1098334 bytes, but the server offers at most 131070 bytes. Linux selects the shift from the largest window that auto-tuning can reach: `tcp_select_initial_window()` takes the maximum of the space, `tcp_rmem[2]` and `net.core.rmem_max`, so a shift of 10 for the 32 MiB of `tcp_rmem[2]`. When `SO_RCVBUF` locks the buffer, `tcp_connect_init()` and the request socket of a listener clamp the window to the space of that buffer, and the shift follows the buffer.

## 2. The steps

| Step | Commit | Test |
| --- | --- | --- |
| 1 ✅ | `plan: add: the receive buffer of the Linux mode` | — |
| 2 ✅ | `tcp: change: the window scale covers the reach of auto-tuning` | `tcp_rcv_wscale_1` (new): a shift of 10, and an offered window above 131070 bytes after DRS; `tcp_rcv_wscale_2` (new): with `SO_RCVBUF`, the shift follows the buffer |
| 3 ✅ | `tcp: change: LinuxTcp gets the receive buffer of Linux` | `tcp_rcv_space_3` (new): DRS acts with the defaults of `LinuxTcp` |
| 4 ✅ | `doc: change: the release note of the receive buffer of Linux` | WHATSNEW entry 53 |
| 5 ✅ | `plan: change: record the receive buffer of the Linux mode` | — |

## 3. The checks

As for T6: each commit builds and passes the module tests, a new test fails before its commit, the CI fingerprint job and the TCP statistics with the attribution of each moved row, the packetdrill suite of inet-gpl, INET's TCP protocol suite with inet-gpl, and the gates. No example and no baseline uses `LinuxTcp` or `windowAutoTuning`, so no baseline should move. The packetdrill harness sets `windowScalingFactor` and `receiveBufferSize` itself, so no verdict should change.

## 4. The decisions

- **D-151 — `LinuxTcp` has the receive buffer of Linux.** `receiveBufferSize = default(131072B)`, the `tcp_rmem[1]` of Linux 7.1. `Tcp` keeps `-1B`: no RFC gives a receive buffer apart from the window.
- **D-152 — The automatic window scale covers the reach of auto-tuning.** With `windowScalingFactor = -1`, auto-tuning on and a receive buffer that `SO_RCVBUF` does not lock, the shift represents `receiveBufferMax`, as `tcp_select_initial_window()` represents `tcp_rmem[2]`; with 32 MiB, the shift is 10. With a locked buffer or without auto-tuning, the rule stays as before. Linux also represents `net.core.rmem_max` (212992 bytes by default); INET has no parameter for it, because it changes the shift only for a `receiveBufferMax` below 131072 bytes, the smallest default of `tcp_rmem[2]`.

## 5. The record

Commits on master `e35907827b`: `35da5f3bfa` (plan), `96ed05c176` (T15 in the open items), `54edd88e4f` (the window scale), `61fd5cd6c5` (the receive buffer of `LinuxTcp`), `19770f3f63` (the release note), and this record.

The window scale changes the values of `tcp_rcv_space_1`. Before, the server offered at most 131070 bytes, and the reads stopped at 130320 bytes in one RTT. Now the reads double for three more RTTs. Each new value is that of the formula of Linux 7.1, computed by hand with its integer arithmetic, as for T6: reads of 231680, 463360 and 926720 bytes give windows of 1390080, 2780160 and 5560320 bytes and buffers of 2196669, 4393339 and 8786678 bytes, below the bound of 32 MiB. In `tcp_linux_defaults_1`, the windows after the handshake are now in units of 1024 bytes (63 and 64); Linux also rounds the window up to a multiple of the unit.

| Evidence | Result |
| --- | --- |
| Each commit builds alone; module tests at each commit | 668, 668, 670, 671, 671 PASS; no undefined `inet::` symbol |
| The new tests fail before their commits | yes: `tcp_rcv_wscale_1` and `tcp_rcv_wscale_2` before the window scale (the client announces a shift of 1), `tcp_rcv_space_3` before the receive buffer of `LinuxTcp` |
| CI fingerprint job (podman, clang-16) at both code commits | OK at each; the one ERROR row of `ethernet.csv` is the expected one of master |
| Statistics of the 38 TCP working directories, against statistics `ebda488` | 122 KEEP, 4 SKIP (expected), no file changes: no example uses `LinuxTcp` or `windowAutoTuning` |
| Packetdrill suite of inet-gpl `0dfbcae` | 303 MATCH, 12 KERNEL_DRIFT, 3 UNSUPPORTED_FEATURE; each of the 318 verdicts as before: the harness sets `windowScalingFactor` and, where a script sets `tcp_rmem`, `receiveBufferSize` |
| INET's TCP protocol suite with inet-gpl | 333 tests: 329 PASS, 4 FAIL (expected) |

The series run, the CI job, the statistics and the suites ran on the same trees with the hashes of before a correction of the trailer of the first commit (`b10f0476be` to `7378c5af14`); only the commit messages changed.

A finding for the owner: the first window of Linux is the space of its buffer, rounded down to a multiple of the MSS (`tcp_select_initial_window()`): 64240 bytes in the SYN of a client with an MSS of 1460, and 65160 bytes in the SYN-ACK of a server with timestamps (1448 bytes of payload in a segment). `LinuxTcp` offers `advertisedWindow`, 65535 bytes, in both. After the handshake, the window field counts units of 1024 bytes, which hides most of the difference. A later item can give `LinuxTcp` the first window of Linux.
