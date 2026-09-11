# DHCP — English check procedures: the declined address

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc2131/catalog.md](../../../standard/rfc2131/catalog.md), [rfc2132/catalog.md](../../../standard/rfc2132/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Duplicate address declined

Checks: **RFC2131-DECL-1** (should), **RFC2131-DECL-2** (must), **RFC2131-DECL-5**
(description), **RFC2131-DECL-6** (must and must not), **RFC2131-DECL-7** (must).

### Requirement

RFC 2131 §4.4.1: the client should check the address of the DHCPACK before it uses it, for
example with an address resolution request for that address; when it broadcasts such a
request, it must fill in its own hardware address as the sender hardware address and 0 as the
sender protocol address, to avoid confusing the caches of other hosts. §3.1 and §4.4.1: if the
address appears to be in use, the client must send a DHCPDECLINE and restart the configuration
process. §4.4.4: the client broadcasts DHCPDECLINE messages. Table 5, DHCPDECLINE column: the
`requested IP address` and `server identifier` options are a `MUST`; `IP address lease time`,
`parameter request list`, `maximum message size`, `vendor class identifier` and all other
options are a `MUST NOT`; `ciaddr` is 0.

### Scenario constants

- The mockup with an occupant. The server gives out addresses from 192.168.1.100, so the first
  address it offers is 192.168.1.100, and the host O holds 192.168.1.100 from the start.
- O runs no DHCP. It answers an address resolution request for 192.168.1.100, which is what
  lets the client discover the conflict.
- This is the one check where addresses are resolved by traffic on the link and not without
  it, because the address resolution request is the stimulus.
- Observation stops 30 seconds after the start, which leaves room for the ten-second pause of
  RFC2131-DECL-3 before the restart.

### Procedure

1. Build the mockup with the server, the client and the occupant O, and give O the address the
   server will offer first.
2. Let the client start and get a DHCPACK for 192.168.1.100.
3. Observe the address resolution request the client sends for that address, O's answer, and
   the next message the client sends.

### Expected observations

1. The client gets a DHCPACK whose `yiaddr` is 192.168.1.100. This confirms the first half of
   the stimulus.
2. The client broadcasts an address resolution request for 192.168.1.100 (RFC2131-DECL-1).
3. That request carries the client's own hardware address as the sender hardware address and
   0.0.0.0 as the sender protocol address (RFC2131-DECL-7).
4. O answers the request, which tells the client that 192.168.1.100 is in use. This confirms
   the second half of the stimulus.
5. A message of type 4, DHCPDECLINE, leaves the client (RFC2131-DECL-2).
6. Its IP destination address is 255.255.255.255 (RFC2131-DECL-5).
7. Its option area holds code 50, `requested IP address`, with the value 192.168.1.100, and
   code 54, `server identifier`; and it holds no code 51, no code 55, no code 57 and no code
   60. Its `ciaddr` is 0.0.0.0 (RFC2131-DECL-6).
8. A message of type 1, DHCPDISCOVER, leaves the client after the DHCPDECLINE
   (RFC2131-DECL-2, the restart half).
9. No frame leaves the client with 192.168.1.100 as its IP source address.

### Notes

- Observation 3 is a rule about a message of a different protocol, and RFC 2131 states it
  anyway, with a reason: a sender protocol address of 192.168.1.100 in that request would
  teach every host on the subnet a binding that the client is about to give up, and the
  occupant would lose its own traffic.
- Observation 9 is the point of the whole mechanism. The client asked, learned the address was
  taken, and must not use it. A client that sent the DHCPDECLINE and then used the address
  anyway would pass observations 5 to 8.
- The check depends on observation 2, and observation 2 covers a `SHOULD`. A client that never
  probes cannot reach observations 4 to 9 at all: it has no way to learn of the conflict. So a
  failure at observation 2 does not tell whether the client would obey RFC2131-DECL-2 if it
  ever found a conflict; the results must say that, and must not read the failure as a refusal
  to send a DHCPDECLINE.
- The server's half of the mechanism, RFC2131-DECL-4, is that it marks the declined address as
  not available. Seeing it needs a second client to ask afterwards and to get a different
  address. That is a sharpening candidate; the ledger records the statement without a check.
- RFC2131-DECL-3, the pause of at least ten seconds before the restart, is a time with no
  stated distribution but with a lower bound only. The window of this check is long enough that
  a conforming pause fits inside it, and the check states no upper bound on the instant of
  observation 8.
