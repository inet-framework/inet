# IPv4 checks — header checksum

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [../checks.md](../checks.md), [features.md](../features.md)

The English check procedures of the feature IPV4-F-HEADER-CHECKSUM. The recompute half of
the feature is in [TTL decrement](ttl.md#ttl-decrement); this file holds the verification
half. The common mockup, the observation rules, and the index of all checks are in
[`../checks.md`](../checks.md). The procedures come from the specification only. They name
no simulation model and no code.

## Checksum discard

Checks: **RFC1122-CKSUM-1** (must), which governs **RFC791-CKSUM-2** (description).

### Requirement

RFC 1122 §3.2.1.2: a host must verify the header checksum on every received datagram and
silently discard every datagram that has a bad checksum. RFC 791 §1.4: a datagram whose
header checksum fails is discarded at once by the entity that detects the error.

### Scenario constants

- Mockup with a relay. Application data: 100 octets, to port 5000.
- The relay replaces the header checksum of the datagram with its one's complement, a value
  that cannot be right for any header, and changes nothing else. This is the one check in
  which the relay corrupts the checksum instead of keeping it valid.
- Every node computes checksums (the common mockup says so); the relay's rewrite is the
  only wrong value on the path.

### Procedure

1. Build the mockup with the relay.
2. Tell the relay to corrupt the header checksum of the datagram.
3. Let host A send the datagram.
4. Observe link 2 before the relay, host B's interface, host B's internet module, host B's
   UDP, and the messages that leave host B.

### Expected observations

1. On link 2, the datagram appears. Record its header checksum as CK.
2. At host B's interface, the datagram arrives with a header checksum that differs from CK.
   This confirms the corruption.
3. Host B discards the datagram: a discard of this datagram is recorded at host B's
   internet module (RFC1122-CKSUM-1, RFC791-CKSUM-2). This is the observation that is in
   time.
4. Host B's UDP never receives the datagram (the "discard" half).
5. No ICMP message leaves host B (the "silently" half).

The check passes if observations 1 to 3 occur in this order and observations 4 and 5 record
nothing within the time limit.

### Notes

- The header is otherwise intact, so a receiver that verifies the checksum has exactly one
  reason to discard. A receiver that consults the checksum only when the header is already
  malformed passes the datagram upward and fails observation 3.
- The complement of a correct checksum is wrong with certainty; a random value would be
  wrong with probability 65535 in 65536, and a check does not rest on probability.
