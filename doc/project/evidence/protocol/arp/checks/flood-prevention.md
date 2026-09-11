# ARP — English check procedures: flood prevention

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc1122/catalog.md](../../../standard/rfc1122/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## The request rate

Checks: **RFC1122-AFLOOD-1** (must).

### Requirement

RFC 1122 §2.3.2.1: a mechanism to prevent ARP flooding — repeatedly sending an ARP request
for the same IP address, at a high rate — must be included. The recommended maximum rate is
one per second per destination. The mechanism is a MUST; the rate is a recommendation.

### Size and value arithmetic

The check needs a destination that never answers, and a stream of datagrams fast enough
that the datagrams cannot set the rate themselves. With one datagram every 0.1 s over 10
seconds, a host with no mechanism at all would send about 100 requests. The recommended
maximum over the same window is

    10 s × 1 request per second = 10 requests

so the two predictions are an order of magnitude apart, and the count separates them with
no need for a tolerance. The check allows the recommended maximum plus one, to let a host
send its first request at the moment the first datagram arrives.

### Scenario constants

- The mockup is the absent neighbour. Host A sends to 10.0.0.99, a protocol address on
  link 1 that no station owns.
- Host A sends one datagram of 100 octets every 0.1 s, from 0.1 s to the end of the run.
- The run lasts 10 s, and the count window is the whole run.
- No station ever answers a request for that address.

### Procedure

1. Build the mockup with two hosts on one Ethernet link.
2. Let host A send its stream of datagrams to the address that nobody owns.
3. Count every ARP request that host A puts on link 1, from the start to the end of the
   run.

### Expected observations

1. At least one request for 10.0.0.99 leaves host A near 0.1 s.
2. No reply for that address ever crosses link 1.
3. The number of requests for that address over the 10-second run is at most 11.

### Notes

- Observation 1 proves the stimulus and observation 2 proves the failure: the resolution
  really was retried, because nothing ever satisfied it.
- Observation 3 uses the recommended rate as the bound. A host that chose a lower rate also
  passes; a host that chose a higher one fails the observation while still satisfying the
  MUST, because the MUST is about the mechanism and not the number. The results say so if
  that case appears, and the verdict then belongs to the recommendation and not to the
  requirement.
- The address must be on the link and unowned. An address off the link would be resolved to
  a gateway that answers, and the resolution would succeed; an address that a station owns
  would be answered once and never asked again.
- This is the one check whose subject is a rate. It stays at level 3 because the rate is a
  bound on a count and not a distribution: the check asks how many requests fit in a known
  window, and needs no statistical test. The timer that produces the rate is level 4 work;
  see [`standards.md`](../standards.md#target-level).
