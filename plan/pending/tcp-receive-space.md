# The receive space of the Linux mode (T6)

Status: **planned** — item T6 of `plan/pending/tcp-open-items.md`, decision D-134; the owner decides the questions of section 5 before the code.

- Branch `topic/tcp-receive-space`, on `topic/tcp-syn-data-drop` (T12).
- The decision numbers continue those of `plan/pending/tcp-syn-data-drop.md` (D-146 onward).

## 1. The problem

With `windowAutoTuning` (on in `LinuxTcp`), INET grows the receive buffer in two cases only: when out-of-order data arrive (`tcp_rcvbuf_grow()` at the end of `tcp_data_queue_ofo()`, D-114) and when its memory overflows (`tcp_clamp_window()`). Its receive space stays at the first offer, at most 10 MSS. In Linux, the main growth of a long transfer is Dynamic Right-Sizing (DRS): at each read of the application, `tcp_rcv_space_adjust()` compares the bytes that the application read in the last receive RTT with the receive space, and `tcp_rcvbuf_grow()` grows the buffer to twice that, plus a growth for slow start, up to `tcp_rmem[2]`. Without DRS, the offered window of a `LinuxTcp` receiver in a long transfer stays smaller than in Linux, and the throughput of a path with a large bandwidth-delay product is smaller.

`plan/done/tcp-receive-memory-repairs.md` lists four parts of the receive memory that the packetdrill suite does not reach: DRS, the bound of `tcp_rmem[2]`, the collapse of `tcp_prune_queue()`, and the pad and the slab classes of the truesize of a small packet. No script of the suite checks the receive RTT or the receive space, and `TCP_INFO` of inet-gpl reports neither, so the oracle is the code of Linux 7.1 (`/var/tmp/claude-staged/linux-src/`, `net/ipv4/tcp_input.c` and `net/ipv4/tcp.c`).

## 2. What Linux does

- **The receive RTT** (`tcp_rcv_rtt_measure()`, `tcp_rcv_rtt_measure_ts()`, `tcp_rcv_rtt_update()`). Without timestamps, the receiver measures once per window: the time until `rcv_nxt` passes the right edge of the window at the start of the measurement, and it keeps the minimum. With timestamps, each segment of at least one MSS with a new TSecr gives a sample (now minus TSecr), smoothed by 1/8. The estimate is in microseconds, times 8.
- **The receive space** (`tcp_init_buffer_space()`): at the end of the handshake, the space is min(`rcv_ssthresh`, `rcv_wnd`, 10 × advmss), with the time and the read sequence number of that moment.
- **DRS** (`tcp_rcv_space_adjust()`, called at each read: `tcp_recvmsg()`, `tcp_read_sock()` and others). Once a receive RTT has passed since the last measurement, `copied` is the bytes that the application read since then, less the unread bytes; if it is larger than the space, `tcp_rcvbuf_grow(copied)` runs. The measurement starts again in each case.
- **The growth** (`tcp_rcvbuf_grow(newval)`): the space becomes `newval`; the window is twice the space; with a receive RTT below `tcp_rcvbuf_low_rtt` (1 ms by default), the window grows by the window × RTT / 1 ms, else by twice the window × the relative growth of the space (slow start); the out-of-order span adds to it; the buffer becomes the space of that window, at most `tcp_rmem[2]`, and the window clamp follows. A buffer that `SO_RCVBUF` locked does not grow.
- **The bound** `tcp_rmem[2]`: max(128 KiB, min(32 MiB, a share of the memory)), so 32 MiB on a usual host.

## 3. The steps

| Step | Commit | Test |
| --- | --- | --- |
| 1 | `plan: add: the receive space of the Linux mode` | — |
| 2 | `tcp: add: the receive RTT of Linux` | a module test of the estimate, with and without timestamps (through the status query) |
| 3 | `tcp: refactor: one growth of the receive buffer, as tcp_rcvbuf_grow()` | the out-of-order growth stays as it is (the tests of D-114) |
| 4 | `tcp: add: Dynamic Right-Sizing in the Linux mode` | a long transfer with a `LinuxTcp` receiver: the offered window per RTT, computed by hand from the code of Linux |
| 5 | `tcp: add: the bound of the receive buffer` | a growth that stops at the bound |
| 6 | `doc: change: the release note of the receive space` | — |
| 7 | `plan: change: record the receive space of the Linux mode` | — |

The collapse of `tcp_prune_queue()`, and the pad and the slab classes of a small packet, become items of their own (section 5, Q4).

## 4. The checks

Each step as before: each commit builds and passes the module tests, a new test fails before its commit, the CI fingerprint job and the TCP statistics with the attribution of each moved row, the packetdrill suite of inet-gpl (its `tcp_rcvbuf_*` scripts must keep their verdicts), and the gates.

## 5. Questions for the owner

- **Q1 — The read of the application.** With `autoRead`, INET gives the data to the application when it arrives, so each arrival is a read for DRS; in the explicit-read mode, each READ is one. Is that the model you want?
- **Q2 — `tcp_rcvbuf_low_rtt`.** A parameter (`receiveBufferLowRtt`, 1 ms as Linux), or the fixed value of Linux?
- **Q3 — The bound.** A parameter `receiveBufferMax` (the bound of `tcp_rmem[2]`): 32 MiB in `LinuxTcp`; and in `Tcp`, where auto-tuning is off by default, the same value or no bound?
- **Q4 — The other two parts.** The collapse of `tcp_prune_queue()` and the slab classes of the truesize: separate items after T6, or leave them out?
- **Q5 — The receive RTT in `TCP_INFO`.** Should inet-gpl's `PacketDrillApp` also report `tcpi_rcv_rtt` and `tcpi_rcv_space`, so that scripts can check them later?
