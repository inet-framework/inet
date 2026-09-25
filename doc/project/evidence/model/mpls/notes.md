# MPLS — quirks and follow-ups

> **Kind:** reference · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [results.md](results.md), [coverage.md](coverage.md)

What this document is for. The workflow's artifacts each answer one question: what the standard
says, what a run showed, what the model claims. This one holds what a person learned while doing
the work and would otherwise have to learn again — the behaviors of the model that surprised the
author, the traps in the scenarios and the tooling that cost a debugging cycle, and the ordered
list of what to do next.

The tooling quirks that apply to every protocol are in
[`ipv4/notes.md`](../ipv4/notes.md#tooling-quirks). What follows is what MPLS added, in one
pass: level 2 on 2026-09-24, over RFC 3031, RFC 3032, RFC 3443 and RFC 5462. The gaps themselves
are in [`results.md`](results.md#the-model-gaps).

## Model quirks

### The TTL of a label is a placeholder, and the LSP is invisible to the IPv4 TTL

**Fixed on** 2026-09-25 (gaps 1, 2 and 3): a push copies the TTL of the entry below it or of the
IPv4 header, each LSR forwards with the incoming TTL minus one and discards a packet whose
outgoing TTL is zero, the ingress decrements the IPv4 TTL before the first label, and a pop that
empties the stack writes the outgoing TTL into the IPv4 header. What follows describes the model
before the repair.

`Mpls::pushLabel` and `Mpls::swapLabel` build a new `MplsHeader` and never set its TTL
([Mpls.cc:139-153](../../../../../src/inet/networklayer/mpls/Mpls.cc#L139)), so every entry on the
wire has TTL 0 (gap 1). The IPv4 TTL does not help: the ingress labels a datagram as it comes up
from the link, before IPv4 sees it
([Mpls.cc:202-208](../../../../../src/inet/networklayer/mpls/Mpls.cc#L202)), and the egress sends
the popped datagram down to the link
([Mpls.cc:269-283](../../../../../src/inet/networklayer/mpls/Mpls.cc#L269)). A datagram that
crosses a three-LSR path arrives with the TTL it was sent with (gap 2). With penultimate hop
popping it loses one, because the egress then gets an unlabeled datagram and forwards it by IPv4.
No LSR looks at a TTL at all, so a packet with TTL 2 crosses the whole LSP (gap 3). A repair has
to cover the three together; any one alone leaves the others visible.

### The MPLS models run on PPP links only

**Fixed on** 2026-09-25 (gap 4): a packet of another protocol goes up, and `Mpls` tags a packet
for a link as IPv4 does, so the Ethernet layer encapsulates it. One limit stays: a LIB entry has
no next hop, so the frame goes to the broadcast address of the link. On a point-to-point link
that is the neighbor; on a shared LAN every station gets the frame, and `Icmp` sends no error
about a datagram that arrived in a link-layer broadcast. What follows describes the model before
the repair.

`Mpls::processPacketFromL2` knows two protocols, MPLS and IPv4, and throws "Unknown message
received" for every other one, with a note "FIXME remove throw above"
([Mpls.cc:210-214](../../../../../src/inet/networklayer/mpls/Mpls.cc#L210)). On an Ethernet link
the first ARP request stops the run at 0.008 s (gap 4). With `GlobalArp`, which sends no ARP
packet, the run goes further and stops at the MAC: the labeled packet arrives without an
Ethernet header, "Cannot convert chunk from type inet::MplsHeader to type
inet::EthernetMacHeader". The same throw stops an MPLS router that gets an IPv6 packet from a
link, a Neighbor Solicitation included; since the repair it goes up to the network layer. Every
example of INET that uses MPLS has PPP links.

### IPv4 only, and the pop says so

From the network layer, a packet that is not IPv4 goes to the link unlabeled: "only the Ipv4
protocol supported yet" ([Mpls.cc:69-73](../../../../../src/inet/networklayer/mpls/Mpls.cc#L69)).
The pop of the bottom label sets the protocol to IPv4 whatever the label
([Mpls.cc:160-162](../../../../../src/inet/networklayer/mpls/Mpls.cc#L160)), so the label does not
identify the network layer; it does not have to, as long as IPv4 is the only one. Nothing in the
documentation claims IPv6, so the IPv6 statements are `no check` in the ledger.

### The LIB is the ILM and the NHLFE, and the classifier is the FTN

A `LibTable` entry maps an incoming interface and label to an outgoing interface and a list of
operations, push, swap and pop, applied in order
([Mpls.cc:165-194](../../../../../src/inet/networklayer/mpls/Mpls.cc#L165)). A swap and a push in one
entry make a stack of two; two pops at the egress give the datagram. The ingress needs no RSVP:
the `RsvpClassifier` of `RsvpMplsRouter` maps a destination to a label of its `<fecentry>`, and
`RsvpClassifier::lookupLabel` resolves that label in the LIB with the interface name `""`, which
the LIB entry matches with `<inInterface>any</inInterface>`. The label of the `<fecentry>` is a
handle, not a label on the wire; the tests use 1000. An entry with `any` also matches a labeled
packet that arrives with that value on any interface.

### The LIB refuses the label 0, in a debug build

**Fixed on** 2026-09-25 (gap 5): the LIB accepts the reserved values as outgoing labels, the label
0 at the bottom is popped with no binding, a swap to the label 3 pops the stack, and the LIB
allocates labels from 16. The allocation mattered: RSVP-TE got the labels 1, 2 and 3 before, and
the Implicit NULL rule alone popped the label 3 of two examples. What follows describes the model
before the repair.

`LibTable::readTableFromXML` asserts that each label value is above 0
([LibTable.cc:132, 142, 152](../../../../../src/inet/networklayer/mpls/LibTable.cc#L132)). A
debug build stops at initialization on a swap to 0, which is how `Rfc3032ExplicitNull` fails. A
release build defines `NDEBUG` and drops the `ASSERT`; there the swap to 0 works, and R3, with no
binding for 0, discards the datagram (gap 5). The label 3 is legal to the LIB and goes onto the
wire as an ordinary label.

### No MTU in MPLS, and none in PPP either

**Fixed on** 2026-09-25 for MPLS (gap 6): `Mpls` compares the length of a packet with the MTU of
the outgoing interface, fragments an IPv4 datagram with the DF bit clear, each fragment with the
label stack, and reports one with the DF bit through the ICMP module that its new parameter
`icmpModule` names. `Ppp` still sends a frame longer than its `mtu`. What follows describes the
model before the repair.

`Mpls` sends a labeled packet of any length, and `Ppp` sends a frame longer than its `mtu`
parameter; the parameter only tells IPv4 the MTU of the interface. IPv4 fragments for the MTU of
the first link, before the ingress adds a label; after that no layer looks at a length (gap 6).

### PPP has no control protocols

The PPP model has no LCP and no network control protocol. It sends a frame of any protocol from
the first moment, so no MPLS Control Protocol precedes the labeled packets (gap 7). The PPP
Protocol of a labeled packet is right, 0281 hex.

### A label of -1 marks native IPv4

`Mpls` treats a packet with the label `(uint32_t)-1` as a native IPv4 packet of RSVP or the TED,
pops the entry and passes the packet up
([Mpls.cc:226-234](../../../../../src/inet/networklayer/mpls/Mpls.cc#L226)). The field has 20
bits, so the serializer writes 1048575, and a packet read back from the wire no longer has the
marker. The tests switch the RSVP hellos off and never see such a packet.

## Scenario quirks

### Switch the RSVP hellos off

`RsvpMplsRouter` has an RSVP module whose `helloInterval` and `helloTimeout` have no default, and
Cmdenv stops with "The simulation attempted to prompt for user input". Set
`*.R*.rsvp.helloInterval = 0s` and `*.R*.rsvp.helloTimeout = 0s`; the tests bind every label by
hand, and no signaling runs.

### The interface names come from the gate order

The LIB names interfaces, so the order of the `pppg++` connections decides the names: on the
path, R1 has `ppp0` towards A and `ppp1` towards R2, and R2 and R3 have `ppp0` upstream and `ppp1`
downstream. In the Ethernet variant, the Ethernet interface of R1 and R2 is `eth0`, and the PPP
interface of R2 towards R3 becomes `ppp0`.

### The same datagram on two links is the one with the same IPv4 Identification

The TTL checks compare one datagram on two links. IPv4 gives each datagram of a sender its own
Identification, and no LSR changes it, so the helper records the TTL by that value. The
UDP port is not enough, because every datagram of a flow has it.

### The MTU of a PPP link is a parameter of each end

`*.R2.ppp[1].ppp.mtu = 500B` and `*.R3.ppp[0].ppp.mtu = 500B` give L3 its MTU in both directions,
as the check says. The ICMP message of `Rfc3032TooBigIcmp` expects 496, the MTU minus one entry.

## Tooling quirks

The first three quirks below hold for every protocol, and belong in
[`ipv4/notes.md`](../ipv4/notes.md#tooling-quirks). That section changes on the unmerged IGMP and
MLD branch of wave 1, so they stay here until the branches of the wave merge.

### A test ends at its first failure, so a rule that can fail gets a test of its own

A failing step, a forbidden event of a guard, and a count that goes too high all decide the whole
test at once (`ProtocolTester::offerToGuards`, `tests/protocol/lib/ProtocolTester.cc:723-746`).
Where the model fails more than one rule of a check, only the first gets a verdict, and the ledger
must write "not reached" for the others. The TTL checks therefore have seven tests, and the check
of the DF bit two.

### Order the steps by the time of their events

A guard runs from the start, and a step waits for the step before it. In the first run of
`Rfc3032TooBigFragments`, the step of the whole datagram of flow 2000 came after the step of flow
2001 on L2; the datagram of flow 2000 had already crossed L3, the step waited for the next one,
and the guard of the too-big packet failed first. RFC3032-FRAG-9 had no verdict. Put first the
step whose event comes first.

### A value reaches the report only through an assertion

`EventPattern` swallows an exception of a filter and treats it as "no match"
(`tests/protocol/lib/EventPattern.cc:192-201`), and a forbidden event reports the guard, not the
packet. A rule whose value matters is an `assertEvent` on a `.once` step, whose `require` throws a
sentence with the value; the tester prints it as "the predicate raised: ...".

### Reading a label stack entry from its octets

[MplsChecks.h](../../../../../tests/protocol/mpls/MplsChecks.h) serializes each `MplsHeader` chunk
with `Chunk::serialize` and decodes the 4 octets, so the checks read the wire, not the fields of
the chunk. `stackBetweenLinkAndIpv4` checks the order of the chunks, and `payloadLength` gives
the PPP Information field. Name a constant with a prefix of its own: `ETHERTYPE_MPLS_UNICAST`
clashes with a member of the `EtherType` enum of INET.

### The exploration dump

`ZzExplore.test` and `explore.sh` in `audit/mpls-level2/` of `inet-master` (outside git) run the
path with LIB entries from the command line, and dump every frame on the links with its PPP
Protocol and the octets of its first entry. They showed the TTL 0, the IPv4 TTL 32 at B, the PPP
Protocol 0281 hex, the octets `00 06 41 00` of the label 100, the refusal of the label 0, the
label 3 on the wire, and the ARP stop on Ethernet before any test read them. Never commit it.
`gen-mpls-tests.py` holds the mockups, the bindings, the step builders and the 21 tests with the
gap paragraphs of the failing ones; `gen-mpls-ledger.py` and `gen-mpls-conformance.py` write the
ledger and the matrix. The copies of the two for the repairs of 2026-09-25, with the verdicts of
their run, are in `audit/mpls-level2-fixes/`.

## Follow-ups, in the order I would do them

1. **Done on 2026-09-25: the labels have a TTL, and the LSP its hops** (gaps 1, 2 and 3). The nine
   TTL tests pass.
2. **Done on 2026-09-25: an MPLS router lives on an Ethernet link** (gap 4).
   `Rfc3032EthernetEncapsulation` passes.
3. **Done on 2026-09-25: the reserved labels** (gap 5). `Rfc3032ExplicitNull` and
   `Rfc3032ImplicitNull` pass, and their declarations are gone.
4. **Done on 2026-09-25: the MTU check and fragmentation** (gap 6). The three `Rfc3032TooBig*`
   tests pass, and their declarations are gone.
5. **The MPLS Control Protocol** (gap 7): it needs LCP in the PPP model first, and IPCP beside it;
   the plan is `plan/pending/ppp-lcp-and-mplscp.md`. Then remove the declaration of
   `Rfc3032PppMplscp`.
6. **A next hop in the NHLFE** (RFC 3031 §3.10), filled by RSVP-TE, LDP and the XML of the LIB, and
   resolved by ARP, so that a labeled frame on an Ethernet link goes to its neighbor only.
7. **Level 3**: a label without a binding (the model discards it), a pop of an unlabeled packet,
   the ICMP message of RFC 3032 §2.3, a second NHLFE for one label, and wrong MPLS Control
   Protocol packets.
8. **The 2 owed statements**: a check of the Router Alert label, which `MplsPacket.msg:16` names,
   with a rule for the local software from the document of a protocol that uses it; see
   [the coverage debt](coverage.md#the-coverage-debt-the-checks-this-pass-owes).
9. **Correct the documentation**: name RFC 3031 and RFC 3032 in `Mpls.ned`, say that the model
   labels IPv4 only, and take out Frame Relay and ATM, which INET does not have.
