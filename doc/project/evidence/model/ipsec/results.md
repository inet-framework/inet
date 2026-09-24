# IPsec — run results and model analysis

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../../protocol/ipsec/checks.md), [features.md](../../protocol/ipsec/features.md), [coverage.md](coverage.md)

Step 7 artifact of the standards test workflow for the IPsec level 2 pass. This is the first
document of the pass that may name the simulation model and reference code, and it does both:
the verdicts of the run, the class of every failure, and where the model implements, or fails
to implement, each checked behavior.

## Run record

- Date: 2026-09-24 19:35 +0200
- INET: branch `topic/standards-tests-ipsec-level2`, commit `829ba07bae`, tree clean
- Trees: src `5c4f41c600`, tests/protocol `3ffda5ff0f`
- OMNeT++: 6.4.0
- Build: debug, built from this commit
- Compiler: Ubuntu clang version 23.0.0 (++20260325083105+68994554ea12-1~exp1~20260325203127.404)
- Platform: Ubuntu 26.04.1 LTS, Linux 7.0.0-34-generic x86_64
- Command: `inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/ipsec$'`

31 tests: 15 PASS, 16 FAIL, 6 of them declared expected.

## Verdicts

One row per test. The check sections are in [`checks/`](../../protocol/ipsec/checks); the
statements are those of the `Checks:` line of each section. Eight checks have two tests: four
run once on IPv4 and once on IPv6, the sequence numbers run once for ESP and once for AH, the
tunnel mode once for each protocol, and the place of the AH ICV has a test of its own, so that the
Payload Length does not hide it.

| Test | Check | Verdict | Class |
| --- | --- | --- | --- |
| `Rfc4301OutboundDispositions` | [Outbound dispositions](../../protocol/ipsec/checks/policy.md#outbound-dispositions) | PASS | — |
| `Rfc4301InboundDispositions` | [Inbound dispositions](../../protocol/ipsec/checks/policy.md#inbound-dispositions) | PASS | — |
| `Rfc4301FirstMatchDecides` | [The first matching entry decides](../../protocol/ipsec/checks/policy.md#the-first-matching-entry-decides) | PASS | — |
| `Rfc4301Selectors` | [Selectors](../../protocol/ipsec/checks/policy.md#selectors) | PASS | — |
| `Rfc4301SaPerDirection` | [One SA for each direction](../../protocol/ipsec/checks/security-associations.md#one-sa-for-each-direction) | PASS | — |
| `Rfc4301InboundSaLookup` | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | PASS | — |
| `Rfc4301InboundSelectorCheck` | [Inbound selector check](../../protocol/ipsec/checks/security-associations.md#inbound-selector-check) | FAIL at observation 3 | defect, [gap 5](#gap-5-defect--no-selector-check-after-ah-or-esp-processing) |
| `Rfc4301ProtectWithoutSa` | [A PROTECT entry without an SA](../../protocol/ipsec/checks/security-associations.md#a-protect-entry-without-an-sa) | FAIL at observation 2 | defect, [gap 6](#gap-6-defect--a-protect-entry-without-an-sa-sends-the-packet-in-clear) |
| `Rfc4301SaLifetime` | [The lifetime of an SA](../../protocol/ipsec/checks/security-associations.md#the-lifetime-of-an-sa) | FAIL at observation 3, declared expected | unimplemented feature, [gap 9](#gap-9-unimplemented-feature--no-sa-lifetime) |
| `Rfc4301ParallelSas` | [Parallel SAs for classes of traffic](../../protocol/ipsec/checks/security-associations.md#parallel-sas-for-classes-of-traffic) | FAIL at observation 2, declared expected | unimplemented feature, [gap 10](#gap-10-unimplemented-feature--no-choice-of-an-sa-by-dscp) |
| `Rfc4303SequenceNumbers` | [Sequence numbers of an SA](../../protocol/ipsec/checks/sequence-numbers.md#sequence-numbers-of-an-sa) | FAIL at observation 4 | defect, [gap 1](#gap-1-defect--the-first-packet-of-an-sa-carries-sequence-number-0) |
| `Rfc4302SequenceNumbers` | [Sequence numbers of an SA](../../protocol/ipsec/checks/sequence-numbers.md#sequence-numbers-of-an-sa) | FAIL at observation 4 | defect, [gap 1](#gap-1-defect--the-first-packet-of-an-sa-carries-sequence-number-0) |
| `Rfc4303EspFormat` | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | PASS | — |
| `Rfc4303EspFormatIpv6` | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | PASS | — |
| `Rfc4303EspPadding` | [ESP padding](../../protocol/ipsec/checks/esp.md#esp-padding) | FAIL at observation 5 | defect, [gap 4](#gap-4-defect--the-esp-padding-octets-are-63-not-1-2-3) |
| `Rfc4303EspServices` | [ESP services](../../protocol/ipsec/checks/esp.md#esp-services) | PASS | — |
| `Rfc4303EspBothNull` | [An ESP SA without encryption and without integrity](../../protocol/ipsec/checks/esp.md#an-esp-sa-without-encryption-and-without-integrity) | PASS (the configuration is refused) | — |
| `Rfc4303TfcPadding` | [Traffic flow confidentiality padding](../../protocol/ipsec/checks/esp.md#traffic-flow-confidentiality-padding) | PASS | — |
| `Rfc4303DummyPackets` | [Dummy packets](../../protocol/ipsec/checks/esp.md#dummy-packets) | FAIL at observation 1, declared expected | unimplemented feature, [gap 11](#gap-11-unimplemented-feature--no-dummy-packets) |
| `Rfc4302AhFormat` | [AH format in transport mode](../../protocol/ipsec/checks/ah.md#ah-format-in-transport-mode) | FAIL at observation 6 | defect, [gap 2](#gap-2-defect--the-ah-payload-length-is-always-0) |
| `Rfc4302AhFormatIpv6` | [AH format in transport mode](../../protocol/ipsec/checks/ah.md#ah-format-in-transport-mode) | FAIL at observation 6 | defect, [gap 2](#gap-2-defect--the-ah-payload-length-is-always-0) |
| `Rfc4302AhIcvPosition` | [AH format in transport mode](../../protocol/ipsec/checks/ah.md#ah-format-in-transport-mode) | FAIL at observation 5 | defect, [gap 3](#gap-3-defect--the-ah-icv-is-at-the-end-of-the-packet) |
| `Rfc4302AhIcvPositionIpv6` | [AH format in transport mode](../../protocol/ipsec/checks/ah.md#ah-format-in-transport-mode) | FAIL at observation 5 | defect, [gap 3](#gap-3-defect--the-ah-icv-is-at-the-end-of-the-packet) |
| `Rfc4302AhAcrossRouter` | [AH across a router](../../protocol/ipsec/checks/ah.md#ah-across-a-router) | PASS, with no weight: the receiver never checks the ICV | — |
| `Rfc4302AhAcrossRouterIpv6` | [AH across a router](../../protocol/ipsec/checks/ah.md#ah-across-a-router) | PASS, with no weight: the receiver never checks the ICV | — |
| `Rfc4301TunnelModeEsp` | [Tunnel mode between two hosts](../../protocol/ipsec/checks/modes.md#tunnel-mode-between-two-hosts) | FAIL at observation 3, declared expected | unimplemented feature, [gap 8](#gap-8-unimplemented-feature--no-tunnel-mode) |
| `Rfc4301TunnelModeAh` | [Tunnel mode between two hosts](../../protocol/ipsec/checks/modes.md#tunnel-mode-between-two-hosts) | FAIL at observation 3, declared expected | unimplemented feature, [gap 8](#gap-8-unimplemented-feature--no-tunnel-mode) |
| `Rfc4301AhAndEsp` | [AH and ESP on one packet](../../protocol/ipsec/checks/modes.md#ah-and-esp-on-one-packet) | FAIL at observation 1 | untestable claim, [gap 7](#gap-7-untestable-claim--ah-and-esp-cannot-protect-one-packet) |
| `Rfc4301Fragments` | [Fragments of a protected datagram](../../protocol/ipsec/checks/fragmentation.md#fragments-of-a-protected-datagram) | PASS | — |
| `Rfc4301FragmentsIpv6` | [Fragments of a protected datagram](../../protocol/ipsec/checks/fragmentation.md#fragments-of-a-protected-datagram) | PASS | — |
| `Rfc4301PathMtuIpv6` | [Path MTU of a protected flow](../../protocol/ipsec/checks/fragmentation.md#path-mtu-of-a-protected-flow) | FAIL at observation 2, declared expected | unimplemented feature, [gap 12](#gap-12-unimplemented-feature--no-path-mtu) |

## The class of every failure

Nine failures are of the class **defect** of
[the guide](../../../guide/derive-tests-from-a-standard.md#the-class-of-a-failure-and-when-to-declare-it-expected):
the model has code for the behavior, and the code gets it wrong. None of them is declared
expected, because no limitation blocks a repair; each gap names the code that a repair would
change.

One failure, `Rfc4301AhAndEsp`, is an **untestable claim**: the model has the code for AH and ESP
on one packet, and its documentation names the combination, but its configuration cannot express
it, so the check cannot be built. The test fails and names the missing part; it declares nothing.

Six failures are **unimplemented features**: tunnel mode (two tests), the SA lifetime, the choice
of an SA by DSCP, dummy packets, and the path MTU. The model says so in its own documentation, or
has no code for the behavior at all; each test declares `%# expected-result: FAIL` and names the
reason in its description.

No failure of this run is a test error or a misread of the specification. The first runs had
found three test errors, all repaired before the commit of the tests: a count with an assertion
counted one event, the router of the path had IPsec too, and the refusal of the both-NULL SA
went to stderr, not to stdout. See [`notes.md`](notes.md).

## The model gaps

### Gap 1 (defect) — the first packet of an SA carries Sequence Number 0

- **Tests**: `Rfc4303SequenceNumbers` and `Rfc4302SequenceNumbers`, observation 4. The
  increments and the separate counters of observations 2 and 3 hold.
- **Statements**: RFC4303-SEQ-7, OSEQ-1; RFC4302-SEQ-8, OSEQ-1.
- **The code**: `SecurityAssociation` starts `seqNum` at 0, and `getAndIncSeqNum` returns it
  before the increment
  ([`SecurityAssociation.h:42, 72`](../../../../../src/inet/networklayer/ipsec/SecurityAssociation.h));
  `IPsec::espProtect` and `IPsec::ahProtect` write that value
  ([`IPsec.cc:549, 575`](../../../../../src/inet/networklayer/ipsec/IPsec.cc)). RFC 4302 §3.3.2
  and RFC 4303 §3.3.3 increment the counter first, so the first packet carries 1.
- **Scope**: every SA, AH and ESP. A receiver with anti-replay would see a number that the
  standard never sends; the model has no receiver that checks.

### Gap 2 (defect) — the AH Payload Length is always 0

- **Tests**: `Rfc4302AhFormat` and `Rfc4302AhFormatIpv6`, observation 6.
- **Statement**: RFC4302-LEN-1.
- **The code**: `IPsec::ahProtect` sets the Next Header, the SPI, the Sequence Number and the ICV
  length of the header, and never its Payload Length
  ([`IPsec.cc:574-579`](../../../../../src/inet/networklayer/ipsec/IPsec.cc)). With a 16-octet
  ICV the field is 5 in IPv4 and 6 in IPv6.
- **Scope**: every AH packet. The dissector of AH reads the field as the length of the payload
  in octets, not in words ([`IpSecProtocolDissector.cc:28`](../../../../../src/inet/networklayer/ipsec/IpSecProtocolDissector.cc)),
  so a repair of this gap changes the dissector too.

### Gap 3 (defect) — the AH ICV is at the end of the packet

- **Tests**: `Rfc4302AhIcvPosition` and `Rfc4302AhIcvPositionIpv6`, observation 5.
- **Statements**: RFC4302-FMT-2, ICV-1, ICV-2, ICV-4; in IPv6 also LEN-2.
- **The code**: `IPsec::ahProtect` appends the ICV after the payload
  ([`IPsec.cc:581-583`](../../../../../src/inet/networklayer/ipsec/IPsec.cc)). In RFC 4302 §2 the
  ICV is the last field of the AH header, before the payload. In IPv6 the model also leaves the
  ICV at 16 octets, where the AH header must be a multiple of 8 octets: 20 octets of ICV field.
- **Scope**: every AH packet. The lengths are right, so the overhead that the model exists to
  study is right; the order is not, and a dissector or a capture reads the payload at the wrong
  offset.

### Gap 4 (defect) — the ESP padding octets are 63, not 1, 2, 3

- **Test**: `Rfc4303EspPadding`, observation 5. The Pad Length and the alignment of
  observations 3 and 4 hold.
- **Statement**: RFC4303-PAD-9.
- **The code**: `IPsec::espProtect` computes the Pad Length right, and fills the padding with a
  `ByteCountChunk`, whose octets are the default value 63
  ([`IPsec.cc:535-536`](../../../../../src/inet/networklayer/ipsec/IPsec.cc)). RFC 4303 §2.4 gives
  the octets 1, 2, 3 and so on when the cipher names no contents.
- **Scope**: every ESP packet with padding. A receiver that inspects the padding (RFC4303-PAD-10,
  a should) would reject it.

### Gap 5 (defect) — no selector check after AH or ESP processing

- **Test**: `Rfc4301InboundSelectorCheck`, observation 3: B delivers the datagrams of flow 3000,
  which SA 101 does not cover.
- **Statements**: RFC4301-IN-32, IN-33, SAD-6.
- **The code**: `IPsec::processIngressPacket` finds the SA of an AH or ESP packet by its SPI,
  checks that the SA has the same protocol, removes the header and accepts the packet
  ([`IPsec.cc:803-927`](../../../../../src/inet/networklayer/ipsec/IPsec.cc)). It never matches the
  packet against the selectors of the SA, which the SAD entry holds.
- **Scope**: every inbound SA. A sender, or an attacker with the keys of one SA, can reach every
  port of the receiver through it.

### Gap 6 (defect) — a PROTECT entry without an SA sends the packet in clear

- **Test**: `Rfc4301ProtectWithoutSa`, observation 2: flow 2000 leaves A as a plain UDP datagram.
- **Statement**: RFC4301-OUT-10.
- **The code**: `IPsec::protectDatagram` applies every SA of the entry that matches the packet
  ([`IPsec.cc:732-758`](../../../../../src/inet/networklayer/ipsec/IPsec.cc)). With no SA, no
  protection is applied, and the packet is accepted as it is
  ([`IPsec.cc:760-778`](../../../../../src/inet/networklayer/ipsec/IPsec.cc)). RFC 4301 §5.1
  discards a packet whose SA cannot be created.
- **Scope**: every PROTECT entry whose SAs do not match the packet: an entry with no SA, and an
  entry whose SAs narrow its selectors. Traffic that the policy says to protect leaves in clear.

### Gap 7 (untestable claim) — AH and ESP cannot protect one packet

- **Test**: `Rfc4301AhAndEsp`, observation 1: no packet with AH around ESP.
- **Statements**: RFC4301-SA-4, RFC4303-LOC-4.
- **The code**: `IPsec::protectDatagram` has the code for both headers on one packet, ESP first
  ([`IPsec.cc:734-753`](../../../../../src/inet/networklayer/ipsec/IPsec.cc)), and the module
  documentation names the combination, "If Protection is AH_ESP, both headers are added"
  ([`IPsec.ned:149`](../../../../../src/inet/networklayer/ipsec/IPsec.ned)). But the enum
  `Protection` has no `AH_ESP`, each PROTECT entry has one protection, and every SA of the entry
  takes it ([`IPsec.cc:176-233`](../../../../../src/inet/networklayer/ipsec/IPsec.cc)). No
  configuration reaches the code.
- **Scope**: a second half waits behind the first. On ingress, after the AH header the model goes
  to ESP only when the packet arrived as ESP: the ESP branch tests the protocol of the packet
  before AH processing ([`IPsec.cc:803-867`](../../../../../src/inet/networklayer/ipsec/IPsec.cc)),
  so a packet with AH around ESP reaches the SPD lookup of unprotected traffic after AH. This is
  a reading of the code; no run reaches it.

### Gap 8 (unimplemented feature) — no tunnel mode

- **Tests**: `Rfc4301TunnelModeEsp` and `Rfc4301TunnelModeAh`, observation 3: the payload holds
  the UDP datagram, not an inner IPv4 header.
- **Statements**: RFC4301-SA-47, SA-50, TUN-1, TUN-19, TUN-26, RFC4303-LOC-9, LOC-11, OENC-2,
  IICV-15, RFC4302-LOC-9, LOC-11, LOC-13.
- **The code**: none. The documentation says "Transport mode only. Tunnel mode is not supported."
  ([`IPsec.ned:40`](../../../../../src/inet/networklayer/ipsec/IPsec.ned)), and the configuration
  has no element for the mode.
- **Scope**: RFC 4301 §4.1 asks a host for both modes (RFC4301-SA-50) and a security gateway for
  tunnel mode (SA-51), so the model can play neither a complete host nor any gateway.

### Gap 9 (unimplemented feature) — no SA lifetime

- **Test**: `Rfc4301SaLifetime`, observation 3 (the fifth packet of SA 102, at 5 s); observation 2
  would fail at 6 s.
- **Statements**: RFC4301-SADI-10, SADI-11.
- **The code**: none. "SA lifetime is omitted because all SAs are statically configured"
  ([`SecurityAssociation.h:30-35`](../../../../../src/inet/networklayer/ipsec/SecurityAssociation.h)).
- **Scope**: every SA stays in use for the whole run.

### Gap 10 (unimplemented feature) — no choice of an SA by DSCP

- **Test**: `Rfc4301ParallelSas`, observation 2: the packets with DSCP 46 carry SPI 101.
- **Statements**: RFC4301-SA-25, SA-26.
- **The code**: none. "DSCP-based SA selection is not implemented."
  ([`IPsec.ned:43`](../../../../../src/inet/networklayer/ipsec/IPsec.ned)). A PROTECT entry with two
  ESP SAs applies the first that matches to every packet, and logs the second as unused
  ([`IPsec.cc:754-756`](../../../../../src/inet/networklayer/ipsec/IPsec.cc)).
- **Scope**: every set of parallel SAs; the classes of traffic share one SA and one sequence
  counter.

### Gap 11 (unimplemented feature) — no dummy packets

- **Test**: `Rfc4303DummyPackets`, observation 1.
- **Statements**: RFC4303-NH-2, NH-3, NH-5; NH-4 has no verdict, because no dummy packet reaches
  B.
- **The code**: none. `IPsec::espProtect` always writes the protocol of the payload into the
  Next Header of the trailer ([`IPsec.cc:537-540`](../../../../../src/inet/networklayer/ipsec/IPsec.cc)),
  and no code or parameter makes a packet with Next Header 59.
- **Scope**: the traffic flow confidentiality that dummy packets give; the TFC padding of RFC 4303
  §2.7 works.

### Gap 12 (unimplemented feature) — no path MTU

- **Test**: `Rfc4301PathMtuIpv6`, observation 2: after the Packet Too Big message with MTU 1280,
  A goes on to send ESP packets of 1410 octets.
- **Statements**: RFC4301-OUT-21, RFC4302-FRAG-10, RFC4303-FRAG-9.
- **The code**: none, in IPsec or in the IPv6 layer. `Icmpv6` turns the Packet Too Big message into
  an indication for the protocol that the quoted packet names
  ([`Icmpv6.cc:124-128`](../../../../../src/inet/networklayer/icmpv6/Icmpv6.cc)), and no code keeps
  the MTU for the destination or for the SA.
- **Scope**: every protected flow on a path with a smaller MTU. On IPv6 the flow stops; on IPv4
  without DF, routers fragment, and the `Rfc4301Fragments` test shows that B still delivers.

## What the model does well

- **The SPD** is an ordered list, searched first to last, with the three actions
  ([`SecurityPolicyDatabase.cc:43`](../../../../../src/inet/networklayer/ipsec/SecurityPolicyDatabase.cc),
  [`IPsec.cc:406-444`](../../../../../src/inet/networklayer/ipsec/IPsec.cc)). A packet that no entry
  matches is discarded, outbound and inbound; an unprotected packet that matches an inbound PROTECT
  entry is discarded too.
- **The selectors** match address ranges, the next layer protocol, port ranges and, for IPv4, the
  ICMP type and code ranges, and a selector that an entry does not name is ANY.
- **The inbound SA lookup** finds the SA by its SPI and direction, and discards a packet with no
  SA or with an SA of the other protocol ([`IPsec.cc:803-927`](../../../../../src/inet/networklayer/ipsec/IPsec.cc)).
  Each direction has its own SA.
- **The ESP packet** has the right order and lengths: SPI, Sequence Number, the IV of the cipher,
  the plaintext with its padding to the cipher block and to 4 octets, the Pad Length, the Next
  Header, and the ICV of the integrity algorithm, in IPv4 and IPv6. The three services —
  confidentiality only, integrity only with NULL encryption, and a combined algorithm — give the
  right fields, and the receiver restores every datagram.
- **An ESP SA with NULL encryption and NULL integrity is refused** when it is configured, with a
  message that names the reason ([`IPsec.cc:192-200`](../../../../../src/inet/networklayer/ipsec/IPsec.cc)).
- **TFC padding** is added only where the payload carries its own length, a UDP datagram, and the
  UDP Length field and the Pad Length stay right
  ([`IPsec.cc:385, 493`](../../../../../src/inet/networklayer/ipsec/IPsec.cc)).
- **Fragmentation**: the model applies AH and ESP in the post-routing hook, before
  fragmentation, and in the local-in hook, after reassembly; only the first fragment holds the
  AH or ESP header, and a packet that a router fragments on its way is delivered.

## Other findings

- **The receiver never checks the ICV.** AH and ESP accept a packet without a verification of
  its ICV: "TODO check icv" ([`IPsec.cc:832`](../../../../../src/inet/networklayer/ipsec/IPsec.cc))
  and "TODO calculate icv bytes length from SPI and use espHeader.icvBytes for verify it"
  ([`IPsec.cc:885`](../../../../../src/inet/networklayer/ipsec/IPsec.cc)). A bare TODO is a claim, so
  the level 3 checks of a failed ICV will be defects. The pass of `Rfc4302AhAcrossRouter` has no
  weight for this reason: the receiver cannot reject a packet whose TTL the router changed.
- **Two integrity algorithms have wrong ICV lengths.** `HMAC_SHA1` gives 160 bits, where
  HMAC-SHA-1-96 of RFC 2404 gives 96, and `HMAC_SHA2_384_192` gives 384 bits, where RFC 4868
  gives 192 ([`IPsec.cc:682-698`](../../../../../src/inet/networklayer/ipsec/IPsec.cc)). The
  algorithm documents are level 5 of the standards map; the checks use HMAC-SHA-256-128, whose
  16 octets are right.
- **AH carries its payload as an `EncryptedChunk`**, although AH does not encrypt
  ([`IPsec.cc:568-572`](../../../../../src/inet/networklayer/ipsec/IPsec.cc)). The dissector opens it,
  so no check sees a difference; a model user who reads the chunks does.
- **The documentation of the module is stale in three places**: it names a protection `AH_ESP`
  that does not exist (line 149), an action `DROP` where the configuration says `DISCARD` (lines
  142 and 165), and an element `IcvNumBits` in its example that the configuration rejects (line
  125) ([`IPsec.ned`](../../../../../src/inet/networklayer/ipsec/IPsec.ned)).
- **Multicast traffic bypasses IPsec**, as the documentation says
  ([`IPsec.ned:41`](../../../../../src/inet/networklayer/ipsec/IPsec.ned),
  [`IPsec.cc:412-417`](../../../../../src/inet/networklayer/ipsec/IPsec.cc)). Multicast is optional
  in the standard, so the checks do not ask for it.
- **The model has no anti-replay service**, as the documentation says
  ([`IPsec.ned:42`](../../../../../src/inet/networklayer/ipsec/IPsec.ned)). Its SAs are keyed by
  hand, and RFC 4302 §5 and RFC 4303 §5 advise against anti-replay on such an SA
  (RFC4302-CONF-5, RFC4303-CONF-5), so for the model as it is, the absence agrees with the
  advice. The anti-replay feature stays mandatory for an implementation with key management.

## What the next pass owes

In the order I would do it:

1. **Level 3**, the crafted packets: an ICV that fails, on AH and on ESP (the TODOs above make
   these defects), a fragment offered to AH or ESP, a non-zero Reserved field, wrong padding, and a
   replayed packet. See [the coverage debt](coverage.md#the-coverage-debt-the-checks-this-pass-owes).
2. **After a repair of gap 7**, run `Rfc4301AhAndEsp` again, and then read the ingress path of AH
   around ESP, which no run reaches today.
3. **The level 2 statements this pass left**: the ICMP report of an outbound discard, nested SAs,
   a change of the SPD at run time, a host with more than one SPD, IPv4 options and IPv6 extension
   headers, and multicast. The closing list of
   [`checks.md`](../../protocol/ipsec/checks.md#statements-this-pass-wrote-no-check-for) names what
   each needs.
