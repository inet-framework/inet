# Repair the TCP defects that the packetdrill suite found on master

Status: **done** — landed on master on 2026-10-14.

- Branch `topic/tcp-packetdrill-repairs`, on master `365c3941f7`.
- The decision numbers continue those of `plan/done/tcp-staged-landing.md` (D-108 onward).

## 1. The problem

inet-gpl runs the packetdrill suite against INET: 318 scripts of Linux and of the packetdrill
project. Against the TCP branch `topic/tcp-new-audit-fixes`, the suite had 303 MATCH, 12
KERNEL_DRIFT and 3 UNSUPPORTED. After the staged landing of that branch, inet-gpl moves to master
and to `LinuxTcp` (inet-gpl `plan/pending/packetdrill-inet-master.md`). On master, 8 scripts
diverge. Each one also diverges with the old explicit Linux parameters of inet-gpl, so none comes
from `LinuxTcp`. Each is a defect of master, and five of them are also gaps of the RFCs or of the
socket options that master claims:

| Cause | Scripts | What master does | What the RFC or Linux asks |
| --- | --- | --- | --- |
| A | `prr-ss-ack-below-snd_una-cubic` (both sets) | ignores the SACK blocks of an ACK below SND.UNA | RFC 6675 section 2 counts an ACK with new SACK data as a duplicate ACK; Linux processes it (`old_ack` in `tcp_ack()`) |
| B | `tcp_fastopen_server_client-ack-dropped-then-recovery-ms-timestamps` | a Fast Open server drops the SACK blocks of the ACK that completes the handshake | the recovery exists when the data goes, in SYN_RCVD (Linux: the child socket is complete) |
| C | `tcp_rcv_wnd_shrink_nomem` | `windowAutoTuning` offers the auto-tuned window less the unread data | Linux offers the free buffer at the scaling ratio, at most `rcv_ssthresh` (`__tcp_select_window()`) |
| D | `fr-4pkt` | NewReno without SACK does not retransmit a loss of the first data segment fast | Linux and FreeBSD do; RFC 6582 starts `recover` at the ISS, which an ACK of the SYN alone does not pass |
| E | `nonblocking-connect-bypass-errno`, `nonblocking-sendto-errnos` | TCP_SYNCNT after the open has no effect | Linux reads `icsk_syn_retries` at each timeout |
| F | `mss-setsockopt-tcp_maxseg-server` | TCP_MAXSEG on a listening socket does not limit the MSS option of the SYN-ACK | Linux limits the MSS option of each connection of the listener by `user_mss` |

The TCP branch passed A only because its PRR counted SACKed bytes twice (D-28), and D only because
its `recover` started at 0. It had B, and it had E through the run-time parameter that D-66
replaced. F occurs only with an explicit `mss` parameter.

## 2. The steps

| Step | Commit | Test | Cause |
| --- | --- | --- | --- |
| 1 ✅ | `plan: add: repair the TCP defects that the packetdrill suite found` | — | — |
| 2 ✅ | `tcp: fix: the SACK blocks of an ACK below SND.UNA` | `tcp_sack_old_ack_1` | A (D-108) |
| 3 ✅ | `tcp: fix: the SACK recovery of a Fast Open server in SYN_RCVD` | `tcp_fastopen_server_sack_1` | B (D-109) |
| 4 ✅ | `tcp: fix: the offer of a locked buffer counts unread data once` | `tcp_rcvbuf_autotune_2` | C (D-110) |
| 5 ✅ | `tcp: fix: NewReno retransmits a loss of the first segment fast` | `tcp_newreno_first_segment_1` | D (D-111) |
| 6 ✅ | `tcp: fix: TCP_SYNCNT also after the open` | `tcp_socket_synretries_2` | E (D-112) |
| 7 ✅ | `tcp: fix: TCP_MAXSEG of a listening socket limits the MSS option` | `tcp_maxseg_listen_1` | F (D-113) |
| 8 ✅ | `plan: change: record the repairs` | — | — |
| 9 ✅ | `plan: location: the repairs, done` | — | — |

Step 6 also gives the test client `synRetriesScript`, which sets TCP_SYNCNT on an open socket.

## 3. The evidence

As for a stage of the staged landing: each new test fails at the commit before its own; each
commit builds alone with no undefined `inet::` symbol; the module tests and the TCP protocol tests
pass at each commit; unit and serializer tests at the head; the CI fingerprint job at each code
commit (`ghci-series.sh`); the TCP fingerprint rows in debug at the head; the six gates and the
combined pull-request gate. And the packetdrill suite of inet-gpl, built against the head with
`LinuxTcp`, has no divergence of A to F.

## 4. The decisions

- **D-108 — A late ACK with new SACK data is a duplicate ACK.** RFC 6675 section 2 counts each
  ACK that SACKs new data as a duplicate ACK, also one that moves SND.UNA. Master keeps the
  duplicate ACK of RFC 5681 at SND.UNA, and only an ACK below SND.UNA counts by the SACKed bytes
  of the segment (`prrNewlyDelivered()`, which D-28 made exact). Linux processes the SACK blocks of
  such an ACK on its `old_ack` path. With timestamps, PAWS drops a late ACK first (RFC 7323
  section 5.3), as in Linux, so the test runs without them, as the packetdrill script does.
- **D-109 — A Fast Open server starts its algorithm after the SYN-ACK.** The algorithm chooses
  its recovery when it starts, and SACK is settled when the options of the SYN-ACK are written.
  The start stays before the application can send in SYN_RCVD (RFC 7413 section 4.2.2), because
  the indications of the SYN reach the application later. The TCP branch created the recovery
  at its first use; the staged landing did not bring that over, so each Fast Open server
  connection lost its SACK recovery.
- **D-110 — Only a locked buffer counts the unread data once.** With a buffer that SO_RCVBUF
  locked, the free space at the payload ratio already counts the unread data, at their truesize,
  so the auto-tuned window no longer counts them a second time, as Linux
  `__tcp_select_window()`. An unlocked buffer keeps the limit of master (D-61): it grows without
  the bound of `tcp_rmem` (`tcp_clamp_window()`), and the auto-tuned window less the unread data
  keeps the offer near Linux, which closes the window when the application does not read. The
  free space at the ratio for an unlocked buffer too breaks `tcp_ooo-before-and-after-accept`,
  because the truesize model of master is not that of Linux for large segments (section 5).
- **D-111 — NewReno's `recover` starts one below the ISS.** RFC 6582 step 1 starts it at the ISS,
  and step 2 then refuses a fast retransmit of a lost first segment, because the duplicate ACKs
  cover only the SYN. FreeBSD keeps `snd_recover` as the next sequence number and starts it at the
  ISS, which is one below the ISS in the terms of RFC 6582; Linux has no such check outside a
  recovery. `TcpNewReno::ackProcessed()` already gives `snd_una - 2` after each ACK, so only the
  first window changes. The check of step 2 stays strict, which keeps `tcp_newreno_rto_1`.
- **D-112 — TCP_SYNCNT acts on an open socket.** `TcpSocket::setSynRetries()` keeps the value for
  the open command (D-66) and, on an open socket, sends a SETOPTION command, as
  `setReceiveBufferSize()` does. The connection uses the value from the next timeout on, as Linux
  reads `icsk_syn_retries` at each timeout, and a listener passes it to the connections that it
  forks.
- **D-113 — TCP_MAXSEG in LISTEN limits the MSS option.** D-75 limited the MSS option only before
  the open. A listening socket is open, but its connections come later; Linux limits the MSS
  option of each of them by `user_mss`. With `mss = -1` the SYN in LISTEN already took TCP_MAXSEG
  (D-99), so the defect needed an explicit `mss`.

## 5. Open findings

- **G — Limited Transmit retransmits with `lossDetectionMode = "dupthresh"`.** At the first
  duplicate ACK with SACK, step (3.3) of RFC 6675 sends up to one segment of new data. Master
  takes the segment from `NextSeg()`, whose rule 3 returns an old unSACKed segment when no new data
  waits, so it retransmits the first hole at the first duplicate ACK, and again when the recovery
  starts (a D-SACK follows). RACK, the default, restricts the step to new data. `LegacyTcp` uses
  `dupthresh`, so a repair moves the dumps of module tests; a branch of its own.
- **H — The receive memory of a locked buffer.** After repair C, `tcp_rcv_wnd_shrink_nomem` gets
  to its last step: Linux accepts a second 20000-byte segment into a locked buffer of 32000 bytes
  that holds about 23000 bytes, and then offers a zero window. Master refuses it by its byte check
  (beyond the first unread byte plus the buffer) and by its truesize of 33000 bytes for a
  20000-byte segment (one page of order 3). A truesize as `alloc_skb_with_frags()` gives, with
  memory alone deciding for a locked buffer, repairs the script but breaks
  `tcp_ooo-before-and-after-accept`, which needs the larger truesize. The model needs a closer
  study of the Linux code (the tun allocation, `tcp_try_coalesce()`, `tcp_can_ingest()`). The TCP
  branch passed the script only because it grew a buffer that SO_RCVBUF had locked.

## 6. The record — landed 2026-10-14

The owner reviewed and approved the repairs on 2026-10-14.

Evidence, debug build against `omnetpp-6.x`: each commit builds alone with no undefined `inet::`
symbol, and the module tests pass at each commit (628 at the plan, then one more with each repair,
634 at the head). Each new test fails at the commit before its own: the late ACKs change nothing and
the RTO repairs the hole at t=2.009 (A); the server drops the SACK options and repairs the second
hole at the RTO, t=1.512 s (B); the server takes 100000 bytes (C); the first segment goes again at
the RTO, t=2.001 (D); the client sends its SYN at t=3.001 and t=7.001 and does not fail (E, with the
test client of the head); the SYN-ACK announces 1000 (F). The TCP standards tests pass at each
commit (26 pass and 1 expected failure; the packetdrill tests skip without inet-gpl); unit 126 and
serializer 4 pass at the head. The CI fingerprint job runs at each code commit, no row moves, and
the old error of `ethernet.csv` stays. The TCP fingerprint rows run in debug at the head with no
runtime error outside the five lwIP rows, and the `tyf` ingredient differs in the same 69 rows as
after S20.

The packetdrill suite of inet-gpl (branch `topic/packetdrill-inet-master`, `tcp.ini` with `LinuxTcp`
and the window only), built against the head: 302 MATCH, 12 KERNEL_DRIFT, 3 UNSUPPORTED_FEATURE and
1 DIVERGENCE, where master gives 296 MATCH and 7 DIVERGENCE; the one left is finding H. The
packetdrill wrappers of INET (`tests/protocol/tcp`, with `INETGPL_ROOT`) give 328 PASS, 4 expected
failures (the 3 scripts that need a feature INET does not have, and `Rfc9293ChecksumDefault`) and 1
unexpected failure, `tcp_rcv_wnd_shrink_nomem` (H). The suite also confirms two values of `LinuxTcp`
that S19b marked "verify": with the value of `Tcp`, `abortOnDataAfterClose` fails 2 user_timeout
scripts and `accEcnOptionEnabled` fails 28 AccECN scripts; no script reaches `maxPersistTimeout`.
