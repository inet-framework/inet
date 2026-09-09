# IPv6 checks — run results and model analysis (pass 1, level 2)

> **Kind:** report · **Status:** snapshot 2026-09-09 · **Seal:** none · **Owns:** — · **Stands on:** [rfc8200/catalog.md](../../standard/rfc8200/catalog.md), [rfc4443/catalog.md](../../standard/rfc4443/catalog.md), [checks.md](../../protocol/ipv6/checks.md)

Step 7 artifact of the standards test workflow. This is the first document of the IPv6
workflow that may reference code.

- Date: 2026-09-09. Tree: `inet-rfc-tests-ipv6`, branch `topic/rfc-tests-ipv6`, built from
  commit `95f9805952` (`master`). No source file changed on this branch.
- Command, after the `setenv` scripts of OMNeT++ and INET:

  ```sh
  cd tests/protocol/lib && MODE=debug ./build.sh
  inet_run_protocol_tests -p inet -w ipv6
  ```

  The `ipv6` suite also holds the two pre-existing Mobile IPv6 tests; they ran and passed
  and are not part of this pass.

## Verdicts

| Test | Checks | Verdict |
| --- | --- | --- |
| Rfc8200DatagramDelivery.test | RFC8200-HDR-1, HDR-2, HDR-3, HDR-4, DLV-1, HL-1, CKSUM-1 | PASS |
| Rfc8200HopLimitExpiry.test | RFC8200-HL-2, RFC4443-TE-1, RFC4443-ERR-2 | PASS |
| Rfc8200HopLimitOneAtDestination.test | RFC8200-HL-3; covers HL-1 | PASS |
| Rfc8200SourceFragmentation.test | RFC8200-FRAG-3, FRAG-4, FRAG-5 (structure), FRAG-6, EXT-1, REASM-1, REASM-2, MTU-4; covers HDR-3 | PASS |
| Rfc8200FragmentPayloadLength.test | RFC8200-FRAG-4, FRAG-5 (payload length), HDR-2 | **FAIL (expected)** — model gap |
| Rfc8200FragmentIdentification.test | RFC8200-FRAG-2 | PASS |
| Rfc4443PacketTooBig.test | RFC8200-FRAG-1, RFC4443-PTB-1, RFC4443-ERR-2 | PASS |
| Rfc4443PacketTooBigMtu.test | RFC4443-PTB-2 | **FAIL (expected)** — model gap |
| Rfc8200LinkMtuPacket.test | RFC8200-MTU-2 | PASS |
| Mipv6Registration.test, Mipv6Interface.test (pre-existing) | — | PASS |

Summary: 11 tests in the suite, 9 PASS, 2 FAIL (expected), 0 unexpected, in 2.5 s. Both
failures are model gaps declared with `%# expected-result: FAIL`; each test keeps the
faithful assertion and fails at the step its description predicts. No specification misread
was found.

## Deviations between the English observations and the test steps

Each one is also stated in the `%description` of its test. None weakens an expectation.

1. **Addresses by predicate.** The filter engine holds an address as an opaque value, so
   "host A's address as source" and "the same addresses on link 2" are typed predicates;
   the node's address comes from the address resolver.
2. **The program's receipt observed at UDP** (datagram delivery, observation 4), as in the
   IPv4 pass: the sink emits no signal the tester turns into an event.
3. **The router's report as the record of the decision** (hop limit expiry and packet too
   big, observation 2). The model emits no discard signal for a hop limit of zero or for a
   packet that does not fit the next link; the ICMPv6 message is created in the same
   instant, and the step observes it where the router's ICMPv6 hands it to its IPv6 layer
   (`packetReceivedFromUpper` of `router.ipv6.ipv6`). The absence watch on link 2 opens
   after host A has received the report, so a forward in the same instant as the decision
   would escape it; the report is the in-time observation, as the discard record was in
   IPv4.
4. **A fragment is not dissected beyond its fragment header.** The UDP header of a first
   fragment is read from the chunk sequence by a typed reader; the fragment header's own
   fields are filtered by their chunk name (`Ipv6FragmentHeader.fragmentOffset`).
5. **The payload length of a fragment is read by type.** On a fragment the generic field
   reader can resolve `ipv6.payloadLength` to the fragment header, which has no such field;
   the payload-length test reads the base header directly, and reads the first fragment's
   value through a capture because one event serves one step.
6. **The fragment offset is compared in octets.** The model stores the offset in octets and
   serializes it in 8-octet units; the check document gives both (1232 octets, 154 units).
7. **Two checks where the English had one.** The payload length of the fragments and the
   MTU field of Packet Too Big are separate tests, because each fails, and a failing field in
   the middle of a program would have hidden the reassembly and the suppression verdicts
   behind it. The check document was split the same way before the run was recorded.

## Scenario settings beyond the templates

- The applications start at 5 s. IPv6 needs about 4 s in this mockup before a host can
  send: router solicitation and advertisement, address configuration, and duplicate
  address detection run first, and a datagram handed to IPv6 earlier waits in neighbor
  discovery until they finish. The mockup states the 5 s.
- `**.udp.checksumMode = "computed"` in the datagram-delivery test, so the UDP checksum
  field carries a computed value; in the model's default mode the field is a placeholder.
- `*.hostA.eth[0].mac.mtu = 1280B` in the three fragmentation tests and
  `*.router.eth[1].mac.mtu = 1280B` in the two Packet Too Big tests: the minimum IPv6 link
  MTU, on the side that must react to it.
- `timeToLive` of the sending application sets the hop limit: 64 by default in the mockup,
  1 and 2 in the hop limit checks.

## Failure history during authoring

Every initial failure was a test error. None was a model gap, and none changed what a
passing test proves.

1. **A capture with a unit breaks the next step** (the TCP pass lesson, met again): a
   capture of `ipv6.payloadLength` stopped the run with "Attempt to use the value '108B' as
   a dimensionless number". The capture was dropped; where a length is needed later, a typed
   capture strips the unit.
2. **`ipv6.payloadLength` on a fragment errored** with "field 'payloadLength' not found on
   'inet::Ipv6FragmentHeader'": the generic field reader landed on the fragment header. A
   typed reader of the base header replaced it (deviation 5).
3. **The first fragmentation program filtered fragments by payload length** and never
   matched, because the field carries the wrong value (see the gaps). That is how the gap
   was found; the check was split so the rest of the program could run.

## Model analysis — where INET implements the checked behavior

All line numbers are of this tree.

**Passes.**

- **Header fields (HDR-1, HDR-3, HDR-4, DLV-1):** `Ipv6::encapsulate`
  ([Ipv6.cc:940-966](../../../../../src/inet/networklayer/ipv6/Ipv6.cc#L940-L966)) sets the
  next header from the transport protocol, the payload length from the transport packet,
  and the addresses; local delivery demultiplexes on the protocol field
  (`localDeliverFinish`). Observed: version 6, payload length 108, next header 17, host A to
  host B, delivery to UDP.
- **Hop limit (HL-1, HL-2, HL-3, RFC4443-TE-1):** the decrement sits in `routePacket` for a
  forwarded packet only
  ([Ipv6.cc:466-494](../../../../../src/inet/networklayer/ipv6/Ipv6.cc#L466-L494)); the
  check `getHopLimit() <= 0` and the Time Exceeded sit in `fragmentAndSend`
  ([Ipv6.cc:1052-1057](../../../../../src/inet/networklayer/ipv6/Ipv6.cc#L1052-L1057)); a
  packet for the node itself takes the local delivery path before any of that
  ([Ipv6.cc:455-462](../../../../../src/inet/networklayer/ipv6/Ipv6.cc#L455-L462)).
  Observed: 64 on link 1, 63 on link 2; with hop limit 1, Time Exceeded type 3 code 0 at
  host A and nothing on link 2; with hop limit 2, delivery at host B.
- **Source fragmentation (FRAG-2, FRAG-3, FRAG-6, EXT-1):** `fragmentAndSend`
  ([Ipv6.cc:1073-1130](../../../../../src/inet/networklayer/ipv6/Ipv6.cc#L1073-L1130))
  computes the piece as the largest multiple of 8 that fits the MTU after the two headers
  (1232 for 1280), draws one identification per packet from a counter, and builds every
  fragment from a copy of the base header plus a fragment header. Observed: pieces 1232 and
  228, offsets 0 and 1232, M = 1 and 0, one identification, a different one for the next
  packet; the router forwarded both fragments unchanged.
- **No router fragmentation (FRAG-1, RFC4443-PTB-1):**
  [Ipv6.cc:1066-1071](../../../../../src/inet/networklayer/ipv6/Ipv6.cc#L1066-L1071):
  "routed datagrams are not fragmented", a forwarded packet larger than the MTU gets
  `ICMPv6_PACKET_TOO_BIG`. Observed: type 2 at host A, nothing on link 2.
- **Reassembly (REASM-1, REASM-2, MTU-4):** `Ipv6FragBuf::addFragment`
  ([Ipv6FragBuf.cc:37-116](../../../../../src/inet/networklayer/ipv6/Ipv6FragBuf.cc#L37-L116))
  keys on identification, source and destination — the three fields of RFC 8200, unlike the
  IPv4 buffer, which lacks the protocol — and measures each fragment from the packet rather
  than from the payload length field. Observed: the 1460-octet datagram at host B's UDP.
- **Link MTU (MTU-2):** a 1500-octet packet crossed both links and reached UDP.
- **UDP checksum (CKSUM-1):** with the computed mode on, the field carried a non-zero value.
  The compute half only; the discard of a zero checksum is a level 3 crafted input.

**Gaps.** Two statements failed; each is a field the model does not fill, and neither is
touched by a test.

1. **Packet Too Big carries MTU 0 (RFC4443-PTB-2).**
   [Icmpv6.cc:271-273](../../../../../src/inet/networklayer/icmpv6/Icmpv6.cc#L271-L273):
   `// TODO implement MTU support.` then `createPacketTooBigMsg(0)`; the caller in
   `Ipv6::fragmentAndSend` passes no MTU either
   ([Ipv6.cc:1069](../../../../../src/inet/networklayer/ipv6/Ipv6.cc#L1069), "TODO set
   MTU"). Observed in the trace: `Icmpv6PacketTooBigMsg ... code = 0, MTU = 0`. The message
   is sent, to the right node, with the right type; it carries nothing a source could act
   on, and path MTU discovery (RFC 8201) cannot work from it. The ICMPv6 module claims
   RFC 4443 explicitly, so this is a gap against a stated claim.
2. **Every fragment carries the payload length of the original packet (RFC8200-FRAG-4,
   FRAG-5, HDR-2).** The fragment loop of `fragmentAndSend` duplicates the base header
   ([Ipv6.cc:1108-1112](../../../../../src/inet/networklayer/ipv6/Ipv6.cc#L1108-L1112)) and
   sets its next header to 44, but never calls `setPayloadLength`. Observed in the trace:
   `payloadLength = 1460 B` on a 1280-octet fragment packet and on a 276-octet one. The
   model's own receiver is unaffected, because `Ipv6FragBuf` measures the fragment
   ([Ipv6FragBuf.cc:72](../../../../../src/inet/networklayer/ipv6/Ipv6FragBuf.cc#L72))
   instead of reading the field; any receiver that follows RFC 8200 §4.5 ("the length of
   each fragment is computed by subtracting from the packet's Payload Length the length of
   the headers") reads 1452 octets for both pieces and reassembles garbage or rejects the
   fragments. A recorded trace of the model shows the same wrong field.

## Model observations the checks did not claim

1. **The default hop limit is 30** (`IPv6_DEFAULT_ADVCURHOPLIMIT` in
   [Ipv6InterfaceData.h:31](../../../../../src/inet/networklayer/ipv6/Ipv6InterfaceData.h#L31)),
   and a request for hop limit 0 stops a debug build with an assertion
   ([Ipv6.cc:966](../../../../../src/inet/networklayer/ipv6/Ipv6.cc#L966)), as in IPv4.
2. **The error message quote fills the message up to 1280 octets**
   ([Icmpv6.cc:282-290](../../../../../src/inet/networklayer/icmpv6/Icmpv6.cc#L282-L290)),
   which is the rule of RFC4443-ERR-1; not asserted in this pass. The comment above the code
   warns that the quoted packet is not truncated but only re-measured.
3. **The model cites RFC 2460 in its header definitions**
   ([Ipv6Header.msg:24](../../../../../src/inet/networklayer/ipv6/Ipv6Header.msg#L24),
   [Ipv6ExtensionHeaders.msg:78](../../../../../src/inet/networklayer/ipv6/Ipv6ExtensionHeaders.msg#L78)),
   a document that RFC 8200 replaced in 2017, and RFC 8200 in one option-processing
   comment. See `conformance.md`.
4. **Neighbor discovery frames share the wire with every scenario**, and a datagram handed
   to IPv6 before duplicate address detection completes waits about 3 s in neighbor
   discovery. The checks tolerate the frames and start after the wait.

## Feature support

The verdicts above feed the support column and the achieved level of the coverage ledger,
[`coverage.md`](coverage.md#feature-support), by the rules of step 7 and of the levels.

## Sharpening candidates for the next pass

- **Level 3 opens with RFC 8504**, the node requirements, and the message processing rules
  of RFC 4443 §2.4: the five cases in which no error is sent, the unknown types, the
  crafted fragments of RFC8200-REASM-4 to REASM-6, a packet that arrives with hop limit 0
  (the exact case of RFC8200-HL-3), and the zero UDP checksum that a receiver must discard.
- **RFC4443-ERR-1**, the content of the error message: quote length up to 1280 octets.
- **RFC4443-DU-4**, the closed port, belongs with a UDP-over-IPv6 pass.
- **Level 4:** the 60-second reassembly timer (REASM-3) and path MTU discovery (RFC 8201),
  which the Packet Too Big gap blocks until the MTU field is filled.
- **A serializer unit test** for the ICMPv6 pseudo-header checksum (CKSUM-2) and for the
  8-octet unit of the fragment offset.
