# UDP — coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rfc768/catalog.md](../../standard/rfc768/catalog.md), [rfc792/catalog.md](../../standard/rfc792/catalog.md), [features.md](../../protocol/udp/features.md), [results.md](results.md)

The single place that holds the changing state of the UDP workflow. Every other artifact
of this protocol states what a standard says and never changes again unless the standard
changes. This one changes on every pass.

**No step edits an artifact of an earlier step. Steps 5, 6 and 7 record their outcome here.**

State of the ledger: run of 2026-09-08, on top of the IPv4 branch, source identical to
`master`.

## Statement coverage

`Status` is the state of the workflow, not of the standard: `selected` (a check targets it),
`covered` (a selected check establishes it as a side effect), `candidate` (practical, not yet
chosen), `later` (needs a toolset beyond the current level, or another test category).

| Catalog ID | Status | Check section | Test file | Verdict |
| --- | --- | --- | --- | --- |
| [RFC768-HDR-1](../../standard/rfc768/catalog.md#rfc768-hdr-1) | covered | [datagram-delivery](../../protocol/udp/checks.md#datagram-delivery) | Rfc768DatagramDelivery.test | PASS |
| [RFC768-HDR-2](../../standard/rfc768/catalog.md#rfc768-hdr-2) | selected | [datagram-delivery](../../protocol/udp/checks.md#datagram-delivery), [port-unreachable](../../protocol/udp/checks.md#port-unreachable) | Rfc768DatagramDelivery.test, Rfc768PortUnreachable.test | PASS |
| [RFC768-HDR-3](../../standard/rfc768/catalog.md#rfc768-hdr-3) | selected | [datagram-delivery](../../protocol/udp/checks.md#datagram-delivery) | Rfc768DatagramDelivery.test | PASS; the minimum of eight not observed, no sender can produce an empty datagram |
| [RFC768-CKSUM-1](../../standard/rfc768/catalog.md#rfc768-cksum-1) | later (unit test) | — | — | — |
| [RFC768-CKSUM-2](../../standard/rfc768/catalog.md#rfc768-cksum-2) | selected | [checksum-presence-and-absence](../../protocol/udp/checks.md#checksum-presence-and-absence) | Rfc768Checksum.test | PASS |
| [RFC768-UI-1](../../standard/rfc768/catalog.md#rfc768-ui-1) | selected | [datagram-delivery](../../protocol/udp/checks.md#datagram-delivery) | Rfc768DatagramDelivery.test | PASS; the data half. The source port and address at the program interface are not observable, and confirmed on the wire |
| [RFC768-IP-1](../../standard/rfc768/catalog.md#rfc768-ip-1) | later (unit test) | — | — | — |
| [RFC768-PROTO-1](../../standard/rfc768/catalog.md#rfc768-proto-1) | selected | [datagram-delivery](../../protocol/udp/checks.md#datagram-delivery) | Rfc768DatagramDelivery.test | PASS |
| [RFC792-DU-3](../../standard/rfc792/catalog.md#rfc792-du-3) | selected | [port-unreachable](../../protocol/udp/checks.md#port-unreachable) | Rfc768PortUnreachable.test | PASS |

9 entries: 7 reached a test and passed; 2 are unit-test material and wait for that suite.

## Feature support

The rule of step 7: `supported` when every core check ran and passed, `partial` when a core
check passed and another core check failed as a model gap or did not run, `not supported`
when every core check that ran failed as a model gap, `untested` when no core check ran.

| Feature | Core checks and their state | Support |
| --- | --- | --- |
| [UDP-F-DELIVERY](../../protocol/udp/features.md#udp-f-delivery) | RFC768-HDR-2, PROTO-1, UI-1 all PASS | **supported** |
| [UDP-F-HEADER](../../protocol/udp/features.md#udp-f-header) | RFC768-HDR-3 PASS | **supported** |
| [UDP-F-CHECKSUM](../../protocol/udp/features.md#udp-f-checksum) | RFC768-CKSUM-2 PASS | **supported** |
| [UDP-F-PORT-UNREACHABLE](../../protocol/udp/features.md#udp-f-port-unreachable) | RFC792-DU-3 PASS | **supported** |

All four features supported. Three bounds on that word:

1. `UDP-F-HEADER` rests on datagrams of 100 and 200 octets; the minimum length of eight was
   not observed, because no sender in the model can produce an empty datagram. Whether the
   receiver accepts one is a level 3 question, for injection.
2. `UDP-F-CHECKSUM` rests on presence and absence, which is all RFC 768 lets a wire check
   establish. That a nonzero value is the right one is RFC768-CKSUM-1, unit-test material;
   the test sets the computed mode, so the observed value is a real checksum, but the wire
   assertion alone would accept the model's declared placeholder.
3. `UDP-F-DELIVERY` confirms the source port and address on the wire, not at the program
   interface.

## Achieved level

**Level 2 reached.** Target: level 2, from
[`standards.md`](../../protocol/udp/standards.md#target-level).

**Hold** — every normal-path mandatory mechanism of RFC 768 appears as a feature. The
document is three pages, and the inventory is short:

| RFC 768 mechanism | Where | Feature, or the reason it is not one |
| --- | --- | --- |
| source port, optional | Fields | UDP-F-HEADER (supporting), UDP-F-DELIVERY |
| destination port selects the receiver | Fields | UDP-F-DELIVERY |
| length, minimum eight | Fields | UDP-F-HEADER |
| checksum over a pseudo header; zero means none | Fields | UDP-F-CHECKSUM, level `optional` |
| the user interface: ports, receive with source, send | User Interface | UDP-F-DELIVERY |
| addresses and protocol from the IP header | IP Interface | UDP-F-DELIVERY (supporting; unit test) |
| protocol 17 | Protocol Number | UDP-F-DELIVERY |
| port unreachable report | RFC 792 | UDP-F-PORT-UNREACHABLE, level `optional` |
| options in the surplus area | RFC 9868 | level 5; not in the in-scope set |

**Run** — every mandatory feature has a core check that ran and has a verdict: 2 of 2, all
core checks PASS.

| Level | State | Evidence |
| --- | --- | --- |
| 1, Survey | **reached** | The standards map pins the in-scope set; [`conformance.md`](conformance.md) maps the model claim onto it and names what the claim lacks. |
| 2, Core | **reached** | The inventory above; 7 of 7 level-2 statements with a PASS. |
| 3, Edge | not started | RFC 1122 §4.1 is not in the in-scope set; the discard of a wrong checksum and the 8-octet datagram need interception and injection. |
| 4, Dynamics | **not applicable** | UDP defines no timer and no control loop. |
| 5, Complete | not started | RFC 9868 options; the two unit-test entries. |

## Pass log

| Pass | Date | Level | Scope | Result |
| --- | --- | --- | --- | --- |
| 1 | 2026-09-08 | **2, reached** | RFC 768 in scope with RFC 792's port unreachable; 9 catalog entries (8 new, 1 added to the shared RFC 792 catalog); 4 features; 3 checks; 3 tests | 3 PASS; 4 features supported; two observations not runnable, recorded; see [`results.md`](results.md) and [`conformance.md`](conformance.md) |

## Out of scope

The catalog records what this pass left out: the checksum arithmetic and the IP interface
rule, both unit-test material; the options of RFC 9868; the RFC 1122 host requirements.
