# DSDV conformance

> **Kind:** report · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [coverage](coverage.md)

## Claims

`src/inet/routing/dsdv/Dsdv.ned` claims the bounded IPv4 host-routing profile
of Perkins/Bhagwat (SIGCOMM 1994), §§3–4. `DsdvTable.cc` implements freshness,
metric selection, odd-sequence withdrawals, separate advertised state and the
model's explicit settling/dissemination choices. `Dsdv.cc` supplies UDP,
owned /32 forwarding routes, timers and lifecycle integration. The user guide's
DSDV section bounds operation to one bidirectional interface and excludes
external wire interoperability and module recreation without persistent state.

## Matrix

Measured on 2026-11-08 using the run record in [results](results.md):

| Feature | Claimed | Measured support | Verdict |
| --- | --- | --- | --- |
| DSDV-F-SELECT | yes | supported | confirmed within the checked traces and graphs |
| DSDV-F-LOSS | yes | supported | confirmed within controlled loss, expiry and recovery fixtures |
| DSDV-F-DUMPS | yes | supported | confirmed within checked capacities and update sequences |
| DSDV-F-SETTLE | yes | supported | confirmed for the explicit model smoothing/deadline law |
| DSDV-F-PROFILE | yes | supported | confirmed for tested encoding, attachment and lifecycle paths |

The damping ambiguity, parity/metric limits, checked finite-run sequences,
retained invalid floors, smoothing equation, restart rules and wire format are
engineering choices. They are not presented as unambiguous requirements of the
paper. Layer-2 routing, IPv6, default/prefix propagation, security, asymmetric
links, alternate metrics, bounded garbage collection and process recreation
without persistent state remain outside the claim. No performance claim is made.
