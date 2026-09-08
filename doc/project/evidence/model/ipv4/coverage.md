# IPv4 — coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rfc791/catalog.md](../../standard/rfc791/catalog.md), [rfc792/catalog.md](../../standard/rfc792/catalog.md), [features.md](../../protocol/ipv4/features.md), [results.md](results.md)

The single place that holds the changing state of the IPv4 workflow. Every other artifact
of this protocol states what a standard says and never changes again unless the standard
changes. This one changes on every pass.

**No step edits an artifact of an earlier step. Steps 5, 6 and 7 record their outcome here.**

The ledger spans the documents of the in-scope set, so one table mixes RFC 791 and RFC 792
identifiers: a check may establish statements of two documents at once.

State of the ledger: run of 2026-09-08, tree at commit `63ef8b0322`.

## Statement coverage

`Status` is the state of the workflow, not of the standard: `selected` (a check targets it),
`covered` (a selected check establishes it as a side effect), `candidate` (practical, not yet
chosen), `later` (needs a toolset beyond the current level).

| Catalog ID | Status | Check section | Test file | Verdict |
| --- | --- | --- | --- | --- |
| [RFC791-TTL-1](../../standard/rfc791/catalog.md#rfc791-ttl-1) | selected | [ttl-decrement](../../protocol/ipv4/checks.md#ttl-decrement) | Rfc791TtlDecrement.test | PASS |
| [RFC791-TTL-2](../../standard/rfc791/catalog.md#rfc791-ttl-2) | selected | [ttl-expiry](../../protocol/ipv4/checks.md#ttl-expiry) | Rfc791TtlExpiry.test | PASS |
| [RFC791-TTL-3](../../standard/rfc791/catalog.md#rfc791-ttl-3) | covered | [ttl-decrement](../../protocol/ipv4/checks.md#ttl-decrement) | Rfc791TtlDecrement.test | PASS |
| [RFC791-FRAG-1](../../standard/rfc791/catalog.md#rfc791-frag-1) | selected | [fragment-and-reassembly](../../protocol/ipv4/checks.md#fragment-and-reassembly) | Rfc791FragmentReassembly.test | PASS |
| [RFC791-FRAG-2](../../standard/rfc791/catalog.md#rfc791-frag-2) | selected | [fragment-and-reassembly](../../protocol/ipv4/checks.md#fragment-and-reassembly) | Rfc791FragmentReassembly.test | PASS |
| [RFC791-FRAG-3](../../standard/rfc791/catalog.md#rfc791-frag-3) | selected | [fragment-and-reassembly](../../protocol/ipv4/checks.md#fragment-and-reassembly) | Rfc791FragmentReassembly.test | PASS |
| [RFC791-FRAG-4](../../standard/rfc791/catalog.md#rfc791-frag-4) | selected | [fragment-and-reassembly](../../protocol/ipv4/checks.md#fragment-and-reassembly) | Rfc791FragmentReassembly.test | PASS |
| [RFC791-FRAG-5](../../standard/rfc791/catalog.md#rfc791-frag-5) | selected | [dont-fragment](../../protocol/ipv4/checks.md#dont-fragment) | Rfc791DontFragment.test | PASS |
| [RFC791-FRAG-6](../../standard/rfc791/catalog.md#rfc791-frag-6) | selected | [minimum-sizes](../../protocol/ipv4/checks.md#minimum-sizes) | Rfc791MinimumSizes.test | PASS |
| [RFC791-REASM-1](../../standard/rfc791/catalog.md#rfc791-reasm-1) | selected | [fragment-and-reassembly](../../protocol/ipv4/checks.md#fragment-and-reassembly) | Rfc791FragmentReassembly.test | PASS |
| [RFC791-REASM-2](../../standard/rfc791/catalog.md#rfc791-reasm-2) | selected | [minimum-sizes](../../protocol/ipv4/checks.md#minimum-sizes) | Rfc791MinimumSizes.test | PASS |
| [RFC791-REASM-3](../../standard/rfc791/catalog.md#rfc791-reasm-3) | later (level 3) | — | — | — |
| [RFC791-CKSUM-1](../../standard/rfc791/catalog.md#rfc791-cksum-1) | selected | [ttl-decrement](../../protocol/ipv4/checks.md#ttl-decrement) | Rfc791TtlDecrement.test | PASS |
| [RFC791-CKSUM-2](../../standard/rfc791/catalog.md#rfc791-cksum-2) | later (level 3) | — | — | — |
| [RFC791-ID-1](../../standard/rfc791/catalog.md#rfc791-id-1) | selected | [identification](../../protocol/ipv4/checks.md#identification) | Rfc791Identification.test | PASS |
| [RFC791-HDR-1](../../standard/rfc791/catalog.md#rfc791-hdr-1) | selected | [datagram-delivery](../../protocol/ipv4/checks.md#datagram-delivery) | Rfc791DatagramDelivery.test | PASS |
| [RFC791-HDR-2](../../standard/rfc791/catalog.md#rfc791-hdr-2) | selected | [datagram-delivery](../../protocol/ipv4/checks.md#datagram-delivery) | Rfc791DatagramDelivery.test | PASS |
| [RFC791-HDR-3](../../standard/rfc791/catalog.md#rfc791-hdr-3) | selected | [datagram-delivery](../../protocol/ipv4/checks.md#datagram-delivery) | Rfc791DatagramDelivery.test | PASS |
| [RFC791-PROTO-1](../../standard/rfc791/catalog.md#rfc791-proto-1) | selected | [datagram-delivery](../../protocol/ipv4/checks.md#datagram-delivery) | Rfc791DatagramDelivery.test | PASS |
| [RFC791-FWD-1](../../standard/rfc791/catalog.md#rfc791-fwd-1) | selected | [datagram-delivery](../../protocol/ipv4/checks.md#datagram-delivery) | Rfc791DatagramDelivery.test | PASS |
| [RFC791-DLV-1](../../standard/rfc791/catalog.md#rfc791-dlv-1) | selected | [datagram-delivery](../../protocol/ipv4/checks.md#datagram-delivery) | Rfc791DatagramDelivery.test | PASS |
| [RFC792-TE-1](../../standard/rfc792/catalog.md#rfc792-te-1) | selected | [ttl-expiry](../../protocol/ipv4/checks.md#ttl-expiry) | Rfc791TtlExpiry.test | PASS |
| [RFC792-DU-4](../../standard/rfc792/catalog.md#rfc792-du-4) | selected | [dont-fragment](../../protocol/ipv4/checks.md#dont-fragment) | Rfc791DontFragment.test | PASS |

23 entries: 21 reached a test and passed; 2 wait for the level 3 toolset.

## Feature support

The rule of step 7: `supported` when every core check ran and passed, `partial` when a core
check passed and another core check failed as a model gap or did not run, `not supported`
when every core check that ran failed as a model gap, `untested` when no core check ran.

| Feature | Core checks and their state | Support |
| --- | --- | --- |
| [IPV4-F-DELIVERY](../../protocol/ipv4/features.md#ipv4-f-delivery) | RFC791-FWD-1, DLV-1, PROTO-1 all PASS | **supported** |
| [IPV4-F-HEADER](../../protocol/ipv4/features.md#ipv4-f-header) | RFC791-HDR-1, HDR-2, HDR-3 all PASS | **supported** |
| [IPV4-F-TTL](../../protocol/ipv4/features.md#ipv4-f-ttl) | RFC791-TTL-1 PASS, RFC791-TTL-2 PASS | **supported** |
| [IPV4-F-FRAGMENTATION](../../protocol/ipv4/features.md#ipv4-f-fragmentation) | RFC791-FRAG-1..4 all PASS | **supported** |
| [IPV4-F-REASSEMBLY](../../protocol/ipv4/features.md#ipv4-f-reassembly) | RFC791-REASM-1 PASS | **supported** |
| [IPV4-F-DONT-FRAGMENT](../../protocol/ipv4/features.md#ipv4-f-dont-fragment) | RFC791-FRAG-5 PASS | **supported** |
| [IPV4-F-IDENTIFICATION](../../protocol/ipv4/features.md#ipv4-f-identification) | RFC791-ID-1 PASS, RFC791-FRAG-3 PASS | **supported** |
| [IPV4-F-HEADER-CHECKSUM](../../protocol/ipv4/features.md#ipv4-f-header-checksum) | RFC791-CKSUM-1 PASS; RFC791-CKSUM-2 did not run (level 3) | **partial** |
| [IPV4-F-MIN-SIZE](../../protocol/ipv4/features.md#ipv4-f-min-size) | RFC791-FRAG-6 PASS, RFC791-REASM-2 PASS | **supported** |
| [IPV4-F-ERROR-REPORT](../../protocol/ipv4/features.md#ipv4-f-error-report) | RFC792-DU-4 PASS, RFC792-TE-1 PASS | **supported** |

Nine features supported, one partial. The partial one is the verification half of the
checksum, which needs a corrupted datagram in flight; the level 2 half, the recompute per
hop, passed. No test failed against the model, so nothing is `not supported`.

Two bounds on these values. A `supported` feature is supported as far as its checks reach:
one datagram size, one MTU per check, one topology. And the checksum passed only with the
model's computed mode enabled; in its default mode the field is a placeholder, which
[`results.md`](results.md#model-observations-the-checks-did-not-claim) records.

## Achieved level

**Level 2 reached.** Target: level 2, from
[`standards.md`](../../protocol/ipv4/standards.md#target-level).

The exit criterion of level 2 has two halves, and both hold.

**Hold** — every normal-path mandatory mechanism of RFC 791 appears as a feature. The
inventory below is what makes that claim checkable: every mechanism the document defines,
and where it went.

| RFC 791 mechanism | Where | Feature, or the reason it is not one |
| --- | --- | --- |
| addressing, forwarding by a gateway, delivery to the program | §2.2, §2.4, §3.1 | IPV4-F-DELIVERY |
| version, header length, total length | §3.1 | IPV4-F-HEADER |
| type of service and precedence | §3.1 | out of scope: RFC 2474 replaced the byte; level 5 |
| identification, flags, offset; fragmentation | §2.3, §3.1 | IPV4-F-FRAGMENTATION, IPV4-F-DONT-FRAGMENT, IPV4-F-IDENTIFICATION |
| the 68-octet and 576-octet floors | §3.1, §3.2 | IPV4-F-MIN-SIZE |
| time to live | §3.1, §3.2 | IPV4-F-TTL |
| protocol field | §3.1 | IPV4-F-DELIVERY |
| header checksum | §1.4, §3.1 | IPV4-F-HEADER-CHECKSUM |
| source and destination addresses | §3.1 | IPV4-F-DELIVERY |
| options | §3.1 | level 5, by the level table of the guide |
| reassembly | §2.3, §3.2 | IPV4-F-REASSEMBLY, level `unstated` |
| error reports through ICMP | §3.2, RFC 792 | IPV4-F-ERROR-REPORT, level `optional` |
| security annex | appendix | out of scope |

**Run** — every mandatory feature has a core check that ran and has a verdict: 8 of 8
mandatory features, every core check that belongs to level 2 ran, all PASS.

| Level | State | Evidence |
| --- | --- | --- |
| 1, Survey | **reached** | The standards map pins the in-scope set; [`conformance.md`](conformance.md) maps every model claim onto it and names the missing version statement. |
| 2, Core | **reached** | The inventory above, and 21 of 21 level-2 statements with a PASS. |
| 3, Edge | not started | RFC791-CKSUM-2 and RFC791-REASM-3 wait for interception and injection; RFC 1122 and RFC 6864 are not in the in-scope set. CKSUM-2 is the first candidate for a defect: see the results. |
| 4, Dynamics | not started | the reassembly timer |
| 5, Complete | not started | options, type of service, the other ICMP messages |

Per feature, every one of the ten reaches level 2. `IPV4-F-HEADER-CHECKSUM` reaches it with
a `partial` support value, because its second core check is level 3 work; the two axes are
different, and the ledger keeps them apart.

## Pass log

| Pass | Date | Level | Scope | Result |
| --- | --- | --- | --- | --- |
| 1 | 2026-09-02 | 2, partial | RFC 791 + RFC 792 error signals; 17 catalog entries; 3 checks; 3 tests | 4 PASS; two mandatory features untested |
| 2 | 2026-09-08 | **2, reached** | Level 2 pass: 23 catalog entries (6 new), 10 features (2 new), 7 checks (4 new), 7 tests (4 new, 1 extended). Supersedes an artifact-only re-application that was dropped with its branch. | 8 PASS; 9 features supported, 1 partial; see [`results.md`](results.md) and [`conformance.md`](conformance.md) |

## Out of scope

The catalogs record what this pass left out of the standard: options, type of service and
precedence, the security annex, the reassembly timer, and the ICMP messages other than the
two error reports. Each becomes level 3 to level 5 work.
