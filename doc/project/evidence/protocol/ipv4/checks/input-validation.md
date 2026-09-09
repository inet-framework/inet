# IPv4 checks — input validation

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [../checks.md](../checks.md), [features.md](../features.md)

The English check procedures of the feature IPV4-F-INPUT-VALIDATION: the silent discards
that RFC 1122 requires of a host. The checksum case is in [checksum.md](checksum.md). The
common mockup, the observation rules, and the index of all checks are in
[`../checks.md`](../checks.md). The procedures come from the specification only. They name
no simulation model and no code.

Every check in this file has the same shape: the relay turns a valid datagram into an
invalid one on the last link, one observation at host B's interface confirms the crafted
field, the discard record at host B is the in-time observation, and two absence watches
cover the two halves of "silently".

## Version discard

Checks: **RFC1122-VER-1** (must).

### Requirement

RFC 1122 §3.2.1.1: a datagram whose version number is not 4 must be silently discarded.

### Scenario constants

- Mockup with a relay. Application data: 100 octets, to port 5000.
- The relay sets the version field of the datagram to 5 and keeps the header checksum
  valid, so that the version is the only reason a receiver has to discard it.

### Procedure

1. Build the mockup with the relay.
2. Tell the relay to rewrite the version to 5.
3. Let host A send the datagram.
4. Observe host B's interface, host B's internet module, host B's UDP, and the messages
   that leave host B.

### Expected observations

1. At host B's interface, the datagram arrives with version 5 and destination port 5000.
   This confirms the stimulus.
2. Host B discards the datagram: a discard is recorded at host B's internet module
   (RFC1122-VER-1).
3. Host B's UDP never receives the datagram.
4. No ICMP message leaves host B.

The check passes if observations 1 and 2 occur in this order and observations 3 and 4
record nothing within the time limit.

### Notes

- A version of 5 names no protocol, so no receiver can process the datagram as anything.
  The check does not ask what a receiver does with version 6.

## Foreign destination

Checks: **RFC1122-ADDR-2** (must).

### Requirement

RFC 1122 §3.2.1.3: a host must silently discard an incoming datagram that is not destined
for it. A datagram is destined for the host when its destination address is one of the
host's addresses, a broadcast address valid for the connected network, or a multicast group
the host joined on that interface.

### Scenario constants

- Mockup with a relay. Application data: 100 octets, to port 5000.
- The relay rewrites the destination address to 192.168.99.99, an address that no node of
  the mockup owns and that is neither broadcast nor multicast, and keeps the checksum
  valid. The frame still reaches host B's interface.

### Procedure

1. Build the mockup with the relay.
2. Tell the relay to rewrite the destination address.
3. Let host A send the datagram.
4. Observe host B's interface, host B's internet module, host B's UDP, and every datagram
   that leaves host B.

### Expected observations

1. At host B's interface, the datagram arrives with destination 192.168.99.99. Record its
   identification. This confirms the stimulus.
2. Host B discards the datagram: a discard is recorded at host B's internet module
   (RFC1122-ADDR-2).
3. Host B's UDP never receives the datagram.
4. Nothing leaves host B on account of the datagram: neither a forwarded copy with its
   identification nor an ICMP message.

The check passes if observations 1 and 2 occur in this order and observations 3 and 4
record nothing within the time limit.

### Notes

- Observation 4 has two halves for a reason: a host that forwards the datagram acts as a
  gateway without being one, and a host that answers it with Destination Unreachable
  reports about traffic that was never addressed to it.

## Invalid source address

Checks: **RFC1122-ADDR-3** (must), **RFC1122-ADDR-4** (must not; the form used).

### Requirement

RFC 1122 §3.2.1.3: a host must silently discard an incoming datagram whose source address
is invalid by the rules of that section. The limited broadcast address must not be used as
a source address. The validation may happen in the IP layer or in each transport protocol.

### Scenario constants

- Mockup with a relay. Application data: 100 octets, to port 5000, on which a program
  listens at host B.
- The relay rewrites the source address to 255.255.255.255 and keeps the checksum valid.

### Procedure

1. Build the mockup with the relay.
2. Tell the relay to rewrite the source address.
3. Let host A send the datagram.
4. Observe host B's interface, host B's internet module and UDP, the program on host B, and
   the messages that leave host B.

### Expected observations

1. At host B's interface, the datagram arrives with source 255.255.255.255 and destination
   port 5000. This confirms the stimulus.
2. Host B discards the datagram: a discard is recorded at host B's internet module or at
   host B's UDP (RFC1122-ADDR-3 allows either layer).
3. The program on host B never receives the data.
4. No ICMP message leaves host B.

The check passes if observations 1 and 2 occur in this order and observations 3 and 4
record nothing within the time limit.

### Notes

- The port is open on purpose. With a closed port the datagram would be discarded for the
  port, and the check would not tell whether the source address was examined.
- Observation 4 is also the subject of [no error for an invalid
  source](error-report.md#no-error-for-an-invalid-source), which uses a closed port so that
  a report would otherwise be due. The two checks are kept apart so that a failure of the
  discard does not hide the result of the suppression rule.

## Unknown ICMP type

Checks: **RFC1122-ICMP-1** (must).

### Requirement

RFC 1122 §3.2.2: if an ICMP message of unknown type is received, it must be silently
discarded.

### Scenario constants

- Mockup with a relay. Host A sends one ICMP echo request of 56 octets of data to host B.
- The relay rewrites the type of the echo request to 42, a type number that names no ICMP
  message, and keeps the ICMP checksum valid.

### Procedure

1. Build the mockup with the relay.
2. Tell the relay to rewrite the ICMP type.
3. Let host A send the echo request.
4. Observe host B's interface and every ICMP message that leaves host B.

### Expected observations

1. At host B's interface, an ICMP message of type 42 arrives. This confirms the stimulus.
2. Host B discards the message: a discard is recorded at host B (RFC1122-ICMP-1).
3. No ICMP message leaves host B: neither an echo reply nor an error report.

The check passes if observations 1 and 2 occur in this order and observation 3 records
nothing within the time limit.

### Notes

- The echo request is only the vehicle. The echo function itself is out of the IPv4 scope;
  see the standards map.
- A receiver that stops with an error on an unknown type fails the check as surely as one
  that answers it: "silently discarded" leaves the host running.
