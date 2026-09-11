# ARP — English check procedures: input validation

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc826/catalog.md](../../../standard/rfc826/catalog.md), [rfc5494/catalog.md](../../../standard/rfc5494/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

Five checks live here. The first watches a normal exchange on a link with three stations.
The other four put a crafted packet on the interface of a host, because no program can
produce one; see
[why a crafted packet is needed at all](../checks.md#why-a-crafted-packet-is-needed-at-all).

All five share one shape. The stimulus is a packet that the receiver cannot use, and the
expected outcome is an absence: no ARP packet leaves the receiver, and the receiver goes on
working. RFC 826 states the rule once, at the head of its reception algorithm — "Negative
conditionals indicate an end of processing and a discarding of the packet" — so every one of
these checks covers [RFC826-RECV-1](../../../standard/rfc826/catalog.md#rfc826-recv-1) as
well.

## A request for a third station

Checks: **RFC826-RECV-5** (description). Covers **RFC826-RECV-1**.

### Requirement

RFC 826, Packet Reception: the receiver asks whether it is the target protocol address. A
negative answer ends the processing and discards the packet. A request is a broadcast, so
every station on the link receives it, and all but one of them take this branch.

### Scenario constants

- The mockup is the shared link, with hosts A, B and C.
- Host A sends one datagram of 100 octets to host C at 0.1 s. It has never reached host C.
- Host B sends nothing and receives nothing. It runs no program.

### Procedure

1. Build the mockup with three hosts on one Ethernet segment.
2. Let host A resolve the address of host C.
3. Watch host B for the whole run.

### Expected observations

1. The request from host A arrives at host B. Its target protocol address is the address of
   host C, not of host B.
2. Host C answers the request.
3. Host B puts no ARP packet on the link at any time.

### Notes

- Observation 1 proves the stimulus reached the station that must stay silent. Without it,
  a switch that dropped the broadcast would let the check pass.
- Observation 2 shows that the request was a well-formed one that a station does answer.
  The silence of host B is therefore a decision about the target address and not a reaction
  to a broken packet.
- Host B still learns nothing here, and the standard does not ask it to: the merge step of
  RFC826-RECV-4 changes an entry that exists, and host B has none for host A.

## An unsolicited reply

Checks: **RFC826-RECV-10** (description). Covers **RFC826-RECV-1**.

### Requirement

RFC 826, Packet Reception: the opcode question has one positive branch, the request. A
reply takes the negative branch, so the processing ends and the packet is discarded. The
example states the outcome: the receiver "notices the packet is a reply and throws it
away". The table is filled first, which is the subject of
[A reply fills the table](cache.md#a-reply-fills-the-table); this check is about the
silence.

### Scenario constants

- The mockup is the shared link.
- A crafted reply arrives at host B at 0.5 s, as if from the link. Its sender fields hold
  the addresses of host A and its target fields hold the addresses of host B. No request
  from host B ever asked for it.
- Nothing else happens in the run: no host sends a datagram.

### Procedure

1. Build the mockup with three hosts on one Ethernet segment.
2. Put the crafted reply on the interface of host B at 0.5 s.
3. Watch host B until 1 s.

### Expected observations

1. The crafted reply arrives at host B, with the opcode of a reply.
2. Host B puts no ARP packet on the link afterwards.

### Notes

- A reply that answers a real request would prove nothing: a host that answered it would be
  wrong, but a host that stayed silent could be silent because it was satisfied. An
  unsolicited reply removes that ambiguity, and it is also the shape an attacker uses.
- The check makes no statement about the table of host B. What the reply did to the table
  is the other check of this pair.

## An experimental opcode

Checks: **RFC826-RECV-10** (description), **RFC5494-NUM-3** (description).

### Requirement

RFC 826 defines two opcodes, 1 and 2, and its reception algorithm answers a request and
discards everything else. RFC 5494 §3 allocates the opcodes 24 and 25 for experiments. A
station that is not part of the experiment therefore receives a well-formed packet whose
opcode it does not handle, and the rule of RFC 826 applies: end of processing, and the
packet is discarded.

### Scenario constants

- The mockup is the shared link.
- A crafted packet arrives at host B at 0.5 s. Every field is that of a valid request from
  host A to host B, except the opcode, which is 24 — OP_EXP1 of RFC 5494 §3.
- Nothing else happens in the run.

### Procedure

1. Build the mockup with three hosts on one Ethernet segment.
2. Put the crafted packet on the interface of host B at 0.5 s.
3. Watch host B until 1 s.

### Expected observations

1. The crafted packet arrives at host B, and its opcode is 24.
2. Host B puts no ARP packet on the link afterwards.
3. Host B goes on working: it is still a running station when the run ends.

### Notes

- Observation 3 is written down because this is the first check whose stimulus is a value
  the standard reserves for somebody else. A station that stops on an unknown opcode fails
  the requirement in the strongest possible way, and the observation names the outcome so
  that a reader can tell it from a mere absence of a reply.
- The target protocol address of the crafted packet is the address of host B on purpose.
  The packet therefore reaches the opcode question, which is the last question of the
  algorithm; a packet addressed elsewhere would be discarded one question earlier and would
  check nothing new.

## An experimental hardware space

Checks: **RFC826-RECV-2** (description), **RFC5494-NUM-2** (description).

### Requirement

RFC 826, Packet Reception: the first question is whether the receiver has the hardware type
of the hardware space field. A negative answer ends the processing and discards the packet.
RFC 5494 §3 allocates the hardware space values 36 and 256 for experiments, so a station on
an Ethernet link meets a defined value that is not the value of its own hardware.

### Scenario constants

- The mockup is the shared link.
- A crafted packet arrives at host B at 0.5 s. Every field is that of a valid request from
  host A to host B, except the hardware space, which is 36 — HW_EXP1 of RFC 5494 §3.
- The hardware length field stays 6, so that the packet is still readable and the length is
  not a second reason to discard it.
- Nothing else happens in the run.

### Procedure

1. Build the mockup with three hosts on one Ethernet segment.
2. Put the crafted packet on the interface of host B at 0.5 s.
3. Watch host B until 1 s.

### Expected observations

1. The crafted packet arrives at host B, and its hardware space field is 36.
2. Host B puts no ARP packet on the link afterwards.
3. Host B goes on working.

### Notes

- The target protocol address is again the address of host B, so that the only reason to
  discard the packet is the one under test.
- This check and the next one are the two that make the hardware space and the protocol
  space fields matter. Every other check on an Ethernet link sees the same two values in
  every packet, so nothing in them can tell whether a receiver reads the fields at all.

## An unknown protocol space

Checks: **RFC826-RECV-3** (description).

### Requirement

RFC 826, Packet Reception: the second question is whether the receiver speaks the protocol
in the protocol space field. A negative answer ends the processing and discards the packet.
The field holds Ethertype numbers, [RFC5494-NUM-4](../../../standard/rfc5494/catalog.md#rfc5494-num-4).

### Scenario constants

- The mockup is the shared link. The hosts run IPv4 and nothing else.
- A crafted packet arrives at host B at 0.5 s. Every field is that of a valid request from
  host A to host B, except the protocol space, which is the Ethertype of IPv6, 0x86DD.
- The protocol length field stays 4, so that the four address fields keep the widths the
  packet actually has.
- Nothing else happens in the run.

### Procedure

1. Build the mockup with three hosts on one Ethernet segment.
2. Put the crafted packet on the interface of host B at 0.5 s.
3. Watch host B until 1 s.

### Expected observations

1. The crafted packet arrives at host B, and its protocol space field is 0x86DD.
2. Host B puts no ARP packet on the link afterwards.
3. Host B goes on working.

### Notes

- The protocol space value is a real one and not an invented number, which keeps the check
  honest: a station may legitimately meet this value on a link where a second protocol
  runs, and it must then answer for that protocol or stay silent, never answer for IPv4.
- The address lengths of the crafted packet stay those of IPv4 even though the protocol
  space says IPv6. That is deliberate: a packet with 16-octet protocol addresses would
  differ in its length as well, and the check would no longer be about the protocol space
  alone. RFC826-RECV-11 permits a receiver to notice the disagreement, and a receiver that
  discards the packet for that reason also satisfies this check — the outcome is the same
  silence.
