# MPLS — run results and model analysis

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../../protocol/mpls/checks.md), [features.md](../../protocol/mpls/features.md), [coverage.md](coverage.md)

Step 7 artifact of the standards test workflow for the MPLS level 2 pass. This is the first
document of the pass that may name the simulation model and reference code, and it does both:
the verdicts of the run, the class of every failure, and where the model implements, or fails
to implement, each checked behavior.

## Run record

- Date: 2026-09-24 21:55 +0200
- INET: branch `topic/standards-tests-mpls-level2`, commit `daac3593fb`, tree clean
- Trees: src `5c4f41c600`, tests/protocol `de3a0cb0f0`
- OMNeT++: 6.4.0
- Build: debug, built from this commit
- Compiler: Ubuntu clang version 23.0.0 (++20260325083105+68994554ea12-1~exp1~20260325203127.404)
- Platform: Ubuntu 26.04.1 LTS, Linux 7.0.0-34-generic x86_64
- Command: `inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/mpls$'`

21 tests: 5 PASS, 16 FAIL, 6 of them declared expected.

## Verdicts

One row per test. The check sections are in [`checks/`](../../protocol/mpls/checks); the
statements are those of the `Checks:` line of each section. A test ends at its first failure, so
where two rules of one check could both fail, each rule has a test of its own: the TTL at each
LSR has three tests, the TTL after the last label four, and the too-big datagram with the DF bit
two.

| Test | Check | Verdict | Class |
| --- | --- | --- | --- |
| `Rfc3032LabelStackEntry` | [The label stack entry](../../protocol/mpls/checks/encoding.md#the-label-stack-entry) | PASS | — |
| `Rfc3032TwoLabelStack` | [A label stack of two entries](../../protocol/mpls/checks/encoding.md#a-label-stack-of-two-entries) | PASS | — |
| `Rfc3031LabelSwitching` | [Label switching along an LSP](../../protocol/mpls/checks/forwarding.md#label-switching-along-an-lsp) | PASS | — |
| `Rfc3031PenultimateHopPopping` | [Penultimate hop popping](../../protocol/mpls/checks/forwarding.md#penultimate-hop-popping) | PASS | — |
| `Rfc3032ExplicitNull` | [The IPv4 Explicit NULL label](../../protocol/mpls/checks/reserved-labels.md#the-ipv4-explicit-null-label) | FAIL at initialization, declared expected | unimplemented feature, [gap 5](#gap-5-unimplemented-feature--no-reserved-label-values) |
| `Rfc3032ImplicitNull` | [The Implicit NULL label](../../protocol/mpls/checks/reserved-labels.md#the-implicit-null-label) | FAIL at observation 2, declared expected | unimplemented feature, [gap 5](#gap-5-unimplemented-feature--no-reserved-label-values) |
| `Rfc3032FirstLabelTtl` | [The TTL of the first label](../../protocol/mpls/checks/ttl.md#the-ttl-of-the-first-label) | FAIL at observation 2 | defect, [gap 1](#gap-1-defect--the-ttl-field-of-every-label-stack-entry-is-0) |
| `Rfc3032TtlAtEachLsr` | [The TTL at each LSR](../../protocol/mpls/checks/ttl.md#the-ttl-at-each-lsr) | FAIL at observation 2 | defect, [gap 1](#gap-1-defect--the-ttl-field-of-every-label-stack-entry-is-0) |
| `Rfc3032TtlWithPush` | [The TTL at each LSR](../../protocol/mpls/checks/ttl.md#the-ttl-at-each-lsr) | FAIL at observation 3 | defect, [gap 1](#gap-1-defect--the-ttl-field-of-every-label-stack-entry-is-0) |
| `Rfc3443TtlAfterTwoPops` | [The TTL at each LSR](../../protocol/mpls/checks/ttl.md#the-ttl-at-each-lsr) | FAIL at observation 4 | defect, [gap 2](#gap-2-defect--the-lsp-does-not-count-its-hops-in-the-ipv4-ttl) |
| `Rfc3032TtlAfterPop` | [The TTL after the last label](../../protocol/mpls/checks/ttl.md#the-ttl-after-the-last-label) | FAIL at observation 2 | defect, [gap 2](#gap-2-defect--the-lsp-does-not-count-its-hops-in-the-ipv4-ttl) |
| `Rfc3443TtlAfterPenultimatePop` | [The TTL after the last label](../../protocol/mpls/checks/ttl.md#the-ttl-after-the-last-label) | FAIL at observation 3 | defect, [gap 2](#gap-2-defect--the-lsp-does-not-count-its-hops-in-the-ipv4-ttl) |
| `Rfc3031TtlAcrossLsp` | [The TTL after the last label](../../protocol/mpls/checks/ttl.md#the-ttl-after-the-last-label) | FAIL at observation 4: TTL 32 | defect, [gap 2](#gap-2-defect--the-lsp-does-not-count-its-hops-in-the-ipv4-ttl) |
| `Rfc3031TtlAcrossLspPenultimate` | [The TTL after the last label](../../protocol/mpls/checks/ttl.md#the-ttl-after-the-last-label) | FAIL at observation 4: TTL 31 | defect, [gap 2](#gap-2-defect--the-lsp-does-not-count-its-hops-in-the-ipv4-ttl) |
| `Rfc3032TtlExpiry` | [A TTL that reaches zero](../../protocol/mpls/checks/ttl.md#a-ttl-that-reaches-zero) | FAIL at observation 3 | defect, [gap 3](#gap-3-defect--a-labeled-packet-whose-ttl-reaches-zero-goes-on) |
| `Rfc3032TooBigFragments` | [A labeled datagram too big for the next link](../../protocol/mpls/checks/fragmentation.md#a-labeled-datagram-too-big-for-the-next-link) | FAIL at observation 3, declared expected | unimplemented feature, [gap 6](#gap-6-unimplemented-feature--no-mtu-check-and-no-fragmentation-of-a-labeled-datagram) |
| `Rfc3032TooBigDontFragment` | [A too-big labeled datagram that may not be fragmented](../../protocol/mpls/checks/fragmentation.md#a-too-big-labeled-datagram-that-may-not-be-fragmented) | FAIL at observation 2, declared expected | unimplemented feature, [gap 6](#gap-6-unimplemented-feature--no-mtu-check-and-no-fragmentation-of-a-labeled-datagram) |
| `Rfc3032TooBigIcmp` | [A too-big labeled datagram that may not be fragmented](../../protocol/mpls/checks/fragmentation.md#a-too-big-labeled-datagram-that-may-not-be-fragmented) | FAIL at observation 3, declared expected | unimplemented feature, [gap 6](#gap-6-unimplemented-feature--no-mtu-check-and-no-fragmentation-of-a-labeled-datagram) |
| `Rfc3032PppEncapsulation` | [Labeled packets on a PPP link](../../protocol/mpls/checks/links.md#labeled-packets-on-a-ppp-link) | PASS | — |
| `Rfc3032PppMplscp` | [The MPLS Control Protocol](../../protocol/mpls/checks/links.md#the-mpls-control-protocol) | FAIL at observation 2, declared expected | unimplemented feature, [gap 7](#gap-7-unimplemented-feature--no-mpls-control-protocol) |
| `Rfc3032EthernetEncapsulation` | [Labeled packets on an Ethernet link](../../protocol/mpls/checks/links.md#labeled-packets-on-an-ethernet-link) | FAIL: the run stops at 0.008 s | defect, [gap 4](#gap-4-defect--an-mpls-router-on-an-ethernet-link-stops-the-run) |

## The class of every failure

Ten failures are of the class **defect** of
[the guide](../../../guide/derive-tests-from-a-standard.md#the-class-of-a-failure-and-when-to-declare-it-expected):
the model has code or a field for the behavior, and gets it wrong. Nine are TTL failures: the
`MplsHeader` has a TTL field, which the serializer writes to the wire, and a field that is
present with a placeholder value is a claim (see
[conformance.md, part 1](conformance.md#part-1--the-claims)). The tenth is the Ethernet link,
which the documentation of `Mpls` names. None of them is declared expected, because no
limitation blocks a repair; each gap names the code that a repair would change.

Six failures are **unimplemented features**: the reserved label values (two tests), the MTU
check and the fragmentation of a labeled datagram (three tests), and the MPLS Control Protocol.
The model has no code for them and names none of them; each test declares
`%# expected-result: FAIL` and names the reason in its description.

No failure of this run is a test error or a misread of the specification. The first run found
one order problem, repaired before this run: in `Rfc3032TooBigFragments` the guard of
observation 3 failed before the step of observation 2 matched, so RFC3032-FRAG-9 had no verdict.
See [`notes.md`](notes.md).

## The model gaps

### Gap 1 (defect) — the TTL field of every label stack entry is 0

- **Tests**: `Rfc3032FirstLabelTtl`, observation 2: the entry has TTL 0 above an IPv4 header
  with TTL 32. `Rfc3032TtlAtEachLsr`, observation 2, and `Rfc3032TtlWithPush`, observation 3:
  the entry on L3 has TTL 0, the same as on L2.
- **Statements**: RFC3032-TTL-9, RFC3031-TTL-4, RFC3443-PUSH-1, MOD-1; RFC3032-TTL-1, TTL-2,
  TTL-5, RFC3031-TTL-5, RFC3443-TERM-2, ITTL-2, OTTL-2; RFC3032-TTL-6, RFC3031-TTL-2.
- **The code**: `Mpls::pushLabel` and `Mpls::swapLabel` build a new `MplsHeader`, set its label
  and its S bit, and never set its TTL
  ([`Mpls.cc:139-153`](../../../../../src/inet/networklayer/mpls/Mpls.cc)), so the field keeps
  its default 0 ([`MplsPacket.msg:19`](../../../../../src/inet/networklayer/mpls/MplsPacket.msg)).
  At the ingress, `Mpls::tryLabelAndForwardIpv4Datagram` reads the IPv4 header and marks it
  unused ([`Mpls.cc:98-99`](../../../../../src/inet/networklayer/mpls/Mpls.cc)). A swap drops
  the incoming TTL with the old header.
- **Scope**: every labeled packet that the model sends. `MplsPacketSerializer` writes the 0 to
  the wire ([`MplsPacketSerializer.cc:23`](../../../../../src/inet/networklayer/mpls/MplsPacketSerializer.cc)),
  so a capture shows it, and a real LSR that gets such a packet discards it (RFC3032-TTL-3).

### Gap 2 (defect) — the LSP does not count its hops in the IPv4 TTL

- **Tests**: `Rfc3032TtlAfterPop`, observation 2, and `Rfc3443TtlAfterTwoPops`, observation 4:
  on L4 the IPv4 TTL is 32, where the entry on L3 had 0. `Rfc3443TtlAfterPenultimatePop`,
  observation 3: on L3 the IPv4 TTL is 32 after the pop. `Rfc3031TtlAcrossLsp`, observation 4:
  B gets TTL 32, where three routers give 29. `Rfc3031TtlAcrossLspPenultimate`, observation 4:
  TTL 31, because only R3 forwards the datagram by IPv4.
- **Statements**: RFC3032-TTL-10, RFC3031-TTL-6, RFC3443-OTTL-1, ITTL-4, MOD-1; RFC3443-ITTL-5;
  RFC3443-OTTL-5; RFC3031-TTL-1.
- **The code**: `Mpls::popLabel` removes the header and, at the bottom of the stack, sets the
  protocol to IPv4; it never writes the IPv4 TTL
  ([`Mpls.cc:156-163`](../../../../../src/inet/networklayer/mpls/Mpls.cc)). The ingress labels a
  datagram as it arrives from the link, before IPv4 forwards it
  ([`Mpls.cc:202-208`](../../../../../src/inet/networklayer/mpls/Mpls.cc)), and the egress sends
  the popped datagram to the link, not to IPv4
  ([`Mpls.cc:269-283`](../../../../../src/inet/networklayer/mpls/Mpls.cc)). So neither end
  decrements the IPv4 TTL, and the LSRs between them cannot, because their TTL is the label TTL
  of gap 1.
- **Scope**: every datagram that crosses an LSP arrives with the TTL it had at the ingress, as if
  the LSP had no routers. A repair of gap 1 alone does not repair this gap: the pop must also copy
  the TTL, and the ingress and the egress must count as hops.

### Gap 3 (defect) — a labeled packet whose TTL reaches zero goes on

- **Test**: `Rfc3032TtlExpiry`, observation 3: the datagrams of flow 2001, sent with TTL 2, cross
  L3; B delivers them.
- **Statements**: RFC3032-TTL-3, RFC3443-TERM-3.
- **The code**: `Mpls::processMplsPacketFromL2` looks up the label, applies the operations and
  sends the packet, with no look at a TTL
  ([`Mpls.cc:217-285`](../../../../../src/inet/networklayer/mpls/Mpls.cc)).
- **Scope**: a forwarding loop of labeled packets never ends, and no TTL limits the reach of a
  labeled datagram. A repair of gap 1 alone gives each entry a TTL that nothing checks.

### Gap 4 (defect) — an MPLS router on an Ethernet link stops the run

- **Test**: `Rfc3032EthernetEncapsulation`: at 0.008 s the ARP request of R1 reaches the MPLS
  module of R2, and the run stops with "Unknown message received". No step gets a verdict.
- **Statements**: RFC3032-LAN-1, LAN-2, LAN-3.
- **The code**: `Mpls::processPacketFromL2` throws for every packet that is neither MPLS nor IPv4,
  with a note "FIXME remove throw above"
  ([`Mpls.cc:210-214`](../../../../../src/inet/networklayer/mpls/Mpls.cc)). The documentation of
  the module names Ethernet as a link layer
  ([`Mpls.ned:39-40`](../../../../../src/inet/networklayer/mpls/Mpls.ned)).
- **Scope**: every MPLS router with an Ethernet interface, from the first ARP packet. A second
  fault waits behind this one: in the exploration with `GlobalArp`, which sends no ARP packet, the
  labeled packet reached the Ethernet MAC without an Ethernet header, and "Cannot convert chunk
  from type inet::MplsHeader to type inet::EthernetMacHeader" stopped the run. The MPLS models of
  INET run on PPP links only.

### Gap 5 (unimplemented feature) — no reserved label values

- **Tests**: `Rfc3032ExplicitNull`: the LIB refuses the swap to the label 0, and the run stops at
  initialization. `Rfc3032ImplicitNull`, observation 2: R2 sends the label 3 on L3, and R3, which
  has no binding for it, discards the datagrams.
- **Statements**: RFC3032-ENC-11; RFC3032-ENC-15, RFC3031-NULL-1.
- **The code**: none. `Mpls.cc` gives no label value a meaning of its own, and
  `LibTable::readTableFromXML` asserts that each label value of the configuration is above 0
  ([`LibTable.cc:132, 142, 152`](../../../../../src/inet/networklayer/mpls/LibTable.cc)).
- **Scope**: an egress cannot ask for the Explicit NULL label, which keeps the Traffic Class field
  to the last hop, and the Implicit NULL label goes onto the wire as an ordinary label. The
  assertion of the LIB exists in a debug build only: a release build defines `NDEBUG`, which
  removes `ASSERT`, so there R2 sends the label 0 and R3, with no binding, discards the datagrams.
  The test fails in both builds, at a different place. Penultimate
  hop popping works through an explicit pop in the LIB of the penultimate LSR.

### Gap 6 (unimplemented feature) — no MTU check and no fragmentation of a labeled datagram

- **Tests**: `Rfc3032TooBigFragments`, observation 3: R2 sends a labeled packet of 1032 octets onto
  L3, whose frames carry at most 500 octets. `Rfc3032TooBigDontFragment`, observation 2: the same
  with the DF bit set. `Rfc3032TooBigIcmp`, observation 3: A gets no ICMP message.
- **Statements**: RFC3032-FRAG-8, FRAG-11, FRAG-13, FRAG-14, FRAG-15. RFC3032-FRAG-12 has no
  verdict, because no fragment exists; RFC3032-FRAG-9 passes.
- **The code**: none. `Mpls::processMplsPacketFromL2` sends a labeled packet of any length
  ([`Mpls.cc:217-285`](../../../../../src/inet/networklayer/mpls/Mpls.cc)), and `Ppp` sends it
  although it is longer than the `mtu` parameter of the interface.
- **Scope**: every LSP over a link with a smaller MTU. IPv4 fragments a datagram for the MTU of
  the first link only, before the ingress; the 4 octets of each label and every link after the
  ingress are outside its view.

### Gap 7 (unimplemented feature) — no MPLS Control Protocol

- **Test**: `Rfc3032PppMplscp`, observation 2: R1 sends the first labeled packet onto L2 with no
  frame of the PPP Protocol 8281 hex before it.
- **Statements**: RFC3032-PPP-1, PPP-5, PPP-11.
- **The code**: none. The PPP model has no LCP and no network control protocol; it sends a frame
  of any protocol from the start ([`Ppp.cc`](../../../../../src/inet/linklayer/ppp/Ppp.cc)).
- **Scope**: every PPP link. The labeled packets themselves have the right PPP Protocol, 0281 hex
  (`Rfc3032PppEncapsulation` passes).

## What the model does well

- **The label stack entry** has the layout of RFC 3032 §2.1 on the wire: 20 bits of label, 3 bits
  of Traffic Class, the S bit and 8 bits of TTL, in that order
  ([`MplsPacketSerializer.cc:17-24`](../../../../../src/inet/networklayer/mpls/MplsPacketSerializer.cc)).
  The stack sits between the link header and the IPv4 header, top entry first, and only the
  bottom entry has the S bit ([`Mpls.cc:139-145`](../../../../../src/inet/networklayer/mpls/Mpls.cc)).
- **Push, swap and pop**, and their combinations in one LIB entry, do what RFC 3031 §3.10 says:
  a swap and a push give a stack of two entries, and two pops at the egress give the IPv4
  datagram ([`Mpls.cc:165-194`](../../../../../src/inet/networklayer/mpls/Mpls.cc)).
- **The ILM** is the LIB, keyed by the incoming interface and label; **the FTN** is the classifier
  of the ingress, which maps a destination to a LIB entry
  ([`LibTable.cc`](../../../../../src/inet/networklayer/mpls/LibTable.cc)).
- **Penultimate hop popping** works: the penultimate LSR pops the stack, and the egress, with no
  binding, forwards the datagram by IPv4.
- **The PPP encapsulation** carries a labeled packet with the PPP Protocol 0281 hex and nothing
  but the label stack and the datagram in the Information field.
- **A label without a binding** is discarded
  ([`Mpls.cc:240-246`](../../../../../src/inet/networklayer/mpls/Mpls.cc)), as RFC 3031 §3.18
  asks; the standards map puts that check at level 3, so no test of this pass reaches it.

## Other findings

- **The model names no standard.** Part 1 of [`conformance.md`](conformance.md#part-1--the-claims)
  found no RFC number anywhere in the MPLS model or its chapter of the User's Guide. The
  documentation of `Mpls` also names Frame Relay and ATM as link layers
  ([`Mpls.ned:39-40`](../../../../../src/inet/networklayer/mpls/Mpls.ned)), which INET does not
  have, so that part of the claim cannot be tested.
- **The model labels IPv4 only.** A packet of another protocol from the network layer goes to the
  link unlabeled, "only the Ipv4 protocol supported yet"
  ([`Mpls.cc:69-73`](../../../../../src/inet/networklayer/mpls/Mpls.cc)), and the pop of the
  bottom label always gives IPv4 ([`Mpls.cc:160-162`](../../../../../src/inet/networklayer/mpls/Mpls.cc)).
  The User's Guide names plain IPv4 as the example of an unlabeled packet, and nothing claims
  IPv6, so the checks for a labeled IPv6 datagram are not owed. The same throw as gap 4 stops an
  MPLS router that gets an IPv6 packet from a link.
- **The Router Alert label is claimed and does nothing.** The field comment of `MplsHeader` says
  that the label 1 "represents the router alert label"
  ([`MplsPacket.msg:16`](../../../../../src/inet/networklayer/mpls/MplsPacket.msg)), and no code
  treats the label 1 differently. The check needs local software in the LSR and a rule for it,
  which RFC 3032 does not give; the two statements are owed (see
  [the coverage debt](coverage.md#the-coverage-debt-the-checks-this-pass-owes)).
- **The Traffic Class field is always 0.** The field comment names QoS and ECN
  ([`MplsPacket.msg:17`](../../../../../src/inet/networklayer/mpls/MplsPacket.msg)), but no code
  sets or copies the field. RFC 5462 §3 only permits these uses, so no check of this pass fails
  on it.
- **The label -1 marks native IPv4.** `Mpls` treats a packet with the label `(uint32_t)-1` as a
  native IPv4 packet of RSVP or the TED, and passes it up
  ([`Mpls.cc:226-234`](../../../../../src/inet/networklayer/mpls/Mpls.cc)). The label field has 20
  bits, so the serializer writes 1048575, and a packet read back from the wire has a label that
  no longer matches the marker.

## What the next pass owes

In the order I would do it:

1. **The TTL**: a repair of gaps 1, 2 and 3 together, and a run of the nine TTL tests.
2. **Level 3**, the packets that no binding covers: an incoming label without a binding (the
   model discards it), a pop of an unlabeled packet, and the ICMP messages of RFC 3032 §2.3.
3. **The two owed statements** of the Router Alert label, with a rule for the local software
   from the document of a protocol that uses the label.
4. **After a repair of gap 4**, run `Rfc3032EthernetEncapsulation` again, and then look at the
   Ethernet header that the second fault leaves out.
