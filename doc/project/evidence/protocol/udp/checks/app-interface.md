# UDP — English check procedures: application interface

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc768/catalog.md](../../../standard/rfc768/catalog.md), [rfc792/catalog.md](../../../standard/rfc792/catalog.md), [rfc1122/catalog.md](../../../standard/rfc1122/catalog.md)

The common mockup, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## TTL and TOS from the program

Checks: **RFC1122-UAPI-1** (must).

### Requirement

RFC 1122 §4.1.4: an application-layer program must be able to set the TTL and TOS values as
well as IP options for sending a UDP datagram, and these values must be passed transparently
to the IP layer. Transparently means unchanged: the value the program names is the value
that leaves the host.

### Scenario constants

- Host A sends 100 octets from port 4000 to port 5000 of host B.
- The sending program names a TTL and a TOS. Both values differ from the ones a host would
  use on its own, so that the datagram cannot carry them by chance.

### Procedure

1. Build the mockup with host A and host B.
2. Let the program on host A send the datagram with the named TTL and TOS.
3. Read the TTL field and the TOS field of the datagram on link 1.

### Expected observations

1. On link 1, a datagram to port 5000 whose TTL field holds the value the program named.
2. The same datagram carries the TOS value the program named.
3. Host B's UDP hands the 100 octets upward, so the two values did not stop the delivery.

### Notes

- The check reads the datagram on the first link, where no gateway has yet decremented the
  TTL. A value read after a gateway would test the TTL rule instead of this one.
- The third value of the requirement, the IP options, needs an interface for options that
  the scenario tooling does not have to offer. The results record what was reachable.

## Source address from the program

Checks: **RFC1122-UMH-2** (must).

### Requirement

RFC 1122 §4.1.3.5: an application program must be able to specify the IP source address to
be used for sending a UDP datagram, or to leave it unspecified, in which case the networking
software chooses one.

### Scenario constants

- Host A has two interfaces with a different address on each, one on link 1 and one on
  link 3.
- The sending program on host A names the address of the interface that is **not** on the
  path to host B, and sends 100 octets to port 5000 of host B.

### Procedure

1. Build the mockup with host A, host B and the second link of host A.
2. Let the program send the datagram with the named source address.
3. Read the source address of the datagram on link 1.

### Expected observations

1. On link 1, a datagram to port 5000 whose source address is the one the program named.
2. Host B's UDP hands the 100 octets upward.

### Notes

- The program names the address of the other interface on purpose. If it named the address
  that the host would choose anyway, the check could not tell obedience from chance.
- Both addresses belong to host A, so the datagram still obeys
  [Valid source address](delivery.md#valid-source-address); the two checks do not conflict.
