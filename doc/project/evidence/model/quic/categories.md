# QUIC checks — category decisions

> **Kind:** decision · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [results.md](results.md), [test-anatomy.md](../../../design/test-anatomy.md)

Step 9 artifact of the standards test workflow. For each check, this document records the
test category and the reason. The categories and what each one can establish are
[test-anatomy.md](../../../design/test-anatomy.md#the-categories); the mapping from
observation class to category is in the guide, step 9. The category was predicted at step 3
from the observation class of each statement, and confirmed after the run recorded in
[`results.md`](results.md).

## A note that applies to every QUIC check

QUIC is the first protocol in this tree whose packets the test framework cannot dissect.
The model registers no protocol dissector for QUIC, and its module emits none of the
signals the framework turns into events. Every check below is therefore observed one layer
down, on the UDP datagrams that carry the QUIC packets, and reads QUIC fields through typed
predicates rather than through field filters.

That changes how a check is written. It does not change its category: the evidence is still
a packet on a link, so these are protocol tests. What it does change is the cost of writing
one and the fragility of the result, and [`results.md`](results.md) records that as a
tooling finding.

## Decisions for the six checks

### Connection establishment (RFC9000-PKT-1, PKT-2, SIZE-1) → protocol test

The header form, the version field and the size of the datagram are all properties of a
packet on the path. The size observation is the strongest of the three, because it needs no
field at all: a datagram is 1200 octets or it is not.

### Stream data transfer (RFC9000-STR-2, STR-3, PKT-3) → protocol test

The STREAM frames and the packet numbers are wire values; the total the receiving
application accepts is an end-to-end observation. Ordered delivery of data that arrives out
of order is the half this check cannot reach: it needs a path that reorders, which is level
3, and the check would then be a protocol test with interception.

### Stream identifiers (RFC9000-STR-1) → protocol test

One field of one frame, compared against a rule about its low bits. Nothing internal is
needed. This is the check most exposed to the absence of a dissector, because it depends
entirely on reading a frame field.

### Flow control (RFC9000-FC-1) → protocol test

The advertised limit, the data the sender sends before the limit moves, and the frame that
raises it are all on the path. As with TCP's window check, the scenario sets a limit small
enough to bind, and the timing that follows is a consequence rather than a measurement.

### Acknowledgment (RFC9000-ACK-1) → protocol test

A packet number sent and a packet number acknowledged, both on the wire. The timing bound
that the same statement carries — the declared maximum acknowledgment delay — is a
distribution and a statistical test at level 4, with RFC 9002.

### Connection close (RFC9000-CLOSE-1) → protocol test

The frame is an ordinary packet on the path and the silence that follows is an absence.
The closing and draining states behind it are internal; a check of those is a module test
with a state signal.

## Category guidance for the open catalog entries

| Entry or area | Likely category | Reason |
| --- | --- | --- |
| Ordered delivery of reordered data (the other half of RFC9000-STR-2) | protocol test with interception | needs a path that reorders |
| The acknowledgment delay bound (the other half of RFC9000-ACK-1) | statistical test | a distribution against a declared bound; level 4 |
| Loss detection and congestion control (RFC 9002) | statistical test | the congestion window trajectory is the observable |
| Connection errors, stateless reset, version negotiation | protocol test with injection | need a crafted or invalid packet |
| Address validation with Retry, the anti-amplification limit | protocol test with injection | need a server that challenges an unverified address |
| Packet protection and the handshake (RFC 9001) | unit test | the cryptographic transforms are algorithmic, and no wire observation reaches them |
| The connection state machine | module test with a state signal | the closing and draining states are internal |

Two of these are blocked by more than their level. A check that reads a QUIC field needs
either a dissector for the protocol or a typed predicate for every field it touches; the
results record that as the first tooling task for a QUIC level 3.
