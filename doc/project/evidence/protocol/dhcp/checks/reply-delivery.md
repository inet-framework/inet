# DHCP — English check procedures: where a reply goes

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc2131/catalog.md](../../../standard/rfc2131/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Broadcast bit honoured

Checks: **RFC2131-BCAST-4** (description), **RFC2131-BCAST-5** (description),
**RFC2131-BCAST-6** (should).

### Requirement

RFC 2131 §4.1: if `giaddr` is zero and `ciaddr` is zero and the broadcast bit is set, then the
server broadcasts DHCPOFFER and DHCPACK messages to 255.255.255.255. If the broadcast bit is not
set and `giaddr` is zero and `ciaddr` is zero, then the server unicasts them to the client's
hardware address and `yiaddr` address. A server sending a message directly to a client should
examine the BROADCAST bit in the `flags` field and send a broadcast or a unicast accordingly.

### Scenario constants

- The mockup with two clients. Client C clears the BROADCAST bit in its DHCPDISCOVER and its
  DHCPREQUEST; client D sets it. Both have `ciaddr` zero and `giaddr` zero, because neither one
  holds an address yet.
- The two clients start a tenth of a second apart, so that each reply can be attributed to the
  request it answers.
- Observation stops one second after the start.

### Procedure

1. Build the mockup with the server and two clients, one with the bit clear and one with the bit
   set.
2. Let both clients start.
3. For each client, read the top bit of the `flags` field of its DHCPDISCOVER, and then the IP
   and link-layer destination addresses of the DHCPOFFER and the DHCPACK that answer it.

### Expected observations

1. The DHCPDISCOVER of client C carries the top bit of `flags` clear, and its `ciaddr` and
   `giaddr` are 0.0.0.0. This confirms the first stimulus.
2. The DHCPDISCOVER of client D carries the top bit of `flags` set, and its `ciaddr` and
   `giaddr` are 0.0.0.0. This confirms the second stimulus.
3. The DHCPOFFER that answers client C has an IP destination address equal to its own `yiaddr`,
   and a link-layer destination address equal to the `chaddr` of that message: a unicast
   (RFC2131-BCAST-5, RFC2131-BCAST-6).
4. The DHCPACK that answers client C is a unicast in the same way (RFC2131-BCAST-5).
5. The DHCPOFFER that answers client D has IP destination address 255.255.255.255 and the
   broadcast link-layer destination address (RFC2131-BCAST-4, RFC2131-BCAST-6).
6. The DHCPACK that answers client D is a broadcast in the same way (RFC2131-BCAST-4).
7. The `flags` field that the server copies into each reply equals the `flags` field of the
   request it answers, so the two replies differ in their destination and in their `flags` field
   alike.

### Notes

- The two clients are what makes this a check rather than two half-checks. A server that always
  broadcasts passes observations 5 and 6 and fails 3 and 4; a server that always unicasts does
  the opposite. Only a server that reads the bit passes all four, and that is exactly
  RFC2131-BCAST-6.
- Observation 3 states two addresses and both matter. A reply whose IP destination is the
  client's new address but whose link-layer destination is the broadcast address would reach the
  client, and it would also reach every other host on the subnet. RFC 2131 asks for a unicast at
  both layers because a client that cleared the bit has said it can receive one.
- The deadlock behind the bit is worth keeping in view: the server is sending a client its
  address, by unicast, to that address. It works only because the reply also names the hardware
  address, which the client does have. A client that cannot accept such a datagram before it is
  configured is the client that sets the bit.
- RFC2131-BCAST-1, the client's own rule for choosing the bit, has no check. The value follows
  from what the client's own software can receive, which no observer on a link can know: a client
  that can receive a unicast and sets the bit anyway is wasteful and not wrong. The ledger
  records the statement without a check.
- RFC2131-BCAST-3, the third rule of §4.1, needs a non-zero `ciaddr` and is therefore a rule
  about a renewal; it is checked in [Renewal at T1](lease.md#renewal-at-t1).
