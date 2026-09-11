# ARP — English check procedures: the reply

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc826/catalog.md](../../../standard/rfc826/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Reply field values

Checks: **RFC826-RECV-8** (description), **RFC826-RECV-9** (description). Covers
**RFC826-FMT-6** and **RFC826-TABLE-2**.

### Requirement

RFC 826, Packet Reception: a receiver that is the target protocol address of a request swaps
the hardware and the protocol fields, puts its own hardware and protocol addresses in the
sender fields, sets the opcode to reply, and sends the packet to the new target hardware
address on the same hardware on which the request arrived. The example of the document adds
that the reply goes directly and not as a broadcast.

### Scenario constants

- The mockup is the pair. Host A sends one datagram to host B at 0.1 s, which starts one
  exchange.

### Procedure

1. Build the mockup with two hosts on one Ethernet link.
2. Let host A send its datagram, so that host A puts a request on link 1.
3. Read every field of the request, and then every field of the reply.

### Expected observations

1. The request from host A, with the addresses of host A in the sender fields and the
   address of host B in the target protocol address.
2. A reply from host B on link 1, with the opcode of a reply.
3. The sender hardware address and the sender protocol address of the reply are the
   addresses of host B.
4. The target hardware address and the target protocol address of the reply are the
   addresses of host A, which stood in the sender fields of the request.
5. The frame that carries the reply is addressed to the hardware address of host A, and not
   to the hardware broadcast address.
6. The reply is as long as the request.

### Notes

- Observation 1 is the stimulus. Observations 3 and 4 together are the swap that
  RFC826-RECV-8 demands, read from the packet and not from the state of host B.
- Observation 5 is RFC826-RECV-9. The check states the negative half as well — not the
  broadcast address — because a station that answers by broadcast still delivers the reply
  and still breaks the rule.
- Observation 6 is RFC826-FMT-6. The two lengths are equal because the format is fixed for
  one hardware type and one protocol; the statement matters for an implementation that
  reuses the buffer of the request.
- "On the same hardware" is one link here, so the observation cannot separate the interface
  from the link. A host with two interfaces would show it, and that mockup belongs to a
  later pass.

## Learning from a request

Checks: **RFC826-RECV-6** (description), **RFC826-RECV-7** (description).

### Requirement

RFC 826, Packet Reception: a receiver that is the target and did not merge adds the triplet
of the protocol type, the sender protocol address and the sender hardware address to its
table. The document states that this happens before the opcode is read, and gives the
reason: communication is bidirectional, so a host that hears from another host will
probably want to answer it. The example says the outcome plainly: after the exchange, "if
Y's Internet module then wants to talk to X, this will also succeed since Y has remembered
the information from X's request".

### Scenario constants

- The mockup is the pair.
- Host A sends one datagram to port 5000 of host B at 0.1 s.
- Host B sends one datagram to port 5000 of host A at 0.5 s, after the first exchange has
  finished. Host B has never reached host A by itself.

### Procedure

1. Build the mockup with two hosts on one Ethernet link.
2. Let host A resolve the address of host B and let the datagram arrive.
3. Let host B send its own datagram to host A.
4. Watch link 1 for any request from host B.

### Expected observations

1. The request from host A near 0.1 s, and the reply from host B.
2. The datagram of host B leaves host B after 0.5 s and reaches host A.
3. No ARP request for the address of host A ever leaves host B.

### Notes

- Observation 3 is the whole check, and observation 2 is what makes the absence mean
  something: host B really did send a datagram, and it needed a hardware address to do so.
- The table entry of host B came from the sender fields of the request of host A. Nothing
  else on the link could have given it: host A sent no reply, and host B asked nothing.
- This check covers the request branch of RFC826-RECV-7. The reply branch — that a reply
  fills the table in the same way — needs a reply that no request provoked, and that is
  [A reply fills the table](cache.md#a-reply-fills-the-table).
