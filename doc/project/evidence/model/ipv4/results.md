# IPv4 checks — run results and model analysis (pass 3, level 3)

> **Kind:** report · **Status:** snapshot 2026-09-10 · **Seal:** none · **Owns:** — · **Stands on:** [rfc791/catalog.md](../../standard/rfc791/catalog.md), [rfc792/catalog.md](../../standard/rfc792/catalog.md), [rfc1122/catalog.md](../../standard/rfc1122/catalog.md), [rfc6864/catalog.md](../../standard/rfc6864/catalog.md), [checks.md](../../protocol/ipv4/checks.md)

Step 7 artifact of the standards test workflow. This is the first document of the IPv4
workflow that may reference code. It supersedes the pass 2 report; the pass 2 verdicts are
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
  inet_run_protocol_tests -p inet -w ipv4
  ```
- Earlier passes ran on other trees. The pass log of [`coverage.md`](coverage.md#pass-log)
  names them. Every verdict below is the verdict of the run above, and it repeats the
  verdict of the earlier pass.

## Verdicts

| Test | Checks | Verdict |
| --- | --- | --- |
| Rfc791DatagramDelivery.test | RFC791-FWD-1, DLV-1, PROTO-1, HDR-1, HDR-2, HDR-3 | PASS |
| Rfc791TtlDecrement.test | RFC791-TTL-1, TTL-3, CKSUM-1 | PASS |
| Rfc791TtlExpiry.test | RFC791-TTL-2, RFC792-TE-1 | PASS |
| Rfc791FragmentReassembly.test | RFC791-FRAG-1..4, REASM-1; RFC1122-REASM-1 | PASS |
| Rfc791DontFragment.test | RFC791-FRAG-5, RFC792-DU-4; RFC6864-ID-6 | PASS |
| Rfc791Identification.test | RFC6864-ID-5 (governs RFC791-ID-1) | PASS |
| Rfc791MinimumSizes.test | RFC791-FRAG-6; RFC1122-REASM-2 (governs RFC791-REASM-2) | PASS |
| Rfc1122TtlOneAtDestination.test | RFC1122-TTL-2; covers TTL-3, RFC791-TTL-1 | PASS |
| Rfc791InterleavedReassembly.test | RFC791-REASM-3, RFC1122-REASM-1 | PASS |
| Rfc791SameIdDifferentProtocol.test | RFC791-REASM-1 (four-field key), REASM-3 | **FAIL (expected)** — model gap |
| Rfc6864AtomicIdentification.test | RFC6864-ID-3, ID-7; covers ID-6, notes ID-2 | PASS |
| Rfc1122ChecksumDiscard.test | RFC1122-CKSUM-1 (governs RFC791-CKSUM-2) | **FAIL (expected)** — model gap |
| Rfc1122VersionDiscard.test | RFC1122-VER-1 | **FAIL (expected)** — model gap |
| Rfc1122ForeignDestination.test | RFC1122-ADDR-2 | PASS |
| Rfc1122InvalidSourceAddress.test | RFC1122-ADDR-3, ADDR-4 | **FAIL (expected)** — model gap |
| Rfc1122UnknownIcmpType.test | RFC1122-ICMP-1 | **FAIL (expected)** — model gap, a runtime error |
| Rfc1122HostErrorReport.test | RFC1122-ERR-1, DU-1, ICMP-2, ICMP-4 | PASS |
| Rfc1122NoErrorAboutError.test | RFC1122-ICMP-5 | PASS |
| Rfc1122NoErrorForBroadcast.test | RFC1122-ICMP-6 | PASS |
| Rfc1122NoErrorForLinkBroadcast.test | RFC1122-ICMP-7 | **FAIL (expected)** — model gap |
| Rfc1122NoErrorForNonInitialFragment.test | RFC1122-ICMP-8 | PASS |
| Rfc1122NoErrorForInvalidSource.test | RFC1122-ICMP-9 | PASS |

Summary: 23 tests, 17 PASS, 6 FAIL (expected), 0 unexpected, in 2.6 s. Every FAIL is a
model gap declared with `%# expected-result: FAIL`; each test keeps the faithful assertion,
and each failed at the step its description predicts (the discard record, the absence
watch, or the runtime error). No specification misread was found.

## Deviations between the English observations and the test steps

Each one is also stated in the `%description` of its test. None weakens an expectation.
The five deviations of pass 2 stand and are not repeated here.

1. **A UDP discard record is read from the header, not filtered.** Five checks observe
   "host B's UDP finds no program for the port" (host error report, the three suppression
   checks at a closed port, and the invalid source address). UDP removes the packet's
   protocol tag before it looks for a socket
   ([Udp.cc:932](../../../../../src/inet/transportlayer/udp/Udp.cc#L932)), so the drop
   record of
   [Udp.cc:1093-1098](../../../../../src/inet/transportlayer/udp/Udp.cc#L1093-L1098)
   carries no tag and a `udp.destPort == N` expression cannot dissect it. The step reads
   the UDP header at the front of the record instead. Same observation, other reader.
2. **The two halves of "silently" are one absence watch.** In the four silent-discard
   checks, "nothing goes upward" and "nothing goes back" are one `never` step over the
   whole host with a predicate for both, because two consecutive watches cannot cover the
   same window: the second would open when the first closes, after any report would have
   left.
3. **Host error report, one message at two points.** The TOS byte is an outer header
   field, read at host A's interface; the quoted header is read at host A's network layer
   (`packetSentToUpper` of `hostA.ipv4.ip`), where the ICMP message is the front of the
   packet and the quote can be walked without the outer headers.
4. **Non-initial fragment, the quote at R2's network layer.** The absence watch reads the
   quoted header of every Time Exceeded that R2's ICMP hands to its network layer
   (`packetReceivedFromUpper` of `r2.ipv4.ip`), for the same reason.
5. **The relay's knowledge of an identification.** In the atomic-identification and
   same-identification checks the relay rewrites an identification "to that of datagram 1".
   The tap's mutator cannot read captures, so it subtracts one from the field, which is
   right because the model draws identifications from one counter per network layer
   ([Ipv4.cc:1089](../../../../../src/inet/networklayer/ipv4/Ipv4.cc#L1089)). This is model
   knowledge inside a test; one observation confirms the result on the wire, so a wrong
   assumption fails that step instead of passing vacuously.
6. **Two taps for one relay.** The same-identification check needs a rewrite and a hold at
   once; a tap carries one rule, so the relay is two taps in series.
7. **The ICMP checksum under a type rewrite.** The unknown-type check rewrites the type of
   an echo request. The model's ICMP runs in its declared checksum mode, in which the
   field is correct by declaration; that is the simulation form of a rewriter that
   recomputes it. The IPv4 header checksum is recomputed by the helper after every IPv4
   rewrite, and corrupted on purpose only in the checksum check.

## Scenario settings beyond the templates

- `**.ipv4.ip.checksumMode = "computed"` in the seven relay checks that rewrite or corrupt
  an IPv4 header. The pattern names the IPv4 module only: `**.checksumMode` would also
  switch UDP and ICMP to computed checksums, and a rewritten source or destination address
  would then fail the UDP pseudo-header checksum, which is not the rule under test.
- `*.hostB.ipv4.ip.timeToLive = 1` in the no-error-about-an-error check: host B's own
  error reports leave with TTL 1 and expire at the gateway. Ordinary configuration, and
  the observation of RFC1122-TTL-4 for free.
- `*.hostA.ipv4.ip.limitedBroadcast = true` in the broadcast check; the model gates
  limited broadcasts from the upper layer behind this switch.
- `*.hostA.app[*].dontFragment = true` in the atomic-identification check.
- A `PingApp` on host A as the vehicle of the unknown-type and same-identification checks.
- `*.r1.eth[1].mac.mtu = 576B` on the two-gateway mockup.

## Failure history during authoring

Every initial failure was a test error. None was a model gap, and none changed what a
passing test proves.

1. **A UDP drop record cannot be filtered by `udp.*`** (deviation 1). Four tests waited
   for `hostB.udp packetDropped udp.destPort == N` and missed the deadline although the
   trace showed the drop and the report that followed it.
2. **The MAC's send signal marks the end of a transmission.** `packetSentToLower` at an
   Ethernet MAC is emitted when the frame has left, and a node 50 ns down the link
   processes it at once, before the sender's next frame produces its own signal. Two tests
   had "the gateway discards or forwards the first datagram" after "the sender's second
   frame", and the discard happened before the step opened. The steps and the two check
   documents were reordered.
3. **A fragment is not dissected beyond IP** (already a pass 2 lesson). The
   same-identification test selected fragments with `udp.destPort` and `icmpv4.type`,
   which never match a fragment; `ipv4.protocolId == 17` and `== 1` do, and the enum
   compares as a number.
4. **The runner counts a runtime error as a FAIL.** The unknown-type test was first
   declared `expected-result: ERROR`; the runner reported `FAIL (unexpected)`. The
   declaration is `FAIL`, and the description says that the verdict line is missing
   because the simulation stopped.

## Model analysis — where INET implements the checked behavior

All line numbers are of this tree. The pass 2 analysis of the normal path stands; this
section covers the level 3 behaviors.

**Passes.**

- **TTL 1 at the destination (RFC1122-TTL-2):** the local delivery path
  (`preroutingFinish` to `reassembleAndDeliver`,
  [Ipv4.cc:345-400](../../../../../src/inet/networklayer/ipv4/Ipv4.cc#L345-L400)) reads no
  TTL; the check `getTimeToLive() <= 0` sits on the forwarding path only
  ([Ipv4.cc:941-951](../../../../../src/inet/networklayer/ipv4/Ipv4.cc#L941-L951)).
  Observed: 108 octets at host B's UDP after arrival with TTL 1.
- **A configurable default TTL (RFC1122-TTL-4):**
  [Ipv4.ned:98](../../../../../src/inet/networklayer/ipv4/Ipv4.ned#L98),
  `int timeToLive = default(32)`. Observed: host B's report left with TTL 1 when set so.
- **Foreign destination (RFC1122-ADDR-2):** a host with forwarding off drops a datagram
  for another address with reason `FORWARDING_DISABLED` and no report
  ([Ipv4.cc:405-414](../../../../../src/inet/networklayer/ipv4/Ipv4.cc#L405-L414)).
  Observed: the discard record, nothing else.
- **Host error report (RFC1122-ERR-1, DU-1, ICMP-2, ICMP-4):** UDP asks ICMP for
  Destination Unreachable code 3
  ([Udp.cc:1093-1120](../../../../../src/inet/transportlayer/udp/Udp.cc#L1093-L1120));
  ICMP quotes the header plus `quoteLength` octets
  ([Icmp.cc:206-210](../../../../../src/inet/networklayer/ipv4/Icmp.cc#L206-L210)),
  8 by default ([Icmp.ned:23](../../../../../src/inet/networklayer/ipv4/Icmp.ned#L23)).
  Observed: type 3 code 3, TOS 0, the quoted identification, addresses, protocol 17 and
  the two UDP ports as sent.
- **Suppression, four of five cases:** `Icmp::maySendErrorMessage`
  ([Icmp.cc:101-141](../../../../../src/inet/networklayer/ipv4/Icmp.cc#L101-L141))
  declines a report for a broadcast or multicast destination (line 111), for a source that
  is unspecified, multicast or broadcast (lines 117-121), for a non-first fragment (line
  126), and for an ICMP error message (lines 131-137). Observed: no Time Exceeded for the
  expired report, no Destination Unreachable for the broadcast or the broadcast-sourced
  datagram, and exactly one Time Exceeded for the two expired fragments — the trace shows
  "won't send ICMP error messages about errors in non-first fragments".
- **Interleaved reassembly (RFC791-REASM-3):** one buffer per key, fragments accepted in
  any order ([Ipv4FragBuf.cc](../../../../../src/inet/networklayer/ipv4/Ipv4FragBuf.cc)).
  Observed: datagram 2 delivered whole before the held fragment of datagram 1 arrived,
  then datagram 1 whole.
- **Atomic identification (RFC6864-ID-3, ID-7):** only a fragment enters the buffer
  ([Ipv4.cc:825](../../../../../src/inet/networklayer/ipv4/Ipv4.cc#L825)), so a repeated
  identification on an atomic datagram is never read; forwarding changes the TTL only
  ([Ipv4.cc:320-325](../../../../../src/inet/networklayer/ipv4/Ipv4.cc#L320-L325)).
  Observed: both datagrams delivered; DF set on link 2.

**Gaps.** Six statements failed; each is a behavior the model lacks, and none is touched
by a test.

1. **Header checksum verification (RFC1122-CKSUM-1, RFC791-CKSUM-2).**
   [Ipv4.cc:282](../../../../../src/inet/networklayer/ipv4/Ipv4.cc#L282):
   `if (!ipv4Header->isCorrect() && !ipv4Header->verifyChecksum())`. The checksum is
   consulted only when the header is already structurally wrong. Observed: the datagram
   with the complemented checksum was delivered to UDP; no discard record. Pass 2 named
   this the first candidate for a defect; it is one now.
2. **Version (RFC1122-VER-1).** `Ipv4::handleIncomingDatagram`
   ([Ipv4.cc:268-310](../../../../../src/inet/networklayer/ipv4/Ipv4.cc#L268-L310)) reads
   no version field. Observed: the version-5 datagram was delivered to UDP.
3. **Invalid source address (RFC1122-ADDR-3).** Neither the network layer
   (`handleIncomingDatagram`, `preroutingFinish`) nor UDP
   ([Udp.cc:926-990](../../../../../src/inet/transportlayer/udp/Udp.cc#L926-L990))
   examines the source address of a received datagram. Observed: the datagram with source
   255.255.255.255 reached the sink application.
4. **Unknown ICMP type (RFC1122-ICMP-1).**
   [Icmp.cc:326](../../../../../src/inet/networklayer/ipv4/Icmp.cc#L326):
   `default: throw cRuntimeError("Unknown ICMP type %d", ...)`. Observed:
   `<!> Error: Unknown ICMP type 42 -- in module (inet::Icmp) Rfc791TapNet.hostB.ipv4.icmp`
   and the end of the simulation. A crafted ICMP message stops the whole simulation; this
   is the one gap that is also a robustness problem for every scenario that carries
   external or fuzzed traffic.
5. **Link-layer broadcast (RFC1122-ICMP-7).** `maySendErrorMessage` decides from the IP
   addresses only and never consults the `MacAddressInd` of the frame. Observed:
   Destination Unreachable left host B at t = 0.10004367 s, and the absence watch fired.
6. **The reassembly key (RFC791-REASM-1, REASM-3).**
   [Ipv4FragBuf.h:31-38](../../../../../src/inet/networklayer/ipv4/Ipv4FragBuf.h#L31-L38)
   keys a buffer on identification, source and destination;
   [Ipv4FragBuf.cc:38-41](../../../../../src/inet/networklayer/ipv4/Ipv4FragBuf.cc#L38-L41)
   fills it. The protocol, the fourth field of RFC 791 §2.3, is missing. Observed: the two
   ICMP fragments entered the UDP datagram's buffer, completed it, and the result went up as
   ICMP ("Passing up to protocol icmpv4"); host B answered the echo. The last UDP fragment
   then found no buffer ("No complete datagram yet") and the UDP datagram was lost. The
   practical exposure is small: the model's own sources draw the identification from one
   counter for all protocols, so a collision needs a forged or a wrapped identification.

## Model observations the checks did not claim

1. **Off-net datagrams are not limited to 576 octets (RFC1122-FRAG-4, a should).** The
   fragment-and-reassembly trace shows the host send the 1028-octet datagram whole to an
   off-net destination ("Sending UdpBasicAppData-0 (1028 B) to output interface eth0").
   The model declines the should. Recorded as `declined` in the ledger; not a defect.
2. **A request for TTL 0 stops a debug build.**
   [Ipv4.cc:1094-1095](../../../../../src/inet/networklayer/ipv4/Ipv4.cc#L1094-L1095):
   `if (ttl != -1) { ASSERT(ttl > 0); }`. RFC1122-TTL-1 says a host must not send TTL 0;
   the model refuses with an assertion, which a protocol test cannot classify as a pass
   or a failure. It stays a candidate; the category decision is in `categories.md`.
3. **Loopback and class E sources are not in the suppression list.** `maySendErrorMessage`
   covers the unspecified, multicast and broadcast sources of RFC1122-ICMP-9 but not a
   loopback (127.x) or class E source. Not tested; a sharpening candidate.
4. **RFC 6864 is followed but not claimed.** The identification handling satisfies the
   RFC 6864 checks (uniqueness for non-atomic datagrams, the field of an atomic datagram
   ignored, DF forwarded intact), and no module documentation names the document. See
   `conformance.md`.
5. The two observations of pass 2 stand: the checksum is not computed by default, and the
   identification counter is per module.

## Feature support

The verdicts above feed the support column and the achieved level of the coverage ledger,
[`coverage.md`](coverage.md#feature-support), by the rules of step 7 and of the levels.

## Sharpening candidates for the next pass

- **Module tests for the five internal handoffs** of IPV4-F-ERROR-DELIVERY (RFC1122-ICMP-3,
  DU-2, TE-1, PP-2) and the two interface statements (REASM-3, FRAG-2): a module test can
  ask the interface what a protocol test cannot see on the wire.
- **A standing negative rule in the framework** for RFC1122-ADDR-4 (no datagram on any link
  with a broadcast, zero or loopback source), which no ordered step can express.
- **Loopback and class E sources** for RFC1122-ICMP-9, where the code reading predicts a
  gap; a subnet-directed broadcast and a multicast destination for RFC1122-ICMP-6.
- **Local fragmentation (RFC1122-FRAG-1)** with a small MTU on host A's own interface, and
  RFC1122-FRAG-3 with it.
- **Parameter Problem (RFC1122-PP-1):** the model sends one for a total length larger than
  the data ([Ipv4.cc:290-294](../../../../../src/inet/networklayer/ipv4/Ipv4.cc#L290-L294));
  a crafted total length is one relay rule away.
- **RFC6864-ID-4** needs a retransmitted non-atomic datagram: a TCP scenario, in the TCP
  suite.
- **Level 4 opens with the reassembly timeout** (RFC1122-REASM-4, REASM-5): the
  `fragmentTimeout` parameter and a withheld fragment.
