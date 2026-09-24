# MPLS — English check procedures: the time to live

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc3031/catalog.md](../../../standard/rfc3031/catalog.md), [rfc3032/catalog.md](../../../standard/rfc3032/catalog.md), [rfc3443/catalog.md](../../../standard/rfc3443/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

The checks of this file use the Uniform Model of RFC 3443 §3.1, which is the TTL treatment of
RFC 3032 §2.4. "The same datagram" on two links is the datagram with the same IPv4
Identification.

## The TTL of the first label

Checks: **RFC3032-TTL-9** (must), **RFC3031-TTL-4** (should), **RFC3443-PUSH-1**, **MOD-1**
(description); covers **RFC3032-TTL-8**, **RFC3443-ITTL-1** (description).

### Requirement

RFC 3032 §2.4.3, RFC 3031 §3.23 and RFC 3443 §3.1 and §3.6: when an LSR labels an IP packet for
the first time, the TTL field of the new entry gets the value of the IP TTL field. If the IP
processing of the LSR decrements the IP TTL, it does so before.

### Scenario constants

- The path and the LSP, with no change. Flow 2000.

### Procedure

1. Build the path, and configure the bindings of the LSP.
2. Start flow 2000 at A.
3. Observe the frames of R1 on L2.

### Expected observations

1. On L2, from R1, labeled packets that carry the datagrams of flow 2000. This confirms the
   stimulus.
2. In each packet, the TTL field of the entry equals the TTL field of the IPv4 header below it
   (RFC3032-TTL-9, RFC3031-TTL-4, RFC3443-PUSH-1, MOD-1).

## The TTL at each LSR

Checks: **RFC3032-TTL-1**, **TTL-2**, **RFC3443-TERM-2**, **ITTL-2**, **OTTL-2**,
**RFC3032-TTL-6**, **RFC3443-ITTL-5** (description), **RFC3032-TTL-5** (must), **RFC3031-TTL-5**,
**TTL-2** (should); covers **RFC3032-TTL-7**, **RFC3443-TERM-1** (description).

### Requirement

RFC 3032 §2.4.1 and §2.4.2, RFC 3031 §3.23 and RFC 3443 §2.3 and §3.4 to §3.6: the incoming TTL of
a labeled packet is the TTL field of its top entry. The outgoing TTL is one less, and never less
than zero. An LSR that forwards a labeled packet sets the TTL field of the top entry to the
outgoing TTL, whatever labels it pushes or pops. With a stack of more than one entry, the TTL
counts all LSR hops of the hierarchy. An LSR that pops more than one label gives the outgoing TTL
of one pop to the next.

### Scenario constants

- Two runs. Run 1: the path and the LSP, with no change. Run 2: the bindings of
  [A label stack of two entries](encoding.md#a-label-stack-of-two-entries): R2 replaces the
  label and pushes a new one, and R3 pops two labels.
- Flow 2000.

### Procedure

1. Build the path, and configure the bindings of the run.
2. Start flow 2000 at A.
3. Observe the frames of R1 on L2, of R2 on L3 and of R3 on L4.

### Expected observations

1. On L2 and on L3, labeled packets that carry the datagrams of flow 2000. This confirms the
   stimulus.
2. Run 1: on L3, the TTL field of the entry is one less than the TTL field of the entry of the
   same datagram on L2 (RFC3032-TTL-1, TTL-2, TTL-5, RFC3031-TTL-5, RFC3443-TERM-2, ITTL-2,
   OTTL-2).
3. Run 2: on L3, the TTL field of the top entry is one less than the TTL field of the entry of the
   same datagram on L2 (RFC3032-TTL-6, RFC3031-TTL-2).
4. Run 2: on L4, the IPv4 TTL of each datagram is one less than the TTL field of its top entry on
   L3 (RFC3443-ITTL-5).

## The TTL after the last label

Checks: **RFC3032-TTL-10**, **RFC3031-TTL-6**, **TTL-1** (should), **RFC3443-OTTL-1**,
**ITTL-4**, **MOD-1**, **OTTL-5** (description); covers **RFC3443-OTTL-3** (should (lower case)).

### Requirement

RFC 3032 §2.4.3, RFC 3031 §3.23 and RFC 3443 §3.1, §3.4 and §3.5: when an LSR pops the last label,
the IP TTL field gets the outgoing TTL. At a penultimate hop pop, the TTL field of the exposed
IP header gets the outgoing TTL. A packet leaves the LSP with the TTL that it would have without
label switching: each LSR on the path counts as one hop.

### Scenario constants

- Two runs. Run 1: the path and the LSP, with no change. Run 2: the bindings of
  [Penultimate hop popping](forwarding.md#penultimate-hop-popping).
- Flow 2000, whose datagrams leave A with TTL 32.

### Procedure

1. Build the path, and configure the bindings of the run.
2. Start flow 2000 at A.
3. Observe the frames of R1 on L2, of R2 on L3 and of R3 on L4.

### Expected observations

1. Run 1: labeled packets of flow 2000 on L3 and IPv4 datagrams of flow 2000 on L4. Run 2:
   labeled packets on L2 and IPv4 datagrams on L3. This confirms the stimulus.
2. Run 1: on L4, the IPv4 TTL of each datagram is one less than the TTL field of the entry of the
   same datagram on L3 (RFC3032-TTL-10, RFC3031-TTL-6, RFC3443-OTTL-1, ITTL-4, MOD-1).
3. Run 2: on L3, the IPv4 TTL of each datagram is one less than the TTL field of the entry of the
   same datagram on L2 (RFC3443-OTTL-5, RFC3032-TTL-10).
4. In both runs: on L4, the IPv4 TTL of each datagram is 29, because the three routers R1, R2 and
   R3 forwarded it (RFC3031-TTL-1).

## A TTL that reaches zero

Checks: **RFC3032-TTL-3** (must not), **RFC3443-TERM-3** (description); covers **RFC3032-TTL-4**
(may), **RFC3443-TERM-4** (description), **IMPL-2** (must (lower case)).

### Requirement

RFC 3032 §2.4.2 and RFC 3443 §2.3: an LSR does not forward a labeled packet whose outgoing TTL is
zero, with or without its label stack. It discards the packet, or gives it to the network layer
for the processing of the error.

### Scenario constants

- The path and the LSP, with no change.
- Flow 2000, whose datagrams leave A with TTL 32, and flow 2001, whose datagrams leave A with
  TTL 2. R1 forwards a datagram of flow 2001 with the IP TTL 1, and labels it with the TTL 1;
  the outgoing TTL of R2 is then zero.

### Procedure

1. Build the path, and configure the bindings of the LSP.
2. Start flows 2000 and 2001 at A.
3. Observe the frames of R1 on L2 and of R2 on L3, and the datagrams that B delivers.

### Expected observations

1. On L2, from R1, labeled packets that carry the datagrams of flows 2000 and 2001. This confirms
   the stimulus.
2. B delivers the five datagrams of flow 2000. This confirms the path.
3. On L3, from R2, no datagram of flow 2001 (RFC3032-TTL-3, RFC3443-TERM-3).
4. B delivers no datagram of flow 2001 (RFC3032-TTL-3).
