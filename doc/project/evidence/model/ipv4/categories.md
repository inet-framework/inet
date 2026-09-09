# IPv4 checks — category decisions

> **Kind:** decision · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [results.md](results.md), [test-anatomy.md](../../../design/test-anatomy.md)

Step 9 artifact of the standards test workflow. For each check, this document records the
test category and the reason. The categories and what each one can establish are
[test-anatomy.md](../../../design/test-anatomy.md#the-categories); the mapping from
observation class to category is in the guide, step 9. The category was predicted at step 3
from the observation class of each statement, and confirmed after the run recorded in
[`results.md`](results.md).

## Decisions for the seven level 2 checks

The decisions of pass 2 stand unchanged: datagram delivery, TTL decrement, TTL expiry,
fragment and reassembly, don't fragment, identification, and minimum sizes are protocol
tests, each with the reason given in that pass. Two notes were added to them by this pass:
the datagram-delivery check now also asserts the sender's own source address (still wire
evidence), and the identification check now targets the RFC 6864 statement that governs
the RFC 791 one (the same two fields on link 1).

## Decisions for the fifteen level 3 checks

### TTL 1 at the destination (RFC1122-TTL-2) → protocol test

A TTL of 1 on link 2 and the delivery at host B's UDP are wire and end-to-end evidence. No
relay is needed: a boundary value is not a fault.

### Interleaved reassembly (RFC791-REASM-3, RFC1122-REASM-1) → protocol test with interception

The relay holds one fragment; every observation is a fragment on a link or a datagram at
UDP. The hold is the level 3 toolset at its simplest.

### Same identification, different protocol (RFC791-REASM-1, REASM-3) → protocol test with interception

Two relay rules craft the collision; the evidence is the delivery at UDP and the echo
reply on the wire. The failure is a property of the receiver's buffer key, visible from
outside as a lost datagram.

### Atomic identification (RFC6864-ID-3, ID-7) → protocol test with interception

A rewritten identification and the DF bit on two links are wire evidence; the two
deliveries at UDP are end-to-end.

### Checksum discard (RFC1122-CKSUM-1) → protocol test with interception

The corrupted field is on the wire, and the discard record plus the two absences are the
outside view of "silently discard". The algorithm of the checksum stays a serializer unit
test, as in pass 2.

### Version discard, foreign destination, invalid source address (RFC1122-VER-1, ADDR-2, ADDR-3) → protocol tests with interception

The same shape three times: one rewritten field, one discard record, one absence watch. The
discard record is an internal signal with a packet attached, which the guide's table maps
to a protocol test with a state-signal step.

### Unknown ICMP type (RFC1122-ICMP-1) → protocol test with interception

A rewritten ICMP type on the wire and the absence of any answer. The category does not
change because the model stops the simulation: the check observes from outside, and a stop
is one of the ways it fails.

### Host error report (RFC1122-ERR-1, DU-1, ICMP-2, ICMP-4) → protocol test

Every field the check reads is in one ICMP message on the wire: type, code, TOS, the quoted
header and the quoted UDP header. The check walks the quote with a typed predicate because
the filter expressions cannot reach into a quoted header; that is a tooling detail, not a
category question.

### No error about an error, for a broadcast, for a link-layer broadcast, for a non-initial fragment, for an invalid source (RFC1122-ICMP-5 to ICMP-9) → protocol tests

An absence of an ICMP message on the wire, anchored on the node's own record of the event.
Two of the five need the relay (the link-layer broadcast, the invalid source), one needs a
second gateway, and two need only configuration. An absence step is still a protocol test;
the anchor guards it against a vacuous pass.

## Category guidance for the open catalog entries

| Entry | Likely category | Reason |
| --- | --- | --- |
| RFC1122-REASM-3, FRAG-2 | module test | an interface between layers (MMS_R, MMS_S); nothing on the wire |
| RFC1122-ICMP-3, DU-2, TE-1, PP-2 | module test | the handoff of a received error to the transport protocol happens inside the host; the model carries it as an indication that no packet signal shows |
| RFC1122-DU-3 | module test | a rule for the consumer of a report |
| RFC1122-TTL-1 | module test | the model refuses a TTL of 0 with an assertion; a module test can assert the refusal, a protocol test cannot classify it |
| RFC1122-ADDR-4 | protocol test with a standing rule | a negative over a whole run, not an ordered step; the framework needs a standing `never` |
| RFC1122-FRAG-1, FRAG-3 | protocol test | fragments on the first link with a small host MTU |
| RFC1122-PP-1 | protocol test with interception | a crafted total length; the model sends Parameter Problem for it |
| RFC1122-REASM-4, REASM-5 | protocol test at level 4, statistical test for the range | a timer and its report; the value is a distribution |
| RFC6864-ID-4 | protocol test in the TCP suite | needs a retransmitted non-atomic datagram |
| RFC6864-ID-1 | none | what a node does not use a field for is not observable |
| RFC791-CKSUM-1, algorithm half; RFC791-FRAG-2, encoding half | unit test | serializer concerns, as in pass 2 |
