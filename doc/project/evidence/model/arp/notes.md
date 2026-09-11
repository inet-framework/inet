# ARP — quirks and follow-ups

> **Kind:** reference · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [results.md](results.md), [coverage.md](coverage.md)

What this document is for. The workflow's artifacts each answer one question: what the
standard says, what a run showed, what the model claims. This one holds what a person
learned while doing the work and would otherwise have to learn again — the behaviours of the
model that surprised the author, the traps in the tooling that cost a debugging cycle, and
the ordered list of what to do next.

The tooling quirks that apply to every protocol are in
[`ipv4/notes.md`](../ipv4/notes.md#tooling-quirks). What follows is what ARP added, in one
pass: levels 1, 2 and 3 together, on 2026-09-11.

## Model quirks

### Four fields of the packet do not exist

`ArpPacket` carries the opcode and the four addresses, and nothing else. Its own comment
lists what it leaves out
([`ArpPacket.msg:31-34`](../../../../../src/inet/networklayer/arp/ipv4/ArpPacket.msg)): the
hardware type, the protocol type, and the two address lengths. `ArpPacketSerializer` writes
the four constants on the way out and compares them on the way in.

Three consequences, and each one shaped a check:

- A check that needs one of the four values cannot build the packet from field values. The
  suite has a second builder for that, `buildRawArpPacket` in `ArpMutations.h`, which writes
  the 28 octets itself.
- A check that wants to *read* one of the four values has to serialize the observed packet,
  which is what `arpOctets` does. The octets are the honest representation of what the wire
  carries; there is nothing else to read.
- RFC826-GEN-1, the generalization to other hardware, is not reachable against this model at
  all. The ledger records it as `no check` for that reason, and it is not a level 3 blocker
  because the statement is a `should`.

### Arp::processArpPacket stops the run in three places

The one method that receives every ARP packet ends the run on three different inputs:

| Input | Where | RFC 826 wants |
| --- | --- | --- |
| an unspecified sender hardware address | [`Arp.cc:268-269`](../../../../../src/inet/networklayer/arp/ipv4/Arp.cc) | the algorithm has no question about it; the packet would be merged and then discarded |
| an unspecified sender protocol address | [`Arp.cc:270-271`](../../../../../src/inet/networklayer/arp/ipv4/Arp.cc) | the same. This is the shape RFC 5227 calls an ARP Probe |
| an opcode that is neither 1, 2, 3 nor 4 | [`Arp.cc:357-358`](../../../../../src/inet/networklayer/arp/ipv4/Arp.cc) | end of processing, and the packet discarded. This is gap 1 |

A fourth one is not a `throw` in this file: the method peeks the packet at
[`Arp.cc:231`](../../../../../src/inet/networklayer/arp/ipv4/Arp.cc) with the default flags, and
a packet the serializer marked incorrect cannot be peeked. That is gap 2.

All four are the same decision taken four times: treat an input the module cannot handle as
a programming error rather than as a packet from an untrusted link. It is a defensible
choice inside a simulation, and RFC 826 asks for the opposite in one sentence at the head of
its reception algorithm. The pass tested two of the four; the second row is a level 4
question, because its document is RFC 5227.

### The waiting datagram is in the wrong layer, and there is more of it than the standard asks

RFC 1122 §2.3.2.2 asks the **link layer** to save **at least one** datagram per unresolved
address. The model keeps a `cPacketQueue` per address in the **network layer**
([`Ipv4.h:91`](../../../../../src/inet/networklayer/ipv4/Ipv4.h)) and saves every datagram.
Both differences favour the model, and neither is visible from a link, so the check passes
and the note records the difference.

### Proxy ARP is on by default

`proxyArpInterfaces` defaults to `"*"`
([`Arp.ned:45`](../../../../../src/inet/networklayer/arp/ipv4/Arp.ned)), so any node that can
route a destination out of another interface answers for it. That is RFC 1027 behaviour, and
RFC826-TABLE-2 says a host sends information about itself only. No check of this pass meets
the case, because a host with one interface never has another interface to route out of. A
router mockup would meet it at once, and it is the second follow-up below.

### The flush is a lifetime test at the moment of use

There is no periodic sweep of the table. `Arp::resolveL3Address` compares
`lastUpdate + cacheTimeout` against the current time and starts a new resolution when the
entry is too old ([`Arp.cc:413-421`](../../../../../src/inet/networklayer/arp/ipv4/Arp.cc)), and
`Arp::getL3AddressFor` does the same in the other direction. An entry that nobody asks for
stays in the table forever, and a reader of the table would see it. RFC 1122 §2.3.2.1 asks
for a mechanism and names four possibilities, and this is a variant of the first one, so the
check passes. A module test that reads the table would see the difference.

### Two senders have no callers

`Arp::sendArpGratuitous` and `Arp::sendArpProbe` are public methods that nothing in the tree
calls, and neither is part of the `IArp` interface. The second one names RFC 5227 in its
comment. Anything at level 4 starts here.

### The resolution signals are not packets

`Arp` emits five signals ([`Arp.ned:48-52`](../../../../../src/inet/networklayer/arp/ipv4/Arp.ned)).
Two carry a packet, `arpRequestSent` and `arpReplySent`, and the framework can observe those.
The other three — `arpResolutionInitiated`, `arpResolutionCompleted`, `arpResolutionFailed` —
carry an `IArp::Notification` object, which is neither a packet nor a scalar, so the tester
turns none of them into an event. Every check of this pass watches the interface of a host
instead, which is where a neighbour would watch.

## Tooling quirks

### A protocol name in a content expression resolves to the last chunk of that protocol

This one cost the first debugging cycle of the pass. `ethernetmac.dest` does not read the
destination address of the frame: the dissector attributes both the Ethernet header and the
frame check sequence to the protocol `ethernetmac`, and `evalPacketField` keeps the last
chunk it saw. The error it raises says so plainly once you read it: `field 'dest' not found
on 'inet::EthernetFcs'`.

Read a frame field through the chunk class name instead: `EthernetMacHeader.dest`. The guide
now carries the rule among its expression pitfalls.

### A MacAddress field comes back as a pointer

`evalPacketField(packet, "arp.srcMacAddress")` returns a `cValue` of pointer type. The text
form is `value.pointerValue().get<MacAddress>()->str()`, which `macAddressField` in
`ArpMutations.h` wraps. An `Ipv4Address` behaves the same way.

### `.protocol("arp")` matches nothing at a MAC module

At `mac.packetSentToLower` the packet's `PacketProtocolTag` names the link-layer protocol,
not ARP, so a `protocol("arp")` clause never matches there. Select the packet by its content
instead: `.packet("arp.opcode == 1")` reaches through the encapsulation, because the
`PacketFilter` dissects.

### `packetSentToUpper` carries a packet whose header is gone

At `hostB.udp` the signal `packetSentToUpper` fires with the payload, not the datagram, so
`udp.destPort == 5000` never matches. Either watch `packetReceivedFromLower` at the same
module, which still has the header, or identify the packet by its data length, as
`Rfc1122ArpPacketQueue.test` does. The difference matters when the check is about the handoff
to the program and not about the arrival at the host.

### The tester ends the run at its verdict, so `finish()` output can under-report

The `ProtocolTester` calls `endSimulation()` the moment it decides, which can be before a
packet in flight reaches a program. A first version of `Rfc1122ArpPacketQueue.test` asserted
the line `hostB.app[0]: received 1 packets` and got `received 0 packets`: the verdict came at
the UDP module and the run ended before the program got the datagram. Where a check needs the
program to have it, watch the handoff and not the count.

### A greedy step consumes its whole window

`betweenTimes`, `atLeastTimes`, `atMostTimes`, `anyNumberOfTimes` and `never` all wait out
their `within` window before they advance, so a positive observation that happens inside that
window cannot be a later step. Three tests of this suite hit it and moved the observation to
a line the receiving program prints at the end of the run, with a deviation note in the
`%description`. Two more reordered their steps so that the watch closes before the frame the
next step reads.

### The raw event trace is not available in a self-contained test

`ProtocolTester` forces `logEvents` off as soon as a program runs
([`ProtocolTester.cc:100`](../../../../../tests/protocol/lib/ProtocolTester.cc)), and a test that
uses `Define_ProtocolTestProgram()` always runs one. The way to read the real sequence while
authoring is the EV log itself: every packet dump in `work/<Test>/test.out` names the chunks
and their field values, which is what the trace would have shown.

### An injected ARP packet needs its dispatch tags, but not its interface tag

`buildArpPacket` adds `PacketProtocolTag`, `DirectionTag` and a `DispatchProtocolReq` with
the indication primitive, which is what carries the packet up the dispatcher chain to the
`arp` module. It also adds `InterfaceInd`, and that one is redundant: a push into
`eth[0].upperLayerOut` makes the interface add it
([`NetworkInterface.cc:232-234`](../../../../../src/inet/networklayer/common/NetworkInterface.cc)).
The helper keeps it so that the packet it returns is complete on its own.

### opp_test calls a run that stops a FAIL, not an ERROR

A simulation that ends with an error prints no verdict line, so the assertion on that line
does not find it and `opp_test` reports FAIL. The three tests of this suite that find a gap
therefore declare `%# expected-result: FAIL`. `ERROR` is for a test that cannot run at all.

## Follow-ups, in the order I would do them

1. **Level 4: RFC 5227.** The document the standards map holds back, and the one the model
   already names in a comment. Two questions first: can the model probe at all, given that
   `sendArpProbe` has no caller, and what does a received probe do to a receiver, given the
   throw at [`Arp.cc:270-271`](../../../../../src/inet/networklayer/arp/ipv4/Arp.cc)?
2. **A router mockup.** It would reach three things at once that one link cannot: proxy ARP
   and RFC826-TABLE-2, `Arp::addressRecognized` with two interfaces, and the "on the same
   hardware" half of RFC826-RECV-9.
3. **A serializer unit test.** Gap 2 lives in the serializer and a unit test would find it
   from one packet with no network. The corner cases of the layout belong there too; see
   [`categories.md`](categories.md#where-a-second-category-would-add-something).
4. **A module test for the discard.** Gap 1 and gap 2 are both a missing discard, and a
   protocol test can only watch a run stop. A module test could assert what a fixed model
   would show: the table unchanged, the module still running.
5. **Level 5: RFC 1027 and RFC 903.** Proxy ARP as a feature of its own, and the reverse
   protocol the model deliberately refuses. The refusal is worth a check: the model throws on
   both RARP opcodes, and a station that receives one should stay silent.
