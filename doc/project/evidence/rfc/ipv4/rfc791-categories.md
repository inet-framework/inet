# RFC 791 checks — category decisions

> **Kind:** decision · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rfc791-results.md](rfc791-results.md), [test-anatomy.md](../../../design/test-anatomy.md)

Step 6 artifact of the RFC test workflow (see
[derive-tests-from-an-rfc.md](../../../guide/derive-tests-from-an-rfc.md)).
For each check, this document records the test category and the reason. The categories and
what each one can establish are
[test-anatomy.md](../../../design/test-anatomy.md#the-categories); the mapping from
observation class to category is in the guide, step 6.

## Decisions for the selected checks

### TTL decrement (RFC791-TTL-1, RFC791-TTL-3) → protocol test

The statement is about the behavior of a gateway as seen on the wire: the TTL field before
and after one hop. The packet trace shows the complete evidence. A protocol test observes
the two links and compares the field values. No other category fits better.

### Fragment and reassembly (RFC791-FRAG-1..4, RFC791-REASM-1) → protocol test

The fragment fields on the wire and the delivery of the reassembled datagram are both
externally observable. The size arithmetic is a scenario property, not a code property, so
the exact offsets belong in a protocol test.

One aspect points elsewhere: the rule that the offset field counts 8-octet units is an
encoding statement about the bit layout. A serializer unit test (`tests/unit`) can check the
encoding of the offset field directly. The protocol test here checks octet positions only.

### Don't fragment (RFC791-FRAG-5, RFC792-DU-4) → protocol test

The discard is visible as an absence on the wire, and the ICMP report is a normal packet on
link 1. Both are packet-trace evidence. The "may" strength of the ICMP clause stays a
documentation concern (see the check document), not a category concern.

## Category guidance for the open catalog entries

| Entry | Likely category | Reason |
| --- | --- | --- |
| RFC791-TTL-2, RFC792-TE-1 | protocol | wire absence plus an ICMP report |
| RFC791-FRAG-6 | protocol | wire observation with a 68-octet datagram |
| RFC791-REASM-2 | protocol | end-to-end delivery of a 576-octet datagram |
| RFC791-REASM-3 | protocol (injection) | needs crafted or interleaved fragment trains |
| RFC791-CKSUM-1 | unit + protocol | the algorithm is a serializer concern; the change across a hop is wire-visible |
| RFC791-CKSUM-2 | protocol (interception) | needs corruption of a datagram in flight |
| RFC791-ID-1 | protocol | compare the identification of two successive datagrams |
