# IPsec tunnel mode

**Status:** not started. Written 2026-09-25 on `topic/standards-tests-ipsec-level2-fixes`, as the
plan for gap 8 of the IPsec results; the user decided that large missing features get a plan and
no code in the repair branches.

## Goal

The model protects in transport mode only ("Transport mode only. Tunnel mode is not supported",
`IPsec.ned`). RFC 4301 §4.1 asks a host for both modes (RFC4301-SA-50) and a security gateway for
tunnel mode (SA-51). The tests that show it are `Rfc4301TunnelModeEsp` and `Rfc4301TunnelModeAh` of
`tests/protocol/ipsec`, which declare an expected FAIL today.

## Why it is large

- **A second IP header.** In tunnel mode the sender puts the whole datagram inside AH or ESP,
  behind a new outer header with the addresses of the tunnel endpoints (RFC 4301 §5.1.2). The
  outer header needs its own fields: TTL, DSCP and ECN copied or set by the rules of §5.1.2.1, the
  DF bit, and for IPv6 the flow label. The IP layer must route the outer datagram, which means a
  second pass through routing for a datagram that the post-routing hook already holds.
- **A gateway.** A security gateway protects traffic that it forwards, so the hooks of the
  forwarding path take part, and the SPD of a gateway sees both sides.
- **The receiver.** After AH or ESP processing, the inner datagram goes back into the IP layer as
  if it had arrived: to local delivery, or to forwarding at a gateway, with the selector check of
  gap 5 on the inner header.
- **Fragmentation and the path MTU** of the outer datagram meet the plan `ipv6-path-mtu.md`.

## Steps

1. [ ] **Configuration** — a `Mode` element (TRANSPORT, TUNNEL) and the tunnel endpoint addresses
   (`TunnelLocalAddress`, `TunnelRemoteAddress`) of an SA.
2. [ ] **Host to host** — the outer header on egress, IPv4 and IPv6, and the inner datagram on
   ingress; `Rfc4301TunnelModeEsp` and `Rfc4301TunnelModeAh` pass, and their declarations go.
3. [ ] **The outer header fields** of RFC 4301 §5.1.2.1, with checks of level 3.
4. [ ] **A security gateway** — protection of forwarded traffic, with a mockup of two gateways.
5. [ ] **Baselines** — the fingerprints and statistical results of `examples/inet/ipsec`, and new
   examples of tunnel mode.
