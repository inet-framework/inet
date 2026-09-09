# UDP — English check procedures: checksum

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc768/catalog.md](../../../standard/rfc768/catalog.md), [rfc792/catalog.md](../../../standard/rfc792/catalog.md), [rfc1122/catalog.md](../../../standard/rfc1122/catalog.md)

The common mockup, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Checksum presence and absence

Checks: **RFC768-CKSUM-2** (description). Also covers: **RFC768-CKSUM-1** (description,
presence only).

### Requirement

RFC 768: a sender that generates a checksum transmits a nonzero value — a computed zero
becomes all ones — and a sender that generates none transmits zero. The receiver accepts a
datagram of either kind.

### Scenario constants

- Host A generates checksums. Host C generates none.
- Host A sends 100 octets to port 5000 of host B. Host C sends 200 octets to port 5000 of
  host B.
- Host B runs one receiving program, on port 5000.

### Procedure

1. Build the mockup with all three hosts, host B between the other two.
2. Let host A and host C send, 0.1 s apart.
3. Observe both links and what host B's UDP hands upward.

### Expected observations

1. On link 1, the datagram from host A to port 5000 carries a nonzero checksum. This
   confirms the stimulus of the first half.
2. Host B's UDP hands 100 octets upward: the datagram with a checksum was accepted.
3. On link 2, the datagram from host C to port 5000 carries an all-zero checksum. This
   confirms the stimulus of the second half.
4. Host B's UDP hands 200 octets upward: the datagram without a checksum was accepted, not
   discarded as a failed one.

### Notes

- Whether the nonzero value in observation 1 is the right one is RFC768-CKSUM-1, an
  encoding statement, and a serializer unit test is its home. The wire check establishes
  presence.
- Whether a datagram with a wrong checksum is discarded is not in RFC 768 at all; RFC 1122
  states it, and it needs a corrupted datagram in flight. Both are level 3.
- A sender that cannot be told to generate no checksum cannot produce observation 3. That
  is then a limit of the scenario tooling, recorded in the results, not a judgment of the
  receiver.

## Checksum by default

Checks: **RFC1122-UCK-3** (must). Also covers: **RFC1122-UCK-1** (must, the generate half),
**RFC768-CKSUM-1** (description, the arithmetic).

### Requirement

RFC 1122 §4.1.3.4: a host must implement the facility to generate and validate UDP
checksums, and the generation must default to on. An application may be able to turn the
generation off, but nothing needs to be said for it to be on. RFC 768 defines the value:
the one's complement of the one's complement sum of a pseudo header of information from the
IP header, the UDP header and the data.

### Scenario constants

- Host A sends 100 octets from port 4000 to port 5000 of host B.
- Nothing in the scenario says anything about the checksum. No program asks for it and no
  program turns it off. This absence is the whole point of the check.

### Procedure

1. Build the mockup with host A and host B.
2. Let host A send the datagram.
3. Read the datagram on link 1. Compute the checksum that RFC 768 defines over the pseudo
   header, the UDP header and the data of that datagram, and compare it with the value in
   the checksum field.

### Expected observations

1. On link 1, a datagram to port 5000 whose checksum field is not zero. A zero would mean
   that the sender generated no checksum, which is the state that must not be the default.
2. The value in the checksum field is the value that RFC 768 defines for that datagram.
3. Host B's UDP hands the 100 octets upward, so the receiver accepted its own default.

### Notes

- Observation 1 alone is too weak. Any constant that is not zero would pass it, and a
  constant is not a checksum: a receiver that recomputes cannot validate it. Observation 2
  is what makes the check a check of RFC1122-UCK-3.
- The check reads the datagram as it travels. If the sender cannot even produce the octets
  of a datagram in the default state, that is the answer to observation 2, and the results
  record it as such.

## Checksum discard

Checks: **RFC1122-UCK-4** (must).

### Requirement

RFC 1122 §4.1.3.4: if a UDP datagram is received with a checksum that is non-zero and
invalid, UDP must silently discard the datagram. Silently means that no report goes back.

### Scenario constants

- Host A sends 100 octets from port 4000 to port 5000 of host B. Both hosts generate and
  check real checksums.
- A relay on the path replaces the checksum of the datagram with a value that is wrong for
  it and is not zero, and changes nothing else.
- A program on host B listens on port 5000 on purpose, so that only the checksum can be the
  reason for a discard.

### Procedure

1. Build the mockup with a relay between host A and host B.
2. Let host A send the datagram. The relay changes the checksum of the first datagram to
   port 5000.
3. Observe the datagram before the relay and at host B, then watch host B.

### Expected observations

1. The datagram on the path before the relay, with its original checksum.
2. The datagram at host B's interface, with a checksum that differs from the original one.
   This confirms that the change took effect.
3. Host B discards the datagram.
4. Host B hands nothing to the program on port 5000.
5. No message leaves host B about the datagram.

### Notes

- Observations 4 and 5 are the "silently" of the requirement. They are absences, and an
  absence needs a window: the window opens when the datagram arrives.
- The relay must leave the length alone. A wrong length is a different fault, and a
  receiver may drop the datagram for that reason instead, which would hide the checksum.

## Checksum covers the data

Checks: **RFC768-CKSUM-1** (description), **RFC1122-UCK-4** (must).

### Requirement

RFC 768: the checksum is computed over a pseudo header, the UDP header **and the data**.
RFC 1122 §4.1.3.4: a datagram whose non-zero checksum does not match is silently discarded.
Together the two say that a change of one octet of the data must be caught.

### Scenario constants

- Host A sends 100 octets from port 4000 to port 5000 of host B. Both hosts use real
  checksums.
- A relay changes one octet of the data of the datagram and leaves every header field
  alone, the checksum field included.

### Procedure

1. Build the mockup with a relay between host A and host B.
2. Let host A send the datagram. The relay changes one octet of its data.
3. Observe the datagram before the relay and at host B, then watch host B.

### Expected observations

1. The datagram on the path before the relay.
2. The datagram at host B's interface, of the same length and with the same checksum field
   as before. Only the data differs.
3. Host B discards the datagram.
4. Host B hands nothing to the program on port 5000, and sends nothing back.

### Notes

- The datagram keeps its length, so the length rule cannot explain the discard. The only
  statement left is the coverage of the checksum.
- This check and [Checksum discard](#checksum-discard) are two sides of one rule. One
  changes the value and keeps what it covers; this one keeps the value and changes what it
  covers.

## Checksum covers the pseudo header

Checks: **RFC768-CKSUM-1** (description), **RFC1122-UCK-4** (must).

### Requirement

RFC 768: the pseudo header conceptually prefixed to the UDP header contains the source
address, the destination address, the protocol and the UDP length, and this information
gives protection against misrouted datagrams. RFC 1122 §4.1.3.4: a datagram whose non-zero
checksum does not match is silently discarded.

### Scenario constants

- Host A sends 100 octets from port 4000 to port 5000 of host B. Both hosts use real
  checksums.
- A relay changes the source address of the datagram to another unicast address, keeps every
  checksum of the layer below valid, and leaves the UDP header and the data alone.
- The new source address is a normal unicast address, so that no rule about invalid
  addresses can explain the discard. It needs no owner: the receiver never answers this
  datagram, and the check is about the value the receiver sums.

### Procedure

1. Build the mockup with a relay between host A and host B.
2. Let host A send the datagram. The relay rewrites its source address.
3. Observe the datagram at host B, then watch host B.

### Expected observations

1. The datagram at host B's interface, carrying the new source address and the original
   UDP checksum.
2. Host B discards the datagram.
3. Host B hands nothing to the program on port 5000, and sends nothing back.

### Notes

- This is the "misrouted datagrams" sentence of RFC 768, read as a check. Nothing in the
  UDP header changed, so a receiver that computed the checksum over the UDP header and the
  data alone would accept the datagram.
- The check needs the layer below to stay correct. If the relay left a wrong checksum
  there, the datagram would never reach UDP.

## Zero checksum accepted

Checks: **RFC768-CKSUM-2** (description), **RFC1122-UCK-5** (may).

### Requirement

RFC 768: an all-zero transmitted checksum value means that the transmitter generated no
checksum. RFC 1122 §4.1.3.4: an application may be able to control whether datagrams
without checksums are discarded or passed to the application. Where no application uses
that control, an all-zero checksum is not a failed checksum, and the datagram goes up.

### Scenario constants

- Host A sends 100 octets from port 4000 to port 5000 of host B, with a real checksum.
- A relay sets the checksum field of the datagram to zero and changes nothing else.
- No program asks for datagrams without checksums to be discarded.

### Procedure

1. Build the mockup with a relay between host A and host B.
2. Let host A send the datagram. The relay sets its checksum to zero.
3. Observe the datagram at host B and what host B's UDP hands upward.

### Expected observations

1. The datagram at host B's interface with an all-zero checksum field.
2. Host B's UDP hands the 100 octets upward to the program on port 5000.

### Notes

- The discard rule of RFC1122-UCK-4 names a checksum that is "non-zero and invalid". This
  check is the other half of that sentence: zero is outside the rule.
- The check states what a receiver does when no application chose. The permission itself,
  that an application may choose, is an interface and no check on a link can see it.
