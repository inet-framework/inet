# IPsec — English check procedures: fragmentation and path MTU

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc4301/catalog.md](../../../standard/rfc4301/catalog.md), [rfc4302/catalog.md](../../../standard/rfc4302/catalog.md), [rfc4303/catalog.md](../../../standard/rfc4303/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Fragments of a protected datagram

Checks: **RFC4301-IN-5**, **RFC4302-FRAG-1**, **REAS-2**, **RFC4303-FRAG-1**, **FRAG-2**,
**REAS-1** (description), **RFC4301-SA-44**, **RFC4302-FRAG-2** (must (lower case)),
**RFC4301-SA-45**, **SA-46** (must not), **RFC4303-FRAG-3** (may, must (lower case)); covers
**RFC4302-FRAG-3** (description), **OICV-14**, **REAS-5**, **RFC4303-REAS-5** (must (lower
case)).

### Requirement

RFC 4302 §3.3.4 and §3.4.1, RFC 4303 §3.3.4 and §3.4.1, RFC 4301 §4.1 and §5.2: in transport
mode AH and ESP apply to a whole IP datagram, never to a fragment. The sender fragments after
the IPsec processing, so only the first fragment holds the AH or ESP header. A router can also
fragment a protected packet on its way. The receiver reassembles the fragments before the IPsec
processing, and delivers the datagram.

### Scenario constants

- The path, once on IPv4 and once on IPv6.
- On IPv4 the MTU of L2 is 1000 octets. Four flows run from A to B:
  - flow 2000 on ESP SA 101 and flow 2001 on AH SA 103, with 3000 octets of payload: A itself
    fragments them for L1;
  - flow 2002 on ESP SA 102 and flow 2003 on AH SA 104, with 1400 octets of payload: the
    packets leave A whole, and R fragments them for L2.
- On IPv6 the MTU of both links is 1500 octets, because an IPv6 router does not fragment. Flows
  2000 and 2001 run as on IPv4.
- The SPDs of A and B name the SAs.

### Procedure

1. Build the path, and configure the SPDs and the SAs.
2. Start the flows at A.
3. Observe the packets of A on L1 and of R on L2, and the datagrams that B delivers.

### Expected observations

1. On L1, from A, the fragments of each datagram of flows 2000 and 2001: IPv4 fragments of
   protocol 50 or 51, or IPv6 packets with a Fragment header whose Next Header is 50 or 51. This
   confirms the stimulus.
2. B delivers the five datagrams of each flow, each with its whole payload: B reassembles before
   AH and ESP (RFC4301-IN-5, SA-44, RFC4302-REAS-2, RFC4303-REAS-1).
3. On IPv4, B also delivers the datagrams of flows 2002 and 2003, which R fragmented on L2
   (RFC4302-FRAG-2, RFC4303-FRAG-3).
4. Only the first fragment of each datagram holds an AH or ESP header, directly after the IP
   header or the Fragment header; the other fragments hold the rest of the one AH or ESP packet.
   A applied AH or ESP once to the whole datagram, then fragmented it (RFC4301-SA-45, SA-46,
   RFC4302-FRAG-1, RFC4303-FRAG-1, FRAG-2).

## Path MTU of a protected flow

Checks: **RFC4301-OUT-21**, **RFC4302-FRAG-10**, **RFC4303-FRAG-9** (must); covers
**RFC4301-OUT-5** (must (lower case)), **SADI-22** (must), **RFC4302-FRAG-9**, **RFC4303-FRAG-8**
(may (lower case)).

### Requirement

RFC 4301 §5.1, RFC 4302 §3.3.4 and RFC 4303 §3.3.4: an IPsec implementation keeps the path MTU
of an SA and acts on the reports of it: on a native host, the report of an ICMP Packet Too Big
message reaches the protected traffic, so that the next packets fit the path.

### Scenario constants

- The path, IPv6. The MTU of L2 is 1280 octets. Flow 2000 on ESP SA 101 and flow 2001 on AH
  SA 103 run from A to B, with 1300 octets of payload in each datagram: each protected packet is
  longer than 1280 octets.
- The SPDs of A, R and B let the ICMPv6 Packet Too Big messages of R reach A.

### Procedure

1. Build the path, and configure the SPDs and the two SAs.
2. Start the two flows at A.
3. Observe the packets of A on L1, the ICMPv6 messages of R, and the datagrams that B delivers.

### Expected observations

1. On L1, from R to A, ICMPv6 Packet Too Big messages with MTU 1280. This confirms the stimulus.
2. After the first Packet Too Big message for each flow, every packet of the flow that A sends
   is 1280 octets or shorter (RFC4301-OUT-21, RFC4302-FRAG-10, RFC4303-FRAG-9).
3. B delivers the datagrams that A sends after the report, for both flows.
