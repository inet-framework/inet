# IPv6 checks — run results and model analysis (pass 2, level 3)

> **Kind:** report · **Status:** snapshot 2026-09-10 · **Seal:** none · **Owns:** — · **Stands on:** [rfc8200/catalog.md](../../standard/rfc8200/catalog.md), [rfc4443/catalog.md](../../standard/rfc4443/catalog.md), [rfc8504/catalog.md](../../standard/rfc8504/catalog.md), [checks.md](../../protocol/ipv6/checks.md)

Step 7 artifact of the standards test workflow. This is the first document of the IPv6
workflow that may reference code. It supersedes the pass 1 report; the pass 1 verdicts are
repeated in the table below, because every test ran again on this tree.

- Date: 2026-09-10 15:18 +0200
- INET: branch `master`, commit `0868c36c88`, tree clean
- OMNeT++: 6.4.0
- Build: debug, built from this commit
- Compiler: Ubuntu clang version 23.0.0
- Platform: Ubuntu 26.04.1 LTS, Linux 7.0.0-31-generic x86_64
- Command, after the `setenv` scripts of OMNeT++ and INET:

  ```sh
  cd tests/protocol/lib && MODE=debug ./build.sh
  inet_run_protocol_tests -p inet -w ipv6
  ```
- Earlier passes ran on other trees. The pass log of [`coverage.md`](coverage.md#pass-log)
  names them. Every verdict below is the verdict of the run above, and it repeats the
  verdict of the earlier pass.

  The whole suite runs in about 2.5 seconds. It used to need nine minutes, and one test
  accounted for all of it: the atomic fragment, where the model stops with an assertion of
  the standard library. The runtime installs a handler for SIGABRT that prints a stack trace
  annotated with source lines, and resolving 25 frames against a debug build of the whole of
  INET costs those minutes. The test now restores the default handler for that signal, so the
  process ends at once. Nothing about the check changed, and neither did its verdict: the
  model still aborts, the assertion message is still printed, and the missing verdict line is
  still the failure. See the note in
  [`notes.md`](notes.md#a-crash-need-not-cost-minutes).

## Verdicts

| Test | Checks | Verdict |
| --- | --- | --- |
| Rfc8200DatagramDelivery.test | RFC8200-HDR-1, HDR-2, HDR-3, HDR-4, DLV-1, HL-1, CKSUM-1 | PASS |
| Rfc8200HopLimitExpiry.test | RFC8200-HL-2, RFC4443-TE-1, RFC4443-ERR-2 | PASS |
| Rfc8200HopLimitOneAtDestination.test | RFC8200-HL-3; covers HL-1 | PASS |
| Rfc8200HopLimitZeroAtDestination.test | RFC8200-HL-3, the exact case | PASS |
| Rfc8200SourceFragmentation.test | RFC8200-FRAG-3, FRAG-4, FRAG-5 (structure), FRAG-6, EXT-1, REASM-1, REASM-2, MTU-4; covers HDR-3, RFC8504-NR-2, NR-3, NR-4, NR-8 (the sender halves) | PASS |
| Rfc8200FragmentPayloadLength.test | RFC8200-FRAG-4, FRAG-5 (payload length), HDR-2 | **FAIL (unexpected)** — defect |
| Rfc8200FragmentIdentification.test | RFC8200-FRAG-2; notes RFC8504-NR-5 | PASS |
| Rfc8200AtomicFragment.test | RFC8504-NR-4 (receiver; governs RFC8200-REASM-6) | **FAIL (unexpected)** — defect, a C++ assertion |
| Rfc8200OverlappingFragments.test | RFC8504-NR-3 (receiver; governs RFC8200-REASM-5) | **FAIL (unexpected)** — defect, a runtime assertion |
| Rfc8200ShortFragment.test | RFC8200-REASM-4 | PASS |
| Rfc8200OversizedFragmentOffset.test | RFC8200-REASM-8 | PASS |
| Rfc4443PacketTooBig.test | RFC8200-FRAG-1, RFC4443-PTB-1, RFC4443-ERR-2; covers RFC8504-NR-11 | PASS |
| Rfc4443PacketTooBigMtu.test | RFC4443-PTB-2 | **FAIL (unexpected)** — defect |
| Rfc8200LinkMtuPacket.test | RFC8200-MTU-2 | PASS |
| Rfc4443HostErrorReport.test | RFC4443-DU-4, ERR-1, ERR-2, SRC-1 | PASS |
| Rfc4443ReportSourceAddress.test | RFC4443-SRC-2 | PASS |
| Rfc4443NoErrorAboutError.test | RFC4443-MPR-4 | PASS |
| Rfc4443NoErrorForMulticast.test | RFC4443-MPR-6 | PASS |
| Rfc4443NoErrorForLinkMulticast.test | RFC4443-MPR-7 | **FAIL (unexpected)** — defect |
| Rfc4443NoErrorForLinkBroadcast.test | RFC4443-MPR-8 | **FAIL (unexpected)** — defect |
| Rfc4443NoErrorForUnspecifiedSource.test | RFC4443-MPR-9 | PASS |
| Rfc4443ErrorForUnknownProtocol.test | RFC4443-MPR-3, MPR-4 | **FAIL (unexpected)** — defect, a runtime error |
| Rfc8200ZeroUdpChecksum.test | RFC8200-CKSUM-1 (the discard) | PASS |
| Rfc8200UnrecognizedNextHeader.test | RFC8504-NR-6 (governs RFC8200-EXT-3) | PASS |
| Rfc8200UnassignedNextHeader.test | RFC8504-NR-6, for an unassigned value | PASS |
| Rfc4443UnknownErrorType.test | RFC4443-MPR-4 (the silence); notes MPR-1 | PASS |
| Rfc4443UnknownInformationalType.test | RFC4443-MPR-2 | **FAIL (unexpected)** — defect, a runtime error |

Summary: 27 tests in the suite, 19 PASS, **0 FAIL (expected), 8 FAIL (unexpected)**, so the suite
reports FAIL.

## Which failures are declared, and which are not

Reviewed against
[the third principle of the guide](../../../guide/derive-tests-from-a-standard.md#principle-a-claimed-feature-gets-a-test):
a failure is declared expected only where the model does **not** claim the behavior, and a claim is
code. **All eight failures of this suite are defects, so none is declared.** IPv6 in this tree has
real fragmentation, reassembly and ICMPv6 code, and every one of these eight is that code doing the
wrong thing.

| Test | The claim, in the model |
| --- | --- |
| `Rfc4443NoErrorForLinkBroadcast.test`, `…LinkMulticast.test` | `Icmpv6::validateDatagramPromptingError` suppresses for four conditions, one citing RFC 4443 §2.4(e). The mechanism is there and the link-layer condition is missing from it. |
| `Rfc4443UnknownInformationalType.test` | The type switch has a `default:` branch for a type it does not know, and that branch throws. |
| `Rfc4443ErrorForUnknownProtocol.test` | The report is built and sent; the crash is in the path that delivers it. |
| `Rfc4443PacketTooBigMtu.test` | `Icmpv6::createPacketTooBigMsg` takes an `mtu` parameter (Icmpv6.h:55) and the caller hands it a literal 0 (Icmpv6.cc:273). The `// TODO implement MTU support.` above it is a bare "to do", which says the behavior is wanted and unfinished — a claim — and not a reason why it is unsupported. |
| `Rfc8200FragmentPayloadLength.test` | `Ipv6::fragmentAndSend` builds every fragment and sets its header; the payload length it writes is the copied one. |
| `Rfc8200AtomicFragment.test` | The fragment buffer runs and uses an iterator it has already erased. A memory bug in live code. |
| `Rfc8200OverlappingFragments.test` | The reassembly buffer runs, completes a datagram from the overlapping set, and then trips the model's own assertion in `Ipv6::decapsulate`. |

**Four of the eight stop the simulation**, and that is the loudest finding of this suite: a
crafted but lawful input meets a `default:` branch, an erased iterator or an assertion, and the
run ends. A stop is never a declarable failure.

Every FAIL is a
model gap declared with `%# expected-result: FAIL`; each test keeps the faithful assertion
and fails at the step, or in the way, its description predicts. Four of the eight failures
stop the simulation: the model's answer to three crafted inputs and to one report is an
assertion or a runtime error, not a discard. No specification misread was found.

## Deviations between the English observations and the test steps

Each one is also stated in the `%description` of its test. None weakens an expectation.
The seven deviations of pass 1 stand and are not repeated here.

1. **The arrival at the interface is the anchor for a host's silent discard.** The model's
   internet layer emits no signal when it hands an ICMPv6 message to its ICMPv6 module
   ([Ipv6.cc:887-888](../../../../../src/inet/networklayer/ipv6/Ipv6.cc#L887-L888)), and a
   host that discards silently leaves no record. The unknown-type checks and the
   suppression checks therefore anchor their absence watch on the arrival of the crafted
   packet at the host's interface, which precedes the decision; the check document says so.
2. **Two relay rules for one scenario.** The unrecognized-next-header checks need a rewrite
   on the way to host B and a drop on the way back, because host A stops the simulation
   when host B's report reaches it; that stop is a check of its own. One tap carries one
   rule, so the relay is two taps in series.
3. **A protocol number is read by type.** A filter on `ipv6.protocolId == 253` never
   matches, because the dissector has no handler for the number and the filter treats the
   exception as a non-match; the checks read the base header directly. The same for the
   payload length of a fragment, as in pass 1.
4. **The relay shortens a frame by its data, not by a header field.** The frame a tap holds
   begins with a physical-layer header in front of the link-layer header, so the helper
   that shortens the first fragment measures the payload from the IPv6 header's own
   position and writes the true length into the field.
5. **The unknown error type is not asserted to be passed up** (RFC4443-MPR-1); the check
   asserts the silence about it (MPR-4), and the ledger records the handoff as internal.

## Scenario settings beyond the templates

- `**.udp.checksumMode = "computed"` in the zero-checksum test, so that the receiver has a
  checksum to verify; in the model's default mode the field is a placeholder.
- `*.hostA.eth[0].mac.mtu = 1280B` in the four crafted-fragment tests, as in pass 1.
- `*.hostA.app[0].multicastInterface = "eth0"` in the multicast test: the model needs the
  outgoing interface named for a link-local multicast destination.
- A `PingApp` on host A as the vehicle of the unknown-informational-type test.

## Failure history during authoring

Every initial failure was a test error. None was a model gap, and none changed what a
passing test proves.

1. **The shortened fragment carried the wrong payload length** (1244 for 1236): the helper
   measured from the frame's front, and the frame at a tap begins with an 8-octet
   physical-layer header. The helper now measures from the IPv6 header. The model had done
   the right thing all along: it discarded the 1228-octet piece and sent Parameter Problem
   code 0.
2. **`ipv6.protocolId == 253` never matched** at host B's interface, although the trace
   showed the header; the number maps to an INET-internal protocol name that has no
   dissector, and the filter's exception became a non-match. A typed reader replaced it.
3. **The Parameter Problem report crashed host A** when it arrived, before the absence
   watch could run. That is a gap of its own; the check was split so that host B's correct
   behavior and host A's crash are two verdicts (deviation 2).
4. **`hostX.ipv6.ipv6 packetSentToUpper` never fired for an ICMPv6 message**, because the
   internet layer hands ICMPv6 messages over without a signal (deviation 1). The step was
   removed and the arrival became the anchor. With that repair the unknown-error-type test
   passes: the model passes a type-100 error up and stays silent.

## Model analysis — where INET implements the checked behavior

All line numbers are of this tree. The pass 1 analysis of the normal path stands; this
section covers the level 3 behaviors.

**Passes.**

- **Hop limit 0 at the destination (RFC8200-HL-3):** local delivery precedes the hop limit
  check ([Ipv6.cc:455-462](../../../../../src/inet/networklayer/ipv6/Ipv6.cc#L455-L462)).
  Observed: delivery of a packet with hop limit 0.
- **Short fragment (REASM-4) and oversized offset (REASM-8):**
  [Ipv6FragBuf.cc:83-100](../../../../../src/inet/networklayer/ipv6/Ipv6FragBuf.cc#L83-L100)
  discards a non-final fragment whose measured length is not a multiple of 8, and a fragment
  whose offset plus length exceeds 65,535, and sends Parameter Problem code 0 for both.
  Observed: code 0 at host A, nothing at host B's UDP.
- **Host error report (DU-4, ERR-1, ERR-2, SRC-1) and the router's source (SRC-2):**
  UDP asks for Destination Unreachable code 4
  ([Udp.cc:1121-1128](../../../../../src/inet/transportlayer/udp/Udp.cc#L1121-L1128));
  ICMPv6 quotes the invoking packet up to 1280 octets
  ([Icmpv6.cc:282-290](../../../../../src/inet/networklayer/icmpv6/Icmpv6.cc#L282-L290));
  the internet layer selects the source address of the report by the outgoing interface.
  Observed: type 1 code 4 from host B's address to host A's, payload length 156; the
  router's Time Exceeded from the router's link 1 address.
- **Suppression, three of five cases (MPR-4, MPR-6, MPR-9):**
  `Icmpv6::validateDatagramPromptingError`
  ([Icmpv6.cc:371-399](../../../../../src/inet/networklayer/icmpv6/Icmpv6.cc#L371-L399))
  declines a report for a multicast destination, an unspecified or multicast source, and an
  ICMPv6 message of a type below 128. Observed: no Time Exceeded for the expiring report,
  no Destination Unreachable for the multicast or the unspecified-source datagram.
- **Unknown error type (MPR-1 wire half, MPR-4):**
  [Icmpv6.cc:103-118](../../../../../src/inet/networklayer/icmpv6/Icmpv6.cc#L103-L118)
  treats every type below 128 as an error and passes it up as an indication. Observed:
  nothing sent back, the simulation running on.
- **Zero UDP checksum (CKSUM-1, the discard):**
  [Udp.cc:1011-1015](../../../../../src/inet/transportlayer/udp/Udp.cc#L1011-L1015): "on udp
  under Ipv6, the checksum 0000 is invalid". Observed: the discard record, nothing to the
  program.
- **Unrecognized and unassigned next header (RFC8504-NR-6, RFC8200-EXT-3):**
  [Ipv6.cc:862-868](../../../../../src/inet/networklayer/ipv6/Ipv6.cc#L862-L868): a protocol
  with no connected handler gets `ICMPv6_PARAMETER_PROBLEM` with
  `UNRECOGNIZED_NEXT_HDR_TYPE`. Observed for 253 and for 200: code 1 toward host A, nothing
  at UDP.

**Gaps.** Seven statements failed on this tree, two of them since pass 1. Each is a
behavior the model lacks or a stop where a discard was due; none is touched by a test.

1. **Packet Too Big carries MTU 0 (RFC4443-PTB-2)** — pass 1,
   [Icmpv6.cc:271-273](../../../../../src/inet/networklayer/icmpv6/Icmpv6.cc#L271-L273).
2. **Every fragment carries the original payload length (RFC8200-FRAG-4, FRAG-5, HDR-2)**
   — pass 1, [Ipv6.cc:1108-1112](../../../../../src/inet/networklayer/ipv6/Ipv6.cc#L1108-L1112).
   It reappears below as the reason two crafted-fragment cases end in an assertion.
3. **An atomic fragment stops the simulation (RFC8504-NR-4, RFC8200-REASM-6).**
   `Ipv6FragBuf::addFragment` looks the buffer up before it creates it
   ([Ipv6FragBuf.cc:45-53](../../../../../src/inet/networklayer/ipv6/Ipv6FragBuf.cc#L45-L53))
   and, when the very first fragment completes the datagram, erases that stale end
   iterator ([Ipv6FragBuf.cc:168](../../../../../src/inet/networklayer/ipv6/Ipv6FragBuf.cc#L168)).
   Observed: `Assertion '__position != end()' failed` in the C++ library, and a nine-minute
   stack trace. Any single-fragment datagram triggers it, not only a crafted one.
4. **Overlapping fragments are merged, then the merged datagram trips an assertion
   (RFC8504-NR-3, RFC8200-REASM-5).** `ChunkBuffer::replace` merges the overlapping regions
   ([ChunkBuffer.cc:115-147](../../../../../src/inet/common/packet/ChunkBuffer.cc#L115-L147)),
   the buffer completes a 1452-octet datagram instead of discarding the set, and
   `Ipv6::decapsulate` then asserts `payloadLength <= packet->getDataLength()`
   ([Ipv6.cc:900-902](../../../../../src/inet/networklayer/ipv6/Ipv6.cc#L900-L902)), because
   the reassembled header still says 1460 (gap 2). RFC 5722, folded into RFC 8200 and
   restated by RFC 8504, requires a silent discard; the model's reassembly code quotes
   RFC 2460, which predates the rule.
5. **An unknown informational ICMPv6 type stops the simulation (RFC4443-MPR-2).**
   [Icmpv6.cc:190-191](../../../../../src/inet/networklayer/icmpv6/Icmpv6.cc#L190-L191):
   `default: throw cRuntimeError("Unknown ICMPv6 message type %d received", type)`.
   Observed: `Unknown ICMPv6 message type 200 received` at host B. The same switch handles
   every type below 128 as an error first, which is why an unknown error type passes.
6. **A report about a packet of an unknown upper-layer protocol stops the source
   (RFC4443-MPR-3, MPR-4).** Host A's ICMPv6 selects the transport protocol from the quoted
   header and sends the indication to a dispatcher that has no such protocol. Observed:
   `handleMessage(): Unknown protocol: protocolId = 78, protocolName = nexthopforwarding` in
   `hostA.ipv6.lp`. RFC 4443 §2.4 (d) drops such a report; the model stops.
7. **A report is sent about a link-layer multicast or broadcast (RFC4443-MPR-7, MPR-8).**
   `validateDatagramPromptingError` reads the IPv6 addresses only and never consults the
   `MacAddressInd` of the frame. Observed: Destination Unreachable left host B in both
   cases, and the absence watch fired. The same gap as in IPv4.

## Model observations the checks did not claim

1. **Fragment identifications are a counter** (0, 1, ...), from `curFragmentId++`
   ([Ipv6.cc:1091](../../../../../src/inet/networklayer/ipv6/Ipv6.cc#L1091)). RFC 8504 §5.1
   says a node should avoid predictable values; the model declines the should. Recorded as
   `declined` in the ledger.
2. **Protocol numbers 253 and 254 are INET-internal names** (`IP_PROT_NEXT_HOP_FORWARDING`,
   `IP_PROT_ECHO`,
   [IpProtocolId.msg:55-56](../../../../../src/inet/networklayer/common/IpProtocolId.msg#L55-L56)),
   although RFC 3692 reserves them for experiments. It is why the dispatcher of gap 6 names
   "nexthopforwarding" for a packet that carried 253.
3. **ICMPv6 dispatches by the type field's range, not by the message class:** a message
   object of one class with the type of another is processed by the type. The relay
   rewrites therefore exercise the model as a wire would.
4. **The model's own receiver never sees the payload-length gap**, because it measures
   fragments; a crafted fragment is the first thing that makes the wrong field matter
   (gap 4).

## Feature support

The verdicts above feed the support column and the achieved level of the coverage ledger,
[`coverage.md`](coverage.md#feature-support), by the rules of step 7 and of the levels.

## Sharpening candidates for the next pass

- **Module tests for the handoffs** of IPV6-F-ERROR-DELIVERY (RFC4443-MPR-1, UL-1, UL-2,
  UL-3): what a protocol test cannot see on the wire.
- **The pointer field** of the Parameter Problem messages (RFC8200-EXT-3 names offset 6;
  REASM-4 names the payload length field; REASM-8 the offset field); the model marks it
  "TODO set pointer".
- **RFC4443-MPR-5**, a redirect in flight, once a neighbor discovery pass gives the mockup a
  redirect; **RFC4443-DU-P**, a point-to-point link; **RFC8200-REASM-7**, a first fragment
  without its upper-layer header.
- **The multicast exceptions** of RFC4443-MPR-6: a Packet Too Big for a multicast packet
  needs a router that forwards multicast.
- **Level 4:** the reassembly timer (REASM-3), the rate limit (MPR-10), congestion (DU-C),
  and path MTU discovery once the MTU field is filled.
