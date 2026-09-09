# IPv4 checks — time to live

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [../checks.md](../checks.md), [features.md](../features.md)

The English check procedures of the features IPV4-F-TTL. One section per check; the section
names are the anchors that the coverage ledger links to. The common mockup, the
observation rules, and the index of all checks are in [`../checks.md`](../checks.md); the
statements are in the catalogs under [`../../../standard/`](../../../standard). The
procedures come from the specification only. They name no simulation model and no code.

## TTL decrement

Checks: **RFC791-TTL-1** (must), **RFC791-CKSUM-1** (description). Also covers: **RFC791-TTL-3** (description).

### Requirement

RFC 791 §3.2: each module that processes the internet header must decrease the Time to Live
field. When no time information is available, the module must decrement the field by 1. The
sender sets the initial value. RFC 791 §3.1: the header checksum is recomputed at each point
that the header is processed, because fields such as the time to live change.

### Scenario constants

- Application data: 100 octets. Both links carry the datagram without fragmentation.
- The sender TTL on host A is set to 32.

### Procedure

1. Build the mockup network.
2. Set the sender TTL on host A to 32.
3. Start the network and let host A send the datagram.
4. Observe the datagram on link 1 and record its TTL value and its header checksum.
5. Observe the same datagram on link 2 and record its TTL value and its header checksum.
6. Observe the arrival of the datagram at host B.

### Expected observations

1. On link 1, the datagram appears with TTL = 32. This is the sender value (RFC791-TTL-3).
2. On link 2, the same datagram appears with TTL = 31, that is, the recorded value from
   link 1 minus 1 (RFC791-TTL-1).
3. Host B receives the datagram with TTL = 31.
4. The header checksum on link 2 differs from the header checksum on link 1
   (RFC791-CKSUM-1). The TTL changed by one, so a recomputed checksum cannot equal the old
   one: the two values differ by 256 in the one's complement sum.

The check passes if all four observations occur in this order within the time limit.

### Notes

- Compare the TTL on link 2 against the *recorded* value from link 1, not against the
  constant 31. The relative comparison stays correct if the sender default changes.
- The RFC permits a decrement larger than 1 when a module holds a datagram for more than one
  second. The forward delay in this mockup is far below one second, so the expected
  decrement is exactly 1. If the observed decrement is larger, examine the gateway before
  you change the expectation.


## TTL expiry

Checks: **RFC791-TTL-2** (must), **RFC792-TE-1** (must + may).

### Requirement

RFC 791 §3.2: a datagram whose time to live reaches zero must be destroyed. RFC 792: a
gateway that finds the time to live zero must discard the datagram, and it may notify the
source with a time exceeded message (type 11, code 0).

### Scenario constants

- Application data: 100 octets.
- The sender TTL on host A is set to 1: the datagram survives exactly one hop. The gateway
  decrements it to zero and must destroy it.

### Procedure

1. Build the mockup network.
2. Set the sender TTL on host A to 1.
3. Let host A send the datagram.
4. Observe link 1, link 2, host B, and the messages that return to host A.

### Expected observations

1. On link 1, the datagram appears with TTL = 1. This confirms the stimulus.
2. The gateway discards the datagram: a discard of this datagram is recorded at the gateway
   (RFC791-TTL-2). This is the observation that is in time — see the notes.
3. Host A receives an ICMP time exceeded message, type 11, code 0 (RFC792-TE-1).
4. From then on, the datagram never appears on link 2 and host B never receives it
   (RFC791-TTL-2, the wire form of observation 2).

### Notes

- Observation 2 is recorded at the gateway rather than on the wire. A watch on link 2 that
  starts only after the report arrives at host A starts too late: a forbidden forward would
  leave the gateway at the moment of the decision, before the report crosses link 1. The
  discard record is available at that moment; the wire absence of observation 4 then guards
  the rest of the window.
- Observation 3 checks a `may` clause. Its absence would not be a specification violation.
  The check asserts it because the cooperative behavior is the one worth confirming; a
  model that stays silent gets a note in the results and a `declined` in the matrix, not a
  defect. The two halves of RFC792-TE-1 carry different strengths, and the ledger keeps them
  apart.
- RFC 791 leaves open whether a gateway destroys a datagram that arrives with TTL 1 before
  or after the decrement. The observable is the same: nothing crosses link 2.


## TTL 1 at the destination

Checks: **RFC1122-TTL-2** (must not). Also covers: **RFC1122-TTL-3** (must), **RFC791-TTL-1**
(must).

### Requirement

RFC 1122 §3.2.1.7: a host must not discard a datagram just because it was received with a
TTL below 2. The intent of the field is that a gateway discards an expired datagram and the
destination host does not. RFC 1122 §3.2.1.7 also requires that the transport layer can
set the TTL of every datagram.

### Scenario constants

- Application data: 100 octets.
- The sender TTL on host A is 2: exactly the hop count of the path. The gateway decrements
  it to 1, and host B receives it with TTL 1.

### Procedure

1. Build the mockup network.
2. Set the sender TTL on host A to 2.
3. Let host A send the datagram.
4. Observe link 1, link 2, host B's UDP, and the messages that return to host A.

### Expected observations

1. On link 1, the datagram appears with TTL = 2. This confirms the stimulus and the
   sender's control of the field (RFC1122-TTL-3).
2. On link 2, the datagram appears with TTL = 1 (RFC791-TTL-1).
3. Host B's UDP receives the datagram, 108 octets (RFC1122-TTL-2): the host delivered a
   datagram that arrived with TTL 1.
4. Host A receives no Time Exceeded message: host B did not treat the datagram as expired.

The check passes if observations 1 to 3 occur in this order and observation 4 records no
message within the time limit.

### Notes

- Observation 3 is the whole requirement; observation 4 is the wire form of the same fact
  and guards against a host that delivers and reports at once.
- A TTL of 1 at the destination is a boundary value, not a fault; the check needs no relay.
