# IPv6 checks — input validation

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [../checks.md](../checks.md), [features.md](../features.md)

The English check procedures of the feature IPV6-F-INPUT-VALIDATION: what a node must not
process, and what it does instead. The common mockup, the observation rules, and the index
of all checks are in [`../checks.md`](../checks.md). The procedures come from the
specification only. They name no simulation model and no code.

## Zero UDP checksum

Checks: **RFC8200-CKSUM-1**, the discard half (must).

### Requirement

RFC 8200 §8.1: IPv6 receivers must discard UDP packets containing a zero checksum, and
should log the error.

### Scenario constants

- Mockup with a relay. Application data: 100 octets, to port 5000, on which a program
  listens at host B. Every node computes UDP checksums.
- The relay rewrites the UDP checksum field of the datagram to 0.

### Procedure

1. Build the mockup with the relay.
2. Tell the relay to rewrite the UDP checksum to 0.
3. Let host A send the datagram.
4. Observe host B's interface, host B's UDP, and the program on host B.

### Expected observations

1. At host B's interface, the datagram arrives with a UDP checksum of 0 and destination
   port 5000. This confirms the stimulus.
2. Host B's UDP discards the datagram: a discard is recorded at host B's UDP
   (RFC8200-CKSUM-1).
3. The program on host B never receives the data.

The check passes if observations 1 and 2 occur in this order and observation 3 records
nothing within the time limit.

### Notes

- The port is open on purpose: a closed port would discard the datagram for another reason.
- Whether the receiver discards the datagram because the field is zero or because a
  computed checksum does not match is not visible from outside; both satisfy the
  observable.

## Unrecognized next header

Checks: **RFC8504-NR-6** (must), which governs **RFC8200-EXT-3** (should).

### Requirement

RFC 8200 §4: a destination that meets a next header value it does not recognize should
discard the packet and send Parameter Problem code 1 to the source, with the pointer at the
offending field. RFC 8504 §5.2: this action must be taken, for an unrecognized upper-layer
protocol as well as for an unrecognized extension header.

### Scenario constants

- Mockup with a relay. Application data: 100 octets, to port 5000.
- The relay rewrites the next header field of the packet to 253, a protocol number that
  RFC 3692 reserves for experiments and that host B does not implement. The relay also
  removes host B's report on its way back to host A, because what host A does with a
  report about an unknown protocol is a check of its own
  ([error for an unknown protocol](error-report.md#error-for-an-unknown-protocol)); this
  check judges host B.

### Procedure

1. Build the mockup with the relay.
2. Tell the relay to rewrite the next header to 253, and to remove the report that returns.
3. Let host A send the datagram.
4. Observe host B's interface, host B's own origination of messages, the messages that
   leave host B, and host B's UDP.

### Expected observations

1. At host B's interface, the packet arrives with next header 253. This confirms the
   stimulus.
2. Host B originates a Parameter Problem message, type 4 code 1: recorded at host B at the
   moment of the decision (RFC8504-NR-6, RFC8200-EXT-3).
3. That message leaves host B, type 4 code 1, addressed to host A.
4. From then on, nothing of the packet reaches UDP at host B.

The check passes if observations 1 to 3 occur in this order and observation 4 records
nothing within the time limit.

### Notes

- The pointer field, which should name the offset of the next header field (6), is not
  asserted; it is a sharpening candidate.

## Unassigned next header

Checks: **RFC8504-NR-6** (must), which governs **RFC8200-EXT-3** (should), for a value that
nobody has assigned.

### Requirement

The same as [unrecognized next header](#unrecognized-next-header). RFC 8504 §5.2 says the
action applies whether the value is an unrecognized extension header or an unrecognized
upper-layer protocol; a number that no registry assigns is the plainest such value.

### Scenario constants

- The same as [unrecognized next header](#unrecognized-next-header), with the next header
  rewritten to 200, a number that IANA has not assigned.

### Procedure

The same as the previous check, with 200.

### Expected observations

The same four observations as the previous check, with next header 200: the packet arrives
with 200, host B originates Parameter Problem code 1, the message leaves host B toward host
A, and nothing reaches UDP.

### Notes

- Two values because a node may know one number by name without implementing it and have
  never heard of the other; RFC 8200 asks for the same action in both cases.

## Unknown ICMPv6 error type

Checks: **RFC4443-MPR-4** (must not), for an error message of unknown type; notes
**RFC4443-MPR-1** (must; internal).

### Requirement

RFC 4443 §2.4 (a): an ICMPv6 error message of unknown type received at its destination
must be passed to the upper-layer process that sent the invoking packet. §2.4 (e.1): no
error message is originated about an error message, whatever its type.

### Scenario constants

- Mockup with a relay. Host A sends 100 octets to port 5001 on host B; no program listens,
  so host B returns Destination Unreachable to host A.
- The relay rewrites the type of that message from 1 to 100, a value below 128 that names
  no message. Host A receives an error message of unknown type.

### Procedure

1. Build the mockup with the relay.
2. Tell the relay to rewrite the ICMPv6 type of the report to 100.
3. Let host A send the datagram.
4. Observe host A's interface and every ICMPv6 message host A sends.

### Expected observations

1. At host A's interface, an ICMPv6 message of type 100 arrives. This confirms the
   stimulus and is the anchor; the handoff to the upper-layer process that RFC4443-MPR-1
   requires happens inside the node and is not observed here.
2. From then on, host A sends no ICMPv6 error message (RFC4443-MPR-4), and it keeps
   running.

The check passes if observation 1 occurs and observation 2 records nothing within the time
limit.

## Unknown ICMPv6 informational type

Checks: **RFC4443-MPR-2** (must).

### Requirement

RFC 4443 §2.4 (b): an ICMPv6 informational message of unknown type must be silently
discarded.

### Scenario constants

- Mockup with a relay. Host A sends one echo request of 56 octets of data to host B, as the
  vehicle.
- The relay rewrites the type of the echo request from 128 to 200, a value of 128 or above
  that names no message.

### Procedure

1. Build the mockup with the relay.
2. Tell the relay to rewrite the ICMPv6 type to 200.
3. Let host A send the echo request.
4. Observe host B's interface and every ICMPv6 message host B sends.

### Expected observations

1. At host B's interface, an ICMPv6 message of type 200 arrives. This confirms the
   stimulus and is the anchor.
2. From then on, host B sends no ICMPv6 message in response: neither an echo reply nor an
   error report (RFC4443-MPR-2), and it keeps running.

The check passes if observation 1 occurs and observation 2 records nothing within the time
limit.

### Notes

- "Silently discarded" leaves the host running; a node that stops on an unknown type fails
  the check as surely as one that answers it.
