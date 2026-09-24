# MPLS — English check procedures: the link encapsulations

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc3032/catalog.md](../../../standard/rfc3032/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Labeled packets on a PPP link

Checks: **RFC3032-PPP-13**, **PPP-12**, **PPP-16** (description).

### Requirement

RFC 3032 §4.3: on a PPP link, the PPP Protocol field of a labeled unicast packet is 0281 hex. The
Information field carries exactly one labeled packet, with the label stack entries in the format
of §2.

### Scenario constants

- The path and the LSP, with no change. Flow 2000.

### Procedure

1. Build the path, and configure the bindings of the LSP.
2. Start flow 2000 at A.
3. Observe the frames of R1 on L2.

### Expected observations

1. On L2, from R1, labeled packets that carry the datagrams of flow 2000. This confirms the
   stimulus.
2. The PPP Protocol field of each frame is 0281 hex (RFC3032-PPP-13).
3. The Information field of each frame holds the entry of label 100 in the format of §2, and the
   IPv4 datagram after it, and nothing more: its length is 4 octets plus the Total Length of the
   datagram (RFC3032-PPP-12, PPP-16).

## The MPLS Control Protocol

Checks: **RFC3032-PPP-1**, **PPP-11** (must (lower case)), **PPP-5** (description).

### Requirement

RFC 3032 §4.1 to §4.3: after LCP has opened a PPP link, PPP sends MPLS Control Protocol packets,
with the PPP Protocol field 8281 hex, to enable labeled packets. PPP sends no labeled packet
before the MPLS Control Protocol is in the Opened state.

### Scenario constants

- The path and the LSP, with no change. Flow 2000.

### Procedure

1. Build the path, and configure the bindings of the LSP.
2. Start flow 2000 at A.
3. Observe the frames of R1 on L2.

### Expected observations

1. On L2, from R1, labeled packets that carry the datagrams of flow 2000. This confirms the
   stimulus.
2. Before the first labeled packet, R1 sends on L2 at least one frame with the PPP Protocol field
   8281 hex (RFC3032-PPP-1, PPP-11, PPP-5).

## Labeled packets on an Ethernet link

Checks: **RFC3032-LAN-3**, **LAN-1** (description), **LAN-2** (may (lower case)).

### Requirement

RFC 3032 §5: on a LAN, the ethertype 8847 hex marks a frame that carries a labeled unicast packet.
Each frame carries exactly one labeled packet, and the label stack entries come right after the
link headers and right before the network header.

### Scenario constants

- The path, with an Ethernet link as L2 between R1 and R2, and the LSP with the NHLFE of R1 and
  the ILM of R2 on that link. Flow 2000.

### Procedure

1. Build the path, and configure the bindings of the LSP.
2. Start flow 2000 at A.
3. Observe the frames of R1 on L2, and the datagrams that B delivers.

### Expected observations

1. On L2, from R1, frames with the ethertype 8847 hex that carry the datagrams of flow 2000
   (RFC3032-LAN-3). This confirms the stimulus.
2. In each such frame, the entry of label 100 comes right after the Ethernet header, and the IPv4
   header right after the entry (RFC3032-LAN-1, LAN-2).
3. B delivers the five datagrams of flow 2000.
