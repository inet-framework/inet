# MPLS — English check procedures: the label stack encoding

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc3031/catalog.md](../../../standard/rfc3031/catalog.md), [rfc3032/catalog.md](../../../standard/rfc3032/catalog.md), [rfc5462/catalog.md](../../../standard/rfc5462/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## The label stack entry

Checks: **RFC3032-ENC-1**, **ENC-2**, **ENC-3**, **ENC-5**, **ENC-6**, **ENC-7**, **ENC-9**,
**ENC-10**, **RFC5462-TC-1** (description), **RFC3031-TTL-3** (must); covers **RFC3032-ENC-4**,
**RFC3031-LBL-3** (description), **LBL-4** (must (lower case)).

### Requirement

RFC 3032 §2.1, RFC 5462 §2.1 and RFC 3031 §3.23: a label stack entry is 4 octets. In the order
of transmission it holds the Label field of 20 bits, the Traffic Class field of 3 bits, the S
bit and the TTL field of 8 bits. The stack comes after the link header and before the network
header, and the network header follows the entry whose S bit is set. With one entry, that entry
is the bottom of the stack and has the S bit set.

### Scenario constants

- The path and the LSP, with no change. Flow 2000.

### Procedure

1. Build the path, and configure the bindings of the LSP.
2. Start flow 2000 at A.
3. Observe the frames of R1 on L2.

### Expected observations

1. On L2, from R1, labeled packets that carry the datagrams of flow 2000. This confirms the
   stimulus.
2. Each packet holds one label stack entry of 4 octets between the link header and the IPv4
   header, and the IPv4 header follows the entry (RFC3032-ENC-1, ENC-7, ENC-9).
3. The first 20 bits of the entry hold the label 100, and the bit after the 3 bits of the Traffic
   Class field, the S bit, is 1 (RFC3032-ENC-2, ENC-3, ENC-5, ENC-10, RFC5462-TC-1). The last
   8 bits are the TTL field (RFC3032-ENC-6, RFC3031-TTL-3); the checks of
   [`ttl.md`](ttl.md) examine its value.

## A label stack of two entries

Checks: **RFC3032-ENC-8**, **ENC-10**, **RFC3031-NHLFE-4**, **STK-1** (description), **NHLFE-7**
(must); covers **RFC3031-STK-2** (may (lower case)), **LSP-7** (description).

### Requirement

RFC 3032 §2.1 and RFC 3031 §3.9 and §3.10: the label stack is a last-in, first-out stack. The top
entry comes first in the packet, the bottom entry last, and only the bottom entry has the S bit
set. One operation of an NHLFE replaces the top label and then pushes a new label. When the next
hop of an NHLFE is the LSR itself, the LSR pops the top label and forwards the packet by what
remains.

### Scenario constants

- The path. R1 as in the LSP. The NHLFE of R2 for label 100 replaces it with 200 and then pushes
  300, next hop R3.
- R3 maps label 300 to an NHLFE that pops the stack with R3 itself as the next hop, and maps
  label 200 to an NHLFE that pops the stack with B as the next hop.
- Flow 2000.

### Procedure

1. Build the path, and configure the bindings.
2. Start flow 2000 at A.
3. Observe the frames of R2 on L3 and of R3 on L4, and the datagrams that B delivers.

### Expected observations

1. On L3, from R2, labeled packets that carry the datagrams of flow 2000. This confirms the
   stimulus.
2. Each packet holds two entries: first the entry with the label 300 and the S bit 0, then the
   entry with the label 200 and the S bit 1, then the IPv4 header (RFC3032-ENC-8, ENC-10,
   RFC3031-NHLFE-4, STK-1).
3. On L4, from R3, the datagrams of flow 2000 with no label stack entry (RFC3031-NHLFE-7).
4. B delivers the five datagrams of flow 2000 (RFC3031-NHLFE-7).
