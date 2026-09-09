# UDP — English check procedures: delivery

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc768/catalog.md](../../../standard/rfc768/catalog.md), [rfc792/catalog.md](../../../standard/rfc792/catalog.md), [rfc1122/catalog.md](../../../standard/rfc1122/catalog.md)

The common mockup, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Datagram delivery

Checks: **RFC768-HDR-2**, **RFC768-HDR-3**, **RFC768-PROTO-1** (description),
**RFC768-UI-1** (should). Also covers: **RFC768-HDR-1** (description).

### Requirement

RFC 768: the destination port selects the receiving program within the destination
address; the length field counts the header and the data, so the minimum is eight; the
datagram is IP protocol 17; a receive operation returns the data with the source port and
the source address.

### Scenario constants

- Host A runs three sending programs. P1 sends 100 octets from port 4000 to port 5000 of
  host B. P2 sends 200 octets from port 4001 to port 5001. P3 sends an empty datagram, 0
  octets, from port 4002 to port 5000.
- Host B runs two receiving programs, on port 5000 and on port 5001.

### Procedure

1. Build the mockup with host A and host B.
2. Let P1, P2 and P3 send, in that order, 0.1 s apart.
3. Observe link 1, host B's UDP module, and what it hands to each program.

### Expected observations

1. On link 1, a datagram to port 5000 from port 4000 with a UDP length of 108 octets, in an
   IP datagram whose protocol field is 17. This confirms the stimulus and covers
   RFC768-HDR-1 (a meaningful source port), RFC768-HDR-3 and RFC768-PROTO-1.
2. Host B's UDP hands 100 octets of data upward to the program on port 5000 (RFC768-HDR-2,
   RFC768-UI-1).
3. On link 1, a datagram to port 5001 from port 4001 with a UDP length of 208 octets.
4. Host B's UDP hands 200 octets of data upward to the program on port 5001, and not to the
   program on port 5000 (RFC768-HDR-2: the port selected the receiver).
5. On link 1, a datagram to port 5000 with a UDP length of 8 octets: the empty datagram
   (RFC768-HDR-3, the minimum).
6. Host B's UDP hands 0 octets of data upward to the program on port 5000.

### Notes

- The source port and the source address that the receiving program learns are part of
  RFC768-UI-1 but live at the program interface, which not every environment exposes. The
  check confirms them on the wire in observations 1 and 3 and leaves the interface half to
  the notes of the results.
- Observation 4's "not to the program on port 5000" is established together with the port
  unreachable check: a datagram to an open port reaches its program, a datagram to a
  closed one reaches none.

## Valid source address

Checks: **RFC1122-UADDR-2** (must).

### Requirement

RFC 1122 §4.1.3.6: when a host sends a UDP datagram, the source address must be one of the
IP addresses of that host. The receiving program learns the source address of a datagram
(RFC768-UI-1), and that address is worth nothing unless the sender obeys this rule.

### Scenario constants

- Host A has two interfaces, one on link 1 and one on link 3, with a different address on
  each.
- Host A sends 100 octets from port 4000 to port 5000 of host B, which is on link 1.
- No program names a source address, so the host chooses one itself.

### Procedure

1. Build the mockup with host A, host B and the second link of host A.
2. Let host A send the datagram.
3. Read the source address of the datagram on link 1 and compare it with the two addresses
   of host A.

### Expected observations

1. On link 1, a datagram to port 5000 whose source address is one of the two addresses of
   host A.
2. Host B's UDP hands the 100 octets upward, so the datagram was a normal one and not a
   discarded one.

### Notes

- The rule says "one of", so the check accepts either address. Which one a host picks for
  a destination is a routing question, and no statement of the in-scope set answers it.
- A host with one interface would make the check weak: any address it uses would be the
  only candidate. The second interface is what gives the check its content.

## Empty datagram

Checks: **RFC768-HDR-3** (description), the minimum of eight.

### Requirement

RFC 768: the length field is the length in octets of this user datagram, header and data
included, so the minimum value of the length is eight. A datagram of that size carries no
data, and a receiver accepts it like any other.

### Scenario constants

- Host A sends 100 octets from port 4000 to port 5000 of host B.
- A relay removes the data of the datagram and corrects the length fields, so that an
  8-octet datagram reaches host B.
- The relay also sets the checksum to zero, which RFC 768 reads as "the transmitter
  generated no checksum". A value that covered the removed data would be wrong for the new
  datagram, and the receiver would then discard it for the checksum instead of answering
  this check.

### Procedure

1. Build the mockup with a relay between host A and host B.
2. Let host A send the datagram. The relay removes its data.
3. Read the length field at host B and watch what host B's UDP hands upward.

### Expected observations

1. The datagram on the path before the relay, with a length field of 108.
2. The datagram at host B's interface with a length field of 8, the minimum.
3. Host B's UDP hands zero octets upward to the program on port 5000.

### Notes

- The check exists because a sender cannot produce this datagram. The level 2 pass observed
  the length rule for datagrams that carry data and recorded the minimum as unreached; the
  relay is what closes that gap.
- The send half of the rule, that a sender counts header and data in the field, belongs to
  [Datagram delivery](#datagram-delivery).
