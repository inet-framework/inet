# IPv4 checks — category decisions

> **Kind:** decision · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [results.md](results.md), [test-anatomy.md](../../../design/test-anatomy.md)

Step 9 artifact of the standards test workflow. For each check, this document records the
test category and the reason. The categories and what each one can establish are
[test-anatomy.md](../../../design/test-anatomy.md#the-categories); the mapping from
observation class to category is in the guide, step 9. The category was predicted at step 3
from the observation class of each statement, and confirmed after the run recorded in
[`results.md`](results.md).

## Decisions for the seven checks

### Datagram delivery (RFC791-FWD-1, DLV-1, PROTO-1, HDR-1, HDR-2, HDR-3) → protocol test

Every field the check reads — version, header length, total length, protocol, the two
addresses — is on the wire, and the delivery to UDP and to the program is an end-to-end
observation. The packet trace and the signals of the destination carry the whole evidence.

### TTL decrement (RFC791-TTL-1, TTL-3, CKSUM-1) → protocol test

The TTL before and after one hop is wire evidence. So is the checksum: the check compares
the field on the two links and needs no knowledge of the algorithm. The algorithm itself —
that the value is the one's complement sum of the header — is an encoding statement, and a
serializer unit test in `tests/unit` is its home. That is RFC791-CKSUM-1's second half and
a later pass.

### TTL expiry (RFC791-TTL-2, RFC792-TE-1) → protocol test

The destruction is visible as an absence on link 2 and at host B, and the time exceeded
report is an ordinary packet on link 1. An absence step is still a protocol test; the
stimulus observation on link 1 guards it against a vacuous pass.

### Fragment and reassembly (RFC791-FRAG-1..4, REASM-1) → protocol test

The fragment fields on the wire and the delivery of the reassembled datagram are both
externally observable. The size arithmetic is a scenario property, not a code property, so
the exact offsets belong in a protocol test. One aspect points elsewhere: that the offset
field counts 8-octet units is an encoding statement about the bit layout; a serializer unit
test can check the encoding directly. The protocol test checks octet positions only.

### Don't fragment (RFC791-FRAG-5, RFC792-DU-4) → protocol test

The discard is visible as an absence on the wire, and the ICMP report is a normal packet on
link 1. The `may` strength of the ICMP clause is a matter for the ledger and the matrix,
not for the category.

### Identification (RFC791-ID-1) → protocol test

Two identification fields on link 1, compared for inequality. Nothing internal is needed.
The full uniqueness property over the active time of a datagram would need a long run and a
statistical argument about reuse; that is a later pass and possibly a statistical test.

### Minimum sizes (RFC791-FRAG-6, REASM-2) → protocol test

A whole 68-octet datagram on link 2 and a reassembled 576-octet datagram at host B are both
wire and end-to-end observations. The twelve-fragment split that the 68-octet MTU produces
is the same mechanism as the fragment check, exercised harder.

## Category guidance for the open catalog entries

| Entry | Likely category | Reason |
| --- | --- | --- |
| RFC791-REASM-3 | protocol test with injection | needs two interleaved fragment trains, or crafted fragments; level 3 |
| RFC791-CKSUM-2 | protocol test with interception | needs a datagram corrupted in flight; level 3 |
| RFC791-CKSUM-1, algorithm half | unit test | the checksum algorithm is a serializer concern |
| RFC791-FRAG-2, encoding half | unit test | the 8-octet unit of the offset field is a bit-layout statement |

Both open protocol entries need the level 3 toolset. Neither blocks level 2.
