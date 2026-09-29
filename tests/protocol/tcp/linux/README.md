# The TCP packetdrill scripts of the Linux kernel's selftests as protocol tests

Each `.test` file in this folder is a wrapper for one packetdrill script of inet-gpl: the
script with the same path below `tests/protocol/`. For example, this wrapper:

    tcp/linux/tcp_accecn_2nd_data_as_first.test

runs this script of inet-gpl:

    tests/protocol/tcp/linux/tcp_accecn_2nd_data_as_first.pkt

inet-gpl copies the scripts from https://github.com/torvalds/linux.git at
`d96fcfe1b7f9`, folder `tools/testing/selftests/net/packetdrill/`.

A wrapper gives the script's id to `inet_run_packetdrill`, which runs the script in INET's
`PacketDrillApp` through inet-gpl's `tests/packetdrill/suite.py inet-one`. The wrapper
passes when the output ends with `PACKETDRILL <id>: PASS`.

A pass is **Linux equivalence**, not conformance to an RFC. Linux 7.1.3 passes every
script that has a wrapper, and a pass says that INET does the same.

The scripts, the packetdrill parser and the packet comparison are GPL licensed and stay in
inet-gpl. These files hold no line of a script. Without inet-gpl, every wrapper gives SKIP:

```sh
cd <inet> && source setenv -q && cd <inet-gpl> && source setenv -q
inet_run_protocol_tests --filter tcp/linux/
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
| TCP-F-CONGESTION-WINDOW | 10 |
| TCP-F-TERMINATE | 7 |
| TCP-F-SEGMENT-ACCEPTANCE | 6 |
| TCP-F-FLOW-CONTROL | 5 |
| TCP-F-DATA-TRANSFER | 4 |
| TCP-F-FAST-RETRANSMIT | 4 |
| TCP-F-ACKNOWLEDGE | 3 |
| TCP-F-ESTABLISH | 2 |
| TCP-F-RESTART-IDLE | 1 |
| TCP-F-RTO-BACKOFF | 1 |
| TCP-F-RTO-BOUNDS | 1 |

| Document outside INET's TCP catalogs | Wrappers |
| --- | --- |
| RFC 9768 | 58 |
| RFC 7413 | 18 |
| RFC 2018 | 4 |
| RFC 6675 | 4 |
| RFC 6937 | 4 |
| RFC 7323 | 3 |
| RFC 9293 section 3.7.4 (Nagle), outside the feature map | 3 |
| RFC 3042 | 2 |
| RFC 5482 | 2 |
| RFC 2385 | 1 |
| RFC 2883 | 1 |
| RFC 3168 | 1 |
| RFC 4987 | 1 |
| RFC 5961 | 1 |

| Linux socket interface, no standard | Wrappers |
| --- | --- |
| MSG_ZEROCOPY | 11 |
| MSG_EOR | 4 |
| SO_TIMESTAMPING | 4 |
| blocking socket calls | 4 |
| TCP_INFO | 3 |
| system call argument checks | 3 |
| TCP_INQ | 2 |
| TCP_USER_TIMEOUT | 2 |
| sendfile | 1 |
| splice | 1 |

## Counts

- wrappers: **158**, of which 1 expect `FAIL` because INET
  cannot run the script
- scripts without a wrapper: **1**, listed below

## Scripts without a wrapper

A script that no longer passes on Linux (kernel drift) states no Linux behavior, so a
wrapper for it would prove nothing. The suite does not run a skipped script.

| Script | Why | Detail |
| --- | --- | --- |
| `tcp/linux/tcp_syncookies_ip6_9k` | skipped | forces --ip_version=ipv6, out of IPv4-only scope |
