# IPv4 checks — identification

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [../checks.md](../checks.md), [features.md](../features.md)

The English check procedures of the features IPV4-F-IDENTIFICATION. One section per check; the section
names are the anchors that the coverage ledger links to. The common mockup, the
observation rules, and the index of all checks are in [`../checks.md`](../checks.md); the
statements are in the catalogs under [`../../../standard/`](../../../standard). The
procedures come from the specification only. They name no simulation model and no code.

## Identification

Checks: **RFC791-ID-1** (must).

### Requirement

RFC 791 §2.3: the sender sets the identification field to a value that must be unique for
the source, the destination and the protocol for the time the datagram is active.

### Scenario constants

- Host A sends two datagrams of 100 octets each to host B, 0.1 s apart, from one source
  port to one destination port: the same source, destination and protocol.

### Procedure

1. Build the mockup network.
2. Let host A send two datagrams in succession.
3. Observe both on link 1 and record their identification fields.

### Expected observations

1. On link 1, the first datagram appears. Record its identification as ID1. This confirms
   the stimulus.
2. On link 1, the second datagram appears with an identification that differs from ID1
   (RFC791-ID-1).

### Notes

- Two datagrams show that the sender does not reuse a value at once. Uniqueness over the
  whole active time of a datagram needs a long run with many datagrams; that is a
  sharpening candidate, not a level 2 check.
- RFC 6864 rewrites this requirement for atomic datagrams. It is outside the in-scope set;
  see [`standards.md`](../standards.md#override-table).


## Atomic identification

Checks: **RFC6864-ID-3** (must), **RFC6864-ID-7** (must not). Also covers:
**RFC6864-ID-6** (must not). Notes: **RFC6864-ID-2** (may).

### Requirement

RFC 6864 §4: an atomic datagram has DF = 1, MF = 0 and offset 0. §4.1: every device that
examines an IPv4 header must ignore the identification of an atomic datagram, because a
source may set that field to any value. §4.3: a datagram with DF set must not be
fragmented, and a transit device must not clear DF.

### Scenario constants

- Mockup with a relay. Both links keep the default MTU, so nothing is fragmented.
- Host A sends two datagrams of 100 octets with DF set, at the same time, to port 5000 and
  to port 5001 of host B. Both are atomic.
- The relay rewrites the identification of datagram 2 to the identification of datagram 1
  and keeps the checksum valid. Host B then receives two atomic datagrams that agree in
  identification, source, destination and protocol.

### Procedure

1. Build the mockup with the relay.
2. Set the don't fragment flag for both applications on host A.
3. Tell the relay to rewrite the identification of datagram 2.
4. Let host A send the two datagrams.
5. Observe link 1, link 2 before the relay, host B's interface, and host B's UDP.

### Expected observations

1. On link 1, datagram 1 appears with DF = 1, MF = 0, offset 0: an atomic datagram. Record
   its identification as ID1.
2. On link 2, datagram 1 appears with DF = 1 and no fragmentation (RFC6864-ID-7,
   RFC6864-ID-6): the gateway forwarded the flag as it was. The gateway is done with
   datagram 1 before host A has finished sending datagram 2.
3. On link 1, datagram 2 appears with DF = 1 and an identification that differs from ID1.
   The source chose distinct values, which RFC6864-ID-2 permits but does not require.
4. Host B's UDP receives datagram 1: 108 octets, port 5000.
5. At host B's interface, datagram 2 arrives with identification ID1 and DF = 1. This
   confirms that the rewrite took effect.
6. Host B's UDP receives datagram 2: 108 octets, port 5001 (RFC6864-ID-3): the repeated
   identification of an atomic datagram changed nothing.

The check passes if all six observations occur in this order within the time limit.

### Notes

- Observation 3 records a fact about the source; it is not a pass condition on its own,
  but the rewrite in observation 5 needs two distinct values to be visible.
- A receiver that fails this check treats the second datagram as a duplicate, or holds it
  in a reassembly buffer. Both are the failure RFC 6864 §4.1 describes.
