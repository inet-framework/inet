# IPv6 — quirks and follow-ups

> **Kind:** reference · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [results.md](results.md), [coverage.md](coverage.md)

What this document is for. The workflow's artifacts each answer one question: what the
standard says, what a run showed, what the model claims. This one holds what a person
learned while doing the work and would otherwise have to learn again — the behaviors of the
model that surprised the author, the traps in the tooling that cost a debugging cycle, and
the ordered list of what to do next. None of it belongs in the catalog, the feature map or
the ledger, and all of it is expensive to rediscover.

The cross-protocol tooling quirks (the release-mode build of the test library, the
silent ini key with a wrong path, the negative step that opens too late, the MAC send
signal at the end of a transmission, the UDP drop record without a protocol tag) are
recorded once, in [`../ipv4/notes.md`](../ipv4/notes.md#tooling-quirks); this file adds
what IPv6 taught.

## Model quirks

### Packet Too Big carries MTU 0

[Icmpv6.cc:271-273](../../../../../src/inet/networklayer/icmpv6/Icmpv6.cc#L271-L273):
`// TODO implement MTU support.` then `createPacketTooBigMsg(0)`. The message is sent, to
the source, with type 2; the field that path MTU discovery reads is zero. The module claims
RFC 4443 by name. This blocks any level 4 work on RFC 8201 until it is filled; the fix is to
pass the interface MTU from `Ipv6::fragmentAndSend`, where it is known.

### A fragment carries the original payload length

The fragment loop of [Ipv6::fragmentAndSend](../../../../../src/inet/networklayer/ipv6/Ipv6.cc#L1073-L1130)
duplicates the base header, sets its next header to 44, and never rewrites the payload
length. Both fragments of a 1500-octet packet say 1460. The model's own receiver does not
notice, because [Ipv6FragBuf](../../../../../src/inet/networklayer/ipv6/Ipv6FragBuf.cc#L72)
measures the fragment instead of reading the field. A recorded trace shows the wrong value,
and a receiver that follows RFC 8200 §4.5 would compute the fragment length from it.

### The router does not fragment, and there is no discard signal for it

`fragmentAndSend` handles both edge cases of the router in one place: a hop limit of zero
and a packet larger than the next link. Neither emits `packetDropped`; both call the ICMPv6
module. So the in-time observation of a discard is the router's origination of the report,
visible as `packetReceivedFromUpper` at `router.ipv6.ipv6` with the ICMPv6 type.

### The default hop limit is 30, and a request for 0 is an assertion

`IPv6_DEFAULT_ADVCURHOPLIMIT` is 30
([Ipv6InterfaceData.h:31](../../../../../src/inet/networklayer/ipv6/Ipv6InterfaceData.h#L31));
[Ipv6.cc:966](../../../../../src/inet/networklayer/ipv6/Ipv6.cc#L966) asserts a positive hop
limit at encapsulation, as IPv4 does for the TTL. The mockup sets 64 through the
application's `timeToLive` so that the checks do not depend on the default.

### The reassembly key is right

Unlike the IPv4 buffer, `Ipv6FragBuf::Key` holds identification, source and destination —
exactly the three fields of RFC 8200 §4.5. There is no protocol field in the IPv6 rule, so
nothing is missing.

### The model cites RFC 2460 and RFC 2463

The header and extension header definitions, the extension header order, and the
reassembly code cite the obsoleted RFC 2460; `Icmpv6.h` cites RFC 2463. One comment cites
RFC 8200. The reassembly section is the one RFC 8200 changed most.

## Tooling quirks

### IPv6 needs about 4 s before a host can send

Router solicitation and advertisement, address configuration and duplicate address
detection run at start; a datagram handed to IPv6 at 1 s waited in neighbor discovery until
4.2 s. Start the applications at 5 s and give the first step a window of 6 s. The
`Ipv6FlatNetworkConfigurator` assigns the addresses and the routes; neighbor discovery still
runs, and its frames (ICMPv6 types 133 to 136) share the wire with every scenario. Filters
on `udp.destPort` or on a specific ICMPv6 type never see them.

### The fragment header is a chunk of its own, addressed by class name

`Ipv6FragmentHeader.fragmentOffset`, `.moreFragments`, `.identification` and
`.nextHeaderProtocol` work in a filter expression. Two traps beside it: a fragment is not
dissected beyond the fragment header, so `udp.*` never matches one (read the UDP header
from the chunk sequence, `udpDestinationPortInFragment()` in `Ipv6Fields.h`); and the
generic field reader can resolve `ipv6.payloadLength` on a fragment to the fragment header,
which has no such field, and stop the run — read the base header by type
(`ipv6PayloadLength()`). The offset field counts octets in the model and 8-octet units on
the wire.

### `Icmpv6Header` has no code field; the subclasses do

`icmpv6.code` does not exist; use the message class: `Icmpv6TimeExceededMsg.code`,
`Icmpv6PacketTooBigMsg.code`, `Icmpv6PacketTooBigMsg.MTU`. `icmpv6.type` works on every
message.

### Capture arithmetic works; a captured unit does not

`ipv6.hopLimit == {hl} - 1` is a valid expression. A capture of a unit-bearing field
(`ipv6.payloadLength`, "108B") breaks the next step with "Attempt to use the value '108B'
as a dimensionless number", as the TCP pass found; capture such a field with a lambda that
returns a plain number.

### Nodes and configurator

`StandardHost6` and `Router6` are `StandardHost` and `Router` with `hasIpv4 = false` and
`hasIpv6 = true`; the module paths are `<node>.ipv6.ipv6` for the IPv6 module and
`<node>.ipv6.icmpv6` for ICMPv6. The application's `timeToLive` parameter sets the hop
limit.

## Follow-ups, in the order I would do them

1. **Report the two gaps to the model**: the Packet Too Big MTU field and the fragment
   payload length. Both are one line each in the code named above; neither belongs in this
   branch (the tests measure the model as it is, and a failing test is the finding).
2. **A documentation pass** that names RFC 8200 and RFC 4443 in `Ipv6.ned` and the message
   definitions, in the style `Icmpv6.ned` already uses.
3. **Level 3 with RFC 8504**: the message processing rules of RFC 4443 §2.4, the crafted
   fragments of REASM-4 to REASM-6 (the reassembly code quotes RFC 2460 for them, and
   RFC 8200 changed the section), a packet that arrives with hop limit 0, the zero UDP
   checksum, and the content of a report (ERR-1). The relay of the IPv4 pass carries over;
   the rewrite helper needs an IPv6 variant without a header checksum to recompute.
4. **A UDP-over-IPv6 pass** for RFC4443-DU-4 and the transport side of RFC 8200 §8.1.
5. **Level 4**: the reassembly timer, and path MTU discovery once the MTU field is filled.
6. **A serializer unit test** for the ICMPv6 pseudo-header checksum and the 8-octet unit of
   the fragment offset.
