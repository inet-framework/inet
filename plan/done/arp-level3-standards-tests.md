# ARP standards tests — level 3

> **Kind:** how · **Status:** done · **Stands on:**
> [derive-tests-from-a-standard.md](../../doc/project/guide/derive-tests-from-a-standard.md),
> [arp/standards.md](../../doc/project/evidence/protocol/arp/standards.md)

ARP had no evidence tree. One test sat in `tests/protocol/arp/` — `ArpResolution.test` — and
it came from the framework demo, not from a standard. This plan ran the whole workflow for
ARP at target level 3, so the pass did the work of levels 1, 2 and 3 in one go. The work is
on `master`, in the worktree `inet-master`.

## The in-scope set, and why

The RFC-editor metadata of RFC 826 names two documents that update it: RFC 5227 and
RFC 5494. RFC 1122 states the host requirements for ARP in §2.3.2. The level table of the
guide puts the host-requirements document at level 3 and the timer documents at level 4.
That gives:

| Document | Relation | In scope | Reason |
| --- | --- | --- | --- |
| RFC 826 | `base` | yes | the protocol |
| RFC 1122 §2.3.2, the ARP paragraph of §2.3.3, the third paragraph of §2.4 | `companion` | yes | the host requirements: cache flush, flood prevention, the packet queue, no error report |
| RFC 5494 | `updates` | yes | the reserved and the experimental values of `ar$hrd` and `ar$op`; it makes a crafted opcode a defined edge case |
| RFC 5227 | `updates` | no | the address conflict detection is a control loop with five timers (PROBE_WAIT, PROBE_NUM, ANNOUNCE_WAIT, ANNOUNCE_INTERVAL, DEFEND_INTERVAL). The guide files timers and control loops at level 4 |
| RFC 903 | separate protocol | no | RARP is its own protocol |
| RFC 1027 | `companion` | no | proxy ARP over a subnet gateway; level 5 |
| RFC 1868 | experimental | no | UNARP was never a standards-track update |

RFC 826 predates RFC 2119. It carries two `must` words and no other keyword, so most of its
catalog is `description`, and the level of a feature comes from the "only path" rule of the
guide.

**Decided during the work:** the in-scope set names the exact clauses of RFC 1122 and not
three whole sections. §2.3.3 and §2.4 each hold one paragraph about address translation and
several about the Ethernet encapsulation and the interface between IP and the link layer. A
pass on the Ethernet link layer owns those. Without the boundary the level 3 exit criterion
would count encapsulation MUSTs that this pass never meant to test.

## What level 3 added for ARP

Level 2 for ARP is the normal exchange: a request goes out broadcast, the target answers
directly, the datagram follows, and the second datagram needs no request. Level 3 needs a
crafted packet, which no application can produce:

- a request whose target protocol address is a third host — this one turned out to be
  level 2 after all, once the mockup gained a third station and a switch;
- an unsolicited reply;
- a packet with the experimental opcode 24 of RFC 5494;
- a packet with the experimental hardware space 36, and one with the protocol space of IPv6;
- a second request from the same protocol address with a different hardware address, and a
  reply from a station that is not there.

The level 3 toolset is therefore **injection**. No check needed the `PacketTap`: ARP packets
are short-lived and a crafted one is easier to build than to intercept.

## Steps

- [x] **Step 1 — download.** `standard/rfc826/rfc826.txt`, `standard/rfc5494/rfc5494.txt`.
      RFC 1122 was cached already, from the IPv4 pass.
- [x] **Step 2 — standards map.** `protocol/arp/standards.md`: seven documents, six override
      rows, the in-scope set, and the target level.
- [x] **Step 3 — catalogs.** `standard/rfc826/catalog.md` (28 entries) and
      `standard/rfc5494/catalog.md` (5 entries), both new. `standard/rfc1122/catalog.md`
      gained six entries whose identifiers start with `A`; its scope statement, its index
      note and its out-of-scope section now name three protocols.
- [x] **Step 4 — features.** `protocol/arp/features.md`: 9 features, 7 mandatory.
- [x] **Step 5 — checks.** `protocol/arp/checks.md` plus eight files under
      `protocol/arp/checks/`: 16 checks, 3 mockups, 4 statements with a stated reason for
      having no check.
- [x] **Step 6 — tests.** 16 tests in `tests/protocol/arp/`, plus `ArpMutations.h`.
      `ArpResolution.test` is gone and `Rfc826AddressResolution.test` replaces it; the
      cookbook heading of `lib/AUTHORING.md` keeps its snippet and loses the file name.
- [x] **Step 7 — results.** `model/arp/results.md`, with the run record and two model gaps.
- [x] **Step 8 — claims and conformance.** `model/arp/conformance.md`.
- [x] **Step 9 — categories.** `model/arp/categories.md`.
- [x] **Ledger.** `model/arp/coverage.md` and `model/arp/notes.md`.
- [x] **Commit.** Seven commits, one per step or step group, and the links and anchors
      checked.

## The tests

Sixteen tests, and the verdict each one reached:

| Test | Checks | Verdict |
| --- | --- | --- |
| Rfc826AddressResolution | RFC826-REQ-1, REQ-4, REQ-6, RFC1122-AUSE-1 | PASS |
| Rfc826CachedMapping | RFC826-REQ-2 | PASS |
| Rfc1122CacheFlush | RFC1122-ACACHE-1, ACACHE-2 | PASS |
| Rfc826ReplyFields | RFC826-RECV-8, RECV-9 | PASS |
| Rfc826LearningFromRequest | RFC826-RECV-6, RECV-7 | PASS |
| Rfc826PacketLayout | RFC826-FMT-1, FMT-2, FMT-4, FMT-7, RECV-11, RFC5494-NUM-1, NUM-4 | PASS |
| Rfc826ThirdStationRequest | RFC826-RECV-5 | PASS |
| Rfc1122ArpPacketQueue | RFC1122-AQUEUE-1 | PASS |
| Rfc1122ArpFloodPrevention | RFC1122-AFLOOD-1 | PASS |
| Rfc1122NoDestinationUnreachable | RFC1122-ANOERR-1 | PASS |
| Rfc826UnsolicitedReply | RFC826-RECV-10 | PASS |
| Rfc826SupersedingHardwareAddress | RFC826-TABLE-1, RECV-4 | PASS |
| Rfc826MergeBeforeOpcode | RFC826-RECV-7, RECV-6 | PASS |
| Rfc5494ExperimentalOpcode | RFC826-RECV-10, RFC5494-NUM-3 | FAIL, gap 1 |
| Rfc5494ExperimentalHardwareSpace | RFC826-RECV-2, RFC5494-NUM-2 | FAIL, gap 2 |
| Rfc826UnknownProtocolSpace | RFC826-RECV-3 | FAIL, gap 2 |

## Facts found during the work

- **`ArpPacket` has no hardware space, protocol space or length fields.** The serializer
  writes the four constants and reads them back
  (`ArpPacketSerializer.cc:34-46`). A check that crafts or reads one of them must work in
  octets, so `ArpMutations.h` carries two builders and an `arpOctets` reader. RFC826-GEN-1
  is unreachable against this model for the same reason.
- **`Arp::processArpPacket` ends the run on four kinds of input** it cannot handle: an
  unspecified sender hardware address, an unspecified sender protocol address, an opcode
  outside the four it knows, and a packet the serializer marked incorrect. RFC 826 asks for a
  discard in one sentence at the head of its reception algorithm. The pass tested two of the
  four; the second is an ARP Probe and belongs to level 4 with RFC 5227.
- **The model exceeds RFC 1122 twice.** It saves every datagram that waits, not only the
  latest, and it keeps the queue in the network layer and not the link layer
  (`Ipv4.h:91`). And the request rate lands on one per second exactly — 10 requests in a
  10-second run — which is the recommended maximum.
- **Proxy ARP is on by default** (`Arp.ned:45`), which contradicts RFC826-TABLE-2 for a node
  that can route. No check of this pass meets it, because a host with one interface never has
  a second interface to route out of.
- **The model claims nothing where a reader would look.** The documentation comment of the
  `Arp` module names no standard; the two RFC 826 citations are source comments, one of them
  hedged as "a'la". Four features are `undocumented` in the matrix.
- **A protocol name in a content expression resolves to the last chunk of that protocol.**
  `ethernetmac.dest` lands on the frame check sequence. Use the chunk class name,
  `EthernetMacHeader.dest`. This cost the first debugging cycle of the pass, and the guide now
  carries the rule among its expression pitfalls.
- **A greedy step consumes its whole window,** so a positive observation inside it cannot be
  a later step. Three tests moved such an observation to a line the receiving program prints
  at the end of the run, with a deviation note; two more reordered their steps.
- **opp_test calls a run that stops a FAIL and not an ERROR,** because no verdict line is
  printed. The three gap tests therefore declare `expected-result: FAIL`.

## Result

**Level 3, reached.** 16 tests in the suite: 13 PASS and 3 declared FAIL, naming two model
gaps. Every mandatory statement of the in-scope set has a check that ran and passed; the
three failures are against statements whose strength is `description`, which is the strength
of nearly every sentence of RFC 826. Nine features: seven supported, one partial, one
untested. Four of the 39 catalog statements carry `no check`, each with a reason, and none of
them is mandatory.

Every other protocol suite of the same run is unchanged: 10 suites, 250 tests, all PASS at
the suite level.

The full account is in `doc/project/evidence/model/arp/`.
