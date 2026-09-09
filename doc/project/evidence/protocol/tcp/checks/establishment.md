# TCP — English check procedures: connection establishment

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc9293/catalog.md](../../../standard/rfc9293/catalog.md)

The common mockups, the scenario constants and the index are in [`checks.md`](../checks.md).
The procedures come from the specification only. They name no simulation model and no code.

## Connection establishment

- **Checks** — RFC9293-EST-1, RFC9293-EST-2, RFC9293-SEQ-1, RFC9293-HDR-1 (description),
  RFC9293-CKSUM-1 (must). Also covers: RFC9293-ISS-1 (description), RFC9293-OPT-1
  (should).
- **Requirement** — a connection opens with three segments. Host A sends a SYN carrying its
  own initial sequence number and a checksum. Host B answers with a segment that carries
  both SYN and ACK, and whose acknowledgment field is host A's initial sequence number plus
  one, because the SYN occupies one sequence number. Host A answers with an ACK, without
  SYN, whose acknowledgment field is host B's initial sequence number plus one. Every
  header is a whole number of 32-bit words, at least five.

### Value arithmetic

Write `ISS_A` for the sequence number of host A's SYN and `ISS_B` for the sequence number
of host B's SYN-ACK. Both are chosen by their sender and are unknown before the run.

| Segment | Sequence number | Acknowledgment field |
| --- | --- | --- |
| 1, A to B | `ISS_A` | none (the ACK bit is clear) |
| 2, B to A | `ISS_B` | `ISS_A + 1` |
| 3, A to B | `ISS_A + 1` | `ISS_B + 1` |

The plus-one in row 2 and row 3 is the whole of RFC9293-SEQ-1: the SYN occupied exactly one
sequence number.

### Procedure

1. Build the common mockup.
2. Let host A open a connection to host B at the open time.
3. Observe the segments that host A sends and receives on link L.

### Expected observations

1. Host A sends a segment with SYN set and ACK clear, a header length of at least 20
   octets, and a nonzero checksum. Record its sequence number as `ISS_A` and its header
   length. *(This observation confirms the stimulus, and covers HDR-1 and CKSUM-1. A header
   length of 24 octets means the SYN carries the MSS option, RFC9293-OPT-1; the option is a
   `should`, so its absence is a note and not a failure.)*
2. Host A receives a segment with SYN set and ACK set, whose acknowledgment field equals
   `ISS_A + 1`. Record its sequence number as `ISS_B`.
3. Host A sends a segment with ACK set and SYN clear, whose acknowledgment field equals
   `ISS_B + 1`, with a header length of 20 octets: no option, five words.

### Notes

- The order of the three observations is part of the requirement.
- Simultaneous open is a separate case in the RFC and is out of scope in this pass.
