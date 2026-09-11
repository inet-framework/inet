# RFC 826 (ARP) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC826-*` · **Stands on:** [standards.md](../../protocol/arp/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow, for one document of
the in-scope set: RFC 826, An Ethernet Address Resolution Protocol of November 1982, still
the Internet Standard (STD 37). The catalog comes from the RFC text only. It contains no
simulation model names and no code references.

Source, cached in this folder:

- `rfc826.txt` — An Ethernet Address Resolution Protocol, November 1982. Downloaded
  2026-09-11 from <https://www.rfc-editor.org/rfc/rfc826.txt>.

The family of documents and the in-scope set are in
[`standards.md`](../../protocol/arp/standards.md). The features these statements build are
in [`features.md`](../../protocol/arp/features.md). The host requirements that RFC 1122 adds
are in [`rfc1122/catalog.md`](../rfc1122/catalog.md), in the entries whose identifiers start
with `A`, and the number spaces that RFC 5494 redefines are in
[`rfc5494/catalog.md`](../rfc5494/catalog.md).

The state of the workflow — which statement a check targets, which test carries it, and
what the run said — is **not** in this document. It lives in the coverage ledger,
[`arp/coverage.md`](../../model/arp/coverage.md). Keeping it out is deliberate: this
catalog states what the standard says, so a new test or a new run must never force an edit
here.

Quotes are verbatim. A reference such as `rfc826.txt:188-190` points to a line of the
cached file in this folder.

## Index

| ID | Statement |
| --- | --- |
| [RFC826-FMT-1](#rfc826-fmt-1) | The packet carries nine fields in one fixed order. |
| [RFC826-FMT-2](#rfc826-fmt-2) | For the 10 Mbit Ethernet the hardware space is 1 and the hardware length is 6. |
| [RFC826-FMT-3](#rfc826-fmt-3) | The packet travels in a frame whose protocol type is the address resolution type. |
| [RFC826-FMT-4](#rfc826-fmt-4) | There is no padding between the addresses, and the three 16-bit fields go most significant byte first. |
| [RFC826-FMT-5](#rfc826-fmt-5) | The opcode of a request is 1 and the opcode of a reply is 2. |
| [RFC826-FMT-6](#rfc826-fmt-6) | A reply has the same length as a request. |
| [RFC826-FMT-7](#rfc826-fmt-7) | One packet resolves one address. |
| [RFC826-REQ-1](#rfc826-req-1) | A sender must ask the address resolution module to convert the pair into a hardware address. |
| [RFC826-REQ-2](#rfc826-req-2) | When the table holds the pair, the module returns the hardware address and the packet goes out. |
| [RFC826-REQ-3](#rfc826-req-3) | When the table does not hold the pair, the waiting packet is thrown away and a request is generated. |
| [RFC826-REQ-4](#rfc826-req-4) | A request carries the hardware space, the protocol, the two lengths, the opcode, the sender addresses and the target protocol address. |
| [RFC826-REQ-5](#rfc826-req-5) | The target hardware address of a request means nothing and may be the hardware broadcast address. |
| [RFC826-REQ-6](#rfc826-req-6) | The request is broadcast to all stations on the hardware that the routing chose. |
| [RFC826-RECV-1](#rfc826-recv-1) | A negative answer in the reception algorithm ends the processing and discards the packet. |
| [RFC826-RECV-2](#rfc826-recv-2) | A packet whose hardware space the receiver does not have is discarded. |
| [RFC826-RECV-3](#rfc826-recv-3) | A packet whose protocol the receiver does not speak is discarded. |
| [RFC826-RECV-4](#rfc826-recv-4) | A sender protocol address that the table already holds updates the hardware address of that entry. |
| [RFC826-RECV-5](#rfc826-recv-5) | A packet whose target protocol address is not the receiver's own is discarded without a reply. |
| [RFC826-RECV-6](#rfc826-recv-6) | The receiver adds the sender triplet to the table when it is the target and the merge did not happen. |
| [RFC826-RECV-7](#rfc826-recv-7) | The sender information enters the table before the opcode is read. |
| [RFC826-RECV-8](#rfc826-recv-8) | A request addressed to the receiver is answered with the fields swapped and the opcode set to reply. |
| [RFC826-RECV-9](#rfc826-recv-9) | The reply goes to the target hardware address on the same hardware, and is not broadcast. |
| [RFC826-RECV-10](#rfc826-recv-10) | A reply is thrown away after the table is updated, and provokes nothing. |
| [RFC826-RECV-11](#rfc826-recv-11) | The two length fields may be checked for consistency. |
| [RFC826-TABLE-1](#rfc826-table-1) | A new sender hardware address supersedes the one the table holds. |
| [RFC826-TABLE-2](#rfc826-table-2) | A host sends information about itself only. |
| [RFC826-TABLE-3](#rfc826-table-3) | Table aging and timeouts are outside the scope of this protocol. |
| [RFC826-GEN-1](#rfc826-gen-1) | The hardware fields allow other hardware, and the protocol field should name the protocol being resolved. |

## How to read an entry

The conventions of an entry — strength, class, and the `Overridden by` field — are the ones
of [`rfc791/catalog.md`](../rfc791/catalog.md#how-to-read-an-entry). RFC 826 predates
RFC 2119 by eleven years. It uses the word `must` twice and no other keyword, so the
strength of nearly every entry is `description`.

Two things about the reception algorithm, `rfc826.txt:197-234`, decide how its entries read.
The algorithm is written as a chain of questions, and the document states the rule for a
negative answer once, at the top: "Negative conditionals indicate an end of processing and a
discarding of the packet", `rfc826.txt:200-201`. That sentence is
[RFC826-RECV-1](#rfc826-recv-1), and every discard entry below points at it. The document
also introduces the algorithm as one that the receiver follows loosely — "goes through an
algorithm similar to the following", `rfc826.txt:198-199` — which is why the steps are
`description` and not `must`.

## Packet format

### RFC826-FMT-1

**The packet carries nine fields in one fixed order.**

> "16.bit: (ar$hrd) Hardware address space ... 16.bit: (ar$pro) Protocol address space ...
> 8.bit: (ar$hln) byte length of each hardware address / 8.bit: (ar$pln) byte length of each
> protocol address / 16.bit: (ar$op) opcode (ares_op$REQUEST | ares_op$REPLY) / nbytes:
> (ar$sha) Hardware address of sender of this packet ... mbytes: (ar$spa) Protocol address of
> sender of this packet ... nbytes: (ar$tha) Hardware address of target of this packet (if
> known) / mbytes: (ar$tpa) Protocol address of target." — Packet format,
> `rfc826.txt:141-155`

- Strength: description. Class: encoding.
- Check idea: read the octets of a packet on the link and compare each field with the
  layout. The lengths of the two address fields follow the `ar$hln` and `ar$pln` values of
  the same packet.

### RFC826-FMT-2

**For the 10 Mbit Ethernet the hardware space is 1 and the hardware length is 6.**

> "For the 10Mbit Ethernet <ar$hrd, ar$hln> takes on the value <1, 6>." — Generalization,
> `rfc826.txt:237-238`

The definitions section gives the same number a name:

> "ares_hrd$Ethernet (= 1)." — Definitions, `rfc826.txt:126`

- Strength: description. Class: encoding.
- Check idea: every ARP packet on an Ethernet link carries 1 in the hardware space field
  and 6 in the hardware length field.

### RFC826-FMT-3

**The packet travels in a frame whose protocol type is the address resolution type.**

> "generates an Ethernet packet with a type field of ether_type$ADDRESS_RESOLUTION." —
> Packet Generation, `rfc826.txt:175-176`

The packet format states the same field:

> "16.bit: Protocol type = ether_type$ADDRESS_RESOLUTION" — Packet format,
> `rfc826.txt:139`

- Strength: description. Class: wire.
- Check idea: a request and a reply on the link are both carried as the address resolution
  protocol and not as a datagram of the protocol being resolved.

### RFC826-FMT-4

**There is no padding between the addresses, and the three 16-bit fields go most significant
byte first.**

> "There are no padding bytes between addresses. The packet data should be viewed as a byte
> stream in which only 3 byte pairs are defined to be words (ar$hrd, ar$pro and ar$op) which
> are sent most significant byte first (Ethernet/PDP-10 byte style)." — Why is it done this
> way, `rfc826.txt:320-323`

The notes give the reason for the warning:

> "Numbers here are in the Ethernet standard, which is high byte first." — Notes,
> `rfc826.txt:63-64`

- Strength: description. Class: encoding.
- Check idea: serialize a packet and read the octets. The opcode of a request is the two
  octets 0x00 0x01, in that order, and the sender protocol address follows the sender
  hardware address with nothing between them.

### RFC826-FMT-5

**The opcode of a request is 1 and the opcode of a reply is 2.**

> "ares_op$REQUEST (= 1, high byte transmitted first) and ares_op$REPLY (= 2)," —
> Definitions, `rfc826.txt:123-124`

- Strength: description. Class: wire.
- Check idea: the first packet of an exchange carries opcode 1 and the answer carries
  opcode 2.

### RFC826-FMT-6

**A reply has the same length as a request.**

> "This format allows the packet buffer to be reused if a reply is generated; a reply has
> the same length as a request, and several of the fields are the same." — Why is it done
> this way, `rfc826.txt:268-270`

- Strength: description. Class: wire.
- Check idea: measure the request and the reply of one exchange on the link. The two
  lengths are equal.

### RFC826-FMT-7

**One packet resolves one address.**

> "This format does not allow for more than one resolution to be done in the same packet.
> This is for simplicity." — Why is it done this way, `rfc826.txt:260-261`

- Strength: description. Class: encoding.
- Check idea: the layout holds one target protocol address, so a host that needs two
  addresses sends two packets.

## Request generation

### RFC826-REQ-1

**A sender must ask the address resolution module to convert the pair into a hardware
address.**

> "In the case of the 10Mbit Ethernet, address resolution is needed and some lower layer
> (probably the hardware driver) must consult the Address Resolution module (perhaps
> implemented in the Ethernet support module) to convert the <protocol type, target protocol
> address> pair to a 48.bit Ethernet address." — Packet Generation, `rfc826.txt:164-169`

- Strength: must. Class: wire. This is one of the two `must` words of the document.
- Check idea: a host with a datagram for a neighbour on an Ethernet link puts an ARP
  request on the link before it puts the datagram there.

### RFC826-REQ-2

**When the table holds the pair, the module returns the hardware address and the packet goes
out.**

> "The Address Resolution module tries to find this pair in a table. If it finds the pair,
> it gives the corresponding 48.bit Ethernet address back to the caller (hardware driver)
> which then transmits the packet." — Packet Generation, `rfc826.txt:169-172`

- Strength: description. Class: wire (absence).
- Check idea: after one exchange, a second datagram to the same neighbour leaves the host
  with no new request on the link.

### RFC826-REQ-3

**When the table does not hold the pair, the waiting packet is thrown away and a request is
generated.**

> "If it does not, it probably informs the caller that it is throwing the packet away (on
> the assumption the packet will be retransmitted by a higher network layer), and generates
> an Ethernet packet with a type field of ether_type$ADDRESS_RESOLUTION." — Packet
> Generation, `rfc826.txt:172-176`

- Strength: description; the word "probably" makes the discard a suggestion and not a rule.
  Class: end-to-end.
- Overridden by: [RFC1122-AQUEUE-1](../rfc1122/catalog.md#rfc1122-aqueue-1). RFC 1122
  §2.3.2.2 says the link layer should save at least the latest packet instead, and send it
  when the address is resolved. A test targets the RFC 1122 statement.
- Check idea: the two documents predict two different outcomes for the first datagram of an
  exchange, so one observation tells them apart: whether the datagram arrives after the
  reply or never arrives.

### RFC826-REQ-4

**A request carries the hardware space, the protocol, the two lengths, the opcode, the
sender addresses and the target protocol address.**

> "The Address Resolution module then sets the ar$hrd field to ares_hrd$Ethernet, ar$pro to
> the protocol type that is being resolved, ar$hln to 6 (the number of bytes in a 48.bit
> Ethernet address), ar$pln to the length of an address in that protocol, ar$op to
> ares_op$REQUEST, ar$sha with the 48.bit ethernet address of itself, ar$spa with the
> protocol address of itself, and ar$tpa with the protocol address of the machine that is
> trying to be accessed." — Packet Generation, `rfc826.txt:176-184`

- Strength: description. Class: wire.
- Check idea: read every field of the request on the link. The sender fields hold the
  addresses of the host that sent it, and the target protocol address holds the address
  that the host wants.

### RFC826-REQ-5

**The target hardware address of a request means nothing and may be the hardware broadcast
address.**

> "It does not set ar$tha to anything in particular, because it is this value that it is
> trying to determine. It could set ar$tha to the broadcast address for the hardware (all
> ones in the case of the 10Mbit Ethernet) if that makes it convenient for some aspect of
> the implementation." — Packet Generation, `rfc826.txt:184-188`

The rationale section says the same thing about the field:

> "It has no meaning in the request form, since it is this number that the machine is
> requesting." — Why is it done this way, `rfc826.txt:312-313`

- Strength: may. Class: wire.
- Check idea: the field carries any value in a request, so a check reads it and asserts
  nothing about the value. A receiver must not depend on it either.

### RFC826-REQ-6

**The request is broadcast to all stations on the hardware that the routing chose.**

> "It then causes this packet to be broadcast to all stations on the Ethernet cable
> originally determined by the routing mechanism." — Packet Generation,
> `rfc826.txt:188-190`

- Strength: description. Class: wire.
- Check idea: the frame that carries the request has the hardware broadcast address as its
  destination, and every station on the link receives it.

## Packet reception

### RFC826-RECV-1

**A negative answer in the reception algorithm ends the processing and discards the packet.**

> "When an address resolution packet is received, the receiving Ethernet module gives the
> packet to the Address Resolution module which goes through an algorithm similar to the
> following. Negative conditionals indicate an end of processing and a discarding of the
> packet." — Packet Reception, `rfc826.txt:197-201`

- Strength: description. Class: end-to-end (absence).
- Check idea: this sentence gives every question of the algorithm its negative branch. A
  check of a discard asserts two things: the receiver sends nothing, and the receiver goes
  on working. The entries below name the individual questions.

### RFC826-RECV-2

**A packet whose hardware space the receiver does not have is discarded.**

> "?Do I have the hardware type in ar$hrd? / Yes: (almost definitely)" — Packet Reception,
> `rfc826.txt:203-204`

- Strength: description. Class: end-to-end (absence).
- Governed by: [RFC826-RECV-1](#rfc826-recv-1) for the negative branch.
- Check idea: a packet with a hardware space value that the link does not use provokes no
  reply. RFC 5494 reserves two values for experiments, and one of them serves as the
  crafted value; see [RFC5494-NUM-2](../rfc5494/catalog.md#rfc5494-num-2).

### RFC826-RECV-3

**A packet whose protocol the receiver does not speak is discarded.**

> "?Do I speak the protocol in ar$pro?" — Packet Reception, `rfc826.txt:206`

- Strength: description. Class: end-to-end (absence).
- Governed by: [RFC826-RECV-1](#rfc826-recv-1) for the negative branch.
- Check idea: a packet that asks for an address of a protocol the receiver does not run
  provokes no reply, even when the target protocol address would match numerically.

### RFC826-RECV-4

**A sender protocol address that the table already holds updates the hardware address of
that entry.**

> "If the pair <protocol type, sender protocol address> is already in my translation table,
> update the sender hardware address field of the entry with the new information in the
> packet and set Merge_flag to true." — Packet Reception, `rfc826.txt:210-213`

- Strength: description. Class: internal, with a wire consequence.
- Check idea: the update happens before the target test, so it happens for a packet that is
  not addressed to the receiver as well. Its consequence on the wire is where the next
  datagram of the receiver goes; see [RFC826-TABLE-1](#rfc826-table-1).

### RFC826-RECV-5

**A packet whose target protocol address is not the receiver's own is discarded without a
reply.**

> "?Am I the target protocol address? / Yes:" — Packet Reception, `rfc826.txt:214-215`

- Strength: description. Class: end-to-end (absence).
- Governed by: [RFC826-RECV-1](#rfc826-recv-1) for the negative branch.
- Check idea: a request that asks for the address of a third host reaches every station on
  the link, and only the station that owns the target address answers. The others send
  nothing.

### RFC826-RECV-6

**The receiver adds the sender triplet to the table when it is the target and the merge did
not happen.**

> "If Merge_flag is false, add the triplet <protocol type, sender protocol address, sender
> hardware address> to the translation table." — Packet Reception, `rfc826.txt:216-218`

- Strength: description. Class: internal, with a wire consequence.
- Check idea: a host that answered a request needs no request of its own to answer back:
  the table entry came from the request. The example of the document states the outcome —
  "If Y's Internet module then wants to talk to X, this will also succeed since Y has
  remembered the information from X's request for Address Resolution",
  `rfc826.txt:408-410`.

### RFC826-RECV-7

**The sender information enters the table before the opcode is read.**

> "Notice that the <protocol type, sender protocol address, sender hardware address> triplet
> is merged into the table before the opcode is looked at. This is on the assumption that
> communcation is bidirectional; if A has some reason to talk to B, then B will probably
> have some reason to talk to A." — Packet Reception, `rfc826.txt:227-231`

The algorithm says it once more, at the point where it matters:

> "?Is the opcode ares_op$REQUEST? (NOW look at the opcode!!)" — Packet Reception,
> `rfc826.txt:219`

- Strength: description. Class: internal, with a wire consequence.
- Check idea: a packet whose opcode is a reply fills the table of the receiver in the same
  way a request does. The consequence on the wire is that a later datagram to the sender of
  that reply needs no request.

### RFC826-RECV-8

**A request addressed to the receiver is answered with the fields swapped and the opcode set
to reply.**

> "Swap hardware and protocol fields, putting the local hardware and protocol addresses in
> the sender fields. / Set the ar$op field to ares_op$REPLY" — Packet Reception,
> `rfc826.txt:221-223`

- Strength: description. Class: wire.
- Check idea: read the four address fields of the reply. The sender fields hold the
  addresses of the host that answered, and the target fields hold the addresses that stood
  in the sender fields of the request.

### RFC826-RECV-9

**The reply goes to the target hardware address on the same hardware, and is not broadcast.**

> "Send the packet to the (new) target hardware address on the same hardware on which the
> request was received." — Packet Reception, `rfc826.txt:224-225`

The example states the negative half:

> "sets the opcode to reply, and sends the packet directly (not broadcast) to EA(X)." — An
> Example, `rfc826.txt:399-401`

- Strength: description. Class: wire.
- Check idea: the frame that carries the reply has the hardware address of the requester as
  its destination, not the broadcast address, and it leaves on the interface that received
  the request.

### RFC826-RECV-10

**A reply is thrown away after the table is updated, and provokes nothing.**

> "?Is the opcode ares_op$REQUEST? (NOW look at the opcode!!)" — Packet Reception,
> `rfc826.txt:219`

The example states the outcome for the other branch:

> "Machine X gets the reply packet from Y, forms the map from <ET(IP), IPA(Y)> to EA(Y),
> notices the packet is a reply and throws it away." — An Example, `rfc826.txt:404-406`

- Strength: description. Class: end-to-end (absence).
- Governed by: [RFC826-RECV-1](#rfc826-recv-1). A reply is the negative branch of the
  opcode question, so the processing ends there.
- Check idea: a reply that no request provoked still updates the table of the receiver, and
  the receiver answers nothing. A packet with an opcode that is neither a request nor a
  reply takes the same branch; see [RFC5494-NUM-3](../rfc5494/catalog.md#rfc5494-num-3).

### RFC826-RECV-11

**The two length fields may be checked for consistency.**

> "[optionally check the hardware length ar$hln] ... [optionally check the protocol length
> ar$pln]" — Packet Reception, `rfc826.txt:205-208`

The rationale gives the reason the check is optional:

> "In theory, the length fields (ar$hln and ar$pln) are redundant, since the length of a
> protocol address should be determined by the hardware type (found in ar$hrd) and the
> protocol type (found in ar$pro). It is included for optional consistency checking, and for
> network monitoring and debugging" — Why is it done this way, `rfc826.txt:288-292`

- Strength: may. Class: encoding.
- Check idea: a receiver that skips the check conforms, so an absent check is not a
  failure. A check can only read the two fields of a sent packet and compare them with the
  address lengths of the same packet.

## The translation table

### RFC826-TABLE-1

**A new sender hardware address supersedes the one the table holds.**

> "Notice also that if an entry already exists for the <protocol type, sender protocol
> address> pair, then the new hardware address supersedes the old one." — Packet Reception,
> `rfc826.txt:231-234`

The Related issue section says why, and adds the outcome on a link:

> "Recall that if the <protocol type, sender protocol address> is already in the translation
> table, then the sender hardware address supersedes the existing entry. Therefore, on a
> perfect Ethernet where a broadcast REQUEST reaches all stations on the cable, each station
> will be get the new hardware address." — Related issue, `rfc826.txt:443-447`

- Strength: description. Class: wire.
- Check idea: after a host learned a mapping, a second packet from the same protocol
  address with a different hardware address changes where the next datagram of that host
  goes.

### RFC826-TABLE-2

**A host sends information about itself only.**

> "Since hosts don't transmit information about anyone other than themselves, rebooting a
> host will cause its address mapping table to be up to date." — Related issue,
> `rfc826.txt:459-461`

- Strength: description. Class: wire.
- Check idea: read the sender fields of every ARP packet a host sends and compare them with
  the addresses of that host. A host never puts the mapping of a neighbour there.
- Note: proxy ARP is the one mechanism that breaks this statement, and it is the reason
  RFC 1027 exists. The standards map leaves RFC 1027 out of scope at this level.

### RFC826-TABLE-3

**Table aging and timeouts are outside the scope of this protocol.**

> "It may be desirable to have table aging and/or timeouts. The implementation of these is
> outside the scope of this protocol." — Related issue, `rfc826.txt:415-416`

- Strength: description. Class: internal.
- Overridden by: [RFC1122-ACACHE-1](../rfc1122/catalog.md#rfc1122-acache-1) and
  [RFC1122-ACACHE-2](../rfc1122/catalog.md#rfc1122-acache-2). RFC 1122 §2.3.2.1 makes the
  flush mechanism a MUST and the timeout value configurable, and its DISCUSSION names this
  sentence as the gap it closes: "The ARP specification [LINK:2] suggests but does not
  require a timeout mechanism", `rfc1122.txt:1308-1309`.
- Check idea: none against this document. The test targets the RFC 1122 entries.

## Generalization

### RFC826-GEN-1

**The hardware fields allow other hardware, and the protocol field should name the protocol
being resolved.**

> "Generalization: The ar$hrd and ar$hln fields allow this protocol and packet format to be
> used for non-10Mbit Ethernets. ... For other hardware networks, the ar$pro field may no
> longer correspond to the Ethernet type field, but it should be associated with the
> protocol whose address resolution is being sought." — Generalization,
> `rfc826.txt:236-242`

- Strength: should. Class: encoding.
- Check idea: the statement is about hardware other than the Ethernet, and the in-scope set
  of this pass has one hardware type. The standards map files the other hardware types at
  level 5.

## Out of scope in this catalog

Four parts of the document state no checkable behaviour of a host.

- **The Problem, the Motivation, and the Abstract**, `rfc826.txt:17-110`. They say why a
  translation protocol is needed. The one statement of substance in them, that the Ethernet
  needs 48-bit addresses which protocol addresses do not match, is the reason for the
  protocol and not a rule of it.
- **Network monitoring and debugging**, `rfc826.txt:326-364`. A monitor is a station that
  enters every sender triplet it sees whether it is the target or not, which is the opposite
  of [RFC826-RECV-5](#rfc826-recv-5). The document describes the monitor in the permissive
  voice ("it can determine", "the monitor could try") and a monitor is not a host. The
  area belongs to level 5, with the other optional roles.
- **The timeout proposals of the Related issue section**, `rfc826.txt:418-457`. The section
  offers three ways to age an entry and settles on none: "This issue clearly needs more
  thought if it is believed to be important", `rfc826.txt:468-469`. RFC 1122 §2.3.2.1
  governs the area; see [RFC826-TABLE-3](#rfc826-table-3).
- **The rest of the Why is it done this way section**, `rfc826.txt:245-323`. It argues for
  the design: why not periodic broadcast, why one resolution per packet, why 16 bits for the
  opcode. Four of its sentences do state a rule, and they are catalogued:
  [RFC826-FMT-4](#rfc826-fmt-4), [RFC826-FMT-6](#rfc826-fmt-6),
  [RFC826-FMT-7](#rfc826-fmt-7) and [RFC826-RECV-11](#rfc826-recv-11).

The Notes section also asks that requests for hardware space values go to the author by
network mail, `rfc826.txt:68-75`. RFC 5494 replaced that procedure with the IANA rules; see
[`rfc5494/catalog.md`](../rfc5494/catalog.md).
