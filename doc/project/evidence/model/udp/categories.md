# UDP checks — category decisions

> **Kind:** decision · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [results.md](results.md), [test-anatomy.md](../../../design/test-anatomy.md)

Step 9 artifact of the standards test workflow. For each check, this document records the
test category and the reason. The categories and what each one can establish are
[test-anatomy.md](../../../design/test-anatomy.md#the-categories); the mapping from
observation class to category is in the guide, step 9. The category was predicted at step 3
from the observation class of each statement, and confirmed after the run recorded in
[`results.md`](results.md).

## Decisions for the three checks

### Datagram delivery (RFC768-HDR-1, HDR-2, HDR-3, PROTO-1, UI-1) → protocol test

The ports, the length and the IP protocol number are wire fields; the handoff to the
program is an end-to-end observation at the receiver. The size of each datagram is the
scenario's way to tell the two programs apart, which is a scenario property and not a code
property.

### Checksum presence and absence (RFC768-CKSUM-2, CKSUM-1) → protocol test, with a unit test beside it

Presence and absence of the checksum are wire values; the acceptance of both kinds is
end-to-end. Whether the nonzero value is the right one — RFC768-CKSUM-1, the one's
complement sum over the pseudo header — is an encoding statement, and a serializer unit test
in `tests/unit` is its home. The protocol test does not check the arithmetic.

### Port unreachable (RFC792-DU-3, RFC768-HDR-2) → protocol test

The discard at the receiver is an internal event with a packet signal, the report is an
ordinary packet on link 1, and the absence of a handoff is an end-to-end observation. The
`may` strength of the report is a matter for the ledger and the matrix, not for the
category.

## Category guidance for the open catalog entries

| Entry | Likely category | Reason |
| --- | --- | --- |
| RFC768-CKSUM-1 | unit test | the checksum arithmetic is a serializer concern |
| RFC768-IP-1 | unit test | the pseudo header is where the module's view of the IP addresses becomes visible; a serializer test computes it |
| RFC 1122 §4.1.3.4, discard on a wrong checksum | protocol test with interception | needs a corrupted datagram in flight; level 3, and a document outside the in-scope set |

UDP has no timer and no control loop, so no entry points at a statistical test.
