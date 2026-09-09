# UDP — quirks and follow-ups

> **Kind:** reference · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [results.md](results.md), [coverage.md](coverage.md)

What this document is for. The workflow's artifacts each answer one question: what the
standard says, what a run showed, what the model claims. This one holds what a person
learned while doing the work and would otherwise have to learn again — the behaviors of the
model that surprised the author, the traps in the tooling that cost a debugging cycle, and
the ordered list of what to do next.

The tooling quirks that apply to every protocol are in
[`ipv4/notes.md`](../ipv4/notes.md#tooling-quirks): the release-mode default of the test
library, the ini pattern that silently matches nothing, and the negative step that opens too
late. What follows is what UDP added, in two passes: level 2 on 2026-09-08 and level 3 on
2026-09-09.

## Model quirks

### The checksum is not computed by default, and a disabled mode exists

[Udp.ned:64](../../../../../src/inet/transportlayer/udp/Udp.ned#L64) declares
`checksumMode @enum("disabled", "declared", "computed") = default("declared")`. The three
modes map onto RFC 768 exactly: `computed` generates a real checksum, `disabled` transmits
zero — which the RFC defines as "no checksum was generated" — and `declared` is a
simulation placeholder that is neither. The checksum check needs one sender in each of the
first two modes.

### UDP verifies what IPv4 does not

In the computed mode the UDP receiver checks every nonzero checksum and discards on failure
([Udp.cc:948-956](../../../../../src/inet/transportlayer/udp/Udp.cc#L948-L956)), and accepts
a zero over IPv4 as "none"
([Udp.cc:1012-1017](../../../../../src/inet/transportlayer/udp/Udp.cc#L1012-L1017)). That is
RFC 768 honored exactly, and RFC 1122's discard rule already in place for a level 3 check.

The contrast with the network layer is worth a look by whoever owns both: the IPv4 header
checksum is consulted **only when the header is already structurally wrong**
([Ipv4.cc:282](../../../../../src/inet/networklayer/ipv4/Ipv4.cc#L282)), so a structurally
correct IPv4 header with a wrong checksum passes. The same tree verifies at one layer and
not at the other.

### The drop signal loses its protocol tag

`Udp::processUDPPacket` removes the `PacketProtocolTag` before it drops an undeliverable
datagram. The packet that rides on the `packetDropped` signal can therefore no longer be
dissected as UDP, and `udp.destPort` on it is a silent non-match. The port-unreachable check
matches the discard by size instead. Any observer that filters UDP drops by field is
affected, not only this test. IPv4's drop paths keep the tag, which is why the IPv4 checks
could filter their discards by field.

### The model works from RFC 768 and never names it

`Udp.ned` says "as defined by the RFC" twice and never gives a number, while
[Udp.cc:872-877](../../../../../src/inet/transportlayer/udp/Udp.cc#L872-L877) quotes RFC 768
verbatim at the line that implements its all-ones rule. The one dated citation in the tree
is in a BSD-derived header that nothing includes, and its date is wrong:
[udphdr.h:43](../../../../../src/inet/transportlayer/udp/headers/udphdr.h#L43) says
"Per RFC 768, September, 1981" — RFC 768 is dated 28 August 1980.

### The IPv6 checksum rule is stated without its source

[Udp.cc:88-95](../../../../../src/inet/transportlayer/udp/Udp.cc#L88-L95) paraphrases the
rule that the checksum is mandatory over IPv6, inside a `TODO`, naming no document.

## Scenario quirks

### No application can send an empty datagram

RFC 768 sets the minimum length at eight octets, the header alone. No sender in this model
can produce it: `Packet::insertAt`, which every application reaches through `insertAtBack`,
refuses a chunk of length zero (`CHUNK_CHECK_USAGE(chunk->getChunkLength() > b(0), "chunk is
empty")`), and a run with `messageLength = 0B` stops with `Error: chunk is empty`. This is a
property of the packet library, not of UDP. Whether the **receiver** accepts an 8-octet
datagram is therefore an injection question, and level 3.

### The relay can build the datagram a sender cannot

The note above stands for a sender, and level 3 answered the question it left open. A
`PacketTap` on the path takes a normal datagram and removes its data, and an 8-octet
datagram then reaches the receiver without a sender that could build one. UDP accepts it and
hands zero octets to the program. The run then stops in a result filter of the receiving
program; see gap 3 of [`results.md`](results.md#gap-3--an-empty-datagram-stops-the-run).

### A zero-length payload stops a run in the statistics, not in the protocol

`DataAgeFilter::receiveSignal` (`src/inet/common/ResultFilters.cc`) calls `packet->peekData()`
with no flags. A packet of zero length answers with an empty chunk, which is refused. Every
standard receiving program declares that filter, through
`@statistic[endToEndDelay](source="dataAge(packetReceived)")`. Any protocol that can deliver
an empty payload to an application meets this, so it is worth knowing before a pass on
another protocol spends a debugging cycle on it. The filter needs `PF_ALLOW_EMPTY`, or a
guard for the zero-length case.

### A drop record may have lost its header, not only its tag

The note above says the drop signal loses the protocol tag. Level 3 found the other half:
`Udp::processUdpPacket` pops the UDP header before it records a drop for a wrong checksum,
and it puts the header back only on the path where no program has the port. So a drop record
of a bad checksum has neither the tag nor the header, and neither a filter expression nor
`peekAtFront<UdpHeader>` can read the port. The tests match such a record by the data length
instead, and accept both shapes; see `droppedDatagramWithData` in
`tests/protocol/udp/UdpMutations.h`.

### A header the model only declared correct cannot be serialized

`UdpHeaderSerializer::serializeFields` throws for any checksum mode but disabled or computed.
A helper that computes what a checksum should be must therefore work on a copy of the header
and set the mode on the copy, or the check crashes instead of giving a verdict. The same
trap waits for any check that serializes a chunk the model marked rather than computed.

### The program's receipt is not observable

`UdpSink` emits only `packetReceived`, which the tester does not turn into an event. Every
"the application received N octets" observation is UDP's upward handoff instead
(`packetSentToUpper` with a size predicate), and two programs are told apart by the sizes of
their data.

## Follow-ups, in the order I would do them

The first three of pass 1 are done. What follows is the list as pass 2 leaves it.

1. **Correct gap 2: validate the source address of a received datagram.** It is the only gap
   where the model does nothing at all, and one correction at the IPv4 layer would close it
   for UDP and for IPv4 together. RFC 1122 states the rule at both layers and demands only
   that one of them act.
2. **Decide what the default checksum mode should be** (gap 1). The computed mode is correct
   and five checks depend on it. The question is whether a model of a host should put a
   placeholder on the wire when no one has chosen. It is a decision about defaults, not a
   defect in the module.
3. **Give the result filters a zero-length case** (gap 3), so that an empty payload does not
   stop a run. `dataAge` is the one this pass met; a reader who owns the filters should look
   for the others.
4. **Build a module test category**, one that watches the gate between a protocol module and
   the program above it. Seven statements of RFC 1122 §4.1 wait for it, four of them a
   `must`, and no test of two nodes on a link will ever reach them. This is what keeps UDP at level 3
   partial, and it would serve every other protocol in this tree.
5. **A serializer unit test for RFC1122-UCK-6**, the all-ones rule for a computed zero. It
   needs data whose sum is exactly zero, which a unit test can choose and a run cannot.
6. **Ask why the drop path strips the protocol tag and pops the header.** If neither is
   deliberate, keeping both would make every UDP drop observable by field, here and
   elsewhere.
7. **Give a sending program an interface for IP options**, so that RFC1122-UOPT-2 can be
   checked on the wire.
8. **Level 5, when it comes: RFC 9868**, the UDP options in the surplus area beyond the
   length field, published October 2025 and the only document that updates RFC 768.
