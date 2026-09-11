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

### Four crafted inputs stop the simulation

- An **atomic fragment** (offset 0, M = 0): `Ipv6FragBuf::addFragment` finds the buffer
  before it creates it and erases the stale end iterator when the first fragment completes
  the datagram ([Ipv6FragBuf.cc:168](../../../../../src/inet/networklayer/ipv6/Ipv6FragBuf.cc#L168)).
  A C++ library assertion, and a nine-minute stack trace in a debug build. Any
  single-fragment datagram does it.
- **Overlapping fragments**: `ChunkBuffer::replace` merges them, the buffer completes a
  short datagram, and `Ipv6::decapsulate` asserts on the payload length, because the header
  still says 1460. RFC 5722 wants a silent discard.
- An **unknown ICMPv6 type of 128 or above**: `throw cRuntimeError("Unknown ICMPv6 message
  type ...")` ([Icmpv6.cc:191](../../../../../src/inet/networklayer/icmpv6/Icmpv6.cc#L191)).
  A type below 128 is fine: the switch treats every such type as an error first.
- A **report about a packet of an unknown upper-layer protocol**, received at the source:
  ICMPv6 hands the indication to a dispatcher that has no such protocol, and the dispatcher
  throws. Protocol numbers 253 and 254 carry INET-internal names ("nexthopforwarding",
  "echo"), which is why the error message names one.

### The suppression rules stop at the IP layer

`Icmpv6::validateDatagramPromptingError` covers the multicast destination, the unspecified
and multicast source, and the error-about-error case; not the link-layer multicast or
broadcast. Same as IPv4.

### The fragment identification is a counter

`curFragmentId++`, from 0. RFC 8504 §5.1 says a node should avoid predictable values; a
declined should, recorded in the ledger.

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

### The frame at a relay begins with a physical-layer header

A `PacketTap` hands the mutator the frame as it travels: an 8-octet Ethernet physical-layer
header precedes the link-layer header. A helper that measures a payload from the front of
the frame is off by 8; measure from the IPv6 header's own position (`shortenFrame()` in
`Ipv6Fields.h` does).

### A protocol number without a dissector cannot be filtered

`ipv6.protocolId == 253` never matches: the dissector has no handler for the number, and the
filter turns the exception into a non-match. Read the base header by type
(`ipv6NextHeader()`).

### The internet layer hands ICMPv6 over without a signal

`Ipv6` sends an ICMPv6 message to its ICMPv6 module without `packetSentToUpper`
([Ipv6.cc:887-888](../../../../../src/inet/networklayer/ipv6/Ipv6.cc#L887-L888)). A step
that waits for that signal never fires; anchor on the arrival at the interface instead.

### Two relay rules need two taps, and a relay can hide a crash

One tap carries one rule. When the node under test behaves and the *other* node crashes on
the reply, a second tap that drops the reply lets the first verdict stand, and the crash
gets a test of its own (the unrecognized-next-header pair).

### A crash need not cost minutes

A C++ assertion in a debug build ends with a symbolized stack trace of the 880 MB library.
The atomic-fragment test took about nine minutes for that reason alone, which was the whole
runtime of the suite.

The cost is not the crash; it is the trace. OMNeT++'s common library installs
`backward::SignalHandling`, a global object that catches SIGABRT and prints a stack trace
annotated with source lines. Twenty-five frames resolved against a debug build of all of INET
take those minutes. The trace has no value in a test whose expected result is the crash and
whose description already names the file and the line.

The cure is one line in the test, at file scope, after the runtime's own handler is installed:

```cpp
static const int restoreDefaultAbortHandler = (signal(SIGABRT, SIG_DFL), 0);
```

The suite went from 9 minutes 4 seconds to 2.4 seconds, with the same 29 tests and the same
19 PASS and 8 undeclared failures, every one a defect. The assertion message still reaches `test.err`, so the diagnosis
is unchanged; only the frames are gone, and removing the line brings them back while somebody
works on the gap.

Two conditions decide whether a test needs this. It needs it when the model **aborts** — an
assertion, a `std::terminate`, a segmentation fault. It does not need it when the model raises
a `cRuntimeError`, which OMNeT++ reports and exits cleanly: the other three crashing tests of
this suite end in under a second for that reason. Keep the test either way; the crash is the
finding.

### Nodes and configurator

`StandardHost6` and `Router6` are `StandardHost` and `Router` with `hasIpv4 = false` and
`hasIpv6 = true`; the module paths are `<node>.ipv6.ipv6` for the IPv6 module and
`<node>.ipv6.icmpv6` for ICMPv6. The application's `timeToLive` parameter sets the hop
limit.

## Follow-ups, in the order I would do them

1. **Report the four stops to the model first**: the atomic fragment (any single-fragment
   datagram crashes the buffer), the unknown ICMPv6 type, the report about an unknown
   protocol, and the overlapping fragments. Each is a few lines; none belongs in this
   branch (the tests measure the model as it is, and a failing test is the finding).
2. **Then the field gaps**: the Packet Too Big MTU, the fragment payload length, and the
   two link-layer suppression cases.
3. **A reassembly pass against RFC 8200 §4.5**, which the model still reads as RFC 2460,
   together with a documentation pass that names RFC 8200 and RFC 4443 in `Ipv6.ned` and
   the message definitions.
4. **Module tests for the handoffs** of IPV6-F-ERROR-DELIVERY (RFC4443-MPR-1, UL-1 to
   UL-3), which no protocol test can see.
5. **Cheap sharpenings**: the pointer field of Parameter Problem, a first fragment without
   its upper-layer header (REASM-7), a multicast source for MPR-9.
6. **Level 4**: the reassembly timer, the rate limit, congestion, and path MTU discovery
   once the MTU field is filled. **Level 5**: the other extension headers, where
   RFC8200-EXT-2 and RFC8504-NR-7 wait.
7. **A serializer unit test** for the ICMPv6 pseudo-header checksum and the 8-octet unit of
   the fragment offset.
