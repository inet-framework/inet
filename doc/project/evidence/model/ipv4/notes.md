# IPv4 — quirks and follow-ups

> **Kind:** reference · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [results.md](results.md), [coverage.md](coverage.md)

What this document is for. The workflow's artifacts each answer one question: what the
standard says, what a run showed, what the model claims. This one holds what a person
learned while doing the work and would otherwise have to learn again — the behaviors of the
model that surprised the author, the traps in the tooling that cost a debugging cycle, and
the ordered list of what to do next. None of it belongs in the catalog, the feature map or
the ledger, and all of it is expensive to rediscover.

It changes with the tree, like any reference.

## Model quirks

### The header checksum is not computed by default

[Ipv4.ned:97](../../../../../src/inet/networklayer/ipv4/Ipv4.ned#L97) declares
`checksumMode @enum("declared", "computed") = default("declared")`. In the declared mode
the field holds a fixed placeholder and the recompute never runs. Any check that reads the
checksum must set `**.checksumMode = "computed"` in its ini. Note the pattern: a single
`*.` before a module name does not match, see the tooling quirks below.

### A wrong checksum alone does not cause a discard

[Ipv4.cc:282](../../../../../src/inet/networklayer/ipv4/Ipv4.cc#L282) reads:

```cpp
if (!ipv4Header->isCorrect() && !ipv4Header->verifyChecksum()) {
```

Because of the `&&`, the checksum is consulted **only when the header is already
structurally wrong**. A structurally correct header carrying a wrong checksum passes. That
is RFC 791 §1.4, "the internet datagram is discarded at once by the entity which detects the
error", and it is the strongest candidate for a `defect` verdict when level 3 arrives with
the interception toolset. Contrast UDP, which verifies unconditionally in its computed mode.

### The identification counter is per module

[Ipv4.cc:1089](../../../../../src/inet/networklayer/ipv4/Ipv4.cc#L1089) increments one
counter for every datagram the module originates, rather than one per source, destination
and protocol. That over-satisfies RFC791-ID-1 and under-serves RFC 6864, which is out of
scope.

### No active module names a standard

`Ipv4.ned` says "Implements the IPv4 protocol" and `Icmp.ned` says "ICMP implementation".
Neither names RFC 791 or RFC 792. The only two mentions in the whole tree sit in
BSD-derived headers that the simulation does not use for the datagram. The house style for
the fix is one module away, in `Igmpv2.ned`.

## Tooling quirks

Each of these cost a debugging cycle. They are properties of the test framework and the
scenario language, not of IPv4, and they apply to every protocol in this tree.

### The protocol test library builds in release mode by default

`tests/protocol/lib/build.sh` defaults to `MODE=release` and then fails to link against a
debug INET with `ld.lld: error: unable to find library -lINET`. Build it as
`MODE=debug ./build.sh`.

### An ini key with a wrong path applies nothing, silently

`*.ipv4.arp.typename = "GlobalArp"` matches nothing: the pattern lacks the host segment,
and the real path is `<network>.<host>.ipv4.arp`. The key sat in the pass 1 templates and
had never taken effect — the traces carried ARP frames throughout. Use `**.` when in doubt.
The same defect hid the checksum mode for a while.

### A negative step can open too late to prove anything

This is the subtlest of them. A `never` step anchored after the ICMP report arrives at the
source starts **after** the moment a forbidden forward would have happened: the gateway
decides, and the report crosses the link afterwards. The don't-fragment and TTL-expiry
checks therefore observe the gateway's own discard record — `packetDropped` on
`router.ipv4.ip`, filtered by the identification — right after the stimulus, and keep the
`never` as the guard for the rest of the window. A `never` that passes because its window
opened after the event is a vacuous pass, and it looks exactly like a real one.

### Two environment scripts, and a discovery crash

The runner needs the `setenv` of OMNeT++ **and** the `setenv` of INET sourced, in that
order. Pass `-p inet` to skip project discovery, which crashes on a `~/.omnetpp` directory.

## Follow-ups, in the order I would do them

1. **Level 3 opens with RFC791-CKSUM-2**, the discard of a bad header. The code reading
   above says what the check will find. It needs interception to corrupt a datagram in
   flight.
2. **RFC791-REASM-3**, fragments of two datagrams that must not mix. Needs injection or two
   interleaved fragment trains.
3. **Bring RFC 1122 and RFC 6864 into the in-scope set.** RFC 1122 changes the strength of
   several statements, including making the reassembly feature's level explicit;
   RFC 6864 rewrites the identification requirement. Both need their own catalog files.
4. **A standing negative rule in the framework**, or a `never` that runs beside later steps.
   It would let the two negative checks drop the discard-record step and stay on the wire,
   which is where the requirement actually lives.
5. **Sharpen the fragment check** with the total lengths (572 B and 476 B) as unit literals,
   and add a 576-octet datagram that arrives whole, for the other half of RFC791-REASM-2.
6. **A serializer unit test** for the checksum algorithm and for the 8-octet unit of the
   fragment offset. Both are encoding statements that a protocol test cannot reach.
