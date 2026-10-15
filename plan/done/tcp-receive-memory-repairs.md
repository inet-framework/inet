# Bring the receive memory of a Linux-mode receiver to Linux 7.1

Status: **done** — landed on master on 2026-10-15.

- Branch `topic/tcp-receive-memory-repairs`, on master `2424a1a865`.
- The decision numbers continue those of `plan/done/tcp-packetdrill-repairs.md` (D-114 onward).
- The packetdrill suite of inet-gpl is the arbiter; the Linux source is that of the commit that
  the suite pins (`d96fcfe1b7f9`).

## 1. The problem

The packetdrill suite of inet-gpl on master has 302 MATCH and one divergence,
`tcp_rcv_wnd_shrink_nomem` (finding H of `plan/done/tcp-packetdrill-repairs.md`). Its last step
sends two 20000-byte segments into a buffer that SO_RCVBUF locked at 32000 bytes. Linux accepts
the second one, because a socket takes a segment while its memory is within the buffer before the
segment (`tcp_can_ingest()`), and then offers a zero window. Master refuses it for two reasons,
and each one is a gap of the Linux model that `windowAutoTuning` and SO_RCVBUF claim:

| Gap | Master | Linux 7.1 |
| --- | --- | --- |
| 1 | a segment above 16 KB costs pages of 32 KB (33000 bytes for 20000 bytes) | tun takes a head of 4096 bytes and pages of the largest order that the rest fills (`tun_alloc_skb()`, `alloc_skb_with_frags()`): 20712 bytes |
| 2 | a locked buffer also refuses data beyond its first unread byte plus its size | memory alone decides (`tcp_try_rmem_schedule()`) |
| 3 | an owned connection grows its buffer only when its memory outgrows it (`tcp_clamp_window()`) | each out-of-order segment also grows it to the window of twice `rcvq_space` plus the out-of-order span (`tcp_rcvbuf_grow()` in `tcp_data_queue_ofo()`) |

Gap 3 must come first: `tcp_ooo-before-and-after-accept` expects a buffer above 131072 bytes
after 99000 bytes of out-of-order data, and master reaches it only through the large truesize of
gap 1. An experiment with all three repairs gives 303 MATCH and no divergence.

## 2. The steps

| Step | Commit | Test | Gap |
| --- | --- | --- | --- |
| 1 ✅ | `plan: add: the receive memory of a Linux-mode receiver` | — | — |
| 2 ✅ | `tcp: fix: out-of-order data grow the receive buffer, as Linux` | `tcp_rcvbuf_grow_1` (changes) | 3 (D-114) |
| 3 ✅ | `tcp: fix: the truesize of a large segment, as tun` | `tcp_rcvbuf_truesize_1` | 1 (D-115) |
| 4 ✅ | `tcp: fix: a locked buffer accepts by its memory` | `tcp_rcvbuf_lock_3` | 2 (D-116) |
| 5 ✅ | `plan: change: record the receive memory repairs` | — | — |
| 6 ✅ | `plan: location: the receive memory repairs, done` | — | — |

## 3. The evidence

As for the packetdrill repairs: each new test fails at the commit before its own; each commit
builds alone; the module tests and the TCP protocol tests pass at each commit; unit and serializer
at the head; the CI fingerprint job at each code commit; the six gates and the combined
pull-request gate; and the packetdrill suite against each code commit (the INET wrappers too).

## 4. The decisions

- **D-114 — Out-of-order data grow the buffer as `tcp_rcvbuf_grow()`.** At each out-of-order
  segment, an accepted connection with `windowAutoTuning` and an unlocked buffer grows the buffer
  to the window of twice the receive space and the out-of-order span, at the payload ratio, and
  the clamp of the offer follows (`tcp_win_from_space()`). The receive space is the first offer,
  at most 10 MSS (`tcp_init_buffer_space()`). INET has no DRS (`tcp_rcv_space_adjust()`), which
  grows the space when the application reads; Linux does not grow it either before it measures
  the receive RTT, and the extra growth of a measured RTT is 0 then. There is no upper bound
  (`tcp_rmem[2]`), as for the growth at memory overflow (`tcp_clamp_window()`), which stays.
  `tcp_rcvbuf_grow_1`, written for master's model, changes: the buffer grows at the first
  out-of-order segment, not only when its memory overflows.
- **D-115 — The truesize of a segment follows the tun path of Linux.** A packet that does not fit
  one page with the pad of 64 bytes gets a linear page and the pages of
  `alloc_skb_with_frags()`, of the largest order up to 3 that the rest fills. A smaller packet
  keeps the power-of-two charge of master, with its minimum of 1024 bytes and without the pad:
  the module tests and the packetdrill scripts pin those values, and the pad moves only a few
  sizes at the edge of a class. The charge changes for payloads of 7769 bytes and more.
- **D-116 — A locked buffer counts memory, not bytes.** Linux takes a segment while the memory is
  within the buffer (`tcp_can_ingest()`), so the segment that first crosses the buffer gets in,
  and the check of the memory that S18 added refuses the next one. A buffer that is not locked
  keeps its limit in bytes (D-61); in the Linux mode it grows when its memory overflows.

## 5. Open findings

- The receive memory still differs from Linux where the suite does not reach: DRS, the bound of
  `tcp_rmem[2]`, the collapse of `tcp_prune_queue()`, and the pad and the slab classes of a small
  packet.
- **G** of `plan/done/tcp-packetdrill-repairs.md` (Limited Transmit in the `dupthresh` mode) stays
  open.

## 6. The record — landed 2026-10-15

The owner reviewed and approved the receive memory repairs on 2026-10-15.

Evidence, debug build against `omnetpp-6.x`: each commit builds alone with no undefined `inet::`
symbol, and the module tests pass at each commit (635 at the plan and after step 2, 636, 637 at
the head). Each test fails at the commit before its own: `tcp_rcvbuf_grow_1` reports 18240 bytes
(step 2); the locked buffer of `tcp_rcvbuf_truesize_1` takes 60585 bytes (step 3); the server of
`tcp_rcvbuf_lock_3` drops the second segment (step 4). The TCP standards tests pass at each
commit (26 pass and 1 expected failure; the packetdrill tests skip without inet-gpl); unit 126
and serializer 4 pass at the head. The CI fingerprint job runs at each code commit, no row moves,
and the old error of `ethernet.csv` stays. The TCP fingerprint rows run in debug at the head with
no runtime error outside the five lwIP rows, and the `tyf` ingredient differs in the same 69 rows
as after S20.

The packetdrill suite of inet-gpl master (`7c121e2`, `LinuxTcp`): 302 MATCH and 1 DIVERGENCE
after step 2 and after step 3, as on master; 303 MATCH, 12 KERNEL_DRIFT, 3 UNSUPPORTED_FEATURE
and no DIVERGENCE at the head, as on the TCP branch. The packetdrill wrappers of INET
(`tests/protocol/tcp`, with `INETGPL_ROOT`) give 333 tests at the head: 329 PASS and 4 expected
failures, no unexpected failure.
