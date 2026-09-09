# IPv4 checks — error reports and their suppression

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [../checks.md](../checks.md), [features.md](../features.md)

The English check procedures of the features IPV4-F-ERROR-REPORT and
IPV4-F-ERROR-SUPPRESSION. The two gateway reports of RFC 792 are asserted inside
[TTL expiry](ttl.md#ttl-expiry) and [don't fragment](fragmentation.md#dont-fragment); this
file holds the host report and the five cases in which no report may be sent. The common
mockup, the observation rules, and the index of all checks are in
[`../checks.md`](../checks.md). The procedures come from the specification only. They name
no simulation model and no code.

The five suppression checks share one shape: a datagram that would normally earn an error
report arrives in one of the five forbidden forms; the node's own record of the event (a
discard, or a port that no program opened) is the in-time observation; and an absence watch
opens at that moment and covers every ICMP message the node could send.

## Host error report

Checks: **RFC1122-ERR-1** (must, "wherever practical"), **RFC1122-DU-1** (should),
**RFC1122-ICMP-2** (must: the quote is unchanged), **RFC1122-ICMP-4** (should: TOS zero).

### Requirement

RFC 1122 §3.3.8: wherever practical, a host must return an ICMP error message when it
detects an error. §3.2.2.1: a host should send Destination Unreachable with code 3 when the
transport protocol cannot demultiplex the datagram and has no way of its own to tell the
sender. §3.2.2: every ICMP error message quotes the internet header and at least the first
8 data octets of the datagram that caused it, and this header and data must be unchanged;
an error message should carry zero TOS bits.

### Scenario constants

- Common mockup. Application data: 100 octets, from port 4000 on host A to port 5001 on
  host B. No program listens on port 5001.

### Procedure

1. Build the mockup network.
2. Let host A send the datagram.
3. Observe link 1, host B's UDP, and the messages that return to host A.

### Expected observations

1. On link 1, the datagram appears with destination port 5001. Record its identification,
   its source and destination addresses, and its ports.
2. Host B's UDP finds no program for port 5001: a discard for want of a port is recorded at
   host B's UDP. This confirms the stimulus and is the in-time observation.
3. Host A receives a Destination Unreachable message, type 3, code 3 (RFC1122-ERR-1,
   RFC1122-DU-1), whose type of service byte is 0 (RFC1122-ICMP-4).
4. The quoted internet header of that message carries the recorded identification, source
   address, destination address and protocol 17, and the 8 octets after the header are the
   UDP header with source port 4000 and destination port 5001 (RFC1122-ICMP-2).

The check passes if all four observations occur in this order within the time limit.

### Notes

- Observations 3 and 4 concern one message; the second reads deeper into the same message.
- RFC1122-DU-1 is a should. A host that stays silent declines it, and then RFC1122-ERR-1
  asks whether a report was practical; for a closed UDP port it is, so silence would count
  against the must as well.
- RFC 792 lets the message quote exactly 8 octets after the header; RFC 1122 lets it quote
  more. The check reads the first 8 and accepts more.

## No error about an error

Checks: **RFC1122-ICMP-5** (must not).

### Requirement

RFC 1122 §3.2.2: an ICMP error message must not be sent as the result of receiving an ICMP
error message.

### Scenario constants

- Common mockup. Host A sends 100 octets to port 5001 on host B; no program listens.
- Host B's default TTL, which its own error messages carry, is 1. The Destination
  Unreachable that host B returns therefore expires at the gateway, which must discard it
  and must not report the expiry with Time Exceeded.

### Procedure

1. Build the mockup network.
2. Set the default TTL of host B to 1.
3. Let host A send the datagram.
4. Observe link 1, link 2 in the direction of the gateway, the gateway's internet module,
   and every ICMP message the gateway sends toward host B.

### Expected observations

1. On link 1, the datagram appears with destination port 5001.
2. On link 2, host B's Destination Unreachable, type 3, code 3, appears with TTL = 1. Record
   the identification of the datagram that carries it. This confirms the stimulus: an error
   message that is about to expire.
3. The gateway discards it: a discard of that datagram is recorded at the gateway's
   internet module.
4. From that moment, the gateway sends no Time Exceeded message toward host B, and host B
   receives none (RFC1122-ICMP-5).

The check passes if observations 1 to 3 occur in this order and observation 4 records
nothing within the time limit.

### Notes

- The scenario needs no relay: a default TTL of 1 on host B is ordinary configuration.
- The gateway's discard of the expired datagram is RFC791-TTL-2 and is not asserted again
  here; observation 3 serves as the anchor of the absence watch.

## No error for a broadcast

Checks: **RFC1122-ICMP-6** (must not).

### Requirement

RFC 1122 §3.2.2: an ICMP error message must not be sent as the result of receiving a
datagram destined to an IP broadcast or IP multicast address.

### Scenario constants

- Mockup on one link: host A and host B share link 1.
- Host A sends 100 octets to the limited broadcast address 255.255.255.255, port 5001. No
  program listens on port 5001 at host B.

### Procedure

1. Build the one-link mockup.
2. Let host A send the datagram to the limited broadcast address.
3. Observe link 1, host B's UDP, and every ICMP message host B sends.

### Expected observations

1. On link 1, the datagram appears with destination 255.255.255.255 and port 5001.
2. Host B's UDP finds no program for port 5001: a discard for want of a port is recorded at
   host B's UDP. This confirms that the datagram reached the point where a report would be
   due.
3. From that moment, host B sends no ICMP message, and host A receives none
   (RFC1122-ICMP-6).

The check passes if observations 1 and 2 occur in this order and observation 3 records
nothing within the time limit.

### Notes

- This is the broadcast storm case that RFC 1122 gives as its example: one broadcast
  datagram to a closed port, and every host on the link answers.
- A subnet-directed broadcast and a multicast destination are variants for a later pass.

## No error for a link-layer broadcast

Checks: **RFC1122-ICMP-7** (must not).

### Requirement

RFC 1122 §3.2.2: an ICMP error message must not be sent as the result of receiving a
datagram sent as a link-layer broadcast. The implementation note of that section says why
the rule exists beside the previous one: some hosts broadcast a datagram whose IP
destination is a unicast address, and the link layer must tell the IP layer that a frame
was a broadcast.

### Scenario constants

- Mockup with a relay. Host A sends 100 octets to port 5001 on host B; no program listens.
- The relay rewrites the destination address of the link-layer frame that carries the
  datagram to the broadcast address of the link layer. The IP destination stays host B's
  unicast address.

### Procedure

1. Build the mockup with the relay.
2. Tell the relay to rewrite the link-layer destination of the datagram's frame.
3. Let host A send the datagram.
4. Observe host B's interface, host B's UDP, and every ICMP message host B sends.

### Expected observations

1. At host B's interface, the datagram arrives in a frame whose link-layer destination is
   the broadcast address, with IP destination host B and port 5001. This confirms the
   stimulus.
2. Host B's UDP finds no program for port 5001: a discard for want of a port is recorded at
   host B's UDP.
3. From that moment, host B sends no ICMP message (RFC1122-ICMP-7).

The check passes if observations 1 and 2 occur in this order and observation 3 records
nothing within the time limit.

### Notes

- The only difference from [host error report](#host-error-report) is the frame's
  link-layer destination. A host that reports here does not consult the link layer, which
  is exactly what the implementation note of RFC 1122 §3.2.2 asks for.

## No error for a non-initial fragment

Checks: **RFC1122-ICMP-8** (must not).

### Requirement

RFC 1122 §3.2.2: an ICMP error message must not be sent as the result of receiving a
non-initial fragment. RFC 792 lets a gateway report an expired datagram with Time Exceeded;
when the datagram arrives in fragments, at most the first fragment may earn the report.

### Scenario constants

- Mockup with two gateways. Link 2, between R1 and R2, has MTU 576; links 1 and 3 keep the
  default.
- Host A sends 1000 octets to port 5000 with TTL 2. R1 decrements the TTL to 1 and splits
  the datagram into two fragments (572 and 476 octets, offsets 0 and 552). R2 decrements
  each fragment to 0 and must discard both. It may report the first; it must not report the
  second.

### Procedure

1. Build the two-gateway mockup with MTU 576 on link 2.
2. Set the sender TTL on host A to 2.
3. Let host A send the datagram.
4. Observe link 1, link 2, R2's internet module, and every ICMP message R2 sends.

### Expected observations

1. On link 1, the datagram appears with TTL = 2.
2. On link 2, the first fragment appears: offset 0, MF = 1, TTL = 1. Record its
   identification.
3. R2 discards the first fragment: a discard of the fragment with offset 0 is recorded at
   R2's internet module. This happens while R1 still transmits the last fragment.
4. On link 2, the last fragment appears: the same identification, offset 552, MF = 0,
   TTL = 1.
5. R2 discards the last fragment: a discard of the fragment with offset 552 is recorded at
   R2's internet module.
6. From that moment, R2 sends no ICMP error message whose quoted header has a fragment
   offset other than 0 (RFC1122-ICMP-8).

The check passes if observations 1 to 5 occur in this order and observation 6 records
nothing within the time limit.

### Notes

- Right after observation 3, R2 may send one Time Exceeded that quotes the first fragment
  (RFC792-TE-1, a may). The check neither requires nor forbids it.
- Observation 6 reads the quoted header, not the outer one: the outer datagram of an error
  report is never a fragment.

## No error for an invalid source

Checks: **RFC1122-ICMP-9** (must not).

### Requirement

RFC 1122 §3.2.2: an ICMP error message must not be sent as the result of receiving a
datagram whose source address does not define a single host, for example a broadcast
address.

### Scenario constants

- Mockup with a relay. Host A sends 100 octets to port 5001 on host B; no program listens.
- The relay rewrites the source address to 255.255.255.255 and keeps the checksum valid.

### Procedure

1. Build the mockup with the relay.
2. Tell the relay to rewrite the source address.
3. Let host A send the datagram.
4. Observe host B's interface, host B's internet module and UDP, and every ICMP message
   host B sends.

### Expected observations

1. At host B's interface, the datagram arrives with source 255.255.255.255 and port 5001.
   This confirms the stimulus.
2. Host B records a discard of the datagram: either at its internet module, as an invalid
   source (RFC1122-ADDR-3), or at its UDP, for want of a port.
3. From that moment, host B sends no ICMP message (RFC1122-ICMP-9).

The check passes if observations 1 and 2 occur in this order and observation 3 records
nothing within the time limit.

### Notes

- Whichever layer discards the datagram, the report is forbidden. A host that discards it
  at the IP layer satisfies [invalid source address](input-validation.md#invalid-source-address)
  as well; a host that lets it through to UDP still must not answer.
