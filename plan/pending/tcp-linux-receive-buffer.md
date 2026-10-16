# The receive buffer of the Linux mode (T15)

Status: **in progress** — item T15 of `plan/pending/tcp-open-items.md`, added by the owner on 2026-10-16.

- Branch `topic/tcp-linux-receive-buffer`, on master `e35907827b`.
- The decision numbers continue those of `plan/done/tcp-receive-space.md` (D-151 onward).

## 1. The problem

T6 found that `LinuxTcp` turns `windowAutoTuning` on, but sets no receive buffer (`receiveBufferSize = -1B`). Auto-tuning acts only with a receive buffer, so with the defaults of `LinuxTcp`, neither the growth for out-of-order data (D-114) nor Dynamic Right-Sizing (T6) acts, and the offered window stays at `advertisedWindow`. Linux gives each socket the buffer `tcp_rmem[1]`, 131072 bytes (`tcp_init()` in `net/ipv4/tcp.c`).

A second defect stops the growth even with a receive buffer. With `windowScalingFactor = -1`, INET selects the smallest shift that represents the largest window that the connection can offer at the time of its SYN: with a buffer of 131072 bytes, half the buffer, so a shift of 1. The window field can then never offer more than 131070 bytes. In `tcp_rcv_space_1`, DRS grows the buffer of the server to 1098334 bytes, but the server offers at most 131070 bytes. Linux selects the shift from the largest window that auto-tuning can reach: `tcp_select_initial_window()` takes the maximum of the space, `tcp_rmem[2]` and `net.core.rmem_max`, so a shift of 10 for the 32 MiB of `tcp_rmem[2]`. When `SO_RCVBUF` locks the buffer, `tcp_connect_init()` and the request socket of a listener clamp the window to the space of that buffer, and the shift follows the buffer.

## 2. The steps

| Step | Commit | Test |
| --- | --- | --- |
| 1 | `plan: add: the receive buffer of the Linux mode` | — |
| 2 | `tcp: change: the window scale covers the reach of auto-tuning` | `tcp_rcv_wscale_1` (new): a shift of 10, and an offered window above 131070 bytes after DRS; `tcp_rcv_wscale_2` (new): with `SO_RCVBUF`, the shift follows the buffer |
| 3 | `tcp: change: LinuxTcp gets the receive buffer of Linux` | `tcp_rcv_space_3` (new): DRS acts with the defaults of `LinuxTcp` |
| 4 | `doc: change: the release note of the receive buffer of Linux` | WHATSNEW entry 53 |
| 5 | `plan: change: record the receive buffer of the Linux mode` | — |

## 3. The checks

As for T6: each commit builds and passes the module tests, a new test fails before its commit, the CI fingerprint job and the TCP statistics with the attribution of each moved row, the packetdrill suite of inet-gpl, INET's TCP protocol suite with inet-gpl, and the gates. No example and no baseline uses `LinuxTcp` or `windowAutoTuning`, so no baseline should move. The packetdrill harness sets `windowScalingFactor` and `receiveBufferSize` itself, so no verdict should change.

## 4. The decisions

- **D-151 — `LinuxTcp` has the receive buffer of Linux.** `receiveBufferSize = default(131072B)`, the `tcp_rmem[1]` of Linux 7.1. `Tcp` keeps `-1B`: no RFC gives a receive buffer apart from the window.
- **D-152 — The automatic window scale covers the reach of auto-tuning.** With `windowScalingFactor = -1`, auto-tuning on and a receive buffer that `SO_RCVBUF` does not lock, the shift represents `receiveBufferMax`, as `tcp_select_initial_window()` represents `tcp_rmem[2]`; with 32 MiB, the shift is 10. With a locked buffer or without auto-tuning, the rule stays as before. Linux also represents `net.core.rmem_max` (212992 bytes by default); INET has no parameter for it, because it changes the shift only for a `receiveBufferMax` below 131072 bytes, which `tcp_rmem[2]` of Linux cannot have.

## 5. The record

(after the steps)
