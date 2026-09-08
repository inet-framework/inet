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
late. What follows is what UDP added.

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

### The program's receipt is not observable

`UdpSink` emits only `packetReceived`, which the tester does not turn into an event. Every
"the application received N octets" observation is UDP's upward handoff instead
(`packetSentToUpper` with a size predicate), and two programs are told apart by the sizes of
their data.

## Follow-ups, in the order I would do them

1. **Bring RFC 1122 §4.1 into the in-scope set.** It turns the checksum from optional into
   mandatory to implement and to check, and it adds the discard rule. That single document
   moves `UDP-F-CHECKSUM` from `optional` to `mandatory` and opens level 3.
2. **Inject an 8-octet datagram**, so that the minimum length of RFC768-HDR-3 is observed at
   the receiver, since no sender can produce it.
3. **A serializer unit test for RFC768-CKSUM-1**, the one's complement sum over the pseudo
   header. It is the only way to tell a real checksum from the declared placeholder, and the
   wire check cannot.
4. **Teach the tester to read the indication tags** that UDP attaches on delivery, so that
   the source port and address halves of RFC768-UI-1 can be observed at the program
   interface rather than inferred from the wire.
5. **Ask why the drop path strips the protocol tag.** If it is not deliberate, keeping it
   would make every UDP drop observable by field, here and elsewhere.
6. **Level 5, when it comes: RFC 9868**, the UDP options in the surplus area beyond the
   length field, published October 2025 and the only document that updates RFC 768.
