# UDP checks — run results and model analysis (pass 1, level 2)

> **Kind:** report · **Status:** snapshot 2026-09-08 · **Seal:** none · **Owns:** — · **Stands on:** [rfc768/catalog.md](../../standard/rfc768/catalog.md), [rfc792/catalog.md](../../standard/rfc792/catalog.md), [checks.md](../../protocol/udp/checks.md)

Step 7 artifact of the standards test workflow. This is the first document of the UDP
workflow that may reference code.

- Date: 2026-09-08. Tree: `inet-rfc-tests-udp`, branch `topic/rfc-tests-udp` on top of
  `topic/rfc-tests-ipv4`, source identical to `master`.
- Command, after the `setenv` scripts of OMNeT++ and INET:

  ```sh
  cd tests/protocol/lib && MODE=debug ./build.sh
  inet_run_protocol_tests -p inet -w udp
  ```

## Verdicts

| Test | Checks | Verdict |
| --- | --- | --- |
| Rfc768DatagramDelivery.test | RFC768-HDR-1, HDR-2, HDR-3, PROTO-1, UI-1 | PASS |
| Rfc768Checksum.test | RFC768-CKSUM-2 (CKSUM-1 presence only) | PASS |
| Rfc768PortUnreachable.test | RFC792-DU-3, RFC768-HDR-2 | PASS |

Summary: 3 PASS in 0.4 s. The IPv4 suite of the same tree stays at 8 PASS. The model
conforms to every selected statement. No `%# expected-result: FAIL` marker was necessary.

## Observations that could not run

Both are limits of the scenario tooling, not verdicts on the UDP module. They are stated in
the `%description` of the test and here, and the ledger carries them as bounds.

1. **Datagram delivery, observations 5 and 6: the empty datagram.** No application in this
   model can send a UDP datagram with no data. `Packet::insertAt`, which every sender
   reaches through `insertAtBack`, refuses a chunk of length zero
   (`CHUNK_CHECK_USAGE(chunk->getChunkLength() > b(0), "chunk is empty")`,
   `src/inet/common/packet/Packet.cc`), and a run with `messageLength = 0B` stopped with
   `Error: chunk is empty`. The minimum length of eight in RFC768-HDR-3 is therefore not
   observed from a sender. Whether the receiver accepts an 8-octet datagram is a question
   for injection, which is the level 3 toolset.
2. **The two halves of RFC768-UI-1 at the program interface.** The program's receipt of the
   data is observed as UDP's upward handoff; the source port and the source address that
   the program learns are not observable by the tester. They are confirmed on the wire.

## Deviations between the English observations and the test steps

1. **A program's receipt is observed at UDP.** As in the IPv4 pass: the sink application
   emits only `packetReceived`, which the tester does not turn into an event, so every
   "hands N octets upward" observation is `hostB.udp` `packetSentToUpper` with a size
   predicate. Two programs are told apart by the sizes 100 and 200.
2. **Port unreachable, observation 2, the discard matched by size.** `Udp::processUDPPacket`
   removes the `PacketProtocolTag` before it drops an undeliverable datagram, so the packet
   that rides on the `packetDropped` signal cannot be dissected as UDP: `udp.destPort` on it
   is a silent non-match. The step matches the 108-octet size instead. The IPv4 drop paths
   keep the tag, which is why the IPv4 tests could filter their discards by field.

## Scenario settings beyond the templates

- `*.hostA.udp.checksumMode = "computed"` and `*.hostC.udp.checksumMode = "disabled"` in
  the checksum test: the two senders that RFC768-CKSUM-2 needs. The default mode is
  neither; see the observations below.
- `*.hostB.numApps = 0` in the port unreachable test, so that no port is open.
- The checksum test's network is a chain, host A — host B — host C, with host B a host with
  two interfaces and no forwarding.

## Failure history during authoring

One initial failure, a test error: the `udp.destPort` filter on the drop signal (deviation
2). Everything else passed on its first run.

## Model analysis — where INET implements the checked behavior

- **Demultiplexing by destination port (HDR-2):** the port is read at
  [Udp.cc:940](../../../../../src/inet/transportlayer/udp/Udp.cc#L940) and the socket found
  by it in `findSocketForUnicastPacket`,
  [Udp.cc:1036-1038](../../../../../src/inet/transportlayer/udp/Udp.cc#L1036-L1038), called
  at [Udp.cc:967](../../../../../src/inet/transportlayer/udp/Udp.cc#L967).
- **Length field (HDR-3):** set to header plus data on send,
  [Udp.cc:792-796](../../../../../src/inet/transportlayer/udp/Udp.cc#L792-L796); checked
  against the actual size on receipt,
  [Udp.cc:944-945](../../../../../src/inet/transportlayer/udp/Udp.cc#L944-L945).
- **Checksum (CKSUM-2):** `insertChecksum`,
  [Udp.cc:817-848](../../../../../src/inet/transportlayer/udp/Udp.cc#L817-L848): zero in the
  disabled mode, a placeholder in the declared mode, the Internet checksum over the pseudo
  header in the computed mode. The all-ones rule for a computed zero is at
  [Udp.cc:872-877](../../../../../src/inet/transportlayer/udp/Udp.cc#L872-L877), with the
  RFC 768 sentence quoted verbatim above it. A received zero is accepted over IPv4,
  [Udp.cc:1012-1017](../../../../../src/inet/transportlayer/udp/Udp.cc#L1012-L1017).
- **Port unreachable (RFC792-DU-3):** `processUndeliverablePacket`,
  [Udp.cc:1093-1132](../../../../../src/inet/transportlayer/udp/Udp.cc#L1093-L1132), emits
  the drop with reason `NO_PORT_FOUND` and asks ICMP for destination unreachable, code 3,
  through an `Icmpv4SendErrorReq`; ICMP decides whether to send it in `maySendErrorMessage`,
  [Icmp.cc:101-136](../../../../../src/inet/networklayer/ipv4/Icmp.cc#L101-L136). Observed:
  type 3, code 3 at host A.
- **Protocol 17 (PROTO-1):** observed on the wire in the IP header of every datagram.

## Model observations the checks did not claim

1. **The checksum is not computed by default.**
   [Udp.ned:64](../../../../../src/inet/transportlayer/udp/Udp.ned#L64):
   `string checksumMode @enum("disabled", "declared", "computed") = default("declared");`.
   The declared mode writes the placeholder `0xC00D`. The wire check of RFC768-CKSUM-2
   asks only for a nonzero value, so it would accept the placeholder; the test sets the
   computed mode, and the value observed is a real one. The distinction between a real and a
   placeholder checksum is a unit-test matter, RFC768-CKSUM-1.
2. **A wrong checksum or a wrong length is discarded.**
   [Udp.cc:948-956](../../../../../src/inet/transportlayer/udp/Udp.cc#L948-L956) drops with
   reason `INCORRECTLY_RECEIVED`, unconditionally in the computed mode. That rule is
   RFC 1122's, not RFC 768's, and it needs a corrupted datagram to check: level 3. It is
   worth noting the contrast with the IPv4 header checksum, which the model consults only
   when the header is already structurally wrong.
3. **The model states the IPv6 checksum rule without its source.**
   [Udp.cc:88-95](../../../../../src/inet/transportlayer/udp/Udp.cc#L88-L95) paraphrases the
   rule that the checksum is mandatory over IPv6 inside a `TODO` comment, with no RFC named.
4. **The drop signal loses the protocol tag** (deviation 2). A `packetDropped` emitted by
   `Udp` carries a packet that a dissector can no longer identify as UDP. Any observer that
   filters drops by UDP field is affected, not only this test.

## Feature support

The verdicts above feed the support column and the achieved level of the coverage ledger,
[`coverage.md`](coverage.md#feature-support).

## Sharpening candidates for the next pass

- **Level 3 opens with RFC 1122 §4.1**: the checksum becomes mandatory, and the discard of a
  wrong checksum (observation 2) becomes a check with interception.
- **Inject an 8-octet datagram**, so that the minimum of RFC768-HDR-3 is observed at the
  receiver, since no sender can produce it.
- **A serializer unit test for RFC768-CKSUM-1**, which also tells a real checksum from the
  declared placeholder.
- **Observe the source at the program interface**, if the tester learns to read the
  indication tags that UDP attaches, so that RFC768-UI-1 is complete.
