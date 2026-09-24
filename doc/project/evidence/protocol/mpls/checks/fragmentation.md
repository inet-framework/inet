# MPLS — English check procedures: fragmentation and the path MTU

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc3032/catalog.md](../../../standard/rfc3032/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## A labeled datagram too big for the next link

Checks: **RFC3032-FRAG-8**, **FRAG-9**, **FRAG-11** (must), **FRAG-12** (description); covers
**RFC3032-FRAG-1** (may (lower case)), **FRAG-7**, **FRAG-10** (may), **PPP-15** (description).

### Requirement

RFC 3032 §3.3 and §3.4: a labeled IP datagram that is longer than the payload that the next link
can carry is too big. A datagram that is not too big goes without fragmentation. A too-big IPv4
datagram with the DF bit clear is discarded, or fragmented: each fragment is at least N octets
shorter than the payload of the link, where N is the length of the label stack, and each fragment
carries the label stack that the datagram would carry.

### Scenario constants

- The path and the LSP. The frames of L3 carry at most 500 octets of payload in both directions.
- Flow 2000, and flow 2001 with 1000 octets of payload in each datagram, which gives a labeled
  packet of 1032 octets.

### Procedure

1. Build the path, set the payload limit of L3, and configure the bindings of the LSP.
2. Start flows 2000 and 2001 at A.
3. Observe the frames of R1 on L2 and of R2 on L3, and the datagrams that B delivers.

### Expected observations

1. On L2, from R1, labeled packets that carry the datagrams of flows 2000 and 2001; each packet of
   flow 2001 is 1032 octets long. This confirms the stimulus.
2. On L3, from R2, the datagrams of flow 2000, each whole: its IPv4 header shows no fragment
   (RFC3032-FRAG-9).
3. On L3, from R2, no labeled packet longer than 500 octets (RFC3032-FRAG-8, FRAG-11).
4. On L3, from R2, each fragment of a datagram carries one entry with the label 200
   (RFC3032-FRAG-12).
5. B delivers no datagram of flow 2001 other than with 1000 octets of payload. R2 can discard the
   datagrams of flow 2001, or fragment them, and B then delivers them whole.

## A too-big labeled datagram that may not be fragmented

Checks: **RFC3032-FRAG-13** (must not), **FRAG-14**, **FRAG-15** (description).

### Requirement

RFC 3032 §3.4: an LSR does not forward a too-big labeled IPv4 datagram with the DF bit set. It
sends an ICMP Destination Unreachable message to the source, with the code "fragmentation needed
and DF set" and a Next-Hop MTU that is the payload of the link minus the length of the label
stack.

### Scenario constants

- The path and the LSP. The frames of L3 carry at most 500 octets of payload in both directions.
- Flow 2002, with 1000 octets of payload in each datagram and the DF bit set.

### Procedure

1. Build the path, set the payload limit of L3, and configure the bindings of the LSP.
2. Start flow 2002 at A.
3. Observe the frames of R1 on L2 and of R2 on L3, and the ICMP messages that A receives.

### Expected observations

1. On L2, from R1, labeled packets that carry the datagrams of flow 2002, with the DF bit set.
   This confirms the stimulus.
2. On L3, from R2, no datagram of flow 2002 (RFC3032-FRAG-13).
3. A receives ICMP Destination Unreachable messages with the code 4 and the Next-Hop MTU 496
   (RFC3032-FRAG-14, FRAG-15).
