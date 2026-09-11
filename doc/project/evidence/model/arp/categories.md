# ARP checks — category decisions

> **Kind:** decision · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [results.md](results.md), [test-anatomy.md](../../../design/test-anatomy.md)

Step 9 artifact of the standards test workflow. For each check, this document records the
test category and the reason. The categories and what each one can establish are
[test-anatomy.md](../../../design/test-anatomy.md#the-categories); the mapping from
observation class to category is in the guide, step 9. The category was predicted at step 3
from the observation class of each statement, and confirmed after the run recorded in
[`results.md`](results.md).

Sixteen checks, and every one of them is a **protocol test**. That is unusual, and it has a
plain cause: ARP is a protocol with no interface to a program and no state that a neighbour
cannot infer. A host asks, a host answers, and a table decides where the next frame goes.
The table is the only internal state of the protocol, and the destination hardware address
of the next frame makes it visible, so even the two checks that are about the table stay
observations of a link.

## The ten checks of level 2

### Address resolution (RFC826-REQ-1, REQ-4, REQ-6, RFC1122-AUSE-1) → protocol test

Every field it reads is a wire field of a packet on a link, and the frame's destination
address is a wire field of the frame that carries it. The "must consult the module" of
RFC826-REQ-1 is an internal statement in its words and a wire statement in its effect: the
request on the link is the consultation, seen from outside.

### The cached mapping (RFC826-REQ-2) → protocol test

The statement is about a table lookup, which is internal. What the check counts is requests
on a link, which is not. A hit is an absence, and an absence of a packet is a wire
observation.

### The cache flush (RFC1122-ACACHE-1, ACACHE-2) → protocol test

The same shape as the cached mapping, with the sign reversed: a request that appears is the
evidence. The lifetime is a scenario constant and not a measured distribution, so this is
not a statistical test. A level 4 pass that asks *when* the entry expires, and not whether,
would need one.

### Reply field values (RFC826-RECV-8, RECV-9) → protocol test

All four address fields, the opcode, the frame's destination address and the packet length
are wire values of two packets in one exchange.

### Learning from a request (RFC826-RECV-6, RECV-7) → protocol test

The statement is about a table, which is internal, and the check is an absence on a link:
host B sends a datagram and asks nothing first. A module test could read the table itself
and would establish more — it would separate "the entry is there" from "the entry was not
needed" — but the absence is enough for the statement, and it needs no access to the module.

### Packet layout on the wire (RFC826-FMT-1, FMT-2, FMT-4, FMT-7, RECV-11, RFC5494-NUM-1, NUM-4) → protocol test, with a unit test beside it

The observation class of all seven statements is `encoding`, and the table of step 9 maps
`encoding` to a **unit test**. This check stays a protocol test, for one reason: the octets
it reads are the octets of a packet that a real exchange put on a link, so the check
establishes that the exchange produces a conforming packet and not only that the serializer
can produce one.

A serializer unit test in `tests/unit` is still the right home for the corner cases of the
layout, and this pass leaves them there: an address field whose width comes from a length
field other than 6 or 4, and the round trip of a packet whose two spaces are values the
model does not know. The second one is where gap 2 would be fixed, and a unit test would
find it without a network.

### A request for a third station (RFC826-RECV-5) → protocol test

A broadcast request reaches three stations and two of them stay silent. Both halves are
wire observations, and the silence is the statement.

### The waiting datagram (RFC1122-AQUEUE-1) → protocol test

End-to-end: the datagram arrives at the program on the far host, and the step order
establishes that it arrived after the reply. The queue itself is internal, and in this model
it is not even in the layer the requirement names; neither fact changes the observation.

### The request rate (RFC1122-AFLOOD-1) → protocol test

A count of packets on a link over a known window. It is a bound on a count and not a
distribution, so it needs no tolerance and is not a statistical test. The timer that
produces the rate is level 4 work, and a statistical test is its category.

### No destination unreachable (RFC1122-ANOERR-1) → protocol test

`error-signal`, in its absent form. The absence of an ICMP message is exactly what the
category can establish, and the two positive observations that precede it are what make the
absence mean something.

## The six checks of level 3

### An unsolicited reply (RFC826-RECV-10) → protocol test

The stimulus is injected and the outcome is an absence on a link. Injection is a tool of the
framework and not a category: the check still observes two stations on a link.

### An experimental opcode (RFC826-RECV-10, RFC5494-NUM-3) → protocol test

The same shape. The verdict came from a run that stopped, which the category can report
because the framework never reaches its verdict line; the absence of that line is the
failure. A module test would report the same defect more precisely, by asserting that the
table is unchanged and the module is still running, and the next pass should add one; see
[`results.md`](results.md#sharpening-candidates-for-the-next-pass).

### An experimental hardware space (RFC826-RECV-2, RFC5494-NUM-2) → protocol test, and a unit test should join it

The statement is `end-to-end` in its absent form, so a protocol test is right. But the
defect the check found lives in the serializer, one layer below the observation, and a
serializer unit test would find it from a single packet with no network at all. The two
categories answer two different questions here: whether a station stays silent, and whether
a packet can be read back. Both are worth having.

### An unknown protocol space (RFC826-RECV-3) → protocol test, and a unit test should join it

The same decision, for the same reason, on the second of the two questions the serializer
answers.

### A newer hardware address (RFC826-TABLE-1, RECV-4) → protocol test

The statement is about a table entry, which is `internal`, and the step 9 table would send
an internal statement without a signal to a **module test**. This check stays a protocol
test because the entry has a wire consequence that is complete: the destination hardware
address of the next frame *is* the entry. Nothing about the statement is left unobserved.

### A reply fills the table (RFC826-RECV-7, RECV-6) → protocol test

The same decision as the previous one. The two together are the strongest case in this suite
for reading internal state through the frame it produces, and the reason it works is that
ARP has only one piece of internal state.

## Where a second category would add something

| Check | Category to add | What it would establish |
| --- | --- | --- |
| Packet layout on the wire | unit test | the corner cases of the layout: a width other than 6 or 4, and the round trip of an unknown hardware space or protocol space — which is gap 2, findable without a network |
| An experimental opcode | module test | that the table is unchanged and the module still runs after the packet, which is what a fixed model would show |
| An experimental hardware space, An unknown protocol space | unit test | the same as the layout row: the read-back of a packet the model does not know |
| Learning from a request | module test | the table entry itself, which would separate "the entry is there" from "no request was needed" |
| The cache flush, The request rate | statistical test, at level 4 | *when* an entry expires and *when* a retry happens, with a tolerance, rather than whether |

No check of this pass belongs in another suite, so none was written elsewhere. The five rows
above are additions and not moves: each check keeps its catalog entry and its protocol test.
