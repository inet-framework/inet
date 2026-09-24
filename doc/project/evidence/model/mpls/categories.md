# MPLS checks — category decisions

> **Kind:** decision · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [results.md](results.md), [test-anatomy.md](../../../design/test-anatomy.md)

Step 9 artifact of the standards test workflow. For each check, this document records the
category the check belongs to, confirmed after the run, and the reason. The categories and
what each can establish are in [test-anatomy.md](../../../design/test-anatomy.md#the-categories);
the rule that the category must match the claim is
[TR-CAT-MATCH](../../../rule/testing.md#tr-cat-match).

## The decision, per check

All 15 checks of this pass are **protocol tests**, and their 21 tests are all in
`tests/protocol/mpls/`. For the classes `wire`, `encoding`, `end-to-end` and `error-signal`, the
prediction that step 3 made from the observation class held. For the class `internal`, it held
too, for a reason of its own: the ILM, the FTN and the NHLFE are tables inside an LSR, but each
entry decides which label a packet carries on the next link, or whether it carries one. A check
reads the table through that effect. The class `encoding` is read from the octets of each label
stack entry, which the helper of the tests serializes, so a field in the wrong bits fails a
check.

| Check | Observation class of its statements | Category | Reason |
| --- | --- | --- | --- |
| [The label stack entry](../../protocol/mpls/checks/encoding.md#the-label-stack-entry) | encoding, wire | protocol test | The octets of one entry and its place in the frame. |
| [A label stack of two entries](../../protocol/mpls/checks/encoding.md#a-label-stack-of-two-entries) | wire, internal | protocol test | The order and the S bits of two entries on the link, and the datagram after two pops. |
| [Label switching along an LSP](../../protocol/mpls/checks/forwarding.md#label-switching-along-an-lsp) | internal, end-to-end | protocol test | The label of one datagram on each link of the LSP, and its delivery. |
| [Penultimate hop popping](../../protocol/mpls/checks/forwarding.md#penultimate-hop-popping) | internal | protocol test | The unlabeled datagram on the last link but one. |
| [The IPv4 Explicit NULL label](../../protocol/mpls/checks/reserved-labels.md#the-ipv4-explicit-null-label) | wire | protocol test | The label 0 on one link, and the IPv4 datagram on the next. |
| [The Implicit NULL label](../../protocol/mpls/checks/reserved-labels.md#the-implicit-null-label) | internal | protocol test | The absence of the label 3 on the link, where an NHLFE names it. |
| [The TTL of the first label](../../protocol/mpls/checks/ttl.md#the-ttl-of-the-first-label) | wire | protocol test | The TTL of the entry and of the IPv4 header in one frame. |
| [The TTL at each LSR](../../protocol/mpls/checks/ttl.md#the-ttl-at-each-lsr) | encoding, internal, wire | protocol test | The TTL of one datagram on two links. |
| [The TTL after the last label](../../protocol/mpls/checks/ttl.md#the-ttl-after-the-last-label) | wire, internal | protocol test | The TTL of one datagram before and after the pop, and at the end of the path. |
| [A TTL that reaches zero](../../protocol/mpls/checks/ttl.md#a-ttl-that-reaches-zero) | end-to-end, wire | protocol test | Whether a datagram leaves the LSR whose outgoing TTL is zero. |
| [A labeled datagram too big for the next link](../../protocol/mpls/checks/fragmentation.md#a-labeled-datagram-too-big-for-the-next-link) | internal, wire | protocol test | The lengths and the labels of the packets on the link with the small MTU. |
| [A too-big labeled datagram that may not be fragmented](../../protocol/mpls/checks/fragmentation.md#a-too-big-labeled-datagram-that-may-not-be-fragmented) | end-to-end, error-signal | protocol test | The absence of the datagram on the next link, and the ICMP message at the source. |
| [Labeled packets on a PPP link](../../protocol/mpls/checks/links.md#labeled-packets-on-a-ppp-link) | wire, encoding | protocol test | The PPP Protocol field and the Information field of one frame. |
| [The MPLS Control Protocol](../../protocol/mpls/checks/links.md#the-mpls-control-protocol) | wire | protocol test | The order of the frames of two PPP protocols on the link. |
| [Labeled packets on an Ethernet link](../../protocol/mpls/checks/links.md#labeled-packets-on-an-ethernet-link) | wire | protocol test | The ethertype and the order of the headers of one frame. |

## The statements that belong in another suite

| Statements | Category | Why |
| --- | --- | --- |
| the timers of the MPLS Control Protocol (RFC3032-PPP-8, PPP-9) | a protocol test at level 4 | a wait that a check measures over time, with the negotiation that a later pass adds |
| the choice of exactly one NHLFE among several (RFC3031-ILM-2, FTN-2) | a protocol test at level 3, or a statistical test for the spread | one packet on one path is a protocol test; how the LSR spreads a flow over the paths is a distribution |

## A category this pass did not need

No check needs a module test: every checked behavior shows in a frame on a link or in a
datagram that B delivers, also the tables of the class `internal`. No check needs a fingerprint
test: the pass locks behaviors to the standard, not trajectories to a reference run.
