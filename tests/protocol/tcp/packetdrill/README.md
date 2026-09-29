# The TCP packetdrill scripts of the packetdrill repository as protocol tests

Each `.test` file in this folder is a wrapper for one packetdrill script of inet-gpl: the
script with the same path below `tests/protocol/`. For example, this wrapper:

    tcp/packetdrill/blocking/blocking-accept.test

runs this script of inet-gpl:

    tests/protocol/tcp/packetdrill/blocking/blocking-accept.pkt

inet-gpl copies the scripts from https://github.com/google/packetdrill.git at
`2c4001c4d6fc`, folder `gtests/net/tcp/`.

A wrapper gives the script's id to `inet_run_packetdrill`, which runs the script in INET's
`PacketDrillApp` through inet-gpl's `tests/packetdrill/suite.py inet-one`. The wrapper
passes when the output ends with `PACKETDRILL <id>: PASS`.

A pass is **Linux equivalence**, not conformance to an RFC. Linux 7.1.3 passes every
script that has a wrapper, and a pass says that INET does the same.

The scripts, the packetdrill parser and the packet comparison are GPL licensed and stay in
inet-gpl. These files hold no line of a script. Without inet-gpl, every wrapper gives SKIP:

```sh
cd <inet> && source setenv -q && cd <inet-gpl> && source setenv -q
inet_run_protocol_tests --filter tcp/packetdrill/
```

`tests/packetdrill/suite.py gen-wrappers` in inet-gpl writes the wrappers and this file, and
`suite.py gen-wrappers --check` shows the files that are out of date. Do not edit them by hand.

## What the wrappers exercise

The `Features:` line of a wrapper names ids of INET's TCP feature map
(`doc/project/evidence/protocol/tcp/features.md`). If a script is outside the map, its
wrapper names the document that INET's TCP evidence does not catalog, or the Linux
socket interface that no standard defines. The map is
`tests/packetdrill/tcp/features.yaml` in inet-gpl. One script can count in more than
one row.

| TCP feature | Wrappers |
| --- | --- |
| TCP-F-CONGESTION-WINDOW | 11 |
| TCP-F-FAST-RETRANSMIT | 8 |
| TCP-F-TERMINATE | 8 |
| TCP-F-DATA-TRANSFER | 3 |
| TCP-F-RESTART-IDLE | 1 |
| TCP-F-SEGMENT-ACCEPTANCE | 1 |

| Document outside INET's TCP catalogs | Wrappers |
| --- | --- |
| RFC 7413 | 62 |
| RFC 2018 | 4 |
| RFC 6675 | 4 |
| RFC 6937 | 4 |
| RFC 9438 | 4 |
| RFC 9293 section 3.7.4 (Nagle), outside the feature map | 3 |
| RFC 3042 | 2 |
| RFC 5482 | 2 |
| RFC 7323 | 2 |
| RFC 2385 | 1 |
| RFC 3168 | 1 |
| RFC 4821 | 1 |

| Linux socket interface, no standard | Wrappers |
| --- | --- |
| MSG_ZEROCOPY | 10 |
| MSG_EOR | 4 |
| blocking socket calls | 4 |
| SO_TIMESTAMPING | 3 |
| TCP_INFO | 3 |
| TCP_MAXSEG | 3 |
| system call argument checks | 3 |
| TCP_INQ | 2 |
| TCP_NOTSENT_LOWAT | 2 |
| TCP_USER_TIMEOUT | 2 |
| cwnd moderation | 1 |
| epoll | 1 |
| ioctl | 1 |
| sendfile | 1 |
| splice | 1 |

## Counts

- wrappers: **148**, of which 2 expect `FAIL` because INET
  cannot run the script
- scripts without a wrapper: **14**, listed below

## Scripts without a wrapper

A script that no longer passes on Linux (kernel drift) states no Linux behavior, so a
wrapper for it would prove nothing. The suite does not run a skipped script.

| Script | Why | Detail |
| --- | --- | --- |
| `tcp/packetdrill/cubic/cubic-bulk-166k-idle-restart` | kernel drift | as cubic/cubic-bulk-166k (cwnd 36 vs the asserted 40) |
| `tcp/packetdrill/cubic/cubic-bulk-166k` | kernel drift | CUBIC slow start reaches cwnd 36 where the script asserts 40: Linux only grows cwnd while the sender is cwnd-limited, and under fq pacing at 166 kbit/s it is pacing-limited for part of the run. Script and kernel disagree about the growth schedule, not about CUBIC. |
| `tcp/packetdrill/cwnd_moderation/cwnd-moderation-ecn-enter-cwr-no-moderation-700` | kernel drift | asserts tcpi_snd_cwnd == 4 after ECN-triggered CWR; this kernel keeps 5 |
| `tcp/packetdrill/epoll/epoll_out_edge` | kernel drift | expects a 16000-byte outbound burst, the kernel emits 20000 in one skb. The data QUEUED is right -- forcing tun offloads off yields exactly 16000 -- but then the PSH bit differs, so this is kernel packetization, not sndbuf accounting. tcp_wmem='4096 50000 4194304' was verified to land inside the netns. |
| `tcp/packetdrill/epoll/epoll_out_edge_default_notsent_lowat` | kernel drift | as epoll/epoll_out_edge (16000 expected vs 20000 emitted) |
| `tcp/packetdrill/epoll/epoll_out_edge_notsent_lowat` | kernel drift | as epoll/epoll_out_edge (16000 expected vs 20000 emitted) |
| `tcp/packetdrill/fastopen/client/syn-data-icmp-unreach-frag-needed-with-seq-ipv6` | skipped | uses ICMPv6 'packet_too_big', out of IPv4-only scope (icmp_type_from_word only maps ICMPv4 'unreachable'); was masquerading as INET_DIVERGE via semantic_error()'s generic 'Script error', not a real behavioral divergence |
| `tcp/packetdrill/fastopen/server/client-ack-dropped-then-recovery-ms-timestamps` | kernel drift | reproducer for upstream 794200d66273 ('tcp: undo cwnd on Fast Open spurious SYNACK retransmit'); the advertised window comes back 1053 where the script pins 256 |
| `tcp/packetdrill/gro/gro-mss-option` | kernel drift | the injected data segment carries the ACK flag but no `ack N` field, so packetdrill sends ack_seq=0, remapped to the server's ISN (snd_una-1); this kernel drops a data segment bearing that stale ACK, so nothing is ever received and it ACKs 1 instead of 10001. Adding `ack 1` makes the script pass -- but that means editing a copied upstream script, which would break the sha256 check of linux-results.csv. The only script with this pattern. |
| `tcp/packetdrill/mtu_probe/basic-v6` | skipped | forces --ip_version=ipv6, out of IPv4-only scope |
| `tcp/packetdrill/notsent_lowat/notsent-lowat-default` | kernel drift | as epoll/epoll_out_edge (16000 expected vs 20000 emitted) |
| `tcp/packetdrill/shutdown/shutdown-rdwr-write-queue-close` | kernel drift | expects no payload on the segment after shutdown(SHUT_RDWR) with unacknowledged data queued; this kernel still sends 1000 bytes |
| `tcp/packetdrill/ts_recent/invalid_ack` | kernel drift | same family as gro-mss-option: the script expects a segment with an out-of-window ACK to still update ts_recent/advance the connection; this kernel rejects it, so the reply acks 1 rather than 1001 |
| `tcp/packetdrill/zerocopy/maxfrags` | kernel drift | 'tcp_MAX_SKB_FRAGS test' -- the split into 17 frags + 1 is correct here (CONFIG_MAX_SKB_FRAGS=17 matches what the script assumes), but the final 1-byte segment's payload content differs. Not a build-config mismatch. |
