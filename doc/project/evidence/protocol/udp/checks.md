# UDP — English check procedures

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rfc768/catalog.md](../../standard/rfc768/catalog.md), [rfc792/catalog.md](../../standard/rfc792/catalog.md)

Step 5 artifact of the standards test workflow. This file holds the English check
procedures for UDP, one section per check. The procedures come from the specification only.
They name no simulation model and no code.

## Common mockup

Two hosts on one link. For the checksum check a third host joins, on a second link of host
B, so that two senders with different checksum settings reach one receiver.

```
   host A  ------------  host B  ------------  host C
            link 1                link 2
```

- Host A and host C send. Host B receives; it runs one program per open port.
- Timing: the senders start shortly after the start. Every expected event occurs within 1
  second; observation stops after 1 second.
- The data is carried by UDP over IPv4 with a 20-octet IP header and no options. A UDP
  datagram with N octets of data is 8 + N octets long and rides in an IP datagram of
  28 + N octets.
- The programs on host B are receivers that open one port each. No program on host B opens
  port 6000.
- Where a check tells two datagrams apart at the receiver, it uses their sizes: 100 octets
  and 200 octets. Nothing in UDP depends on those sizes.

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

## Port unreachable

Checks: **RFC792-DU-3** (may). Also covers: **RFC768-HDR-2** (the closed-port half).

### Requirement

RFC 792: if the destination host cannot deliver the datagram because the indicated port is
not active, the destination host may send a destination unreachable message, code 3, to
the source. RFC 768: the destination port selects the receiver; a port that no program
opened selects none.

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
3. Host A receives an ICMP destination unreachable message, type 3, code 3 (RFC792-DU-3).
4. From then on, host B's UDP hands nothing upward.

### Notes

- Observation 3 checks a `may` clause. Silence also conforms; a model that stays silent
  gets a `declined` in the matrix, not a defect.
- The discard record in observation 2 plays the role that the gateway's discard plays in
  the IPv4 checks: the positive event that precedes the report, so that the negative
  observation is not anchored too late.
