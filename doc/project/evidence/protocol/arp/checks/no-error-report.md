# ARP — English check procedures: no error report

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc1122/catalog.md](../../../standard/rfc1122/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## No destination unreachable

Checks: **RFC1122-ANOERR-1** (must not).

### Requirement

RFC 1122 §2.4: the link layer must not report a Destination Unreachable error to IP solely
because there is no ARP cache entry for a destination. The requirements summary of §2.5
puts the statement in the MUST NOT column.

### Scenario constants

- The mockup is the absent neighbour. Host A sends to 10.0.0.99, a protocol address on
  link 1 that no station owns.
- Host A sends one datagram of 100 octets every 0.5 s, from 0.1 s, for the whole run.
- The run lasts 10 s, long enough that every resolution of every datagram has started and
  given up.
- No station ever answers a request for that address.

### Procedure

1. Build the mockup with two hosts on one Ethernet link.
2. Let host A send its datagrams to the address that nobody owns.
3. Watch link 1 and host A for any ICMP message, for the whole run.

### Expected observations

1. Requests for 10.0.0.99 leave host A: the resolution really was attempted.
2. No reply for that address crosses link 1: the resolution really failed.
3. No ICMP Destination Unreachable message leaves host A, and none reaches the program that
   sent the datagrams, at any time in the run.

### Notes

- Observations 1 and 2 make the absence in observation 3 mean something. Without them a
  host that sent nothing at all would pass.
- The statement forbids one reason for the report and not the report itself. A host may
  send a Destination Unreachable for a destination it cannot route, or for a port with no
  listener. The scenario gives it neither cause: the address is on the link, so it is
  routable, and no datagram reaches any host, so no port can be closed. The missing table
  entry is the only possible reason left, and that reason is the forbidden one.
- The observation watches for any ICMP message and not for the Destination Unreachable type
  alone. Nothing in this scenario gives a host a reason to send any ICMP message, so the
  wider watch costs nothing and catches a host that reports the failure as another type.
- The run must be long enough for the resolution to give up. A host that is still retrying
  has not yet decided anything, and a check that stopped there would pass a host that
  reports the failure a moment later. Ten seconds is long for a resolution that RFC 1122
  limits to one request per second.
