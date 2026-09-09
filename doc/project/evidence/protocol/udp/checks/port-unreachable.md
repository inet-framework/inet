# UDP — English check procedures: port unreachable

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc768/catalog.md](../../../standard/rfc768/catalog.md), [rfc792/catalog.md](../../../standard/rfc792/catalog.md), [rfc1122/catalog.md](../../../standard/rfc1122/catalog.md)

The common mockup, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Port unreachable

Checks: **RFC1122-UPORT-1** (should), which governs **RFC792-DU-3** (may). Also covers:
**RFC768-HDR-2** (the closed-port half).

### Requirement

RFC 1122 §4.1.3.1: if a datagram arrives addressed to a UDP port for which there is no
pending LISTEN call, UDP should send an ICMP Port Unreachable message. RFC 792: the
destination host may send a destination unreachable message, code 3, to the source. RFC
768: the destination port selects the receiver; a port that no program opened selects none.

### Scenario constants

- Host A sends 100 octets from port 4000 to port 6000 of host B. No program on host B has
  opened port 6000.

### Procedure

1. Build the mockup with host A and host B.
2. Let host A send the datagram.
3. Observe link 1 in both directions, host B's UDP module, and what it hands upward.

### Expected observations

1. On link 1, the datagram to port 6000 from port 4000. This confirms the stimulus.
2. Host B's UDP discards the datagram: a discard for a port without a program is recorded
   at host B (RFC768-HDR-2, the closed-port half). This observation is in time; a watch
   on the upward handoff that starts after the report arrives would start too late.
3. Host A receives an ICMP destination unreachable message, type 3, code 3
   (RFC1122-UPORT-1, RFC792-DU-3).
4. From then on, host B's UDP hands nothing upward.

### Notes

- Observation 3 checked a `may` clause at level 2, where RFC 792 was the only source. From
  level 3 on RFC 1122 §4.1.3.1 is in scope and states the same report as a `should`. The
  procedure does not change; what changes is that silence now needs a reason. A model that
  stays silent still gets a `declined` in the matrix and not a defect, because a `should`
  is not a `must`.
- The discard record in observation 2 plays the role that the gateway's discard plays in
  the IPv4 checks: the positive event that precedes the report, so that the negative
  observation is not anchored too late.
