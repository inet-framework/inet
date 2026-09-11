# ARP — English check procedures: the packet format

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc826/catalog.md](../../../standard/rfc826/catalog.md), [rfc5494/catalog.md](../../../standard/rfc5494/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Packet layout on the wire

Checks: **RFC826-FMT-1** (description), **RFC826-FMT-2** (description), **RFC826-FMT-4**
(description), **RFC826-FMT-7** (description), **RFC826-RECV-11** (may),
**RFC5494-NUM-1** (description), **RFC5494-NUM-4** (description).

### Requirement

RFC 826, Packet format: the data of the packet is nine fields in one order — a 16-bit
hardware space, a 16-bit protocol space, an 8-bit hardware length, an 8-bit protocol
length, a 16-bit opcode, then the sender hardware address, the sender protocol address, the
target hardware address and the target protocol address. The widths of the four address
fields come from the two length fields of the same packet. There is no padding between the
addresses, and the three 16-bit fields go most significant byte first. For the 10 Mbit
Ethernet the hardware space is 1 and the hardware length is 6. RFC 5494 §3 reserves 0 and
65535 in the hardware space and in the opcode, and RFC 5494 §2 says the protocol space
holds Ethertype numbers.

### Size and value arithmetic

The link is an Ethernet and the protocol resolved is IPv4, so the two length fields are 6
and 4. The data of the packet is therefore

    2 + 2 + 1 + 1 + 2 + 6 + 4 + 6 + 4 = 28 octets

and the octets of a request, read from the front, are

| Offset | Octets | Field | Value |
| --- | --- | --- | --- |
| 0 | 2 | hardware space | 0x00 0x01 |
| 2 | 2 | protocol space | 0x08 0x00, the Ethertype of IPv4 |
| 4 | 1 | hardware length | 0x06 |
| 5 | 1 | protocol length | 0x04 |
| 6 | 2 | opcode | 0x00 0x01 for a request, 0x00 0x02 for a reply |
| 8 | 6 | sender hardware address | the hardware address of the sender |
| 14 | 4 | sender protocol address | the protocol address of the sender |
| 18 | 6 | target hardware address | meaningless in a request |
| 24 | 4 | target protocol address | the address being resolved |

The first two rows are the two values the reserved-value rule forbids, read the other way
round: 1 is neither 0 nor 65535, and 0x0800 is neither.

### Scenario constants

- The mockup is the pair. One exchange, started by one datagram from host A at 0.1 s.

### Procedure

1. Build the mockup with two hosts on one Ethernet link.
2. Let host A send its datagram, so that a request and a reply cross link 1.
3. Read the octets of the request, in order, and compare them with the table above.

### Expected observations

1. The request on link 1 is 28 octets of ARP data.
2. Its first two octets are 0x00 0x01: the hardware space of the Ethernet, most significant
   byte first.
3. Its next two octets are 0x08 0x00: the protocol space of IPv4, from the Ethertype space.
4. Its hardware length octet is 6 and its protocol length octet is 4, and these are the
   widths that the four address fields actually have in the same packet.
5. Its opcode octets are 0x00 0x01.
6. The sender hardware address starts at offset 8 and the sender protocol address follows
   it at offset 14, with no octet between them.
7. The packet holds exactly one target protocol address.

### Notes

- Observation 4 is RFC826-RECV-11 seen from the sending side. The statement permits a
  receiver to check the two lengths against the addresses; a check of a packet on the link
  can only establish that the two agree, which is the condition that makes such a check
  possible at all.
- Observation 6 is the no-padding rule. It is the one rule of RFC826-FMT-4 that a check can
  see directly; the byte order half of the same statement is observation 2, where 0x00 0x01
  and not 0x01 0x00 is the whole point.
- Observation 7 is RFC826-FMT-7. A 28-octet packet with one target protocol address cannot
  hold a second resolution, so the length is the evidence.
- The reply is not read here. Its length is checked in
  [Reply field values](reply.md#reply-field-values), observation 6, and its fields are
  checked there field by field.
- This check reads octets and not fields, which makes it an encoding check. The step 9
  table calls an encoding statement a unit test by default. It stays here because the
  octets it reads are the octets of a packet that a real exchange put on a link, and that
  is what the format statements are about.
