# MPLS — English check procedures: label switching

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc3031/catalog.md](../../../standard/rfc3031/catalog.md), [rfc3032/catalog.md](../../../standard/rfc3032/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Label switching along an LSP

Checks: **RFC3031-FTN-1**, **SWAP-2**, **LSP-1**, **ILM-1**, **SWAP-1**, **NHLFE-1**,
**NHLFE-2**, **LSP-4**, **NHLFE-3**, **LSP-6**, **RFC3032-NLP-1** (description), **NLP-4**,
**NLP-5**, **NLP-2**, **NLP-3** (must (lower case)); covers **RFC3031-LSP-2**, **LSP-3**,
**SWAP-4** (description), **NHLFE-5**, **NHLFE-6** (may).

### Requirement

RFC 3031 §3.10 to §3.13 and §3.15, and RFC 3032 §2.2: the LSP Ingress maps an unlabeled packet
to its FEC, finds the NHLFE in the FTN, and pushes a label. Each intermediate LSR looks up the
top label in its ILM and applies the operation of the NHLFE, here a swap. The LSP Egress pops
the last label, finds the network layer protocol of the packet from the label, and forwards the
packet by its network header. The first label and every label that replaces it identify the
network layer protocol.

### Scenario constants

- The path and the LSP, with no change. Flow 2000.

### Procedure

1. Build the path, and configure the bindings of the LSP.
2. Start flow 2000 at A.
3. Observe the frames of R1 on L2, of R2 on L3 and of R3 on L4, and the datagrams that B
   delivers.

### Expected observations

1. On L2, from R1, the datagrams of flow 2000 with one label stack entry, label 100
   (RFC3031-FTN-1, SWAP-2, LSP-1, RFC3032-NLP-4). This confirms the stimulus.
2. On L3, from R2, the same datagrams with one entry, label 200 (RFC3031-ILM-1, SWAP-1,
   NHLFE-1, NHLFE-2, LSP-4, RFC3032-NLP-5).
3. On L4, from R3, the same datagrams with no label stack entry, as IPv4 datagrams: the PPP
   Protocol field is 0021 hex (RFC3031-NHLFE-3, LSP-6, RFC3032-NLP-2, NLP-3).
4. B delivers the five datagrams of flow 2000, each with 100 octets of payload (RFC3032-NLP-1).

## Penultimate hop popping

Checks: **RFC3031-PHP-1** (may (lower case)), **PHP-2**, **PHP-3** (description), **PHP-6**
(must); covers **RFC3031-PHP-4**, **PHP-5**, **NULL-3** (description).

### Requirement

RFC 3031 §3.16: the penultimate LSR of an LSP can pop the label stack instead of the LSP Egress.
An LSR that can pop the stack at all does so when its downstream peer asks for it. The egress
then gets the packet unlabeled, and forwards it by its network header.

### Scenario constants

- The path. R1 as in the LSP. R3 asks for penultimate hop popping: the NHLFE of R2 for label 100
  pops the stack, next hop R3. R3 has no binding.
- Flow 2000.

### Procedure

1. Build the path, and configure the bindings.
2. Start flow 2000 at A.
3. Observe the frames of R1 on L2, of R2 on L3 and of R3 on L4, and the datagrams that B
   delivers.

### Expected observations

1. On L2, from R1, the datagrams of flow 2000 with label 100. This confirms the stimulus.
2. On L3, from R2, the same datagrams with no label stack entry, as IPv4 datagrams
   (RFC3031-PHP-1, PHP-2, PHP-6).
3. On L4, from R3, the same datagrams as IPv4 datagrams (RFC3031-PHP-3).
4. B delivers the five datagrams of flow 2000 (RFC3031-PHP-3).
