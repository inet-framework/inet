# IPsec checks — category decisions

> **Kind:** decision · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [results.md](results.md), [test-anatomy.md](../../../design/test-anatomy.md)

Step 9 artifact of the standards test workflow. For each check, this document records the
category the check belongs to, confirmed after the run, and the reason. The categories and
what each can establish are in [test-anatomy.md](../../../design/test-anatomy.md#the-categories);
the rule that the category must match the claim is
[TR-CAT-MATCH](../../../rule/testing.md#tr-cat-match).

## The decision, per check

All 23 checks of this pass are **protocol tests**, and their 31 tests are all in
`tests/protocol/ipsec/`. For the classes `wire` and `end-to-end`, the prediction that step 3
made from the observation class held. For the class `internal`, it held too, for a reason of its
own: the SPD and the SAD are databases inside a host, but IPsec exists to decide what crosses the
boundary, so each entry shows in what leaves a host and in what its applications receive. A check
reads the database through that effect. The class `encoding` is read from the chunks of a frame,
because the model has no serializer for AH and ESP; the order and the lengths of the chunks are
the layout on the wire.

| Check | Observation class of its statements | Category | Reason |
| --- | --- | --- | --- |
| [Outbound dispositions](../../protocol/ipsec/checks/policy.md#outbound-dispositions) | end-to-end, wire, internal | protocol test | Which packets leave a host, protected, in clear or not at all, and which the receiver delivers. |
| [Inbound dispositions](../../protocol/ipsec/checks/policy.md#inbound-dispositions) | end-to-end, internal | protocol test | Which plain datagrams a host delivers, seen at its UDP layer. |
| [The first matching entry decides](../../protocol/ipsec/checks/policy.md#the-first-matching-entry-decides) | internal | protocol test | The order of the SPD shows in which of two flows leaves the host. |
| [Selectors](../../protocol/ipsec/checks/policy.md#selectors) | internal | protocol test | Each selector decides which packets of five kinds leave, and how. |
| [One SA for each direction](../../protocol/ipsec/checks/security-associations.md#one-sa-for-each-direction) | internal, wire | protocol test | The SPI of the packets in each direction. |
| [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | internal, end-to-end | protocol test | Which protected packets the receiver delivers, by their SPI and protocol. |
| [Inbound selector check](../../protocol/ipsec/checks/security-associations.md#inbound-selector-check) | end-to-end, internal | protocol test | Whether the receiver delivers a packet that its SA does not cover. |
| [A PROTECT entry without an SA](../../protocol/ipsec/checks/security-associations.md#a-protect-entry-without-an-sa) | end-to-end | protocol test | Whether a packet without an SA leaves the host. |
| [The lifetime of an SA](../../protocol/ipsec/checks/security-associations.md#the-lifetime-of-an-sa) | internal | protocol test | The end of an SA shows as the end of its packets on the link. |
| [Parallel SAs for classes of traffic](../../protocol/ipsec/checks/security-associations.md#parallel-sas-for-classes-of-traffic) | internal, end-to-end | protocol test | The SPI that each class of traffic carries on the link. |
| [Sequence numbers of an SA](../../protocol/ipsec/checks/sequence-numbers.md#sequence-numbers-of-an-sa) | wire, internal | protocol test | The Sequence Number field of consecutive packets of each SA. |
| [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | wire, encoding, end-to-end | protocol test | The order and lengths of the fields of one ESP packet, and the datagram that the receiver restores. |
| [ESP padding](../../protocol/ipsec/checks/esp.md#esp-padding) | encoding, end-to-end | protocol test | The padding arithmetic of two ciphers, read from the plaintext of the packet. |
| [ESP services](../../protocol/ipsec/checks/esp.md#esp-services) | wire, internal | protocol test | The IV and the ICV that each service puts on the link. |
| [An ESP SA without encryption and without integrity](../../protocol/ipsec/checks/esp.md#an-esp-sa-without-encryption-and-without-integrity) | internal | protocol test | The refusal of the configuration, or the absence of the SA on the link. |
| [Traffic flow confidentiality padding](../../protocol/ipsec/checks/esp.md#traffic-flow-confidentiality-padding) | wire, end-to-end | protocol test | The lengths of the packets on the link and of the datagrams that the receiver delivers. |
| [Dummy packets](../../protocol/ipsec/checks/esp.md#dummy-packets) | wire, end-to-end | protocol test | A packet with Next Header 59 on the link, and what the receiver delivers. |
| [AH format in transport mode](../../protocol/ipsec/checks/ah.md#ah-format-in-transport-mode) | wire, encoding, end-to-end | protocol test | The fields and the place of the ICV in one AH packet, on IPv4 and IPv6. |
| [AH across a router](../../protocol/ipsec/checks/ah.md#ah-across-a-router) | encoding, end-to-end | protocol test | A changed TTL on the way, and the delivery at the receiver. |
| [Tunnel mode between two hosts](../../protocol/ipsec/checks/modes.md#tunnel-mode-between-two-hosts) | wire, end-to-end | protocol test | The outer and the inner header of one packet. |
| [AH and ESP on one packet](../../protocol/ipsec/checks/modes.md#ah-and-esp-on-one-packet) | wire, end-to-end | protocol test | The order of the two IPsec headers in one packet. |
| [Fragments of a protected datagram](../../protocol/ipsec/checks/fragmentation.md#fragments-of-a-protected-datagram) | wire, end-to-end | protocol test | The fragments on two links, and the datagram that the receiver restores. |
| [Path MTU of a protected flow](../../protocol/ipsec/checks/fragmentation.md#path-mtu-of-a-protected-flow) | error-signal, wire, end-to-end | protocol test | The ICMPv6 report of the router and the lengths of the next packets. |

## The statements that belong in another suite

| Statements | Category | Why |
| --- | --- | --- |
| the structure of the SPD, its caches and the SAD (RFC4301-SPD-1, SPD-39, SPD-43, SPDE-22, SAD-2, SAD-7, SAD-8, SADI-12, OUT-4, DB-2, DB-3) | a module test or a test with state signals, level 4 | state inside a host with no effect on the link |
| every auditable event and its log entry (RFC4301-SA-19, FUNC-10 to FUNC-12, SADI-4, DISC-2, DISC-3, IN-18, IN-19, IN-26, IN-27, IN-34, IN-35, and the audit entries of RFC 4302 and RFC 4303) | a test with state signals, level 4 | a log entry, which no link carries |
| the transforms and the implicit padding of the algorithms (the `later` statements of level 5 in [`coverage.md`](coverage.md#statement-coverage)) | unit test | one algorithm on one message, when the model does cryptography |
| the random TFC padding length | statistical test | the distribution of a random length; the protocol test only checks that the lengths differ |

## A category this pass did not need

No check needs a module test: every checked behavior shows in a packet on a link or in a
datagram that a host delivers, also the SPD and SAD entries of the class `internal`. No check
needs a fingerprint test: the pass locks behaviors to the standard, not trajectories to a
reference run.
