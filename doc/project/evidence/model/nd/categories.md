# ND checks — category decisions

> **Kind:** decision · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [results.md](results.md), [test-anatomy.md](../../../design/test-anatomy.md)

Step 9 artifact of the standards test workflow. For each check, this document records the
category the check belongs to, confirmed after the run, and the reason. The categories and
what each can establish are in [test-anatomy.md](../../../design/test-anatomy.md#the-categories);
the rule that the category must match the claim is
[TR-CAT-MATCH](../../../rule/testing.md#tr-cat-match).

## The decision, per check

All 34 checks of this pass are **protocol tests**, and their 37 tests are all in
`tests/protocol/nd/`. The prediction that step 3 made from the observation class held for every
one of them: Neighbor Discovery is a protocol of messages between neighbors, so `wire` dominates
the catalogs, and a `wire` or `end-to-end` statement maps to a protocol test.

| Check | Observation class of its statements | Category | Reason |
| --- | --- | --- | --- |
| [Router Solicitation of a host that comes up](../../protocol/nd/checks/router-discovery.md#router-solicitation-of-a-host-that-comes-up) | wire | protocol test | Addresses, option and header of one message on a link, and the absence of later ones. |
| [Router Solicitations on a link without a router](../../protocol/nd/checks/router-discovery.md#router-solicitations-on-a-link-without-a-router) | wire, end-to-end | protocol test | Count and spacing of messages on a link, against the bounds of the standard. |
| [Router Advertisement header and addressing](../../protocol/nd/checks/router-discovery.md#router-advertisement-header-and-addressing) | wire | protocol test | Addresses and header of one message on a link, and the absence of messages from the hosts. |
| [Router Advertisement fields](../../protocol/nd/checks/router-discovery.md#router-advertisement-fields) | wire, end-to-end | protocol test | Fields and options of one message, as the message holds them and as the serializer writes the options. |
| [Prefix Information option](../../protocol/nd/checks/router-discovery.md#prefix-information-option) | wire | protocol test | Fields of one option, with its Reserved fields in the octets that the serializer writes. |
| [Router Advertisement in answer to a solicitation](../../protocol/nd/checks/router-discovery.md#router-advertisement-in-answer-to-a-solicitation) | wire | protocol test | The instant of an answer against the instant of its solicitation, with a bound of 3.5 s. |
| [Unsolicited Router Advertisements](../../protocol/nd/checks/router-discovery.md#unsolicited-router-advertisements) | wire | protocol test | Instants of consecutive messages on one link, against the bounds that the standard gives. |
| [The default router](../../protocol/nd/checks/router-discovery.md#the-default-router) | end-to-end | protocol test | The link-layer destination of a forwarded packet after one advertisement. |
| [A router with Router Lifetime zero](../../protocol/nd/checks/router-discovery.md#a-router-with-router-lifetime-zero) | end-to-end | protocol test | The link-layer destination of a packet after two advertisements. |
| [Hop limit from the router](../../protocol/nd/checks/parameters.md#hop-limit-from-the-router) | wire, end-to-end | protocol test | The hop limit of a packet that the host sends after an advertisement. |
| [Hop limit without a router](../../protocol/nd/checks/parameters.md#hop-limit-without-a-router) | wire, end-to-end | protocol test | The hop limit of a packet that the host sends with no router. |
| [Retransmission timer from the router](../../protocol/nd/checks/parameters.md#retransmission-timer-from-the-router) | wire | protocol test | Spacing of solicitations on a link after an advertisement, with a window of 2 to 3 s. |
| [MTU from the router](../../protocol/nd/checks/parameters.md#mtu-from-the-router) | end-to-end | protocol test | The sizes of the packets that a host sends after an advertisement. |
| [On-link neighbor reached directly](../../protocol/nd/checks/on-link.md#on-link-neighbor-reached-directly) | wire, end-to-end | protocol test | The solicitation and the link-layer destination of one packet. |
| [Prefix with the on-link flag clear](../../protocol/nd/checks/on-link.md#prefix-with-the-on-link-flag-clear) | wire, end-to-end | protocol test | The link-layer destination of one packet, and the absence of a solicitation before it. |
| [No router and no on-link prefix](../../protocol/nd/checks/on-link.md#no-router-and-no-on-link-prefix) | wire | protocol test | The absence of two kinds of message, against a control packet that leaves. |
| [Prefix after its valid lifetime](../../protocol/nd/checks/on-link.md#prefix-after-its-valid-lifetime) | wire, end-to-end | protocol test | The absence of two kinds of message after a lifetime that a stopped router set. |
| [Address resolution of a host](../../protocol/nd/checks/address-resolution.md#address-resolution-of-a-host) | wire, end-to-end | protocol test | A solicitation, its answer and the queued packet on one link. |
| [Neighbor Solicitation and Advertisement fields](../../protocol/nd/checks/address-resolution.md#neighbor-solicitation-and-advertisement-fields) | wire | protocol test | Header fields and flags of two messages, with the Reserved fields in the octets that the serializer writes. |
| [Address resolution of a router](../../protocol/nd/checks/address-resolution.md#address-resolution-of-a-router) | wire | protocol test | One flag of an answer on a link. |
| [Address resolution failure](../../protocol/nd/checks/address-resolution.md#address-resolution-failure) | wire, end-to-end, error-signal | protocol test | Count and spacing of solicitations, and the ICMPv6 error on the other link. |
| [Redirect from the first-hop router](../../protocol/nd/checks/redirect.md#redirect-from-the-first-hop-router) | wire | protocol test | Addresses of one message on a link after a forwarded packet. |
| [Redirect fields](../../protocol/nd/checks/redirect.md#redirect-fields) | wire | protocol test | Header and options of one message, in the octets that the serializer writes. |
| [Redirect of a large packet](../../protocol/nd/checks/redirect.md#redirect-of-a-large-packet) | wire | protocol test | The size of one message and the absence of a Fragment header. |
| [Host follows a Redirect](../../protocol/nd/checks/redirect.md#host-follows-a-redirect) | wire, end-to-end | protocol test | The link-layer destination of the packets after one message. |
| [Redirect to an on-link destination](../../protocol/nd/checks/redirect.md#redirect-to-an-on-link-destination) | wire, end-to-end | protocol test | Addresses of one message, and the link-layer destination of the packets after it. |
| [Link-local address and its Duplicate Address Detection](../../protocol/nd/checks/autoconfiguration.md#link-local-address-and-its-duplicate-address-detection) | wire, end-to-end, internal, encoding | protocol test | One solicitation, its option, and the absence of packets from the address for 1 s. |
| [Duplicate Address Detection of a router](../../protocol/nd/checks/autoconfiguration.md#duplicate-address-detection-of-a-router) | wire | protocol test | Two solicitations of a router, and the instant of its first advertisement. |
| [Global address and its Duplicate Address Detection](../../protocol/nd/checks/autoconfiguration.md#global-address-and-its-duplicate-address-detection) | wire, end-to-end | protocol test | One solicitation and the source address of a later packet. |
| [Duplicate link-local address](../../protocol/nd/checks/autoconfiguration.md#duplicate-link-local-address) | wire, end-to-end | protocol test | An answer on a link, and the silence of the node with the duplicate after it. |
| [Prefix with the Autonomous flag clear](../../protocol/nd/checks/autoconfiguration.md#prefix-with-the-autonomous-flag-clear) | end-to-end | protocol test | The absence of solicitations and of packets from the prefix. |
| [Address after its valid lifetime](../../protocol/nd/checks/autoconfiguration.md#address-after-its-valid-lifetime) | wire, internal | protocol test | The absence of packets from the prefix after a lifetime that a stopped router set. |
| [Groups joined before Duplicate Address Detection](../../protocol/nd/checks/multicast.md#groups-joined-before-duplicate-address-detection) | wire, end-to-end | protocol test | An MLD Report on the link before a solicitation. |
| [All-routers group of a router](../../protocol/nd/checks/multicast.md#all-routers-group-of-a-router) | end-to-end | protocol test | An MLD Report of the router on the link. |

The check of the link-local address targets two statements of class `internal` and one of
class `encoding`, and it reaches all three on the wire: the tentative address shows as the
absence of packets from it for 1 s, and the layout of the link-local address shows in the
target of the solicitation. The check of address resolution failure holds the one
`error-signal` statement, the Destination Unreachable that the router sends on the other link.

### The checks with a time window, and why they are not statistical tests

Five checks compare instants: the solicitations 4 s apart, the answer within 3.5 s, the
unsolicited advertisements at most 16 s apart and then 198 to 600 s apart, the retransmissions
2 to 3 s or 1 to 1.5 s apart, and the address tentative for 1 s. Each one tests a bound that the
standard gives as a figure, and one run can break a bound. None of them tests the shape of a
distribution; that half of RFC4861-HOST-47, ADV-22, ADV-32, HOST-6 and RFC4862-DAD-14 is a
statistical test, and the ledger records it as `later`.

The upper bound of a retransmission window reads "about every RetransTimer" as at most one and a
half times the timer. That is a reading of the text, and the checks say so in their notes.

### The checks that read the octets

Most field checks read the message as it holds its fields. The Reserved fields and the options
are the exception: the checks read them from the octets that the serializer writes, with
`ndBytesOf` of [`NdChecks.h`](../../../../../tests/protocol/nd/NdChecks.h), because the Redirect
of the model has no Reserved field and the model has no class for the Redirected Header option.
A wrong layout of those parts would show. The whole bit layout, and the deserializer, are a
serializer test (below).

### The management module of two tests

`Rfc4861RaFields` and `Rfc4861RetransTimerFromRouter` set AdvReachableTime and AdvRetransTimer
with `NdRouterVariables` of [`NdChecks.h`](../../../../../tests/protocol/nd/NdChecks.h), because
the model has no parameter for them. The module plays the system management of RFC 4861
§6.2.1; it changes no behavior of the model, and the two tests stay protocol tests.

## The statements that belong in another suite

| Statements | Category | Why |
| --- | --- | --- |
| RFC4861-RS-2, RA-2, NS-3, NA-2, RDM-2, OPT-14, OPT-37, OPT-44 and the other `encoding` entries | serializer unit test | the whole bit layout and its deserialization; the protocol tests read only the Reserved fields and the options from the octets |
| RFC4861-RS-8, RA-8, NS-9, NA-9, RDM-8 | the ICMPv6 suite | the checksum is the one of ICMPv6, RFC 4443, the same for every message type |
| the random halves of RFC4861-HOST-47, HOST-6, HOST-17, HOST-18, ADV-22, ADV-32, ANY-2, AR-40 and RFC4862-DAD-14, DAD-15 | statistical test | the distribution of a random delay or interval |
| the Neighbor Cache states and flags, and Neighbor Unreachability Detection | a test with state signals, level 4 | the state inside a node; Neighbor Unreachability Detection is RFC 4861 §7.3, outside the in-scope set of this level |

## A category this pass did not need

No check needs a module test: every checked behavior shows in a message on a link or in a
forwarded packet. No check needs a fingerprint test: the pass locks behaviors to the standard,
not trajectories to a reference run.
