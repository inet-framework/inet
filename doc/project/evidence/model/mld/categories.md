# MLD checks — category decisions

> **Kind:** decision · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [results.md](results.md), [test-anatomy.md](../../../design/test-anatomy.md)

Step 9 artifact of the standards test workflow. For each check, this document records the
category the check belongs to, confirmed after the run, and the reason. The categories and
what each can establish are in [test-anatomy.md](../../../design/test-anatomy.md#the-categories);
the rule that the category must match the claim is
[TR-CAT-MATCH](../../../rule/testing.md#tr-cat-match).

## The decision, per check

All 32 checks of this pass are **protocol tests**, and their 47 tests are all in
`tests/protocol/mld/`. The decision is the one of the IGMP twin,
[`igmp/categories.md`](../igmp/categories.md): for the classes `wire` and `end-to-end`, the
prediction of step 3 held; for the class `internal`, which 24 checks hold, it did not,
because MLD state shows on the link — in the Reports, the Queries and the forwarded datagrams —
and a check reads it there.

| Check | Observation class of its statements | Category | Reason |
| --- | --- | --- | --- |
| [Multicast Listener Report encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-report-encapsulation) | wire, encoding | protocol test | Header, fields, length and checksum of one message on a link; the checksum from the octets that the serializer writes, with the pseudo-header. |
| [Multicast Listener Query encapsulation](../../protocol/mld/checks/message-format.md#multicast-listener-query-encapsulation) | wire, internal, encoding | protocol test | Header, fields, length and checksum of one message on a link. |
| [Router Alert on every message](../../protocol/mld/checks/message-format.md#router-alert-on-every-message) | wire | protocol test | An option of the Hop-by-Hop Options header, the Hop Limit and the source address of every MLD message on a link. |
| [Timer relations in the Query](../../protocol/mld/checks/message-format.md#timer-relations-in-the-query) | internal | protocol test | Two fields of one Query, and the interval between two Queries, which shows the Query Interval of the router. |
| [Start of listening reported at once](../../protocol/mld/checks/listener-reports.md#start-of-listening-reported-at-once) | wire, end-to-end, internal | protocol test | The instant and the records of one Report after a local request, and the addresses that the Reports of a node name. |
| [Start of listening repeated](../../protocol/mld/checks/listener-reports.md#start-of-listening-repeated) | wire, internal | protocol test | The count and the spacing of Reports, against a bound; the retransmission state shows in the Reports. |
| [Stop of listening reported](../../protocol/mld/checks/listener-reports.md#stop-of-listening-reported) | wire | protocol test | The records of the Reports after a local request. |
| [Source list change](../../protocol/mld/checks/listener-reports.md#source-list-change) | wire | protocol test | The records of the Reports after two local requests. |
| [Change inside EXCLUDE mode](../../protocol/mld/checks/listener-reports.md#change-inside-exclude-mode) | wire | protocol test | The records of the Reports after two local requests. |
| [A change during the repetitions](../../protocol/mld/checks/listener-reports.md#a-change-during-the-repetitions) | wire, internal | protocol test | The records of successive Reports; the retransmission state of each source shows in which Report holds it. |
| [Response to a General Query](../../protocol/mld/checks/query-response.md#response-to-a-general-query) | wire, internal | protocol test | The delay and the records of an answer; the listening state shows in the records. |
| [Response to a Multicast Address Specific Query](../../protocol/mld/checks/query-response.md#response-to-a-multicast-address-specific-query) | wire | protocol test | The delay and the records of an answer, and the absence of an answer. |
| [Response to a Multicast Address and Source Specific Query](../../protocol/mld/checks/query-response.md#response-to-a-multicast-address-and-source-specific-query) | wire, internal | protocol test | The records of an answer, and the absence of an answer; the recorded source list shows in the answer. |
| [General Queries at startup and after it](../../protocol/mld/checks/router-queries.md#general-queries-at-startup-and-after-it) | wire, internal | protocol test | Instants of consecutive Queries, against the figures of the standard. |
| [Querier election](../../protocol/mld/checks/router-queries.md#querier-election) | wire, internal | protocol test | The Queries of two routers over time; the querier state shows in which router sends. |
| [Forwarding after a start of listening](../../protocol/mld/checks/router-state.md#forwarding-after-a-start-of-listening) | wire, end-to-end, internal | protocol test | The arrival of forwarded datagrams on the link; the router state shows in the forwarding. |
| [Source-specific forwarding](../../protocol/mld/checks/router-state.md#source-specific-forwarding) | end-to-end, internal | protocol test | Which source of an address arrives on the link; the source records show in the forwarding. |
| [Listening timeout without a stop](../../protocol/mld/checks/router-state.md#listening-timeout-without-a-stop) | internal | protocol test | The instant at which forwarded datagrams stop; the Filter Timer shows in the forwarding. |
| [Stop of listening and the last listener query](../../protocol/mld/checks/router-state.md#stop-of-listening-and-the-last-listener-query) | wire, end-to-end, internal, encoding | protocol test | The Queries after a stop and the instant at which the forwarding stops. |
| [Stop of listening with another listener](../../protocol/mld/checks/router-state.md#stop-of-listening-with-another-listener) | end-to-end, internal | protocol test | The continuity of the forwarding after a stop. |
| [Source blocked while another listener wants it](../../protocol/mld/checks/router-state.md#source-blocked-while-another-listener-wants-it) | wire, end-to-end, internal | protocol test | The Queries after a BLOCK record, their sources and flags, and the continuity of the forwarding. |
| [Source blocked by its only listener](../../protocol/mld/checks/router-state.md#source-blocked-by-its-only-listener) | end-to-end, internal | protocol test | The Query after a BLOCK record and the instant at which the forwarding of the source stops. |
| [A router that comes up after a start of listening](../../protocol/mld/checks/router-state.md#a-router-that-comes-up-after-a-start-of-listening) | internal | protocol test | The forwarding after an answer to the first Query that the router sends on the link. |
| [Listener in MLDv1 mode](../../protocol/mld/checks/compatibility.md#listener-in-mldv1-mode) | wire, end-to-end, internal, encoding | protocol test | The version, the fields and the timing of the messages of a host after an older Query. |
| [Report suppression in MLDv1 mode](../../protocol/mld/checks/compatibility.md#report-suppression-in-mldv1-mode) | end-to-end, internal | protocol test | The number of answers of two hosts to one Query. |
| [Done in MLDv1 mode](../../protocol/mld/checks/compatibility.md#done-in-mldv1-mode) | wire | protocol test | The type, the length and the destination of one message. |
| [Done at an MLDv1 router](../../protocol/mld/checks/compatibility.md#done-at-an-mldv1-router) | wire, internal | protocol test | The Queries after a Done and the instant at which the forwarding stops. |
| [Listener back to MLDv2](../../protocol/mld/checks/compatibility.md#listener-back-to-mldv2) | internal | protocol test | The version of the Reports of a host at three instants. |
| [Router with an MLDv1 listener](../../protocol/mld/checks/compatibility.md#router-with-an-mldv1-listener) | wire, end-to-end, internal | protocol test | The forwarding and the Queries around the listening of an older host. |
| [Querier configured for an MLDv1 router](../../protocol/mld/checks/compatibility.md#querier-configured-for-an-mldv1-router) | wire, internal | protocol test | The length and the Maximum Response Code of the Queries of a configured router. |
| [A mode change cancels the pending reports](../../protocol/mld/checks/compatibility.md#a-mode-change-cancels-the-pending-reports) | internal | protocol test | The absence of a Version 2 Report after an older Query. |
| [A BLOCK record for an address in MLDv1 mode](../../protocol/mld/checks/compatibility.md#a-block-record-for-an-address-in-mldv1-mode) | end-to-end | protocol test | The absence of a Query after a BLOCK record, and the continuity of the forwarding. |

### The checks with a time window, and why they are not statistical tests

Most checks compare instants against bounds that the standard gives as figures: a Report within
10 ms of its request, a repetition within 1 s, an answer within the Maximum Response Delay and
not within 1 ms of the Query, Queries 31.25 s and 125 s apart, a querier that takes over 255 s
after the last Query of the other router, a forwarding that stops within the Last Listener Query
Time or the Multicast Address Listening Interval, and a host that leaves MLDv1 mode between 290
and 293 seconds. One run can break each bound; none of them tests the shape of a distribution.

### The checks that read the octets

The checksum of a Report and of a Query is the one field that the checks read from the octets:
`mldChecksumOk` of [`MldChecks.h`](../../../../../tests/protocol/mld/MldChecks.h) serializes the
MLD message and sums it with the pseudo-header of RFC 8200 §8.1. The Hop-by-Hop Options header
and its Router Alert option are read from the chunk of the extension header. Every other field
is read from the message as it holds its fields. The whole bit layout, the floating-point codes
and the deserializer are a serializer test (below).

### The test modules of the mockups

Two modules of [`MldChecks.h`](../../../../../tests/protocol/mld/MldChecks.h) stand in for parts
that are outside MLD. `MldRequests` plays the sockets of a host: it makes the calls of the
service interface of RFC 9777 §3 at fixed instants. `MulticastLeafRoute6` plays the multicast
routing protocol of the router: one route from L2 to L1, with L1 as a leaf. Neither module
changes a behavior of MLD, and the tests stay protocol tests. The `ScenarioManager` of the tests
breaks and restores a link, which plays a node that leaves or joins L1.

## The statements that belong in another suite

| Statements | Category | Why |
| --- | --- | --- |
| RFC9777-QRY-4, QRY-5, QRY-7, QRY-15, QRY-19, REP-4, REP-5, REP-8, REP-10, REP-13, REP-14, VER-1, TIMER-12 | serializer unit test | the `encoding` entries: the exact bit layout and the floating-point codes; the protocol tests read the values that the codes hold, and the checksum from the octets |
| RFC9777-RQ-1, RQ-5, RQ-6, RST-10, RST-16, RST-19, RFC2710-TIMER-4 | a test with state signals, level 4 | state inside a node that no message shows |
| RFC9777-COMPR-6, COMPR-8, COMPR-9 | a test with state signals, level 4 | a warning or a log entry, which no link carries |
| the random halves of RFC9777-LQRY-2, LSN-13 and RFC2710-NODE-13, NODE-14 | statistical test | the distribution of a random delay or interval |
| RFC9777-QRY-4, REP-4, as far as the checksum is concerned | the ICMPv6 suite, besides this one | the pseudo-header is the one of every ICMPv6 message, RFC 4443 §2.3; this pass checks it on MLD messages because RFC 9777 names it, and the gap reaches Neighbor Discovery too |

## A category this pass did not need

No check needs a module test: every checked behavior shows in a message on a link or in a
forwarded datagram, also the state of the class `internal`. No check needs a fingerprint test:
the pass locks behaviors to the standard, not trajectories to a reference run.
