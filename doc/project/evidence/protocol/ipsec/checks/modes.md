# IPsec — English check procedures: modes and combined SAs

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc4301/catalog.md](../../../standard/rfc4301/catalog.md), [rfc4302/catalog.md](../../../standard/rfc4302/catalog.md), [rfc4303/catalog.md](../../../standard/rfc4303/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Tunnel mode between two hosts

Checks: **RFC4301-SA-47**, **TUN-1**, **TUN-19**, **TUN-26**, **RFC4303-LOC-9**, **LOC-11**,
**OENC-2**, **IICV-15**, **RFC4302-LOC-9**, **LOC-11**, **LOC-13** (description),
**RFC4301-SA-50** (must); covers **RFC4301-SA-41** (may), **TUN-2**, **TUN-3**, **TUN-11**,
**TUN-22**, **TUN-23**, **RFC4302-LOC-12**, **LOC-14**, **RFC4303-LOC-12**, **LOC-13**
(description).

### Requirement

RFC 4301 §4.1 and §5.1.2, RFC 4302 §3.1.2, RFC 4303 §3.1.2: a host supports tunnel mode, and two
hosts can use a tunnel mode SA between themselves. In tunnel mode the sender puts the whole
original IP datagram inside AH or ESP, behind a new outer IP header. The outer header carries
the addresses of the tunnel endpoints and names protocol 50 or 51; the inner header keeps the
ultimate addresses and is not changed. The receiver takes the inner datagram out and delivers
it.

### Scenario constants

- The link, IPv4. Flow 2000 runs from A to B on ESP SA 101 in tunnel mode, and flow 2001 on
  AH SA 103 in tunnel mode. The tunnel endpoints are A and B themselves.
- The SPDs of A and B name the two SAs, each with tunnel mode and the addresses of A and B as
  the tunnel addresses.

### Procedure

1. Build the link, and configure the SPDs and the two tunnel mode SAs.
2. Start the two flows at A.
3. Observe the packets of A on L1, and the datagrams that B delivers.

### Expected observations

1. On L1, from A to B, ESP packets with SPI 101 and AH packets with SPI 103. This confirms the
   stimulus.
2. The outer IPv4 header of each packet has the source 10.0.1.10 and the destination 10.0.1.11,
   the tunnel endpoints, and names protocol 50 or 51 (RFC4301-TUN-1, TUN-19, TUN-26).
3. Inside the ESP payload, read with the keys of SA 101, and after the AH header, comes a whole
   IPv4 header with the addresses of A and B and protocol 17: the inner header. The ESP trailer
   names Next Header 4, and the AH header names Next Header 4 (RFC4301-SA-47, RFC4303-LOC-9,
   LOC-11, OENC-2, RFC4302-LOC-9, LOC-11, LOC-13).
4. B delivers the five datagrams of each flow (RFC4301-SA-50, RFC4303-IICV-15).

## AH and ESP on one packet

Checks: **RFC4301-SA-4** (must (lower case)), **RFC4303-LOC-4** (description); covers
**RFC4301-SA-3**, **COMB-1**, **RFC4302-REAS-1** (description).

### Requirement

RFC 4301 §4.1 and RFC 4303 §3.1.1: when a stream needs both AH and ESP, two SAs protect it,
one for each protocol. In transport mode the sender applies ESP first and AH after it, so AH
covers the ESP packet. The receiver removes AH, then ESP, and delivers the datagram.

### Scenario constants

- The link, IPv4. Flow 2000 runs from A to B.
- The SPD of A: flow 2000 PROTECT with ESP SA 101 and AH SA 103, applied in this order. The SPD of
  B: flow 2000 inbound PROTECT with the same two SAs.

### Procedure

1. Build the link, and configure the SPDs and the two SAs.
2. Start flow 2000 at A.
3. Observe the packets of A on L1, and the datagrams that B delivers.

### Expected observations

1. On L1, from A to B, packets whose IPv4 header names protocol 51, whose AH header with SPI 103
   names Next Header 50, and after which comes the ESP header with SPI 101 (RFC4301-SA-4,
   RFC4303-LOC-4). This confirms the stimulus.
2. B delivers the five datagrams (RFC4301-SA-4).
