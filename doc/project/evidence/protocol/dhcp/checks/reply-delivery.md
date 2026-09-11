# DHCP — English check procedures: where a reply goes

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc2131/catalog.md](../../../standard/rfc2131/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

Two checks ask the server the same question with the BROADCAST bit at its two values. They are
separate checks because a server can obey one rule and not the other, and one verdict for the
pair would say which of the two happened only by accident.

The `SHOULD` of RFC2131-BCAST-6 — that the server reads the bit at all — needs both checks. It
is satisfied only when both pass; when one passes and one fails, the server is not reading the
bit, it is doing one thing always.

## Reply to a client that clears the broadcast bit

Checks: **RFC2131-BCAST-5** (description), **RFC2131-BCAST-6** (should, the clear half).

### Requirement

RFC 2131 §4.1: if `giaddr` is zero and `ciaddr` is zero and the broadcast bit is set, then the
server broadcasts DHCPOFFER and DHCPACK messages to 255.255.255.255. If the broadcast bit is not
set and `giaddr` is zero and `ciaddr` is zero, then the server unicasts them to the client's
hardware address and `yiaddr` address. A server sending a message directly to a client should
examine the BROADCAST bit in the `flags` field and send a broadcast or a unicast accordingly.

### Scenario constants

- The plain mockup. Client C is a client of the mockup and clears the BROADCAST bit in its
  DHCPDISCOVER and its DHCPREQUEST.
- Both messages carry `ciaddr` 0.0.0.0 and `giaddr` 0.0.0.0, because the client holds no address
  yet, so the first two rules of §4.1 do not apply and the bit decides.
- Observation stops one second after the start.

### Procedure

1. Build the plain mockup.
2. Let client C start, and read the top bit of the `flags` field of its DHCPDISCOVER.
3. Read the IP and link-layer destination addresses of the DHCPOFFER and the DHCPACK that answer
   it.

### Expected observations

1. The DHCPDISCOVER of client C carries the top bit of `flags` clear, and its `ciaddr` and
   `giaddr` are 0.0.0.0. This confirms the stimulus.
2. The DHCPOFFER that answers it has an IP destination address equal to its own `yiaddr`, and a
   link-layer destination address equal to the `chaddr` of that message: a unicast
   (RFC2131-BCAST-5, RFC2131-BCAST-6).
3. The DHCPACK that answers the DHCPREQUEST is a unicast in the same way (RFC2131-BCAST-5).
4. The `flags` field of both replies has its top bit clear, the value the request carried.

### Notes

- Observation 2 states two addresses and both matter. A reply whose IP destination is the
  client's new address but whose link-layer destination is the broadcast address would reach the
  client, and it would also reach every other host on the subnet. RFC 2131 asks for a unicast at
  both layers because a client that cleared the bit has said it can receive one.
- The deadlock behind the bit is worth keeping in view: the server is sending a client its
  address, by unicast, to that address. It works only because the reply also names the hardware
  address, which the client does have. A client that cannot accept such a datagram before it is
  configured is the client that sets the bit.
- A server that broadcasts here reaches the client too, so nothing in the mockup stops working.
  What it loses is the point of the rule: every other host on the subnet receives a message
  addressed to one client, and a subnet with many clients starting at once carries every reply
  to every host.

## Reply to a client that sets the broadcast bit

Checks: **RFC2131-BCAST-4** (description), **RFC2131-BCAST-6** (should, the set half).

### Requirement

RFC 2131 §4.1: if `giaddr` is zero and `ciaddr` is zero and the broadcast bit is set, then the
server broadcasts DHCPOFFER and DHCPACK messages to 255.255.255.255, with the link-layer
broadcast address as the link-layer destination.

### Scenario constants

- The mockup with an injector. The request is a **crafted** DHCPDISCOVER with the BROADCAST bit
  set, put on the link with a hardware address of its own. It stands for a client that cannot
  receive a unicast before it is configured.
- Its `ciaddr` and `giaddr` are 0.0.0.0, so the bit decides.
- Observation stops one second after the injection.

The crafted request is what makes this a level 3 check. A client that clears the bit cannot
produce the other value, and crafting the request is the only way to ask the server this half of
the question.

### Procedure

1. Build the mockup with the server and an injector.
2. Put a crafted DHCPDISCOVER with the BROADCAST bit set onto the link.
3. Read the destination of the DHCPOFFER that answers it.

### Expected observations

1. The crafted DHCPDISCOVER reaches the server with the top bit of `flags` set and with `ciaddr`
   and `giaddr` 0.0.0.0. This confirms the stimulus.
2. The DHCPOFFER that answers it has IP destination address 255.255.255.255 and the broadcast
   link-layer destination address (RFC2131-BCAST-4, RFC2131-BCAST-6).

### Notes

- There is no observation for a DHCPACK here, because no node answers the DHCPOFFER: the client
  the crafted request speaks for does not exist. The DHCPOFFER alone carries the rule, and the
  DHCPACK of the other check covers the other value of the bit.
- Whether the reply carries the bit **back** in its own `flags` field is a different statement,
  the `flags` row of RFC 2131 table 3, and it has a check of its own: [The flags field of a
  server reply](#the-flags-field-of-a-server-reply). It is separate because a server can send
  the reply to the right destination and still write the wrong value into the field.
- This check and the one above are the pair that establishes RFC2131-BCAST-6. A server that
  always broadcasts passes this one and fails the other; a server that always unicasts does the
  opposite.
- RFC2131-BCAST-1, the client's own rule for choosing the bit, has no check. The value follows
  from what the client's own software can receive, which no observer on a link can know: a client
  that can receive a unicast and sets the bit anyway is wasteful and not wrong. The ledger
  records the statement without a check.
- RFC2131-BCAST-3, the third rule of §4.1, needs a non-zero `ciaddr` and is therefore a rule
  about a renewal; it is checked in [Renewal at T1](lease.md#renewal-at-t1).

## The flags field of a server reply

Checks: **RFC2131-OFF-6** (description, the `flags` half), **RFC2131-ACK-8** (description, the
`flags` half).

### Requirement

RFC 2131 table 3, DHCPOFFER column: `flags` is the `flags` from the client DHCPDISCOVER message.
DHCPACK column: `flags` is the `flags` from the client DHCPREQUEST message.

### Scenario constants

- The mockup with an injector. The request is the same **crafted** DHCPDISCOVER as in the check
  above, with the BROADCAST bit set.
- Observation stops one second after the injection.

The bit must be **set** in the request for this check to say anything. Where every client of the
mockup clears it, a server that always writes zero into the field passes by accident, which is
why [Offer contents](exchange.md#offer-contents) can cover the field only for the value zero.

### Procedure

1. Build the mockup with the server and an injector.
2. Put a crafted DHCPDISCOVER with the BROADCAST bit set onto the link.
3. Read the `flags` field of the DHCPOFFER that answers it.

### Expected observations

1. The crafted DHCPDISCOVER reaches the server with the top bit of `flags` set. This confirms the
   stimulus.
2. The DHCPOFFER that answers it carries the top bit of its own `flags` field set: the value the
   request carried (RFC2131-OFF-6).

### Notes

- The field matters to a relay agent and not to the client. RFC 2131 §4.1 says a relay agent that
  forwards a reply directly to a client should examine the BROADCAST bit of the message it is
  forwarding, so a reply that lost the bit tells the relay agent to unicast to a client that
  asked for a broadcast. Without a relay agent in the mockup the consequence is invisible, and the
  field rule is still a field rule.
- There is no observation for a DHCPACK, for the same reason as in the check above: no node
  answers the DHCPOFFER that the crafted request draws. The `flags` half of RFC2131-ACK-8 is
  therefore covered only for the value zero, by [Acknowledgement
  contents](exchange.md#acknowledgement-contents). The ledger records that bound.
