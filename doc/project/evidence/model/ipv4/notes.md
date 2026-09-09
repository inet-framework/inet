# IPv4 — quirks and follow-ups

> **Kind:** reference · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [results.md](results.md), [coverage.md](coverage.md)

What this document is for. The workflow's artifacts each answer one question: what the
standard says, what a run showed, what the model claims. This one holds what a person
learned while doing the work and would otherwise have to learn again — the behaviors of the
model that surprised the author, the traps in the tooling that cost a debugging cycle, and
the ordered list of what to do next. None of it belongs in the catalog, the feature map or
the ledger, and all of it is expensive to rediscover.

It changes with the tree, like any reference. The level 2 entries stay; the level 3 pass
added the rest.

## Model quirks

### The header checksum is not computed by default

[Ipv4.ned:97](../../../../../src/inet/networklayer/ipv4/Ipv4.ned#L97) declares
`checksumMode @enum("declared", "computed") = default("declared")`. In the declared mode
the field holds a fixed placeholder and the recompute never runs. Any check that reads the
checksum must set `**.ipv4.ip.checksumMode = "computed"` in its ini. Name the IPv4 module in
the pattern: `**.checksumMode` also switches UDP and ICMP to computed checksums, and then a
rewritten address fails the UDP pseudo-header checksum instead of the rule under test.

### A wrong checksum alone does not cause a discard — confirmed at level 3

[Ipv4.cc:282](../../../../../src/inet/networklayer/ipv4/Ipv4.cc#L282) reads:

```cpp
if (!ipv4Header->isCorrect() && !ipv4Header->verifyChecksum()) {
```

Because of the `&&`, the checksum is consulted **only when the header is already
structurally wrong**. The level 3 checksum check delivered a datagram with the complemented
checksum straight to UDP. This is RFC 1122 §3.2.1.2, a MUST.

### The receive path validates nothing else either

`Ipv4::handleIncomingDatagram` reads no version field; neither the network layer nor UDP
looks at the source address. A datagram with version 5 or with source 255.255.255.255
reaches the application. The one input rule the model does follow is the destination: a
host with forwarding off drops a datagram for another address, silently.

### An unknown ICMP type stops the simulation

[Icmp.cc:326](../../../../../src/inet/networklayer/ipv4/Icmp.cc#L326):
`default: throw cRuntimeError("Unknown ICMP type %d", ...)`. One crafted ICMP message ends
the run. RFC 1122 §3.2.2 says silently discard. Any scenario that carries external or
fuzzed traffic can hit this.

### The ICMP suppression rules stop at the IP layer

[Icmp::maySendErrorMessage](../../../../../src/inet/networklayer/ipv4/Icmp.cc#L101-L141)
implements four of the five RFC 1122 §3.2.2 prohibitions well. The fifth, a datagram that
arrived as a link-layer broadcast, is not there: the function never looks at the
`MacAddressInd` of the frame. Also missing from the source-address list are the loopback and
class E forms, which RFC 1122 names as examples; not tested yet.

### The reassembly key has three fields

[Ipv4FragBuf.h:31-38](../../../../../src/inet/networklayer/ipv4/Ipv4FragBuf.h#L31-L38):
`Key { id, src, dest }`. RFC 791 §2.3 names four fields; the protocol is missing. Two
datagrams of different protocols with one identification share a buffer. The exposure in
the model's own traffic is small because of the next quirk.

### The identification counter is per module

[Ipv4.cc:1089](../../../../../src/inet/networklayer/ipv4/Ipv4.cc#L1089) increments one
counter for every datagram the module originates, whatever the protocol, starting at 0
([Ipv4.cc:89](../../../../../src/inet/networklayer/ipv4/Ipv4.cc#L89)). Two consequences: it
over-satisfies the uniqueness rule, and it makes a relay able to "rewrite the identification
to that of the previous datagram" by subtracting one. Two level 3 tests rely on that, and
each confirms the result on the wire so that a change of the counter fails a step instead of
passing vacuously.

### A request for TTL 0 is an assertion, not a datagram

[Ipv4.cc:1094-1095](../../../../../src/inet/networklayer/ipv4/Ipv4.cc#L1094-L1095):
`if (ttl != -1) { ASSERT(ttl > 0); }`. RFC 1122 §3.2.1.7 says a host must not send TTL 0;
the model refuses with an assertion in a debug build and would send the zero in a release
build. A protocol test cannot classify that; a module test can.

### Off-net datagrams are sent whole

The model never applies the 576-octet limit of RFC 1122 §3.3.3 to an off-net destination;
it sends the datagram whole and lets the gateway fragment it. A should, declined.

### No active module names a standard

`Ipv4.ned` says "Implements the IPv4 protocol" and `Icmp.ned` says "ICMP implementation".
Neither names RFC 791, RFC 792, RFC 1122 or RFC 6864. At level 3 this costs two
`undocumented` verdicts: the model follows RFC 6864 and cannot be credited for it. The house
style for the fix is one module away, in `Igmpv2.ned`.

## Tooling quirks

Each of these cost a debugging cycle. They are properties of the test framework, the
scenario language, or the model's signals, not of IPv4, and they apply to every protocol in
this tree.

### The protocol test library builds in release mode by default

`tests/protocol/lib/build.sh` defaults to `MODE=release` and then fails to link against a
debug INET with `ld.lld: error: unable to find library -lINET`. Build it as
`MODE=debug ./build.sh`. A fresh worktree needs a full INET build first (`make makefiles &&
make MODE=debug`), which takes a few minutes with ccache; copy `.oppfeaturestate` from a
sibling worktree so the feature set is the same.

### An ini key with a wrong path applies nothing, silently

`*.ipv4.arp.typename = "GlobalArp"` matches nothing: the pattern lacks the host segment,
and the real path is `<network>.<host>.ipv4.arp`. Use `**.` when in doubt. The same defect
hid the checksum mode for a while.

### A negative step can open too late to prove anything

A `never` step anchored after the ICMP report arrives at the source starts **after** the
moment a forbidden forward would have happened. The don't-fragment and TTL-expiry checks
therefore observe the gateway's own discard record — `packetDropped` on `router.ipv4.ip`,
filtered by the identification — right after the stimulus, and keep the `never` as the
guard for the rest of the window. Every level 3 silent-discard and suppression check uses
the same anchor: the node's own record of the event.

### Two consecutive `never` steps cannot cover one window

The second opens when the first closes. "Nothing upward and nothing back" therefore has to
be one `never` over the whole host with a predicate for both halves
(`silenceBroken()` in `Ipv4Mutations.h`), not two steps.

### The MAC's send signal marks the end of the transmission

`packetSentToLower` at an Ethernet MAC is emitted when the frame has left, and the node 50
ns down the link processes it in the same instant. So "the gateway forwards or discards
frame 1" happens **before** "the sender emits frame 2", and an ordered program must list it
that way. Two level 3 tests had it the other way round and missed the discard.

### A UDP drop record carries no protocol tag

`Udp::processUDPPacket` removes the `PacketProtocolTag` before it looks for a socket
([Udp.cc:932](../../../../../src/inet/transportlayer/udp/Udp.cc#L932)), so the
`packetDropped` record of a closed port cannot be dissected: `udp.destPort == N` never
matches it, and the step times out although the trace shows the drop and the report. Read
the UDP header at the front of the record instead (`udpDestinationPort()` in
`Ipv4Mutations.h`).

### A fragment is not dissected beyond IP

`udp.destPort` and `icmpv4.type` never match a fragment, not even the first one.
`ipv4.protocolId == 17` does; the enum field compares as a number.

### The relay: what a mutator can and cannot do

- The frame a `PacketTap` hands to a `mutate` lambda starts with the link-layer header;
  `rewriteHeader<T>()` in `Ipv4Mutations.h` removes the chunks in front of the header of
  type `T`, hands a mutable copy over, and puts everything back. `rewriteIpv4Header()` also
  recomputes the header checksum, so a rewritten field never fails for the checksum.
- A mutator cannot read captures. Anything it needs from an earlier datagram has to come
  from a property of the model (the counter minus one) and be confirmed by a later step.
- One tap carries one rule. Two taps in series work, and the configurator joins the whole
  chain into one link.
- A `delay` holds only the selected frame; other frames pass while it is held.
- The ICMP checksum survives a type rewrite only because the model's ICMP runs in declared
  mode. In computed mode the rewrite would need a recompute that the model exposes only as a
  member of `Icmp`.

### Two environment scripts, a discovery crash, and how the runner counts a crash

The runner needs the `setenv` of OMNeT++ **and** the `setenv` of INET sourced, in that
order. Pass `-p inet` to skip project discovery, which crashes on a `~/.omnetpp` directory.
A simulation that stops with a runtime error counts as `FAIL`, not `ERROR`: declare
`%# expected-result: FAIL` and say in the description why the verdict line is missing.

## Follow-ups, in the order I would do them

1. **Report the four robustness gaps to the model.** The unknown-ICMP-type crash first, then
   the checksum `&&`, the missing version check, and the missing source validation. Each
   is a few lines in `Ipv4.cc` or `Icmp.cc`; none belongs in this branch (the tests measure
   the model as it is).
2. **The reassembly key** (`Ipv4FragBuf::Key` needs the protocol) and **the link-layer
   broadcast** rule (`maySendErrorMessage` needs the `MacAddressInd`). Same rule: report,
   do not fix here.
3. **A module-test pass for IPV4-F-ERROR-DELIVERY** (RFC1122-ICMP-3, DU-2, TE-1, PP-2), the
   two interface statements (REASM-3, FRAG-2), and TTL-1. That is what turns the
   `unverified` row into a verdict and lifts five of the six features that are level 3
   partial.
4. **A standing negative rule in the framework** for RFC1122-ADDR-4 (and it would let the
   level 2 negative checks stay on the wire).
5. **Cheap sharpenings within the relay toolset:** loopback and class E sources for
   RFC1122-ICMP-9 (the code predicts a gap), a subnet-directed broadcast and a multicast
   destination for ICMP-6, a crafted total length for PP-1, a small host MTU for FRAG-1 and
   FRAG-3.
6. **Level 4 opens with the reassembly timeout** (RFC1122-REASM-4, REASM-5): the
   `fragmentTimeout` parameter and a withheld fragment; the ICMP Time Exceeded code 1 only
   when fragment zero arrived.
7. **RFC6864-ID-4 in the TCP suite**, where a retransmission is one dropped segment away.
8. **A serializer unit test** for the checksum algorithm and for the 8-octet unit of the
   fragment offset, as before.
