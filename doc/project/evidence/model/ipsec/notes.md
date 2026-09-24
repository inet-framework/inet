# IPsec — quirks and follow-ups

> **Kind:** reference · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [results.md](results.md), [coverage.md](coverage.md)

What this document is for. The workflow's artifacts each answer one question: what the standard
says, what a run showed, what the model claims. This one holds what a person learned while doing
the work and would otherwise have to learn again — the behaviors of the model that surprised the
author, the traps in the scenarios and the tooling that cost a debugging cycle, and the ordered
list of what to do next.

The tooling quirks that apply to every protocol are in
[`ipv4/notes.md`](../ipv4/notes.md#tooling-quirks). What follows is what IPsec added, in one
pass: level 2 on 2026-09-24, over RFC 4301, RFC 4302 and RFC 4303. The gaps themselves are in
[`results.md`](results.md#the-model-gaps).

## Model quirks

### The configuration is the interface, and it rejects what it does not know

The SPD of a host is an XML element of the parameter `spdConfig`, and `IPsec::initSecurityDBs`
checks every tag against a fixed list
([IPsec.cc:156-242](../../../../../src/inet/networklayer/ipsec/IPsec.cc#L156)). An element for the
mode of an SA, its lifetime, its DSCP values or dummy packets stops the run at initialization. A
test of a missing option cannot even name the option, so its description says what it would set
(tunnel mode, SA lifetime, DSCP selection, dummy packets).

### One protection for each PROTECT entry, so AH and ESP never meet

Each `SecurityPolicy` has one `Protection`, AH or ESP, and every `SecurityAssociation` inside it
takes the rule of the entry ([IPsec.cc:176-233](../../../../../src/inet/networklayer/ipsec/IPsec.cc#L176)).
The code that applies ESP and then AH to one packet
([IPsec.cc:734-753](../../../../../src/inet/networklayer/ipsec/IPsec.cc#L734)) is therefore
unreachable, and so is the documentation that promises it, "If Protection is AH_ESP"
([IPsec.ned:149](../../../../../src/inet/networklayer/ipsec/IPsec.ned#L149)): the enum has no
`AH_ESP` (gap 7). The ingress path has a second half of the problem: the ESP branch tests the
protocol of the packet as it arrived, so a packet with AH around ESP reaches the SPD lookup of
unprotected traffic after AH ([IPsec.cc:803-867](../../../../../src/inet/networklayer/ipsec/IPsec.cc#L803)).
No run reaches it yet.

### The receiver never checks an ICV

AH and ESP accept a packet without a verification: "TODO check icv"
([IPsec.cc:832](../../../../../src/inet/networklayer/ipsec/IPsec.cc#L832)) and "TODO calculate icv
bytes length from SPI and use espHeader.icvBytes for verify it"
([IPsec.cc:885](../../../../../src/inet/networklayer/ipsec/IPsec.cc#L885)). A pass that depends on
the ICV has no weight: `Rfc4302AhAcrossRouter` passes because nothing can reject the packet, not
because the mutable fields are zero in the ICV. The ledger marks such passes "with no weight".
The TODOs are claims, so the level 3 checks of a failed ICV will be defects.

### The counters start at zero

`SecurityAssociation::getAndIncSeqNum` returns `seqNum++` from 0
([SecurityAssociation.h:42, 72](../../../../../src/inet/networklayer/ipsec/SecurityAssociation.h#L42)),
so the first packet of every SA carries 0 (gap 1). The increments are right, and each SA has its
own counter.

### The AH header is 12 octets, and its ICV trails the packet

`IPsec::ahProtect` builds the fixed part of the AH header, leaves the Payload Length at 0, wraps
the payload in an `EncryptedChunk`, and appends the ICV at the end
([IPsec.cc:561-586](../../../../../src/inet/networklayer/ipsec/IPsec.cc#L561)) (gaps 2 and 3). The
lengths are right, so the overhead is right; the layout is not. The AH dissector reads the Payload
Length as the length of the payload in octets
([IpSecProtocolDissector.cc:28](../../../../../src/inet/networklayer/ipsec/IpSecProtocolDissector.cc#L28)),
which works only because the field is 0: repair the header and the dissector together. AH also
carries its payload as an `EncryptedChunk`, although AH does not encrypt.

### The ESP padding has the right length and the wrong octets

`IPsec::espProtect` pads to the cipher block and to 4 octets, and the Pad Length is right; the
padding is a `ByteCountChunk`, whose octets are 63, the default of the chunk
([IPsec.cc:535-536](../../../../../src/inet/networklayer/ipsec/IPsec.cc#L535)) (gap 4). With NULL
encryption the padding is in clear, which is how the check reads it.

### The SA decides alone on ingress, and nothing decides without an SA on egress

On ingress the SPI selects the SA, the protocol must match, and the packet is accepted; the
selectors of the SA are never compared with the packet (gap 5). On egress a PROTECT entry applies
the SAs that match, and with none the packet leaves in clear (gap 6). Both are holes in the
access control that IPsec exists for, and both are small repairs.

### Two integrity algorithms have wrong ICV lengths

`HMAC_SHA1` gives 160 bits where HMAC-SHA-1-96 (RFC 2404) gives 96, and `HMAC_SHA2_384_192` gives
384 bits where RFC 4868 gives 192 ([IPsec.cc:682-698](../../../../../src/inet/networklayer/ipsec/IPsec.cc#L682)).
The algorithm documents are level 5, so no gap names them; the checks use HMAC-SHA-256-128, whose
16 octets are right. A check that names HMAC-SHA-1-96 fails on the model's `HMAC_SHA1` for a
reason that is not the check's.

### No path MTU anywhere

A router sends ICMPv6 Packet Too Big, and `Icmpv6` passes it up as an indication for the protocol
of the quoted packet ([Icmpv6.cc:124-128](../../../../../src/inet/networklayer/icmpv6/Icmpv6.cc#L124)).
Neither IPsec nor the IPv6 layer keeps the MTU, so a protected IPv6 flow on a smaller path never
arrives (gap 12). On IPv4 without DF the routers fragment, and the protected datagrams arrive.

### What the model does right about fragments

IPsec runs in the post-routing hook, before fragmentation, and in the local-in hook, after
reassembly. Only the first fragment holds the AH or ESP header, the ICV arrives in the last one,
and a router may fragment a protected IPv4 packet on its way.

### The documentation of the module is stale in three places

`IPsec.ned` names a protection `AH_ESP` that does not exist (line 149), an action `DROP` where the
configuration says `DISCARD` (lines 142 and 165), and an element `IcvNumBits` in its example that
the configuration rejects (line 125).

## Scenario quirks

### Give IPsec to the hosts only

`**.ipv4.hasIpsec = true` also gives the router an IPsec module, which then asks for its
`spdConfig`, and Cmdenv stops with "The simulation attempted to prompt for user input". The run
ends without a verdict line, and the runner counts a FAIL with no reason. Use
`*.host*.ipv4.hasIpsec = true`; the reason is in `test.err`.

### Every SPD ends with a DISCARD entry, and an IPv6 SPD begins with ICMPv6

The checks give each SPD a final entry that discards everything else, in both directions, as
RFC4301-SPD-17 advises. On IPv6 that entry would discard the unicast Neighbor Advertisements, so
each IPv6 SPD begins with a BYPASS entry for protocol 58 in both directions. Multicast bypasses
IPsec in the model, so the Neighbor Solicitations need no entry.

### The same SPI at both ends, and the direction in the entry

A manually keyed SA appears twice: as an `OUT` entry at the sender and as an `IN` entry at the
receiver, with the same SPI. The receiver finds it by SPI and direction only
([SecurityAssociationDatabase.cc:42-48](../../../../../src/inet/networklayer/ipsec/SecurityAssociationDatabase.cc#L42)).
The selectors of an `IN` entry are written from the receiver's side: local port is its own port.

### The MTU of an Ethernet link is a parameter of its MAC

`*.router.eth[1].mtu` applies nothing; `*.router.eth[1].mac.mtu = 1000B` does. The exploration
showed the router sending 1500-octet fragments until the key had the right path.

### A replay is a crafted packet, even when a configuration can make one

A sender with two outbound SAs that share one SPI would make two packets with the same Sequence
Number on one inbound SA, a duplicate that the anti-replay window must drop. The standard gives
each SA its own SPI, so that configuration is a crafted stimulus, and the anti-replay checks stay
with level 3.

### A refusal at configuration is a result

The both-NULL SA stops the model at initialization, with "Cannot set authenticationAlg=NONE if
espMode=INTEGRITY" on stderr. The check accepts that as the refusal that RFC 4301 asks for, and
the test expects it with `%exitcode: 1` and `%contains-regex: stderr`; the first version looked
at stdout and failed.

## Tooling quirks

### An assertion makes a pattern match once

A pattern with an assertion matches only the first event that its filter picks
(`ProtocolTester::patternMatches`, `tests/protocol/lib/ProtocolTester.cc:546-550`). A counting
guard with an assertion, `exactlyTimes(5, ...)` that also checks the length of each datagram,
counts one and fails. Count with a plain filter, and check each event with a `never` guard of its
own.

### A delivered datagram is seen at UDP, not at the application

The tester subscribes to a fixed set of packet signals, and the `packetReceived` signal of an
application is not among them. A datagram is "delivered" when `<host>.udp` emits
`packetSentToUpper`, and its port is in the `L4PortInd` tag (`deliveredPort` of
[IpsecChecks.h](../../../../../tests/protocol/ipsec/IpsecChecks.h)). `on()` matches a path prefix,
so `hostB.app[*]` matches nothing.

### Reading AH and ESP out of a frame

[IpsecChecks.h](../../../../../tests/protocol/ipsec/IpsecChecks.h) flattens the chunks of a frame
(`wireChunks`) and opens the `EncryptedChunk` after an AH header, as the dissector does, so the
order is the order of the wire. For ESP it opens the payload "with the keys of the SA"
(`espPlain`) and reads the trailer, the padding, the IV length and the ICV. Fragments carry a
`SliceChunk` of the protected packet, and an IPv6 fragment an `Ipv6FragmentHeader` chunk of its
own; `isFragment`, `fragmentProtocolOf` and `ipsecHeaderFirst` read them.

### The exploration dump and the generators

`mkexplore.py` in `audit/ipsec-level2/` of `inet-master` (outside git) writes `ZzExplore.test`, a
dump of every frame at the named MACs with all its chunks, slices and encrypted chunks opened. It
showed the zero sequence numbers, the Payload Length, the trailing AH ICV, the 63 padding octets,
the 20-octet `HMAC_SHA1` ICV, the fragment layout and the ignored Packet Too Big before any test
read them. Never commit it. `gen-ipsec-tests.py` holds the mockups, the SPD builder and the step
builders, and `gen-ipsec-specs.py` the 31 tests with the gap paragraphs of the failing ones;
`gen-ipsec-ledger.py` and `gen-ipsec-conformance.py` write the ledger and the matrix.

## Follow-ups, in the order I would do them

1. **Start the sequence counter at 1** (gap 1): `getAndIncSeqNum` increments first. Then run
   `Rfc4303SequenceNumbers` and `Rfc4302SequenceNumbers` again.
2. **Fill the ESP padding with 1, 2, 3** (gap 4): a `BytesChunk` in place of the `ByteCountChunk`.
   Then run `Rfc4303EspPadding`.
3. **Discard a PROTECT packet without an SA** (gap 6), and **check the selectors of the SA on
   ingress** (gap 5). Then run `Rfc4301ProtectWithoutSa` and `Rfc4301InboundSelectorCheck`.
4. **Lay out the AH header as RFC 4302 does** (gaps 2 and 3): the Payload Length in words minus 2,
   the ICV inside the header with its IPv6 padding, and the dissector in the same change. Then run
   the four `Rfc4302Ah*` format tests.
5. **Let one flow have AH and ESP** (gap 7): a configuration that reaches the existing code, and
   an ingress path that goes on to ESP after AH. Then run `Rfc4301AhAndEsp`.
6. **Level 3**: an ICV that fails on AH and ESP (the TODOs of `IPsec.cc:832, 885`), a fragment
   offered to AH or ESP, a non-zero Reserved field, wrong padding, a replayed packet. The passes
   of `Rfc4302AhAcrossRouter` get their weight then.
7. **The missing features, each with its declaration**: dummy packets (gap 11), the path MTU in
   the IPv6 layer and in IPsec (gap 12), the SA lifetime (gap 9), the choice of an SA by DSCP
   (gap 10), and tunnel mode (gap 8). The declaration of each test goes with the implementation.
8. **Correct the documentation and the algorithm lengths**: the three stale places of `IPsec.ned`,
   and the ICV lengths of `HMAC_SHA1` and `HMAC_SHA2_384_192`.
9. **The 6 owed statements**: a check with an IPv6 Hop-by-Hop or Destination Options header; see
   [the coverage debt](coverage.md#the-coverage-debt-the-checks-this-pass-owes).
