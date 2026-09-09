# UDP — English check procedures: input validation

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc768/catalog.md](../../../standard/rfc768/catalog.md), [rfc792/catalog.md](../../../standard/rfc792/catalog.md), [rfc1122/catalog.md](../../../standard/rfc1122/catalog.md)

The common mockup, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Multicast source address

Checks: **RFC1122-UADDR-1** (must).

### Requirement

RFC 1122 §4.1.3.6: a UDP datagram received with an invalid IP source address, for example a
broadcast or a multicast address, must be discarded by UDP or by the IP layer. A single
host cannot own such an address, so no answer could ever reach the sender.

### Scenario constants

- Host A sends 100 octets from port 4000 to port 5000 of host B.
- A relay rewrites the source address of the datagram to a multicast address and keeps
  every checksum valid, so that no other rule can explain a discard.
- A program on host B listens on port 5000 on purpose.

### Procedure

1. Build the mockup with a relay between host A and host B.
2. Let host A send the datagram. The relay rewrites its source address.
3. Observe the datagram at host B, then watch host B.

### Expected observations

1. The datagram at host B's interface, carrying the multicast source address.
2. Host B discards it, at the network layer or at UDP. The requirement names both layers
   and demands only that one of them acts.
3. Host B hands nothing to the program on port 5000, and sends nothing back.

### Notes

- The requirement allows either layer to do the discard, so the check accepts a record from
  either one. Which layer acts is a design choice and no statement of the in-scope set
  makes it.
- The IPv4 checks hold the same rule for the broadcast form, seen from the network layer
  ([RFC1122-ADDR-3](../../../standard/rfc1122/catalog.md#rfc1122-addr-3)). This check uses
  the multicast form, which §4.1.3.6 names first, and watches the UDP end of it.
