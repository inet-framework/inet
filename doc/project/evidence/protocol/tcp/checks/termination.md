# TCP — English check procedures: connection termination

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc9293/catalog.md](../../../standard/rfc9293/catalog.md)

The common mockups, the scenario constants and the index are in [`checks.md`](../checks.md).
The procedures come from the specification only. They name no simulation model and no code.

## Connection termination

- **Checks** — RFC9293-FIN-1 (description), RFC9293-FIN-2 (description).
- **Requirement** — a normal close carries a FIN in each direction, and each FIN is
  acknowledged. The FIN occupies one sequence number, so the acknowledgment that answers a
  FIN whose sequence number is `F` equals `F + 1`. The two directions close one at a time.

### Value arithmetic

Write `F_A` for the sequence number of host A's FIN and `F_B` for the sequence number of
host B's FIN.

| Segment | Direction | Acknowledgment field |
| --- | --- | --- |
| FIN of A | A to B | — |
| ACK of that FIN | B to A | `F_A + 1` |
| FIN of B | B to A | — |
| ACK of that FIN | A to B | `F_B + 1` |

### Procedure

1. Build the common mockup, open the connection, and send a 100-octet block as above.
2. Let host A close the connection at the close time.
3. Observe the four segments of the close on link L.

### Expected observations

1. Host A sends a segment with FIN set. Record its sequence number as `F_A`. *(This
   observation confirms the stimulus: the close really started.)*
2. Host A receives a segment with ACK set whose acknowledgment field equals `F_A + 1`. The
   plus-one is RFC9293-FIN-1: the FIN occupied one sequence number.
3. Host A receives a segment with FIN set. Record its sequence number as `F_B`.
4. Host A sends a segment with ACK set whose acknowledgment field equals `F_B + 1`.

### Notes

- Observation 2 and observation 3 may arrive in one segment, because a peer may set FIN and
  ACK together. The check requires two matches and one segment can satisfy only one of
  them, so the check as written expects the peer to answer with a separate acknowledgment
  first. Figure 12 of the RFC shows exactly that separation.
- Simultaneous close and the TIME-WAIT duration are out of scope.
