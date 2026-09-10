# IPv4 — coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rfc791/catalog.md](../../standard/rfc791/catalog.md), [rfc792/catalog.md](../../standard/rfc792/catalog.md), [rfc1122/catalog.md](../../standard/rfc1122/catalog.md), [rfc6864/catalog.md](../../standard/rfc6864/catalog.md), [features.md](../../protocol/ipv4/features.md), [results.md](results.md)

The single place that holds the changing state of the IPv4 workflow. Every other artifact
of this protocol states what a standard says and never changes again unless the standard
changes. This one changes on every pass.

**No step edits an artifact of an earlier step. Steps 5, 6 and 7 record their outcome here.**

The ledger spans the documents of the in-scope set, so one table mixes RFC 791, RFC 792,
RFC 1122 and RFC 6864 identifiers: a check may establish statements of two documents at
once, and a later document may govern an earlier statement.

State of the ledger, from this run:

- Date: 2026-09-10 15:18 +0200
- INET: branch `master`, commit `0868c36c88`, tree clean
- OMNeT++: 6.4.0
- Build: debug, built from this commit
- Compiler: Ubuntu clang version 23.0.0
- Platform: Ubuntu 26.04.1 LTS, Linux 7.0.0-31-generic x86_64
- Command: `inet_run_protocol_tests -p inet -w ipv4`
- Target level: 3

## Statement coverage

`Status` is the state of the workflow, not of the standard: `selected` (a check targets it),
`covered` (a selected check establishes it as a side effect), `candidate` (practical, not yet
chosen), `later` (needs a toolset or a category beyond this pass), `superseded by X` (a later
in-scope document governs; the check targets X, and the row keeps the verdict of that
check). A verdict of `declined` marks a `should` or `may` that the model does not follow,
which is not a failure.

| Catalog ID | Status | Check section | Test file | Verdict |
| --- | --- | --- | --- | --- |
| [RFC791-TTL-1](../../standard/rfc791/catalog.md#rfc791-ttl-1) | selected | [ttl-decrement](../../protocol/ipv4/checks/ttl.md#ttl-decrement) | Rfc791TtlDecrement.test | PASS |
| [RFC791-TTL-2](../../standard/rfc791/catalog.md#rfc791-ttl-2) | selected | [ttl-expiry](../../protocol/ipv4/checks/ttl.md#ttl-expiry) | Rfc791TtlExpiry.test | PASS |
| [RFC791-TTL-3](../../standard/rfc791/catalog.md#rfc791-ttl-3) | covered | [ttl-decrement](../../protocol/ipv4/checks/ttl.md#ttl-decrement) | Rfc791TtlDecrement.test | PASS |
| [RFC791-FRAG-1](../../standard/rfc791/catalog.md#rfc791-frag-1) | selected | [fragment-and-reassembly](../../protocol/ipv4/checks/fragmentation.md#fragment-and-reassembly) | Rfc791FragmentReassembly.test | PASS |
| [RFC791-FRAG-2](../../standard/rfc791/catalog.md#rfc791-frag-2) | selected | [fragment-and-reassembly](../../protocol/ipv4/checks/fragmentation.md#fragment-and-reassembly) | Rfc791FragmentReassembly.test | PASS |
| [RFC791-FRAG-3](../../standard/rfc791/catalog.md#rfc791-frag-3) | selected | [fragment-and-reassembly](../../protocol/ipv4/checks/fragmentation.md#fragment-and-reassembly) | Rfc791FragmentReassembly.test | PASS |
| [RFC791-FRAG-4](../../standard/rfc791/catalog.md#rfc791-frag-4) | selected | [fragment-and-reassembly](../../protocol/ipv4/checks/fragmentation.md#fragment-and-reassembly) | Rfc791FragmentReassembly.test | PASS |
| [RFC791-FRAG-5](../../standard/rfc791/catalog.md#rfc791-frag-5) | superseded by RFC6864-ID-6 | [dont-fragment](../../protocol/ipv4/checks/fragmentation.md#dont-fragment) | Rfc791DontFragment.test | PASS (the same check establishes both) |
| [RFC791-FRAG-6](../../standard/rfc791/catalog.md#rfc791-frag-6) | selected | [minimum-sizes](../../protocol/ipv4/checks/fragmentation.md#minimum-sizes) | Rfc791MinimumSizes.test | PASS |
| [RFC791-REASM-1](../../standard/rfc791/catalog.md#rfc791-reasm-1) | superseded by RFC1122-REASM-1 | [fragment-and-reassembly](../../protocol/ipv4/checks/fragmentation.md#fragment-and-reassembly), [same-identification-different-protocol](../../protocol/ipv4/checks/fragmentation.md#same-identification-different-protocol) | Rfc791FragmentReassembly.test, Rfc791SameIdDifferentProtocol.test | PASS (procedure); **FAIL** (four-field key, model gap) |
| [RFC791-REASM-2](../../standard/rfc791/catalog.md#rfc791-reasm-2) | superseded by RFC1122-REASM-2 | [minimum-sizes](../../protocol/ipv4/checks/fragmentation.md#minimum-sizes) | Rfc791MinimumSizes.test | PASS |
| [RFC791-REASM-3](../../standard/rfc791/catalog.md#rfc791-reasm-3) | selected | [interleaved-reassembly](../../protocol/ipv4/checks/fragmentation.md#interleaved-reassembly), [same-identification-different-protocol](../../protocol/ipv4/checks/fragmentation.md#same-identification-different-protocol) | Rfc791InterleavedReassembly.test, Rfc791SameIdDifferentProtocol.test | PASS (interleaved); **FAIL** (same identification, model gap) |
| [RFC791-CKSUM-1](../../standard/rfc791/catalog.md#rfc791-cksum-1) | selected | [ttl-decrement](../../protocol/ipv4/checks/ttl.md#ttl-decrement) | Rfc791TtlDecrement.test | PASS |
| [RFC791-CKSUM-2](../../standard/rfc791/catalog.md#rfc791-cksum-2) | superseded by RFC1122-CKSUM-1 | [checksum-discard](../../protocol/ipv4/checks/checksum.md#checksum-discard) | Rfc1122ChecksumDiscard.test | **FAIL** (model gap) |
| [RFC791-ID-1](../../standard/rfc791/catalog.md#rfc791-id-1) | superseded by RFC6864-ID-5 | [identification](../../protocol/ipv4/checks/identification.md#identification) | Rfc791Identification.test | PASS |
| [RFC791-HDR-1](../../standard/rfc791/catalog.md#rfc791-hdr-1) | selected | [datagram-delivery](../../protocol/ipv4/checks/delivery.md#datagram-delivery) | Rfc791DatagramDelivery.test | PASS |
| [RFC791-HDR-2](../../standard/rfc791/catalog.md#rfc791-hdr-2) | selected | [datagram-delivery](../../protocol/ipv4/checks/delivery.md#datagram-delivery) | Rfc791DatagramDelivery.test | PASS |
| [RFC791-HDR-3](../../standard/rfc791/catalog.md#rfc791-hdr-3) | selected | [datagram-delivery](../../protocol/ipv4/checks/delivery.md#datagram-delivery) | Rfc791DatagramDelivery.test | PASS |
| [RFC791-PROTO-1](../../standard/rfc791/catalog.md#rfc791-proto-1) | selected | [datagram-delivery](../../protocol/ipv4/checks/delivery.md#datagram-delivery) | Rfc791DatagramDelivery.test | PASS |
| [RFC791-FWD-1](../../standard/rfc791/catalog.md#rfc791-fwd-1) | selected | [datagram-delivery](../../protocol/ipv4/checks/delivery.md#datagram-delivery) | Rfc791DatagramDelivery.test | PASS |
| [RFC791-DLV-1](../../standard/rfc791/catalog.md#rfc791-dlv-1) | selected | [datagram-delivery](../../protocol/ipv4/checks/delivery.md#datagram-delivery) | Rfc791DatagramDelivery.test | PASS |
| [RFC792-TE-1](../../standard/rfc792/catalog.md#rfc792-te-1) | selected | [ttl-expiry](../../protocol/ipv4/checks/ttl.md#ttl-expiry) | Rfc791TtlExpiry.test | PASS |
| [RFC792-DU-4](../../standard/rfc792/catalog.md#rfc792-du-4) | selected | [dont-fragment](../../protocol/ipv4/checks/fragmentation.md#dont-fragment) | Rfc791DontFragment.test | PASS |
| [RFC1122-VER-1](../../standard/rfc1122/catalog.md#rfc1122-ver-1) | selected | [version-discard](../../protocol/ipv4/checks/input-validation.md#version-discard) | Rfc1122VersionDiscard.test | **FAIL** (model gap) |
| [RFC1122-CKSUM-1](../../standard/rfc1122/catalog.md#rfc1122-cksum-1) | selected | [checksum-discard](../../protocol/ipv4/checks/checksum.md#checksum-discard) | Rfc1122ChecksumDiscard.test | **FAIL** (model gap) |
| [RFC1122-ADDR-1](../../standard/rfc1122/catalog.md#rfc1122-addr-1) | covered | [datagram-delivery](../../protocol/ipv4/checks/delivery.md#datagram-delivery) | Rfc791DatagramDelivery.test | PASS |
| [RFC1122-ADDR-2](../../standard/rfc1122/catalog.md#rfc1122-addr-2) | selected | [foreign-destination](../../protocol/ipv4/checks/input-validation.md#foreign-destination) | Rfc1122ForeignDestination.test | PASS |
| [RFC1122-ADDR-3](../../standard/rfc1122/catalog.md#rfc1122-addr-3) | selected | [invalid-source-address](../../protocol/ipv4/checks/input-validation.md#invalid-source-address) | Rfc1122InvalidSourceAddress.test | **FAIL** (model gap) |
| [RFC1122-ADDR-4](../../standard/rfc1122/catalog.md#rfc1122-addr-4) | candidate (needs a standing negative rule) | — | — | — |
| [RFC1122-ID-1](../../standard/rfc1122/catalog.md#rfc1122-id-1) | superseded by RFC6864-ID-4 | — | — | — |
| [RFC1122-TTL-1](../../standard/rfc1122/catalog.md#rfc1122-ttl-1) | candidate (see the results: the model asserts) | — | — | — |
| [RFC1122-TTL-2](../../standard/rfc1122/catalog.md#rfc1122-ttl-2) | selected | [ttl-1-at-the-destination](../../protocol/ipv4/checks/ttl.md#ttl-1-at-the-destination) | Rfc1122TtlOneAtDestination.test | PASS |
| [RFC1122-TTL-3](../../standard/rfc1122/catalog.md#rfc1122-ttl-3) | covered | [ttl-1-at-the-destination](../../protocol/ipv4/checks/ttl.md#ttl-1-at-the-destination) | Rfc1122TtlOneAtDestination.test | PASS |
| [RFC1122-TTL-4](../../standard/rfc1122/catalog.md#rfc1122-ttl-4) | covered | [no-error-about-an-error](../../protocol/ipv4/checks/error-report.md#no-error-about-an-error) | Rfc1122NoErrorAboutError.test | PASS (the configured default appeared on the wire) |
| [RFC1122-REASM-1](../../standard/rfc1122/catalog.md#rfc1122-reasm-1) | selected | [fragment-and-reassembly](../../protocol/ipv4/checks/fragmentation.md#fragment-and-reassembly), [interleaved-reassembly](../../protocol/ipv4/checks/fragmentation.md#interleaved-reassembly) | Rfc791FragmentReassembly.test, Rfc791InterleavedReassembly.test | PASS |
| [RFC1122-REASM-2](../../standard/rfc1122/catalog.md#rfc1122-reasm-2) | selected | [minimum-sizes](../../protocol/ipv4/checks/fragmentation.md#minimum-sizes) | Rfc791MinimumSizes.test | PASS (the must; the two shoulds are untested) |
| [RFC1122-REASM-3](../../standard/rfc1122/catalog.md#rfc1122-reasm-3) | later (module test: an interface) | — | — | — |
| [RFC1122-REASM-4](../../standard/rfc1122/catalog.md#rfc1122-reasm-4) | later (level 4) | — | — | — |
| [RFC1122-REASM-5](../../standard/rfc1122/catalog.md#rfc1122-reasm-5) | later (level 4) | — | — | — |
| [RFC1122-FRAG-1](../../standard/rfc1122/catalog.md#rfc1122-frag-1) | candidate (a may) | — | — | — |
| [RFC1122-FRAG-2](../../standard/rfc1122/catalog.md#rfc1122-frag-2) | later (module test: an interface) | — | — | — |
| [RFC1122-FRAG-3](../../standard/rfc1122/catalog.md#rfc1122-frag-3) | candidate (conditional on the absence of FRAG-1) | — | — | — |
| [RFC1122-FRAG-4](../../standard/rfc1122/catalog.md#rfc1122-frag-4) | covered | [fragment-and-reassembly](../../protocol/ipv4/checks/fragmentation.md#fragment-and-reassembly) | Rfc791FragmentReassembly.test | **declined** (a should: 1028 octets sent whole off-net) |
| [RFC1122-FRAG-5](../../standard/rfc1122/catalog.md#rfc1122-frag-5) | covered | [fragment-and-reassembly](../../protocol/ipv4/checks/fragmentation.md#fragment-and-reassembly) | Rfc791FragmentReassembly.test | PASS (on a gateway interface of the same module type) |
| [RFC1122-ICMP-1](../../standard/rfc1122/catalog.md#rfc1122-icmp-1) | selected | [unknown-icmp-type](../../protocol/ipv4/checks/input-validation.md#unknown-icmp-type) | Rfc1122UnknownIcmpType.test | **FAIL** (model gap: a runtime error) |
| [RFC1122-ICMP-2](../../standard/rfc1122/catalog.md#rfc1122-icmp-2) | selected | [host-error-report](../../protocol/ipv4/checks/error-report.md#host-error-report) | Rfc1122HostErrorReport.test | PASS |
| [RFC1122-ICMP-3](../../standard/rfc1122/catalog.md#rfc1122-icmp-3) | later (module test: a handoff) | — | — | — |
| [RFC1122-ICMP-4](../../standard/rfc1122/catalog.md#rfc1122-icmp-4) | selected | [host-error-report](../../protocol/ipv4/checks/error-report.md#host-error-report) | Rfc1122HostErrorReport.test | PASS (a should) |
| [RFC1122-ICMP-5](../../standard/rfc1122/catalog.md#rfc1122-icmp-5) | selected | [no-error-about-an-error](../../protocol/ipv4/checks/error-report.md#no-error-about-an-error) | Rfc1122NoErrorAboutError.test | PASS |
| [RFC1122-ICMP-6](../../standard/rfc1122/catalog.md#rfc1122-icmp-6) | selected | [no-error-for-a-broadcast](../../protocol/ipv4/checks/error-report.md#no-error-for-a-broadcast) | Rfc1122NoErrorForBroadcast.test | PASS |
| [RFC1122-ICMP-7](../../standard/rfc1122/catalog.md#rfc1122-icmp-7) | selected | [no-error-for-a-link-layer-broadcast](../../protocol/ipv4/checks/error-report.md#no-error-for-a-link-layer-broadcast) | Rfc1122NoErrorForLinkBroadcast.test | **FAIL** (model gap) |
| [RFC1122-ICMP-8](../../standard/rfc1122/catalog.md#rfc1122-icmp-8) | selected | [no-error-for-a-non-initial-fragment](../../protocol/ipv4/checks/error-report.md#no-error-for-a-non-initial-fragment) | Rfc1122NoErrorForNonInitialFragment.test | PASS |
| [RFC1122-ICMP-9](../../standard/rfc1122/catalog.md#rfc1122-icmp-9) | selected | [no-error-for-an-invalid-source](../../protocol/ipv4/checks/error-report.md#no-error-for-an-invalid-source) | Rfc1122NoErrorForInvalidSource.test | PASS |
| [RFC1122-DU-1](../../standard/rfc1122/catalog.md#rfc1122-du-1) | selected | [host-error-report](../../protocol/ipv4/checks/error-report.md#host-error-report) | Rfc1122HostErrorReport.test | PASS (a should) |
| [RFC1122-DU-2](../../standard/rfc1122/catalog.md#rfc1122-du-2) | later (module test: a handoff) | — | — | — |
| [RFC1122-DU-3](../../standard/rfc1122/catalog.md#rfc1122-du-3) | later (module test: internal) | — | — | — |
| [RFC1122-TE-1](../../standard/rfc1122/catalog.md#rfc1122-te-1) | later (module test: a handoff) | — | — | — |
| [RFC1122-PP-1](../../standard/rfc1122/catalog.md#rfc1122-pp-1) | candidate (a should; a crafted total length) | — | — | — |
| [RFC1122-PP-2](../../standard/rfc1122/catalog.md#rfc1122-pp-2) | later (module test: a handoff) | — | — | — |
| [RFC1122-ERR-1](../../standard/rfc1122/catalog.md#rfc1122-err-1) | selected | [host-error-report](../../protocol/ipv4/checks/error-report.md#host-error-report) | Rfc1122HostErrorReport.test | PASS |
| [RFC6864-ID-1](../../standard/rfc6864/catalog.md#rfc6864-id-1) | later (internal) | — | — | — |
| [RFC6864-ID-2](../../standard/rfc6864/catalog.md#rfc6864-id-2) | covered | [atomic-identification](../../protocol/ipv4/checks/identification.md#atomic-identification) | Rfc6864AtomicIdentification.test | PASS (a may; the source chose distinct values) |
| [RFC6864-ID-3](../../standard/rfc6864/catalog.md#rfc6864-id-3) | selected | [atomic-identification](../../protocol/ipv4/checks/identification.md#atomic-identification) | Rfc6864AtomicIdentification.test | PASS |
| [RFC6864-ID-4](../../standard/rfc6864/catalog.md#rfc6864-id-4) | later (needs a retransmission: the TCP suite) | — | — | — |
| [RFC6864-ID-5](../../standard/rfc6864/catalog.md#rfc6864-id-5) | selected | [identification](../../protocol/ipv4/checks/identification.md#identification) | Rfc791Identification.test | PASS |
| [RFC6864-ID-6](../../standard/rfc6864/catalog.md#rfc6864-id-6) | covered | [dont-fragment](../../protocol/ipv4/checks/fragmentation.md#dont-fragment) | Rfc791DontFragment.test | PASS |
| [RFC6864-ID-7](../../standard/rfc6864/catalog.md#rfc6864-id-7) | selected | [atomic-identification](../../protocol/ipv4/checks/identification.md#atomic-identification) | Rfc6864AtomicIdentification.test | PASS |

67 entries: 23 of RFC 791 and RFC 792, 37 of RFC 1122, 7 of RFC 6864. 50 have a verdict
from a check that ran: 41 PASS, 6 FAIL as a model gap, 2 with one PASS and one FAIL
(RFC791-REASM-1 and REASM-3, each with two checks), 1 declined should. 17 have none:
7 need a module test (an interface or a handoff inside the host), 2 wait for level 4 (the
reassembly timer), 1 needs a TCP retransmission, 1 needs a standing negative rule in the
framework, 1 is refused by the model with an assertion, 2 are superseded or internal, and
3 are candidates that no check selected yet (a `may`, a `should`, and a conditional
`must`).

## Feature support

The rule of step 7: `supported` when every core check ran and passed, `partial` when a core
check passed and another core check failed as a model gap or did not run, `not supported`
when every core check that ran failed as a model gap, `untested` when no core check ran. A
feature spans documents, so its core checks may be RFC 1122 or RFC 6864 statements that
govern an RFC 791 one; the governing statement counts.

| Feature | Core checks and their state | Support |
| --- | --- | --- |
| [IPV4-F-DELIVERY](../../protocol/ipv4/features.md#ipv4-f-delivery) | RFC791-FWD-1, DLV-1, PROTO-1 all PASS | **supported** |
| [IPV4-F-HEADER](../../protocol/ipv4/features.md#ipv4-f-header) | RFC791-HDR-1, HDR-2, HDR-3 all PASS | **supported** |
| [IPV4-F-TTL](../../protocol/ipv4/features.md#ipv4-f-ttl) | RFC791-TTL-1 PASS, RFC791-TTL-2 PASS, RFC1122-TTL-2 PASS | **supported** |
| [IPV4-F-FRAGMENTATION](../../protocol/ipv4/features.md#ipv4-f-fragmentation) | RFC791-FRAG-1..4 all PASS | **supported** |
| [IPV4-F-REASSEMBLY](../../protocol/ipv4/features.md#ipv4-f-reassembly) | RFC1122-REASM-1 PASS, RFC791-REASM-1 PASS (procedure) and FAIL (four-field key), RFC791-REASM-3 PASS (interleaved) and FAIL (same identification) | **partial** |
| [IPV4-F-DONT-FRAGMENT](../../protocol/ipv4/features.md#ipv4-f-dont-fragment) | RFC6864-ID-6 PASS, RFC6864-ID-7 PASS | **supported** |
| [IPV4-F-IDENTIFICATION](../../protocol/ipv4/features.md#ipv4-f-identification) | RFC6864-ID-5 PASS, RFC6864-ID-3 PASS, RFC791-FRAG-3 PASS | **supported** |
| [IPV4-F-HEADER-CHECKSUM](../../protocol/ipv4/features.md#ipv4-f-header-checksum) | RFC791-CKSUM-1 PASS; RFC1122-CKSUM-1 FAIL | **partial** |
| [IPV4-F-MIN-SIZE](../../protocol/ipv4/features.md#ipv4-f-min-size) | RFC791-FRAG-6 PASS, RFC1122-REASM-2 PASS | **supported** |
| [IPV4-F-ERROR-REPORT](../../protocol/ipv4/features.md#ipv4-f-error-report) | RFC792-DU-4, RFC792-TE-1, RFC1122-ERR-1, RFC1122-DU-1, RFC1122-ICMP-2 all PASS | **supported** |
| [IPV4-F-INPUT-VALIDATION](../../protocol/ipv4/features.md#ipv4-f-input-validation) | RFC1122-ADDR-2 PASS; RFC1122-VER-1, CKSUM-1, ADDR-3, ICMP-1 FAIL | **partial** |
| [IPV4-F-ERROR-SUPPRESSION](../../protocol/ipv4/features.md#ipv4-f-error-suppression) | RFC1122-ICMP-5, ICMP-6, ICMP-8, ICMP-9 PASS; RFC1122-ICMP-7 FAIL | **partial** |
| [IPV4-F-ERROR-DELIVERY](../../protocol/ipv4/features.md#ipv4-f-error-delivery) | RFC1122-ICMP-3, DU-2, TE-1, PP-2: none ran (internal; module tests) | **untested** |

Eight features supported, four partial, one untested. The four partial ones are where the
level 3 toolset found the model wanting: the verification of the header checksum, the
silent discard of malformed or misaddressed input, the suppression of a report for a
link-layer broadcast, and the four-field reassembly key. The untested one is the handoff of
a received ICMP error to the transport protocol, which happens inside the host and needs a
module test.

Two bounds on these values, as before. A `supported` feature is supported as far as its
checks reach: one datagram size, one MTU, one topology per check. And the checksum
statements passed only with the model's computed mode enabled; in its default mode the
field is a placeholder, which [`results.md`](results.md) records.

## Achieved level

**Level 3, partial.** Target: level 3, from
[`standards.md`](../../protocol/ipv4/standards.md#target-level).

The exit criterion of level 3 has two halves. The first holds; the second does not.

**Hold** — the catalogs hold every mandatory statement of the in-scope documents, including
each `MUST NOT`. The RFC 1122 catalog was checked against the requirements summary of RFC
1122 §3.5 for every in-scope section: every `MUST` and `MUST NOT` row of §3.2.1.1, §3.2.1.2,
the validation rules of §3.2.1.3, §3.2.1.4, §3.2.1.7, §3.2.2 with §3.2.2.1, §3.2.2.4 and
§3.2.2.5, §3.3.2, §3.3.3 and §3.3.8 has an entry. The three deliberate omissions (a `may`
about several subnets, and two sentences addressed to the transport layer) are named at the
end of the catalog. The RFC 6864 catalog holds all seven `>>` requirements of its §4.

**Run** — each mandatory statement has a check that ran: **not yet.** 50 of 67 statements
have a verdict. 14 mandatory statements have none, and they fall into four kinds:

| Kind | Statements | What unlocks them |
| --- | --- | --- |
| an interface or a handoff inside the host | RFC1122-REASM-3, FRAG-2, ICMP-3, DU-2, DU-3, TE-1, PP-2; RFC6864-ID-1 | module tests (step 9 names the category; a later pass writes them in the module suite) |
| a timer | RFC1122-REASM-4, REASM-5 | level 4 |
| another vehicle | RFC6864-ID-4 (a retransmission: TCP), RFC1122-FRAG-3 (a host without local fragmentation) | the TCP suite; a host MTU below the datagram size |
| a tool the framework lacks | RFC1122-ADDR-4 (a standing negative rule over a whole run), RFC1122-TTL-1 (the model refuses the request with an assertion, which no protocol test can classify) | a framework feature; a module test |

| Level | State | Evidence |
| --- | --- | --- |
| 1, Survey | **reached** | The standards map pins the in-scope set; [`conformance.md`](conformance.md) maps every model claim onto it and names the missing version statement. |
| 2, Core | **reached** | The inventory of pass 2, and every level 2 statement with a PASS on this tree. |
| 3, Edge | **partial** | Both catalogs complete for the in-scope sections; 22 checks with the relay, the two-gateway and the one-link mockups; 6 model gaps found; 14 mandatory statements without a check, listed above. |
| 4, Dynamics | not started | the reassembly timer: RFC1122-REASM-4, REASM-5, with the `fragmentTimeout` parameter and a withheld fragment |
| 5, Complete | not started | options, type of service, the ICMP query messages, RFC 1812 for the gateway side |

Per feature, level 3 is reached when every mandatory statement mapped to the feature, core
or supporting, has a verdict:

| Feature | Level 3 | What blocks it |
| --- | --- | --- |
| IPV4-F-DELIVERY | reached | — |
| IPV4-F-HEADER | reached | — |
| IPV4-F-TTL | partial | RFC1122-TTL-1 (the assertion) |
| IPV4-F-FRAGMENTATION | partial | RFC1122-FRAG-2 (interface), FRAG-3 (conditional) |
| IPV4-F-REASSEMBLY | partial | RFC1122-REASM-3 (interface), REASM-4 and REASM-5 (level 4) |
| IPV4-F-DONT-FRAGMENT | reached | — |
| IPV4-F-IDENTIFICATION | partial | RFC6864-ID-4 (TCP), ID-1 (internal) |
| IPV4-F-HEADER-CHECKSUM | reached | — (one check fails; the level asks whether it ran) |
| IPV4-F-MIN-SIZE | reached | — |
| IPV4-F-ERROR-REPORT | reached | — |
| IPV4-F-INPUT-VALIDATION | partial | RFC1122-ADDR-4 (a standing rule) |
| IPV4-F-ERROR-SUPPRESSION | reached | — |
| IPV4-F-ERROR-DELIVERY | not reached | all four core statements are internal |

Seven of thirteen features reach level 3. The level and the support value are different
axes: IPV4-F-HEADER-CHECKSUM reaches level 3 with a `partial` support value, because its
verification check ran and failed, which is exactly what the level exists to find.

## Pass log

| Pass | Date | Level | Scope | Result |
| --- | --- | --- | --- | --- |
| 1 | 2026-09-02 | 2, partial | RFC 791 + RFC 792 error signals; 17 catalog entries; 3 checks; 3 tests | 4 PASS; two mandatory features untested |
| 2 | 2026-09-08 | **2, reached** | Level 2 pass: 23 catalog entries (6 new), 10 features (2 new), 7 checks (4 new), 7 tests (4 new, 1 extended). | 8 PASS; 9 features supported, 1 partial; see [`results.md`](results.md) and [`conformance.md`](conformance.md) |
| 3 | 2026-09-09 | **3, partial** | Level 3 pass: RFC 1122 and RFC 6864 enter the in-scope set (44 new catalog entries, 5 override cross references), 13 features (3 new, 2 levels changed by the new documents), 22 checks (15 new, split into one file per feature), 23 tests (15 new, 1 extended); the relay, two-gateway and one-link mockups. | 17 PASS, 6 FAIL (expected) as model gaps; 8 features supported, 4 partial, 1 untested; 14 mandatory statements without a check; see [`results.md`](results.md) and [`conformance.md`](conformance.md) |

## Out of scope

The catalogs record what this pass left out of the standards: options and source routing,
type of service, the security annex, the subnet and broadcast address forms as
destinations, host routing and redirects, source quench, the ICMP query messages, IGMP and
multicasting, and the layer interface of RFC 1122 §3.4. Each becomes level 4 or level 5
work, or the work of another protocol folder.
