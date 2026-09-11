# ARP — coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rfc826/catalog.md](../../standard/rfc826/catalog.md), [rfc1122/catalog.md](../../standard/rfc1122/catalog.md), [rfc5494/catalog.md](../../standard/rfc5494/catalog.md), [features.md](../../protocol/arp/features.md), [results.md](results.md)

The single place that holds the changing state of the ARP workflow. Every other artifact of
this protocol states what a standard says and never changes again unless the standard
changes. This one changes on every pass.

**No step edits an artifact of an earlier step. Steps 5, 6 and 7 record their outcome here.**

State of the ledger, from this run:

- Date: 2026-09-11 10:50 +0200
- INET: branch `master`, commit `223ba89ce5`, tree clean
- OMNeT++: 6.4.0
- Build: debug, built from this commit
- Compiler: Ubuntu clang version 23.0.0
- Platform: Ubuntu 26.04.1 LTS, Linux 7.0.0-31-generic x86_64
- Command: `inet_run_protocol_tests -p inet -w arp`
- Target level: 3

## Statement coverage

`Status` is the state of the workflow, not of the standard: `selected` (a check targets it),
`covered` (a selected check establishes it as a side effect), `candidate` (practical, not yet
chosen), `later` (needs a toolset beyond the current level, or another test category),
`no check` (no check of two nodes on a link can observe it; the reason is in
[`checks.md`](../../protocol/arp/checks.md#statements-that-no-check-carries)).

| Catalog ID | Status | Check section | Test file | Verdict |
| --- | --- | --- | --- | --- |
| [RFC826-FMT-1](../../standard/rfc826/catalog.md#rfc826-fmt-1) | selected | [packet-layout-on-the-wire](../../protocol/arp/checks/packet-format.md#packet-layout-on-the-wire) | Rfc826PacketLayout.test | PASS |
| [RFC826-FMT-2](../../standard/rfc826/catalog.md#rfc826-fmt-2) | selected | [packet-layout-on-the-wire](../../protocol/arp/checks/packet-format.md#packet-layout-on-the-wire) | Rfc826PacketLayout.test | PASS |
| [RFC826-FMT-3](../../standard/rfc826/catalog.md#rfc826-fmt-3) | covered | [address-resolution](../../protocol/arp/checks/resolution.md#address-resolution) | Rfc826AddressResolution.test | PASS, through the Ethernet type field of the frame |
| [RFC826-FMT-4](../../standard/rfc826/catalog.md#rfc826-fmt-4) | selected | [packet-layout-on-the-wire](../../protocol/arp/checks/packet-format.md#packet-layout-on-the-wire) | Rfc826PacketLayout.test | PASS |
| [RFC826-FMT-5](../../standard/rfc826/catalog.md#rfc826-fmt-5) | covered | [address-resolution](../../protocol/arp/checks/resolution.md#address-resolution), [reply-field-values](../../protocol/arp/checks/reply.md#reply-field-values) | Rfc826AddressResolution.test, Rfc826ReplyFields.test | PASS, both opcode values |
| [RFC826-FMT-6](../../standard/rfc826/catalog.md#rfc826-fmt-6) | covered | [reply-field-values](../../protocol/arp/checks/reply.md#reply-field-values) | Rfc826ReplyFields.test | PASS, both packets are 28 octets |
| [RFC826-FMT-7](../../standard/rfc826/catalog.md#rfc826-fmt-7) | selected | [packet-layout-on-the-wire](../../protocol/arp/checks/packet-format.md#packet-layout-on-the-wire) | Rfc826PacketLayout.test | PASS |
| [RFC826-REQ-1](../../standard/rfc826/catalog.md#rfc826-req-1) | selected | [address-resolution](../../protocol/arp/checks/resolution.md#address-resolution) | Rfc826AddressResolution.test | PASS |
| [RFC826-REQ-2](../../standard/rfc826/catalog.md#rfc826-req-2) | selected | [the-cached-mapping](../../protocol/arp/checks/resolution.md#the-cached-mapping) | Rfc826CachedMapping.test | PASS, one request for five datagrams |
| [RFC826-REQ-3](../../standard/rfc826/catalog.md#rfc826-req-3) | covered | [the-waiting-datagram](../../protocol/arp/checks/queue.md#the-waiting-datagram) | Rfc1122ArpPacketQueue.test | overridden. The model follows the RFC 1122 statement that governs, so it does not throw the datagram away |
| [RFC826-REQ-4](../../standard/rfc826/catalog.md#rfc826-req-4) | selected | [address-resolution](../../protocol/arp/checks/resolution.md#address-resolution) | Rfc826AddressResolution.test | PASS for the five fields the model has; the hardware space, the protocol space and the two lengths are the constants of gap 2 and are right for this link |
| [RFC826-REQ-5](../../standard/rfc826/catalog.md#rfc826-req-5) | no check | — | — | a permission about a field the standard calls meaningless. The run showed all zeros in it |
| [RFC826-REQ-6](../../standard/rfc826/catalog.md#rfc826-req-6) | selected | [address-resolution](../../protocol/arp/checks/resolution.md#address-resolution) | Rfc826AddressResolution.test | PASS |
| [RFC826-RECV-1](../../standard/rfc826/catalog.md#rfc826-recv-1) | covered | the five checks of [input-validation](../../protocol/arp/checks/input-validation.md) | five tests | PASS in three, **FAIL** in two: gap 1 and gap 2 |
| [RFC826-RECV-2](../../standard/rfc826/catalog.md#rfc826-recv-2) | selected | [an-experimental-hardware-space](../../protocol/arp/checks/input-validation.md#an-experimental-hardware-space) | Rfc5494ExperimentalHardwareSpace.test | **FAIL**, gap 2 |
| [RFC826-RECV-3](../../standard/rfc826/catalog.md#rfc826-recv-3) | selected | [an-unknown-protocol-space](../../protocol/arp/checks/input-validation.md#an-unknown-protocol-space) | Rfc826UnknownProtocolSpace.test | **FAIL**, gap 2 |
| [RFC826-RECV-4](../../standard/rfc826/catalog.md#rfc826-recv-4) | selected | [a-newer-hardware-address](../../protocol/arp/checks/cache.md#a-newer-hardware-address) | Rfc826SupersedingHardwareAddress.test | PASS |
| [RFC826-RECV-5](../../standard/rfc826/catalog.md#rfc826-recv-5) | selected | [a-request-for-a-third-station](../../protocol/arp/checks/input-validation.md#a-request-for-a-third-station) | Rfc826ThirdStationRequest.test | PASS for a host. A router with proxy ARP answers for an address that is not its own; see [`results.md`](results.md) |
| [RFC826-RECV-6](../../standard/rfc826/catalog.md#rfc826-recv-6) | selected | [learning-from-a-request](../../protocol/arp/checks/reply.md#learning-from-a-request), [a-reply-fills-the-table](../../protocol/arp/checks/cache.md#a-reply-fills-the-table) | Rfc826LearningFromRequest.test, Rfc826MergeBeforeOpcode.test | PASS, from a request and from a reply |
| [RFC826-RECV-7](../../standard/rfc826/catalog.md#rfc826-recv-7) | selected | [learning-from-a-request](../../protocol/arp/checks/reply.md#learning-from-a-request), [a-reply-fills-the-table](../../protocol/arp/checks/cache.md#a-reply-fills-the-table) | Rfc826LearningFromRequest.test, Rfc826MergeBeforeOpcode.test | PASS, both branches of the opcode question |
| [RFC826-RECV-8](../../standard/rfc826/catalog.md#rfc826-recv-8) | selected | [reply-field-values](../../protocol/arp/checks/reply.md#reply-field-values) | Rfc826ReplyFields.test | PASS, all four address fields |
| [RFC826-RECV-9](../../standard/rfc826/catalog.md#rfc826-recv-9) | selected | [reply-field-values](../../protocol/arp/checks/reply.md#reply-field-values) | Rfc826ReplyFields.test | PASS for "direct, not broadcast". "On the same hardware" needs a host with two interfaces |
| [RFC826-RECV-10](../../standard/rfc826/catalog.md#rfc826-recv-10) | selected | [an-unsolicited-reply](../../protocol/arp/checks/input-validation.md#an-unsolicited-reply), [an-experimental-opcode](../../protocol/arp/checks/input-validation.md#an-experimental-opcode) | Rfc826UnsolicitedReply.test, Rfc5494ExperimentalOpcode.test | PASS for a reply; **FAIL** for any other opcode, gap 1 |
| [RFC826-RECV-11](../../standard/rfc826/catalog.md#rfc826-recv-11) | covered | [packet-layout-on-the-wire](../../protocol/arp/checks/packet-format.md#packet-layout-on-the-wire) | Rfc826PacketLayout.test | PASS on the sending side: the two lengths agree with the widths of the address fields. The receiving side is a permission the model cannot use, because it has no field to read |
| [RFC826-TABLE-1](../../standard/rfc826/catalog.md#rfc826-table-1) | selected | [a-newer-hardware-address](../../protocol/arp/checks/cache.md#a-newer-hardware-address) | Rfc826SupersedingHardwareAddress.test | PASS |
| [RFC826-TABLE-2](../../standard/rfc826/catalog.md#rfc826-table-2) | covered | [address-resolution](../../protocol/arp/checks/resolution.md#address-resolution), [reply-field-values](../../protocol/arp/checks/reply.md#reply-field-values) | Rfc826AddressResolution.test, Rfc826ReplyFields.test | PASS for a host; proxy ARP is the exception and it is level 5 |
| [RFC826-TABLE-3](../../standard/rfc826/catalog.md#rfc826-table-3) | no check | — | — | overridden by RFC1122-ACACHE-1 and ACACHE-2, which have a check |
| [RFC826-GEN-1](../../standard/rfc826/catalog.md#rfc826-gen-1) | no check | — | — | the in-scope set has one hardware type, and the model has no field for another one; see gap 2 |
| [RFC1122-ACACHE-1](../../standard/rfc1122/catalog.md#rfc1122-acache-1) | selected | [the-cache-flush](../../protocol/arp/checks/resolution.md#the-cache-flush) | Rfc1122CacheFlush.test | PASS. The mechanism is a lifetime test at the moment of use, not a periodic sweep |
| [RFC1122-ACACHE-2](../../standard/rfc1122/catalog.md#rfc1122-acache-2) | selected | [the-cache-flush](../../protocol/arp/checks/resolution.md#the-cache-flush) | Rfc1122CacheFlush.test | PASS. The scenario named the value, which is the check |
| [RFC1122-AFLOOD-1](../../standard/rfc1122/catalog.md#rfc1122-aflood-1) | selected | [the-request-rate](../../protocol/arp/checks/flood-prevention.md#the-request-rate) | Rfc1122ArpFloodPrevention.test | PASS. 10 requests in 10 s, one per second, the recommended rate exactly |
| [RFC1122-AQUEUE-1](../../standard/rfc1122/catalog.md#rfc1122-aqueue-1) | selected | [the-waiting-datagram](../../protocol/arp/checks/queue.md#the-waiting-datagram) | Rfc1122ArpPacketQueue.test | PASS, and more than the requirement asks: every datagram is saved, not only the latest |
| [RFC1122-AUSE-1](../../standard/rfc1122/catalog.md#rfc1122-ause-1) | selected | [address-resolution](../../protocol/arp/checks/resolution.md#address-resolution) | Rfc826AddressResolution.test | PASS |
| [RFC1122-ANOERR-1](../../standard/rfc1122/catalog.md#rfc1122-anoerr-1) | selected | [no-destination-unreachable](../../protocol/arp/checks/no-error-report.md#no-destination-unreachable) | Rfc1122NoDestinationUnreachable.test | PASS. No ICMP message of any type in 10 s |
| [RFC5494-NUM-1](../../standard/rfc5494/catalog.md#rfc5494-num-1) | covered | [packet-layout-on-the-wire](../../protocol/arp/checks/packet-format.md#packet-layout-on-the-wire) | Rfc826PacketLayout.test | PASS. Neither reserved value appears in either field |
| [RFC5494-NUM-2](../../standard/rfc5494/catalog.md#rfc5494-num-2) | selected | [an-experimental-hardware-space](../../protocol/arp/checks/input-validation.md#an-experimental-hardware-space) | Rfc5494ExperimentalHardwareSpace.test | **FAIL**, gap 2. The value did its work: it gave the check a defined input |
| [RFC5494-NUM-3](../../standard/rfc5494/catalog.md#rfc5494-num-3) | selected | [an-experimental-opcode](../../protocol/arp/checks/input-validation.md#an-experimental-opcode) | Rfc5494ExperimentalOpcode.test | **FAIL**, gap 1 |
| [RFC5494-NUM-4](../../standard/rfc5494/catalog.md#rfc5494-num-4) | covered | [packet-layout-on-the-wire](../../protocol/arp/checks/packet-format.md#packet-layout-on-the-wire) | Rfc826PacketLayout.test | PASS. The field holds 0x0800, the Ethertype of IPv4 |
| [RFC5494-PROC-1](../../standard/rfc5494/catalog.md#rfc5494-proc-1) | no check | — | — | the statement binds IANA; no behaviour of a host follows from it |

39 entries: 28 from RFC 826, 6 from the ARP part of RFC 1122, and 5 from RFC 5494. 35
reached a test: 32 with a PASS and 3 with a declared FAIL that names a model gap. Four carry
`no check`, each with its reason, and none of the four is mandatory: one is a `may` about a
field the standard calls meaningless, one is overridden, one is a `should` about hardware
that the in-scope set does not have, and one binds IANA.

## Feature support

The rule of step 7: `supported` when every core check ran and passed, `partial` when a core
check passed and another core check failed as a model gap or did not run, `not supported`
when every core check that ran failed as a model gap, `untested` when no core check ran.

| Feature | Core checks and their state | Support |
| --- | --- | --- |
| [ARP-F-RESOLUTION](../../protocol/arp/features.md#arp-f-resolution) | RFC826-REQ-1, REQ-4, REQ-6 and RFC1122-AUSE-1 all PASS | **supported** |
| [ARP-F-REPLY](../../protocol/arp/features.md#arp-f-reply) | RFC826-RECV-8 and RECV-9 PASS | **supported** |
| [ARP-F-CACHE](../../protocol/arp/features.md#arp-f-cache) | RFC826-REQ-2, RECV-4, RECV-6, RECV-7, TABLE-1 and RFC1122-ACACHE-1 all PASS | **supported** |
| [ARP-F-INPUT-VALIDATION](../../protocol/arp/features.md#arp-f-input-validation) | RFC826-RECV-5 PASS, RECV-10 PASS for a reply and FAIL for any other opcode, RECV-2 FAIL, RECV-3 FAIL | **partial** |
| [ARP-F-PACKET-FORMAT](../../protocol/arp/features.md#arp-f-packet-format) | RFC826-FMT-1, FMT-2, FMT-4 and RFC5494-NUM-1, NUM-4 all PASS | **supported** |
| [ARP-F-QUEUE](../../protocol/arp/features.md#arp-f-queue) | RFC1122-AQUEUE-1 PASS | **supported** |
| [ARP-F-FLOOD-PREVENTION](../../protocol/arp/features.md#arp-f-flood-prevention) | RFC1122-AFLOOD-1 PASS | **supported** |
| [ARP-F-NO-ERROR-REPORT](../../protocol/arp/features.md#arp-f-no-error-report) | RFC1122-ANOERR-1 PASS | **supported** |
| [ARP-F-GENERALIZATION](../../protocol/arp/features.md#arp-f-generalization) | RFC826-GEN-1 has no check | **untested** |

Nine features: seven supported, one partial, one untested. The model does the normal
exchange of ARP well, and it follows the reception algorithm of RFC 826 step by step,
including the one step a reader is most likely to get wrong: the merge before the opcode.
Two of the three things RFC 1122 adds are not only met but exceeded — every waiting datagram
is saved, not only the latest, and the request rate lands exactly on the recommended one per
second.

The one partial feature is input validation, and the reason is one shape repeated three
times: RFC 826 wants a packet the receiver cannot use to be discarded, and the model stops
the run. Two of the four questions of the reception algorithm are answered that way
(RFC826-RECV-2 and RECV-3, one gap), and so is the opcode question for a value that is
neither 1 nor 2 (RFC826-RECV-10, a second gap). The two questions that a normal exchange
reaches — the target address, and a reply — are answered correctly.

`ARP-F-GENERALIZATION` is `untested` and not `not supported`. Its one core statement is a
`should` about hardware other than the Ethernet, and the model has no field for a second
hardware type, so no check of two nodes on one Ethernet link can reach it. No verdict is
claimed where no check ran.

Two bounds that this pass records for the next one:

1. `RFC826-RECV-9` confirms "direct, and not broadcast" but not "on the same hardware". One
   link cannot separate the interface from the link. A host with two interfaces would.
2. `RFC826-RECV-5` and `RFC826-TABLE-2` hold for a host. Proxy ARP is on by default in the
   model, so a node that can route answers for an address that is not its own. No check of
   this pass meets it, because a host with one interface has no other interface to route out
   of. The standards map files RFC 1027 at level 5.

## Achieved level

**Level 3, reached.** Target: level 3, from
[`standards.md`](../../protocol/arp/standards.md#target-level). Three model gaps found.

The exit criterion of level 3 is that the catalogs hold every mandatory statement of the
in-scope documents, including each MUST NOT, and that each one has a check.

**Hold** — the catalogs hold every checkable statement of RFC 826, all six statements of the
three in-scope clauses of RFC 1122, and all five of RFC 5494. The mandatory statements of the
set are six: the one `must` of RFC 826 that binds a sender (RFC826-REQ-1), the three MUSTs of
RFC 1122 (ACACHE-1, AFLOOD-1, AUSE-1), and its one MUST NOT (ANOERR-1); RFC 5494 has none.
Every one of the six has a check that ran, and all six passed.

**Run** — 16 tests, 13 PASS and 3 declared FAIL. The three failures are model gaps against
statements whose strength is `description`, which is the strength of nearly every sentence
of RFC 826. They do not hold the level back: the criterion asks that each mandatory statement
have a check, and each one does. They are the findings of the pass.

A level reached with three gaps is the normal outcome and not a contradiction. The level
measures the reach of the catalogs and of the checks; the gaps measure the model.

| Level | State | Evidence |
| --- | --- | --- |
| 1, Survey | **reached** | The standards map pins the in-scope set from the RFC-editor register and not from the model's claim; [`conformance.md`](conformance.md) maps the claim onto it and names what the claim lacks. |
| 2, Core | **reached** | Every normal-path mandatory mechanism of RFC 826 is a feature, and every mandatory feature has a core check that ran and has a verdict. Ten checks need observation alone. |
| 3, Edge | **reached** | Every mandatory statement of the in-scope set has a check that ran and passed. Six checks craft a packet no program can send, and three of them found a gap. |
| 4, Dynamics | not started | RFC 5227, the address conflict detection: five time constants and a defence rate. The retry timer and the cache lifetime also need a tolerance there. |
| 5, Complete | not started | RFC 1027 and proxy ARP, RFC 903 and the reverse protocol, RFC 1868, and the other hardware types of RFC826-GEN-1. |

## Pass log

| Pass | Date | Level | Scope | Result |
| --- | --- | --- | --- | --- |
| 1 | 2026-09-11 | **3, reached** | The first ARP pass, and it did levels 1, 2 and 3 together. RFC 826, three clauses of RFC 1122 and RFC 5494 in scope; two new catalogs with 33 entries and 6 entries added to the shared RFC 1122 catalog; 9 features; 16 checks; 16 tests, one of which replaced a demo test that named no catalog entry | 16 tests: 13 PASS and 3 declared FAIL naming two model gaps in three tests; 7 features supported, 1 partial, 1 untested; 4 statements carry `no check` with a reason; see [`results.md`](results.md) |

## Out of scope

What this pass left out: RFC 5227, which is level 4 because every requirement of its §2
carries one of five time constants; RFC 1027 and proxy ARP, RFC 903 and RFC 1868, which are
level 5; the parts of RFC 1122 §2 that are not about address translation, which belong to a
pass on the Ethernet link layer; and the other hardware types of RFC826-GEN-1, which the
model has no field for.

One thing this pass found and did not test, and the next pass should: `Arp::processArpPacket`
throws on a packet whose sender protocol address is zero, which is the shape RFC 5227 calls
an ARP Probe. That is the same gap as gap 1 and gap 2, from a third place in the same method,
and its document is the one held back to level 4. The details are in
[`results.md`](results.md#sharpening-candidates-for-the-next-pass).
