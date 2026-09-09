# IPv6 checks — category decisions

> **Kind:** decision · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [results.md](results.md), [test-anatomy.md](../../../design/test-anatomy.md)

Step 9 artifact of the standards test workflow. For each check, this document records the
test category and the reason. The categories and what each one can establish are
[test-anatomy.md](../../../design/test-anatomy.md#the-categories); the mapping from
observation class to category is in the guide, step 9. The category was predicted at step 3
from the observation class of each statement, and confirmed after the run recorded in
[`results.md`](results.md).

## Decisions for the nine checks

### Datagram delivery (RFC8200-HDR-1..4, DLV-1, HL-1, CKSUM-1) → protocol test

Every field the check reads is on the wire, and the delivery to UDP and to the program is an
end-to-end observation. The UDP checksum is read as a field, not recomputed; the algorithm
over the pseudo-header is an encoding statement for a unit test.

### Hop limit expiry (RFC8200-HL-2, RFC4443-TE-1, ERR-2) → protocol test

The discard is visible as an absence on link 2 and the Time Exceeded report is an ordinary
packet on link 1. The in-time observation is the router's own origination of the report,
which the tester sees as a packet event at the router's network layer; an absence step is
still a protocol test, and that anchor guards it against a vacuous pass.

### Hop limit 1 at the destination (RFC8200-HL-3) → protocol test

A hop limit of 1 on link 2 and the delivery at host B's UDP are wire and end-to-end
evidence. The exact case of the should, a packet that arrives with hop limit 0, needs a
crafted sender and is a protocol test with injection at level 3.

### Source fragmentation and reassembly (RFC8200-FRAG-3..6, EXT-1, REASM-1, REASM-2, MTU-4) → protocol test

The fragment headers on two links and the reassembled datagram at UDP are wire and
end-to-end observations. The size arithmetic is a scenario property, so the exact offsets
belong in the protocol test; that the offset field counts 8-octet units on the wire is an
encoding statement for a serializer unit test, and the protocol test compares octets.

### Fragment payload length (RFC8200-FRAG-4, FRAG-5, HDR-2) → protocol test

One header field on two fragments on link 1. It is a separate protocol test because it
fails: a failing step in the middle of the fragmentation program would have hidden the
reassembly verdicts. The category is not in question.

### Fragment identification (RFC8200-FRAG-2) → protocol test

Two identification fields on link 1, compared for inequality. The property over the whole
lifetime of a packet is a count over many packets and possibly a statistical test; level 4.

### Packet too big (RFC8200-FRAG-1, RFC4443-PTB-1, ERR-2) → protocol test

The report is a packet on link 1 and the absence of fragments is an absence on link 2,
anchored on the router's origination of the report.

### Packet too big, the MTU field (RFC4443-PTB-2) → protocol test

One field of one message on link 1. Kept apart from the previous check for the same reason
as the payload length: it fails, and the mechanism around it passes.

### Link MTU packet (RFC8200-MTU-2) → protocol test

A 1500-octet packet on both links and at UDP; wire and end-to-end evidence.

## Decisions for the eighteen level 3 checks

### Hop limit 0 at the destination (RFC8200-HL-3) → protocol test with interception

One rewritten field on the last link and a delivery at UDP.

### Atomic fragment, overlapping fragments, short fragment, oversized fragment offset (RFC8504-NR-3, NR-4, RFC8200-REASM-4, REASM-8) → protocol tests with interception

The relay rewrites a flag, an offset, or the length of one fragment; the evidence is a
delivery or an absence at UDP, and a Parameter Problem on link 1. Two of the four end in a
stop of the simulation, which the check records as a failure; the category does not change.

### Host error report and report source address (RFC4443-DU-4, ERR-1, ERR-2, SRC-1, SRC-2) → protocol tests

Every field the checks read is in one ICMPv6 message on the wire: type, code, the two
addresses, the payload length that proves the whole invoking packet was quoted.

### No error about an error, for a multicast destination, for a link-layer multicast, for a link-layer broadcast, for an unspecified source (RFC4443-MPR-4, MPR-6, MPR-7, MPR-8, MPR-9) → protocol tests

An absence of an ICMPv6 message on the wire, anchored on the node's own record of the
event or on the arrival of the crafted packet. Three need the relay, one needs the one-link
mockup, one needs only a closed port.

### Error for an unknown protocol (RFC4443-MPR-3, MPR-4) → protocol test with interception

The relay crafts the packet that makes host B send a report about an unknown protocol; the
evidence is host A's silence, and the model's stop is the failure.

### Zero UDP checksum, unrecognized and unassigned next header (RFC8200-CKSUM-1, RFC8504-NR-6) → protocol tests with interception

A rewritten field, a discard record or a Parameter Problem, and an absence at UDP.

### Unknown ICMPv6 error type and informational type (RFC4443-MPR-4, MPR-2) → protocol tests with interception

A rewritten type and the absence of any answer. The handoff of an unknown error type to the
upper-layer process (RFC4443-MPR-1) is not a protocol-test observable; see the open entries.

## Category guidance for the open catalog entries

| Entry | Likely category | Reason |
| --- | --- | --- |
| RFC4443-MPR-1 (the handoff), UL-1, UL-2, UL-3 | module test | the handoff of a received report to the upper-layer process happens inside the node; the model carries it as an indication that no packet signal shows |
| RFC8200-REASM-7, RFC8504-NR-8 (receiver half) | protocol test with interception | a first fragment shorter than its upper-layer header; a later pass |
| RFC4443-MPR-5 | protocol test | a redirect in flight; needs a neighbor discovery scenario |
| RFC4443-DU-P | protocol test | a point-to-point link in the mockup |
| RFC8200-EXT-2, RFC8504-NR-7 | protocol test at level 5 | two or more extension headers in one packet |
| RFC4443-MPR-10, DU-C | statistical test at level 4 | a rate, and a congested queue |
| RFC8200-REASM-3 | protocol test at level 4, statistical test for the value | a 60-second timer and its report |
| RFC8200-MTU-3 | protocol test at level 4 | path MTU discovery is a control loop driven by Packet Too Big; blocked until the MTU field is filled |
| RFC8200-MTU-5 | none | a rule for the upper layer; the scenarios stay at 1500 octets |
| RFC8200-MTU-1 | none | a scenario constraint on links, obeyed by every check |
| RFC8200-CKSUM-2; the 8-octet unit of the fragment offset | unit test | serializer concerns |
| RFC4443-DU-4 | protocol test | a closed port; belongs with a UDP-over-IPv6 pass |
| the pointer field of Parameter Problem (RFC8200-EXT-3, REASM-4, REASM-8) | protocol test | one more field in a message this pass already observes |
