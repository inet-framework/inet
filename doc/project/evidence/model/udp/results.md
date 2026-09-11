# UDP checks — run results and model analysis (pass 2, level 3)

> **Kind:** report · **Status:** snapshot 2026-09-10 · **Seal:** none · **Owns:** — · **Stands on:** [rfc768/catalog.md](../../standard/rfc768/catalog.md), [rfc792/catalog.md](../../standard/rfc792/catalog.md), [rfc1122/catalog.md](../../standard/rfc1122/catalog.md), [checks.md](../../protocol/udp/checks.md)

Step 7 artifact of the standards test workflow. This is the first document of the UDP
workflow that may reference code.

- Date: 2026-09-10 15:18 +0200
- INET: branch `master`, commit `0868c36c88`, tree clean
- OMNeT++: 6.4.0
- Build: debug, built from this commit
- Compiler: Ubuntu clang version 23.0.0
- Platform: Ubuntu 26.04.1 LTS, Linux 7.0.0-31-generic x86_64
- Command, after the `setenv` scripts of OMNeT++ and INET:

  ```sh
  cd tests/protocol/lib && MODE=debug ./build.sh
  inet_run_protocol_tests -p inet -w udp
  ```
- Earlier passes ran on other trees. The pass log of [`coverage.md`](coverage.md#pass-log)
  names them. Every verdict below is the verdict of the run above, and it repeats the
  verdict of the earlier pass.

## Verdicts

Pass 1, level 2:

| Test | Checks | Verdict |
| --- | --- | --- |
| Rfc768DatagramDelivery.test | RFC768-HDR-1, HDR-2, HDR-3, PROTO-1, UI-1 | PASS |
| Rfc768Checksum.test | RFC768-CKSUM-2 (CKSUM-1 presence only) | PASS |
| Rfc768PortUnreachable.test | RFC792-DU-3, RFC768-HDR-2 | PASS |

Pass 2, level 3:

| Test | Checks | Verdict |
| --- | --- | --- |
| Rfc1122ChecksumDiscard.test | RFC1122-UCK-4 | PASS |
| Rfc768ChecksumCoversData.test | RFC768-CKSUM-1, RFC1122-UCK-4 | PASS |
| Rfc768ChecksumCoversPseudoHeader.test | RFC768-CKSUM-1, RFC1122-UCK-4 | PASS |
| Rfc768ZeroChecksumAccepted.test | RFC768-CKSUM-2, RFC1122-UCK-5 | PASS |
| Rfc1122ValidSourceAddress.test | RFC1122-UADDR-2 | PASS |
| Rfc1122ApplicationTtlAndTos.test | RFC1122-UAPI-1 | PASS |
| Rfc1122ApplicationSourceAddress.test | RFC1122-UMH-2 | PASS |
| Rfc1122ChecksumDefault.test | RFC1122-UCK-3 | **FAIL (unexpected)**, gap 1, a defect |
| Rfc1122MulticastSourceAddress.test | RFC1122-UADDR-1 | FAIL (expected), gap 2, unimplemented |
| Rfc768EmptyDatagram.test | RFC768-HDR-3, the minimum | **FAIL (unexpected)**, gap 3, a defect |

Summary: 13 tests, 10 PASS, **1 FAIL (expected), 2 FAIL (unexpected)**, so the suite reports FAIL.
Each failure is analysed below. The tallies of the other suites are not repeated here: a document
that quotes another suite's numbers goes stale on that suite's next run, and each suite's own
`results.md` holds its own.

## Which failures are declared, and which are not

Reviewed against
[the third principle of the guide](../../../guide/derive-tests-from-a-standard.md#principle-a-claimed-feature-gets-a-test):
a failure is declared expected only where the model does **not** claim the behavior, and a claim is
code.

| Test | Class | The claim, in the model |
| --- | --- | --- |
| `Rfc1122ChecksumDefault.test` | **defect** | The checksum computation exists and works: four tests of this suite set `checksumMode = "computed"` and pass. What is wrong is the value the parameter defaults to, and a setting that exists and holds a value the standard forbids is a defect. |
| `Rfc768EmptyDatagram.test` | **defect** | UDP handles the smallest datagram RFC 768 allows correctly. The defect is in `DataAgeFilter`, a statistic filter every standard receiving program switches on, which calls `peekData()` on a zero-length packet and stops the run. A crash in live code is a defect wherever it sits. |
| `Rfc1122MulticastSourceAddress.test` | unimplemented | Nothing validates the source address of a received datagram, at either layer. The one source test in the IPv4 receive path only warns about an unspecified address and discards nothing. |

The seven new passes are worth as much as the three failures. Four of them use the level 3
toolset to establish what level 2 could only describe: the checksum really covers the data
and the pseudo header, a wrong value really is discarded in silence, and a zero value really
is accepted. Two of them read a value the program named, back off the wire.

## Model gaps

### Gap 1 — the default checksum is a placeholder, not a sum

**Statement.** [RFC1122-UCK-3](../../standard/rfc1122/catalog.md#rfc1122-uck-3), must: the
generation of the checksum "MUST default to checksumming on", `rfc1122.txt:4586-4587`.

**What the model does.** The `checksumMode` parameter of the ~Udp module is
`default("declared")` (src/inet/transportlayer/udp/Udp.ned:64). In that mode
`Udp::insertChecksum` writes the constant 0xC00D into the checksum field
(Udp.cc:826-827) and marks the header correct without summing anything. The receiver in the
same mode consults the flag and not the arithmetic (`Udp::verifyChecksum`, the
CHECKSUM_DECLARED_CORRECT branch). The value cannot even leave the simulation:
`UdpHeaderSerializer::serializeFields` throws for any mode but disabled or computed.

**Why it is a gap.** A host in its default state must put a real checksum on the wire. A
constant is not a checksum: no receiver that computes can validate it, and two INET hosts
agree only because both consult the same flag. The test makes the point with a control:
host C, told to compute, carries exactly the value RFC 768 defines, which is how the test
shows that its own arithmetic is right; host A, told nothing, does not.

**What it costs.** A model run with the default UDP settings does not exercise the checksum
at all. Every check in this pass that is about the value therefore sets
`checksumMode = "computed"` and says so.

**Not a defect of the module.** The computed mode is present, and it is correct: five checks
of this pass depend on it and pass. What fails is the choice of default.

### Gap 2 — no layer validates the source address

**Statement.** [RFC1122-UADDR-1](../../standard/rfc1122/catalog.md#rfc1122-uaddr-1), must: a
datagram with an invalid source address, "e.g., a broadcast or multicast address", must be
discarded by UDP or by the IP layer, `rfc1122.txt:4646-4648`.

**What the model does.** Nothing. The relay set the source address to 224.0.0.1, the
all-hosts multicast address, and the program received the datagram:
`Received packet: ... (100 bytes) 224.0.0.1:4000 --> 10.0.0.6:5000`. Neither
`Ipv4::handleIncomingDatagram` nor `Ipv4::preroutingFinish` nor `Udp::processUdpPacket`
looks at the source address of a received datagram.

**Why it is a gap.** The rule protects a host from a datagram that no single host can have
sent, and it is one rule stated at two layers; the model implements it at neither.

**Already known from the other side.** The IPv4 suite records the same gap for the broadcast
form, in `tests/protocol/ipv4/Rfc1122InvalidSourceAddress.test` and in
[`ipv4/results.md`](../ipv4/results.md). This pass adds the multicast form and the UDP end
of it. One correction at one layer would close both.

### Gap 3 — an empty datagram stops the run

**Statement.** [RFC768-HDR-3](../../standard/rfc768/catalog.md#rfc768-hdr-3), description:
the length counts header and data, "so the minimum value of the length is eight".

**What the model does.** The UDP module is right. The log shows it accepting the 8-octet
datagram and handing zero octets to the program: `Sending payload up to socket sockId=1`,
then `UdpSink: Received packet: ... (0 B) ... 10.0.0.1:4000 --> 10.0.0.6:5000`. The run then
stopped:

```
Error: Returning an empty chunk is not allowed according to the flags: 0 while processing
statistic signal 'packetReceived' ... emitted from (inet::UdpSink)
```

**Where.** The receiving program declares
`@statistic[endToEndDelay](source="dataAge(packetReceived)")`
(src/inet/applications/udpapp/UdpSink.ned:36). `DataAgeFilter::receiveSignal`
(src/inet/common/ResultFilters.cc) calls `packet->peekData()` with no flags, and a packet of
zero length answers with an empty chunk, which is refused
(src/inet/common/packet/chunk/EmptyChunk.h:51).

**Why it is a gap.** RFC 768 allows a datagram of exactly eight octets. A model that stops
with a runtime error when one arrives cannot be used to study it. The fault is not in UDP
but in a statistic that every standard receiving program declares, which makes it the more
likely one for a user to meet.

**Not hidden.** Turning the statistic off in the scenario would make the test pass. The test
leaves it on, because what a user meets is exactly this error.

**How the check became possible.** The level 2 pass recorded that no sending program can
build an empty datagram and left the question to level 3. The relay answers it: it takes a
normal datagram off the wire and removes the data.

## Observations that could not run

These are limits of the scenario tooling, not verdicts on the UDP module. They are stated in
the `%description` of the test and here, and the ledger carries them as bounds.

1. **Datagram delivery, observations 5 and 6: the empty datagram, from a sender.** No
   application in this model can send a UDP datagram with no data. `Packet::insertAt`, which
   every sender reaches through `insertAtBack`, refuses a chunk of length zero
   (`CHUNK_CHECK_USAGE(chunk->getChunkLength() > b(0), "chunk is empty")`,
   `src/inet/common/packet/Packet.cc`), and a run with `messageLength = 0B` stopped with
   `Error: chunk is empty`. The send half of the minimum is therefore still not observed.
   **Pass 2 answered the receive half**: the relay removes the data of a normal datagram, and
   `Rfc768EmptyDatagram.test` shows what happens next. See gap 3.
2. **The two halves of RFC768-UI-1 at the program interface.** The program's receipt of the
   data is observed as UDP's upward handoff; the source port and the source address that
   the program learns are not observable by the tester. They are confirmed on the wire.
3. **Nine statements of RFC 1122 §4.1 that live at the program interface.** UDP passes an
   ICMP error, an IP option and the specific destination address up to the program, and a
   program can name the IP options of a datagram it sends. None of that is traffic between
   two nodes, and the tester turns packet signals into events. The model does implement the
   error path — `Udp::processIcmpv4Error` and `Udp::sendUpErrorIndication` hand the report to
   the socket — but it hands it up as an `Indication`, which is a message and not a packet,
   so no signal the tester watches carries it. The list and the reason for each are in
   [`checks.md`](../../protocol/udp/checks.md#statements-that-no-check-carries); the ledger
   carries them as `no check`.

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

Pass 1:

- `*.hostA.udp.checksumMode = "computed"` and `*.hostC.udp.checksumMode = "disabled"` in
  the checksum test: the two senders that RFC768-CKSUM-2 needs. The default mode is
  neither; see the observations below.
- `*.hostB.numApps = 0` in the port unreachable test, so that no port is open.
- The checksum test's network is a chain, host A — host B — host C, with host B a host with
  two interfaces and no forwarding.

Pass 2:

- `**.udp.checksumMode = "computed"` in the four checksum tests that need a value a receiver
  can recompute. `Rfc1122ChecksumDefault.test` sets it for the control host only, because
  the subject of that check is what a host does when nothing is set.
- `**.ipv4.ip.checksumMode = "computed"` wherever the relay rewrites the IPv4 header, so
  that the rewrite leaves a valid header checksum behind.
- `Rfc1122MulticastSourceAddress.test` says nothing about the UDP checksum on purpose. A
  computed checksum covers the source address, so after the rewrite the receiver could
  discard the datagram for the checksum and not for the address; the check is about the
  address.
- Fixed addresses in `Rfc1122ApplicationSourceAddress.test`, so that the program can name
  the address of its second interface.
- `*.hostA.app[0].timeToLive = 33` and `*.hostA.app[0].tos = 8`: two values a host would not
  choose by itself, so that the datagram cannot carry them by chance.

## Failure history during authoring

Pass 1: one initial failure, a test error, the `udp.destPort` filter on the drop signal
(deviation 2). Everything else passed on its first run.

Pass 2: two initial failures, both real and both kept. `Rfc1122MulticastSourceAddress.test`
and `Rfc768EmptyDatagram.test` failed as unexpected, were analysed, and became gaps 2 and 3
with an `%# expected-result: FAIL` marker. Nothing was reverted, skipped or softened. One
test error was found before the run, by reading the module: `Udp::processUdpPacket` pops the
UDP header before it records a drop for a wrong checksum and puts it back only on the path
where no program has the port, so a drop record cannot always be read by port. The three
checksum tests match the drop by the data length instead; see `droppedDatagramWithData` in
tests/protocol/udp/UdpMutations.h.

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
- **The discard of a wrong checksum (RFC1122-UCK-4):** `verifyChecksum`,
  [Udp.cc:996-1034](../../../../../src/inet/transportlayer/udp/Udp.cc#L996-L1034), sums the
  pseudo header, the header and the data in the computed mode and demands 0xFFFF; the drop
  that follows is at
  [Udp.cc:948-956](../../../../../src/inet/transportlayer/udp/Udp.cc#L948-L956), with the
  reason `INCORRECTLY_RECEIVED`, no answer, and the packet deleted. This is what three tests
  of pass 2 confirm from three sides: the value, the data and the pseudo header.
- **The pseudo header (RFC768-CKSUM-1):** `computeChecksum`,
  [Udp.cc:1036-1064](../../../../../src/inet/transportlayer/udp/Udp.cc#L1036-L1064), builds a
  `TransportPseudoHeader` of the two addresses, the protocol and the length, and sums it
  before the header and the data. The helper of the tests computes the same value
  independently, which is how `Rfc1122ChecksumDefault.test` can tell a real checksum from a
  placeholder.
- **The values a program names (RFC1122-UAPI-1, RFC1122-UMH-2):** the socket keeps the TTL,
  the TOS and the local address, and `Udp::processPacketFromApp` turns them into request
  tags that IPv4 reads; the datagram on the wire carried all three unchanged.
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
