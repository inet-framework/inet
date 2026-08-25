# IPv4 with ARP and ICMP — conformance review

- **Family:** IPv4 with ARP and ICMP
- **Directories:** `src/inet/networklayer/ipv4`, `src/inet/networklayer/arp/ipv4`
- **Specs:** see [spec-manifest.md](spec-manifest.md)
- **Reviewed at:** `7b0a6de5cef3` (2026-08-25)
- **Workflow:** v2 (2026-08-25). Round 1 ran under v1. Round 2 added the requirements v1 never
  enumerated, and ran under v2 throughout.
- **Method:** 221 feature-level requirements enumerated from 15 documents by six agents denied
  access to the source. 42 further requirements added by a deep pass over the serializers, the
  timers, and the constants. 70 `Missing` and `Partial` claims then put to an adversarial
  refutation pass: 58 confirmed, 3 refuted, 7 weakened, 2 miscited. 8 distinct suspected errors,
  all 8 reproduced in a simulation before being graded `Incorrect`.

## 1. Scope statement

Approved at the P2 checkpoint on 2026-08-25. This section decides what counts as a gap. A
requirement outside this scope is reported as `Non-gap`, not as a defect.

### What this family models

The IPv4 network layer of a simulated host or router, in enough detail that transport protocols
above it and link layers below it see realistic datagram service.

| Component | Module | What it covers |
|---|---|---|
| IP forwarding and delivery | `Ipv4` | Sending from the transport layer, receiving, forwarding, local delivery, TTL handling, fragmentation and reassembly, multicast forwarding |
| Routing table | `Ipv4RoutingTable` | Longest-prefix unicast lookup, multicast routes, netmask routes, router id, administrative distance |
| Address resolution | `Arp`, `GlobalArp` | Request and reply exchange, a cache with a timeout, retries, proxy ARP |
| Error and diagnostic messages | `Icmp` | Error message generation for the network and transport layers, echo request and reply |
| Wire format | `Ipv4Header`, `ArpPacket`, `IcmpHeader` | Typed header definitions, plus a registered serializer, dissector, and printer for each |

### Declared fidelity

- **The header is a typed object, and it also serializes to real bytes.** Both representations
  exist. `checksumMode` selects between a declared checksum and a computed one, so a study can
  trade fidelity against speed.
- **Options are partially represented.** `Ipv4OptionType` enumerates the standard option types.
  Typed classes exist for No-Operation, End-of-Options, Record Route, Timestamp, Stream ID, and
  Router Alert. An unrecognized option is carried as `Ipv4OptionUnknown`.
- **ICMP carries typed classes for the echo pair and for Packet Too Big.** Other message types use
  the base `IcmpHeader` with a type and code.
- **Processing time is a single per-packet delay,** not a model of forwarding hardware.
- **The quoted portion of an ICMP error is a parameter,** `quoteLength`, default 8 bytes.

### Deliberately outside this family

- **IGMP.** `Igmpv2` and `Igmpv3` live in the same directory, and have 13 module tests. Excluded
  at the P0 checkpoint. They become their own family, against RFC 2236 and RFC 3376.
- **NAT.** `Ipv4NatTable` is excluded at the P0 checkpoint. RFC 3022 is a different problem.
- **IPsec.** `Ipv4NetworkLayer` can include an `IPsec` submodule. That is its own family.
- **Address configuration.** `Ipv4NodeConfigurator` and the network configurator assign addresses
  and routes. They implement no standard, and they have their own tests.
- **Routing protocols.** OSPF, RIP, PIM, and the rest populate the routing table from outside.
  This review judges the table's lookup semantics, not how routes arrive.

### Known intent that limits conformance

These are stated here so the review does not report them as surprises. Whether each one is
acceptable is a P8 decision, not a P2 one.

- INET models a **simulation** host, not a production stack. A requirement whose only purpose is
  interoperability with real hardware may be a `Non-gap`.
- The `Ipv4` module is a single simple module. The standard's split between a host and a router is
  expressed by the `forwarding` parameter, not by two implementations.

### Source of this statement

Read from `Ipv4.ned`, `Icmp.ned`, `Arp.ned`, `Ipv4RoutingTable.ned`, `Ipv4NetworkLayer.ned`,
`Ipv4Header.msg`, `IcmpHeader.msg`, and `ArpPacket.msg`.

`doc/src/developers-guide/ch-ipv4.rst` was read, and **it is out of date**. It documents a
`procDelay` parameter, a `QueueBase` base class, an `ICMPAccess` accessor, and `errOut` and
`pingIn` gates. None of these appear in the current NED or C++. The scope statement above follows
the code, not the chapter. Refreshing that chapter is separate work, and is recorded in section 6.

## 2. Summary

INET's IPv4 family implements the datagram path well and the receive-side checks badly. Of 414
enumerated requirements, 194 are conformant. The header format, the fragmentation arithmetic, and
the core of RFC 826 hold up. The receive path, the option machinery, and the routing table do not. The header format, the forwarding decision, the
fragmentation arithmetic, and the core of RFC 826 all hold up under a clause-by-clause read.

The largest single problem is that **the receive path does not validate**. [`IPV4-002`](#ipv4-002) is one
mistyped operator at `Ipv4.cc:282` that disables three checks at once: the header checksum, the
version field, and the header length. A test injected five malformed datagrams — one with a bad
checksum, one with version 6, one with a 4-word header — and a receiving application accepted all
five. Three more findings ([`IPV4-107`](#ipv4-107), [`IPV4-115`](#ipv4-115), [`IPV4-119`](#ipv4-119)) show the same shape from the
serializer side: the deserializers mark a chunk incorrect or improperly represented, and no caller
ever asks.

Second, **three legal packets abort the simulation.** A Source Quench, an unassigned ICMP type, and
an RFC 5227 ARP Probe each reach a `cRuntimeError` ([`IPV4-065`](#ipv4-065), [`IPV4-066`](#ipv4-066), [`IPV4-087`](#ipv4-087)). The
standard requires a silent discard in all three cases. INET's own dormant `Arp::sendArpProbe` builds
the packet that crashes it.

Third, **IP options are carried but not acted on.** They serialize correctly and they do reach the
transport layer, but no node inserts a Record Route hop, follows a source route, or stamps a
timestamp while forwarding.

If only one thing is fixed, fix [`IPV4-002`](#ipv4-002). It is a one-line change that restores three mandatory
checks, and no simulation today can rely on a checksum meaning anything.

### Verification coverage

What was checked, and what was not. Read this before the counts below.

| Status | Rows | Verified | By what | Unverified |
|---|---|---|---|---|
| `Incorrect` | 22 | 22 | reproduced in a simulation (P6) | 0 |
| `Missing`, `Partial` | 149 | 151 | adversarial refutation (P7), both rounds | 0 |
| `Full` | 194 | 52 | blind duplication (P7b), round-1 two-obligation rows | 142 |
| `Non-gap` | 46 | 12 | blind duplication (P7b), round-1 rows | 34 |

Every row also passed a mechanical check before assembly: each cited line exists in its file, each
two-obligation clause carries a verdict for both halves, and no agent-assigned status breaks the
evidence rule. That check found three fabricated line citations, all corrected.

**Enumeration.** Round 1 enumerated 221 requirements. A completeness critic then read the same
documents against that matrix and found 193 more with no row — round 1 was about 52% complete, and
only 37% for RFC 1122 and RFC 1812. The cause was a row target in the enumeration prompt, which acted
as a ceiling rather than a guide. Those 193 are round 2, and they are in the matrix below.

Two verification passes measured their own yield:

- The **false-`Full` hunt** changed 9 of the 68 round-1 rows it audited by blind duplication — a 13%
  error rate in the population round 1 never checked. Seven of the nine were the same failure: a
  clause imposing two obligations, graded on one.
- The **refutation pass** changed 17% of round-1 claims and 31% of round-2 claims. Round 2's higher
  rate reflects less familiar ground, not a worse method.

**The enumeration has not converged.** A second completeness critic read all fifteen documents
against the combined 414-row matrix and found about 26 more requirements with no row — including
whole sections both rounds skipped: RFC 1812 §5.3.9 and §5.3.12, and the RFC 1122 §3.2.2.8 recipient
duties. It is still finding sections, not stragglers. A third round is warranted, and until it runs
**the absence of a row remains no evidence at all**.

(That pass reported 35, of which about 9 are false: three of the fifteen documents were mistakenly
left out of the critics' input, so their eleven existing rows were invisible to it.)

Round 2 graded 37% of its rows `Full`, against round 1's 56%. Part of that is the per-obligation
`checked:` field working as intended; part is that round 1 enumerated the requirements easiest to
see, and easy to enumerate correlates with easy to implement. The two cannot be separated here.

### Counts by status and impact

| | `results` | `scope` | `cosmetic` | total |
|---|---|---|---|---|
| `Incorrect` | 21 | 1 | 0 | 22 |
| `Missing` | 18 | 59 | 6 | 83 |
| `Partial` | 20 | 38 | 8 | 66 |
| **total** | **59** | **98** | **14** | **171** |

`Full` 194. `Non-gap` 46. Of 414 requirements, **47% are conformant**. Three further rows are proposed as deliberate simplifications and await
your decision — see section 8.

The deep pass over the serializers and constants adds 42 findings, numbered from [`IPV4-097`](#ipv4-097).

### Where the gaps concentrate

| Area | Requirements | Conformant | Weakest point |
|---|---|---|---|
| Header, checksum, addressing | 38 | 27 | the validation guard, [`IPV4-002`](#ipv4-002) |
| Fragmentation, TTL, path MTU | 47 | 34 | reassembly has no timer, [`IPV4-012`](#ipv4-012) |
| IP options | 37 | 12 | nothing processes an option |
| ICMP | 52 | 26 | four message types unimplemented, two aborts |
| ARP and conflict detection | 33 | 20 | [RFC 5227](https://www.rfc-editor.org/rfc/rfc5227) absent, [`IPV4-077`](#ipv4-077) to [`IPV4-087`](#ipv4-087) |
| Router forwarding | 14 | 5 | no martian filtering, no Redirect |

## 3. Coverage matrix

Every requirement appears, including the conformant ones. A `Full` row carries no identifier.

Confidence is `h` high or `m` medium. No row rests on low confidence: a row that could not be
settled was downgraded in status rather than reported at low confidence.

Everything in this report is a link. An identifier jumps to its matrix row, and a matrix identifier
jumps on to the finding that explains it. A spec clause opens the document at that clause. An
evidence reference opens the file at that line. Nothing here should need a search to check.

### 3a. IP header fields, checksum, and addressing

38 requirements. 27 `Full`, 2 `Incorrect`, 2 `Missing`, 5 `Non-gap`, 2 `Partial`.

| ID | Feature | Spec clause | Level | Status | Impact | Conf. | Evidence |
|---|---|---|---|---|---|---|---|
| <a id="ipv4-001"></a>IPV4-001 | IP version validation | [RFC 1122 §3.2.1.1](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.1.1) | MUST | `Partial` | scope | m | [networklayer/ipv4/Ipv4HeaderSerializer.cc:178](../../../src/inet/networklayer/ipv4/Ipv4HeaderSerializer.cc#L178); [networklayer/ipv4/Ipv4.cc:268-283](../../../src/inet/networklayer/ipv4/Ipv4.cc#L268) |
| <a id="ipv4-002"></a>[IPV4-002](#f1) | IP header checksum verification | [RFC 1122 §3.2.1.2](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.1.2) | MUST | `Incorrect` | results | h | [networklayer/ipv4/Ipv4.cc:282](../../../src/inet/networklayer/ipv4/Ipv4.cc#L282); [networklayer/ipv4/Ipv4Header.cc:97-113](../../../src/inet/networklayer/ipv4/Ipv4Header.cc#L97) |
| — | Own address as IP source | [RFC 1122 §3.2.1.3](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.1.3) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:1063-1068](../../../src/inet/networklayer/ipv4/Ipv4.cc#L1063); [networklayer/ipv4/Ipv4.cc:922-926](../../../src/inet/networklayer/ipv4/Ipv4.cc#L922) |
| — | Destination address validation | [RFC 1122 §3.2.1.3](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.1.3) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:355-420](../../../src/inet/networklayer/ipv4/Ipv4.cc#L355) |
| <a id="ipv4-003"></a>IPV4-003 | Source address validation | [RFC 1122 §3.2.1.3](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.1.3) | MUST | `Missing` | scope | h | [networklayer/ipv4/Ipv4.cc:268-315](../../../src/inet/networklayer/ipv4/Ipv4.cc#L268); [networklayer/ipv4/Ipv4.cc:340-420](../../../src/inet/networklayer/ipv4/Ipv4.cc#L340) |
| — | Transport control of TOS | [RFC 1122 §3.2.1.6](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.1.6) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:1072-1087](../../../src/inet/networklayer/ipv4/Ipv4.cc#L1072); [networklayer/ipv4/Ipv4Header.msg:176](../../../src/inet/networklayer/ipv4/Ipv4Header.msg#L176); tests/module/UDP_… |
| <a id="ipv4-004"></a>IPV4-004 | Response source address selection | [RFC 1122 §3.3.4.2](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.4.2) | SHOULD | `Partial` | results | m | Icmp.cc:143-158 and 215, not 214 |
| — | Link-layer broadcast destination address | [RFC 1122 §3.3.6](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.6) | MUST | `Full` | -- | m | [networklayer/ipv4/Ipv4.cc:1205-1218](../../../src/inet/networklayer/ipv4/Ipv4.cc#L1205) |
| <a id="ipv4-005"></a>IPV4-005 | Link-layer-only broadcast discard | [RFC 1122 §3.3.6](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.6) | SHOULD | `Missing` | scope | m | [networklayer/ipv4/Ipv4.cc:268-315](../../../src/inet/networklayer/ipv4/Ipv4.cc#L268) |
| — | Version field value | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4Header.msg:174](../../../src/inet/networklayer/ipv4/Ipv4Header.msg#L174); [networklayer/ipv4/Ipv4HeaderSerializer.cc:26](../../../src/inet/networklayer/ipv4/Ipv4HeaderSerializer.cc#L26) |
| — | Internet header length field | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4Header.msg:175](../../../src/inet/networklayer/ipv4/Ipv4Header.msg#L175); [networklayer/ipv4/Ipv4HeaderSerializer.cc:23-25](../../../src/inet/networklayer/ipv4/Ipv4HeaderSerializer.cc#L23); netwo… |
| <a id="ipv4-006"></a>IPV4-006 | Type of service field layout | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | MUST | `Non-gap` | -- | h | [networklayer/ipv4/Ipv4Header.msg:176-179](../../../src/inet/networklayer/ipv4/Ipv4Header.msg#L176); [networklayer/ipv4/Ipv4Header.cc:50-68](../../../src/inet/networklayer/ipv4/Ipv4Header.cc#L50) |
| <a id="ipv4-007"></a>IPV4-007 | Limited service indications | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | SHOULD | `Non-gap` | -- | h | [networklayer/ipv4/Ipv4Header.msg:176-179](../../../src/inet/networklayer/ipv4/Ipv4Header.msg#L176) |
| — | Reserved flag bit | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4Header.msg:184](../../../src/inet/networklayer/ipv4/Ipv4Header.msg#L184); [networklayer/ipv4/Ipv4HeaderSerializer.cc:31-32](../../../src/inet/networklayer/ipv4/Ipv4HeaderSerializer.cc#L31) |
| — | Protocol field | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4Header.msg:190](../../../src/inet/networklayer/ipv4/Ipv4Header.msg#L190); [networklayer/ipv4/Ipv4.cc:1049](../../../src/inet/networklayer/ipv4/Ipv4.cc#L1049) |
| — | Header checksum computation | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4Header.cc:70-95](../../../src/inet/networklayer/ipv4/Ipv4Header.cc#L70); [common/checksum/Checksum.cc:167-185](../../../src/inet/common/checksum/Checksum.cc#L167); tests/protocol/C… |
| <a id="ipv4-008"></a>[IPV4-008](#f1) | Discard on checksum failure | [RFC 791 §1.4](https://www.rfc-editor.org/rfc/rfc791#section-1.4) | MUST | `Incorrect` | results | h | [networklayer/ipv4/Ipv4.cc:282](../../../src/inet/networklayer/ipv4/Ipv4.cc#L282) |
| — | Source address field | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4Header.msg:194](../../../src/inet/networklayer/ipv4/Ipv4Header.msg#L194); [networklayer/ipv4/Ipv4HeaderSerializer.cc:40](../../../src/inet/networklayer/ipv4/Ipv4HeaderSerializer.cc#L40) |
| — | Destination address field | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4Header.msg:196](../../../src/inet/networklayer/ipv4/Ipv4Header.msg#L196); [networklayer/ipv4/Ipv4HeaderSerializer.cc:41](../../../src/inet/networklayer/ipv4/Ipv4HeaderSerializer.cc#L41); networkl… |
| <a id="ipv4-009"></a>IPV4-009 | Address class formats | [RFC 791 §3.2](https://www.rfc-editor.org/rfc/rfc791#section-3.2) | MUST | `Non-gap` | -- | h | [networklayer/contract/ipv4/Ipv4Address.h:198](../../../src/inet/networklayer/contract/ipv4/Ipv4Address.h#L198); [networklayer/ipv4/Ipv4RoutingTable.cc:365](../../../src/inet/networklayer/ipv4/Ipv4RoutingTable.cc#L365) |
| <a id="ipv4-010"></a>IPV4-010 | Reserved address forms | [RFC 791 §3.2](https://www.rfc-editor.org/rfc/rfc791#section-3.2) | MUST | `Non-gap` | -- | m | [networklayer/contract/ipv4/Ipv4Address.h:165](../../../src/inet/networklayer/contract/ipv4/Ipv4Address.h#L165); [networklayer/ipv4/Ipv4RoutingTable.cc:365](../../../src/inet/networklayer/ipv4/Ipv4RoutingTable.cc#L365) |
| <a id="ipv4-011"></a>IPV4-011 | Sending oversized datagrams | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | SHOULD | `Partial` | scope | m | [networklayer/ipv4/Ipv4.cc:963-975](../../../src/inet/networklayer/ipv4/Ipv4.cc#L963); [networklayer/ipv4/Icmp.cc:158-186](../../../src/inet/networklayer/ipv4/Icmp.cc#L158) |
| — | Source address validation | [RFC 791 §3.3](https://www.rfc-editor.org/rfc/rfc791#section-3.3) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:1063-1068](../../../src/inet/networklayer/ipv4/Ipv4.cc#L1063) |
| — | Unknown user notification | [RFC 791 §3.3](https://www.rfc-editor.org/rfc/rfc791#section-3.3) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:887-890](../../../src/inet/networklayer/ipv4/Ipv4.cc#L887); [tests/module/IPv4_ICMPerror_NoProtocol.test](../../../tests/module/IPv4_ICMPerror_NoProtocol.test) |
| — | DS field replaces the TOS octet | [RFC 2474 §3](https://www.rfc-editor.org/rfc/rfc2474#section-3) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4Header.msg:178-179](../../../src/inet/networklayer/ipv4/Ipv4Header.msg#L178); [networklayer/ipv4/Ipv4Header.cc:50-68](../../../src/inet/networklayer/ipv4/Ipv4Header.cc#L50) |
| — | Six-bit DSCP matching | [RFC 2474 §3](https://www.rfc-editor.org/rfc/rfc2474#section-3) | MUST | `Full` | -- | m | [networklayer/diffserv/BehaviorAggregateClassifier.cc:112-123](../../../src/inet/networklayer/diffserv/BehaviorAggregateClassifier.cc#L112); networklayer/ipv4/Ipv4Header.c… |
| — | CU bits ignored in PHB selection | [RFC 2474 §3](https://www.rfc-editor.org/rfc/rfc2474#section-3) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4Header.cc:50-52](../../../src/inet/networklayer/ipv4/Ipv4Header.cc#L50); [networklayer/diffserv/BehaviorAggregateClassifier.cc](../../../src/inet/networklayer/diffserv/BehaviorAggregateClassifier.cc):… |
| — | Configurable codepoint to PHB mapping | [RFC 2474 §3](https://www.rfc-editor.org/rfc/rfc2474#section-3) | MUST | `Full` | -- | m | [networklayer/diffserv/BehaviorAggregateClassifier.ned:23](../../../src/inet/networklayer/diffserv/BehaviorAggregateClassifier.ned#L23); networklayer/diffserv/BehaviorAggr… |
| — | Default PHB availability | [RFC 2474 §4.1](https://www.rfc-editor.org/rfc/rfc2474#section-4.1) | MUST | `Full` | -- | m | [networklayer/diffserv/BehaviorAggregateClassifier.cc:96-111](../../../src/inet/networklayer/diffserv/BehaviorAggregateClassifier.cc#L96); [networklayer/diffserv/Dscp.msg:17](../../../src/inet/networklayer/diffserv/Dscp.msg#L17) |
| — | ECN field in the DS byte | [RFC 3168 §5](https://www.rfc-editor.org/rfc/rfc3168#section-5) | MUST | `Full` | -- | h | [networklayer/common/EcnTag.msg:13-16](../../../src/inet/networklayer/common/EcnTag.msg#L13); [networklayer/ipv4/Ipv4Header.cc:60-68](../../../src/inet/networklayer/ipv4/Ipv4Header.cc#L60) |
| — | CE marking only in place of dropping | [RFC 3168 §5](https://www.rfc-editor.org/rfc/rfc3168#section-5) | SHOULD | `Full` | -- | m | [queueing/filter/RedDropper.cc:105-116](../../../src/inet/queueing/filter/RedDropper.cc#L105); [queueing/filter/RedDropper.cc:121-143](../../../src/inet/queueing/filter/RedDropper.cc#L121) |
| — | No CE marking for non-congestion drops | [RFC 3168 §5](https://www.rfc-editor.org/rfc/rfc3168#section-5) | MUST | `Full` | -- | m | [queueing/filter/RedDropper.cc:121-143](../../../src/inet/queueing/filter/RedDropper.cc#L121); [queueing/marker/EcnMarker.cc:30-37](../../../src/inet/queueing/marker/EcnMarker.cc#L30) |
| — | CE never cleared by a router | [RFC 3168 §12](https://www.rfc-editor.org/rfc/rfc3168#section-12) | MUST | `Full` | -- | m | [networklayer/ipv4/Ipv4.cc:320-325](../../../src/inet/networklayer/ipv4/Ipv4.cc#L320); [queueing/marker/EcnMarker.cc:39-72](../../../src/inet/queueing/marker/EcnMarker.cc#L39) |
| — | ECT only when loss signals congestion | [RFC 3168 §5.2](https://www.rfc-editor.org/rfc/rfc3168#section-5.2) | MUST | `Full` | -- | m | [transportlayer/tcp/TcpConnectionUtil.cc:317](../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L317) |
| — | Internet checksum definition | [RFC 1071 §1](https://www.rfc-editor.org/rfc/rfc1071#section-1) | MAY | `Full` | -- | h | [common/checksum/Checksum.cc:167-185](../../../src/inet/common/checksum/Checksum.cc#L167); [tests/unit/Checksum_1.test](../../../tests/unit/Checksum_1.test) |
| — | Order-independent checksum computation | [RFC 1071 §2](https://www.rfc-editor.org/rfc/rfc1071#section-2) | MAY | `Full` | -- | m | [common/checksum/Checksum.h:27,130](../../../src/inet/common/checksum/Checksum.h#L27); [common/checksum/Checksum.cc:167-185](../../../src/inet/common/checksum/Checksum.cc#L167) |
| — | Incremental checksum update equation | [RFC 1624 §3](https://www.rfc-editor.org/rfc/rfc1624#section-3) | MAY | `Full` | -- | m | [queueing/marker/EcnMarker.cc:61-67](../../../src/inet/queueing/marker/EcnMarker.cc#L61); [networklayer/ipv4/Ipv4Header.cc:70-95](../../../src/inet/networklayer/ipv4/Ipv4Header.cc#L70) |
| — | Avoiding the negative zero result | [RFC 1624 §3](https://www.rfc-editor.org/rfc/rfc1624#section-3) | MAY | `Full` | -- | m | [networklayer/ipv4/Ipv4Header.cc:81-91](../../../src/inet/networklayer/ipv4/Ipv4Header.cc#L81); [common/checksum/Checksum.cc:180-184](../../../src/inet/common/checksum/Checksum.cc#L180) |

### 3b. Fragmentation, reassembly, TTL, and path MTU

47 requirements. 34 `Full`, 1 `Incorrect`, 4 `Missing`, 1 `Non-gap`, 7 `Partial`.

| ID | Feature | Spec clause | Level | Status | Impact | Conf. | Evidence |
|---|---|---|---|---|---|---|---|
| — | Zero TTL transmission prohibited | [RFC 1122 §3.2.1.7](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.1.7) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:1093-1103](../../../src/inet/networklayer/ipv4/Ipv4.cc#L1093); [networklayer/ipv4/Ipv4.cc:943](../../../src/inet/networklayer/ipv4/Ipv4.cc#L943) |
| — | Low TTL datagram acceptance | [RFC 1122 §3.2.1.7](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.1.7) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:388-389](../../../src/inet/networklayer/ipv4/Ipv4.cc#L388); [networklayer/ipv4/Ipv4.cc:943](../../../src/inet/networklayer/ipv4/Ipv4.cc#L943) |
| — | Minimum reassembly size | [RFC 1122 §3.3.2](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.2) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4FragBuf.cc:33-95](../../../src/inet/networklayer/ipv4/Ipv4FragBuf.cc#L33); [common/packet/ReassemblyBuffer.h:23-60](../../../src/inet/common/packet/ReassemblyBuffer.h#L23); tests/module… |
| <a id="ipv4-012"></a>[IPV4-012](#f3) | Reassembly timeout | [RFC 1122 §3.3.2](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.2) | MUST | `Partial` | results | h | [networklayer/ipv4/Ipv4.cc:832-836](../../../src/inet/networklayer/ipv4/Ipv4.cc#L832); [networklayer/ipv4/Ipv4FragBuf.cc:97-127](../../../src/inet/networklayer/ipv4/Ipv4FragBuf.cc#L97) |
| <a id="ipv4-013"></a>[IPV4-013](#f3) | Time Exceeded on reassembly timeout | [RFC 1122 §3.3.2](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.2) | MUST | `Incorrect` | results | h | [networklayer/ipv4/Ipv4FragBuf.cc:118](../../../src/inet/networklayer/ipv4/Ipv4FragBuf.cc#L118) |
| — | Fixed reassembly timeout value | [RFC 1122 §3.3.2](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.2) | SHOULD | `Full` | -- | h | [networklayer/ipv4/Ipv4.ned:97](../../../src/inet/networklayer/ipv4/Ipv4.ned#L97); [networklayer/ipv4/Ipv4.cc:81,835](../../../src/inet/networklayer/ipv4/Ipv4.cc#L81) |
| <a id="ipv4-014"></a>IPV4-014 | Default off-net datagram size | [RFC 1122 §3.3.3](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.3) | SHOULD | `Partial` | results | h | [networklayer/ipv4/Ipv4.cc:963-966](../../../src/inet/networklayer/ipv4/Ipv4.cc#L963) |
| — | Accept low TTL packets addressed locally | [RFC 1812 §4.2.2.9](https://www.rfc-editor.org/rfc/rfc1812#section-4.2.2.9) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:388-389](../../../src/inet/networklayer/ipv4/Ipv4.cc#L388); [networklayer/ipv4/Ipv4.cc:819-828](../../../src/inet/networklayer/ipv4/Ipv4.cc#L819) |
| — | TTL decrement on forwarding | [RFC 1812 §5.3.1](https://www.rfc-editor.org/rfc/rfc1812#section-5.3.1) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:321-325](../../../src/inet/networklayer/ipv4/Ipv4.cc#L321); [tests/module/UDP_ttl_ipv4.test](../../../tests/module/UDP_ttl_ipv4.test) |
| — | TTL expiry discard and Time Exceeded | [RFC 1812 §5.3.1](https://www.rfc-editor.org/rfc/rfc1812#section-5.3.1) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:943-951](../../../src/inet/networklayer/ipv4/Ipv4.cc#L943); [networklayer/ipv4/Ipv4.cc:376-379](../../../src/inet/networklayer/ipv4/Ipv4.cc#L376) |
| — | No predictive TTL discard | [RFC 1812 §5.3.1](https://www.rfc-editor.org/rfc/rfc1812#section-5.3.1) | MUST | `Full` | -- | m | [networklayer/ipv4/Ipv4.cc:321-325,943](../../../src/inet/networklayer/ipv4/Ipv4.cc#L321) |
| — | Incremental checksum update | [RFC 1812 §4.2.2.5](https://www.rfc-editor.org/rfc/rfc1812#section-4.2.2.5) | MAY | `Full` | -- | m | [networklayer/ipv4/Ipv4.cc:967-971,1029](../../../src/inet/networklayer/ipv4/Ipv4.cc#L967) |
| — | Reserved header bits preserved | [RFC 1812 §5.2.5](https://www.rfc-editor.org/rfc/rfc1812#section-5.2.5) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4Header.msg:184](../../../src/inet/networklayer/ipv4/Ipv4Header.msg#L184); [networklayer/ipv4/Ipv4.cc:1017](../../../src/inet/networklayer/ipv4/Ipv4.cc#L1017) |
| — | Fragmentation of forwarded datagrams | [RFC 1812 §4.2.2.7](https://www.rfc-editor.org/rfc/rfc1812#section-4.2.2.7) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:675-680,987-1036](../../../src/inet/networklayer/ipv4/Ipv4.cc#L675); [tests/module/IPv4_refragmentation.test](../../../tests/module/IPv4_refragmentation.test) |
| — | No reassembly before forwarding | [RFC 1812 §5.2.6](https://www.rfc-editor.org/rfc/rfc1812#section-5.2.6) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:621-680,819-828](../../../src/inet/networklayer/ipv4/Ipv4.cc#L621) |
| — | Fragmentation needed with DF set | [RFC 1812 §5.2.7.1](https://www.rfc-editor.org/rfc/rfc1812#section-5.2.7.1) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:977-984](../../../src/inet/networklayer/ipv4/Ipv4.cc#L977); [networklayer/ipv4/Icmp.cc:161-187](../../../src/inet/networklayer/ipv4/Icmp.cc#L161); networklayer/ipv4/Icmp… |
| — | Total length field | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:1116,1027](../../../src/inet/networklayer/ipv4/Ipv4.cc#L1116); [networklayer/ipv4/Ipv4.cc:288-297](../../../src/inet/networklayer/ipv4/Ipv4.cc#L288) |
| — | Identification field uniqueness | [RFC 791 §3.2](https://www.rfc-editor.org/rfc/rfc791#section-3.2) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:1089](../../../src/inet/networklayer/ipv4/Ipv4.cc#L1089); [networklayer/ipv4/Ipv4.h:84](../../../src/inet/networklayer/ipv4/Ipv4.h#L84) |
| — | Don't fragment flag | [RFC 791 §2.3](https://www.rfc-editor.org/rfc/rfc791#section-2.3) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:977-984](../../../src/inet/networklayer/ipv4/Ipv4.cc#L977) |
| — | More fragments flag | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:1022-1024,1090](../../../src/inet/networklayer/ipv4/Ipv4.cc#L1022) |
| — | Fragment offset field | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:990,1026](../../../src/inet/networklayer/ipv4/Ipv4.cc#L990); [networklayer/ipv4/Ipv4HeaderSerializer.cc:29-30](../../../src/inet/networklayer/ipv4/Ipv4HeaderSerializer.cc#L29) |
| — | Time to live expiry | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:943-951](../../../src/inet/networklayer/ipv4/Ipv4.cc#L943) |
| — | Time to live decrement | [RFC 791 §3.2](https://www.rfc-editor.org/rfc/rfc791#section-3.2) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:321-325](../../../src/inet/networklayer/ipv4/Ipv4.cc#L321); [tests/module/UDP_ttl_ipv4.test](../../../tests/module/UDP_ttl_ipv4.test) |
| — | Maximum receivable datagram size | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4FragBuf.cc:33-95](../../../src/inet/networklayer/ipv4/Ipv4FragBuf.cc#L33); [common/packet/ReassemblyBuffer.h:23-60](../../../src/inet/common/packet/ReassemblyBuffer.h#L23) |
| — | Minimum forwardable datagram size | [RFC 791 §3.2](https://www.rfc-editor.org/rfc/rfc791#section-3.2) | MUST | `Full` | -- | m | [networklayer/ipv4/Ipv4.cc:963-970,992-993](../../../src/inet/networklayer/ipv4/Ipv4.cc#L963) |
| <a id="ipv4-015"></a>IPV4-015 | Fragmentation procedure | [RFC 791 §3.2](https://www.rfc-editor.org/rfc/rfc791#section-3.2) | MUST | `Partial` | cosmetic | h | [networklayer/ipv4/Ipv4.cc:987-1036](../../../src/inet/networklayer/ipv4/Ipv4.cc#L987) |
| <a id="ipv4-016"></a>IPV4-016 | Fragment reassembly | [RFC 791 §3.2](https://www.rfc-editor.org/rfc/rfc791#section-3.2) | MUST | `Partial` | cosmetic | h | [networklayer/ipv4/Ipv4FragBuf.h:22-33](../../../src/inet/networklayer/ipv4/Ipv4FragBuf.h#L22); [networklayer/ipv4/Ipv4FragBuf.cc:36-60](../../../src/inet/networklayer/ipv4/Ipv4FragBuf.cc#L36) |
| <a id="ipv4-017"></a>IPV4-017 | Reassembly timer | [RFC 791 §3.2](https://www.rfc-editor.org/rfc/rfc791#section-3.2) | MUST | `Partial` | results | h | [networklayer/ipv4/Ipv4.cc:832-836](../../../src/inet/networklayer/ipv4/Ipv4.cc#L832); [networklayer/ipv4/Ipv4FragBuf.cc:97-127](../../../src/inet/networklayer/ipv4/Ipv4FragBuf.cc#L97) |
| — | Reassembly timer initial value | [RFC 791 §3.2](https://www.rfc-editor.org/rfc/rfc791#section-3.2) | SHOULD | `Full` | -- | h | [networklayer/ipv4/Ipv4.ned:97](../../../src/inet/networklayer/ipv4/Ipv4.ned#L97) |
| — | Reassembled total length | [RFC 791 §3.2](https://www.rfc-editor.org/rfc/rfc791#section-3.2) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4FragBuf.cc:69-88](../../../src/inet/networklayer/ipv4/Ipv4FragBuf.cc#L69) |
| — | Atomic datagram classification | [RFC 6864 §4](https://www.rfc-editor.org/rfc/rfc6864#section-4) | MUST | `Full` | -- | m | [networklayer/ipv4/Ipv4.cc:828](../../../src/inet/networklayer/ipv4/Ipv4.cc#L828) |
| — | ID field used only for fragmentation | [RFC 6864 §4.1](https://www.rfc-editor.org/rfc/rfc6864#section-4.1) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4FragBuf.cc:39](../../../src/inet/networklayer/ipv4/Ipv4FragBuf.cc#L39); [networklayer/ipv4/Ipv4HeaderSerializer.cc:28](../../../src/inet/networklayer/ipv4/Ipv4HeaderSerializer.cc#L28); networkla… |
| — | Arbitrary ID in atomic datagrams | [RFC 6864 §4.1](https://www.rfc-editor.org/rfc/rfc6864#section-4.1) | MAY | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:1089](../../../src/inet/networklayer/ipv4/Ipv4.cc#L1089) |
| — | Ignoring the ID of atomic datagrams | [RFC 6864 §4.1](https://www.rfc-editor.org/rfc/rfc6864#section-4.1) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:828](../../../src/inet/networklayer/ipv4/Ipv4.cc#L828); [networklayer/ipv4/Ipv4FragBuf.cc:39](../../../src/inet/networklayer/ipv4/Ipv4FragBuf.cc#L39) |
| — | No ID reuse for retransmitted copies | [RFC 6864 §4.2](https://www.rfc-editor.org/rfc/rfc6864#section-4.2) | MUST | `Full` | -- | m | [networklayer/ipv4/Ipv4.cc:1089](../../../src/inet/networklayer/ipv4/Ipv4.cc#L1089) |
| <a id="ipv4-018"></a>[IPV4-018](#f9) | ID uniqueness for non-atomic datagrams | [RFC 6864 §4.3](https://www.rfc-editor.org/rfc/rfc6864#section-4.3) | MUST | `Partial` | cosmetic | h | [networklayer/ipv4/Ipv4.h:84](../../../src/inet/networklayer/ipv4/Ipv4.h#L84); [networklayer/ipv4/Ipv4.cc:1089](../../../src/inet/networklayer/ipv4/Ipv4.cc#L1089) |
| — | DF=1 datagrams never fragmented | [RFC 6864 §4.3](https://www.rfc-editor.org/rfc/rfc6864#section-4.3) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:977-984](../../../src/inet/networklayer/ipv4/Ipv4.cc#L977) |
| — | DF bit preserved in transit | [RFC 6864 §4.3](https://www.rfc-editor.org/rfc/rfc6864#section-4.3) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:321-325,1017](../../../src/inet/networklayer/ipv4/Ipv4.cc#L321) |
| <a id="ipv4-019"></a>IPV4-019 | Congestion indication preserved at reassembly | [RFC 3168 §5.3](https://www.rfc-editor.org/rfc/rfc3168#section-5.3) | MUST | `Missing` | scope | h | [networklayer/ipv4/Ipv4FragBuf.cc:69-88](../../../src/inet/networklayer/ipv4/Ipv4FragBuf.cc#L69) |
| <a id="ipv4-020"></a>IPV4-020 | No CE when a fragment is Not-ECT | [RFC 3168 §5.3](https://www.rfc-editor.org/rfc/rfc3168#section-5.3) | MUST | `Missing` | scope | m | [networklayer/ipv4/Ipv4FragBuf.cc:69-88](../../../src/inet/networklayer/ipv4/Ipv4FragBuf.cc#L69) |
| <a id="ipv4-021"></a>IPV4-021 | DF-based path MTU probing | [RFC 1191 §2](https://www.rfc-editor.org/rfc/rfc1191#section-2) | MUST | `Partial` | scope | h | [transportlayer/tcp/TcpConnectionUtil.cc:293-295,653-655](../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L293); [transportlayer/tcp/Tcp.ned:199,208](../../../src/inet/transportlayer/tcp/Tcp.ned#L199) |
| <a id="ipv4-022"></a>IPV4-022 | PMTU reduced on Datagram Too Big | [RFC 1191 §3](https://www.rfc-editor.org/rfc/rfc1191#section-3) | MUST | `Partial` | scope | h | [transportlayer/tcp/TcpConnectionUtil.cc:398-412](../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L398); [networklayer/ipv4/Icmp.cc:280-281](../../../src/inet/networklayer/ipv4/Icmp.cc#L280) |
| — | Next-Hop MTU reported by router | [RFC 1191 §4](https://www.rfc-editor.org/rfc/rfc1191#section-4) | MUST | `Full` | -- | h | [networklayer/ipv4/Icmp.cc:161-187](../../../src/inet/networklayer/ipv4/Icmp.cc#L161); [networklayer/ipv4/IcmpHeader.msg:48-54](../../../src/inet/networklayer/ipv4/IcmpHeader.msg#L48); networklayer/ipv4… |
| <a id="ipv4-023"></a>IPV4-023 | Path MTU floor of 68 octets | [RFC 1191 §3](https://www.rfc-editor.org/rfc/rfc1191#section-3) | MUST | `Missing` | scope | h | [transportlayer/tcp/TcpConnectionUtil.cc:400-407](../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L400) |
| — | No PMTU increase from an ICMP message | [RFC 1191 §3](https://www.rfc-editor.org/rfc/rfc1191#section-3) | MUST | `Full` | -- | h | [transportlayer/tcp/TcpConnectionUtil.cc:405-409](../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L405) |
| <a id="ipv4-024"></a>IPV4-024 | MTU plateau search table | [RFC 1191 §7](https://www.rfc-editor.org/rfc/rfc1191#section-7) | MAY | `Missing` | scope | h | [transportlayer/tcp/TcpConnectionUtil.cc:400-401](../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L400); no plateau table found under transportlayer… |
| — | Aging of the path MTU estimate | [RFC 1191 §6.3](https://www.rfc-editor.org/rfc/rfc1191#section-6.3) | SHOULD | `Full` | -- | h | [transportlayer/tcp/TcpConnectionUtil.cc:944-953](../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L944); [transportlayer/tcp/Tcp.ned:209](../../../src/inet/transportlayer/tcp/Tcp.ned#L209) |

### 3c. IP header options

37 requirements. 12 `Full`, 2 `Incorrect`, 13 `Missing`, 2 `Non-gap`, 8 `Partial`.

| ID | Feature | Spec clause | Level | Status | Impact | Conf. | Evidence |
|---|---|---|---|---|---|---|---|
| — | Received option delivery | [RFC 1122 §3.2.1.8](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.1.8) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:895-916](../../../src/inet/networklayer/ipv4/Ipv4.cc#L895); [networklayer/ipv4/Ipv4OptionsTag.msg:28](../../../src/inet/networklayer/ipv4/Ipv4OptionsTag.msg#L28) |
| — | Unknown option silent ignore | [RFC 1122 §3.2.1.8](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.1.8) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4HeaderSerializer.cc:292-308](../../../src/inet/networklayer/ipv4/Ipv4HeaderSerializer.cc#L292); [networklayer/ipv4/Ipv4.cc:268-318](../../../src/inet/networklayer/ipv4/Ipv4.cc#L268) |
| <a id="ipv4-025"></a>IPV4-025 | Source route origination and termination | [RFC 1122 §3.2.1.8c](https://www.rfc-editor.org/rfc/rfc1122) | MUST | `Partial` | scope | m | [networklayer/ipv4/Ipv4.cc:1105-1112](../../../src/inet/networklayer/ipv4/Ipv4.cc#L1105); [networklayer/ipv4/Ipv4Header.msg:100-109](../../../src/inet/networklayer/ipv4/Ipv4Header.msg#L100) |
| — | Completed source route delivery | [RFC 1122 §3.2.1.8c](https://www.rfc-editor.org/rfc/rfc1122) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:895-916](../../../src/inet/networklayer/ipv4/Ipv4.cc#L895) |
| <a id="ipv4-026"></a>IPV4-026 | Return source route construction | [RFC 1122 §3.2.1.8c](https://www.rfc-editor.org/rfc/rfc1122) | MUST | `Missing` | scope | h | searched [networklayer/ipv4/Ipv4.cc](../../../src/inet/networklayer/ipv4/Ipv4.cc) and Icmp.cc: no route-reversal or return-route constructi… |
| — | Single source route option | [RFC 1122 §3.2.1.8c](https://www.rfc-editor.org/rfc/rfc1122) | MUST | `Full` | -- | m | [networklayer/ipv4/Ipv4.cc:1105-1112](../../../src/inet/networklayer/ipv4/Ipv4.cc#L1105); [networklayer/ipv4/Ipv4Header.cc:30-33](../../../src/inet/networklayer/ipv4/Ipv4Header.cc#L30) |
| <a id="ipv4-027"></a>IPV4-027 | Timestamp option at destination | [RFC 1122 §3.2.1.8e](https://www.rfc-editor.org/rfc/rfc1122) | MUST | `Missing` | scope | h | [networklayer/ipv4/Ipv4.cc:895-916](../../../src/inet/networklayer/ipv4/Ipv4.cc#L895), 1136-1141 |
| <a id="ipv4-028"></a>IPV4-028 | Non-local source route forwarding default | [RFC 1122 §3.3.5](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.5) | MUST | `Non-gap` | -- | h | [networklayer/ipv4/Ipv4.cc:407-419](../../../src/inet/networklayer/ipv4/Ipv4.cc#L407); [networklayer/ipv4/Ipv4.ned:90-105](../../../src/inet/networklayer/ipv4/Ipv4.ned#L90) |
| <a id="ipv4-029"></a>IPV4-029 | Local delivery decision rules | [RFC 1812 §5.2.3](https://www.rfc-editor.org/rfc/rfc1812#section-5.2.3) | MUST | `Partial` | results | h | [networklayer/ipv4/Ipv4.cc:344-420](../../../src/inet/networklayer/ipv4/Ipv4.cc#L344) |
| <a id="ipv4-030"></a>IPV4-030 | Discard when no route remains | [RFC 1812 §5.2.4.3](https://www.rfc-editor.org/rfc/rfc1812#section-5.2.4.3) | MUST | `Partial` | scope | h | [networklayer/ipv4/Ipv4.cc:656-663](../../../src/inet/networklayer/ipv4/Ipv4.cc#L656); [networklayer/common/IcmpType.msg:67](../../../src/inet/networklayer/common/IcmpType.msg#L67) |
| <a id="ipv4-031"></a>IPV4-031 | IP Destination Address drives lookup | [RFC 1812 §5.2.4.1](https://www.rfc-editor.org/rfc/rfc1812#section-5.2.4.1) | MUST | `Partial` | scope | h | [networklayer/ipv4/Ipv4.cc:647-654](../../../src/inet/networklayer/ipv4/Ipv4.cc#L647) |
| <a id="ipv4-032"></a>IPV4-032 | Multiple source route options rejected | [RFC 1812 §5.2.4.1](https://www.rfc-editor.org/rfc/rfc1812#section-5.2.4.1) | SHOULD | `Missing` | scope | h | searched [networklayer/ipv4/Ipv4.cc](../../../src/inet/networklayer/ipv4/Ipv4.cc): no option validation; Ipv4HeaderSerializer.cc:262-279 on… |
| <a id="ipv4-033"></a>IPV4-033 | Unfulfillable strict source route | [RFC 1812 §5.2.4.3](https://www.rfc-editor.org/rfc/rfc1812#section-5.2.4.3) | MUST | `Missing` | scope | h | searched [networklayer/ipv4/Ipv4.cc:344-420](../../../src/inet/networklayer/ipv4/Ipv4.cc#L344), 620-670: no strict-source-route handling |
| <a id="ipv4-034"></a>IPV4-034 | Source route option forwarding | [RFC 1812 §5.3.13.4](https://www.rfc-editor.org/rfc/rfc1812#section-5.3.13.4) | MUST | `Missing` | scope | h | [networklayer/ipv4/Ipv4.cc:320-326](../../../src/inet/networklayer/ipv4/Ipv4.cc#L320) |
| <a id="ipv4-035"></a>[IPV4-035](#f4) | Router address inserted into options | [RFC 1812 §4.2.2.2](https://www.rfc-editor.org/rfc/rfc1812#section-4.2.2.2) | MUST | `Partial` | scope | h | [networklayer/ipv4/Ipv4.cc:1136-1141](../../../src/inet/networklayer/ipv4/Ipv4.cc#L1136) |
| <a id="ipv4-036"></a>[IPV4-036](#f4) | Record Route on forwarding | [RFC 1812 §5.3.13.5](https://www.rfc-editor.org/rfc/rfc1812#section-5.3.13.5) | MUST | `Missing` | scope | h | [networklayer/ipv4/Ipv4.cc:320-326](../../../src/inet/networklayer/ipv4/Ipv4.cc#L320); [networklayer/ipv4/Ipv4HeaderSerializer.cc:262-279](../../../src/inet/networklayer/ipv4/Ipv4HeaderSerializer.cc#L262) |
| <a id="ipv4-037"></a>[IPV4-037](#f4) | Timestamp option on forwarding | [RFC 1812 §5.3.13.6](https://www.rfc-editor.org/rfc/rfc1812#section-5.3.13.6) | MUST | `Missing` | scope | h | [networklayer/ipv4/Ipv4.cc:320-326](../../../src/inet/networklayer/ipv4/Ipv4.cc#L320), 1136-1141 |
| — | Unrecognized options passed through | [RFC 1812 §5.3.13.1](https://www.rfc-editor.org/rfc/rfc1812#section-5.3.13.1) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:320-326](../../../src/inet/networklayer/ipv4/Ipv4.cc#L320); [networklayer/ipv4/Ipv4HeaderSerializer.cc:292-308](../../../src/inet/networklayer/ipv4/Ipv4HeaderSerializer.cc#L292) |
| <a id="ipv4-038"></a>[IPV4-038](#f4) | Directed broadcast forwarding | [RFC 1812 §5.3.5.2](https://www.rfc-editor.org/rfc/rfc1812#section-5.3.5.2) | MUST | `Partial` | scope | h | [networklayer/ipv4/Ipv4.cc:392-401](../../../src/inet/networklayer/ipv4/Ipv4.cc#L392); [networklayer/ipv4/Ipv4.ned:102](../../../src/inet/networklayer/ipv4/Ipv4.ned#L102) |
| <a id="ipv4-148"></a>IPV4-148 | Redirect generation preconditions | [RFC 1812 §5.2.7.2](https://www.rfc-editor.org/rfc/rfc1812#section-5.2.7.2) | MUST | `Partial` | scope | h | [networklayer/ipv4/Icmp.cc:261-263](../../../src/inet/networklayer/ipv4/Icmp.cc#L261); [networklayer/ipv4/Ipv4.cc:620-670](../../../src/inet/networklayer/ipv4/Ipv4.cc#L620) |
| <a id="ipv4-039"></a>[IPV4-039](#f4) | Header checksum reverification | [RFC 791 §3.2](https://www.rfc-editor.org/rfc/rfc791#section-3.2) | MUST | `Partial` | results | m | [networklayer/ipv4/Ipv4.cc:282](../../../src/inet/networklayer/ipv4/Ipv4.cc#L282), 966-970, 1027-1029 |
| <a id="ipv4-040"></a>[IPV4-040](#f4) | Option implementation requirement | [RFC 791 §3.2](https://www.rfc-editor.org/rfc/rfc791#section-3.2) | MUST | `Partial` | scope | h | [networklayer/ipv4/Ipv4HeaderSerializer.cc:205-309](../../../src/inet/networklayer/ipv4/Ipv4HeaderSerializer.cc#L205); [networklayer/ipv4/Ipv4.cc:320-326](../../../src/inet/networklayer/ipv4/Ipv4.cc#L320) |
| <a id="ipv4-149"></a>IPV4-149 | Option format and length octet | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | MUST | `Partial` | results | h | [networklayer/ipv4/Ipv4HeaderSerializer.cc:72-88](../../../src/inet/networklayer/ipv4/Ipv4HeaderSerializer.cc#L72), 205-309 |
| <a id="ipv4-041"></a>[IPV4-041](#f4) | Option copying on fragmentation | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | MUST | `Incorrect` | results | h | [networklayer/ipv4/Ipv4.cc:987](../../../src/inet/networklayer/ipv4/Ipv4.cc#L987), 1013-1031 |
| — | End of option list option | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4Header.msg:91-95](../../../src/inet/networklayer/ipv4/Ipv4Header.msg#L91); [networklayer/ipv4/Ipv4HeaderSerializer.cc:58-62](../../../src/inet/networklayer/ipv4/Ipv4HeaderSerializer.cc#L58), 212… |
| — | No operation option | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | MAY | `Full` | -- | h | [networklayer/ipv4/Ipv4Header.msg:85-89](../../../src/inet/networklayer/ipv4/Ipv4Header.msg#L85); [networklayer/ipv4/Ipv4HeaderSerializer.cc:96-99](../../../src/inet/networklayer/ipv4/Ipv4HeaderSerializer.cc#L96), 215… |
| <a id="ipv4-042"></a>[IPV4-042](#f4) | Security option | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | MUST | `Missing` | scope | m | [networklayer/ipv4/Ipv4Header.msg:62](../../../src/inet/networklayer/ipv4/Ipv4Header.msg#L62); [networklayer/ipv4/Ipv4HeaderSerializer.cc:143-147](../../../src/inet/networklayer/ipv4/Ipv4HeaderSerializer.cc#L143), 292-295 |
| <a id="ipv4-043"></a>[IPV4-043](#f4) | Loose source and record route | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | MUST | `Missing` | scope | h | [networklayer/ipv4/Ipv4HeaderSerializer.cc:124-135](../../../src/inet/networklayer/ipv4/Ipv4HeaderSerializer.cc#L124), 262-279; [networklayer/ipv4/Ipv4.cc:320-326](../../../src/inet/networklayer/ipv4/Ipv4.cc#L320) |
| <a id="ipv4-044"></a>[IPV4-044](#f4) | Strict source route forwarding | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | MUST | `Missing` | scope | h | [networklayer/ipv4/Ipv4HeaderSerializer.cc:126-135](../../../src/inet/networklayer/ipv4/Ipv4HeaderSerializer.cc#L126), 262-279; [networklayer/ipv4/Ipv4.cc:320-326](../../../src/inet/networklayer/ipv4/Ipv4.cc#L320) |
| <a id="ipv4-045"></a>[IPV4-045](#f4) | Record route insertion | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | MUST | `Missing` | scope | h | [networklayer/ipv4/Ipv4.cc:320-326](../../../src/inet/networklayer/ipv4/Ipv4.cc#L320); [networklayer/ipv4/Ipv4HeaderSerializer.cc:272](../../../src/inet/networklayer/ipv4/Ipv4HeaderSerializer.cc#L272) |
| <a id="ipv4-046"></a>[IPV4-046](#f4) | Record route overflow handling | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | MUST | `Missing` | scope | h | [networklayer/ipv4/Ipv4.cc:320-326](../../../src/inet/networklayer/ipv4/Ipv4.cc#L320) |
| — | Stream identifier option | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4Header.msg:133-138](../../../src/inet/networklayer/ipv4/Ipv4Header.msg#L133); [networklayer/ipv4/Ipv4HeaderSerializer.cc:101-106](../../../src/inet/networklayer/ipv4/Ipv4HeaderSerializer.cc#L101),… |
| <a id="ipv4-047"></a>[IPV4-047](#f4) | Timestamp option format | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | MUST | `Incorrect` | results | h | [networklayer/ipv4/Ipv4.cc:1136-1141](../../../src/inet/networklayer/ipv4/Ipv4.cc#L1136); [networklayer/ipv4/Ipv4HeaderSerializer.cc:108-113](../../../src/inet/networklayer/ipv4/Ipv4HeaderSerializer.cc#L108) |
| — | Timestamp flag modes | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4HeaderSerializer.cc:108-122](../../../src/inet/networklayer/ipv4/Ipv4HeaderSerializer.cc#L108), 229-260; [networklayer/ipv4/Ipv4Header.msg](../../../src/inet/networklayer/ipv4/Ipv4Header.msg)… |
| <a id="ipv4-048"></a>[IPV4-048](#f4) | Timestamp overflow handling | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | MUST | `Missing` | scope | h | [networklayer/ipv4/Ipv4HeaderSerializer.cc:114](../../../src/inet/networklayer/ipv4/Ipv4HeaderSerializer.cc#L114), 247 |
| <a id="ipv4-049"></a>[IPV4-049](#f4) | SEND interface call | [RFC 791 §3.3](https://www.rfc-editor.org/rfc/rfc791#section-3.3) | MUST | `Partial` | cosmetic | h | [networklayer/ipv4/Ipv4.cc:1040-1112](../../../src/inet/networklayer/ipv4/Ipv4.cc#L1040) |
| — | RECV interface call | [RFC 791 §3.3](https://www.rfc-editor.org/rfc/rfc791#section-3.3) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:895-916](../../../src/inet/networklayer/ipv4/Ipv4.cc#L895) |

### 3d. ICMP

52 requirements. 27 `Full`, 4 `Incorrect`, 9 `Missing`, 7 `Non-gap`, 5 `Partial`.

| ID | Feature | Spec clause | Level | Status | Impact | Conf. | Evidence |
|---|---|---|---|---|---|---|---|
| — | ICMP carried over IP protocol 1 | [RFC 792 — Message Formats](https://www.rfc-editor.org/rfc/rfc792) | MUST | `Full` | -- | h | [common/ProtocolGroup.cc:115](../../../src/inet/common/ProtocolGroup.cc#L115); [networklayer/ipv4/Icmp.cc:368-373](../../../src/inet/networklayer/ipv4/Icmp.cc#L368) |
| — | ICMP checksum computation | [RFC 792 — Message Formats](https://www.rfc-editor.org/rfc/rfc792) | MUST | `Full` | -- | h | [networklayer/ipv4/Icmp.cc:403-412](../../../src/inet/networklayer/ipv4/Icmp.cc#L403); [networklayer/ipv4/Icmp.ned:22](../../../src/inet/networklayer/ipv4/Icmp.ned#L22) |
| — | Unused header fields zeroed | [RFC 792 — Message Formats](https://www.rfc-editor.org/rfc/rfc792) | MUST | `Full` | -- | h | [networklayer/ipv4/IcmpHeaderSerializer.cc:45-52](../../../src/inet/networklayer/ipv4/IcmpHeaderSerializer.cc#L45); [networklayer/ipv4/IcmpHeader.msg:52](../../../src/inet/networklayer/ipv4/IcmpHeader.msg#L52) |
| — | No ICMP errors about ICMP errors | [RFC 792 — Introduction](https://www.rfc-editor.org/rfc/rfc792) | MUST | `Full` | -- | h | [networklayer/ipv4/Icmp.cc:131-138](../../../src/inet/networklayer/ipv4/Icmp.cc#L131) |
| — | ICMP errors only about fragment zero | [RFC 792 — Introduction](https://www.rfc-editor.org/rfc/rfc792) | MUST | `Full` | -- | h | [networklayer/ipv4/Icmp.cc:125-129](../../../src/inet/networklayer/ipv4/Icmp.cc#L125) |
| — | Error message addressing and quoting | [RFC 792 — Destination Unreachable Message](https://www.rfc-editor.org/rfc/rfc792) | MUST | `Full` | -- | h | [networklayer/ipv4/Icmp.cc:208-215](../../../src/inet/networklayer/ipv4/Icmp.cc#L208); [networklayer/ipv4/Icmp.ned:23](../../../src/inet/networklayer/ipv4/Icmp.ned#L23) |
| — | Destination Unreachable message format | [RFC 792 — Destination Unreachable Message](https://www.rfc-editor.org/rfc/rfc792) | MUST | `Full` | -- | h | [networklayer/ipv4/IcmpHeader.msg:23-29](../../../src/inet/networklayer/ipv4/IcmpHeader.msg#L23); [networklayer/ipv4/IcmpHeaderSerializer.cc:38-49](../../../src/inet/networklayer/ipv4/IcmpHeaderSerializer.cc#L38) |
| — | Destination Unreachable net code | [RFC 792 — Destination Unreachable Message](https://www.rfc-editor.org/rfc/rfc792) | MAY | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:658-663](../../../src/inet/networklayer/ipv4/Ipv4.cc#L658) |
| <a id="ipv4-050"></a>IPV4-050 | Destination Unreachable host code | [RFC 792 — Destination Unreachable Message](https://www.rfc-editor.org/rfc/rfc792) | MAY | `Missing` | results | h | [networklayer/ipv4/Ipv4.cc:1188-1205](../../../src/inet/networklayer/ipv4/Ipv4.cc#L1188) (ARP timeout drops silently); no ICMP_DU_HOST_UNREACHABL… |
| — | Destination Unreachable protocol code | [RFC 792 — Destination Unreachable Message](https://www.rfc-editor.org/rfc/rfc792) | MAY | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:886-890](../../../src/inet/networklayer/ipv4/Ipv4.cc#L886); [tests/module/IPv4_ICMPerror_NoProtocol.test](../../../tests/module/IPv4_ICMPerror_NoProtocol.test) |
| — | Destination Unreachable port code | [RFC 792 — Destination Unreachable Message](https://www.rfc-editor.org/rfc/rfc792) | MAY | `Full` | -- | h | [transportlayer/udp/Udp.cc:1113-1116](../../../src/inet/transportlayer/udp/Udp.cc#L1113); [networklayer/ipv4/Icmp.cc:85-89](../../../src/inet/networklayer/ipv4/Icmp.cc#L85) |
| — | Destination Unreachable fragmentation needed code | [RFC 792 — Destination Unreachable Message](https://www.rfc-editor.org/rfc/rfc792) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:978-985](../../../src/inet/networklayer/ipv4/Ipv4.cc#L978); [networklayer/ipv4/Icmp.cc:161-187](../../../src/inet/networklayer/ipv4/Icmp.cc#L161) |
| <a id="ipv4-051"></a>IPV4-051 | Destination Unreachable source route failed code | [RFC 792 — Destination Unreachable Message](https://www.rfc-editor.org/rfc/rfc792) | MAY | `Missing` | scope | h | no IPOPTION_LOOSE/STRICT_SOURCE_ROUTING handling in [networklayer/ipv4/Ipv4.cc](../../../src/inet/networklayer/ipv4/Ipv4.cc); no ICMP_DU_SO… |
| — | Time Exceeded message format | [RFC 792 — Time Exceeded Message](https://www.rfc-editor.org/rfc/rfc792) | MUST | `Full` | -- | h | [networklayer/ipv4/IcmpHeaderSerializer.cc:50-52](../../../src/inet/networklayer/ipv4/IcmpHeaderSerializer.cc#L50); [networklayer/ipv4/IcmpHeader.msg:23-29](../../../src/inet/networklayer/ipv4/IcmpHeader.msg#L23) |
| — | Time Exceeded in transit code | [RFC 792 — Time Exceeded Message](https://www.rfc-editor.org/rfc/rfc792) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:943-951](../../../src/inet/networklayer/ipv4/Ipv4.cc#L943) |
| <a id="ipv4-052"></a>[IPV4-052](#f3) | Time Exceeded reassembly code | [RFC 792 — Time Exceeded Message](https://www.rfc-editor.org/rfc/rfc792) | MAY | `Incorrect` | results | h | [networklayer/ipv4/Ipv4FragBuf.cc:116](../../../src/inet/networklayer/ipv4/Ipv4FragBuf.cc#L116) |
| <a id="ipv4-053"></a>[IPV4-053](#f6) | Parameter Problem message format | [RFC 792 — Parameter Problem Message](https://www.rfc-editor.org/rfc/rfc792) | MUST | `Partial` | results | h | [networklayer/ipv4/IcmpHeader.msg:23-29](../../../src/inet/networklayer/ipv4/IcmpHeader.msg#L23) (no pointer field); networklayer/ipv4/IcmpHeaderSeria… |
| <a id="ipv4-147"></a>[IPV4-147](#f1) | Parameter Problem trigger and discard | [RFC 792 — Parameter Problem Message](https://www.rfc-editor.org/rfc/rfc792) | MUST | `Partial` | results | h | [networklayer/ipv4/Ipv4.cc:291-293,306-311](../../../src/inet/networklayer/ipv4/Ipv4.cc#L291); [networklayer/ipv4/Ipv4.cc:282](../../../src/inet/networklayer/ipv4/Ipv4.cc#L282) |
| <a id="ipv4-054"></a>IPV4-054 | Source Quench generation | [RFC 792 — Source Quench Message](https://www.rfc-editor.org/rfc/rfc792) | MAY | `Non-gap` | -- | h | [networklayer/common/IcmpType.msg:19](../../../src/inet/networklayer/common/IcmpType.msg#L19) (constant only, no sender) |
| <a id="ipv4-055"></a>IPV4-055 | Source Quench receiver behavior | [RFC 792 — Source Quench Message](https://www.rfc-editor.org/rfc/rfc792) | SHOULD | `Incorrect` | results | h | no ICMP_SOURCEQUENCH handling in [networklayer/ipv4/Icmp.cc](../../../src/inet/networklayer/ipv4/Icmp.cc) |
| <a id="ipv4-056"></a>[IPV4-056](#f6) | Redirect message format and codes | [RFC 792 — Redirect Message](https://www.rfc-editor.org/rfc/rfc792) | MUST | `Missing` | scope | h | [networklayer/ipv4/IcmpHeader.msg](../../../src/inet/networklayer/ipv4/IcmpHeader.msg) (no Redirect class, no gateway address field); networklayer… |
| <a id="ipv4-057"></a>[IPV4-057](#f6) | Redirect generation trigger | [RFC 792 — Redirect Message](https://www.rfc-editor.org/rfc/rfc792) | MUST | `Missing` | scope | h | no ICMP_REDIRECT generation anywhere in [networklayer/ipv4/Ipv4.cc](../../../src/inet/networklayer/ipv4/Ipv4.cc) or Icmp.cc |
| — | No Redirect for source routed datagrams | [RFC 792 — Redirect Message](https://www.rfc-editor.org/rfc/rfc792) | MUST | `Full` | -- | m | [networklayer/ipv4/Icmp.cc:261-265](../../../src/inet/networklayer/ipv4/Icmp.cc#L261); [networklayer/ipv4/Ipv4.cc](../../../src/inet/networklayer/ipv4/Ipv4.cc) (no source-route processing) |
| — | Echo and Echo Reply message format | [RFC 792 — Echo or Echo Reply Message](https://www.rfc-editor.org/rfc/rfc792) | MUST | `Full` | -- | h | [networklayer/ipv4/IcmpHeader.msg:32-46](../../../src/inet/networklayer/ipv4/IcmpHeader.msg#L32); [networklayer/ipv4/IcmpHeaderSerializer.cc:22-34](../../../src/inet/networklayer/ipv4/IcmpHeaderSerializer.cc#L22) |
| — | Echo Reply generation | [RFC 792 — Echo or Echo Reply Message](https://www.rfc-editor.org/rfc/rfc792) | MUST | `Full` | -- | h | [networklayer/ipv4/Icmp.cc:335-360](../../../src/inet/networklayer/ipv4/Icmp.cc#L335); [tests/module/pingapp_1.test](../../../tests/module/pingapp_1.test) |
| <a id="ipv4-058"></a>[IPV4-058](#f6) | Timestamp and Timestamp Reply message format | [RFC 792 — Timestamp or Timestamp Reply Message](https://www.rfc-editor.org/rfc/rfc792) | MUST | `Missing` | scope | h | [networklayer/ipv4/IcmpHeader.msg](../../../src/inet/networklayer/ipv4/IcmpHeader.msg) has no timestamp class; IcmpHeaderSerializer.cc:53-54 throw… |
| <a id="ipv4-059"></a>[IPV4-059](#f6) | Timestamp Reply generation | [RFC 792 — Timestamp or Timestamp Reply Message](https://www.rfc-editor.org/rfc/rfc792) | MUST | `Incorrect` | results | h | [networklayer/ipv4/Icmp.cc:317-318](../../../src/inet/networklayer/ipv4/Icmp.cc#L317) |
| <a id="ipv4-060"></a>[IPV4-060](#f6) | Timestamp value semantics | [RFC 792 — Timestamp or Timestamp Reply Message](https://www.rfc-editor.org/rfc/rfc792) | MUST | `Missing` | scope | h | no timestamp field or midnight-UT conversion in [networklayer/ipv4/IcmpHeader.msg](../../../src/inet/networklayer/ipv4/IcmpHeader.msg) or Icmp.cc |
| <a id="ipv4-061"></a>IPV4-061 | Information Request and Reply message format | [RFC 792 — Information Request or Information Reply Message](https://www.rfc-editor.org/rfc/rfc792) | MUST | `Non-gap` | -- | h | [networklayer/common/IcmpType.msg:30-31](../../../src/inet/networklayer/common/IcmpType.msg#L30) (constants only) |
| <a id="ipv4-062"></a>IPV4-062 | Information Reply generation | [RFC 792 — Information Request or Information Reply Message](https://www.rfc-editor.org/rfc/rfc792) | SHOULD | `Non-gap` | -- | h | no ICMP_INFORMATION_REQUEST handling in [networklayer/ipv4/Icmp.cc](../../../src/inet/networklayer/ipv4/Icmp.cc) |
| <a id="ipv4-063"></a>IPV4-063 | Address Mask Request and Reply message format | [RFC 950 — Appendix I](https://www.rfc-editor.org/rfc/rfc950) | MUST | `Non-gap` | -- | h | [networklayer/common/IcmpType.msg:32-33](../../../src/inet/networklayer/common/IcmpType.msg#L32) (constants only) |
| <a id="ipv4-064"></a>IPV4-064 | Address Mask Reply generation | [RFC 950 — Appendix I](https://www.rfc-editor.org/rfc/rfc950) | SHOULD | `Non-gap` | -- | h | no ICMP_MASK_REQUEST handling in [networklayer/ipv4/Icmp.cc](../../../src/inet/networklayer/ipv4/Icmp.cc) |
| — | Source Quench sending prohibition | [RFC 6633 — Section 3](https://www.rfc-editor.org/rfc/rfc6633) | MUST | `Full` | -- | h | grep for ICMP_SOURCEQUENCH in src/ finds only IcmpType.msg:19 |
| <a id="ipv4-065"></a>[IPV4-065](#f2) | Source Quench receiving behavior | [RFC 6633 — Section 3](https://www.rfc-editor.org/rfc/rfc6633) | MUST | `Incorrect` | results | h | [networklayer/ipv4/Icmp.cc:325-326](../../../src/inet/networklayer/ipv4/Icmp.cc#L325) |
| — | Information Request and Reply deprecated | [RFC 6918 — Section 3](https://www.rfc-editor.org/rfc/rfc6918) | MUST | `Full` | -- | h | grep for ICMP_INFORMATION_RE in src/ finds only IcmpType.msg:30-31 |
| — | Address Mask Request and Reply deprecated | [RFC 6918 — Section 3](https://www.rfc-editor.org/rfc/rfc6918) | MUST | `Full` | -- | h | grep for ICMP_MASK_RE in src/ finds only IcmpType.msg:32-33 |
| <a id="ipv4-066"></a>[IPV4-066](#f2) | Unknown ICMP type discard | [RFC 1122 §3.2.2](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.2) | MUST | `Incorrect` | results | h | [networklayer/ipv4/Icmp.cc:325-326](../../../src/inet/networklayer/ipv4/Icmp.cc#L325) |
| <a id="ipv4-150"></a>IPV4-150 | ICMP error payload preservation | [RFC 1122 §3.2.2](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.2) | MUST | `Partial` | results | h | [networklayer/ipv4/Icmp.cc:208-215](../../../src/inet/networklayer/ipv4/Icmp.cc#L208); [networklayer/ipv4/Icmp.cc:42-47](../../../src/inet/networklayer/ipv4/Icmp.cc#L42) |
| <a id="ipv4-067"></a>IPV4-067 | ICMP error suppression for broadcast | [RFC 1122 §3.2.2](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.2) | MUST | `Partial` | cosmetic | h | [networklayer/ipv4/Icmp.cc:110-114](../../../src/inet/networklayer/ipv4/Icmp.cc#L110) |
| — | ICMP error suppression for errors and fragments | [RFC 1122 §3.2.2](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.2) | MUST | `Full` | -- | h | [networklayer/ipv4/Icmp.cc:116-138](../../../src/inet/networklayer/ipv4/Icmp.cc#L116) |
| — | Destination Unreachable as hint | [RFC 1122 §3.2.2.1](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.2.1) | MUST | `Full` | -- | m | [networklayer/ipv4/Icmp.cc:267-307](../../../src/inet/networklayer/ipv4/Icmp.cc#L267) |
| — | Host Redirect generation prohibited | [RFC 1122 §3.2.2.2](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.2.2) | SHOULD | `Full` | -- | h | no ICMP_REDIRECT generation in networklayer/ipv4/ |
| <a id="ipv4-068"></a>[IPV4-068](#f6) | Redirect route update | [RFC 1122 §3.2.2.2](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.2.2) | MUST | `Missing` | scope | h | [networklayer/ipv4/Icmp.cc:261-265](../../../src/inet/networklayer/ipv4/Icmp.cc#L261) |
| — | Echo server function | [RFC 1122 §3.2.2.6](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.2.6) | MUST | `Full` | -- | h | [networklayer/ipv4/Icmp.cc:309-310,335-360](../../../src/inet/networklayer/ipv4/Icmp.cc#L309); [tests/module/pingapp_1.test](../../../tests/module/pingapp_1.test) |
| <a id="ipv4-069"></a>IPV4-069 | Echo Reply source address | [RFC 1122 §3.2.2.6](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.2.6) | MUST | `Partial` | results | m | [networklayer/ipv4/Icmp.cc:351-355](../../../src/inet/networklayer/ipv4/Icmp.cc#L351) |
| <a id="ipv4-070"></a>IPV4-070 | Echo Reply source route reversal | [RFC 1122 §3.2.2.6](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.2.6) | MUST | `Missing` | scope | h | no source route option handling in [networklayer/ipv4/Ipv4.cc](../../../src/inet/networklayer/ipv4/Ipv4.cc); [networklayer/ipv4/Icmp.cc:335](../../../src/inet/networklayer/ipv4/Icmp.cc#L335)-… |
| — | Unauthorized Address Mask Reply | [RFC 1122 §3.2.2.9](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.2.9) | MUST | `Full` | -- | h | no ICMP_MASK_REPLY sender in src/ |
| <a id="ipv4-071"></a>IPV4-071 | Destination Unreachable code selection | [RFC 1812 §4.3.3.1](https://www.rfc-editor.org/rfc/rfc1812#section-4.3.3.1) | MUST | `Partial` | results | h | [networklayer/ipv4/Ipv4.cc:658-663](../../../src/inet/networklayer/ipv4/Ipv4.cc#L658) (code 0); no ICMP_DU_HOST_UNREACHABLE sender in src/ |
| <a id="ipv4-072"></a>IPV4-072 | ICMP error suppression cases | [RFC 1812 §4.3.2.7](https://www.rfc-editor.org/rfc/rfc1812#section-4.3.2.7) | MUST | `Partial` | cosmetic | h | [networklayer/ipv4/Icmp.cc:110-138](../../../src/inet/networklayer/ipv4/Icmp.cc#L110) |
| <a id="ipv4-073"></a>IPV4-073 | Source Quench rate limiting | [RFC 1812 §4.3.2.8](https://www.rfc-editor.org/rfc/rfc1812#section-4.3.2.8) | MUST | `Non-gap` | -- | h | [networklayer/ipv4/Icmp.ned:18-24](../../../src/inet/networklayer/ipv4/Icmp.ned#L18) (no rate-limit parameter); no type 4 sender in src/ |
| <a id="ipv4-074"></a>[IPV4-074](#f7) | Other ICMP error rate limiting | [RFC 1812 §4.3.2.8](https://www.rfc-editor.org/rfc/rfc1812#section-4.3.2.8) | SHOULD | `Missing` | scope | h | [networklayer/ipv4/Icmp.ned:18-24](../../../src/inet/networklayer/ipv4/Icmp.ned#L18); [networklayer/ipv4/Icmp.cc:189-217](../../../src/inet/networklayer/ipv4/Icmp.cc#L189) |
| — | Source Quench origination discouraged | [RFC 1812 §4.3.3.3](https://www.rfc-editor.org/rfc/rfc1812#section-4.3.3.3) | SHOULD | `Full` | -- | h | grep for ICMP_SOURCEQUENCH in src/ finds only IcmpType.msg:19 |

### 3e. ARP and address conflict detection

33 requirements. 20 `Full`, 1 `Incorrect`, 10 `Missing`, 2 `Partial`.

| ID | Feature | Spec clause | Level | Status | Impact | Conf. | Evidence |
|---|---|---|---|---|---|---|---|
| — | Hardware address space field | [RFC 826 — Packet format](https://www.rfc-editor.org/rfc/rfc826) | MUST | `Full` | -- | h | [networklayer/arp/ipv4/ArpPacketSerializer.cc:260](../../../src/inet/networklayer/arp/ipv4/ArpPacketSerializer.cc#L260); [networklayer/arp/ipv4/ArpPacket.msg:153](../../../src/inet/networklayer/arp/ipv4/ArpPacket.msg#L153) |
| — | Protocol address space field | [RFC 826 — Packet format](https://www.rfc-editor.org/rfc/rfc826) | MUST | `Full` | -- | h | [networklayer/arp/ipv4/ArpPacketSerializer.cc:261](../../../src/inet/networklayer/arp/ipv4/ArpPacketSerializer.cc#L261); [networklayer/arp/ipv4/ArpPacket.msg:154](../../../src/inet/networklayer/arp/ipv4/ArpPacket.msg#L154) |
| — | Variable length address fields | [RFC 826 — Packet format](https://www.rfc-editor.org/rfc/rfc826) | MUST | `Full` | -- | h | [networklayer/arp/ipv4/ArpPacketSerializer.cc:262-268,278-284](../../../src/inet/networklayer/arp/ipv4/ArpPacketSerializer.cc#L262) |
| — | Opcode field values | [RFC 826 — Packet format](https://www.rfc-editor.org/rfc/rfc826) | MUST | `Full` | -- | h | [networklayer/arp/ipv4/ArpPacket.msg:143-146](../../../src/inet/networklayer/arp/ipv4/ArpPacket.msg#L143); [networklayer/arp/ipv4/ArpPacketSerializer.cc:264](../../../src/inet/networklayer/arp/ipv4/ArpPacketSerializer.cc#L264) |
| — | Network byte order for word fields | [RFC 826 — Packet format](https://www.rfc-editor.org/rfc/rfc826) | MUST | `Full` | -- | h | [networklayer/arp/ipv4/ArpPacketSerializer.cc:260-264,274-280](../../../src/inet/networklayer/arp/ipv4/ArpPacketSerializer.cc#L260) |
| — | ARP link layer type field | [RFC 826 — Packet Generation](https://www.rfc-editor.org/rfc/rfc826) | MUST | `Full` | -- | h | [common/ProtocolGroup.cc:76](../../../src/inet/common/ProtocolGroup.cc#L76); [linklayer/common/EtherType.msg:17](../../../src/inet/linklayer/common/EtherType.msg#L17) |
| — | Cache lookup before transmission | [RFC 826 — Packet Generation](https://www.rfc-editor.org/rfc/rfc826) | MUST | `Full` | -- | h | [networklayer/arp/ipv4/Arp.cc:390-415](../../../src/inet/networklayer/arp/ipv4/Arp.cc#L390) |
| — | Discard of unresolved datagram | [RFC 826 — Packet Generation](https://www.rfc-editor.org/rfc/rfc826) | SHOULD | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:1154-1158,1186-1203](../../../src/inet/networklayer/ipv4/Ipv4.cc#L1154) |
| — | Request field population | [RFC 826 — Packet Generation](https://www.rfc-editor.org/rfc/rfc826) | MUST | `Full` | -- | h | [networklayer/arp/ipv4/Arp.cc:156-159](../../../src/inet/networklayer/arp/ipv4/Arp.cc#L156) |
| — | Unspecified target hardware address | [RFC 826 — Packet Generation](https://www.rfc-editor.org/rfc/rfc826) | MAY | `Full` | -- | h | [networklayer/arp/ipv4/Arp.cc:154-160](../../../src/inet/networklayer/arp/ipv4/Arp.cc#L154) |
| — | Broadcast of ARP requests | [RFC 826 — Packet Generation](https://www.rfc-editor.org/rfc/rfc826) | MUST | `Full` | -- | h | [networklayer/arp/ipv4/Arp.cc:162-164](../../../src/inet/networklayer/arp/ipv4/Arp.cc#L162) |
| <a id="ipv4-075"></a>IPV4-075 | Hardware type acceptance check | [RFC 826 — Packet Reception](https://www.rfc-editor.org/rfc/rfc826) | MUST | `Missing` | cosmetic | h | [networklayer/arp/ipv4/ArpPacket.msg:150-158](../../../src/inet/networklayer/arp/ipv4/ArpPacket.msg#L150); [networklayer/arp/ipv4/Arp.cc:228-285](../../../src/inet/networklayer/arp/ipv4/Arp.cc#L228) |
| <a id="ipv4-076"></a>IPV4-076 | Protocol type acceptance check | [RFC 826 — Packet Reception](https://www.rfc-editor.org/rfc/rfc826) | MUST | `Missing` | cosmetic | h | [networklayer/arp/ipv4/ArpPacket.msg:150-158](../../../src/inet/networklayer/arp/ipv4/ArpPacket.msg#L150); [networklayer/arp/ipv4/Arp.cc:228-285](../../../src/inet/networklayer/arp/ipv4/Arp.cc#L228) |
| — | Merge flag cache update | [RFC 826 — Packet Reception](https://www.rfc-editor.org/rfc/rfc826) | MUST | `Full` | -- | h | [networklayer/arp/ipv4/Arp.cc:273-281,373-388](../../../src/inet/networklayer/arp/ipv4/Arp.cc#L273) |
| <a id="ipv4-151"></a>IPV4-151 | Cache entry creation only when target | [RFC 826 — Packet Reception](https://www.rfc-editor.org/rfc/rfc826) | MUST | `Partial` | results | h | [networklayer/arp/ipv4/Arp.cc:285-304](../../../src/inet/networklayer/arp/ipv4/Arp.cc#L285) |
| <a id="ipv4-152"></a>IPV4-152 | Opcode examined after merging | [RFC 826 — Packet Reception](https://www.rfc-editor.org/rfc/rfc826) | MUST | `Partial` | scope | h | [networklayer/arp/ipv4/Arp.cc:285,306-344](../../../src/inet/networklayer/arp/ipv4/Arp.cc#L285); [networklayer/arp/ipv4/Arp.ned:334](../../../src/inet/networklayer/arp/ipv4/Arp.ned#L334) |
| — | Reply construction by field swap | [RFC 826 — Packet Reception](https://www.rfc-editor.org/rfc/rfc826) | MUST | `Full` | -- | h | [networklayer/arp/ipv4/Arp.cc:318-325,368-371](../../../src/inet/networklayer/arp/ipv4/Arp.cc#L318) |
| — | Reply delivery to requester | [RFC 826 — Packet Reception](https://www.rfc-editor.org/rfc/rfc826) | MUST | `Full` | -- | h | [networklayer/arp/ipv4/Arp.cc:328-341](../../../src/inet/networklayer/arp/ipv4/Arp.cc#L328) |
| <a id="ipv4-077"></a>[IPV4-077](#f5) | Probe before using an address | [RFC 5227 §2.1](https://www.rfc-editor.org/rfc/rfc5227#section-2.1) | MUST | `Missing` | scope | h | [networklayer/arp/ipv4/Arp.cc:487-516](../../../src/inet/networklayer/arp/ipv4/Arp.cc#L487); [networklayer/arp/ipv4/Arp.ned](../../../src/inet/networklayer/arp/ipv4/Arp.ned) |
| — | No periodic probing | [RFC 5227 §2.1](https://www.rfc-editor.org/rfc/rfc5227#section-2.1) | MUST | `Full` | -- | h | [networklayer/arp/ipv4/Arp.cc:487-516](../../../src/inet/networklayer/arp/ipv4/Arp.cc#L487) |
| <a id="ipv4-078"></a>[IPV4-078](#f5) | ARP Probe packet format | [RFC 5227 §2.1.1](https://www.rfc-editor.org/rfc/rfc5227#section-2.1.1) | MUST | `Partial` | scope | h | [networklayer/arp/ipv4/Arp.cc:487-516](../../../src/inet/networklayer/arp/ipv4/Arp.cc#L487) |
| <a id="ipv4-079"></a>[IPV4-079](#f5) | Probe count and spacing | [RFC 5227 §2.1.1](https://www.rfc-editor.org/rfc/rfc5227#section-2.1.1) | SHOULD | `Missing` | scope | h | [networklayer/arp/ipv4/Arp.cc](../../../src/inet/networklayer/arp/ipv4/Arp.cc); [networklayer/arp/ipv4/Arp.ned](../../../src/inet/networklayer/arp/ipv4/Arp.ned) |
| <a id="ipv4-080"></a>[IPV4-080](#f5) | Conflict detected while probing | [RFC 5227 §2.1.1](https://www.rfc-editor.org/rfc/rfc5227#section-2.1.1) | MUST | `Missing` | scope | h | [networklayer/arp/ipv4/Arp.cc:228-366](../../../src/inet/networklayer/arp/ipv4/Arp.cc#L228) |
| <a id="ipv4-081"></a>[IPV4-081](#f5) | Simultaneous probe conflict | [RFC 5227 §2.1.1](https://www.rfc-editor.org/rfc/rfc5227#section-2.1.1) | SHOULD | `Missing` | scope | h | [networklayer/arp/ipv4/Arp.cc:228-366](../../../src/inet/networklayer/arp/ipv4/Arp.cc#L228) |
| <a id="ipv4-082"></a>[IPV4-082](#f5) | Address claiming rate limit | [RFC 5227 §2.1.1](https://www.rfc-editor.org/rfc/rfc5227#section-2.1.1) | MUST | `Missing` | scope | h | [networklayer/arp/ipv4/Arp.cc](../../../src/inet/networklayer/arp/ipv4/Arp.cc); [networklayer/arp/ipv4/Arp.ned](../../../src/inet/networklayer/arp/ipv4/Arp.ned) |
| <a id="ipv4-083"></a>[IPV4-083](#f5) | Announcing an address | [RFC 5227 §2.3](https://www.rfc-editor.org/rfc/rfc5227#section-2.3) | MUST | `Partial` | scope | h | [networklayer/arp/ipv4/Arp.cc:456](../../../src/inet/networklayer/arp/ipv4/Arp.cc#L456) (and 469-479) |
| <a id="ipv4-084"></a>[IPV4-084](#f5) | Ongoing address conflict detection | [RFC 5227 §2.4](https://www.rfc-editor.org/rfc/rfc5227#section-2.4) | MUST | `Missing` | results | h | [networklayer/arp/ipv4/Arp.cc:265-281](../../../src/inet/networklayer/arp/ipv4/Arp.cc#L265) |
| <a id="ipv4-085"></a>[IPV4-085](#f5) | Single defensive announcement | [RFC 5227 §2.4](https://www.rfc-editor.org/rfc/rfc5227#section-2.4) | MAY | `Missing` | scope | h | [networklayer/arp/ipv4/Arp.cc:441-483](../../../src/inet/networklayer/arp/ipv4/Arp.cc#L441) |
| <a id="ipv4-086"></a>[IPV4-086](#f5) | Cease or defer on repeated conflict | [RFC 5227 §2.4](https://www.rfc-editor.org/rfc/rfc5227#section-2.4) | MUST | `Missing` | scope | h | [networklayer/arp/ipv4/Arp.cc:228-366](../../../src/inet/networklayer/arp/ipv4/Arp.cc#L228) |
| <a id="ipv4-087"></a>[IPV4-087](#f2) | Answering requests while address in use | [RFC 5227 §2.5](https://www.rfc-editor.org/rfc/rfc5227#section-2.5) | MUST | `Incorrect` | scope | h | [networklayer/arp/ipv4/Arp.cc:270-271](../../../src/inet/networklayer/arp/ipv4/Arp.cc#L270) |
| — | ARP cache flush mechanism | [RFC 1122 §2.3.2.1](https://www.rfc-editor.org/rfc/rfc1122#section-2.3.2.1) | MUST | `Full` | -- | h | [networklayer/arp/ipv4/Arp.cc:413-421,432-435](../../../src/inet/networklayer/arp/ipv4/Arp.cc#L413) |
| — | ARP cache timeout configurable | [RFC 1122 §2.3.2.1](https://www.rfc-editor.org/rfc/rfc1122#section-2.3.2.1) | SHOULD | `Full` | -- | h | [networklayer/arp/ipv4/Arp.ned:333](../../../src/inet/networklayer/arp/ipv4/Arp.ned#L333); [networklayer/arp/ipv4/Arp.cc:52](../../../src/inet/networklayer/arp/ipv4/Arp.cc#L52) |
| — | ARP request rate limiting | [RFC 1122 §2.3.2.1](https://www.rfc-editor.org/rfc/rfc1122#section-2.3.2.1) | MUST | `Full` | -- | m | [networklayer/arp/ipv4/Arp.cc:179-190,408-412](../../../src/inet/networklayer/arp/ipv4/Arp.cc#L179); [networklayer/arp/ipv4/Arp.ned:331](../../../src/inet/networklayer/arp/ipv4/Arp.ned#L331) |

### 3f. Router forwarding path

14 requirements. 5 `Full`, 2 `Incorrect`, 3 `Missing`, 1 `Non-gap`, 3 `Partial`.

| ID | Feature | Spec clause | Level | Status | Impact | Conf. | Evidence |
|---|---|---|---|---|---|---|---|
| — | Broadcast source address prohibition | [RFC 1122 §3.2.1.3](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.1.3) | MUST | `Full` | -- | m | [networklayer/ipv4/Ipv4.cc:1063](../../../src/inet/networklayer/ipv4/Ipv4.cc#L1063); [networklayer/ipv4/Ipv4.cc:922-925](../../../src/inet/networklayer/ipv4/Ipv4.cc#L922) |
| <a id="ipv4-088"></a>IPV4-088 | Broadcast address form recognition | [RFC 1122 §3.3.6](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.6) | MUST | `Partial` | scope | h | [networklayer/ipv4/Ipv4.cc:391](../../../src/inet/networklayer/ipv4/Ipv4.cc#L391); [networklayer/ipv4/Ipv4RoutingTable.cc:337-348](../../../src/inet/networklayer/ipv4/Ipv4RoutingTable.cc#L337) |
| <a id="ipv4-089"></a>[IPV4-089](#f1) | IP header validation on receipt | [RFC 1812 §5.2.2](https://www.rfc-editor.org/rfc/rfc1812#section-5.2.2) | MUST | `Incorrect` | results | h | [networklayer/ipv4/Ipv4.cc:278,282](../../../src/inet/networklayer/ipv4/Ipv4.cc#L278); [networklayer/ipv4/Ipv4HeaderSerializer.cc:177-184](../../../src/inet/networklayer/ipv4/Ipv4HeaderSerializer.cc#L177); common… |
| <a id="ipv4-090"></a>IPV4-090 | Truncated packet detection | [RFC 1812 §5.2.2](https://www.rfc-editor.org/rfc/rfc1812#section-5.2.2) | SHOULD | `Partial` | cosmetic | h | [networklayer/ipv4/Ipv4.cc:291-295](../../../src/inet/networklayer/ipv4/Ipv4.cc#L291); [networklayer/ipv4/IcmpHeader.msg:24-30](../../../src/inet/networklayer/ipv4/IcmpHeader.msg#L24) |
| <a id="ipv4-091"></a>[IPV4-091](#f1) | Header checksum verification | [RFC 1812 §4.2.2.5](https://www.rfc-editor.org/rfc/rfc1812#section-4.2.2.5) | MUST | `Incorrect` | results | h | [networklayer/ipv4/Ipv4.cc:282](../../../src/inet/networklayer/ipv4/Ipv4.cc#L282); [networklayer/ipv4/Ipv4Header.cc:97-118](../../../src/inet/networklayer/ipv4/Ipv4Header.cc#L97) |
| — | Longest match route lookup | [RFC 1812 §5.2.4.3](https://www.rfc-editor.org/rfc/rfc1812#section-5.2.4.3) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4RoutingTable.cc:365-382](../../../src/inet/networklayer/ipv4/Ipv4RoutingTable.cc#L365); [networklayer/ipv4/Ipv4RoutingTable.cc:435-450](../../../src/inet/networklayer/ipv4/Ipv4RoutingTable.cc#L435) |
| <a id="ipv4-092"></a>[IPV4-092](#f8) | Martian source address filtering | [RFC 1812 §5.3.7](https://www.rfc-editor.org/rfc/rfc1812#section-5.3.7) | SHOULD | `Missing` | scope | h | [networklayer/ipv4/Ipv4.cc:269-295](../../../src/inet/networklayer/ipv4/Ipv4.cc#L269) and 343-421 (receive path); no source-address test anywher… |
| <a id="ipv4-093"></a>[IPV4-093](#f8) | Martian destination filtering | [RFC 1812 §5.3.7](https://www.rfc-editor.org/rfc/rfc1812#section-5.3.7) | SHOULD | `Missing` | scope | h | [networklayer/ipv4/Ipv4.cc:383-421](../../../src/inet/networklayer/ipv4/Ipv4.cc#L383) (preroutingFinish); no destination-validity test in Ipv4.cc |
| <a id="ipv4-094"></a>IPV4-094 | Link layer broadcast not forwarded | [RFC 1812 §5.3.4](https://www.rfc-editor.org/rfc/rfc1812#section-5.3.4) | MUST | `Missing` | scope | h | [networklayer/ipv4/Ipv4.cc:269-295](../../../src/inet/networklayer/ipv4/Ipv4.cc#L269); no MacAddressInd or link-layer broadcast test in Ipv4.cc |
| — | Limited broadcast handling | [RFC 1812 §5.3.5.1](https://www.rfc-editor.org/rfc/rfc1812#section-5.3.5.1) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:391-407](../../../src/inet/networklayer/ipv4/Ipv4.cc#L391) |
| <a id="ipv4-095"></a>[IPV4-095](#f8) | Network redirect prohibited | [RFC 1812 §5.2.7.2](https://www.rfc-editor.org/rfc/rfc1812#section-5.2.7.2) | MUST | `Partial` | scope | h | [networklayer/ipv4/Icmp.cc:261-265](../../../src/inet/networklayer/ipv4/Icmp.cc#L261); no ICMP_REDIRECT generation anywhere in networklayer/ipv4 |
| — | Redirects ignored for forwarding | [RFC 1812 §5.2.7.2](https://www.rfc-editor.org/rfc/rfc1812#section-5.2.7.2) | MUST | `Full` | -- | h | [networklayer/ipv4/Icmp.cc:261-265](../../../src/inet/networklayer/ipv4/Icmp.cc#L261); [networklayer/contract/IRoute.h:33](../../../src/inet/networklayer/contract/IRoute.h#L33) |
| <a id="ipv4-096"></a>IPV4-096 | Congestion drop precedence ordering | [RFC 1812 §5.3.6](https://www.rfc-editor.org/rfc/rfc1812#section-5.3.6) | MUST | `Non-gap` | -- | m | [networklayer/ipv4/Ipv4.cc](../../../src/inet/networklayer/ipv4/Ipv4.cc) (no queue); networklayer/diffserv, queueing |
| — | Unrecognized codepoint handling | [RFC 2474 §3](https://www.rfc-editor.org/rfc/rfc2474#section-3) | SHOULD | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:320-326](../../../src/inet/networklayer/ipv4/Ipv4.cc#L320); [networklayer/ipv4/Ipv4.cc:343-421](../../../src/inet/networklayer/ipv4/Ipv4.cc#L343) |


### 3g. Wire format, field by field

#### Ipv4HeaderSerializer.cc

| # | Field or behavior | Spec clause | What the code does | Verdict | Evidence |
|---|---|---|---|---|---|
| — | Version (bits 0-3), IHL (bits 4-7) | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | Both taken from the object via the `struct ip` bitfields, endianness selected by `BYTE_ORDER`; IHL = headerLength>>2, deserialize reverses it | Full | Ipv4HeaderSerializer.cc:25-26, 176; [headers/ip.h:56-63](../../../src/inet/networklayer/ipv4/headers/ip.h#L56) |
| — | Whole 20-byte fixed header emitted by `writeBytes((uint8_t*)&iphdr, 20)` | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | Writes the raw C struct; relies on the struct being exactly 20 bytes with no padding (true on the usual ABIs, but not guaranteed) and on `in_addr` being 4 bytes | Full | Ipv4HeaderSerializer.cc:46, 158 |
| <a id="ipv4-097"></a>IPV4-097 | Total Length (offset 2, 16 bits) | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) ("Total Length is measured in octets ... maximum 65,535") | `htons(getTotalLengthField().get<B>())` on an int64 `B`; a value above 65535 is silently truncated, never rejected, never marked | Partial | Ipv4HeaderSerializer.cc:42 |
| — | Flags bit 0 (reserved) | [RFC 1812 §5.2.5](https://www.rfc-editor.org/rfc/rfc1812#section-5.2.5) ("MUST ignore and MUST pass through unchanged") | Modelled as `reservedBit`, written from and read back into the object, carried across forwarding and copied into every fragment | Full | Ipv4HeaderSerializer.cc:31-32, 169 |
| — | Fragment Offset (13 bits, 8-octet units) | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | Model stores a byte offset; serializer asserts `(offset & 7)==0`, divides by 8, masks with `IP_OFFMASK`; deserializer multiplies by 8 | Full | Ipv4HeaderSerializer.cc:29-30, 172 |
| — | Header Checksum (offset 10) | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | Written from the object; serialize refuses any mode other than `CHECKSUM_COMPUTED` (a deliberate INET restriction, not a wire defect) | Full | Ipv4HeaderSerializer.cc:43-45 |
| — | Header padding to a 32-bit boundary | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) "Padding ... zero" | Pads with repeated `IPOPTION_END_OF_OPTIONS` (0x00), which is byte-identical to zero padding | Full | Ipv4HeaderSerializer.cc:61-62 |
| <a id="ipv4-098"></a>IPV4-098 | Option area is written only when `headerLength > 20` | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | The whole option loop is gated on IHL > 5. An option present in the object but not accounted for in `headerLength` is silently dropped from the wire with no error | Incorrect? | Ipv4HeaderSerializer.cc:48-63 |
| <a id="ipv4-099"></a>IPV4-099 | `enableTimestampOption` builds a Timestamp option of length 0 | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) Internet Timestamp ("Length" octet, minimum 4) | Verified as reported. `Ipv4OptionTimestamp` inherits `length = 0` from `TlvOptionBase`; `Ipv4.cc` never sets it and never calls `addChunkLength`, and it runs *after* `setHeaderLength`/`setTotalLengthField`. Combined with row 8 the option never reaches the wire at all | Incorrect? | Ipv4.cc:1136-1141; Ipv4Header.msg:115-128; TlvOptions.msg:12-16; Ipv4HeaderSerializer.cc:48 |
| <a id="ipv4-100"></a>IPV4-100 | Header length arithmetic for options from `Ipv4OptionsReq` | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) ("padding is used to ensure that the internet header ends on a 32 bit boundary") | `addChunkLength(B(opt->getLength()))` sums raw option lengths and `setHeaderLength(getChunkLength())` copies the sum verbatim — no round-up to 4. `Ipv4Header::calculateHeaderByteLength()` does the round-up but is never called here, so any option set whose total is not a multiple of 4 trips the serializer's own assertion | Incorrect? | Ipv4.cc:1105-1114; Ipv4Header.cc:42-48; Ipv4HeaderSerializer.cc:23 |
| <a id="ipv4-101"></a>IPV4-101 | Internet Timestamp option, deserialize of the data area | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) Internet Timestamp | The read loop is bounded by `getRecordAddressArraySize()`, but that array is only sized when `bytes == 8`. For flag 0 (timestamp-only, `bytes == 4`) the size is 0, so **no timestamps are read and the stream is left short by (length-4) bytes** — every following option and the chunk length are then wrong | Incorrect? | Ipv4HeaderSerializer.cc:248-256 |
| <a id="ipv4-102"></a>IPV4-102 | Record Route / LSRR / SSRR option length | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) ("The originating host must compose this option with a large enough route data area ... The size of the option does not change due to adding addresses. The initial contents of the route data area must be zero.") | Both serialize and deserialize define `length == 3 + 4 * recordAddressArraySize` — i.e. the option carries only the addresses already recorded and grows as it is forwarded. Empty slots and the "pointer > length means full" convention cannot be represented | Incorrect? | Ipv4HeaderSerializer.cc:128, 267-272; rfc791.txt:1420-1424, 1524-1526 |
| <a id="ipv4-103"></a>IPV4-103 | Option length octet is written only when `length > 1` | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) (all options except types 0 and 1 carry a length octet) | Asymmetric: the writer suppresses the length octet for any option of length 1, while the reader unconditionally reads a length octet for every type outside {0,1}. Only assertions keep the two in step | Partial | Ipv4HeaderSerializer.cc:77-79 vs 219, 265, 282, 294 |
| <a id="ipv4-104"></a>IPV4-104 | Unknown / malformed option fallback (`TlvOptionRaw`) | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) (length octet counts the type and length octets, so length >= 2) | On the fallback path 2 bytes are always consumed but the recorded `length` is whatever the wire said — 0 or 1 are accepted without `markIncorrect()`. Re-serializing such an option then produces a byte count different from the one consumed | Incorrect? | Ipv4HeaderSerializer.cc:298-308 |
| <a id="ipv4-105"></a>IPV4-105 | Option area may run past IHL | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) (options live inside the header, whose extent is IHL) | The loop condition is checked only *before* each option; an option whose length runs past IHL is parsed in full, eating payload bytes. Nothing calls `markIncorrect()`, and the resulting chunk length then disagrees with `getHeaderLength()`, which is what `Ipv4FragBuf` and `Icmp` use to locate the payload | Missing | Ipv4HeaderSerializer.cc:189-194; Ipv4FragBuf.cc:60; Icmp.cc:133 |
| <a id="ipv4-106"></a>IPV4-106 | `checksumMode` after a round trip | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | Deserialize always sets `CHECKSUM_COMPUTED`, whatever the sender used, so the mode is a field the round trip does not preserve | Partial | Ipv4HeaderSerializer.cc:199-200 |
| <a id="ipv4-107"></a>IPV4-107 | Does any caller act on `markIncorrect()`? | [RFC 1122 §3.2.1.2](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.1.2) ("A host MUST verify the IP header checksum on every received datagram and silently discard every datagram that has a bad checksum") | `Ipv4::handlePacketFromNetwork` guards with `if (!isCorrect() && !verifyChecksum())`. Because of the `&&`, `verifyChecksum()` is short-circuited away for every chunk that is *not* already flagged — which is the normal case. A datagram whose checksum field is simply wrong (or `CHECKSUM_DECLARED_INCORRECT`) is accepted. `Icmp::processIcmpMessage` gets the same test right (`if (!verifyChecksum(packet))`) | Incorrect? | Ipv4.cc:282; Icmp.cc:249; Ipv4HeaderSerializer.cc:178-186 |
| <a id="ipv4-108"></a>IPV4-108 | Timestamp option `oflw` (4 bits) / `flg` (4 bits) octet | [RFC 791 §3.1 ("Overflow (oflw) [4 bits]")](https://www.rfc-editor.org/rfc/rfc791) | `flagbyte = overflow << 4 \ | flag` with `overflow` a `short`; a value above 15 corrupts the neighbouring bits. No clamp, no assertion | Partial | Ipv4HeaderSerializer.cc:114 |

#### IcmpHeaderSerializer.cc

| # | Field or behavior | Spec clause | What the code does | Verdict | Evidence |
|---|---|---|---|---|---|
| — | Type (byte 0), Code (byte 1), Checksum (bytes 2-3) | [RFC 792](https://www.rfc-editor.org/rfc/rfc792), all messages | Written from and read back into the object for every supported type | Full | IcmpHeaderSerializer.cc:22-24, 61-64 |
| <a id="ipv4-109"></a>IPV4-109 | Code octet when never assigned | [RFC 792](https://www.rfc-editor.org/rfc/rfc792) | `IcmpHeader.code` defaults to `-1`; `writeByte(getCode())` then emits 0xFF. Only `IcmpEchoRequest`/`Reply`/`Ptb` override the default | Partial | IcmpHeader.msg:27; IcmpHeaderSerializer.cc:23 |
| — | Echo / Echo Reply Identifier + Sequence Number (bytes 4-7) | [RFC 792](https://www.rfc-editor.org/rfc/rfc792) Echo or Echo Reply | Both written from and read into the object, big endian | Full | IcmpHeaderSerializer.cc:28-29, 34-35, 73-74, 84-85 |
| — | Destination Unreachable / Time Exceeded second word | [RFC 792](https://www.rfc-editor.org/rfc/rfc792) ("unused ... reserved for later extensions and must be zero") | Written as a constant 0 and discarded on read — which is exactly what the RFC asks for | Full | IcmpHeaderSerializer.cc:45-51, 101-104; rfc792.txt:70 |
| — | Destination Unreachable code 4, next-hop MTU (bytes 6-7) | [RFC 1191 §4](https://www.rfc-editor.org/rfc/rfc1191#section-4) | `IcmpPtb` models both the 2 unused bytes and the MTU, and both round-trip | Full | IcmpHeaderSerializer.cc:41-42, 96-97 |
| <a id="ipv4-110"></a>IPV4-110 | Parameter Problem (type 12): Pointer octet + 3 unused octets | [RFC 792](https://www.rfc-editor.org/rfc/rfc792) Parameter Problem, "Pointer" | Not modelled at all, and the serializer's `default:` arm throws. `Ipv4` *does* generate type 12 (twice), so those messages cannot cross a serializing link | Missing | IcmpHeaderSerializer.cc:53-54; rfc792.txt:420, 448; Ipv4.cc:307, 292 |
| <a id="ipv4-111"></a>IPV4-111 | Redirect (type 5): Gateway Internet Address (bytes 4-7) | [RFC 792](https://www.rfc-editor.org/rfc/rfc792) Redirect, "Gateway Internet Address" | Not modelled; the serializer throws for type 5 | Missing | IcmpHeaderSerializer.cc:53-54; rfc792.txt:652, 686 |
| <a id="ipv4-112"></a>IPV4-112 | Types 4, 9, 10, 15, 16, 17, 18 | [RFC 792](https://www.rfc-editor.org/rfc/rfc792) (Source Quench, Information Request/Reply); RFC 1122 3.2.2.9 (Address Mask) | All enumerated in `IcmpType.msg` and reachable through `sendErrorMessage`, but the serializer's `default:` arm throws for every one of them | Missing | IcmpHeaderSerializer.cc:53-54; IcmpType.msg:19-33 |
| <a id="ipv4-113"></a>IPV4-113 | Timestamp / Timestamp Reply (types 13/14): Identifier, Sequence, Originate/Receive/Transmit Timestamp — 20 octets | [RFC 792](https://www.rfc-editor.org/rfc/rfc792) Timestamp or Timestamp Reply | Not modelled. `Icmp` routes a received type 13 into `processEchoRequest`, which peeks it as an `IcmpEchoRequest` and answers with a type **0** Echo Reply carrying no timestamps | Incorrect? | Icmp.cc:317-319, 335-348; rfc792.txt:886, 950 |
| <a id="ipv4-114"></a>IPV4-114 | Deserialize of an unrecognised type | [RFC 792](https://www.rfc-editor.org/rfc/rfc792) (every message has an 8-octet header) | Only 4 bytes are consumed, then `markImproperlyRepresented()`. `FieldsChunkSerializer::deserialize` sets the chunk length from the bytes actually consumed, so the chunk becomes 4 bytes and the remaining 4 header octets are reinterpreted as ICMP payload | Incorrect? | IcmpHeaderSerializer.cc:106-109; FieldsChunkSerializer.cc:28-40 |
| <a id="ipv4-115"></a>IPV4-115 | Does any caller act on `markImproperlyRepresented()`? | [RFC 1122 §3.2.2](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.2) ("If an ICMP message of unknown type is received, it MUST be silently discarded") | Nothing in the IPv4/ICMP path ever calls `isProperlyRepresented()` (the only caller in the tree is the Ethernet PHY checker). `Icmp::processIcmpMessage` instead throws `cRuntimeError("Unknown ICMP type %d")`, aborting the simulation on a legal-but-unsupported message | Incorrect? | Icmp.cc:325-326; rfc1122.txt:2222-2223 |

#### ArpPacketSerializer.cc

| # | Field or behavior | Spec clause | What the code does | Verdict | Evidence |
|---|---|---|---|---|---|
| <a id="ipv4-116"></a>IPV4-116 | `ar$hrd` (offset 0, 16 bits) | [RFC 826](https://www.rfc-editor.org/rfc/rfc826) packet format | Written as the literal `1`; `ArpPacket` has no field for it. A non-Ethernet hardware type cannot be expressed | Missing | ArpPacketSerializer.cc:38; ArpPacket.msg:30-34 |
| <a id="ipv4-117"></a>IPV4-117 | `ar$pro` (offset 2, 16 bits) | [RFC 826](https://www.rfc-editor.org/rfc/rfc826) packet format | Written as the literal `ETHERTYPE_IPv4`; not modelled. RARP opcodes 3/4 exist in the enum but the protocol type is pinned to IPv4 | Missing | ArpPacketSerializer.cc:39; ArpPacket.msg:20-23 |
| <a id="ipv4-118"></a>IPV4-118 | `ar$hln` (offset 4) and `ar$pln` (offset 5) | [RFC 826](https://www.rfc-editor.org/rfc/rfc826) packet format | Written as the literals `MAC_ADDRESS_SIZE` and `4`; not modelled | Missing | ArpPacketSerializer.cc:40-41 |
| — | `ar$op` (offset 6, 16 bits) | [RFC 826](https://www.rfc-editor.org/rfc/rfc826) packet format | From the object, big endian, read back into the object | Full | ArpPacketSerializer.cc:42, 58 |
| — | `ar$sha`, `ar$spa`, `ar$tha`, `ar$tpa` | [RFC 826](https://www.rfc-editor.org/rfc/rfc826) packet format | Correct order and widths for hln=6/pln=4; all four round-trip | Full | ArpPacketSerializer.cc:43-46, 59-62 |
| <a id="ipv4-119"></a>IPV4-119 | Deserialize marks a wrong hardware/protocol type incorrect | [RFC 826](https://www.rfc-editor.org/rfc/rfc826) Packet Reception ("Negative conditionals indicate an end of processing and a discarding of the packet") | Verified as reported. `markIncorrect()` is called for `ar$hrd != 1` and for `ar$pro != 0x0800`, the two values are then discarded, and `Arp::processArpPacket` never calls `isCorrect()` — it peeks the chunk and processes it regardless | Incorrect? | ArpPacketSerializer.cc:52-55; Arp.cc:231-232; rfc826.txt:200-203 |
| <a id="ipv4-120"></a>IPV4-120 | Address fields read with the on-wire `ar$hln`/`ar$pln` as the stride | [RFC 826](https://www.rfc-editor.org/rfc/rfc826) ("nbytes ... n from the ar$hln field") | `readMacAddress()` always reads 48 bits and only *afterwards* seeks to `curpos + size`. With `ar$hln < 6` the returned MAC is built from bytes belonging to the following field; with `ar$hln > 6` the high-order bytes are dropped. The same holds for `readIpv4Address` and `ar$pln` | Incorrect? | ArpPacketSerializer.cc:19-33; MemoryInputStream.h:439, 447 |
| <a id="ipv4-121"></a>IPV4-121 | `ar$hln`/`ar$pln` are never validated | [RFC 826](https://www.rfc-editor.org/rfc/rfc826) ("optionally check the hardware length ar$hln", "optionally check the protocol length ar$pln") | Neither length is range-checked and neither triggers `markIncorrect()`. A large `ar$hln` makes `stream.seek(curpos + B(size))` exceed the stream, tripping the `ASSERT` inside `seek` — a remotely reachable abort rather than a discarded packet | Incorrect? | ArpPacketSerializer.cc:23, 56-57; MemoryInputStream.h:128-131 |
| <a id="ipv4-122"></a>IPV4-122 | Receive path rejects a zero sender protocol address | [RFC 826](https://www.rfc-editor.org/rfc/rfc826) Packet Reception (unrecognised packets are discarded, not fatal); RFC 5227 2.1.1 (an ARP Probe carries `ar$spa` = 0) | `Arp::processArpPacket` throws `cRuntimeError("wrong ARP packet: source IPv4 address is empty")`. INET's own `Arp::sendArpProbe` emits exactly such a packet, so an ARP Probe reaching any INET `Arp` aborts the simulation | Incorrect? | Arp.cc:268-271, 497-501 |

### 3h. Timers and constants

| # | Constant | Where defined | Value | Spec clause | Spec says | Verdict | Evidence |
|---|---|---|---|---|---|---|---|
| — | Reassembly timeout | NED parameter `Ipv4.fragmentTimeout` | 60s | [RFC 1122 §3.3.2](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.2) | "There MUST be a reassembly timeout ... SHOULD be a fixed value, not set from the remaining TTL ... recommended that the value lie between 60 seconds and 120 seconds" | Full | Ipv4.ned:100; rfc1122.txt:3336-3341 |
| <a id="ipv4-123"></a>IPV4-123 | Stale-fragment sweep granularity | C++ literal `10` in a `simtime_t` comparison | 10s | [RFC 1122 §3.3.2](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.2) | Same clause: the timeout is the guaranteed discard point; a hardcoded 10s coarsening pushes the effective timeout to 60-70s and cannot be configured | Partial | Ipv4.cc:833-836 |
| <a id="ipv4-124"></a>[IPV4-124](#f3) | Reassembly timeout is not implemented as a timer | `Ipv4::reassembleAndDeliver` calls `fragbuf.purgeStaleFragments` only on the arrival of another fragment | n/a | [RFC 1122 §3.3.2](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.2) | "If this timeout expires, the partially-reassembled datagram MUST be discarded and an ICMP Time Exceeded message sent to the source host". If no further fragment ever arrives on that node, no sweep runs, nothing is discarded and no ICMP is sent — the buffer leaks | Missing | Ipv4.cc:833-836; Ipv4FragBuf.cc:99-128 |
| <a id="ipv4-125"></a>IPV4-125 | Per-datagram reassembly deadline | `Ipv4FragBuf::addFragment` sets `curBuf->lastupdate = now` on every non-completing fragment | restarts on each fragment | [RFC 1122 §3.3.2](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.2) | The timeout "SHOULD be a fixed value". As written it is an idle timer, so a slow drip of fragments keeps a datagram alive indefinitely | Incorrect? | Ipv4FragBuf.cc:94 |
| <a id="ipv4-126"></a>[IPV4-126](#f7) | EMTU_R / reassembly buffer bound | none — no limit anywhere | unbounded | [RFC 1122 §3.3.2](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.2) | EMTU_R "MUST be greater than or equal to 576, SHOULD be either configurable or indefinite". Indefinite is permitted, so the absence of a limit is legal; but `addFragment` also never checks `offset + len <= 65535` and asserts on `totalLengthField <= headerLength`, so a malformed fragment aborts instead of being dropped | Partial | Ipv4FragBuf.cc:58-62; rfc1122.txt:3285-3290 |
| <a id="ipv4-127"></a>IPV4-127 | Default unicast TTL | NED parameter `Ipv4.timeToLive` | 32 | [RFC 1122 §3.2.1.7](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.1.7); RFC 1812 4.2.2.9 | Configurability is met ("When a fixed TTL value is used, it MUST be configurable"), but the value contradicts the recommendation: "this argues for a default TTL value in excess of 40, and 64 is a common value" | Partial | Ipv4.ned:98; rfc1812.txt:2584-2585 |
| <a id="ipv4-128"></a>IPV4-128 | Default multicast TTL | NED parameter `Ipv4.multicastTimeToLive` | 32 | — (no clause in the cache bounds it) | not assessed | Non-gap | Ipv4.ned:99 |
| <a id="ipv4-129"></a>IPV4-129 | TTL for link-local multicast | C++ literal in `Ipv4::encapsulate` | 1 | — ([RFC 1112](https://www.rfc-editor.org/rfc/rfc1112) not in the cache) | not assessed | Non-gap | Ipv4.cc:1097 |
| <a id="ipv4-130"></a>IPV4-130 | `IPDEFTTL` = 64, `MAXTTL` = 255 | `#define` in the imported BSD header | 64 / 255 | [RFC 1812 §4.2.2.9](https://www.rfc-editor.org/rfc/rfc1812#section-4.2.2.9) | Dead constants — grep shows no use anywhere in the tree. The one value that matches the RFC's "common value" of 64 is the one the model does not use | Non-gap | [headers/ip.h:159-160](../../../src/inet/networklayer/ipv4/headers/ip.h#L159) |
| <a id="ipv4-131"></a>IPV4-131 | ICMP quote length | NED parameter `Icmp.quoteLength` | 8B | [RFC 1812 §4.3.2.3](https://www.rfc-editor.org/rfc/rfc1812#section-4.3.2.3) | "Historically, every ICMP error message has included the Internet header and at least the first 8 data bytes ... This is no longer adequate ... the ICMP datagram SHOULD contain as much of the original datagram as possible without the length of the ICMP datagram exceeding 576 bytes" — the default is the deprecated behaviour | Partial | Icmp.ned:23; Icmp.cc:180, 209; rfc1812.txt:2944-2950 |
| — | ICMP quote length floor | C++ literal `B(8)` in `parseQuoteLengthParameter` | 8B minimum, enforced by throw | [RFC 792](https://www.rfc-editor.org/rfc/rfc792) (error messages carry "the internet header + 64 bits of the original data") | 8 bytes is exactly the RFC 792 floor | Full | Icmp.cc:45-46 |
| <a id="ipv4-132"></a>[IPV4-132](#f7) | ICMP error rate limit | nowhere — no token bucket, no timer, no NED parameter | absent | [RFC 1812 §4.3.2.8](https://www.rfc-editor.org/rfc/rfc1812#section-4.3.2.8) | "A router which sends ICMP Source Quench messages MUST be able to limit the rate at which the messages can be generated. A router SHOULD also be able to limit the rate at which it sends other sorts of ICMP error messages ... The rate limit parameters SHOULD be settable as part of the configuration" | Missing | Icmp.cc:189-217 (no limiter on the only error-generation path); Icmp.ned:18-24 (no parameter) |
| — | ARP retry timeout | NED parameter `Arp.retryTimeout` | 1s | [RFC 1122 §2.3.2.1](https://www.rfc-editor.org/rfc/rfc1122#section-2.3.2.1) | "The recommended maximum rate is 1 per second per destination" | Full | Arp.ned:42; Arp.cc:136, 188 |
| <a id="ipv4-133"></a>IPV4-133 | ARP retry count | NED parameter `Arp.retryCount` | 3 (3 requests total: initial + 2 retries, then failure) | — (no clause in the cache fixes a count) | not assessed | Non-gap | Arp.ned:43; Arp.cc:182-190 |
| <a id="ipv4-134"></a>[IPV4-134](#f7) | ARP flood prevention after a failed resolution | none — the cache entry is erased on failure with no negative entry and no hold-down | absent | [RFC 1122 §2.3.2.1](https://www.rfc-editor.org/rfc/rfc1122#section-2.3.2.1) | "A mechanism to prevent ARP flooding (repeatedly sending an ARP Request for the same IP address, at a high rate) MUST be included." After `requestTimedOut` erases the entry, the very next packet for that address starts a fresh resolution immediately, so the request rate is bounded by the offered load, not by 1/s | Missing | Arp.cc:196-201; Arp.cc:396-406 |
| — | ARP cache timeout | NED parameter `Arp.cacheTimeout` | 120s | [RFC 1122 §2.3.2.1](https://www.rfc-editor.org/rfc/rfc1122#section-2.3.2.1) | "MUST provide a mechanism to flush out-of-date cache entries. If this mechanism involves a timeout, it SHOULD be possible to configure the timeout value" | Full | Arp.ned:44 |
| <a id="ipv4-135"></a>IPV4-135 | ARP cache expiry mechanism | lazy — `lastUpdate + cacheTimeout >= simTime()` tested only inside `resolveL3Address` and `getL3AddressFor`; no sweep, no timer | n/a | [RFC 1122 §2.3.2.1](https://www.rfc-editor.org/rfc/rfc1122#section-2.3.2.1) | The flush requirement is met only for addresses that are looked up again; an address never queried again keeps its entry (and its memory) for the whole run, and `getL3AddressFor` scans the unbounded map linearly | Partial | Arp.cc:413-421, 432-437 |
| <a id="ipv4-136"></a>IPV4-136 | `MAX_IPADDR_OPTION_ENTRIES` = 9, `MAX_TIMESTAMP_OPTION_ENTRIES` = 4 | `cplusplus{{}}` block in Ipv4Header.msg | 9 / 4 | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | These match the caps the 40-byte option area implies for Record Route and for a timestamp-with-address option, but grep shows neither constant is referenced anywhere: the serializer and `appendRecordAddress` accept any count, bounded only by the `headerLength <= 60` assertion | Partial | Ipv4Header.msg:35-36; Ipv4HeaderSerializer.cc:111, 128 |
| <a id="ipv4-137"></a>IPV4-137 | Multicast TTL threshold | C++ member default in `Ipv4InterfaceData::RouterMulticastData` | 0, compared as `ttl <= ttlThreshold` | — (no clause in the cache) | not assessed | Non-gap | Ipv4InterfaceData.h:118-120; Ipv4.cc:797 |
| <a id="ipv4-138"></a>IPV4-138 | IGMPv2/v3 timers (`queryInterval` 125s, `queryResponseInterval` 10s, `lastMemberQueryInterval` 1s, `unsolicitedReportInterval` 10s, `robustnessVariable` 2, and the `HopLimitReq` of 1 at Igmpv2.cc:738) | NED parameters in Igmpv2.ned / Igmpv3.ned, all configurable | as listed | — ([RFC 2236](https://www.rfc-editor.org/rfc/rfc2236) / RFC 3376 are not in the specification cache) | not assessed here; all are NED parameters with derived defaults, none are C++ literals | Non-gap | Igmpv2.ned:102-112; Igmpv3.ned:61-71 |

### 3i. Serializer round trip

**Ipv4HeaderSerializer.** Every field of the 20-byte fixed header round-trips exactly: version, IHL, TOS (and therefore the DSCP/ECN views), total length, identification, all three flag bits including the reserved bit, fragment offset, TTL, protocol, checksum, source and destination. Two things do not survive. `checksumMode` is always rewritten to `CHECKSUM_COMPUTED` on read, so a declared-checksum header comes back as a computed one. And the option area round-trips only for End-of-Option, No-Operation, Stream ID, Router Alert and Record Route; a Timestamp option with flag 0 (timestamp-only) loses every recorded timestamp *and* leaves the stream positioned `length - 4` bytes short, which corrupts every option after it and the chunk length itself. A Record Route option round-trips its bytes but not its meaning: the model's length is derived from the number of addresses already recorded, so the option is re-encoded at a different size than the originator allocated.

**IcmpHeaderSerializer.** Type, code and checksum round-trip for every type the switch handles. Echo Request/Reply round-trip identifier and sequence number; Destination Unreachable code 4 round-trips the unused half-word and the next-hop MTU; the "unused" word of the other Destination Unreachable codes and of Time Exceeded is written as a constant zero and discarded on read, which is what RFC 792 asks for. Nothing else round-trips: types 4, 5, 9, 10, 12 and 13-18 throw on serialize, and on deserialize an unrecognised type consumes only 4 of the 8 header octets, so the chunk comes back 4 bytes long with half the header reinterpreted as payload.

**ArpPacketSerializer.** Only five of the nine RFC 826 fields round-trip: the opcode and the four address fields. `ar$hrd`, `ar$pro`, `ar$hln` and `ar$pln` are written from literals rather than from the object and are discarded on read, so any ARP packet that is not "Ethernet over IPv4 with 6-byte and 4-byte addresses" is normalised to that on the way through. Round-tripping is byte-exact only for hln=6/pln=4; for any other pair the address readers read the wrong number of bytes before repositioning, so the values themselves change.



### 3j. Round 2 — header, checksum, addressing, DS field

24 requirements. 9 `Full`, 1 `Incorrect`, 5 `Missing`, 2 `Non-gap`, 7 `Partial`.

| ID | Feature | Spec clause | Level | Status | Impact | Conf. | Evidence |
|---|---|---|---|---|---|---|---|
| <a id="ipv4-153"></a>IPV4-153 | Header padding to 32-bit boundary | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | MUST | `Partial` | cosmetic | m | [networklayer/ipv4/Ipv4HeaderSerializer.cc:58](../../../src/inet/networklayer/ipv4/Ipv4HeaderSerializer.cc#L58)-61 (pads to headerLength with IPOPTION_END_OF_OPTIONS = 0x00) … |
| <a id="ipv4-154"></a>IPV4-154 | Non-standard timestamp high bit | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | MUST | `Missing` | cosmetic | h | [networklayer/ipv4/Ipv4.cc:1136](../../../src/inet/networklayer/ipv4/Ipv4.cc#L1136)-1140; [networklayer/ipv4/Ipv4HeaderSerializer.cc:118](../../../src/inet/networklayer/ipv4/Ipv4HeaderSerializer.cc#L118) and :253 … |
| <a id="ipv4-155"></a>IPV4-155 | Several addresses per interface | [RFC 791 §3.2](https://www.rfc-editor.org/rfc/rfc791#section-3.2) | MUST | `Partial` | scope | h | [networklayer/ipv4/Ipv4InterfaceData.h:126](../../../src/inet/networklayer/ipv4/Ipv4InterfaceData.h#L126),156,174 (single inetAddr per interface); [networklayer/ipv4/Ipv4.cc:385](../../../src/inet/networklayer/ipv4/Ipv4.cc#L385) … |
| — | Conservative sending liberal receiving | [RFC 791 §3.2](https://www.rfc-editor.org/rfc/rfc791#section-3.2) | MUST | `Full` | -- | m | [networklayer/ipv4/Ipv4.cc:1113](../../../src/inet/networklayer/ipv4/Ipv4.cc#L1113)-1134 (version, IHL, total length, checksum set consistently) … |
| — | Data transmission order | [RFC 791 — Appendix B](https://www.rfc-editor.org/rfc/rfc791) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4HeaderSerializer.cc:29](../../../src/inet/networklayer/ipv4/Ipv4HeaderSerializer.cc#L29)-43 (htons/htonl on every multi-octet field), :106,113-119,131 (writeUint16B … |
| <a id="ipv4-156"></a>IPV4-156 | Rate limiting non-atomic sources | [RFC 6864 §5.2](https://www.rfc-editor.org/rfc/rfc6864#section-5.2) | MUST | `Missing` | scope | h | [networklayer/ipv4/Ipv4.cc:1089](../../../src/inet/networklayer/ipv4/Ipv4.cc#L1089); [networklayer/ipv4/Ipv4.h:84](../../../src/inet/networklayer/ipv4/Ipv4.h#L84) (uint16_t curFragmentId free-running counter) … |
| — | Faster ID reuse with strong checks | [RFC 6864 §5.2](https://www.rfc-editor.org/rfc/rfc6864#section-5.2) | MAY | `Full` | -- | m | [networklayer/ipv4/Ipv4.cc:1089](../../../src/inet/networklayer/ipv4/Ipv4.cc#L1089); [networklayer/ipv4/Ipv4.h:84](../../../src/inet/networklayer/ipv4/Ipv4.h#L84) … |
| <a id="ipv4-157"></a>IPV4-157 | Class selector codepoint mapping | [RFC 2474 §4.2.2.1](https://www.rfc-editor.org/rfc/rfc2474#section-4.2.2.1) | MUST | `Partial` | scope | m | [networklayer/diffserv/Dscp.msg:39](../../../src/inet/networklayer/diffserv/Dscp.msg#L39)-46 (CS1..CS7 names only); [networklayer/diffserv/DiffservQueue.ned:47](../../../src/inet/networklayer/diffserv/DiffservQueue.ned#L47) and :102 … |
| — | Unrecognized codepoint safety | [RFC 2474 §3](https://www.rfc-editor.org/rfc/rfc2474#section-3) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4Header.cc:50](../../../src/inet/networklayer/ipv4/Ipv4Header.cc#L50)-58; [networklayer/diffserv/BehaviorAggregateClassifier.cc:112](../../../src/inet/networklayer/diffserv/BehaviorAggregateClassifier.cc#L112)-125 and :103-109 |
| <a id="ipv4-158"></a>IPV4-158 | Recommended default codepoint mappings | [RFC 2474 §3](https://www.rfc-editor.org/rfc/rfc2474#section-3) | SHOULD | `Missing` | scope | h | [networklayer/diffserv/BehaviorAggregateClassifier.ned:24](../../../src/inet/networklayer/diffserv/BehaviorAggregateClassifier.ned#L24) (dscps = default("")) … |
| — | DS field rewriting permitted | [RFC 2474 §3](https://www.rfc-editor.org/rfc/rfc2474#section-3) | MAY | `Full` | -- | h | [networklayer/diffserv/DscpMarker.cc:78](../../../src/inet/networklayer/diffserv/DscpMarker.cc#L78)-90; [tests/module/diffserv_dscpmarker_1.test](../../../tests/module/diffserv_dscpmarker_1.test) |
| <a id="ipv4-159"></a>IPV4-159 | Boundary node marking duty | [RFC 2474 §7.1](https://www.rfc-editor.org/rfc/rfc2474#section-7.1) | MUST | `Partial` | scope | m | [networklayer/diffserv/DscpMarker.cc:78](../../../src/inet/networklayer/diffserv/DscpMarker.cc#L78)-90; [networklayer/diffserv/MultiFieldClassifier.cc](../../../src/inet/networklayer/diffserv/MultiFieldClassifier.cc) … |
| <a id="ipv4-160"></a>IPV4-160 | Default single ECT codepoint | [RFC 3168 §5](https://www.rfc-editor.org/rfc/rfc3168#section-5) | SHOULD | `Incorrect` | cosmetic | h | [transportlayer/tcp/TcpConnectionUtil.cc:317](../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L317) (IP_ECN_ECT_1 unconditionally) with the comment at :299-301 "We decided to u … |
| <a id="ipv4-161"></a>IPV4-161 | Logging of discarded datagrams | [RFC 1122 §3.1](https://www.rfc-editor.org/rfc/rfc1122#section-3.1) | SHOULD | `Partial` | cosmetic | m | [networklayer/ipv4/Ipv4.cc:282](../../../src/inet/networklayer/ipv4/Ipv4.cc#L282)-289 (no numDropped increment, reason INCORRECTLY_RECEIVED), :400-406, :946-950 … |
| <a id="ipv4-162"></a>IPV4-162 | Special-case address transmission ban | [RFC 1122 §3.2.1.3](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.1.3) | MUST | `Partial` | cosmetic | m | [networklayer/ipv4/Ipv4.cc:922](../../../src/inet/networklayer/ipv4/Ipv4.cc#L922)-925, :1063-1071 … |
| <a id="ipv4-163"></a>IPV4-163 | Identification reuse on retransmitted copy | [RFC 1122 §3.2.1.5](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.1.5) | MAY | `Non-gap` | -- | h | [networklayer/ipv4/Ipv4.cc:1089](../../../src/inet/networklayer/ipv4/Ipv4.cc#L1089) (fresh identification per datagram from the upper layer) … |
| — | Received TOS passed to transport | [RFC 1122 §3.2.1.6](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.1.6) | SHOULD | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:903](../../../src/inet/networklayer/ipv4/Ipv4.cc#L903)-905 (decapsulate attaches TosInd with the full ToS byte plus DscpInd and EcnInd) … |
| — | RFC 795 link-layer TOS mappings | [RFC 1122 §3.2.1.6](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.1.6) | SHOULD | `Full` | -- | h | [networklayer/ipv4/headers/ip.h:89](../../../src/inet/networklayer/ipv4/headers/ip.h#L89)-92 (IPTOS_LOWDELAY etc. defined but never referenced anywhere in src/inet) … |
| <a id="ipv4-164"></a>IPV4-164 | Timestamp server function | [RFC 1122 §3.2.2.8](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.2.8) | MAY | `Missing` | scope | h | [networklayer/ipv4/Icmp.cc:317](../../../src/inet/networklayer/ipv4/Icmp.cc#L317)-319 |
| <a id="ipv4-165"></a>IPV4-165 | Standard timestamp value rules | [RFC 1122 §3.2.2.8](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.2.8) | MUST | `Non-gap` | -- | h | [networklayer/ipv4/Ipv4.cc:1136](../../../src/inet/networklayer/ipv4/Ipv4.cc#L1136)-1140 … |
| — | Gateway probing restraint | [RFC 1122 §3.3.1.4](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.1.4) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc](../../../src/inet/networklayer/ipv4/Ipv4.cc) (no Echo Request generation and no dead-gateway detection anywhere) … |
| — | Application chooses source address | [RFC 1122 §3.3.4.2](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.4.2) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:1044](../../../src/inet/networklayer/ipv4/Ipv4.cc#L1044), :1063-1071 (explicit source honoured, checked against the interface table unless nonLoca … |
| <a id="ipv4-166"></a>IPV4-166 | Wrong-interface datagram discard | [RFC 1122 §3.3.4.2](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.4.2) | MAY | `Missing` | scope | h | [networklayer/ipv4/Ipv4.cc:385](../../../src/inet/networklayer/ipv4/Ipv4.cc#L385) (rt->isLocalAddress with no InterfaceInd comparison) … |
| <a id="ipv4-167"></a>IPV4-167 | Zero source address acceptance | [RFC 1812 §4.2.2.11](https://www.rfc-editor.org/rfc/rfc1812#section-4.2.2.11) | MUST | `Partial` | scope | h | [networklayer/ipv4/Ipv4.cc:824](../../../src/inet/networklayer/ipv4/Ipv4.cc#L824)-825 (warning only) then :828-852 and :855-891 (unconditional delivery) |

### 3k. Round 2 — fragmentation, reassembly, TTL, ECN

20 requirements. 14 `Full`, 6 `Missing`.

| ID | Feature | Spec clause | Level | Status | Impact | Conf. | Evidence |
|---|---|---|---|---|---|---|---|
| <a id="ipv4-168"></a>IPV4-168 | Whole datagram flushes reassembly | [RFC 791 §3.2](https://www.rfc-editor.org/rfc/rfc791#section-3.2) | SHOULD | `Missing` | scope | h | [networklayer/ipv4/Ipv4.cc:828](../../../src/inet/networklayer/ipv4/Ipv4.cc#L828) (reassembly entered only when fragmentOffset != 0 or MF) … |
| — | Fragment size bounded by MTU | [RFC 791 §3.2](https://www.rfc-editor.org/rfc/rfc791#section-3.2) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:963](../../../src/inet/networklayer/ipv4/Ipv4.cc#L963),966 (mtu test, mtu==0 means infinite), :990 and :1032 (fragment sizing and assertion) … |
| — | Overlapping fragment support | [RFC 6864 §4.2](https://www.rfc-editor.org/rfc/rfc6864#section-4.2) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4FragBuf.cc:60](../../../src/inet/networklayer/ipv4/Ipv4FragBuf.cc#L60); [common/packet/ChunkBuffer.cc:36](../../../src/inet/common/packet/ChunkBuffer.cc#L36)-76 (sliceRegions) and :113-146 (replace) … |
| — | Higher-layer integrity verification | [RFC 6864 §5.2](https://www.rfc-editor.org/rfc/rfc6864#section-5.2) | SHOULD | `Full` | -- | m | [transportlayer/udp/Udp.cc:948](../../../src/inet/transportlayer/udp/Udp.cc#L948),996-1010 … |
| — | Drop on full queue | [RFC 3168 §5](https://www.rfc-editor.org/rfc/rfc3168#section-5) | MUST | `Full` | -- | h | [queueing/filter/RedDropper.cc:76](../../../src/inet/queueing/filter/RedDropper.cc#L76)-80, :115-116, :143-145 |
| — | No marking on instantaneous queue size | [RFC 3168 §5.1](https://www.rfc-editor.org/rfc/rfc3168#section-5.1) | SHOULD | `Full` | -- | h | [queueing/filter/RedDropper.cc:69](../../../src/inet/queueing/filter/RedDropper.cc#L69)-96 (EWMA avg), :122-142 (marking branches), :76 (instantaneous length used only for the … |
| — | ECN codepoint preserved at reassembly | [RFC 3168 §5.3](https://www.rfc-editor.org/rfc/rfc3168#section-5.3) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4FragBuf.cc:63](../../../src/inet/networklayer/ipv4/Ipv4FragBuf.cc#L63)-67 (which fragment's header is retained) and :76,82-85 (reassembled header construct … |
| <a id="ipv4-169"></a>IPV4-169 | Egress drop of suspect CE | [RFC 3168 §9.1.2](https://www.rfc-editor.org/rfc/rfc3168#section-9.1.2) | SHOULD | `Missing` | scope | m | [applications/tunapp/TunnelApp.cc:47](../../../src/inet/applications/tunapp/TunnelApp.cc#L47)-51 (Ipv4Socket bound to Protocol::ipv4), :125-135 (egress, packet->clearTags() then … |
| — | Transport control of TTL | [RFC 1122 §3.2.1.7](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.1.7) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:1051](../../../src/inet/networklayer/ipv4/Ipv4.cc#L1051)-1052 and :1094-1103; [networklayer/ipv4/Ipv4.ned:98](../../../src/inet/networklayer/ipv4/Ipv4.ned#L98)-99 |
| <a id="ipv4-170"></a>IPV4-170 | Source Quench origination | [RFC 1122 §3.2.2.3](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.2.3) | MAY | `Missing` | cosmetic | h | [networklayer/common/IcmpType.msg:19](../../../src/inet/networklayer/common/IcmpType.msg#L19) (ICMP_SOURCEQUENCH = 4 declared) … |
| <a id="ipv4-171"></a>IPV4-171 | MMS_R exposed to transport | [RFC 1122 §3.3.2](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.2) | MUST | `Missing` | cosmetic | m | [networklayer/ipv4/Ipv4.h](../../../src/inet/networklayer/ipv4/Ipv4.h) and Ipv4FragBuf.h (no accessor, no capacity field) … |
| — | EMTU_R configurable | [RFC 1122 §3.3.2](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.2) | SHOULD | `Full` | -- | m | [networklayer/ipv4/Ipv4FragBuf.h:44](../../../src/inet/networklayer/ipv4/Ipv4FragBuf.h#L44)-55 and Ipv4FragBuf.cc:33-96 (unbounded std::map of ReassemblyBuffer, no capacity chec … |
| — | MMS_S exposed and respected | [RFC 1122 §3.3.3](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.3) | MUST | `Full` | -- | m | [transportlayer/sctp/SctpAssociationBase.cc:124](../../../src/inet/transportlayer/sctp/SctpAssociationBase.cc#L124), [transportlayer/quic/Path.cc:59](../../../src/inet/transportlayer/quic/Path.cc#L59), [transportlayer/rtp/RtpProfile.cc:137](../../../src/inet/transportlayer/rtp/RtpProfile.cc#L137) (Ne … |
| — | Interface MTU configurable | [RFC 1122 §3.3.3](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.3) | MUST | `Full` | -- | h | [linklayer/ppp/Ppp.ned:43](../../../src/inet/linklayer/ppp/Ppp.ned#L43), [linklayer/ethernet/basic/EthernetCsmaMac.ned:33](../../../src/inet/linklayer/ethernet/basic/EthernetCsmaMac.ned#L33), [linklayer/ieee80211/mac/Ieee80211Mac.ned:85](../../../src/inet/linklayer/ieee80211/mac/Ieee80211Mac.ned#L85) … |
| <a id="ipv4-172"></a>IPV4-172 | All-Subnets-MTU flag | [RFC 1122 §3.3.3](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.3) | MAY | `Missing` | cosmetic | h | searched src/inet for "AllSubnets" and "all-subnets" - no hits … |
| — | Fragmentation quality | [RFC 1812 §4.2.2.7](https://www.rfc-editor.org/rfc/rfc1812#section-4.2.2.7) | SHOULD | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:990](../../../src/inet/networklayer/ipv4/Ipv4.cc#L990),995 (maximal payload per fragment, minimal count) and :1002-1035 (emission loop) |
| — | Reassembly for local delivery | [RFC 1812 §4.2.2.8](https://www.rfc-editor.org/rfc/rfc1812#section-4.2.2.8) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:385](../../../src/inet/networklayer/ipv4/Ipv4.cc#L385)-386 (local delivery), also :355, :363, :407; :819-852 (reassembleAndDeliver) |
| — | TTL checked only when forwarding | [RFC 1812 §4.2.2.9](https://www.rfc-editor.org/rfc/rfc1812#section-4.2.2.9) | MUST | `Full` | -- | m | [networklayer/ipv4/Ipv4.cc:355](../../../src/inet/networklayer/ipv4/Ipv4.cc#L355),385,407 and :819-891 (local-delivery path, no TTL test) … |
| — | Transport control of router TTL | [RFC 1812 §4.2.2.9](https://www.rfc-editor.org/rfc/rfc1812#section-4.2.2.9) | MUST | `Full` | -- | m | [networklayer/ipv4/Ipv4.cc:1051](../../../src/inet/networklayer/ipv4/Ipv4.cc#L1051)-1052 and :1094-1103; [networklayer/ipv4/Ipv4.ned:98](../../../src/inet/networklayer/ipv4/Ipv4.ned#L98) … |
| <a id="ipv4-173"></a>IPV4-173 | Path MTU Discovery when originating | [RFC 1812 §4.2.3.3](https://www.rfc-editor.org/rfc/rfc1812#section-4.2.3.3) | SHOULD | `Missing` | scope | h | [networklayer/ipv4/Ipv4.cc:1053](../../../src/inet/networklayer/ipv4/Ipv4.cc#L1053)-1055 and :1091 (dontFragment comes only from FragmentationReq, default false) … |

### 3l. Round 2 — IP header options

31 requirements. 8 `Full`, 2 `Incorrect`, 8 `Missing`, 7 `Non-gap`, 6 `Partial`.

| ID | Feature | Spec clause | Level | Status | Impact | Conf. | Evidence |
|---|---|---|---|---|---|---|---|
| <a id="ipv4-174"></a>IPV4-174 | Option type octet subfields | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | MUST | `Partial` | results | h | Ipv4Header.msg:29-32 (masks defined, never referenced); Ipv4HeaderSerializer.cc:77,208 (type octet written/read whole) … |
| <a id="ipv4-175"></a>IPV4-175 | Options appear at most once | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | MUST | `Partial` | cosmetic | m | Ipv4Header.cc:29-38 (addOption appends with no duplicate check); Ipv4.cc:1105-1110,1136-1141 |
| <a id="ipv4-176"></a>IPV4-176 | Route and timestamp area initialization | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | MUST | `Missing` | scope | h | Ipv4.cc:1136-1141; Ipv4OptionRecordRoute constructed only at Ipv4HeaderSerializer.cc:268 … |
| <a id="ipv4-177"></a>IPV4-177 | Header length recomputed in later fragments | [RFC 791 §3.2](https://www.rfc-editor.org/rfc/rfc791#section-3.2) | MUST | `Missing` | results | h | Ipv4.cc:987 (FIXME), 1017 (fraghdr = dupShared of the full header), 988 and 1027 (headerLength reused unchanged) … |
| — | Limited-functionality tunnel option | [RFC 3168 §9.1.1](https://www.rfc-editor.org/rfc/rfc3168#section-9.1.1) | MUST | `Full` | -- | m | TunnelApp.cc:143-152 (clearTags then only L3AddressReq and PacketProtocolTag) … |
| <a id="ipv4-178"></a>IPV4-178 | Full-functionality tunnel option | [RFC 3168 §9.1.1](https://www.rfc-editor.org/rfc/rfc3168#section-9.1.1) | SHOULD | `Missing` | results | m | TunnelApp.cc:124-131,143-152; no EcnReq is built from the inner header and no EcnInd from the outer at either tunnel end |
| — | Transport specification of IP options | [RFC 1122 §3.2.1.8](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.1.8) | MUST | `Full` | -- | h | Ipv4OptionsTag.msg:14-23; Ipv4.cc:1105-1110; users Igmpv2.cc:705,723 and Igmpv3.cc:1471,1490 |
| — | Stream Identifier option ignored | [RFC 1122 — 3.2.1.8b](https://www.rfc-editor.org/rfc/rfc1122) | MUST | `Full` | -- | h | Ipv4HeaderSerializer.cc:218-227 (parsed into Ipv4OptionStreamId); no reader of getStreamId outside the serializer |
| — | Stream Identifier option not sent | [RFC 1122 — 3.2.1.8b](https://www.rfc-editor.org/rfc/rfc1122) | SHOULD | `Full` | -- | h | Ipv4OptionStreamId is constructed only at Ipv4HeaderSerializer.cc:221 … |
| <a id="ipv4-179"></a>IPV4-179 | Record Route option optional | [RFC 1122 — 3.2.1.8d](https://www.rfc-editor.org/rfc/rfc1122) | MAY | `Non-gap` | -- | h | Ipv4Header.msg:100-110 (typed class); Ipv4HeaderSerializer.cc:124-135 and 262-278 (serialize and parse) |
| <a id="ipv4-180"></a>IPV4-180 | Timestamp recorded by originator | [RFC 1122 — 3.2.1.8e](https://www.rfc-editor.org/rfc/rfc1122) | MUST | `Incorrect` | results | h | Ipv4.cc:1114 versus 1136-1141; Ipv4Header.cc:29-33; Ipv4HeaderSerializer.cc:48,111 … |
| <a id="ipv4-181"></a>IPV4-181 | Timestamp Reply source and route | [RFC 1122 §3.2.2.8](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.2.8) | MUST | `Incorrect` | results | m | Icmp.cc:317-318 (ICMP_TIMESTAMP_REQUEST dispatched to processEchoRequest), 335-357 (an IcmpEchoReply is built) … |
| — | Local fragmentation optional | [RFC 1122 §3.3.3](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.3) | MAY | `Full` | -- | m | Ipv4.cc:931-1035 (fragmentAndSend), 976-985 (DF handling), 679,691,928 (locally originated traffic reaches it via fragme … |
| <a id="ipv4-182"></a>IPV4-182 | Host as source route intermediate hop | [RFC 1122 §3.3.5](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.5) | MAY | `Non-gap` | -- | h | Ipv4.cc:409-416 (non-local traffic dropped when forwarding is off); no LSRR/SSRR processing anywhere in Ipv4.cc |
| <a id="ipv4-183"></a>IPV4-183 | Options updated when forwarding | [RFC 1122 §3.3.5](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.5) | MUST | `Non-gap` | -- | h | Ipv4.cc:322-326 (prepareForForwarding changes only the TTL) … |
| <a id="ipv4-184"></a>IPV4-184 | Policy filters for non-local source routing | [RFC 1122 §3.3.5](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.5) | MUST | `Non-gap` | -- | h | Ipv4.ned:92-110 (no source-route policy parameter); no source route processing in Ipv4.cc |
| <a id="ipv4-185"></a>IPV4-185 | Non-standard broadcast forms accepted | [RFC 1122 §3.3.6](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.6) | SHOULD | `Partial` | scope | h | Ipv4RoutingTable.cc:316-334; Ipv4Address.cc:264-268 (makeBroadcastAddress produces the all-ones form only) … |
| <a id="ipv4-186"></a>IPV4-186 | Full IP access for transport | [RFC 1122 §3.4](https://www.rfc-editor.org/rfc/rfc1122#section-3.4) | MUST | `Partial` | scope | h | Ipv4.cc:281-282,895-906,1052,1070-1110 |
| <a id="ipv4-187"></a>IPV4-187 | Local option interpretation | [RFC 1812 §4.2.2.1](https://www.rfc-editor.org/rfc/rfc1812#section-4.2.2.1) | MUST | `Partial` | scope | h | Ipv4.cc:281-282 (parsed header attached to NetworkProtocolInd), 895-897 (front offset saved on decapsulation) … |
| <a id="ipv4-188"></a>IPV4-188 | Source route reversal service | [RFC 1812 §4.2.2.1](https://www.rfc-editor.org/rfc/rfc1812#section-4.2.2.1) | MUST | `Missing` | scope | h | Ipv4OptionRecordRoute is constructed only at Ipv4HeaderSerializer.cc:268 … |
| <a id="ipv4-189"></a>IPV4-189 | Single source route when originating | [RFC 1812 §4.2.2.1](https://www.rfc-editor.org/rfc/rfc1812#section-4.2.2.1) | MUST | `Non-gap` | -- | m | Ipv4.cc:1105-1141 (only upper-layer supplied options plus the Timestamp are added) … |
| <a id="ipv4-190"></a>IPV4-190 | Timestamp overflow counting | [RFC 1812 §4.2.2.1](https://www.rfc-editor.org/rfc/rfc1812#section-4.2.2.1) | MUST | `Missing` | results | h | Ipv4.cc:819-857 and 895-915 (reassembleAndDeliver and decapsulate do no option processing) … |
| — | Unrecognized options ignored locally | [RFC 1812 §4.2.2.6](https://www.rfc-editor.org/rfc/rfc1812#section-4.2.2.6) | MUST | `Full` | -- | h | Ipv4HeaderSerializer.cc:294-311 (unknown types become TlvOptionRaw), 81-91 (raw options re-serialized verbatim), 212 and … |
| <a id="ipv4-191"></a>IPV4-191 | Obsolete broadcast forms | [RFC 1812 §4.2.3.1](https://www.rfc-editor.org/rfc/rfc1812#section-4.2.3.1) | SHOULD | `Partial` | scope | m | Ipv4.cc:388-416 (no 0-form check before the unicast route lookup); Ipv4RoutingTable.cc:316-334; Ipv4Address.h:209 |
| — | Local delivery leaves header intact | [RFC 1812 §5.2.1](https://www.rfc-editor.org/rfc/rfc1812#section-5.2.1) | MUST | `Full` | -- | h | Ipv4.cc:388-397 (local delivery calls reassembleAndDeliver directly, the forwarding copy is a dup), 322-324 (TTL decreme … |
| <a id="ipv4-192"></a>IPV4-192 | Header validation not disableable | [RFC 1812 §5.2.2](https://www.rfc-editor.org/rfc/rfc1812#section-5.2.2) | MUST | `Non-gap` | -- | m | Ipv4.ned:97 (checksumMode is the only validation-related parameter); Ipv4.cc:282 … |
| <a id="ipv4-193"></a>IPV4-193 | Precedence-ordered queue service | [RFC 1812 §5.3.3.1](https://www.rfc-editor.org/rfc/rfc1812#section-5.3.3.1) | SHOULD | `Missing` | scope | m | [networklayer/ipv4/Ipv4.h:37](../../../src/inet/networklayer/ipv4/Ipv4.h#L37) |
| <a id="ipv4-194"></a>IPV4-194 | Lower layer precedence mapping | [RFC 1812 §5.3.3.2](https://www.rfc-editor.org/rfc/rfc1812#section-5.3.3.2) | MUST | `Non-gap` | -- | m | QosClassifier.cc:100-165 (user priority derived from IP protocol and TCP/UDP ports, never from IP precedence) … |
| <a id="ipv4-195"></a>IPV4-195 | Revised security option | [RFC 1812 §5.3.13.2](https://www.rfc-editor.org/rfc/rfc1812#section-5.3.13.2) | SHOULD | `Missing` | scope | h | Ipv4Header.msg:62 (IPOPTION_SECURITY is an enum value with no typed class) … |
| — | Stream Identifier passed through | [RFC 1812 §5.3.13.3](https://www.rfc-editor.org/rfc/rfc1812#section-5.3.13.3) | MUST | `Full` | -- | h | Ipv4HeaderSerializer.cc:101-106 and 218-227 (byte-exact round trip) … |
| <a id="ipv4-196"></a>IPV4-196 | Timestamp ignore option default | [RFC 1812 §5.3.13.6](https://www.rfc-editor.org/rfc/rfc1812#section-5.3.13.6) | MUST | `Missing` | results | h | Ipv4.cc:322-326 and 931-1035 (no timestamp insertion on any forwarding path); Ipv4.ned:92-110 (no such parameter) |

### 3m. Round 2 — ICMP

74 requirements. 26 `Full`, 6 `Incorrect`, 10 `Missing`, 23 `Non-gap`, 9 `Partial`.

| ID | Feature | Spec clause | Level | Status | Impact | Conf. | Evidence |
|---|---|---|---|---|---|---|---|
| — | ICMP mandatory in every IP module | [RFC 792 — Introduction](https://www.rfc-editor.org/rfc/rfc792) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4NetworkLayer.ned:66](../../../src/inet/networklayer/ipv4/Ipv4NetworkLayer.ned#L66)-69 (unconditional `icmp: Icmp` submodule) … |
| — | ICMP error demultiplexing to process | [RFC 792 — Destination Unreachable Message](https://www.rfc-editor.org/rfc/rfc792) | MUST | `Full` | -- | h | [networklayer/ipv4/Icmp.cc:206](../../../src/inet/networklayer/ipv4/Icmp.cc#L206)-210; [networklayer/ipv4/Ipv4.cc:223](../../../src/inet/networklayer/ipv4/Ipv4.cc#L223)-247; [transportlayer/udp/Udp.cc:1155](../../../src/inet/transportlayer/udp/Udp.cc#L1155)-1176,1203-1218 … |
| — | ICMP source address selection | [RFC 792 — Message Formats](https://www.rfc-editor.org/rfc/rfc792) | MUST | `Full` | -- | h | [networklayer/ipv4/Icmp.cc:362](../../../src/inet/networklayer/ipv4/Icmp.cc#L362)-374 (no source set) … |
| — | ICMP message time to live | [RFC 792 — Message Formats](https://www.rfc-editor.org/rfc/rfc792) | SHOULD | `Full` | -- | m | [networklayer/ipv4/Ipv4.cc:1093](../../../src/inet/networklayer/ipv4/Ipv4.cc#L1093)-1102 (ttl = defaultTimeToLive); [networklayer/ipv4/Ipv4.ned](../../../src/inet/networklayer/ipv4/Ipv4.ned) timeToLive = default(32) … |
| <a id="ipv4-197"></a>IPV4-197 | Source Quench message format | [RFC 792 — Source Quench Message](https://www.rfc-editor.org/rfc/rfc792) | MUST | `Non-gap` | -- | h | [networklayer/common/IcmpType.msg:19](../../../src/inet/networklayer/common/IcmpType.msg#L19) (constant only); no Source Quench class in [networklayer/ipv4/IcmpHeader.msg](../../../src/inet/networklayer/ipv4/IcmpHeader.msg) … |
| <a id="ipv4-198"></a>IPV4-198 | Redirect recipient route update | [RFC 792 — Redirect Message](https://www.rfc-editor.org/rfc/rfc792) | MUST | `Missing` | results | h | [networklayer/ipv4/Icmp.cc:261](../../../src/inet/networklayer/ipv4/Icmp.cc#L261)-265 (`ICMP_REDIRECT not implemented yet`, packet deleted) … |
| — | Echo checksum odd length padding | [RFC 792 — Echo or Echo Reply Message](https://www.rfc-editor.org/rfc/rfc792) | MUST | `Full` | -- | h | [common/checksum/Checksum.cc:167](../../../src/inet/common/checksum/Checksum.cc#L167)-185 (trailing odd byte added as addr[0]<<8, i.e. zero-padded) … |
| <a id="ipv4-199"></a>IPV4-199 | Originate Timestamp set by requester | [RFC 792 — Timestamp or Timestamp Reply Message](https://www.rfc-editor.org/rfc/rfc792) | MAY | `Missing` | scope | h | [networklayer/ipv4/IcmpHeader.msg](../../../src/inet/networklayer/ipv4/IcmpHeader.msg) (no timestamp message class) … |
| <a id="ipv4-200"></a>IPV4-200 | Information Request with zero source network | [RFC 792 — Information Request or Information Reply Message](https://www.rfc-editor.org/rfc/rfc792) | MAY | `Non-gap` | -- | h | [networklayer/common/IcmpType.msg:30](../../../src/inet/networklayer/common/IcmpType.msg#L30)-31 (constants only); no type 15 generator in src/ |
| <a id="ipv4-201"></a>IPV4-201 | Address Mask Request at boot | [RFC 950 — Section 2.3](https://www.rfc-editor.org/rfc/rfc950) | MAY | `Non-gap` | -- | h | no ICMP_MASK_REQUEST sender anywhere in src/ (searched networklayer/ipv4/, networklayer/common/, applications/) |
| <a id="ipv4-202"></a>IPV4-202 | Unsolicited Address Mask Reply at startup | [RFC 950 — Section 2.3](https://www.rfc-editor.org/rfc/rfc950) | SHOULD | `Non-gap` | -- | h | no ICMP_MASK_REPLY sender in [networklayer/ipv4/Icmp.cc](../../../src/inet/networklayer/ipv4/Icmp.cc) or elsewhere in src/ |
| <a id="ipv4-203"></a>IPV4-203 | Mask correction on received reply | [RFC 950 — Section 2.3](https://www.rfc-editor.org/rfc/rfc950) | SHOULD | `Non-gap` | -- | h | [networklayer/ipv4/Icmp.cc:260](../../../src/inet/networklayer/ipv4/Icmp.cc#L260)-327 has no ICMP_MASK_REPLY case (falls into `default: throw` at :325-326) |
| — | No guessed Address Mask Reply | [RFC 950 — Section 2.3](https://www.rfc-editor.org/rfc/rfc950) | SHOULD | `Full` | -- | h | no code path emits type 18; ICMP_MASK_REPLY occurs only at [networklayer/common/IcmpType.msg:33](../../../src/inet/networklayer/common/IcmpType.msg#L33) and in generated _m files |
| <a id="ipv4-204"></a>IPV4-204 | Fallback mask when discovery fails | [RFC 950 — Section 2.3](https://www.rfc-editor.org/rfc/rfc950) | SHOULD | `Non-gap` | -- | m | no mask-discovery code (no type 17/18 handling) … |
| <a id="ipv4-205"></a>IPV4-205 | Address Mask reply matching not required | [RFC 950 — Appendix I](https://www.rfc-editor.org/rfc/rfc950) | MAY | `Non-gap` | -- | h | no Address Mask request/reply code in [networklayer/ipv4/Icmp.cc](../../../src/inet/networklayer/ipv4/Icmp.cc) |
| <a id="ipv4-206"></a>IPV4-206 | Avoid zero source in mask request | [RFC 950 — Section 2.3](https://www.rfc-editor.org/rfc/rfc950) | SHOULD | `Non-gap` | -- | h | no ICMP_MASK_REQUEST sender in src/ |
| <a id="ipv4-207"></a>IPV4-207 | Router ignores Source Quench | [RFC 6633 — Section 4](https://www.rfc-editor.org/rfc/rfc6633) | MUST | `Incorrect` | results | h | [networklayer/ipv4/Icmp.cc:260](../../../src/inet/networklayer/ipv4/Icmp.cc#L260)-326 — ICMP_SOURCEQUENCH (IcmpType.msg:19) has no case, so it reaches `default: throw cRunt … |
| — | RFC 1016 handling not implemented | [RFC 6633 — Section 7](https://www.rfc-editor.org/rfc/rfc6633) | MUST | `Full` | -- | h | no RFC 1016 style source-quench reaction anywhere; [networklayer/ipv4/Icmp.cc](../../../src/inet/networklayer/ipv4/Icmp.cc) has no Source Quench handling at all |
| — | Remaining deprecated ICMPv4 types | [RFC 6918 — Section 3](https://www.rfc-editor.org/rfc/rfc6918) | MUST | `Full` | -- | h | [networklayer/common/IcmpType.msg:15](../../../src/inet/networklayer/common/IcmpType.msg#L15)-34 defines no constants for types 6, 30-39; no generator for any of them in src/ |
| — | Subnet extension support | [RFC 1122 §3.2.1.3](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.1.3) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4InterfaceData.h:127](../../../src/inet/networklayer/ipv4/Ipv4InterfaceData.h#L127) (netmask member), :157 getNetmask, :175 setNetmask, held per NetworkInterface … |
| — | ICMP error demultiplexing | [RFC 1122 §3.2.2](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.2) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:227](../../../src/inet/networklayer/ipv4/Ipv4.cc#L227)-247 — the quoted Ipv4Header is popped and getProtocolId() selects the DispatchProtocolReq … |
| — | ICMP error TOS zero | [RFC 1122 §3.2.2](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.2) | SHOULD | `Full` | -- | m | [networklayer/ipv4/Icmp.cc:362](../../../src/inet/networklayer/ipv4/Icmp.cc#L362)-374 attaches no TosReq/DscpReq … |
| — | Protocol and Port Unreachable generation | [RFC 1122 §3.2.2.1](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.2.1) | SHOULD | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:887](../../../src/inet/networklayer/ipv4/Ipv4.cc#L887)-891 (code 2); [transportlayer/udp/Udp.cc:1113](../../../src/inet/transportlayer/udp/Udp.cc#L1113)-1118 (code 3) … |
| — | Destination Unreachable reported upward | [RFC 1122 §3.2.2.1](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.2.1) | MUST | `Full` | -- | h | [networklayer/ipv4/Icmp.cc:267](../../../src/inet/networklayer/ipv4/Icmp.cc#L267)-305 -> [networklayer/ipv4/Ipv4.cc:223](../../../src/inet/networklayer/ipv4/Ipv4.cc#L223)-247 -> [transportlayer/udp/Udp.cc:1155](../../../src/inet/transportlayer/udp/Udp.cc#L1155)-1176,1203-1231 … |
| <a id="ipv4-208"></a>IPV4-208 | Illegal Redirect discarded | [RFC 1122 §3.2.2.2](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.2.2) | SHOULD | `Non-gap` | -- | h | [networklayer/ipv4/Icmp.cc:261](../../../src/inet/networklayer/ipv4/Icmp.cc#L261)-265 discards every Redirect; nothing in src/ writes the routing table from an ICMP message |
| <a id="ipv4-209"></a>IPV4-209 | Source Quench reported upward | [RFC 1122 §3.2.2.3](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.2.3) | MUST | `Incorrect` | results | m | [networklayer/ipv4/Icmp.cc:267](../../../src/inet/networklayer/ipv4/Icmp.cc#L267)-269 lists only DU/TE/PP … |
| — | Time Exceeded reported upward | [RFC 1122 §3.2.2.4](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.2.4) | MUST | `Full` | -- | h | [networklayer/ipv4/Icmp.cc:268](../../../src/inet/networklayer/ipv4/Icmp.cc#L268) (ICMP_TIME_EXCEEDED shares the DU/PP branch, :267-305); [networklayer/ipv4/Ipv4.cc:223](../../../src/inet/networklayer/ipv4/Ipv4.cc#L223)-247 … |
| <a id="ipv4-210"></a>IPV4-210 | Parameter Problem generation | [RFC 1122 §3.2.2.5](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.2.5) | SHOULD | `Partial` | results | h | generated at [networklayer/ipv4/Ipv4.cc:291](../../../src/inet/networklayer/ipv4/Ipv4.cc#L291)-294 and :303-311 … |
| — | Parameter Problem reported upward | [RFC 1122 §3.2.2.5](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.2.5) | MUST | `Full` | -- | h | [networklayer/ipv4/Icmp.cc:269](../../../src/inet/networklayer/ipv4/Icmp.cc#L269) (ICMP_PARAMETER_PROBLEM in the DU/TE/PP branch, :267-305) … |
| — | Echo Request data echoed | [RFC 1122 §3.2.2.6](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.2.6) | MUST | `Full` | -- | h | [networklayer/ipv4/Icmp.cc:338](../../../src/inet/networklayer/ipv4/Icmp.cc#L338)-348 — the request header is popped and `reply->insertAtBack(request->peekData())` copies a … |
| — | Echo Reply delivered to user | [RFC 1122 §3.2.2.6](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.2.6) | MUST | `Full` | -- | m | [networklayer/ipv4/Ipv4.cc:864](../../../src/inet/networklayer/ipv4/Ipv4.cc#L864)-877 dups the packet to every matching socket … |
| <a id="ipv4-211"></a>IPV4-211 | Echo Reply option reflection | [RFC 1122 §3.2.2.6](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.2.6) | SHOULD | `Missing` | scope | h | [networklayer/ipv4/Icmp.cc:335](../../../src/inet/networklayer/ipv4/Icmp.cc#L335)-360 never inspects the request's Ipv4Header options … |
| — | Echo client interface | [RFC 1122 §3.2.2.6](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.2.6) | SHOULD | `Full` | -- | h | [applications/pingapp/PingApp.cc:364](../../../src/inet/applications/pingapp/PingApp.cc#L364)-400 (sends IcmpEchoRequest), :233-240 and :440-470 (receives and reports replies) … |
| <a id="ipv4-212"></a>IPV4-212 | Broadcast Echo Request discard | [RFC 1122 §3.2.2.6](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.2.6) | MAY | `Non-gap` | -- | m | [networklayer/ipv4/Icmp.cc:309](../../../src/inet/networklayer/ipv4/Icmp.cc#L309)-311,335-360 replies unconditionally; no broadcast/multicast guard in processEchoRequest |
| — | Information Request obsolete | [RFC 1122 §3.2.2.7](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.2.7) | SHOULD | `Full` | -- | h | [networklayer/common/IcmpType.msg:30](../../../src/inet/networklayer/common/IcmpType.msg#L30)-31 are constants only; no type 15 or 16 generator or handler in src/ |
| <a id="ipv4-213"></a>IPV4-213 | Address mask method configuration | [RFC 1122 §3.2.2.9](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.2.9) | MUST | `Non-gap` | -- | m | static config works: [networklayer/ipv4/Ipv4InterfaceData.h:175](../../../src/inet/networklayer/ipv4/Ipv4InterfaceData.h#L175) setNetmask, [networklayer/ipv4/RoutingTableParser.cc:200](../../../src/inet/networklayer/ipv4/RoutingTableParser.cc#L200)-2 … |
| <a id="ipv4-214"></a>IPV4-214 | Address Mask Request at initialization | [RFC 1122 §3.2.2.9](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.2.9) | MUST | `Non-gap` | -- | h | no ICMP type 17 sender anywhere in src/ |
| <a id="ipv4-215"></a>IPV4-215 | First Address Mask Reply wins | [RFC 1122 §3.2.2.9](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.2.9) | MUST | `Non-gap` | -- | h | [networklayer/ipv4/Icmp.cc:260](../../../src/inet/networklayer/ipv4/Icmp.cc#L260)-327 has no ICMP_MASK_REPLY case |
| <a id="ipv4-216"></a>IPV4-216 | Address Mask Replies ignored when disabled | [RFC 1122 §3.2.2.9](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.2.9) | MUST | `Incorrect` | results | m | [networklayer/ipv4/Icmp.cc:260](../../../src/inet/networklayer/ipv4/Icmp.cc#L260)-326 — type 18 has no case and reaches `default: throw cRuntimeError("Unknown ICMP type %d" … |
| <a id="ipv4-217"></a>IPV4-217 | Address mask reasonableness check | [RFC 1122 §3.2.2.9](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.2.9) | SHOULD | `Non-gap` | -- | m | no path installs a dynamically learned mask … |
| <a id="ipv4-218"></a>IPV4-218 | Agent broadcasts mask at initialization | [RFC 1122 §3.2.2.9](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.2.9) | MUST | `Non-gap` | -- | h | no address-mask-agent parameter in [networklayer/ipv4/Icmp.ned:18](../../../src/inet/networklayer/ipv4/Icmp.ned#L18)-24 or Ipv4RoutingTable.ned; no type 18 sender in src/ |
| — | Local versus remote decision | [RFC 1122 §3.3.1.1](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.1.1) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4RoutingTable.cc:687](../../../src/inet/networklayer/ipv4/Ipv4RoutingTable.cc#L687)-707 adds an IFACENETMASK route per interface from getIPAddress().doAnd(getNetm … |
| <a id="ipv4-219"></a>IPV4-219 | Route cache maintained | [RFC 1122 §3.3.1.2](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.1.2) | MUST | `Partial` | results | h | [networklayer/ipv4/Ipv4RoutingTable.h:76](../../../src/inet/networklayer/ipv4/Ipv4RoutingTable.h#L76)-78 (routingCache: dest -> route), populated at Ipv4RoutingTable.cc:365-383 … |
| <a id="ipv4-220"></a>IPV4-220 | Network Redirect treated as host Redirect | [RFC 1122 §3.3.1.2](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.1.2) | SHOULD | `Non-gap` | -- | h | [networklayer/ipv4/Icmp.cc:261](../../../src/inet/networklayer/ipv4/Icmp.cc#L261)-265 — neither Network nor Host Redirect is processed |
| <a id="ipv4-221"></a>IPV4-221 | Static route table | [RFC 1122 §3.3.1.2](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.1.2) | MAY | `Partial` | scope | m | static routes exist: [networklayer/ipv4/Ipv4RoutingTable.cc:498](../../../src/inet/networklayer/ipv4/Ipv4RoutingTable.cc#L498) addRoute, RoutingTableParser.cc route section, IRoute::MA … |
| — | Configurable initialization data | [RFC 1122 §3.3.1.6](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.1.6) | MUST | `Full` | -- | m | [networklayer/ipv4/Ipv4InterfaceData.h:157](../../../src/inet/networklayer/ipv4/Ipv4InterfaceData.h#L157)-175 (address, netmask) … |
| <a id="ipv4-222"></a>IPV4-222 | Source route forwarding error codes | [RFC 1122 §3.3.5](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.5) | MUST | `Non-gap` | -- | m | no LSRR/SSRR processing anywhere: [networklayer/ipv4/Ipv4.cc](../../../src/inet/networklayer/ipv4/Ipv4.cc) reads options only at :332-337 … |
| <a id="ipv4-223"></a>IPV4-223 | Source Route Failed on refusal to forward | [RFC 1122 §3.3.5](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.5) | SHOULD | `Non-gap` | -- | m | same absence as RD047; ICMP_DU_SOURCE_ROUTE_FAILED occurs only at [networklayer/common/IcmpType.msg:67](../../../src/inet/networklayer/common/IcmpType.msg#L67) |
| <a id="ipv4-224"></a>IPV4-224 | Duty to report errors | [RFC 1122 §3.3.8](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.8) | MUST | `Partial` | results | m | generated: [networklayer/ipv4/Ipv4.cc:293](../../../src/inet/networklayer/ipv4/Ipv4.cc#L293) and :309 (type 12), :663 (type 3 code 0), :891 (3/2), :949 (type 11), :981 PTB … |
| <a id="ipv4-225"></a>IPV4-225 | Unknown ICMP type handling | [RFC 1812 §4.3.2.1](https://www.rfc-editor.org/rfc/rfc1812#section-4.3.2.1) | MUST | `Incorrect` | results | h | [networklayer/ipv4/Icmp.cc:325](../../../src/inet/networklayer/ipv4/Icmp.cc#L325)-326 — `default: throw cRuntimeError("Unknown ICMP type %d", icmpmsg->getType())` … |
| — | ICMP TTL initialization | [RFC 1812 §4.3.2.2](https://www.rfc-editor.org/rfc/rfc1812#section-4.3.2.2) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:1093](../../../src/inet/networklayer/ipv4/Ipv4.cc#L1093)-1102 sets TTL from defaultTimeToLive because [networklayer/ipv4/Icmp.cc:362](../../../src/inet/networklayer/ipv4/Icmp.cc#L362)-374 attaches n … |
| <a id="ipv4-226"></a>IPV4-226 | ICMP error payload length | [RFC 1812 §4.3.2.3](https://www.rfc-editor.org/rfc/rfc1812#section-4.3.2.3) | SHOULD | `Partial` | results | h | [networklayer/ipv4/Icmp.cc:206](../../../src/inet/networklayer/ipv4/Icmp.cc#L206)-210 quotes headerLength + quoteLength … |
| — | ICMP source address selection | [RFC 1812 §4.3.2.4](https://www.rfc-editor.org/rfc/rfc1812#section-4.3.2.4) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:918](../../../src/inet/networklayer/ipv4/Ipv4.cc#L918)-926 — fragmentPostRouting sets an unspecified source address to the outgoing interface's a … |
| <a id="ipv4-227"></a>IPV4-227 | ICMP TOS and precedence | [RFC 1812 §4.3.2.5](https://www.rfc-editor.org/rfc/rfc1812#section-4.3.2.5) | MUST | `Missing` | cosmetic | m | [networklayer/ipv4/Icmp.cc:362](../../../src/inet/networklayer/ipv4/Icmp.cc#L362)-374 attaches no TosReq; [networklayer/ipv4/Ipv4Header.msg:176](../../../src/inet/networklayer/ipv4/Ipv4Header.msg#L176) leaves typeOfService at 0 … |
| <a id="ipv4-228"></a>IPV4-228 | Source route in ICMP errors | [RFC 1812 §4.3.2.6](https://www.rfc-editor.org/rfc/rfc1812#section-4.3.2.6) | SHOULD | `Missing` | scope | m | [networklayer/ipv4/Icmp.cc:189](../../../src/inet/networklayer/ipv4/Icmp.cc#L189)-217 builds the error packet with no Ipv4OptionsReq … |
| <a id="ipv4-229"></a>IPV4-229 | No ICMP error after silent discard | [RFC 1812 §4.3.2.7](https://www.rfc-editor.org/rfc/rfc1812#section-4.3.2.7) | MUST | `Incorrect` | results | m | [networklayer/ipv4/Ipv4.cc:303](../../../src/inet/networklayer/ipv4/Ipv4.cc#L303)-311 — on a header bit error the packet is answered with ICMP_PARAMETER_PROBLEM, whereas RF … |
| <a id="ipv4-230"></a>IPV4-230 | TOS unreachable codes | [RFC 1812 §4.3.3.1](https://www.rfc-editor.org/rfc/rfc1812#section-4.3.3.1) | MUST | `Non-gap` | -- | m | [networklayer/ipv4/Ipv4Route.h](../../../src/inet/networklayer/ipv4/Ipv4Route.h) carries no TOS field and [networklayer/ipv4/Ipv4RoutingTable.cc:365](../../../src/inet/networklayer/ipv4/Ipv4RoutingTable.cc#L365)-383 matches on destinat … |
| — | Redirects ignored for originated traffic | [RFC 1812 §4.3.3.2](https://www.rfc-editor.org/rfc/rfc1812#section-4.3.3.2) | MAY | `Full` | -- | h | [networklayer/ipv4/Icmp.cc:261](../../../src/inet/networklayer/ipv4/Icmp.cc#L261)-265 discards every Redirect, so no route ever changes because of one |
| <a id="ipv4-231"></a>IPV4-231 | Received Source Quench ignored | [RFC 1812 §4.3.3.3](https://www.rfc-editor.org/rfc/rfc1812#section-4.3.3.3) | MAY | `Incorrect` | results | h | [networklayer/ipv4/Icmp.cc:325](../../../src/inet/networklayer/ipv4/Icmp.cc#L325)-326 — ICMP_SOURCEQUENCH (type 4) has no case and reaches `default: throw cRuntimeError` |
| <a id="ipv4-232"></a>IPV4-232 | Parameter Problem for other errors | [RFC 1812 §4.3.3.5](https://www.rfc-editor.org/rfc/rfc1812#section-4.3.3.5) | MUST | `Partial` | results | h | generated at [networklayer/ipv4/Ipv4.cc:291](../../../src/inet/networklayer/ipv4/Ipv4.cc#L291)-294 and :303-311 only … |
| — | Echo Request reassembly size | [RFC 1812 §4.3.3.6](https://www.rfc-editor.org/rfc/rfc1812#section-4.3.3.6) | MUST | `Full` | -- | m | [networklayer/ipv4/Ipv4FragBuf.cc](../../../src/inet/networklayer/ipv4/Ipv4FragBuf.cc) imposes no reassembly size cap; [networklayer/ipv4/Ipv4.cc:819](../../../src/inet/networklayer/ipv4/Ipv4.cc#L819)-853 reassembleAndDeliver … |
| <a id="ipv4-233"></a>IPV4-233 | Echo suppression option | [RFC 1812 §4.3.3.6](https://www.rfc-editor.org/rfc/rfc1812#section-4.3.3.6) | SHOULD | `Partial` | scope | h | [networklayer/ipv4/Icmp.ned:18](../../../src/inet/networklayer/ipv4/Icmp.ned#L18)-24 exposes no echo-suppression parameter; [networklayer/ipv4/Icmp.cc:309](../../../src/inet/networklayer/ipv4/Icmp.cc#L309)-311 always replies |
| <a id="ipv4-234"></a>IPV4-234 | Address Mask server duties | [RFC 1812 §4.3.3.9](https://www.rfc-editor.org/rfc/rfc1812#section-4.3.3.9) | MUST | `Non-gap` | -- | h | no ICMP_MASK_REPLY generator in src/ … |
| — | Address Mask Reply not trusted | [RFC 1812 §4.3.3.9](https://www.rfc-editor.org/rfc/rfc1812#section-4.3.3.9) | MUST | `Full` | -- | h | nothing in src/ reads an Address Mask Reply … |
| <a id="ipv4-235"></a>IPV4-235 | Router Discovery support | [RFC 1812 §4.3.3.10](https://www.rfc-editor.org/rfc/rfc1812#section-4.3.3.10) | MUST | `Missing` | scope | h | [networklayer/common/IcmpType.msg:22](../../../src/inet/networklayer/common/IcmpType.msg#L22)-23 defines ICMP_ROUTER_ADVERTISEMENT/SOLICITATION as constants only … |
| <a id="ipv4-236"></a>IPV4-236 | Parameter Problem for bad length fields | [RFC 1812 §5.2.2](https://www.rfc-editor.org/rfc/rfc1812#section-5.2.2) | MAY | `Partial` | results | h | Ipv4.cc:291; Ipv4HeaderSerializer.cc:178-186; rfc1812.txt 5.2.2 |
| <a id="ipv4-237"></a>IPV4-237 | Transit strict source route check | [RFC 1812 §5.2.2](https://www.rfc-editor.org/rfc/rfc1812#section-5.2.2) | SHOULD | `Missing` | results | h | [networklayer/ipv4/Ipv4.cc](../../../src/inet/networklayer/ipv4/Ipv4.cc) has no source-route inspection on the transit path (preroutingFinish :344 -> routeUnicastPacke … |
| <a id="ipv4-238"></a>IPV4-238 | Unreachable code selection | [RFC 1812 §5.2.7.1](https://www.rfc-editor.org/rfc/rfc1812#section-5.2.7.1) | SHOULD | `Partial` | results | h | codes actually generated: 0 at [networklayer/ipv4/Ipv4.cc:663](../../../src/inet/networklayer/ipv4/Ipv4.cc#L663), 2 at :891, 4 via [networklayer/ipv4/Icmp.cc:161](../../../src/inet/networklayer/ipv4/Icmp.cc#L161)-186 (IcmpHe … |
| <a id="ipv4-239"></a>IPV4-239 | Host Unreachable preferred over Network | [RFC 1812 §5.2.7.1](https://www.rfc-editor.org/rfc/rfc1812#section-5.2.7.1) | MUST | `Missing` | results | h | [networklayer/ipv4/Ipv4.cc:657](../../../src/inet/networklayer/ipv4/Ipv4.cc#L657)-663 sends code 0 for every unroutable destination … |
| <a id="ipv4-240"></a>IPV4-240 | Administratively prohibited code | [RFC 1812 §5.2.7.1](https://www.rfc-editor.org/rfc/rfc1812#section-5.2.7.1) | SHOULD | `Missing` | scope | m | ICMP_DU_COMMUNICATION_PROHIBITED appears only at [networklayer/common/IcmpType.msg:75](../../../src/inet/networklayer/common/IcmpType.msg#L75) … |
| <a id="ipv4-241"></a>IPV4-241 | TOS Redirect capability | [RFC 1812 §5.2.7.2](https://www.rfc-editor.org/rfc/rfc1812#section-5.2.7.2) | MUST | `Missing` | scope | h | no Redirect generator: ICMP_REDIRECT occurs only at [networklayer/common/IcmpType.msg:20](../../../src/inet/networklayer/common/IcmpType.msg#L20), [networklayer/contract/IRoute.h](../../../src/inet/networklayer/contract/IRoute.h): … |
| <a id="ipv4-242"></a>IPV4-242 | Redirect source address | [RFC 1812 §5.2.7.2](https://www.rfc-editor.org/rfc/rfc1812#section-5.2.7.2) | MUST | `Non-gap` | -- | h | no Redirect sender exists in [networklayer/ipv4/Icmp.cc](../../../src/inet/networklayer/ipv4/Icmp.cc) or Ipv4.cc |
| <a id="ipv4-243"></a>IPV4-243 | Time Exceeded suppression option | [RFC 1812 §5.2.7.3](https://www.rfc-editor.org/rfc/rfc1812#section-5.2.7.3) | MAY | `Non-gap` | -- | m | [networklayer/ipv4/Ipv4.cc:942](../../../src/inet/networklayer/ipv4/Ipv4.cc#L942)-951 always sends ICMP_TIME_EXCEEDED on TTL expiry … |
| <a id="ipv4-244"></a>IPV4-244 | Precedence cutoff exemptions | [RFC 1812 §5.3.3.3](https://www.rfc-editor.org/rfc/rfc1812#section-5.3.3.3) | MUST | `Non-gap` | -- | m | no precedence handling on the IPv4 path: a search for "precedence" across networklayer/ finds only the code names in Icm … |

### 3n. Round 2 — ARP and address conflict detection

15 requirements. 6 `Full`, 9 `Missing`.

| ID | Feature | Spec clause | Level | Status | Impact | Conf. | Evidence |
|---|---|---|---|---|---|---|---|
| — | Request header field population | [RFC 826 — Packet Generation](https://www.rfc-editor.org/rfc/rfc826) | MUST | `Full` | -- | h | [networklayer/arp/ipv4/ArpPacketSerializer.cc:38](../../../src/inet/networklayer/arp/ipv4/ArpPacketSerializer.cc#L38)-41; Arp.cc:154-160; ArpPacket.msg:36-45 |
| <a id="ipv4-245"></a>IPV4-245 | Optional address length consistency check | [RFC 826 — Packet Reception](https://www.rfc-editor.org/rfc/rfc826) | MAY | `Missing` | cosmetic | h | ArpPacketSerializer.cc:56-62 (file is 67 lines) |
| — | No periodic broadcast of mappings | [RFC 826 — Why is it done this way??](https://www.rfc-editor.org/rfc/rfc826) | SHOULD | `Full` | -- | h | [networklayer/arp/ipv4/Arp.cc:85](../../../src/inet/networklayer/arp/ipv4/Arp.cc#L85)-94,123-141,179-202,390-423 |
| — | Probe target hardware address zeroed | [RFC 5227 §2.1.1](https://www.rfc-editor.org/rfc/rfc5227#section-2.1.1) | SHOULD | `Full` | -- | h | [networklayer/arp/ipv4/Arp.cc:487](../../../src/inet/networklayer/arp/ipv4/Arp.cc#L487)-516 (esp. 501); Arp.cc:221-226,265-271 |
| <a id="ipv4-246"></a>IPV4-246 | ACD timing constant values | [RFC 5227 §1.1](https://www.rfc-editor.org/rfc/rfc5227#section-1.1) | MUST | `Missing` | scope | h | Arp.h:57-67; Arp.ned:42-45 |
| <a id="ipv4-247"></a>IPV4-247 | Notify configuring agent of conflict | [RFC 5227 §2.1.1](https://www.rfc-editor.org/rfc/rfc5227#section-2.1.1) | SHOULD | `Missing` | scope | h | [networklayer/arp/ipv4/Arp.cc:228](../../../src/inet/networklayer/arp/ipv4/Arp.cc#L228)-366 (no conflict detection on reception) … |
| <a id="ipv4-248"></a>IPV4-248 | Rate limit counts later conflicts | [RFC 5227 §2.1.1](https://www.rfc-editor.org/rfc/rfc5227#section-2.1.1) | MUST | `Missing` | scope | h | Arp.h:62-65 (counters); Arp.h is 123 lines |
| <a id="ipv4-249"></a>IPV4-249 | Address usable after first announcement | [RFC 5227 §2.3](https://www.rfc-editor.org/rfc/rfc5227#section-2.3) | MAY | `Missing` | scope | h | [networklayer/arp/ipv4/Arp.cc:441](../../../src/inet/networklayer/arp/ipv4/Arp.cc#L441)-483 (sendArpGratuitous, zero callers repo-wide); no announcement sequencing anywhere |
| <a id="ipv4-250"></a>IPV4-250 | Immediate cease on first conflict | [RFC 5227 §2.4](https://www.rfc-editor.org/rfc/rfc5227#section-2.4) | MAY | `Missing` | scope | h | [networklayer/arp/ipv4/Arp.cc:228](../../../src/inet/networklayer/arp/ipv4/Arp.cc#L228)-366; DhcpClient.cc:313-320 |
| <a id="ipv4-251"></a>IPV4-251 | Indefinite defense of fixed address | [RFC 5227 §2.4](https://www.rfc-editor.org/rfc/rfc5227#section-2.4) | MAY | `Missing` | scope | h | Arp.ned:42-45 (file is 64 lines) |
| <a id="ipv4-252"></a>IPV4-252 | Log and notify administrator on conflict | [RFC 5227 §2.4](https://www.rfc-editor.org/rfc/rfc5227#section-2.4) | SHOULD | `Missing` | scope | h | [networklayer/arp/ipv4/Arp.cc:221](../../../src/inet/networklayer/arp/ipv4/Arp.cc#L221)-226 (only unconditional EV_DETAIL dump), 228-366 |
| <a id="ipv4-253"></a>IPV4-253 | Reset connections before abandoning address | [RFC 5227 §2.4](https://www.rfc-editor.org/rfc/rfc5227#section-2.4) | SHOULD | `Missing` | scope | h | Arp.h:86-117 (file is 123 lines) |
| — | Broadcast ARP replies not for general use | [RFC 5227 §2.6](https://www.rfc-editor.org/rfc/rfc5227#section-2.6) | SHOULD | `Full` | -- | h | [networklayer/arp/ipv4/Arp.cc:321](../../../src/inet/networklayer/arp/ipv4/Arp.cc#L321)-330 (reply construction), 340-341 (send) |
| — | Accept ARP replies sent by broadcast | [RFC 5227 §1.2.1](https://www.rfc-editor.org/rfc/rfc5227#section-1.2.1) | MUST | `Full` | -- | h | [networklayer/arp/ipv4/Arp.cc:265](../../../src/inet/networklayer/arp/ipv4/Arp.cc#L265)-281 (ar$spa merge), 285-304, 307 (opcode examined only afterwards), 346-348 |
| — | ARP packet queue | [RFC 1122 §2.3.2.2](https://www.rfc-editor.org/rfc/rfc1122#section-2.3.2.2) | SHOULD | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:1146](../../../src/inet/networklayer/ipv4/Ipv4.cc#L1146)-1164 (queue on unresolved), 1167-1186 (flush on resolution), 1188-1204 (drop on failure) … |

### 3o. Round 2 — forwarding and routing

29 requirements. 12 `Full`, 7 `Missing`, 2 `Non-gap`, 8 `Partial`.

| ID | Feature | Spec clause | Level | Status | Impact | Conf. | Evidence |
|---|---|---|---|---|---|---|---|
| <a id="ipv4-254"></a>IPV4-254 | Two independent forwarding classes | [RFC 2474 §4.2.2.2](https://www.rfc-editor.org/rfc/rfc2474#section-4.2.2.2) | MUST | `Partial` | scope | h | [networklayer/diffserv/Dscp.msg:40](../../../src/inet/networklayer/diffserv/Dscp.msg#L40)-46 (CS0-CS7 defined); [networklayer/diffserv/BehaviorAggregateClassifier.cc:110](../../../src/inet/networklayer/diffserv/BehaviorAggregateClassifier.cc#L110)-120 … |
| <a id="ipv4-255"></a>IPV4-255 | Preference for routing traffic classes | [RFC 2474 §4.2.2.2](https://www.rfc-editor.org/rfc/rfc2474#section-4.2.2.2) | MUST | `Partial` | scope | h | [linklayer/ethernet/basic/EthernetInterface.ned:50](../../../src/inet/linklayer/ethernet/basic/EthernetInterface.ned#L50) (default EthernetQueue = DropTailQueue) … |
| <a id="ipv4-256"></a>IPV4-256 | Class selector relative ordering | [RFC 2474 §4.2.2.2](https://www.rfc-editor.org/rfc/rfc2474#section-4.2.2.2) | SHOULD | `Partial` | scope | m | [linklayer/ethernet/basic/EthernetInterface.ned:50](../../../src/inet/linklayer/ethernet/basic/EthernetInterface.ned#L50); [queueing/filter/RedDropper.cc:105](../../../src/inet/queueing/filter/RedDropper.cc#L105)-120 (no DSCP input) … |
| — | ECT codepoints equivalent at routers | [RFC 3168 §5](https://www.rfc-editor.org/rfc/rfc3168#section-5) | MUST | `Full` | -- | h | [queueing/filter/RedDropper.cc:111](../../../src/inet/queueing/filter/RedDropper.cc#L111) and :128-141; [networklayer/common/EcnTag.msg:12](../../../src/inet/networklayer/common/EcnTag.msg#L12)-17; [networklayer/ipv4/Ipv4.cc:1085](../../../src/inet/networklayer/ipv4/Ipv4.cc#L1085) |
| — | Embedded gateway configuration switch | [RFC 1122 §3.1](https://www.rfc-editor.org/rfc/rfc1122#section-3.1) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4RoutingTable.ned:48](../../../src/inet/networklayer/ipv4/Ipv4RoutingTable.ned#L48); [node/base/NetworkLayerNodeBase.ned:26](../../../src/inet/node/base/NetworkLayerNodeBase.ned#L26)-28; [node/inet/StandardHost.ned:25](../../../src/inet/node/inet/StandardHost.ned#L25) … |
| — | Operation without any gateway | [RFC 1122 §3.3.1.1](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.1.1) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4RoutingTable.cc:53](../../../src/inet/networklayer/ipv4/Ipv4RoutingTable.cc#L53)-104 (initialize), :669-706 (updateNetmaskRoutes), :690 … |
| <a id="ipv4-257"></a>IPV4-257 | Multiple default gateways | [RFC 1122 §3.3.1.2](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.1.2) | MUST | `Partial` | results | h | [networklayer/ipv4/Ipv4RoutingTable.cc:476](../../../src/inet/networklayer/ipv4/Ipv4RoutingTable.cc#L476)-480 (old default route deleted when a new one is added) … |
| <a id="ipv4-258"></a>IPV4-258 | TOS in route cache | [RFC 1122 §3.3.1.3](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.1.3) | SHOULD | `Missing` | scope | h | [networklayer/ipv4/Ipv4RoutingTable.h:77](../../../src/inet/networklayer/ipv4/Ipv4RoutingTable.h#L77)-78 (cache keyed by Ipv4Address only) … |
| <a id="ipv4-259"></a>IPV4-259 | Dead gateway detection | [RFC 1122 §3.3.1.4](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.1.4) | MUST | `Missing` | results | h | searched [networklayer/ipv4/Ipv4.cc](../../../src/inet/networklayer/ipv4/Ipv4.cc) (all 1499 lines), Ipv4RoutingTable.cc, Ipv4Route.h/.cc, Ipv4InterfaceData.h … |
| <a id="ipv4-260"></a>IPV4-260 | New default gateway on failure | [RFC 1122 §3.3.1.5](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.1.5) | MUST | `Missing` | results | h | [networklayer/ipv4/Ipv4RoutingTable.cc:476](../../../src/inet/networklayer/ipv4/Ipv4RoutingTable.cc#L476)-480; :426-430; searched Ipv4.cc for any gateway-failure path (none) |
| <a id="ipv4-261"></a>IPV4-261 | Sending bound to source interface | [RFC 1122 §3.3.4.2](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.4.2) | MAY | `Non-gap` | -- | h | [networklayer/ipv4/Ipv4.cc:1060](../../../src/inet/networklayer/ipv4/Ipv4.cc#L1060)-1067 (encapsulate source check); :621-660 (routeUnicastPacket) |
| <a id="ipv4-262"></a>IPV4-262 | Limited broadcast preferred by host | [RFC 1122 §3.3.6](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.6) | SHOULD | `Partial` | scope | h | [networklayer/ipv4/Ipv4.cc:612](../../../src/inet/networklayer/ipv4/Ipv4.cc#L612)-613, :682-712; [networklayer/ipv4/Ipv4.ned:101](../../../src/inet/networklayer/ipv4/Ipv4.ned#L101) (limitedBroadcast=default(false)) … |
| — | Mandatory IP extensions | [RFC 1812 §4.2.1](https://www.rfc-editor.org/rfc/rfc1812#section-4.2.1) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4RoutingTable.cc:375](../../../src/inet/networklayer/ipv4/Ipv4RoutingTable.cc#L375)-379 (classless longest-prefix match), :669-706 (per-interface netmask routes) … |
| — | Router-id stability | [RFC 1812 §4.2.2.2](https://www.rfc-editor.org/rfc/rfc1812#section-4.2.2.2) | MUST | `Full` | -- | m | [networklayer/ipv4/Ipv4RoutingTable.cc:119](../../../src/inet/networklayer/ipv4/Ipv4RoutingTable.cc#L119)-141 (configureRouterId), :83-99 (init), :712-745 (handleOperationStage) … |
| — | Reserved bits zero when originating | [RFC 1812 §4.2.2.3](https://www.rfc-editor.org/rfc/rfc1812#section-4.2.2.3) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4Header.msg:177](../../../src/inet/networklayer/ipv4/Ipv4Header.msg#L177),181; [networklayer/ipv4/Ipv4.cc:1040](../../../src/inet/networklayer/ipv4/Ipv4.cc#L1040)-1145 (encapsulate) … |
| — | Broadcast packets processed normally | [RFC 1812 §4.2.2.11](https://www.rfc-editor.org/rfc/rfc1812#section-4.2.2.11) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:371](../../../src/inet/networklayer/ipv4/Ipv4.cc#L371)-377, :388-408; [networklayer/ipv4/Ipv4RoutingTable.cc:337](../../../src/inet/networklayer/ipv4/Ipv4RoutingTable.cc#L337)-350 |
| <a id="ipv4-263"></a>IPV4-263 | Limited broadcast when originating | [RFC 1812 §4.2.3.1](https://www.rfc-editor.org/rfc/rfc1812#section-4.2.3.1) | SHOULD | `Partial` | scope | h | [networklayer/ipv4/Ipv4.cc:391](../../../src/inet/networklayer/ipv4/Ipv4.cc#L391) (receive), :682-712 (originate); [networklayer/ipv4/Ipv4.ned:101](../../../src/inet/networklayer/ipv4/Ipv4.ned#L101) |
| — | Variable length network prefixes | [RFC 1812 §4.2.3.4](https://www.rfc-editor.org/rfc/rfc1812#section-4.2.3.4) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4RoutingTable.cc:375](../../../src/inet/networklayer/ipv4/Ipv4RoutingTable.cc#L375)-379, :435-452 (routeLessThan), :462-467 … |
| <a id="ipv4-264"></a>IPV4-264 | Local versus remote decision algorithm | [RFC 1812 §5.2.4.2](https://www.rfc-editor.org/rfc/rfc1812#section-5.2.4.2) | MUST | `Non-gap` | -- | h | [networklayer/ipv4/Ipv4.cc:621](../../../src/inet/networklayer/ipv4/Ipv4.cc#L621)-660, :931-938; [networklayer/ipv4/Ipv4RoutingTable.cc:669](../../../src/inet/networklayer/ipv4/Ipv4RoutingTable.cc#L669)-706 |
| <a id="ipv4-265"></a>IPV4-265 | TOS-aware route selection | [RFC 1812 §5.3.2](https://www.rfc-editor.org/rfc/rfc1812#section-5.3.2) | SHOULD | `Missing` | results | h | [networklayer/ipv4/Ipv4Route.h:26](../../../src/inet/networklayer/ipv4/Ipv4Route.h#L26)-36; [networklayer/ipv4/Ipv4RoutingTable.cc:365](../../../src/inet/networklayer/ipv4/Ipv4RoutingTable.cc#L365)-384; [networklayer/ipv4/Ipv4.cc:649](../../../src/inet/networklayer/ipv4/Ipv4.cc#L649)-655 |
| — | All precedence levels processed | [RFC 1812 §5.3.3.3](https://www.rfc-editor.org/rfc/rfc1812#section-5.3.3.3) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:268](../../../src/inet/networklayer/ipv4/Ipv4.cc#L268)-316 (handleIncomingDatagram), :344-421 (preroutingFinish), :904 |
| — | Precedence never rewritten | [RFC 1812 §5.3.3.3](https://www.rfc-editor.org/rfc/rfc1812#section-5.3.3.3) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4.cc:320](../../../src/inet/networklayer/ipv4/Ipv4.cc#L320)-326 (prepareForForwarding), :409-420, :1073-1086 |
| <a id="ipv4-266"></a>IPV4-266 | Limited broadcast preferred by router | [RFC 1812 §5.3.5.1](https://www.rfc-editor.org/rfc/rfc1812#section-5.3.5.1) | SHOULD | `Partial` | scope | h | [networklayer/ipv4/Ipv4.cc:612](../../../src/inet/networklayer/ipv4/Ipv4.cc#L612)-613, :682-712; [networklayer/ipv4/Ipv4.ned:101](../../../src/inet/networklayer/ipv4/Ipv4.ned#L101) |
| — | Directed broadcast classification | [RFC 1812 §5.3.5.2](https://www.rfc-editor.org/rfc/rfc1812#section-5.3.5.2) | MUST | `Full` | -- | m | [networklayer/ipv4/Ipv4.cc:388](../../../src/inet/networklayer/ipv4/Ipv4.cc#L388)-408; [networklayer/ipv4/Ipv4RoutingTable.cc:337](../../../src/inet/networklayer/ipv4/Ipv4RoutingTable.cc#L337)-350; [networklayer/ipv4/Ipv4.ned:102](../../../src/inet/networklayer/ipv4/Ipv4.ned#L102) |
| <a id="ipv4-267"></a>IPV4-267 | Martian check switch default | [RFC 1812 §5.3.7](https://www.rfc-editor.org/rfc/rfc1812#section-5.3.7) | SHOULD | `Missing` | results | h | searched [networklayer/ipv4/Ipv4.cc](../../../src/inet/networklayer/ipv4/Ipv4.cc) (all 1499 lines), Ipv4RoutingTable.cc, Ipv4InterfaceData.cc, Ipv4Route.cc, and networ … |
| <a id="ipv4-268"></a>IPV4-268 | Reverse-path source validation | [RFC 1812 §5.3.8](https://www.rfc-editor.org/rfc/rfc1812#section-5.3.8) | SHOULD | `Missing` | scope | h | grep for reverse-path/uRPF/source-validation across src/inet returns only multicast RPF: [networklayer/ipv4/Ipv4.cc:758](../../../src/inet/networklayer/ipv4/Ipv4.cc#L758)-7 … |
| <a id="ipv4-269"></a>IPV4-269 | Per-interface forwarding control | [RFC 1812 §5.3.11](https://www.rfc-editor.org/rfc/rfc1812#section-5.3.11) | SHOULD | `Missing` | scope | h | [networklayer/ipv4/Ipv4RoutingTable.h:71](../../../src/inet/networklayer/ipv4/Ipv4RoutingTable.h#L71),183-190 (single node-wide flag) … |
| <a id="ipv4-270"></a>IPV4-270 | Route withdrawal on interface failure | [RFC 1812 §5.3.12.3](https://www.rfc-editor.org/rfc/rfc1812#section-5.3.12.3) | MUST | `Partial` | results | h | [networklayer/ipv4/Ipv4RoutingTable.cc:174](../../../src/inet/networklayer/ipv4/Ipv4RoutingTable.cc#L174)-183 (interfaceStateChangedSignal), :669-706 and :690 (updateNetmaskRoutes), :2 … |
| — | Static routes on interface recovery | [RFC 1812 §5.3.12.4](https://www.rfc-editor.org/rfc/rfc1812#section-5.3.12.4) | MUST | `Full` | -- | h | [networklayer/ipv4/Ipv4RoutingTable.cc:174](../../../src/inet/networklayer/ipv4/Ipv4RoutingTable.cc#L174)-183, :669-706, :690 |

## 4. Findings

Grouped by root cause, because one defect often spans several requirements. The matrix stays
one row per requirement. This section states what is actually wrong.

### <a id="f1"></a>F1 — The receive path performs no validation

[`IPV4-002`](#ipv4-002), [`IPV4-008`](#ipv4-008), [`IPV4-089`](#ipv4-089), [`IPV4-091`](#ipv4-091) · `Incorrect` · `results` · high
[`IPV4-147`](#ipv4-147) · `Partial` · `results` · high

`[src/inet/networklayer/ipv4/Ipv4.cc:282](../../../src/inet/networklayer/ipv4/Ipv4.cc#L282)`:

```cpp
if (!ipv4Header->isCorrect() && !ipv4Header->verifyChecksum()) {
```

A datagram is dropped only when the header chunk is already flagged incorrect **and** the checksum
fails. A well-formed header has `isCorrect() == true`, so the condition short-circuits and the
datagram is accepted. Compare `Udp.cc:1006`, which combines two *positive* tests with `&&`. The
polarity here is inverted.

Because the version check and the header-length check sit behind the same guard, they never run
either. RFC 1812 §5.2.2 requires both.

A second, independent reason the check cannot fire: `Ipv4Header::verifyChecksum()` returns a bare
`true` when the mode is `CHECKSUM_DECLARED_CORRECT`, which is INET's default. Even a corrected
guard would pass everything until `checksumMode` is set to `"computed"`.

**Reproduced.** Five injected datagrams — including one whose own `verifyChecksum()` returned
false, one with version 6, and one with IHL 4 — all reached the receiving application. The message
`checksum error found, drop packet` never appeared.

**Consequence.** No simulation can study checksum-related loss, and a corrupted header propagates
through a whole topology.

The same guard also defeats a MUST that looks unrelated. [`IPV4-147`](#ipv4-147): RFC 792 requires a
node that cannot process a header because of bad parameters to **discard** the datagram. INET's two
Parameter Problem triggers are a total-length mismatch ([Ipv4.cc:291](../../../src/inet/networklayer/ipv4/Ipv4.cc#L291))
and a bit error ([Ipv4.cc:306](../../../src/inet/networklayer/ipv4/Ipv4.cc#L306)). A bad version or a
short header length is flagged by `markIncorrect()` at parse time instead, and then falls through
line 282 — so it is neither discarded nor reported. Found by the calibration spot check, not by the
review passes.

### <a id="f2"></a>F2 — Three legal packets abort the simulation

[`IPV4-065`](#ipv4-065), [`IPV4-066`](#ipv4-066) · `Incorrect` · `results` · high
[`IPV4-087`](#ipv4-087) · `Incorrect` · `scope` · high

`Icmp.cc:325` ends the type switch with `throw cRuntimeError("Unknown ICMP type %d")`. RFC 1122
§3.2.2 requires an unknown type to be silently discarded, and RFC 6633 §3 requires the same for a
Source Quench.

`Arp.cc:270` throws `cRuntimeError("wrong ARP packet: source IPv4 address is empty")`. An all-zero
sender address is not malformed — RFC 5227 §2.1.1 defines the ARP Probe exactly that way, and §2.5
requires the address owner to answer it.

**Reproduced.** ICMP types 4, 17, and 200 each aborted the run. A probe-shaped ARP request aborted
the receiving node's simulation.

**Consequence.** A user who models any peer that emits these packets cannot run at all. The failure
mode is a crash, not a wrong result, so it is loud — but it is also unavoidable.

### <a id="f3"></a>F3 — Reassembly has no timer

[`IPV4-012`](#ipv4-012), [`IPV4-013`](#ipv4-013), [`IPV4-052`](#ipv4-052), [`IPV4-124`](#ipv4-124) · `Partial` and `Incorrect` · `results` · high

`Ipv4FragBuf::purgeStaleFragments` runs only when another fragment arrives. A reassembly that
stalls completely is never released and never produces an ICMP. `fragmentTimeout` is therefore not
the timer it appears to be. A hardcoded 10-second sweep granularity coarsens it further.

When the purge does run, it sends Time Exceeded with code 0, "time to live exceeded in transit".
RFC 1122 §3.3.2 requires code 1, "fragment reassembly time exceeded".

**Reproduced.** The purge fired at t=100s, when an unrelated fragment happened to arrive, and the
emitted message carried code 0.

Two related gaps sit in the same buffer. The reassembly key is identification, source, and
destination ([`IPV4-016`](#ipv4-016)) — RFC 791 §3.2 also keys on the protocol field, so two protocols can collide. And there
is no EMTU_R bound at all ([`IPV4-126`](#ipv4-126)), which RFC 1122 §3.3.2 requires to be at least 576 and either
configurable or indefinite.

### <a id="f4"></a>F4 — IP options are carried, never processed

[`IPV4-035`](#ipv4-035) to [`IPV4-049`](#ipv4-049) · mostly `Missing` · `scope`

INET parses every option, re-serializes it, and hands the parsed header to the transport layer
through `NetworkProtocolInd`. Nothing acts on one. No node inserts a Record Route hop, follows a
source route, or writes a timestamp while forwarding.

Two options are worse than unprocessed:

- [`IPV4-041`](#ipv4-041) — fragmentation copies the whole header into every fragment, ignoring the copy bit.
  RFC 791 §3.1 puts Record Route and Timestamp in the first fragment only. The code carries a
  `FIXME` at `Ipv4.cc:987` admitting it. **Reproduced:** a copy-bit-0 Timestamp option appeared in
  every fragment.
- [`IPV4-047`](#ipv4-047) — `enableTimestampOption` builds an option of length 0 that does not grow
  `headerLength`. **Reproduced:** the option is dropped on serialization, and in the other path an
  assertion aborts the run. The one option INET actively inserts is malformed.

An earlier draft of this review claimed that received options never reach the upper layer. The
refutation pass disproved it: `Ipv4.cc:279` attaches the parsed header, and `Ipv4.cc:897` preserves
it across decapsulation. That claim is withdrawn.

### <a id="f5"></a>F5 — RFC 5227 address conflict detection is absent

[`IPV4-077`](#ipv4-077) to [`IPV4-087`](#ipv4-087) · `Missing` · `scope`

None of probing, announcing, ongoing detection, or the defend-and-defer behavior exists.

`Arp::sendArpProbe` and `Arp::sendArpGratuitous` are present and have **zero callers** anywhere in
the repository — source, examples, showcases, tests, and configuration files. A full-tree search
confirmed it. Both are also wrong where they do exist: `Arp.cc:456` sets the target hardware address
to `ff:ff:ff:ff:ff:ff` where RFC 5227 §2.3 requires all zeroes, and `Arp.cc:469` inserts a
self-referential cache entry whose MAC is never set, under a commented-out `updateARPCache` call.

**Consequence.** Address conflict cannot be studied. Worse, the dormant builder produces the packet
that F2 shows will crash a receiving node.

### <a id="f6"></a>F6 — ICMP message types that do not exist

[`IPV4-053`](#ipv4-053), [`IPV4-056`](#ipv4-056) to [`IPV4-060`](#ipv4-060), [`IPV4-068`](#ipv4-068) · `Missing` · `scope`

Redirect, Timestamp and Timestamp Reply, Address Mask Request and Reply, and Parameter Problem's
Pointer field are unimplemented. Two consequences stand out:

- [`IPV4-059`](#ipv4-059) — a Timestamp Request is routed into `processEchoRequest` and answered with a type 0
  Echo Reply carrying no timestamps. **Reproduced.**
- [`IPV4-110`](#ipv4-110) — `Ipv4` *generates* Parameter Problem, and `IcmpHeaderSerializer`'s `default:` arm
  throws for it. The message cannot cross a serializing link.

Redirect is neither generated nor consumed, so [`IPV4-068`](#ipv4-068)'s route-update requirement has nothing to
act on.

### <a id="f7"></a>F7 — Required limits and bounds do not exist

[`IPV4-074`](#ipv4-074), [`IPV4-126`](#ipv4-126), [`IPV4-132`](#ipv4-132), [`IPV4-134`](#ipv4-134) · `Missing` · `scope`

No ICMP error rate limit (RFC 1812 §4.3.2.8). No reassembly buffer bound (RFC 1122 §3.3.2). No ARP
flood prevention (RFC 1122 §2.3.2.1). Each is a mandated safeguard against a resource attack, and
each is absent rather than simplified.

### <a id="f8"></a>F8 — Router forwarding checks that are absent by omission

[`IPV4-092`](#ipv4-092), [`IPV4-093`](#ipv4-093), [`IPV4-095`](#ipv4-095) · `Missing` and `Partial` · `scope`

No martian source filtering, no martian destination filtering, and no link-layer broadcast check on
ingress. These are violated by having no code, so they are invisible to a reader who looks for a
wrong line. `Ipv4.cc:922` also silently rewrites a 0.0.0.0 source address rather than rejecting the
datagram.

### <a id="f9"></a>F9 — The identification field repeats

[`IPV4-018`](#ipv4-018) · `Partial` · `cosmetic`

A single node-wide 16-bit counter (`Ipv4.h:84`). RFC 6864 §4 scopes uniqueness to the
source-destination-protocol tuple for a non-atomic datagram. After 65536 datagrams the values
repeat, which can alias two reassemblies — an effect the missing protocol field in the reassembly
key (F3) makes easier to hit.

### <a id="f10"></a>F10 — The routing table cannot hold two default gateways

`Ipv4RoutingTable.cc:476` · `Partial` · `results` · high

`internalAddRoute` deletes the existing default route before inserting a new one, and it is the only
insertion path. A table can therefore hold exactly one default gateway. RFC 1122 §3.3.1.2 requires a
host to support several, so the requirement is not merely unimplemented — it is unsatisfiable
without changing the table.

Dead-gateway detection does not exist either. Together these make a failed first hop a permanent
black hole rather than a failover, in any simulation with more than one gateway.

Two further routing defects sit beside it. An interface going down withdraws only its netmask
routes, so a static route pointing at a down interface keeps winning lookups. And no martian
filtering or reverse-path check exists anywhere in `src/inet`.

**Refuted down from `Missing` to `Partial`.** The route cache and the default-route fallback both
work. The adversarial pass was aimed at this claim specifically and could not break it.

### <a id="f11"></a>F11 — RFC 5227 was built and never connected

`Missing` · `scope` · high

Round 1 established that `Arp::sendArpProbe` and `Arp::sendArpGratuitous` have zero callers anywhere
in the repository. Round 2 found the third piece: `DhcpClient`'s address-conflict check and its
`sendDecline` call are commented out at `DhcpClient.cc:313`, which leaves `sendDecline` dead too.

So this is not an unwritten feature. Three separate pieces exist and none is wired to anything. All
ten RFC 5227 timing constants are absent, and no code compares a received `ar$spa` against the
node's own addresses.

The work needed is wiring, not implementation — which makes it a cheaper fix than its row count
suggests.

### <a id="f12"></a>F12 — Seven ICMP types abort the simulation, not three

`Incorrect` · `results` · high

Round 1 reproduced the `Icmp.cc:325` default-case throw for types 4, 17 and 200. Round 2 enumerated
the receive-side duties round 1 had skipped, and the same throw is reached by types **4, 9, 10, 15,
16, 17 and 18** — Source Quench, Router Advertisement and Solicitation, Information Request and
Reply, and Address Mask Request and Reply.

RFC 1122 §3.2.2, RFC 1812 §4.3.2.1 and RFC 6633 §3 and §4 each require a silent discard or an
ignore. The committed test `IcmpUnknownTypeDiscard.test` already covers this mechanism.

### <a id="f13"></a>F13 — ECN uses the wrong codepoint, and sets it on the SYN-ACK

`Incorrect` · `results` · high

`TcpConnectionUtil.cc:317` sets `IP_ECN_ECT_1` unconditionally, with a comment saying the choice was
deliberate. RFC 3168 §5 and §6.1.2 say a sender needing only one ECT codepoint SHOULD use ECT(0), and
nothing in INET distinguishes the two — `IP_ECN_ECT_0` appears nowhere in `src/` outside its own enum
definition, and `RedDropper` only tests for "not Not-ECT".

**Reproduced:** 22 segments ECT(1), 23 Not-ECT, zero ECT(0) across one transfer.

The same run showed a separate violation the review had not looked for: the server's SYN-ACK carries
ECT(1). RFC 3168 §6.1.1 forbids setting ECT on a SYN or a SYN-ACK. The client's SYN is correct.

This lives in TCP, not in this family. The requirement is an IP-layer one, so it is reported here and
should be fixed there.

### <a id="f14"></a>F14 — An ICMP error answers a packet that must be discarded silently

`Incorrect` · `scope` · high

RFC 1812 §4.3.2.7 forbids an ICMP error for a packet that failed header validation. `Ipv4.cc:303`
answers a header bit error with a Parameter Problem instead.

Two details matter, and the second reduces the severity:

- The guard is broken. `relativeHeaderLength` divides the header chunk's length by *its own* chunk
  length, so it is always 1.0 and the branch always fires. The comment "ignore bit error if in
  payload" describes something that is not implemented.
- **The branch is unreachable in a normal configuration.** No stock INET link layer hands a
  bit-errored frame to IPv4 — Ppp, every Ethernet MAC, CsmaCaMac, AckingMac, the 802.11 receiver, the
  low-power MACs and `ChecksumCheckerBase` all drop first. Reproducing it needed a test-only error
  injector.

A third defect appeared while reproducing it: with `checksumMode = "computed"` the run dies inside
`Icmp` itself, because `insertChecksum()` serializes a type-12 header and the serializer has no case
for it.

## 5. Defaults and configuration

The code below is conformant. Only the default value is questionable. These stay out of the matrix
for that reason.

| ID | Parameter | Default | Spec or practice | Consequence |
|---|---|---|---|---|
| <a id="ipv4-139"></a>IPV4-139 | `Ipv4.checksumMode`, `Icmp.checksumMode` | `"declared"` | [RFC 1122 §3.2.1.2](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.1.2) | The checksum is asserted, never computed. `verifyChecksum()` cannot fail. Compounds F1. |
| <a id="ipv4-140"></a>IPV4-140 | `Ipv4.timeToLive` | `32` | [RFC 1122 §3.2.1.7](https://www.rfc-editor.org/rfc/rfc1122#section-3.2.1.7) recommends 64 | Every default simulation halves the reachable hop count. |
| <a id="ipv4-141"></a>IPV4-141 | `Ipv4.multicastTimeToLive` | `32` | as above | Same, for multicast. |
| <a id="ipv4-142"></a>IPV4-142 | `Arp.proxyArpInterfaces` | `"*"` | [RFC 1027](https://www.rfc-editor.org/rfc/rfc1027); not a default in any real stack | Proxy ARP answers on every interface unless disabled. A node answers for addresses it does not own. |
| <a id="ipv4-143"></a>IPV4-143 | `Ipv4.enableTimestampOption` | `false` | [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | Off by default, and malformed when on. See [`IPV4-047`](#ipv4-047). |
| <a id="ipv4-144"></a>IPV4-144 | `Tcp.pmtudEnabled` | `false` | [RFC 1191](https://www.rfc-editor.org/rfc/rfc1191) | Path MTU discovery is off, and lives in TCP rather than in the network layer. |
| <a id="ipv4-145"></a>IPV4-145 | `Icmp.quoteLength` | `8B` | [RFC 1812 §4.3.2.3](https://www.rfc-editor.org/rfc/rfc1812#section-4.3.2.3) recommends as much as fits in 576 octets | An ICMP error quotes the historic minimum, so a receiver cannot always identify the flow. |
| <a id="ipv4-146"></a>IPV4-146 | `Ipv4.limitedBroadcast` | `false` | [RFC 1122 §3.3.6](https://www.rfc-editor.org/rfc/rfc1122#section-3.3.6) | A datagram to 255.255.255.255 from the layer above is not sent as a limited broadcast unless enabled. |

## 6. Observations

Defects with no specification content, noticed while reading. Not investigated. Route them to
`/code-review`.

- `doc/src/developers-guide/ch-ipv4.rst` documents an interface that no longer exists: a `procDelay`
  parameter, a `QueueBase` base class, an `ICMPAccess` accessor, and gates `errOut` and `pingIn`.
  The chapter needs a refresh.
- `[src/inet/networklayer/ipv4/headers/ip_icmp.h](../../../src/inet/networklayer/ipv4/headers/ip_icmp.h)` is dead code. Nothing includes it.
- `Ipv4.cc:987` carries a `FIXME` that names its own defect ([`IPV4-041`](#ipv4-041)).
- `Arp.cc:469` carries a commented-out `updateARPCache` call under a `FIXME`.
- `IcmpHeader.code` defaults to `-1` and serializes as `0xFF` for every type that does not override
  it ([`IPV4-109`](#ipv4-109)).
- There is **no ARP module test** anywhere in `tests/module/`. Confirmed by search.
- [`IPV4-138`](#ipv4-138) in section 3h covers the IGMPv2 and IGMPv3 timers. IGMP is outside this family, so
  that row belongs to the IGMP review and should be read as `Non-gap` here.
- The seven tests written to reproduce these findings now live in `tests/protocol/conformance/`.
  Each asserts the standard's behavior and carries an `%expected-failure:` naming its finding id, so
  each flips to `PASS` by itself when its defect is fixed. Three cover the F2 aborts, which had no
  coverage at all, and ARP had no test of any kind.
- `inet_run_protocol_tests` globs `tests/protocol/*.test` without recursing
  (`python/inet/test/all.py`, `get_opp_test_tasks`), so `tests/protocol/conformance/` is not
  discovered by the runner. Registering the folder is a one-line change. Until then the tests run
  directly under `opp_test`.

## 7. Closed

No finding has been fixed yet.

| ID | Feature | Closed by | Date |
|---|---|---|---|

## 8. Accepted omissions

`accepted-omissions.md` does not exist yet. Three rows are **proposed** for it, and none is accepted
until you decide. Until then they stay in the matrix as gaps.

| Proposed | Row | Why the agent proposed it |
|---|---|---|
| [`IPV4-042`](#ipv4-042) | Security option, [RFC 791 §3.1](https://www.rfc-editor.org/rfc/rfc791#section-3.1) | Enumerated in `Ipv4OptionType`, no typed class. No real stack implements it. |
| [`IPV4-075`](#ipv4-075) | ARP `ar$hrd`, [RFC 826](https://www.rfc-editor.org/rfc/rfc826) | The serializer writes the literal 1. INET models Ethernet only. |
| [`IPV4-076`](#ipv4-076) | ARP `ar$pro`, [RFC 826](https://www.rfc-editor.org/rfc/rfc826) | The serializer writes the literal `ETHERTYPE_IPv4`. INET resolves IPv4 only. |

Accepting a row moves it into the ledger, where a future review will not report it again. That is
why only you can do it.
