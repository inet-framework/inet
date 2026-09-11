# ARP — English check procedures: resolution

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc826/catalog.md](../../../standard/rfc826/catalog.md), [rfc1122/catalog.md](../../../standard/rfc1122/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Address resolution

Checks: **RFC826-REQ-1** (must), **RFC826-REQ-4** (description), **RFC826-REQ-6**
(description), **RFC1122-AUSE-1** (must). Covers **RFC826-FMT-3**, **RFC826-FMT-5** and
**RFC826-TABLE-2**.

### Requirement

RFC 826, Packet Generation: a sending layer that needs a hardware address for a protocol
address must consult the address resolution module. On a miss the module builds a packet
with the hardware space of the link, the protocol being resolved, the two address lengths,
the opcode of a request, its own hardware and protocol addresses in the sender fields, and
the wanted protocol address in the target protocol address. It broadcasts that packet on
the hardware that routing chose. RFC 1122 §2.3.3 says that on an Ethernet link ARP is the
protocol that must do this.

### Scenario constants

- The mockup is the pair. Host A sends one datagram of 100 octets to port 5000 of host B,
  at 0.1 s.
- Host A has never reached host B, so its table holds no entry for host B.
- Neither host has any address of the other in advance.

### Procedure

1. Build the mockup with two hosts on one Ethernet link.
2. Let host A send its datagram to host B.
3. Watch link 1 from the moment the simulation starts.

### Expected observations

1. Host A puts an ARP packet on link 1 before it puts any datagram there. The frame carries
   the address resolution protocol and not IPv4.
2. That packet has the opcode of a request. Its sender hardware address and sender protocol
   address are the addresses of host A, and its target protocol address is the address of
   host B.
3. The frame that carries it has the hardware broadcast address as its destination.
4. Host B receives the packet.
5. Host B answers, and the datagram of host A then reaches host B.

### Notes

- Observation 1 is the one that proves the stimulus: without a datagram to send, host A has
  no reason to resolve anything, and the check would pass on an empty link.
- Observation 2 carries RFC826-TABLE-2 as well: the sender fields hold the addresses of the
  host that sent the packet, and of no other host.
- The target hardware address of the request is not part of any observation. RFC826-REQ-5
  declares the field meaningless in a request, so the check reads it and asserts nothing;
  see [the list of statements that no check carries](../checks.md#statements-that-no-check-carries).
- Observation 5 is the end of the story and not the subject of this check. The reply is
  checked in [Reply field values](reply.md#reply-field-values), and the fate of the
  datagram in [The waiting datagram](queue.md#the-waiting-datagram).

## The cached mapping

Checks: **RFC826-REQ-2** (description).

### Requirement

RFC 826, Packet Generation: the module tries to find the pair of the protocol type and the
target protocol address in a table. If it finds the pair, it gives the hardware address
back to the caller, which then transmits the packet. A hit therefore puts nothing on the
link.

### Scenario constants

- The mockup is the pair. Host A sends five datagrams to host B, one every 0.1 s from
  0.1 s, so that four of them follow a resolution that already happened.
- The lifetime of a table entry is longer than the whole run, so that no entry can expire
  during it.

### Procedure

1. Build the mockup with two hosts on one Ethernet link.
2. Let host A send its five datagrams.
3. Count the ARP requests that host A puts on link 1 during the run.

### Expected observations

1. Exactly one request for the address of host B leaves host A.
2. All five datagrams reach host B.

### Notes

- Observation 2 proves the stimulus: five datagrams were really sent, so the single request
  is a cache hit four times and not a sender that stopped.
- The check needs the lifetime of an entry to exceed the run. A host whose entry expires
  after 0.2 s would send more requests and still conform; the check states the condition
  so that the count means what it says.

## The cache flush

Checks: **RFC1122-ACACHE-1** (must), **RFC1122-ACACHE-2** (should).

### Requirement

RFC 1122 §2.3.2.1: an ARP implementation must provide a mechanism to flush out-of-date
cache entries, and if the mechanism uses a timeout, the timeout value should be
configurable. RFC 826 left the question outside its own scope,
[RFC826-TABLE-3](../../../standard/rfc826/catalog.md#rfc826-table-3).

### Scenario constants

- The mockup is the pair.
- The lifetime of an entry is 2 s, a value the scenario chooses. The choice is the check of
  RFC1122-ACACHE-2: a value that a scenario can name is a value that is configurable.
- Host A sends one datagram at 0.1 s and one at 5 s. The gap of 4.9 s is longer than the
  lifetime.
- Observation stops after 6 s.

### Procedure

1. Build the mockup with two hosts on one Ethernet link, and set the lifetime of a table
   entry to 2 s.
2. Let host A send the first datagram, and let the exchange complete.
3. Let host A send the second datagram after the gap.
4. Watch link 1 for requests from host A.

### Expected observations

1. One request for the address of host B leaves host A near 0.1 s, and host B answers it.
2. The first datagram reaches host B.
3. A second request for the same address leaves host A near 5 s.
4. The second datagram reaches host B.

### Notes

- Observation 3 is the visible half of the flush. The entry itself is state inside a host,
  and a check of two hosts on a link cannot read it. A new request for an address that was
  resolved already is the only outward sign that the old entry is gone.
- The check reads the lifetime as a scenario constant, so it establishes that the value is
  configurable (RFC1122-ACACHE-2) and that some flush mechanism exists (RFC1122-ACACHE-1).
  It does not establish which of the four mechanisms of the IMPLEMENTATION note the host
  uses, and the requirement does not ask.
- A host with no timeout at all would fail observation 3 and could still satisfy
  RFC1122-ACACHE-1 through a different mechanism, for example advice from the link layer.
  The notes of the results say so if that case ever appears.
