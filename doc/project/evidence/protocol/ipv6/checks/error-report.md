# IPv6 checks — error reports and their suppression

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [../checks.md](../checks.md), [features.md](../features.md)

The English check procedures of the features IPV6-F-ERROR-REPORT and
IPV6-F-ERROR-SUPPRESSION. The two router reports are asserted inside
[hop limit expiry](hop-limit.md#hop-limit-expiry) and
[packet too big](packet-too-big.md#packet-too-big); this file holds the host report, the
source address of a report, and the cases in which no report may be sent. The common
mockup, the observation rules, and the index of all checks are in
[`../checks.md`](../checks.md). The procedures come from the specification only. They name
no simulation model and no code.

The suppression checks share one shape: a packet that would normally earn an error report
arrives in one of the forbidden forms; the node's own record of the event (a port that no
program opened, or the arrival of the crafted packet) is the anchor; and an absence watch
opens at that moment and covers every ICMPv6 error message the node could send.

## Host error report

Checks: **RFC4443-DU-4** (should), **RFC4443-ERR-1** (must), **RFC4443-ERR-2**
(description), **RFC4443-SRC-1** (must).

### Requirement

RFC 4443 §3.1: a destination node should originate Destination Unreachable code 4 when the
transport protocol has no listener for the packet. §2.4 (c): every error message includes
as much of the invoking packet as fits without exceeding 1280 octets. §3.1: the destination
address of the message is copied from the source of the invoking packet. §2.2 (a): a
response to a message sent to one of the node's unicast addresses carries that address as
its source.

### Scenario constants

- Common mockup. Application data: 100 octets, from port 4000 on host A to port 5001 on
  host B. No program listens on port 5001. The datagram is 148 octets long, so the whole of
  it fits in a report: 8 + 148 = 156 octets of ICMPv6 payload.

### Procedure

1. Build the mockup network.
2. Let host A send the datagram.
3. Observe link 1, host B's UDP, and the messages that return to host A.

### Expected observations

1. On link 1, the datagram appears with destination port 5001. Record its source and
   destination addresses.
2. Host B's UDP finds no program for port 5001: a discard for want of a port is recorded at
   host B's UDP. This confirms the stimulus.
3. Host A receives a Destination Unreachable message, type 1 code 4 (RFC4443-DU-4), whose
   source address is the recorded destination of the datagram (RFC4443-SRC-1), whose
   destination address is the recorded source (RFC4443-ERR-2), and whose payload length is
   156 octets: the ICMPv6 header and the whole invoking packet (RFC4443-ERR-1).

The check passes if all three observations occur in this order within the time limit.

### Notes

- RFC4443-DU-4 is a should. A host that stays silent declines it; the other three
  statements are then unobservable in this scenario and keep their previous verdicts.

## Report source address

Checks: **RFC4443-SRC-2** (must; should).

### Requirement

RFC 4443 §2.2 (b): a message that responds to a packet sent to any address other than one
of the node's own unicast addresses carries a unicast address of the node as its source,
chosen as the source of any other packet would be. A router's report about a packet it
forwarded is such a message.

### Scenario constants

- Common mockup. Application data: 100 octets to port 5000, sender hop limit 1: the router
  discards the packet and reports it.

### Procedure

1. Build the mockup network.
2. Set the sender hop limit on host A to 1.
3. Let host A send the datagram.
4. Observe link 1 and the messages that return to host A.

### Expected observations

1. On link 1, the packet appears with hop limit 1. This confirms the stimulus.
2. Host A receives a Time Exceeded message, type 3 code 0, whose source is the router's
   own unicast address on link 1, the interface toward host A (RFC4443-SRC-2).

The check passes if both observations occur in this order within the time limit.

### Notes

- The must is "a unicast address belonging to the node"; the should is the usual selection,
  which for a report toward link 1 picks the router's address on that link. The check
  asserts the should, which implies the must.

## No error about an error

Checks: **RFC4443-MPR-4** (must not).

### Requirement

RFC 4443 §2.4 (e.1): an ICMPv6 error message must not be originated as a result of
receiving an ICMPv6 error message.

### Scenario constants

- Mockup with a relay. Host A sends 100 octets to port 5001 on host B; no program listens,
  so host B returns Destination Unreachable.
- The relay rewrites the hop limit of that report to 1 on link 2. The router decrements it
  to 0, must discard it, and must not report the expiry with Time Exceeded.

### Procedure

1. Build the mockup with the relay.
2. Tell the relay to rewrite the hop limit of the Destination Unreachable message to 1.
3. Let host A send the datagram.
4. Observe link 1, the router's interface on link 2, and every ICMPv6 message the router
   sends.

### Expected observations

1. On link 1, the datagram appears with destination port 5001.
2. The router's interface on link 2 receives the Destination Unreachable message, type 1,
   with hop limit 1. This confirms the stimulus: an error message about to expire.
3. From then on, the router sends no Time Exceeded message and no other ICMPv6 message
   about it, and it does not forward the expired report toward host A (RFC4443-MPR-4).

The check passes if observations 1 and 2 occur in this order and observation 3 records
nothing within the time limit.

### Notes

- The anchor is the arrival of the report at the router, which precedes the decision; the
  router of this mockup leaves no record of a discard that it does not report.

## Error for an unknown protocol

Checks: **RFC4443-MPR-3** (description) and **RFC4443-MPR-4** (must not), for a report
whose quoted packet names a protocol the receiving node does not implement.

### Requirement

RFC 4443 §2.4 (d): the upper-layer protocol type is extracted from the quoted packet and
used to select the process that handles the error; where no such process can be identified,
the message is dropped. §2.4 (e.1): no error message is originated about an error message.

### Scenario constants

- Mockup with a relay. Application data: 100 octets, to port 5000.
- The relay rewrites the next header of host A's datagram to 253. Host B reports the packet
  with Parameter Problem code 1, and the report, which quotes a packet of protocol 253,
  returns to host A, which implements no such protocol.

### Procedure

1. Build the mockup with the relay.
2. Tell the relay to rewrite the next header to 253.
3. Let host A send the datagram.
4. Observe host B's interface, host A's interface, and every ICMPv6 message host A sends.

### Expected observations

1. At host B's interface, the packet arrives with next header 253.
2. Host A receives host B's Parameter Problem message, type 4 code 1, which quotes a packet
   of protocol 253. This confirms the stimulus for host A and is the anchor.
3. From then on, host A sends no ICMPv6 error message and keeps running (RFC4443-MPR-3,
   RFC4443-MPR-4).

The check passes if observations 1 and 2 occur in this order and observation 3 records
nothing within the time limit.

### Notes

- Host B's half of the scenario is [unrecognized next header](input-validation.md#unrecognized-next-header).
  The two are kept apart so that a failure on either side does not hide the other.

## No error for a multicast destination

Checks: **RFC4443-MPR-6** (must not).

### Requirement

RFC 4443 §2.4 (e.3): an ICMPv6 error message must not be originated as a result of
receiving a packet destined to a multicast address, except Packet Too Big and Parameter
Problem code 2.

### Scenario constants

- Mockup on one link: host A and host B share link 1.
- Host A sends 100 octets to the all-nodes multicast address ff02::1, port 5001. No program
  listens on port 5001 at host B.

### Procedure

1. Build the one-link mockup.
2. Let host A send the datagram to ff02::1.
3. Observe link 1, host B's UDP, and every ICMPv6 message host B sends.

### Expected observations

1. On link 1, the datagram appears with destination ff02::1 and port 5001.
2. Host B's UDP finds no program for port 5001: a discard for want of a port is recorded at
   host B's UDP. This confirms that the datagram reached the point where a report would be
   due.
3. From then on, host B sends no ICMPv6 error message (RFC4443-MPR-6).

The check passes if observations 1 and 2 occur in this order and observation 3 records
nothing within the time limit.

### Notes

- The two exceptions are not exercised: a Packet Too Big for multicast needs a router that
  forwards multicast, and Parameter Problem code 2 needs an option; both are later passes.

## No error for a link-layer multicast

Checks: **RFC4443-MPR-7** (must not).

### Requirement

RFC 4443 §2.4 (e.4): an ICMPv6 error message must not be originated as a result of
receiving a packet sent as a link-layer multicast, with the same exceptions.

### Scenario constants

- Mockup with a relay. Host A sends 100 octets to port 5001 on host B; no program listens.
- The relay rewrites the destination address of the link-layer frame to the all-nodes
  multicast address of the link layer, 33:33:00:00:00:01. The IPv6 destination stays host
  B's unicast address.

### Procedure

1. Build the mockup with the relay.
2. Tell the relay to rewrite the link-layer destination of the datagram's frame.
3. Let host A send the datagram.
4. Observe host B's interface, host B's UDP, and every ICMPv6 message host B sends.

### Expected observations

1. At host B's interface, the datagram arrives in a frame whose link-layer destination is a
   multicast address, with IPv6 destination host B and port 5001. This confirms the
   stimulus.
2. Host B's UDP finds no program for port 5001: a discard for want of a port is recorded at
   host B's UDP.
3. From then on, host B sends no ICMPv6 error message (RFC4443-MPR-7).

The check passes if observations 1 and 2 occur in this order and observation 3 records
nothing within the time limit.

## No error for a link-layer broadcast

Checks: **RFC4443-MPR-8** (must not).

### Requirement

RFC 4443 §2.4 (e.5): an ICMPv6 error message must not be originated as a result of
receiving a packet sent as a link-layer broadcast, with the same exceptions.

### Scenario constants

- The same as [no error for a link-layer multicast](#no-error-for-a-link-layer-multicast),
  with the link-layer broadcast address ff:ff:ff:ff:ff:ff.

### Procedure

The same as the previous check.

### Expected observations

1. At host B's interface, the datagram arrives in a frame whose link-layer destination is
   the broadcast address, with IPv6 destination host B and port 5001.
2. Host B's UDP finds no program for port 5001: a discard for want of a port is recorded at
   host B's UDP.
3. From then on, host B sends no ICMPv6 error message (RFC4443-MPR-8).

The check passes if observations 1 and 2 occur in this order and observation 3 records
nothing within the time limit.

## No error for an unspecified source

Checks: **RFC4443-MPR-9** (must not).

### Requirement

RFC 4443 §2.4 (e.6): an ICMPv6 error message must not be originated as a result of
receiving a packet whose source address does not uniquely identify a single node, for
example the unspecified address.

### Scenario constants

- Mockup with a relay. Host A sends 100 octets to port 5001 on host B; no program listens.
- The relay rewrites the source address to the unspecified address `::`.

### Procedure

1. Build the mockup with the relay.
2. Tell the relay to rewrite the source address.
3. Let host A send the datagram.
4. Observe host B's interface, host B's internet layer and UDP, and every ICMPv6 message
   host B sends.

### Expected observations

1. At host B's interface, the datagram arrives with source `::` and port 5001. This
   confirms the stimulus.
2. Host B records a discard of the datagram: at its internet layer, or at its UDP for want
   of a port.
3. From then on, host B sends no ICMPv6 error message (RFC4443-MPR-9).

The check passes if observations 1 and 2 occur in this order and observation 3 records
nothing within the time limit.

### Notes

- A multicast source and an anycast source are variants for a later pass.
