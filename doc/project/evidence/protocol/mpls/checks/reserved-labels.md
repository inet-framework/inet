# MPLS — English check procedures: the reserved label values

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc3031/catalog.md](../../../standard/rfc3031/catalog.md), [rfc3032/catalog.md](../../../standard/rfc3032/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## The IPv4 Explicit NULL label

Checks: **RFC3032-ENC-11** (must (lower case)).

### Requirement

RFC 3032 §2.1: the label value 0 is the IPv4 Explicit NULL label. It is legal only at the bottom
of the stack. An LSR that gets it pops the stack and forwards the packet by its IPv4 header. The
label has this meaning in every LSR, with no binding.

### Scenario constants

- The path. R1 as in the LSP. R3 distributed the IPv4 Explicit NULL label for the FEC: the NHLFE
  of R2 for label 100 replaces it with 0, next hop R3. R3 has no binding.
- Flow 2000.

### Procedure

1. Build the path, and configure the bindings.
2. Start flow 2000 at A.
3. Observe the frames of R2 on L3 and of R3 on L4, and the datagrams that B delivers.

### Expected observations

1. On L3, from R2, the datagrams of flow 2000 with one entry, label 0. This confirms the
   stimulus.
2. On L4, from R3, the same datagrams with no label stack entry, as IPv4 datagrams
   (RFC3032-ENC-11).
3. B delivers the five datagrams of flow 2000 (RFC3032-ENC-11).

## The Implicit NULL label

Checks: **RFC3032-ENC-15** (may (lower case)), **RFC3031-NULL-1** (must (lower case)).

### Requirement

RFC 3032 §2.1 and RFC 3031 §4.1.5: the label value 3 is the Implicit NULL label. An LSR can
distribute it, but it never appears in the encapsulation. When an LSR would replace the top label
with the Implicit NULL label, it pops the stack instead.

### Scenario constants

- The path. R1 as in the LSP. R3 distributed the Implicit NULL label for the FEC: the NHLFE of R2
  for label 100 replaces it with 3, next hop R3. R3 has no binding.
- Flow 2000.

### Procedure

1. Build the path, and configure the bindings.
2. Start flow 2000 at A.
3. Observe the frames of R1 on L2 and of R2 on L3, and the datagrams that B delivers.

### Expected observations

1. On L2, from R1, the datagrams of flow 2000 with label 100. This confirms the stimulus.
2. On L3, from R2, no packet with the label 3 (RFC3032-ENC-15).
3. On L3, from R2, the datagrams of flow 2000 with no label stack entry, as IPv4 datagrams
   (RFC3032-ENC-15, RFC3031-NULL-1).
4. B delivers the five datagrams of flow 2000 (RFC3031-NULL-1).
