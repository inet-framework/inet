# RIP — English check procedures: the table request

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc2453/catalog.md](../../../standard/rfc2453/catalog.md), [rfc2080/catalog.md](../../../standard/rfc2080/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Table request of a restarted router (RIP version 2)

Checks: **RFC2453-REQ-1**, **REQ-4**, **REQ-8**, **OUT-1**, **MSG-4** (description),
**RFC2453-REQ-6** (must), **RFC2453-GEN-1** (should).

### Requirement

RFC 2453 §3.9.1: a router that comes up multicasts a request for the complete table on every
connected network, from the RIP port. A request with exactly one entry, of address family 0
and metric 16, asks for the whole table, and the router sends its table to the address and
port of the requester, through normal output processing, split horizon included. §3.10: this
response is unicast to the requester. §3.10.2: the response to a request should carry the
version of the request.

### Scenario constants

- The pair. R2 fails at 100 seconds, without notice, and starts again at 110 seconds.
- R1 learned netB from R2 over L1 before the failure. The timeout of that route is 180
  seconds, so R1 still holds it, with metric 2, when R2 comes back.
- Observation lasts 130 seconds.

### Procedure

1. Build the pair, and let both routers start.
2. At 100 seconds, let R2 fail; at 110 seconds, let R2 start again.
3. Observe the messages of R2 on L1 and on netB, and of R1 on L1.

### Expected observations

1. On L1, from R2, before 100 seconds, an update that holds netB with metric 1. This confirms
   the first half of the stimulus: R1 has a route that it learned from R2 over L1.
2. On L1, from R2, after 110 seconds, a request: command 1, version 2, exactly one entry,
   which has address family identifier 0 and metric 16; UDP source port 520, UDP destination
   port 520, IP destination address 224.0.0.9 (RFC2453-REQ-1, REQ-4). This confirms the
   second half of the stimulus.
3. On netB, from R2, within 1 second of observation 2, a request of the same form
   (RFC2453-REQ-8: on every connected network).
4. On L1, from R1, within 5 seconds of observation 2, a response to the IP address of R2 on
   L1, 10.0.12.2, and to UDP port 520, the port the request came from (RFC2453-OUT-1, MSG-4,
   REQ-4).
5. That response has version 2 (RFC2453-GEN-1) and holds netA with metric 1.
6. That response does not hold netB with a metric below 16 (RFC2453-REQ-6: split horizon in
   the answer).

### Notes

- The failure of observation 1 is a crash and not an orderly stop, so that R2 sends nothing
  that would change the route of R1 to netB. R1 therefore holds netB with a finite metric when
  the request comes, and the split horizon of observation 6 has something to hide.
- Observation 4 selects the answer by its unicast destination. A periodic update goes to the
  multicast group, so it cannot be mistaken for the answer.
- Observation 5 reads a `should`. It stays in the check because version 2 is the only
  version that the pair uses, so a response in another version is a finding.

## Table request of a restarted router (RIPng)

Checks: **RFC2080-REQ-1**, **REQ-4**, **REQ-8**, **OUT-1**, **MSG-4** (description),
**RFC2080-REQ-6** (must); covers **RFC2080-GEN-1** (must).

### Requirement

RFC 2080 §2.4.1: a router that comes up multicasts a request on every connected network, from
the RIPng port. A request with exactly one entry, of prefix zero, prefix length zero and
metric 16, asks for the whole table, and the router sends its table to the address and port
of the requester, split horizon included. §2.5: the response goes to the unicast address of
the requester only. §2.5.2: it leaves from a link-local address, because the request came
from the RIPng port.

### Scenario constants

- The pair, on IPv6. R2 fails at 100 seconds and starts again at 110 seconds.
- Observation lasts 130 seconds.

### Procedure

1. Build the pair on IPv6, and let both routers start.
2. At 100 seconds, let R2 fail; at 110 seconds, let R2 start again.
3. Observe the messages of R2 on L1 and on netB, and of R1 on L1.

### Expected observations

1. On L1, from R2, before 100 seconds, an update that holds netB with metric 1. This confirms
   the first half of the stimulus.
2. On L1, from R2, after 110 seconds, a request: command 1, version 1, exactly one entry,
   which has prefix ::, prefix length 0 and metric 16; UDP source port 521, UDP destination
   port 521, IPv6 destination address FF02::9 (RFC2080-REQ-1, REQ-4). Record the source
   address of the request.
3. On netB, from R2, within 1 second of observation 2, a request of the same form
   (RFC2080-REQ-8).
4. On L1, from R1, within 5 seconds of observation 2, a response to the source address of the
   request and to UDP port 521 (RFC2080-OUT-1, MSG-4, REQ-4).
5. That response leaves from a link-local address (covers RFC2080-GEN-1) and holds netA with
   metric 1.
6. That response does not hold netB with a metric below 16 (RFC2080-REQ-6).
