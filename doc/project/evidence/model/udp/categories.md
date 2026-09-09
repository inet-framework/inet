# UDP checks — category decisions

> **Kind:** decision · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [results.md](results.md), [test-anatomy.md](../../../design/test-anatomy.md)

Step 9 artifact of the standards test workflow. For each check, this document records the
test category and the reason. The categories and what each one can establish are
[test-anatomy.md](../../../design/test-anatomy.md#the-categories); the mapping from
observation class to category is in the guide, step 9. The category was predicted at step 3
from the observation class of each statement, and confirmed after the run recorded in
[`results.md`](results.md).

## Decisions for the checks of pass 1, level 2

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

**Revised at level 3.** The prediction was too careful. A protocol test can establish the
arithmetic without becoming a serializer test, in two ways that pass 2 uses. It can change
what the value covers and watch the receiver reject the datagram, which needs no arithmetic
at all; and it can compute the value the standard defines from the datagram on the wire and
compare it with the field, which is arithmetic but stays an observation of one datagram in
one run. A unit test is still the right home for the corner cases of the sum, such as the
all-ones rule for a computed zero (RFC1122-UCK-6).

### Port unreachable (RFC792-DU-3, RFC768-HDR-2) → protocol test

The discard at the receiver is an internal event with a packet signal, the report is an
ordinary packet on link 1, and the absence of a handoff is an end-to-end observation. The
`may` strength of the report is a matter for the ledger and the matrix, not for the
category.

## Decisions for the checks of pass 2, level 3

### Checksum by default (RFC1122-UCK-3) → protocol test

The value on the wire is a wire observation, and the state under test is the absence of
configuration, which is a scenario property. The check computes the value the standard
defines and compares; that arithmetic is a property of the datagram the run produced, not of
a serializer in isolation.

### Checksum discard, covers the data, covers the pseudo header (RFC1122-UCK-4, RFC768-CKSUM-1) → protocol test with interception

Each one needs a datagram that no program can build: a wrong value, changed data, a changed
pseudo header. The discard is an internal event with a packet signal, and the silence that
follows is an end-to-end absence. This is the category the pass 1 guidance predicted for the
discard rule, and it held.

### Zero checksum accepted (RFC768-CKSUM-2, RFC1122-UCK-5) → protocol test with interception

The relay writes a value a sender would not write next to that data. The acceptance is
end-to-end.

### Empty datagram (RFC768-HDR-3, the minimum) → protocol test with interception

A sender cannot build the datagram, so the relay makes one. The length is a wire field and
the handoff of zero octets is end-to-end. The run stopped in a result filter of the
receiving program, which is neither, and that is the finding.

### Multicast source address (RFC1122-UADDR-1) → protocol test with interception

The address is a wire field that no sender would write. The discard may happen at either of
two layers, and both are internal events with a packet signal.

### Valid source address (RFC1122-UADDR-2) → protocol test

A wire field, read against the addresses of the sending node. No interception: the check is
about what a host does by itself.

### TTL and TOS from the program, source address from the program (RFC1122-UAPI-1, UMH-2) → protocol test

The program names a value and the check reads it off the wire. The interface half — that the
program can name it at all — is a scenario property: the run proves it by naming it.

## Category guidance for the open catalog entries

| Entry | Likely category | Reason |
| --- | --- | --- |
| RFC768-IP-1 | unit test | the pseudo header is where the module's view of the IP addresses becomes visible; a serializer test computes it |
| RFC1122-UCK-6, a computed zero goes out as all ones | unit test | needs data whose sum is exactly zero; a unit test chooses the octets, a run cannot |
| RFC1122-UERR-1, UOPT-1, UMH-1, UMH-3, UAPI-2, UAPI-3, UCK-2 | module test | the observation is the gate between UDP and the program above it; a test of two nodes on a link cannot reach it. This is the one category the UDP tree needs and does not have |
| RFC1122-UOPT-2, the program names IP options | protocol test, once a sending program offers the interface | the effect is on the wire; what is missing is a program that can ask for an option |

UDP has no timer and no control loop, so no entry points at a statistical test.

The pass 1 guidance listed RFC768-CKSUM-1 as unit-test material and the discard rule as a
protocol test with interception. The second held. The first was revised: two checks of
pass 2 establish the coverage of the checksum from the wire, and a third compares the
computed value with the field. See the revision under the pass 1 checksum decision.
