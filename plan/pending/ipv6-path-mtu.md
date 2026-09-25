# The path MTU in IPv6, and in IPsec

**Status:** not started. Written 2026-09-25 on `topic/standards-tests-ipsec-level2-fixes`, as the
plan for gap 12 of the IPsec results; the user decided that large missing features get a plan and
no code in the repair branches.

## Goal

No code in the model keeps a path MTU. `Icmpv6` turns a Packet Too Big message into an indication
for the protocol that the quoted packet names, and nothing reads its MTU, so a sender goes on with
packets that a router cannot forward. RFC 4301 §8 asks IPsec to keep the path MTU of an SA
(RFC4301-OUT-21, RFC4302-FRAG-10, RFC4303-FRAG-9), and RFC 8201 asks every IPv6 node for Path MTU
Discovery. The test that shows it is `Rfc4301PathMtuIpv6` of `tests/protocol/ipsec`, which
declares an expected FAIL today.

## Why it is large

- **IPv6 first.** A path MTU cache belongs to the IPv6 layer, keyed by destination (RFC 8201 §5),
  with its aging (§5.3), and the sender must fragment to it at the source, because IPv6 routers
  do not fragment. That is a change to `Ipv6` and `Icmpv6` for every IPv6 flow, not only IPsec.
- **Then IPsec.** RFC 4301 §8.2 keeps the path MTU per SA, and the length that AH or ESP adds must
  be taken off it before the MTU goes to the transport layer.
- **TCP.** The MSS of a TCP connection follows the path MTU, which touches the TCP model.
- **IPv4** has the same need for packets with the DF bit (RFC 1191); it can come later.

## Steps

1. [ ] **The IPv6 path MTU cache** — the Packet Too Big message sets the MTU of a destination;
   `Ipv6` fragments locally generated datagrams to it; aging after 10 minutes.
2. [ ] **IPsec** — the path MTU of an SA, less the IPsec overhead, for the packets of its flows;
   `Rfc4301PathMtuIpv6` passes, and its declaration goes.
3. [ ] **Tests of RFC 8201** in `tests/protocol/ipv6`.
4. [ ] **TCP** — the MSS from the path MTU.
5. [ ] **Baselines** — every IPv6 fingerprint and statistical result that moves.
