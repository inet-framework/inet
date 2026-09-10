# IPv6 — coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rfc8200/catalog.md](../../standard/rfc8200/catalog.md), [rfc4443/catalog.md](../../standard/rfc4443/catalog.md), [rfc8504/catalog.md](../../standard/rfc8504/catalog.md), [features.md](../../protocol/ipv6/features.md), [results.md](results.md)

The single place that holds the changing state of the IPv6 workflow. Every other artifact
of this protocol states what a standard says and never changes again unless the standard
changes. This one changes on every pass.

**No step edits an artifact of an earlier step. Steps 5, 6 and 7 record their outcome here.**

The ledger spans the documents of the in-scope set, so one table mixes RFC 8200, RFC 4443
and RFC 8504 identifiers: a check may establish statements of two documents at once, and a
later document may govern an earlier statement.

State of the ledger, from this run:

- Date: 2026-09-10 15:18 +0200
- INET: branch `master`, commit `0868c36c88`, tree clean
- OMNeT++: 6.4.0
- Build: debug, built from this commit
- Compiler: Ubuntu clang version 23.0.0
- Platform: Ubuntu 26.04.1 LTS, Linux 7.0.0-31-generic x86_64
- Command: `inet_run_protocol_tests -p inet -w ipv6`
- Target level: 3

## Statement coverage

`Status` is the state of the workflow, not of the standard: `selected` (a check targets it),
`covered` (a selected check establishes it as a side effect, or, for an `implementation`
statement, the rows of the document it names), `candidate` (practical, not yet chosen),
`later` (needs a toolset or a category beyond this pass), `constraint` (a rule the
scenarios obey), `superseded by X` (a later in-scope document governs; the check targets X,
and the row keeps the verdict of that check). A verdict of `declined` marks a `should` the
model does not follow, which is not a failure.

| Catalog ID | Status | Check section | Test file | Verdict |
| --- | --- | --- | --- | --- |
| [RFC8200-HDR-1](../../standard/rfc8200/catalog.md#rfc8200-hdr-1) | selected | [datagram-delivery](../../protocol/ipv6/checks/delivery.md#datagram-delivery) | Rfc8200DatagramDelivery.test | PASS |
| [RFC8200-HDR-2](../../standard/rfc8200/catalog.md#rfc8200-hdr-2) | selected | [datagram-delivery](../../protocol/ipv6/checks/delivery.md#datagram-delivery), [fragment-payload-length](../../protocol/ipv6/checks/fragmentation.md#fragment-payload-length) | Rfc8200DatagramDelivery.test, Rfc8200FragmentPayloadLength.test | PASS (whole packet); **FAIL** (fragments, model gap) |
| [RFC8200-HDR-3](../../standard/rfc8200/catalog.md#rfc8200-hdr-3) | selected | [datagram-delivery](../../protocol/ipv6/checks/delivery.md#datagram-delivery), [source-fragmentation-and-reassembly](../../protocol/ipv6/checks/fragmentation.md#source-fragmentation-and-reassembly) | Rfc8200DatagramDelivery.test, Rfc8200SourceFragmentation.test | PASS |
| [RFC8200-HDR-4](../../standard/rfc8200/catalog.md#rfc8200-hdr-4) | selected | [datagram-delivery](../../protocol/ipv6/checks/delivery.md#datagram-delivery) | Rfc8200DatagramDelivery.test | PASS |
| [RFC8200-DLV-1](../../standard/rfc8200/catalog.md#rfc8200-dlv-1) | selected | [datagram-delivery](../../protocol/ipv6/checks/delivery.md#datagram-delivery) | Rfc8200DatagramDelivery.test | PASS |
| [RFC8200-HL-1](../../standard/rfc8200/catalog.md#rfc8200-hl-1) | selected | [datagram-delivery](../../protocol/ipv6/checks/delivery.md#datagram-delivery), [hop-limit-1-at-the-destination](../../protocol/ipv6/checks/hop-limit.md#hop-limit-1-at-the-destination) | Rfc8200DatagramDelivery.test, Rfc8200HopLimitOneAtDestination.test | PASS |
| [RFC8200-HL-2](../../standard/rfc8200/catalog.md#rfc8200-hl-2) | selected | [hop-limit-expiry](../../protocol/ipv6/checks/hop-limit.md#hop-limit-expiry) | Rfc8200HopLimitExpiry.test | PASS |
| [RFC8200-HL-3](../../standard/rfc8200/catalog.md#rfc8200-hl-3) | selected | [hop-limit-1-at-the-destination](../../protocol/ipv6/checks/hop-limit.md#hop-limit-1-at-the-destination), [hop-limit-0-at-the-destination](../../protocol/ipv6/checks/hop-limit.md#hop-limit-0-at-the-destination) | Rfc8200HopLimitOneAtDestination.test, Rfc8200HopLimitZeroAtDestination.test | PASS (hop limit 1); PASS (hop limit 0) |
| [RFC8200-EXT-1](../../standard/rfc8200/catalog.md#rfc8200-ext-1) | selected | [source-fragmentation-and-reassembly](../../protocol/ipv6/checks/fragmentation.md#source-fragmentation-and-reassembly) | Rfc8200SourceFragmentation.test | PASS |
| [RFC8200-EXT-2](../../standard/rfc8200/catalog.md#rfc8200-ext-2) | later (level 5: needs two extension headers; internal) | — | — | — |
| [RFC8200-EXT-3](../../standard/rfc8200/catalog.md#rfc8200-ext-3) | superseded by RFC8504-NR-6 | [unrecognized-next-header](../../protocol/ipv6/checks/input-validation.md#unrecognized-next-header), [unassigned-next-header](../../protocol/ipv6/checks/input-validation.md#unassigned-next-header) | Rfc8200UnrecognizedNextHeader.test, Rfc8200UnassignedNextHeader.test | PASS |
| [RFC8200-FRAG-1](../../standard/rfc8200/catalog.md#rfc8200-frag-1) | selected | [packet-too-big](../../protocol/ipv6/checks/packet-too-big.md#packet-too-big) | Rfc4443PacketTooBig.test | PASS |
| [RFC8200-FRAG-2](../../standard/rfc8200/catalog.md#rfc8200-frag-2) | selected | [fragment-identification](../../protocol/ipv6/checks/fragmentation.md#fragment-identification) | Rfc8200FragmentIdentification.test | PASS |
| [RFC8200-FRAG-3](../../standard/rfc8200/catalog.md#rfc8200-frag-3) | selected | [source-fragmentation-and-reassembly](../../protocol/ipv6/checks/fragmentation.md#source-fragmentation-and-reassembly) | Rfc8200SourceFragmentation.test | PASS |
| [RFC8200-FRAG-4](../../standard/rfc8200/catalog.md#rfc8200-frag-4) | selected | [source-fragmentation-and-reassembly](../../protocol/ipv6/checks/fragmentation.md#source-fragmentation-and-reassembly), [fragment-payload-length](../../protocol/ipv6/checks/fragmentation.md#fragment-payload-length) | Rfc8200SourceFragmentation.test, Rfc8200FragmentPayloadLength.test | PASS (structure); **FAIL** (payload length, model gap) |
| [RFC8200-FRAG-5](../../standard/rfc8200/catalog.md#rfc8200-frag-5) | selected | [source-fragmentation-and-reassembly](../../protocol/ipv6/checks/fragmentation.md#source-fragmentation-and-reassembly), [fragment-payload-length](../../protocol/ipv6/checks/fragmentation.md#fragment-payload-length) | Rfc8200SourceFragmentation.test, Rfc8200FragmentPayloadLength.test | PASS (structure); **FAIL** (payload length, model gap) |
| [RFC8200-FRAG-6](../../standard/rfc8200/catalog.md#rfc8200-frag-6) | superseded by RFC8504-NR-3 (sender half) | [source-fragmentation-and-reassembly](../../protocol/ipv6/checks/fragmentation.md#source-fragmentation-and-reassembly) | Rfc8200SourceFragmentation.test | PASS |
| [RFC8200-REASM-1](../../standard/rfc8200/catalog.md#rfc8200-reasm-1) | selected | [source-fragmentation-and-reassembly](../../protocol/ipv6/checks/fragmentation.md#source-fragmentation-and-reassembly) | Rfc8200SourceFragmentation.test | PASS |
| [RFC8200-REASM-2](../../standard/rfc8200/catalog.md#rfc8200-reasm-2) | selected | [source-fragmentation-and-reassembly](../../protocol/ipv6/checks/fragmentation.md#source-fragmentation-and-reassembly) | Rfc8200SourceFragmentation.test | PASS |
| [RFC8200-REASM-3](../../standard/rfc8200/catalog.md#rfc8200-reasm-3) | later (level 4: a timer) | — | — | — |
| [RFC8200-REASM-4](../../standard/rfc8200/catalog.md#rfc8200-reasm-4) | selected | [short-fragment](../../protocol/ipv6/checks/fragmentation.md#short-fragment) | Rfc8200ShortFragment.test | PASS |
| [RFC8200-REASM-5](../../standard/rfc8200/catalog.md#rfc8200-reasm-5) | superseded by RFC8504-NR-3 (receiver half) | [overlapping-fragments](../../protocol/ipv6/checks/fragmentation.md#overlapping-fragments) | Rfc8200OverlappingFragments.test | **FAIL** (model gap: merged, then an assertion) |
| [RFC8200-REASM-6](../../standard/rfc8200/catalog.md#rfc8200-reasm-6) | superseded by RFC8504-NR-4 (receiver half) | [atomic-fragment](../../protocol/ipv6/checks/fragmentation.md#atomic-fragment) | Rfc8200AtomicFragment.test | **FAIL** (model gap: a C++ assertion) |
| [RFC8200-REASM-7](../../standard/rfc8200/catalog.md#rfc8200-reasm-7) | later (a crafted first fragment without its upper-layer header); superseded by RFC8504-NR-8 | — | — | — |
| [RFC8200-REASM-8](../../standard/rfc8200/catalog.md#rfc8200-reasm-8) | selected | [oversized-fragment-offset](../../protocol/ipv6/checks/fragmentation.md#oversized-fragment-offset) | Rfc8200OversizedFragmentOffset.test | PASS |
| [RFC8200-MTU-1](../../standard/rfc8200/catalog.md#rfc8200-mtu-1) | constraint (every scenario obeys it) | — | — | — |
| [RFC8200-MTU-2](../../standard/rfc8200/catalog.md#rfc8200-mtu-2) | selected | [link-mtu-packet](../../protocol/ipv6/checks/packet-size.md#link-mtu-packet) | Rfc8200LinkMtuPacket.test | PASS |
| [RFC8200-MTU-3](../../standard/rfc8200/catalog.md#rfc8200-mtu-3) | later (level 4: RFC 8201) | — | — | — |
| [RFC8200-MTU-4](../../standard/rfc8200/catalog.md#rfc8200-mtu-4) | selected | [source-fragmentation-and-reassembly](../../protocol/ipv6/checks/fragmentation.md#source-fragmentation-and-reassembly) | Rfc8200SourceFragmentation.test | PASS |
| [RFC8200-MTU-5](../../standard/rfc8200/catalog.md#rfc8200-mtu-5) | candidate (a should for the upper layer) | — | — | — |
| [RFC8200-CKSUM-1](../../standard/rfc8200/catalog.md#rfc8200-cksum-1) | selected | [datagram-delivery](../../protocol/ipv6/checks/delivery.md#datagram-delivery), [zero-udp-checksum](../../protocol/ipv6/checks/input-validation.md#zero-udp-checksum) | Rfc8200DatagramDelivery.test, Rfc8200ZeroUdpChecksum.test | PASS (computed); PASS (zero discarded) |
| [RFC8200-CKSUM-2](../../standard/rfc8200/catalog.md#rfc8200-cksum-2) | later (unit test: encoding) | — | — | — |
| [RFC4443-PTB-1](../../standard/rfc4443/catalog.md#rfc4443-ptb-1) | selected | [packet-too-big](../../protocol/ipv6/checks/packet-too-big.md#packet-too-big) | Rfc4443PacketTooBig.test | PASS |
| [RFC4443-PTB-2](../../standard/rfc4443/catalog.md#rfc4443-ptb-2) | selected | [packet-too-big-the-mtu-field](../../protocol/ipv6/checks/packet-too-big.md#packet-too-big-the-mtu-field) | Rfc4443PacketTooBigMtu.test | **FAIL** (model gap: MTU 0) |
| [RFC4443-TE-1](../../standard/rfc4443/catalog.md#rfc4443-te-1) | selected | [hop-limit-expiry](../../protocol/ipv6/checks/hop-limit.md#hop-limit-expiry) | Rfc8200HopLimitExpiry.test | PASS |
| [RFC4443-DU-4](../../standard/rfc4443/catalog.md#rfc4443-du-4) | selected | [host-error-report](../../protocol/ipv6/checks/error-report.md#host-error-report) | Rfc4443HostErrorReport.test | PASS (a should) |
| [RFC4443-ERR-1](../../standard/rfc4443/catalog.md#rfc4443-err-1) | selected | [host-error-report](../../protocol/ipv6/checks/error-report.md#host-error-report) | Rfc4443HostErrorReport.test | PASS |
| [RFC4443-ERR-2](../../standard/rfc4443/catalog.md#rfc4443-err-2) | selected | [hop-limit-expiry](../../protocol/ipv6/checks/hop-limit.md#hop-limit-expiry), [packet-too-big](../../protocol/ipv6/checks/packet-too-big.md#packet-too-big), [host-error-report](../../protocol/ipv6/checks/error-report.md#host-error-report) | Rfc8200HopLimitExpiry.test, Rfc4443PacketTooBig.test, Rfc4443HostErrorReport.test | PASS |
| [RFC4443-SRC-1](../../standard/rfc4443/catalog.md#rfc4443-src-1) | selected | [host-error-report](../../protocol/ipv6/checks/error-report.md#host-error-report) | Rfc4443HostErrorReport.test | PASS |
| [RFC4443-SRC-2](../../standard/rfc4443/catalog.md#rfc4443-src-2) | selected | [report-source-address](../../protocol/ipv6/checks/error-report.md#report-source-address) | Rfc4443ReportSourceAddress.test | PASS |
| [RFC4443-MPR-1](../../standard/rfc4443/catalog.md#rfc4443-mpr-1) | later (module test: a handoff); its wire half is under MPR-4 | — | — | — |
| [RFC4443-MPR-2](../../standard/rfc4443/catalog.md#rfc4443-mpr-2) | selected | [unknown-icmpv6-informational-type](../../protocol/ipv6/checks/input-validation.md#unknown-icmpv6-informational-type) | Rfc4443UnknownInformationalType.test | **FAIL** (model gap: a runtime error) |
| [RFC4443-MPR-3](../../standard/rfc4443/catalog.md#rfc4443-mpr-3) | selected | [error-for-an-unknown-protocol](../../protocol/ipv6/checks/error-report.md#error-for-an-unknown-protocol) | Rfc4443ErrorForUnknownProtocol.test | **FAIL** (model gap: a runtime error at the source) |
| [RFC4443-MPR-4](../../standard/rfc4443/catalog.md#rfc4443-mpr-4) | selected | [no-error-about-an-error](../../protocol/ipv6/checks/error-report.md#no-error-about-an-error), [unknown-icmpv6-error-type](../../protocol/ipv6/checks/input-validation.md#unknown-icmpv6-error-type) | Rfc4443NoErrorAboutError.test, Rfc4443UnknownErrorType.test | PASS |
| [RFC4443-MPR-5](../../standard/rfc4443/catalog.md#rfc4443-mpr-5) | later (needs a redirect in flight) | — | — | — |
| [RFC4443-MPR-6](../../standard/rfc4443/catalog.md#rfc4443-mpr-6) | selected | [no-error-for-a-multicast-destination](../../protocol/ipv6/checks/error-report.md#no-error-for-a-multicast-destination) | Rfc4443NoErrorForMulticast.test | PASS |
| [RFC4443-MPR-7](../../standard/rfc4443/catalog.md#rfc4443-mpr-7) | selected | [no-error-for-a-link-layer-multicast](../../protocol/ipv6/checks/error-report.md#no-error-for-a-link-layer-multicast) | Rfc4443NoErrorForLinkMulticast.test | **FAIL** (model gap) |
| [RFC4443-MPR-8](../../standard/rfc4443/catalog.md#rfc4443-mpr-8) | selected | [no-error-for-a-link-layer-broadcast](../../protocol/ipv6/checks/error-report.md#no-error-for-a-link-layer-broadcast) | Rfc4443NoErrorForLinkBroadcast.test | **FAIL** (model gap) |
| [RFC4443-MPR-9](../../standard/rfc4443/catalog.md#rfc4443-mpr-9) | selected | [no-error-for-an-unspecified-source](../../protocol/ipv6/checks/error-report.md#no-error-for-an-unspecified-source) | Rfc4443NoErrorForUnspecifiedSource.test | PASS |
| [RFC4443-MPR-10](../../standard/rfc4443/catalog.md#rfc4443-mpr-10) | later (level 4: a rate) | — | — | — |
| [RFC4443-DU-C](../../standard/rfc4443/catalog.md#rfc4443-du-c) | later (level 4: congestion) | — | — | — |
| [RFC4443-DU-P](../../standard/rfc4443/catalog.md#rfc4443-du-p) | later (a point-to-point link) | — | — | — |
| [RFC4443-UL-1](../../standard/rfc4443/catalog.md#rfc4443-ul-1) | later (module test: a handoff) | — | — | — |
| [RFC4443-UL-2](../../standard/rfc4443/catalog.md#rfc4443-ul-2) | later (module test: a handoff) | — | — | — |
| [RFC4443-UL-3](../../standard/rfc4443/catalog.md#rfc4443-ul-3) | later (module test: a handoff) | — | — | — |
| [RFC8504-NR-1](../../standard/rfc8504/catalog.md#rfc8504-nr-1) | covered (implementation: the RFC 8200 rows) | — (every RFC 8200 check) | — | as the RFC 8200 rows: PASS with the gaps of HDR-2, FRAG-4, FRAG-5 |
| [RFC8504-NR-2](../../standard/rfc8504/catalog.md#rfc8504-nr-2) | covered | [source-fragmentation-and-reassembly](../../protocol/ipv6/checks/fragmentation.md#source-fragmentation-and-reassembly) | Rfc8200SourceFragmentation.test | PASS |
| [RFC8504-NR-3](../../standard/rfc8504/catalog.md#rfc8504-nr-3) | selected (both halves) | [source-fragmentation-and-reassembly](../../protocol/ipv6/checks/fragmentation.md#source-fragmentation-and-reassembly), [overlapping-fragments](../../protocol/ipv6/checks/fragmentation.md#overlapping-fragments) | Rfc8200SourceFragmentation.test, Rfc8200OverlappingFragments.test | PASS (the sender creates no overlap); **FAIL** (the receiver merges one, model gap) |
| [RFC8504-NR-4](../../standard/rfc8504/catalog.md#rfc8504-nr-4) | selected (both halves) | [source-fragmentation-and-reassembly](../../protocol/ipv6/checks/fragmentation.md#source-fragmentation-and-reassembly), [atomic-fragment](../../protocol/ipv6/checks/fragmentation.md#atomic-fragment) | Rfc8200SourceFragmentation.test, Rfc8200AtomicFragment.test | PASS (the sender emits none); **FAIL** (the receiver stops on one, model gap) |
| [RFC8504-NR-5](../../standard/rfc8504/catalog.md#rfc8504-nr-5) | covered | [fragment-identification](../../protocol/ipv6/checks/fragmentation.md#fragment-identification) | Rfc8200FragmentIdentification.test | **declined** (a should: the identifications are a counter) |
| [RFC8504-NR-6](../../standard/rfc8504/catalog.md#rfc8504-nr-6) | selected | [unrecognized-next-header](../../protocol/ipv6/checks/input-validation.md#unrecognized-next-header), [unassigned-next-header](../../protocol/ipv6/checks/input-validation.md#unassigned-next-header) | Rfc8200UnrecognizedNextHeader.test, Rfc8200UnassignedNextHeader.test | PASS |
| [RFC8504-NR-7](../../standard/rfc8504/catalog.md#rfc8504-nr-7) | later (level 5: the other extension headers; implementation) | — | — | — |
| [RFC8504-NR-8](../../standard/rfc8504/catalog.md#rfc8504-nr-8) | selected (sender half); later (receiver half, see REASM-7) | [source-fragmentation-and-reassembly](../../protocol/ipv6/checks/fragmentation.md#source-fragmentation-and-reassembly) | Rfc8200SourceFragmentation.test | PASS (the UDP header is in the first fragment) |
| [RFC8504-NR-9](../../standard/rfc8504/catalog.md#rfc8504-nr-9) | later (level 4: RFC 8201; a should) | — | — | — |
| [RFC8504-NR-10](../../standard/rfc8504/catalog.md#rfc8504-nr-10) | covered (implementation: the fragmentation and reassembly rows) | — | — | as those rows: 2 receiver gaps |
| [RFC8504-NR-11](../../standard/rfc8504/catalog.md#rfc8504-nr-11) | covered | [packet-too-big](../../protocol/ipv6/checks/packet-too-big.md#packet-too-big) | Rfc4443PacketTooBig.test | PASS (the report reached the source) |
| [RFC8504-NR-12](../../standard/rfc8504/catalog.md#rfc8504-nr-12) | covered (implementation: the RFC 4443 rows) | — | — | as the RFC 4443 rows |

67 entries: 32 of RFC 8200, 23 of RFC 4443, 12 of RFC 8504. 50 have a verdict from a check
that ran: 34 PASS, 7 FAIL as a model gap, 5 with one PASS and one FAIL (a statement with a
sender half and a receiver half, or with a whole-packet case and a fragment case), 1
declined should, and 3 `implementation` statements whose verdict is the verdict of the rows
they point to. 17 have none: 4 need a module test (the handoffs of a received report), 5
wait for level 4 (the reassembly timer, path MTU discovery, the rate limit, congestion), 2
for level 5 (the other extension headers), 3 for a scenario the mockup lacks (a redirect, a
point-to-point link, a crafted first fragment), 1 for a unit test, 1 is a scenario
constraint, and 1 is a should that no check selected.

## Feature support

The rule of step 7: `supported` when every core check ran and passed, `partial` when a core
check passed and another core check failed as a model gap or did not run, `not supported`
when every core check that ran failed as a model gap, `untested` when no core check ran. A
statement with two halves counts by its halves.

| Feature | Core checks and their state | Support |
| --- | --- | --- |
| [IPV6-F-HEADER](../../protocol/ipv6/features.md#ipv6-f-header) | RFC8200-HDR-1 PASS, HDR-3 PASS; HDR-2 PASS on a whole packet and FAIL on a fragment | **partial** |
| [IPV6-F-DELIVERY](../../protocol/ipv6/features.md#ipv6-f-delivery) | RFC8200-HDR-4 PASS, DLV-1 PASS | **supported** |
| [IPV6-F-HOP-LIMIT](../../protocol/ipv6/features.md#ipv6-f-hop-limit) | RFC8200-HL-1 PASS, HL-2 PASS | **supported** |
| [IPV6-F-SOURCE-FRAGMENTATION](../../protocol/ipv6/features.md#ipv6-f-source-fragmentation) | RFC8200-FRAG-2, FRAG-3 PASS; FRAG-4 and FRAG-5 PASS in structure and FAIL in payload length; RFC8504-NR-3, NR-4, NR-8 sender halves PASS | **partial** |
| [IPV6-F-REASSEMBLY](../../protocol/ipv6/features.md#ipv6-f-reassembly) | RFC8200-REASM-1, REASM-2, MTU-4, REASM-4, REASM-8 PASS; RFC8504-NR-3 receiver half FAIL | **partial** |
| [IPV6-F-PACKET-TOO-BIG](../../protocol/ipv6/features.md#ipv6-f-packet-too-big) | RFC8200-FRAG-1 PASS, RFC4443-PTB-1 PASS; RFC4443-PTB-2 FAIL | **partial** |
| [IPV6-F-PACKET-SIZE](../../protocol/ipv6/features.md#ipv6-f-packet-size) | RFC8200-MTU-2 PASS | **supported** |
| [IPV6-F-ERROR-REPORT](../../protocol/ipv6/features.md#ipv6-f-error-report) | RFC4443-TE-1, PTB-1, ERR-2, SRC-1 all PASS | **supported** |
| [IPV6-F-UPPER-LAYER-CHECKSUM](../../protocol/ipv6/features.md#ipv6-f-upper-layer-checksum) | RFC8200-CKSUM-1 PASS (both halves) | **supported** |
| [IPV6-F-INPUT-VALIDATION](../../protocol/ipv6/features.md#ipv6-f-input-validation) | RFC8200-CKSUM-1 (discard) PASS, RFC8504-NR-6 PASS; RFC4443-MPR-2 FAIL | **partial** |
| [IPV6-F-ERROR-SUPPRESSION](../../protocol/ipv6/features.md#ipv6-f-error-suppression) | RFC4443-MPR-4, MPR-6, MPR-9 PASS; MPR-7, MPR-8 FAIL | **partial** |
| [IPV6-F-ERROR-DELIVERY](../../protocol/ipv6/features.md#ipv6-f-error-delivery) | RFC4443-MPR-1, UL-1, UL-2, UL-3: none ran (internal; module tests). The supporting MPR-3 FAILED: a report about an unknown protocol stops the node | **untested**, with a gap on a supporting statement |

Five features supported, six partial, one untested. Since pass 1 the checksum feature moved
from `supported` as far as one half to `supported` in both halves, and the level 3 toolset
found four more gaps: the receiver's handling of overlapping and of atomic fragments, the
unknown informational ICMPv6 type, the report about an unknown protocol at its source, and
the two link-layer suppression cases. The pass 1 bounds on these values stand.

## Achieved level

**Level 3, partial.** Target: level 3, from
[`standards.md`](../../protocol/ipv6/standards.md#target-level).

The exit criterion of level 3 has two halves. The first holds; the second does not.

**Hold** — the catalogs hold every mandatory statement of the in-scope documents, including
each `MUST NOT`: RFC 8200 §3, §4 (the processing rules and the fragment header), §4.5, §5
and §8.1; RFC 4443 §2 and §3 in full; RFC 8504 §5.1, §5.2, §5.7 and §5.8. The parts left out
are named at the end of each catalog and in the standards map, with the reason.

**Run** — each mandatory statement has a check that ran: **not yet.** 50 of 67 statements
have a verdict. 10 mandatory statements have none, and they fall into four kinds:

| Kind | Statements | What unlocks them |
| --- | --- | --- |
| a handoff inside the node | RFC4443-MPR-1 (its handoff half), UL-1, UL-2, UL-3 | module tests |
| a timer, a rate, or congestion | RFC8200-REASM-3, RFC4443-MPR-10, RFC4443-DU-C | level 4 |
| a scenario the mockup lacks | RFC4443-MPR-5 (a redirect), RFC4443-DU-P (a point-to-point link) | a neighbor discovery pass; a second link type |
| the other extension headers | RFC8200-EXT-2, RFC8504-NR-7 | level 5 |

| Level | State | Evidence |
| --- | --- | --- |
| 1, Survey | **reached** | The standards map pins the in-scope set; [`conformance.md`](conformance.md) maps every model claim onto it and names the obsolete citations. |
| 2, Core | **reached** | The inventory of pass 1, and every level 2 statement with a verdict on this tree. |
| 3, Edge | **partial** | Three catalogs complete for the in-scope sections; 18 checks with the relay and the one-link mockup; 5 new model gaps, 3 of them stops of the simulation; 10 mandatory statements without a check, listed above. |
| 4, Dynamics | not started | the reassembly timer, the rate limit, congestion, path MTU discovery (blocked by the MTU field gap) |
| 5, Complete | not started | the other extension headers and their order, the flow label, the traffic class, jumbograms, the ICMPv6 query messages |

Per feature, level 3 is reached when every mandatory statement mapped to the feature, core
or supporting, has a verdict:

| Feature | Level 3 | What blocks it |
| --- | --- | --- |
| IPV6-F-HEADER | reached | — |
| IPV6-F-DELIVERY | reached | — (RFC8504-NR-1 is covered by the RFC 8200 rows) |
| IPV6-F-HOP-LIMIT | reached | — |
| IPV6-F-SOURCE-FRAGMENTATION | reached | — (RFC8504-NR-5 is a should) |
| IPV6-F-REASSEMBLY | partial | RFC8200-REASM-3 (level 4); RFC8504-NR-8 receiver half and REASM-7 are shoulds |
| IPV6-F-PACKET-TOO-BIG | reached | — |
| IPV6-F-PACKET-SIZE | reached | — (RFC8200-MTU-3 and RFC8504-NR-9 are shoulds) |
| IPV6-F-ERROR-REPORT | partial | RFC4443-MPR-10 (level 4), DU-C (level 4), DU-P (a point-to-point link) |
| IPV6-F-UPPER-LAYER-CHECKSUM | reached | — |
| IPV6-F-INPUT-VALIDATION | partial | RFC8200-EXT-2 and RFC8504-NR-7 (level 5) |
| IPV6-F-ERROR-SUPPRESSION | partial | RFC4443-MPR-5 (a redirect) |
| IPV6-F-ERROR-DELIVERY | not reached | all four core statements are internal |

Seven of twelve features reach level 3. The level and the support value are different
axes: IPV6-F-PACKET-TOO-BIG reaches level 3 with a `partial` support value, because its
MTU field check ran and failed, which is exactly what the level exists to find.

## Pass log

| Pass | Date | Level | Scope | Result |
| --- | --- | --- | --- | --- |
| 1 | 2026-09-09 | **2, reached** | RFC 8200 + RFC 4443 error signals: 34 catalog entries, 9 features, 9 checks, 9 tests | 7 PASS, 2 FAIL (expected) as model gaps; 6 features supported, 3 partial |
| 2 | 2026-09-09 | **3, partial** | Level 3 pass: RFC 8504 enters the in-scope set and the two level 2 documents enter in full (33 new catalog entries, 4 override cross references), 12 features (3 new), 27 checks (18 new, split into one file per feature), 27 tests (18 new); the relay and the one-link mockups. | 19 PASS, 8 FAIL (expected) as model gaps, 4 of them stops of the simulation; 5 features supported, 6 partial, 1 untested; 10 mandatory statements without a check; see [`results.md`](results.md) and [`conformance.md`](conformance.md) |

## Out of scope

The catalogs record what this pass left out of the standards: the extension headers other
than the fragment header and their options, the flow label, the traffic class, jumbograms,
the routing header responses, the ICMPv6 query messages, and the neighbor discovery,
addressing and configuration sections of RFC 8504. Each becomes level 4 or level 5 work,
or the work of another protocol folder.
