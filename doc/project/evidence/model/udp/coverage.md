# UDP — coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rfc768/catalog.md](../../standard/rfc768/catalog.md), [rfc792/catalog.md](../../standard/rfc792/catalog.md), [rfc1122/catalog.md](../../standard/rfc1122/catalog.md), [features.md](../../protocol/udp/features.md), [results.md](results.md)

The single place that holds the changing state of the UDP workflow. Every other artifact
of this protocol states what a standard says and never changes again unless the standard
changes. This one changes on every pass.

**No step edits an artifact of an earlier step. Steps 5, 6 and 7 record their outcome here.**

State of the ledger, from this run:

- Date: 2026-09-10 15:18 +0200
- INET: branch `master`, commit `0868c36c88`, tree clean
- OMNeT++: 6.4.0
- Build: debug, built from this commit
- Compiler: Ubuntu clang version 23.0.0
- Platform: Ubuntu 26.04.1 LTS, Linux 7.0.0-31-generic x86_64
- Command: `inet_run_protocol_tests -p inet -w udp`
- Target level: 3

## Statement coverage

`Status` is the state of the workflow, not of the standard: `selected` (a check targets it),
`covered` (a selected check establishes it as a side effect), `candidate` (practical, not yet
chosen), `later` (needs a toolset beyond the current level, or another test category),
`no check` (no check of two nodes on a link can observe it; the reason is in
[`checks.md`](../../protocol/udp/checks.md#statements-that-no-check-carries)).

| Catalog ID | Status | Check section | Test file | Verdict |
| --- | --- | --- | --- | --- |
| [RFC768-HDR-1](../../standard/rfc768/catalog.md#rfc768-hdr-1) | covered | [datagram-delivery](../../protocol/udp/checks/delivery.md#datagram-delivery) | Rfc768DatagramDelivery.test | PASS |
| [RFC768-HDR-2](../../standard/rfc768/catalog.md#rfc768-hdr-2) | selected | [datagram-delivery](../../protocol/udp/checks/delivery.md#datagram-delivery), [port-unreachable](../../protocol/udp/checks/port-unreachable.md#port-unreachable) | Rfc768DatagramDelivery.test, Rfc768PortUnreachable.test | PASS |
| [RFC768-HDR-3](../../standard/rfc768/catalog.md#rfc768-hdr-3) | selected | [datagram-delivery](../../protocol/udp/checks/delivery.md#datagram-delivery), [empty-datagram](../../protocol/udp/checks/delivery.md#empty-datagram) | Rfc768DatagramDelivery.test, Rfc768EmptyDatagram.test | PASS for a datagram that carries data; **FAIL** for the minimum of eight, gap 3 |
| [RFC768-CKSUM-1](../../standard/rfc768/catalog.md#rfc768-cksum-1) | selected | [checksum-covers-the-data](../../protocol/udp/checks/checksum.md#checksum-covers-the-data), [checksum-covers-the-pseudo-header](../../protocol/udp/checks/checksum.md#checksum-covers-the-pseudo-header) | Rfc768ChecksumCoversData.test, Rfc768ChecksumCoversPseudoHeader.test | PASS; level 2 left this to a unit test, level 3 reached it from the wire |
| [RFC768-CKSUM-2](../../standard/rfc768/catalog.md#rfc768-cksum-2) | selected | [checksum-presence-and-absence](../../protocol/udp/checks/checksum.md#checksum-presence-and-absence), [zero-checksum-accepted](../../protocol/udp/checks/checksum.md#zero-checksum-accepted) | Rfc768Checksum.test, Rfc768ZeroChecksumAccepted.test | PASS |
| [RFC768-UI-1](../../standard/rfc768/catalog.md#rfc768-ui-1) | selected | [datagram-delivery](../../protocol/udp/checks/delivery.md#datagram-delivery) | Rfc768DatagramDelivery.test | PASS; the data half. The source port and address at the program interface are not observable, and confirmed on the wire |
| [RFC768-IP-1](../../standard/rfc768/catalog.md#rfc768-ip-1) | later (unit test) | — | — | — |
| [RFC768-PROTO-1](../../standard/rfc768/catalog.md#rfc768-proto-1) | selected | [datagram-delivery](../../protocol/udp/checks/delivery.md#datagram-delivery) | Rfc768DatagramDelivery.test | PASS |
| [RFC792-DU-3](../../standard/rfc792/catalog.md#rfc792-du-3) | selected | [port-unreachable](../../protocol/udp/checks/port-unreachable.md#port-unreachable) | Rfc768PortUnreachable.test | PASS |
| [RFC1122-UCK-1](../../standard/rfc1122/catalog.md#rfc1122-uck-1) | covered | [checksum-by-default](../../protocol/udp/checks/checksum.md#checksum-by-default) | Rfc1122ChecksumDefault.test | the generate half **FAILS** by default, gap 1; the check half PASSES, in four tests that set the computed mode |
| [RFC1122-UCK-2](../../standard/rfc1122/catalog.md#rfc1122-uck-2) | no check | — | — | a permission for a program interface |
| [RFC1122-UCK-3](../../standard/rfc1122/catalog.md#rfc1122-uck-3) | selected | [checksum-by-default](../../protocol/udp/checks/checksum.md#checksum-by-default) | Rfc1122ChecksumDefault.test | **FAIL**, gap 1 |
| [RFC1122-UCK-4](../../standard/rfc1122/catalog.md#rfc1122-uck-4) | selected | [checksum-discard](../../protocol/udp/checks/checksum.md#checksum-discard), [checksum-covers-the-data](../../protocol/udp/checks/checksum.md#checksum-covers-the-data), [checksum-covers-the-pseudo-header](../../protocol/udp/checks/checksum.md#checksum-covers-the-pseudo-header) | Rfc1122ChecksumDiscard.test, Rfc768ChecksumCoversData.test, Rfc768ChecksumCoversPseudoHeader.test | PASS, from three sides |
| [RFC1122-UCK-5](../../standard/rfc1122/catalog.md#rfc1122-uck-5) | selected | [zero-checksum-accepted](../../protocol/udp/checks/checksum.md#zero-checksum-accepted) | Rfc768ZeroChecksumAccepted.test | PASS, the behaviour when no program uses the permission |
| [RFC1122-UCK-6](../../standard/rfc1122/catalog.md#rfc1122-uck-6) | no check | — | — | needs data whose sum is exactly zero, which the run's addresses and ports decide |
| [RFC1122-UPORT-1](../../standard/rfc1122/catalog.md#rfc1122-uport-1) | selected | [port-unreachable](../../protocol/udp/checks/port-unreachable.md#port-unreachable) | Rfc768PortUnreachable.test | PASS; the same test, now read against a should |
| [RFC1122-UERR-1](../../standard/rfc1122/catalog.md#rfc1122-uerr-1) | no check | — | — | the report reaches the program as an Indication, not as a packet |
| [RFC1122-UOPT-1](../../standard/rfc1122/catalog.md#rfc1122-uopt-1) | no check | — | — | at the program interface |
| [RFC1122-UOPT-2](../../standard/rfc1122/catalog.md#rfc1122-uopt-2) | no check | — | — | no sending program offers an interface for IP options |
| [RFC1122-UMH-1](../../standard/rfc1122/catalog.md#rfc1122-umh-1) | no check | — | — | at the program interface |
| [RFC1122-UMH-2](../../standard/rfc1122/catalog.md#rfc1122-umh-2) | selected | [source-address-from-the-program](../../protocol/udp/checks/app-interface.md#source-address-from-the-program) | Rfc1122ApplicationSourceAddress.test | PASS |
| [RFC1122-UMH-3](../../standard/rfc1122/catalog.md#rfc1122-umh-3) | no check | — | — | at the program interface |
| [RFC1122-UADDR-1](../../standard/rfc1122/catalog.md#rfc1122-uaddr-1) | selected | [multicast-source-address](../../protocol/udp/checks/input-validation.md#multicast-source-address) | Rfc1122MulticastSourceAddress.test | **FAIL**, gap 2 |
| [RFC1122-UADDR-2](../../standard/rfc1122/catalog.md#rfc1122-uaddr-2) | selected | [valid-source-address](../../protocol/udp/checks/delivery.md#valid-source-address) | Rfc1122ValidSourceAddress.test | PASS |
| [RFC1122-UAPI-1](../../standard/rfc1122/catalog.md#rfc1122-uapi-1) | selected | [ttl-and-tos-from-the-program](../../protocol/udp/checks/app-interface.md#ttl-and-tos-from-the-program) | Rfc1122ApplicationTtlAndTos.test | PASS for the TTL and the TOS; the IP options are not reachable |
| [RFC1122-UAPI-2](../../standard/rfc1122/catalog.md#rfc1122-uapi-2) | no check | — | — | at the program interface; and a may |
| [RFC1122-UAPI-3](../../standard/rfc1122/catalog.md#rfc1122-uapi-3) | no check | — | — | §3.4 is an interface catalog, not a behaviour on a link |

27 entries. 17 reached a test: 14 with a PASS and 3 with a declared FAIL that names a model
gap. 9 carry `no check`, each with its reason, and 1 waits for a unit-test suite. The nine
without a check are all statements about the interface between UDP and a program; the list
is in [`checks.md`](../../protocol/udp/checks.md#statements-that-no-check-carries).

## Feature support

The rule of step 7: `supported` when every core check ran and passed, `partial` when a core
check passed and another core check failed as a model gap or did not run, `not supported`
when every core check that ran failed as a model gap, `untested` when no core check ran.

| Feature | Core checks and their state | Support |
| --- | --- | --- |
| [UDP-F-DELIVERY](../../protocol/udp/features.md#udp-f-delivery) | RFC768-HDR-2, PROTO-1, UI-1, RFC1122-UADDR-2 all PASS | **supported** |
| [UDP-F-HEADER](../../protocol/udp/features.md#udp-f-header) | RFC768-HDR-3 PASS with data, FAIL at the minimum | **partial** |
| [UDP-F-CHECKSUM](../../protocol/udp/features.md#udp-f-checksum) | RFC1122-UCK-1 half FAIL, RFC1122-UCK-3 FAIL, RFC768-CKSUM-2 PASS | **partial** |
| [UDP-F-PORT-UNREACHABLE](../../protocol/udp/features.md#udp-f-port-unreachable) | RFC1122-UPORT-1, RFC792-DU-3 PASS | **supported** |
| [UDP-F-INPUT-VALIDATION](../../protocol/udp/features.md#udp-f-input-validation) | RFC1122-UCK-4 PASS, RFC1122-UADDR-1 FAIL | **partial** |
| [UDP-F-ERROR-DELIVERY](../../protocol/udp/features.md#udp-f-error-delivery) | RFC1122-UERR-1 has no check | **untested** |
| [UDP-F-APP-INTERFACE](../../protocol/udp/features.md#udp-f-app-interface) | RFC1122-UAPI-1 and UMH-2 PASS; UOPT-1, UOPT-2 and UMH-1 have no check | **partial** |

Seven features: three supported, three partial, one untested. Pass 1 called all four features
of level 2 supported; two of those four are partial now, and neither changed its behaviour.
What changed is the measure. RFC 1122 entered the in-scope set, and it demands of a host what
RFC 768 only describes:

1. `UDP-F-CHECKSUM` was supported when the question was "is a checksum present or absent".
   RFC 1122 asks whether a host generates one by default and whether it is the right value.
   The answer is no by default and yes in the computed mode; see gap 1.
2. `UDP-F-HEADER` was supported for datagrams that carry data. The minimum of eight is now
   observed, and it stops the run; see gap 3.
3. `UDP-F-INPUT-VALIDATION` is new. Half of it works well: a wrong checksum is discarded in
   silence, confirmed from three sides. The other half, the invalid source address, is not
   implemented at either layer; see gap 2.
4. `UDP-F-ERROR-DELIVERY` is `untested`, not `not supported`. The model does implement the
   path — `Udp::processIcmpv4Error` hands the report to the socket — but it hands it up as an
   `Indication`, and the tester turns only packet signals into events. No verdict is claimed
   where no check ran.

Three bounds that pass 1 recorded, and where they stand now:

1. The minimum length of eight: **reached** at level 3, and it is gap 3.
2. The checksum arithmetic: **reached** at level 3. Two tests establish the coverage from the
   wire, and a third computes the value the standard defines and compares it with the field.
   The bound is gone, and gap 1 is what it uncovered.
3. `UDP-F-DELIVERY` still confirms the source port and address on the wire and not at the
   program interface. This bound stands.

## Achieved level

**Level 3 partial.** Target: level 3, from
[`standards.md`](../../protocol/udp/standards.md#target-level).

The exit criterion of level 3 is that the catalogs hold every mandatory statement of the
in-scope documents, including each MUST NOT, and that each one has a check.

**Hold** — the catalogs hold every statement of RFC 768, the port unreachable entry of
RFC 792, and all 18 statements of RFC 1122 §4.1. Nothing of §4.1 was left out: the two other
parts of that section hold no checkable statement.

**Does not hold** — of the 18 statements of §4.1, nine have a check that ran and nine have
none. The nine without a check share one reason: they describe the interface between UDP and
a program, and a check that watches two nodes on a link cannot see it. Five of the nine are a
`must`. Until a module test can watch the gate between UDP and the program above it, level 3
stays partial for this protocol. This is a limit of the toolset and not a verdict on the
model.

| Level | State | Evidence |
| --- | --- | --- |
| 1, Survey | **reached** | The standards map pins the in-scope set; [`conformance.md`](conformance.md) maps the model claim onto it and names what the claim lacks. |
| 2, Core | **reached** | Pass 1: every normal-path mandatory mechanism of RFC 768 is a feature, and every mandatory feature has a core check with a verdict. |
| 3, Edge | **partial** | All 18 statements of RFC 1122 §4.1 are in the catalog; 9 have a check that ran, and the 9 at the program interface have none. Three model gaps found. |
| 4, Dynamics | **not applicable** | UDP defines no timer and no control loop. |
| 5, Complete | not started | RFC 9868 options; the RFC768-IP-1 unit-test entry. |

## Pass log

| Pass | Date | Level | Scope | Result |
| --- | --- | --- | --- | --- |
| 1 | 2026-09-08 | **2, reached** | RFC 768 in scope with RFC 792's port unreachable; 9 catalog entries (8 new, 1 added to the shared RFC 792 catalog); 4 features; 3 checks; 3 tests | 3 PASS; 4 features supported; two observations not runnable, recorded; see [`results.md`](results.md) and [`conformance.md`](conformance.md) |
| 2 | 2026-09-09 | **3, partial** | RFC 1122 §4.1 added to the in-scope set; 18 catalog entries added to the shared RFC 1122 catalog; 3 features added, 2 changed level; 10 checks added; 10 tests added | 13 tests: 10 PASS, 3 declared FAIL naming three model gaps; 3 features supported, 3 partial, 1 untested; 9 statements carry `no check` with a reason; see [`results.md`](results.md) |

## Out of scope

What the two passes left out: the IP interface rule RFC768-IP-1, which is unit-test material;
the options of RFC 9868, which are level 5; and the nine statements of RFC 1122 §4.1 that
live at the interface between UDP and a program, which need a test category that watches a
module gate and not a link. The checksum arithmetic and the RFC 1122 host requirements were
out of scope after pass 1 and are in scope now.
