# ARP checks — run results and model analysis (pass 1, level 3)

> **Kind:** report · **Status:** snapshot 2026-09-11 · **Seal:** none · **Owns:** — · **Stands on:** [rfc826/catalog.md](../../standard/rfc826/catalog.md), [rfc1122/catalog.md](../../standard/rfc1122/catalog.md), [rfc5494/catalog.md](../../standard/rfc5494/catalog.md), [checks.md](../../protocol/arp/checks.md)

Step 7 artifact of the standards test workflow. This is the first document of the ARP
workflow that may reference code.

- Date: 2026-09-11 10:50 +0200
- INET: branch `master`, commit `223ba89ce5`, tree clean
- OMNeT++: 6.4.0
- Build: debug, built from this commit
- Compiler: Ubuntu clang version 23.0.0 (++20260325083105+68994554ea12-1~exp1~20260325203127.404)
- Platform: Ubuntu 26.04.1 LTS, Linux 7.0.0-31-generic x86_64
- Command, after the `setenv` scripts of OMNeT++ and INET:

  ```sh
  cd tests/protocol/lib && MODE=debug ./build.sh
  inet_run_protocol_tests -p inet -w arp
  ```
- Suite result: 16 TOTAL, 13 PASS, **3 FAIL (unexpected)**, so the suite reports FAIL.

This is the first ARP pass. It targets level 3, so it did the work of levels 1, 2 and 3
together, and there is no earlier verdict to repeat.

## Verdicts

Level 2, observation only:

| Test | Checks | Verdict |
| --- | --- | --- |
| Rfc826AddressResolution.test | RFC826-REQ-1, REQ-4, REQ-6, RFC1122-AUSE-1; covers FMT-3, FMT-5, TABLE-2 | PASS |
| Rfc826CachedMapping.test | RFC826-REQ-2 | PASS |
| Rfc1122CacheFlush.test | RFC1122-ACACHE-1, ACACHE-2 | PASS |
| Rfc826ReplyFields.test | RFC826-RECV-8, RECV-9; covers FMT-6, TABLE-2 | PASS |
| Rfc826LearningFromRequest.test | RFC826-RECV-6, RECV-7 | PASS |
| Rfc826PacketLayout.test | RFC826-FMT-1, FMT-2, FMT-4, FMT-7, RECV-11, RFC5494-NUM-1, NUM-4 | PASS |
| Rfc826ThirdStationRequest.test | RFC826-RECV-5; covers RECV-1 | PASS |
| Rfc1122ArpPacketQueue.test | RFC1122-AQUEUE-1; covers RFC826-REQ-3 | PASS |
| Rfc1122ArpFloodPrevention.test | RFC1122-AFLOOD-1 | PASS |
| Rfc1122NoDestinationUnreachable.test | RFC1122-ANOERR-1 | PASS |

Level 3, a crafted packet:

| Test | Checks | Verdict |
| --- | --- | --- |
| Rfc826UnsolicitedReply.test | RFC826-RECV-10; covers RECV-1 | PASS |
| Rfc826SupersedingHardwareAddress.test | RFC826-TABLE-1, RECV-4 | PASS |
| Rfc826MergeBeforeOpcode.test | RFC826-RECV-7, RECV-6 | PASS |
| Rfc5494ExperimentalOpcode.test | RFC826-RECV-10, RFC5494-NUM-3 | FAIL, gap 1 |
| Rfc5494ExperimentalHardwareSpace.test | RFC826-RECV-2, RFC5494-NUM-2 | FAIL, gap 2 |
| Rfc826UnknownProtocolSpace.test | RFC826-RECV-3 | FAIL, gap 2 |

## The failures

All three failures are **defects**, and none is declared expected. No test error and no
specification misread came out of this pass; each test keeps its faithful assertion.

Reviewed against
[the third principle of the guide](../../../guide/derive-tests-from-a-standard.md#principle-a-claimed-feature-gets-a-test),
a failure is declarable only where the model does not claim the behavior, and a claim is code. All
three of these are code that exists and does the wrong thing:
`ArpPacketSerializer::deserializeFields` tests the hardware space and the protocol space and calls
`markIncorrect` when either fails (ArpPacketSerializer.cc:52 to 55), so the model **detects** both
conditions RFC 826 asks it to detect; and `Arp::processArpPacket` has a `default:` branch for an
opcode it does not know (Arp.cc:358). The detection and the branch are there, and what happens next
is a stop.

The three failures share one shape: RFC 826 asks for a silent discard, and the model stops
the run instead. The reception algorithm of RFC 826 states the rule once, at its head:

> "Negative conditionals indicate an end of processing and a discarding of the packet." —
> `rfc826.txt:200-201`

A station that stops is the strongest possible way to fail that sentence, and it is worse
than a wrong reply: a wrong reply is a packet a neighbour can ignore, while a stop takes the
station off the network.

### Gap 1 — an opcode the model does not know stops the run

`Arp::processArpPacket` reads the opcode in a `switch` whose default branch throws
([`src/inet/networklayer/arp/ipv4/Arp.cc:357-358`](../../../../../src/inet/networklayer/arp/ipv4/Arp.cc)):

```cpp
default:
    throw cRuntimeError("Unsupported opcode %d in received ARP packet", arp->getOpcode());
```

The run ends with `Error: Unsupported opcode 24 in received ARP packet -- in module
(inet::Arp) ArpSharedNet.hostB.ipv4.arp (id=128), at t=0.5s, event #2`. The two reverse
opcodes of RFC 903 have their own branches, and both throw as well
([`Arp.cc:351-355`](../../../../../src/inet/networklayer/arp/ipv4/Arp.cc)), which is a
deliberate statement that the model does not do RARP; the default branch is the one that
this check is about, because RFC 5494 §3 makes 24 and 25 legal values that a neighbour may
put on a link. RFC 826 wants the packet dropped after the table step, which the model has
already done by the time it reaches the `switch`.

### Gap 2 — a hardware space or a protocol space other than the Ethernet and IPv4 stops the run

The packet class of the model carries four fields of the layout of RFC 826 implicitly, and
its own comment says so
([`ArpPacket.msg:31-34`](../../../../../src/inet/networklayer/arp/ipv4/ArpPacket.msg)):

```
//   - hardwareType (not needed for modeling);
//   - protocol type (0x800 IPv4)
//   - hardware address length (6)
//   - protocol address length (4)
```

The serializer writes the four constants and reads them back, and it marks the packet
incorrect when either of the two spaces differs
([`ArpPacketSerializer.cc:51-54`](../../../../../src/inet/networklayer/arp/ipv4/ArpPacketSerializer.cc)):

```cpp
if (stream.readUint16Be() != 1)
    arpPacket->markIncorrect();
if (stream.readUint16Be() != ETHERTYPE_IPv4)
    arpPacket->markIncorrect();
```

`Arp::processArpPacket` then peeks the packet with the default flags
([`Arp.cc:231`](../../../../../src/inet/networklayer/arp/ipv4/Arp.cc)), and a peek of an
incorrect chunk is an error: `Error: Returning an incorrect chunk is not allowed according
to the flags: 0`. Both crafted packets end the run the same way, at the same line, which is
why the two tests name one gap.

The marking itself is right: the reception algorithm of RFC 826 asks exactly these two
questions first, and `markIncorrect` is the answer to both. What is missing is the discard.
The module never reads the mark, and no code path in it produces the "end of processing"
that RFC 826 states.

A related consequence of the same design: the hardware space and the protocol space are not
fields of the packet, so nothing in the model can put a second hardware type or a second
protocol on a link. RFC826-GEN-1 is therefore not checkable against this model at all, and
the ledger records it as `no check` with that reason.

## Where the model implements each checked behaviour

| Behaviour | Where |
| --- | --- |
| The table, and the lookup that starts a resolution | `Arp::resolveL3Address`, [`Arp.cc:390-423`](../../../../../src/inet/networklayer/arp/ipv4/Arp.cc). A miss inserts an entry and calls `initiateArpResolution`; a hit returns the hardware address; an entry older than `cacheTimeout` starts a new resolution. |
| The request and its field values | `Arp::sendArpRequest`, [`Arp.cc:143-177`](../../../../../src/inet/networklayer/arp/ipv4/Arp.cc). It reads its own two addresses from the interface, sets the opcode, and asks the link layer for the broadcast address through a `MacAddressReq` tag. |
| The reception algorithm | `Arp::processArpPacket`, [`Arp.cc:228-366`](../../../../../src/inet/networklayer/arp/ipv4/Arp.cc). The comment at [`Arp.cc:238-263`](../../../../../src/inet/networklayer/arp/ipv4/Arp.cc) quotes the algorithm of RFC 826, and the code follows it step by step, merge flag included. |
| The merge before the opcode | [`Arp.cc:273-281`](../../../../../src/inet/networklayer/arp/ipv4/Arp.cc) for the update of an existing entry, [`Arp.cc:286-304`](../../../../../src/inet/networklayer/arp/ipv4/Arp.cc) for the addition when the host is the target. Both are above the `switch` on the opcode. |
| The reply | [`Arp.cc:308-343`](../../../../../src/inet/networklayer/arp/ipv4/Arp.cc). It builds a new packet rather than reusing the request, and it addresses the frame to the sender hardware address of the request. |
| The target question | `Arp::addressRecognized`, [`Arp.cc:204-219`](../../../../../src/inet/networklayer/arp/ipv4/Arp.cc). It answers yes for a local address, and also for an address it can route out of another interface when proxy ARP is on. |
| The flush of an out-of-date entry | The same lifetime test in `Arp::resolveL3Address`, [`Arp.cc:413-421`](../../../../../src/inet/networklayer/arp/ipv4/Arp.cc), and in `Arp::getL3AddressFor`, [`Arp.cc:432-437`](../../../../../src/inet/networklayer/arp/ipv4/Arp.cc). |
| The request rate | `retryTimeout` (1 s) and `retryCount` (3) in [`Arp.ned:42-43`](../../../../../src/inet/networklayer/arp/ipv4/Arp.ned), driven by `Arp::requestTimedOut`, [`Arp.cc:179-202`](../../../../../src/inet/networklayer/arp/ipv4/Arp.cc). |
| The datagram that waits | `Ipv4::pendingPackets`, [`Ipv4.cc:1146-1165`](../../../../../src/inet/networklayer/ipv4/Ipv4.cc), emptied by `Ipv4::arpResolutionCompleted`, [`Ipv4.cc:1167-1186`](../../../../../src/inet/networklayer/ipv4/Ipv4.cc). |
| The silence after a failed resolution | `Ipv4::arpResolutionTimedOut`, [`Ipv4.cc:1188-1205`](../../../../../src/inet/networklayer/ipv4/Ipv4.cc). It records a packet drop with the reason `ADDRESS_RESOLUTION_FAILED` and sends nothing. |
| The octets of the packet | `ArpPacketSerializer::serializeFields`, [`ArpPacketSerializer.cc:34-46`](../../../../../src/inet/networklayer/arp/ipv4/ArpPacketSerializer.cc). The order, the byte order and the widths are those of RFC 826, and the two spaces are the constants of gap 2. |

Three observations about the model that no check turned into a verdict, and that the next
pass should know:

- **The queue exceeds the requirement, and it is in the wrong layer.** RFC 1122 §2.3.2.2
  asks the **link layer** to save **at least one** packet. The model keeps the queue in the
  network layer, one `cPacketQueue` per unresolved address
  ([`Ipv4.h:91`](../../../../../src/inet/networklayer/ipv4/Ipv4.h)), and it saves every
  datagram, not only the latest. Both differences favour the model: the requirement is a
  floor, and which layer holds the queue is invisible from a link.
- **Proxy ARP is on by default.** `proxyArpInterfaces` defaults to `"*"`
  ([`Arp.ned:45`](../../../../../src/inet/networklayer/arp/ipv4/Arp.ned)), so a node that can
  route answers for an address that is not its own. That contradicts RFC826-TABLE-2, "hosts
  don't transmit information about anyone other than themselves", and it is exactly what
  RFC 1027 defines. The checks of this pass do not meet it: a host with one interface never
  has another interface to route out of, so `addressRecognized` returns false, which is why
  Rfc826ThirdStationRequest passes. A router mockup would meet it, and the standards map
  files RFC 1027 at level 5.
- **Two senders have no callers.** `Arp::sendArpGratuitous`
  ([`Arp.cc:441-483`](../../../../../src/inet/networklayer/arp/ipv4/Arp.cc)) and
  `Arp::sendArpProbe` ([`Arp.cc:487-516`](../../../../../src/inet/networklayer/arp/ipv4/Arp.cc))
  are public methods that nothing in the tree calls, and neither is part of the `IArp`
  interface. The second one names RFC 5227 in its comment. They are the model's half-built
  answer to the document that this pass left at level 4.

## What the run measured

Two numbers are worth recording, because a later pass will compare against them.

- **The request rate.** In Rfc1122ArpFloodPrevention the sending program offered a datagram
  every 0.1 s for 10 s to an address nobody owns, and host A put **10 requests** on the link,
  at t = 0.1, 1.1, 2.1, … 9.1 s. That is one request per second exactly, which is the rate
  RFC 1122 §2.3.2.1 recommends. The bound of the check is 11. The mechanism is the retry
  timer, not a rate limiter: three retries one second apart, then the resolution fails
  (`arpResolutionFailed:count` is 3 for the run) and the next datagram starts a new one.
  A host that sent its datagrams faster than the retry timer would still be limited, because
  a second resolution for a pending address returns at once
  ([`Arp.cc:408-412`](../../../../../src/inet/networklayer/arp/ipv4/Arp.cc)).
- **The packet on the wire.** The ARP data of a request is **28 octets** and the frame that
  carries it is 46 octets before padding, with the Ethernet type 2054 (0x0806). The reply
  has the same 28 octets, which is RFC826-FMT-6.

## Sharpening candidates for the next pass

1. **Level 4, RFC 5227.** The document that the standards map holds back. The model already
   has `sendArpProbe` and `sendArpGratuitous` with no callers, so the first question of that
   pass is whether the model can probe at all. A second question waits there too: a received
   probe carries an all-zero sender protocol address, and `Arp::processArpPacket` throws on
   one ([`Arp.cc:268-271`](../../../../../src/inet/networklayer/arp/ipv4/Arp.cc)) — the same
   shape as gap 1 and gap 2, from a third place in the same method.
2. **A router mockup.** It would reach proxy ARP, `Arp::addressRecognized` with two
   interfaces, and the reply on the interface that received the request, which one link
   cannot separate from the link itself (see the notes of
   [Reply field values](../../protocol/arp/checks/reply.md#reply-field-values)).
3. **A module test for the discard.** Gap 1 and gap 2 are both a missing discard, and the
   test that finds them can only watch a run stop. A module test could assert the state of
   the table after each crafted packet, which is what a fixed model would leave unchanged.
4. **The lengths at the receiving side.** RFC826-RECV-11 is checked on a sent packet only.
   A crafted packet whose length fields disagree with its address fields would check the
   permission from the other side, and the model has no field to read them from, so the
   answer is already known.
