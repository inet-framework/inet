# IPv6 — coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rfc8200/catalog.md](../../standard/rfc8200/catalog.md), [rfc4443/catalog.md](../../standard/rfc4443/catalog.md), [features.md](../../protocol/ipv6/features.md), [results.md](results.md)

The single place that holds the changing state of the IPv6 workflow. Every other artifact
of this protocol states what a standard says and never changes again unless the standard
changes. This one changes on every pass.

**No step edits an artifact of an earlier step. Steps 5, 6 and 7 record their outcome here.**

The ledger spans the documents of the in-scope set, so one table mixes RFC 8200 and
RFC 4443 identifiers: a check may establish statements of two documents at once.

State of the ledger: run of 2026-09-09, tree at commit `95f9805952`, target level 2.

## Statement coverage

`Status` is the state of the workflow, not of the standard: `selected` (a check targets it),
`covered` (a selected check establishes it as a side effect), `candidate` (practical, not yet
chosen), `later` (needs a toolset or a category beyond this pass), `constraint` (a rule the
scenarios obey rather than a behavior a check observes).

| Catalog ID | Status | Check section | Test file | Verdict |
| --- | --- | --- | --- | --- |
| [RFC8200-HDR-1](../../standard/rfc8200/catalog.md#rfc8200-hdr-1) | selected | [datagram-delivery](../../protocol/ipv6/checks.md#datagram-delivery) | Rfc8200DatagramDelivery.test | PASS |
| [RFC8200-HDR-2](../../standard/rfc8200/catalog.md#rfc8200-hdr-2) | selected | [datagram-delivery](../../protocol/ipv6/checks.md#datagram-delivery), [fragment-payload-length](../../protocol/ipv6/checks.md#fragment-payload-length) | Rfc8200DatagramDelivery.test, Rfc8200FragmentPayloadLength.test | PASS (whole packet); **FAIL** (fragments, model gap) |
| [RFC8200-HDR-3](../../standard/rfc8200/catalog.md#rfc8200-hdr-3) | selected | [datagram-delivery](../../protocol/ipv6/checks.md#datagram-delivery), [source-fragmentation-and-reassembly](../../protocol/ipv6/checks.md#source-fragmentation-and-reassembly) | Rfc8200DatagramDelivery.test, Rfc8200SourceFragmentation.test | PASS |
| [RFC8200-HDR-4](../../standard/rfc8200/catalog.md#rfc8200-hdr-4) | selected | [datagram-delivery](../../protocol/ipv6/checks.md#datagram-delivery) | Rfc8200DatagramDelivery.test | PASS |
| [RFC8200-DLV-1](../../standard/rfc8200/catalog.md#rfc8200-dlv-1) | selected | [datagram-delivery](../../protocol/ipv6/checks.md#datagram-delivery) | Rfc8200DatagramDelivery.test | PASS |
| [RFC8200-HL-1](../../standard/rfc8200/catalog.md#rfc8200-hl-1) | selected | [datagram-delivery](../../protocol/ipv6/checks.md#datagram-delivery), [hop-limit-1-at-the-destination](../../protocol/ipv6/checks.md#hop-limit-1-at-the-destination) | Rfc8200DatagramDelivery.test, Rfc8200HopLimitOneAtDestination.test | PASS |
| [RFC8200-HL-2](../../standard/rfc8200/catalog.md#rfc8200-hl-2) | selected | [hop-limit-expiry](../../protocol/ipv6/checks.md#hop-limit-expiry) | Rfc8200HopLimitExpiry.test | PASS |
| [RFC8200-HL-3](../../standard/rfc8200/catalog.md#rfc8200-hl-3) | selected | [hop-limit-1-at-the-destination](../../protocol/ipv6/checks.md#hop-limit-1-at-the-destination) | Rfc8200HopLimitOneAtDestination.test | PASS (the boundary hop limit 1; hop limit 0 is level 3) |
| [RFC8200-EXT-1](../../standard/rfc8200/catalog.md#rfc8200-ext-1) | selected | [source-fragmentation-and-reassembly](../../protocol/ipv6/checks.md#source-fragmentation-and-reassembly) | Rfc8200SourceFragmentation.test | PASS |
| [RFC8200-FRAG-1](../../standard/rfc8200/catalog.md#rfc8200-frag-1) | selected | [packet-too-big](../../protocol/ipv6/checks.md#packet-too-big) | Rfc4443PacketTooBig.test | PASS |
| [RFC8200-FRAG-2](../../standard/rfc8200/catalog.md#rfc8200-frag-2) | selected | [fragment-identification](../../protocol/ipv6/checks.md#fragment-identification) | Rfc8200FragmentIdentification.test | PASS |
| [RFC8200-FRAG-3](../../standard/rfc8200/catalog.md#rfc8200-frag-3) | selected | [source-fragmentation-and-reassembly](../../protocol/ipv6/checks.md#source-fragmentation-and-reassembly) | Rfc8200SourceFragmentation.test | PASS |
| [RFC8200-FRAG-4](../../standard/rfc8200/catalog.md#rfc8200-frag-4) | selected | [source-fragmentation-and-reassembly](../../protocol/ipv6/checks.md#source-fragmentation-and-reassembly), [fragment-payload-length](../../protocol/ipv6/checks.md#fragment-payload-length) | Rfc8200SourceFragmentation.test, Rfc8200FragmentPayloadLength.test | PASS (structure); **FAIL** (payload length, model gap) |
| [RFC8200-FRAG-5](../../standard/rfc8200/catalog.md#rfc8200-frag-5) | selected | [source-fragmentation-and-reassembly](../../protocol/ipv6/checks.md#source-fragmentation-and-reassembly), [fragment-payload-length](../../protocol/ipv6/checks.md#fragment-payload-length) | Rfc8200SourceFragmentation.test, Rfc8200FragmentPayloadLength.test | PASS (structure); **FAIL** (payload length, model gap) |
| [RFC8200-FRAG-6](../../standard/rfc8200/catalog.md#rfc8200-frag-6) | selected | [source-fragmentation-and-reassembly](../../protocol/ipv6/checks.md#source-fragmentation-and-reassembly) | Rfc8200SourceFragmentation.test | PASS |
| [RFC8200-REASM-1](../../standard/rfc8200/catalog.md#rfc8200-reasm-1) | selected | [source-fragmentation-and-reassembly](../../protocol/ipv6/checks.md#source-fragmentation-and-reassembly) | Rfc8200SourceFragmentation.test | PASS |
| [RFC8200-REASM-2](../../standard/rfc8200/catalog.md#rfc8200-reasm-2) | selected | [source-fragmentation-and-reassembly](../../protocol/ipv6/checks.md#source-fragmentation-and-reassembly) | Rfc8200SourceFragmentation.test | PASS |
| [RFC8200-REASM-3](../../standard/rfc8200/catalog.md#rfc8200-reasm-3) | later (level 4: a timer) | — | — | — |
| [RFC8200-REASM-4](../../standard/rfc8200/catalog.md#rfc8200-reasm-4) | later (level 3: a crafted fragment) | — | — | — |
| [RFC8200-REASM-5](../../standard/rfc8200/catalog.md#rfc8200-reasm-5) | later (level 3: crafted fragments) | — | — | — |
| [RFC8200-REASM-6](../../standard/rfc8200/catalog.md#rfc8200-reasm-6) | later (level 3: a crafted fragment header) | — | — | — |
| [RFC8200-MTU-1](../../standard/rfc8200/catalog.md#rfc8200-mtu-1) | constraint (every scenario obeys it) | — | — | — |
| [RFC8200-MTU-2](../../standard/rfc8200/catalog.md#rfc8200-mtu-2) | selected | [link-mtu-packet](../../protocol/ipv6/checks.md#link-mtu-packet) | Rfc8200LinkMtuPacket.test | PASS |
| [RFC8200-MTU-3](../../standard/rfc8200/catalog.md#rfc8200-mtu-3) | later (level 4: RFC 8201) | — | — | — |
| [RFC8200-MTU-4](../../standard/rfc8200/catalog.md#rfc8200-mtu-4) | selected | [source-fragmentation-and-reassembly](../../protocol/ipv6/checks.md#source-fragmentation-and-reassembly) | Rfc8200SourceFragmentation.test | PASS |
| [RFC8200-MTU-5](../../standard/rfc8200/catalog.md#rfc8200-mtu-5) | candidate (a should for the upper layer) | — | — | — |
| [RFC8200-CKSUM-1](../../standard/rfc8200/catalog.md#rfc8200-cksum-1) | selected | [datagram-delivery](../../protocol/ipv6/checks.md#datagram-delivery) | Rfc8200DatagramDelivery.test | PASS (the compute half; the discard of a zero checksum is level 3) |
| [RFC8200-CKSUM-2](../../standard/rfc8200/catalog.md#rfc8200-cksum-2) | later (unit test: encoding) | — | — | — |
| [RFC4443-PTB-1](../../standard/rfc4443/catalog.md#rfc4443-ptb-1) | selected | [packet-too-big](../../protocol/ipv6/checks.md#packet-too-big) | Rfc4443PacketTooBig.test | PASS |
| [RFC4443-PTB-2](../../standard/rfc4443/catalog.md#rfc4443-ptb-2) | selected | [packet-too-big-the-mtu-field](../../protocol/ipv6/checks.md#packet-too-big-the-mtu-field) | Rfc4443PacketTooBigMtu.test | **FAIL** (model gap: MTU 0) |
| [RFC4443-TE-1](../../standard/rfc4443/catalog.md#rfc4443-te-1) | selected | [hop-limit-expiry](../../protocol/ipv6/checks.md#hop-limit-expiry) | Rfc8200HopLimitExpiry.test | PASS |
| [RFC4443-DU-4](../../standard/rfc4443/catalog.md#rfc4443-du-4) | candidate (a should; the UDP-over-IPv6 pass) | — | — | — |
| [RFC4443-ERR-1](../../standard/rfc4443/catalog.md#rfc4443-err-1) | later (level 3: the content of a report) | — | — | — |
| [RFC4443-ERR-2](../../standard/rfc4443/catalog.md#rfc4443-err-2) | selected | [hop-limit-expiry](../../protocol/ipv6/checks.md#hop-limit-expiry), [packet-too-big](../../protocol/ipv6/checks.md#packet-too-big) | Rfc8200HopLimitExpiry.test, Rfc4443PacketTooBig.test | PASS |

34 entries: 28 of RFC 8200, 6 of RFC 4443. 24 have a verdict from a check that ran: 20 PASS,
1 FAIL as a model gap, 3 with one PASS and one FAIL (the payload length of a fragment, which
is wrong while the rest of the fragment is right). 10 have none: 4 wait for level 3 (crafted
fragments, the content of a report), 2 for level 4 (the reassembly timer, path MTU
discovery), 1 for a unit test, 1 is a scenario constraint, and 2 are candidates no check
selected (two shoulds).

## Feature support

The rule of step 7: `supported` when every core check ran and passed, `partial` when a core
check passed and another core check failed as a model gap or did not run, `not supported`
when every core check that ran failed as a model gap, `untested` when no core check ran.

| Feature | Core checks and their state | Support |
| --- | --- | --- |
| [IPV6-F-HEADER](../../protocol/ipv6/features.md#ipv6-f-header) | RFC8200-HDR-1 PASS, HDR-3 PASS; HDR-2 PASS on a whole packet and FAIL on a fragment | **partial** |
| [IPV6-F-DELIVERY](../../protocol/ipv6/features.md#ipv6-f-delivery) | RFC8200-HDR-4 PASS, DLV-1 PASS | **supported** |
| [IPV6-F-HOP-LIMIT](../../protocol/ipv6/features.md#ipv6-f-hop-limit) | RFC8200-HL-1 PASS, HL-2 PASS | **supported** |
| [IPV6-F-SOURCE-FRAGMENTATION](../../protocol/ipv6/features.md#ipv6-f-source-fragmentation) | RFC8200-FRAG-2, FRAG-3, FRAG-6 PASS; FRAG-4 and FRAG-5 PASS in structure and FAIL in payload length | **partial** |
| [IPV6-F-REASSEMBLY](../../protocol/ipv6/features.md#ipv6-f-reassembly) | RFC8200-REASM-1, REASM-2, MTU-4 all PASS | **supported** |
| [IPV6-F-PACKET-TOO-BIG](../../protocol/ipv6/features.md#ipv6-f-packet-too-big) | RFC8200-FRAG-1 PASS, RFC4443-PTB-1 PASS; RFC4443-PTB-2 FAIL | **partial** |
| [IPV6-F-PACKET-SIZE](../../protocol/ipv6/features.md#ipv6-f-packet-size) | RFC8200-MTU-2 PASS | **supported** |
| [IPV6-F-ERROR-REPORT](../../protocol/ipv6/features.md#ipv6-f-error-report) | RFC4443-TE-1, PTB-1, ERR-2 all PASS | **supported** |
| [IPV6-F-UPPER-LAYER-CHECKSUM](../../protocol/ipv6/features.md#ipv6-f-upper-layer-checksum) | RFC8200-CKSUM-1 PASS (the compute half) | **supported** |

Six features supported, three partial. The three partial ones share two fields: the payload
length that a fragment should carry and does not, and the MTU that a Packet Too Big message
should carry and does not. The mechanisms around both fields work: the fragments are
well-formed otherwise and reassemble, and the report is sent to the right node with the
right type.

Two bounds on these values. A `supported` feature is supported as far as its checks reach:
one datagram size, one MTU, one topology per check. And the UDP checksum passed with the
model's computed mode enabled; in its default mode the field is a placeholder, which
[`results.md`](results.md) records.

## Achieved level

**Level 2 reached.** Target: level 2, from
[`standards.md`](../../protocol/ipv6/standards.md#target-level).

The exit criterion of level 2 has two halves, and both hold.

**Hold** — every normal-path mandatory mechanism of RFC 8200 appears as a feature. The
inventory below is what makes that claim checkable: every mechanism the document defines,
and where it went.

| RFC 8200 mechanism | Where | Feature, or the reason it is not one |
| --- | --- | --- |
| version, payload length, next header | §3 | IPV6-F-HEADER |
| traffic class | §3, §7 | out of scope: RFC 2474 and RFC 3168 define the byte; level 5 |
| flow label | §3, §6 | out of scope: RFC 6437 defines it; level 5 |
| hop limit | §3 | IPV6-F-HOP-LIMIT |
| source and destination addresses; demultiplexing at the destination | §3, §4 | IPV6-F-DELIVERY |
| extension headers in transit | §4 | IPV6-F-SOURCE-FRAGMENTATION (the fragment header); the other headers are level 5 |
| extension header order, hop-by-hop, routing, destination options, no next header | §4.1 to §4.4, §4.6 to §4.8 | level 5, by the level table of the guide |
| fragmentation at the source | §4.5 | IPV6-F-SOURCE-FRAGMENTATION, level `optional` |
| no fragmentation by routers; Packet Too Big | §4.5, §5; RFC 4443 §3.2 | IPV6-F-PACKET-TOO-BIG |
| reassembly, including the 1500-octet floor | §4.5, §5 | IPV6-F-REASSEMBLY |
| reassembly errors and the timer | §4.5 | supporting statements; levels 3 and 4 |
| the 1280-octet link MTU and the link-MTU packet | §5 | IPV6-F-PACKET-SIZE |
| path MTU discovery | §5, RFC 8201 | level 4 |
| upper-layer checksums | §8.1 | IPV6-F-UPPER-LAYER-CHECKSUM |
| maximum packet lifetime, upper-layer payload size, routing header responses | §8.2 to §8.4 | out of scope: a note without a rule, a transport rule, and a level 5 header |
| error reports through ICMPv6 | RFC 4443 | IPV6-F-ERROR-REPORT, level `mandatory` |
| security considerations | §10 | out of scope |

**Run** — every mandatory feature has a core check that ran and has a verdict: 8 of 8
mandatory features, and the one optional feature as well; every core check that belongs to
level 2 ran. Two of them failed, and a failed check is a verdict.

| Level | State | Evidence |
| --- | --- | --- |
| 1, Survey | **reached** | The standards map pins the in-scope set; [`conformance.md`](conformance.md) maps every model claim onto it and names the obsolete citation. |
| 2, Core | **reached** | The inventory above, and 24 of 24 level-2 statements with a verdict: 20 PASS, 1 FAIL, 3 with both. |
| 3, Edge | not started | RFC 8504 and the message processing rules of RFC 4443 §2.4; the crafted fragments of REASM-4 to REASM-6; a packet that arrives with hop limit 0; the zero UDP checksum; the content of a report (ERR-1). |
| 4, Dynamics | not started | the reassembly timer (REASM-3); path MTU discovery (RFC 8201, MTU-3), which the Packet Too Big gap blocks. |
| 5, Complete | not started | the other extension headers and their order, the flow label, the traffic class, jumbograms, the ICMPv6 query messages. |

Per feature, every one of the nine reaches level 2. Three of them reach it with a `partial`
support value, because a core check ran and failed; the two axes are different, and the
ledger keeps them apart.

## Pass log

| Pass | Date | Level | Scope | Result |
| --- | --- | --- | --- | --- |
| 1 | 2026-09-09 | **2, reached** | RFC 8200 + RFC 4443 error signals: 34 catalog entries, 9 features, 9 checks, 9 tests | 7 PASS, 2 FAIL (expected) as model gaps; 6 features supported, 3 partial; see [`results.md`](results.md) and [`conformance.md`](conformance.md) |

## Out of scope

The catalogs record what this pass left out of the standards: the extension headers other
than the fragment header, the flow label, the traffic class, jumbograms, the routing header
responses, the reassembly timer, the ICMPv6 message processing rules, and the ICMPv6
messages other than the three error reports. Each becomes level 3 to level 5 work.
