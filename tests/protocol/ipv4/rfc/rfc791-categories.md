# RFC 791 checks — category decisions

Step 6 artifact of the RFC test workflow (see [`../../RFC-WORKFLOW.md`](../../RFC-WORKFLOW.md)).
For each check, this document records the test category and the reason. The INET test tree
offers these categories: protocol, unit, module, packet, fingerprint, statistical,
validation, and others under `tests/`.

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

## General rule of thumb

| Observation class (rfc791-checklist.md) | Usual category |
| --- | --- |
| wire, end-to-end, error-signal | protocol test |
| encoding, algorithm on one message | unit test |
| internal state with a scalar signal | protocol test with a state-signal step |
| internal state without a signal | module test |
| distributions, long-run averages | statistical test |
| whole-trajectory regression lock | fingerprint test |
