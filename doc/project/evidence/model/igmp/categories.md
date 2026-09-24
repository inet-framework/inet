# IGMP checks — category decisions

> **Kind:** decision · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [results.md](results.md), [test-anatomy.md](../../../design/test-anatomy.md)

Step 9 artifact of the standards test workflow. For each check, this document records the
category the check belongs to, confirmed after the run, and the reason. The categories and
what each can establish are in [test-anatomy.md](../../../design/test-anatomy.md#the-categories);
the rule that the category must match the claim is
[TR-CAT-MATCH](../../../rule/testing.md#tr-cat-match).

## The decision, per check

All 32 checks of this pass are **protocol tests**, and their 42 tests are all in
`tests/protocol/igmp/`. For the classes `wire` and `end-to-end`, the prediction that step 3
made from the observation class held. For the class `internal`, it did not: the usual category
of a state without a signal is a module test, and 26 checks hold an `internal` statement. IGMP
exists to move state between nodes, so its state shows on the link: the interface state of a
host in its Reports, the retransmission state in which Reports hold a source, the router state
in the Queries and in the forwarded datagrams, and the timers in the instants of both. A check
reads the state through that effect, and a protocol test is then the right category: it
observes the behavior that the standard exists for, and the state has no other use. The
statements whose state has no effect on the link are the exception, below.

| Check | Observation class of its statements | Category | Reason |
| --- | --- | --- | --- |
| [Membership Report encapsulation](../../protocol/igmp/checks/message-format.md#membership-report-encapsulation) | wire, encoding | protocol test | Header, fields, length and checksum of one message on a link; the checksum from the octets that the serializer writes. |
| [Membership Query encapsulation](../../protocol/igmp/checks/message-format.md#membership-query-encapsulation) | wire, internal, encoding | protocol test | Header, fields, length and checksum of one message on a link. |
| [Router Alert and precedence on every message](../../protocol/igmp/checks/message-format.md#router-alert-and-precedence-on-every-message) | wire | protocol test | An IPv4 option and a header field of every IGMP message on a link. |
| [Timer relations in the Query](../../protocol/igmp/checks/message-format.md#timer-relations-in-the-query) | internal | protocol test | Two fields of one Query, and the interval between two Queries, which shows the Query Interval of the router. |
| [Join reported at once](../../protocol/igmp/checks/host-reports.md#join-reported-at-once) | wire, end-to-end, internal | protocol test | The instant and the records of one Report after a local request. |
| [Join repeated](../../protocol/igmp/checks/host-reports.md#join-repeated) | wire, internal | protocol test | The count and the spacing of Reports, against a bound; the retransmission state shows in the Reports. |
| [Leave reported](../../protocol/igmp/checks/host-reports.md#leave-reported) | wire | protocol test | The records of the Reports after a local request. |
| [Source list change](../../protocol/igmp/checks/host-reports.md#source-list-change) | wire | protocol test | The records of the Reports after two local requests. |
| [Change inside EXCLUDE mode](../../protocol/igmp/checks/host-reports.md#change-inside-exclude-mode) | wire | protocol test | The records of the Reports after two local requests. |
| [A change during the repetitions](../../protocol/igmp/checks/host-reports.md#a-change-during-the-repetitions) | wire, internal | protocol test | The records of successive Reports; the retransmission state of each source shows in which Report holds it. |
| [Response to a General Query](../../protocol/igmp/checks/query-response.md#response-to-a-general-query) | wire, end-to-end, internal | protocol test | The delay and the records of an answer; the interface state shows in the records. |
| [Response to a Group-Specific Query](../../protocol/igmp/checks/query-response.md#response-to-a-group-specific-query) | wire, internal | protocol test | The delay and the records of an answer, and the absence of an answer. |
| [Response to a Group-and-Source-Specific Query](../../protocol/igmp/checks/query-response.md#response-to-a-group-and-source-specific-query) | wire, internal | protocol test | The records of an answer; the recorded source list shows in the answer. |
| [General Queries at startup and after it](../../protocol/igmp/checks/router-queries.md#general-queries-at-startup-and-after-it) | wire, internal | protocol test | Instants of consecutive Queries, against the figures of the standard. |
| [Querier election](../../protocol/igmp/checks/router-queries.md#querier-election) | wire, end-to-end, internal | protocol test | The Queries of two routers over time; the querier state shows in which router sends. |
| [Forwarding after a join](../../protocol/igmp/checks/router-state.md#forwarding-after-a-join) | end-to-end, internal | protocol test | The arrival of forwarded datagrams on the link; the router state shows in the forwarding. |
| [Source-specific forwarding](../../protocol/igmp/checks/router-state.md#source-specific-forwarding) | internal | protocol test | Which source of a group arrives on the link; the source records show in the forwarding. |
| [Membership timeout without a leave](../../protocol/igmp/checks/router-state.md#membership-timeout-without-a-leave) | internal | protocol test | The instant at which forwarded datagrams stop; the Group Timer shows in the forwarding. |
| [Leave and the last member query](../../protocol/igmp/checks/router-state.md#leave-and-the-last-member-query) | wire, internal | protocol test | The Queries after a leave and the instant at which the forwarding stops. |
| [Leave with another member](../../protocol/igmp/checks/router-state.md#leave-with-another-member) | internal | protocol test | The continuity of the forwarding after a leave. |
| [Source blocked while another member wants it](../../protocol/igmp/checks/router-state.md#source-blocked-while-another-member-wants-it) | wire, internal | protocol test | The Queries after a BLOCK record, their sources and flags, and the continuity of the forwarding. |
| [Source blocked by its only member](../../protocol/igmp/checks/router-state.md#source-blocked-by-its-only-member) | wire, internal | protocol test | The Query after a BLOCK record and the instant at which the forwarding of the source stops. |
| [A router that comes up after a join](../../protocol/igmp/checks/router-state.md#a-router-that-comes-up-after-a-join) | internal | protocol test | The forwarding after an answer to the first Query that the router sends on the link. |
| [Host in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#host-in-igmpv2-mode) | wire, end-to-end, internal | protocol test | The version, the fields and the timing of the messages of a host after an older Query. |
| [Report suppression in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#report-suppression-in-igmpv2-mode) | end-to-end, internal | protocol test | The number of answers of two hosts to one Query. |
| [Leave in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#leave-in-igmpv2-mode) | wire, end-to-end, internal | protocol test | The type, the length and the destination of one message. |
| [Leave at an IGMPv2 router](../../protocol/igmp/checks/compatibility.md#leave-at-an-igmpv2-router) | wire, end-to-end, internal | protocol test | The Queries after a Leave and the instant at which the forwarding stops. |
| [Host back to IGMPv3](../../protocol/igmp/checks/compatibility.md#host-back-to-igmpv3) | internal | protocol test | The version of the Reports of a host at two instants. |
| [Router with an IGMPv2 member](../../protocol/igmp/checks/compatibility.md#router-with-an-igmpv2-member) | end-to-end, internal | protocol test | The forwarding and the Queries around the membership of an older host. |
| [Querier with an IGMPv2 router](../../protocol/igmp/checks/compatibility.md#querier-with-an-igmpv2-router) | wire, internal | protocol test | The length and the Max Resp Code of the Queries after an older Query. |
| [A mode change cancels the pending reports](../../protocol/igmp/checks/compatibility.md#a-mode-change-cancels-the-pending-reports) | internal | protocol test | The absence of a Version 3 Report after an older Query. |
| [A BLOCK record for a group in IGMPv2 mode](../../protocol/igmp/checks/compatibility.md#a-block-record-for-a-group-in-igmpv2-mode) | end-to-end | protocol test | The absence of a Query after a BLOCK record, and the continuity of the forwarding. |

### The checks with a time window, and why they are not statistical tests

Most checks compare instants: a Report within 10 ms of its request, a repetition within 1 s, an
answer within the Max Response Time and not within 1 ms of the Query, Queries 31.25 s and 125 s
apart, a querier that takes over 255 s after the last Query of the other router, a forwarding
that stops within the Last Member Query Time or the Group Membership Interval. Each one tests a
bound that the standard gives as a figure, and one run can break a bound. None of them tests the
shape of a distribution; the uniform delay of an answer and the uniform interval of a
repetition have a statistical half that no check reads.

The margins are stated in the checks: 0.1 s for an interval of the standard, which has no random
part; 1 ms for "at the instant of the Query"; and up to 1 s where a check reads the end of the
forwarding, because the source sends one datagram every 0.5 s.

### The checks that read the octets

The checksum of a Report and of a Query is the one field that the checks read from the octets:
`igmpChecksumOk` of [`IgmpChecks.h`](../../../../../tests/protocol/igmp/IgmpChecks.h) serializes
the IGMP part and sums it. Every other field is read from the message as it holds its fields.
The whole bit layout, the floating-point codes and the deserializer are a serializer test
(below).

### The test modules of the mockups

Two modules of [`IgmpChecks.h`](../../../../../tests/protocol/igmp/IgmpChecks.h) stand in for
parts that are outside IGMP. `IgmpRequests` plays the sockets of a host: it makes the calls of
the service interface of RFC 9776 §2 at fixed instants. `MulticastLeafRoute` plays the multicast
routing protocol of the router: one route from L2 to L1, with L1 as a leaf. Neither module
changes a behavior of IGMP, and the tests stay protocol tests. The `ScenarioManager` of the
tests breaks and restores a link, which plays a node that leaves or joins L1.

## The statements that belong in another suite

| Statements | Category | Why |
| --- | --- | --- |
| RFC9776-QRY-3, QRY-4, QRY-9, QRY-16, QRY-17, REP-6, REP-9, TIMER-14, TIMER-15 | serializer unit test | the `encoding` entries: the exact bit layout and the floating-point codes; the protocol tests read the values that the codes hold, and the checksum from the octets |
| RFC9776-HOST-2, RQ-3, RQ-4 | a test with state signals, level 4 | state inside a node that no message shows |
| RFC9776-COMPR-7, COMPR-9, COMPR-10 | a test with state signals, level 4 | a warning or a log entry, which no link carries |
| the random halves of RFC9776-HQRY-1, HOST-18 and RFC2236-HOST-21 | statistical test | the distribution of a random delay or interval |

## A category this pass did not need

No check needs a module test: every checked behavior shows in a message on a link or in a
forwarded datagram, also the state of the class `internal`. No check needs a fingerprint test:
the pass locks behaviors to the standard, not trajectories to a reference run.
