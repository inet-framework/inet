# Repair the TCP defects that the packetdrill suite found on master

Status: **in progress** — the plan.

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
| 1 | `plan: add: repair the TCP defects that the packetdrill suite found` | — | — |
| 2 | `tcp: fix: the SACK blocks of an ACK below SND.UNA` | `tcp_sack_old_ack_1` | A |
| 3 | `tcp: fix: the SACK recovery of a Fast Open server in SYN_RCVD` | `tcp_fastopen_server_sack_1` | B |
| 4 | `tcp: fix: the window offer of windowAutoTuning, as Linux` | `tcp_rcvbuf_autotune_2` | C |
| 5 | `tcp: fix: NewReno retransmits a loss of the first segment fast` | `tcp_newreno_first_segment_1` | D |
| 6 | `tcp: fix: TCP_SYNCNT also after the open` | `tcp_syn_retries_2` | E |
| 7 | `tcp: fix: TCP_MAXSEG of a listening socket limits the MSS option` | `tcp_maxseg_listen_1` | F |
| 8 | `plan: change: record the repairs` | — | — |
| 9 | `plan: location: the repairs, done` | — | — |

The names of the tests can change when a name is already taken.

## 3. The evidence

As for a stage of the staged landing: each new test fails at the commit before its own; each
commit builds alone with no undefined `inet::` symbol; the module tests and the TCP protocol tests
pass at each commit; unit and serializer tests at the head; the CI fingerprint job at each code
commit (`ghci-series.sh`); the TCP fingerprint rows in debug at the head; the six gates and the
combined pull-request gate. And the packetdrill suite of inet-gpl, built against the head with
`LinuxTcp`, has no divergence of A to F.
