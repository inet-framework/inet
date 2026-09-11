# ARP — English check procedures: the translation table

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc826/catalog.md](../../../standard/rfc826/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

Both checks here read the table of a host without reading its state. The table is inside a
host, and two stations on a link cannot see it. What they can see is where the next
datagram of that host goes: the destination hardware address of the frame is the entry of
the table, made visible. Both checks therefore change the table with a crafted packet and
then read the next frame.

## A newer hardware address

Checks: **RFC826-TABLE-1** (description), **RFC826-RECV-4** (description).

### Requirement

RFC 826, Packet Reception: if an entry already exists for the pair of the protocol type and
the sender protocol address, the sender hardware address field of that entry is updated with
the new information in the packet, and the merge flag is set. The document states the
consequence twice: "the new hardware address supersedes the old one", and, in the Related
issue section, that on a cable where a broadcast request reaches every station, each station
gets the new hardware address.

### Scenario constants

- The mockup is the shared link, with hosts A, B and C. Host C sends and receives nothing.
- Host A sends datagrams to host B: one at 0.1 s, which starts a normal exchange, and one
  at 0.8 s.
- A crafted request arrives at host A at 0.5 s, as if from the link. Its sender protocol
  address is the address of host B and its sender hardware address is a second hardware
  address, different from the one host B really has. Its target protocol address is the
  address of host C, so that host A takes the merge branch and not the reply branch.
- The lifetime of a table entry is longer than the run, so that nothing expires.

### Procedure

1. Build the mockup with three hosts on one Ethernet segment.
2. Let host A resolve the address of host B, and read the destination hardware address of
   the frame that carries the first datagram.
3. Put the crafted request on the interface of host A at 0.5 s.
4. Let host A send the second datagram, and read the destination hardware address of its
   frame.

### Expected observations

1. The request from host A near 0.1 s, and the reply from host B.
2. The frame that carries the first datagram is addressed to the real hardware address of
   host B.
3. The crafted request arrives at host A, with the address of host B in its sender protocol
   address and the second hardware address in its sender hardware address.
4. The frame that carries the second datagram is addressed to the second hardware address.
5. No new ARP request for the address of host B leaves host A after 0.5 s.

### Notes

- Observations 2 and 4 are the same reading at two times, and the difference between them
  is the whole check. Observation 3 is the stimulus.
- Observation 5 matters because a host that discarded the crafted packet and then re-asked
  would also send the second datagram to the right place — the right place by then being
  the real address again. The absence of a new request shows that the change came from the
  crafted packet and not from a second resolution.
- The crafted packet is addressed to host C on purpose. RFC826-RECV-4 applies before the
  target question, so a packet that is not for host A must still update an entry that host A
  holds. A packet addressed to host A would update the entry too, and would prove less.
- The second hardware address is any address that is not the address of host B and is not a
  broadcast or multicast address. Its value carries no meaning.

## A reply fills the table

Checks: **RFC826-RECV-7** (description), **RFC826-RECV-6** (description).

### Requirement

RFC 826, Packet Reception: the sender triplet is merged into the table before the opcode is
looked at, and the document says why: "This is on the assumption that communcation is
bidirectional". A receiver that is the target and did not merge adds the triplet. Both steps
come before the opcode question, so a reply teaches the receiver exactly as much as a
request does.

### Scenario constants

- The mockup is the shared link, with hosts A, B and C.
- A crafted reply arrives at host A at 0.3 s, as if from the link. Its sender protocol
  address is the address of host C and its sender hardware address is a hardware address
  that host C does not have. Its target fields hold the addresses of host A, so that host A
  is the target and the add step applies.
- Host A has never reached host C, so it holds no entry for host C.
- Host A sends one datagram to host C at 0.6 s, after the crafted reply.
- Host C sends and receives nothing, and runs no program.

### Procedure

1. Build the mockup with three hosts on one Ethernet segment.
2. Put the crafted reply on the interface of host A at 0.3 s.
3. Let host A send its datagram to host C at 0.6 s.
4. Watch link 1 for a request from host A, and read the frame that carries the datagram.

### Expected observations

1. The crafted reply arrives at host A, with the opcode of a reply and the address of
   host C in its sender protocol address.
2. No ARP request for the address of host C ever leaves host A.
3. The frame that carries the datagram of host A is addressed to the hardware address that
   the crafted reply carried.

### Notes

- Observation 2 is RFC826-RECV-7: the table entry was made from a packet whose opcode was a
  reply, which the algorithm reads only after the table step. Observation 3 shows which
  entry was made, and RFC826-RECV-6 is the step that made it, because host A held nothing
  for host C before.
- The hardware address in the crafted reply is deliberately not the address of host C. If
  it were, observation 3 could not tell a table entry from a fresh resolution that host A
  did quietly.
- The datagram never reaches host C, and that is expected: it goes to a hardware address
  that nobody has. The check is about where host A sends it, not about delivery. This is
  also the reason the same packet shape is a known attack, and the reason RFC 5227 exists;
  the standards map files that document at level 4.
- Host C receives the datagram at no point, so it needs no program. A station that is
  present with an address is all the check needs of it.
