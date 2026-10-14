# Bring the receive memory of a Linux-mode receiver to Linux 7.1

Status: **in progress** — the plan.

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
| 1 | `plan: add: the receive memory of a Linux-mode receiver` | — | — |
| 2 | `tcp: fix: out-of-order data grow the receive buffer, as Linux` | `tcp_rcvbuf_grow_1` (changes) | 3 |
| 3 | `tcp: fix: the truesize of a large segment, as tun` | `tcp_rcvbuf_truesize_1` | 1 |
| 4 | `tcp: fix: a locked buffer accepts by its memory` | `tcp_rcvbuf_lock_3` | 2 |
| 5 | `plan: change: record the receive memory repairs` | — | — |
| 6 | `plan: location: the receive memory repairs, done` | — | — |

## 3. The evidence

As for the packetdrill repairs: each new test fails at the commit before its own; each commit
builds alone; the module tests and the TCP protocol tests pass at each commit; unit and serializer
at the head; the CI fingerprint job at each code commit; the six gates and the combined
pull-request gate; and the packetdrill suite against each code commit (the INET wrappers too).
