# DHCP — quirks and follow-ups

> **Kind:** reference · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [results.md](results.md), [coverage.md](coverage.md)

What this document is for. The workflow's artifacts each answer one question: what the standard
says, what a run showed, what the model claims. This one holds what a person learned while doing
the work and would otherwise have to learn again — the behaviors of the model that surprised the
author, the traps in the tooling that cost a debugging cycle, and the ordered list of what to do
next.

The tooling quirks that apply to every protocol are in
[`ipv4/notes.md`](../ipv4/notes.md#tooling-quirks). What follows is what DHCP added, in one pass:
levels 1 to 3 on 2026-09-11.

## Model quirks

### The options are a flat record, so "the option is absent" is a per-field rule

`DhcpOptions` in [DhcpMessage.msg:71-87](../../../../../src/inet/applications/dhcp/DhcpMessage.msg#L71)
holds the union of the thirteen options the model supports as ordinary fields, and its own
documentation says so: "this DhcpOptions class statically holds the union of all options actually
used by the DHCP protocol models. Options absent from a packet are represented by empty/unfilled
DhcpOptions fields."

Nothing in the record says "this option is not here". The **serializer** decides, and its rule
differs per option:

| Kind of option | Absent when |
| --- | --- |
| an address (50, 1, 54) | the address is unspecified |
| a time (51, 58, 59) | the value is zero |
| a list (55, 3, 6, 42) | the array is empty |
| the message type (53) | the value is -1 |
| a string (12) | the string is empty |
| the client identifier (61) | the MAC address is unspecified |

That matters for every check of a `MUST NOT`, and there are many: RFC 2131 tables 3 and 5 are
mostly prohibitions. `hasOption` in
[DhcpChecks.h](../../../../../tests/protocol/dhcp/DhcpChecks.h) follows the serializer's rule
per code, so the predicate is true exactly when the octets on the wire hold the code. Writing
the check against the field alone — "the address is 0.0.0.0" — would have been a check of the
record and not of the message.

The trap this rule sets is gap 4. A list option is absent when the **array** is empty, and
`sendOffer` sets `setDnsArraySize(1)` and then fills the one element from a field nobody ever
assigns. So the option reaches the wire holding 0.0.0.0, and no configuration can stop it.

### The model has no `siaddr` field

[DhcpMessageSerializer.cc:34-35](../../../../../src/inet/applications/dhcp/DhcpMessageSerializer.cc#L34)
writes four zero octets where `siaddr` belongs, with the comment "FIXME siaddr is missing from
the packet representation". `DhcpMessage` has no such field.

The consequence for this pass is small and worth knowing: the `siaddr` half of
[RFC2131-NAK-5](../../standard/rfc2131/catalog.md#rfc2131-nak-5) — a DHCPNAK carries `siaddr` 0 —
is satisfied by construction and cannot fail. A check cannot tell a model that zeroes the field
on purpose from one that has no field at all.

It also explains the comment on gap 2. `offer->setGiaddr(lease->gateway); // next server ip`
writes the meaning of the missing field into the field that is present.

### The client redraws the transaction identifier in three places

[DhcpClient.cc:515](../../../../../src/inet/applications/dhcp/DhcpClient.cc#L515),
[:590](../../../../../src/inet/applications/dhcp/DhcpClient.cc#L590) and
[:634](../../../../../src/inet/applications/dhcp/DhcpClient.cc#L634) each call
`intuniform(0, RAND_MAX)`. Only the second one, in `sendDiscover`, is where RFC 2131 wants a new
value. The first is gap 1.

Worth keeping in view while reading the fix: the third is in `sendDecline`, and RFC 2131 table 5
does say a DHCPDECLINE carries an `xid` "selected by client", `rfc2131.txt:2030-2031`. So that
one is right and the one in `sendRequest` is the only wrong one.

### One timeout does the work of three

[DhcpClient.cc:45](../../../../../src/inet/applications/dhcp/DhcpClient.cc#L45) sets
`responseTimeout = 60` and `scheduleTimerTO` at
[:674](../../../../../src/inet/applications/dhcp/DhcpClient.cc#L674) reschedules that same value
for every wait the client has: WAIT_OFFER and WAIT_ACK alike. RFC 2131 has three different
quantities here, and the model has one constant:

| RFC 2131 | What it governs | Value the RFC suggests |
| --- | --- | --- |
| §4.1, `rfc2131.txt:1322-1330` | the delay between two retransmissions | 4, then 8, doubling to 64, each with a fuzz of one second |
| §4.4.3, `rfc2131.txt:2179-2181` | how long a client waits in total before it tells the user | 60 seconds, or 4 tries |
| §4.4.5, `rfc2131.txt:2265-2269` | the wait before a renewal is repeated | half the remaining time, at least 60 seconds |

Gaps 13 and 14 are both consequences of that single constant. The comment on line 45 cites
§4.4.3, which is the right section for the number and the wrong section for the use it is put to.

This also constrains any scenario that wants to observe REBINDING. T2 minus T1 is 0.375 of the
lease, and the client abandons the lease 60 seconds after T1, so a lease above about 160 seconds
makes REBINDING unreachable. The rebinding and expiry checks use a lease of 120 seconds for that
reason, and the reason is written into
[`lease.md`](../../protocol/dhcp/checks/lease.md#rebinding-at-t2) from the standard's side.

### Two mechanisms are written out as comments

The duplicate detection at
[DhcpClient.cc:315-320](../../../../../src/inet/applications/dhcp/DhcpClient.cc#L315) and the
release at [:724-725](../../../../../src/inet/applications/dhcp/DhcpClient.cc#L724). Both
comments quote the RFC correctly and both name what is missing.

`sendDecline` at [:632](../../../../../src/inet/applications/dhcp/DhcpClient.cc#L632) is
complete, and its only call site is the one inside the comment. A person who greps for
`sendDecline` finds three hits and must read them to see that none of the three is a live call.
That cost a cycle of this pass.

### The server handles two of the five messages it can receive

RFC 2131 §4.3 lists five. `DhcpServer::processDhcpMessage` handles DHCPDISCOVER and DHCPREQUEST
and drops DHCPINFORM, DHCPDECLINE and DHCPRELEASE with "BOOTREQUEST arrived, but DHCP message
type is unknown", [DhcpServer.cc:276](../../../../../src/inet/applications/dhcp/DhcpServer.cc#L276).

The warning is misleading and worth fixing while somebody is there: the message type is not
unknown, it is unhandled. A reader of that log line would look for a parsing problem.

### The INIT-REBOOT branch asks its two questions in the wrong order

Gap 10, and the reason a pair of checks was needed to find it. The branch at
[DhcpServer.cc:235-253](../../../../../src/inet/applications/dhcp/DhcpServer.cc#L235) looks the
requested address up in `leased` first and tests the subnet only for an address it finds. So:

| Requested address | RFC 2131 §4.3.2 wants | The model does |
| --- | --- | --- |
| in the subnet, not leased | silence | silence ✓ |
| in the subnet, leased to this client | a DHCPACK | a DHCPACK ✓ |
| in the subnet, leased, wrong address | a DHCPNAK | a DHCPNAK ✓ |
| **not in the subnet** | a DHCPNAK | silence ✗ |

Three of the four rows are right, which is why one check alone could not have found this. Moving
the subnet test above the table lookup fixes the fourth row and changes nothing in the other
three.

## Scenario quirks

### A DHCP client refuses to start on an interface that already has an address

"Refusing to start DHCP on interface "eth0" that already has an IP address", during network
initialization. So a scenario must **not** name the DHCP client's interface in the
`Ipv4NetworkConfigurator` configuration. Naming the server and the other hosts is fine; the
configurator leaves an unnamed interface on a shared link alone.

This cost a cycle on the decline check, where the occupant host needed an explicit address and
the client needed none.

### An injected packet carries no arriving-interface tag

A packet pushed into `eth[0].upperLayerOut` enters **above** the MAC, and the MAC is what adds
`InterfaceInd`. A DHCP server drops a message whose arriving interface is not the one it serves,
at [DhcpServer.cc:163-167](../../../../../src/inet/applications/dhcp/DhcpServer.cc#L163), so an
injected message without the tag never reaches the application.

`wrapDhcp` in [DhcpChecks.h](../../../../../tests/protocol/dhcp/DhcpChecks.h) takes the name of
the node the packet arrives at and looks the identifier up in that node's own interface table.
Nothing else in the framework needs this, because the framework's own injection self tests inject
into a node whose application does not read the tag.

### An injected reply never appears at the receiving node's MAC

For the same reason: it enters above the MAC. A check that injects a reply at a client must
observe it at `<node>.udp` and not at `<node>.eth[0].mac`. Both crafted-reply checks of this pass
do.

### A crafted reply must arrive while the client is still in SELECTING

On a 100 Mbit link the real DHCPOFFER answers the DHCPDISCOVER within about 30 microseconds, so
there is no instant at which a crafted reply could be placed before it. The two crafted-reply
checks therefore use a mockup with **no server**: a client with no answer stays in SELECTING for
the whole WAIT_OFFER wait, and the crafted message is then the only answer it can get.

### A relay holds one rule, and one rule names one occurrence or every one

`PacketTap::configure` replaces whatever rule the tap held, so one tap cannot drop the second and
the third match. The expiry check needs exactly that, and it uses **two taps in series**: the
first drops the second DHCPACK of the run, the second then sees only the first and the third, so
its own second match is the third of the run. The arithmetic is worth writing down because it is
easy to get wrong in the other direction.

### Two consecutive `once` steps cannot read one event

The engine offers each event to one step. Several checks of this pass read one message against
many rules — a stimulus confirmation and then five field rules — and the pattern that works is
to observe the **stimulus at one module** and the **fields at another**: the message leaving
`<node>.udp` and then the same message leaving `<node>.eth[0].mac`. Both are events of one
message and the first comes first.

The first version of the discover-contents check had two `once` steps on the same MAC and the
second one timed out.

### A filter expression reaches a DHCP field but not a DHCP address

`DhcpMessage.op == 1` and `DhcpMessage.options.messageType == 2` both work: `PacketFilter` builds
its map from the chunk class name and reaches nested `cObject` fields. An address field does not
work, because `Ipv4Address` reaches the expression engine as a pointer value and cannot be
compared with a string. Every address comparison of this pass is in a lambda.

There is a second trap inside that one. `Ipv4Address::str()` prints `<unspec>` for 0.0.0.0, so a
comparison against `"0.0.0.0"` fails silently. `quad()` in `DhcpChecks.h` calls `str(false)`,
which always prints the dotted quad. The first version of the discover-contents check compared
against `"0.0.0.0"` and failed for that reason alone.

### There is no DHCP dissector, and the checks work anyway

No `Register_Protocol_Dissector` exists for DHCP and no port is registered in the UDP protocol
group, so `UdpProtocolDissector` finds no data protocol and falls through to
`DefaultProtocolDissector`, which visits `packet->peekData()` as one chunk. Since the payload is
a single `DhcpMessage` chunk, that chunk is what gets visited, and `PacketFilter` registers it
under the name `DhcpMessage`. So the expressions work by accident of the default path rather than
by design. A future DHCP dissector would not break them, but it is worth knowing why they work.

### A growth test must advance its record on every event it sees

The sharpest trap of the pass, and it produced a false PASS. The backoff check compares two
consecutive intervals. The first version recorded the instant only when its predicate **matched**,
so with a constant stream of 60-second repetitions it compared the newest interval against a
stale one and matched at t=180: the interval from the recorded t=60 was 120 seconds, which is
greater than 60. The test passed and the model has no backoff at all.

The predicate now updates its record on every DHCPDISCOVER it sees, whether it matches or not. A
check whose own bookkeeping is wrong reports a pass that means nothing.

## Follow-ups, in the order I would do them

The model items come first because five of them are a few lines each and every one of them turns a
red test green.

1. **Move the subnet test above the table lookup in the INIT-REBOOT branch** (gap 10). One
   reordering. It takes [DHCP-F-NAK](../../protocol/dhcp/features.md#dhcp-f-nak) out of `defect`
   and gives four ledger rows a real verdict.
2. **Uncomment the duplicate detection** (gap 11). `sendDecline` is written and complete; what is
   missing is the probe that triggers it and the wait for an answer. It takes
   [DHCP-F-DECLINE](../../protocol/dhcp/features.md#dhcp-f-decline) out of `defect` and gives four
   more rows a real verdict. Items 1 and 2 are the two feature-level defects of the matrix, and
   both are a trigger that never fires behind a mechanism that is already written.
3. **Fix the four one-line field defects**: `giaddr` in a DHCPOFFER (gap 2), `ciaddr` in a DHCPACK
   (gap 3), the `flags` copy in both replies (gap 8), and the transaction identifier in
   `sendRequest` (gap 1). Four features leave `partial`.
4. **Stop emitting a domain name server option with no value** (gap 4), and give the server a
   parameter for it while there. One `partial` feature becomes supported.
5. **Replace the single `responseTimeout` with a real backoff** (gaps 13 and 14). This is the
   largest of the model changes and the one that needs level 4 to measure properly, but it is a
   defect and not a missing feature: the code for the delay exists and carries the wrong law.
6. **Write the thirteen checks the pass owes**, from
   [the debt table](coverage.md#the-coverage-debt-thirteen-checks-this-pass-owes). Four mockups
   clear eleven of them. This is the only test-work item, and it is what blocks level 3.
7. **Decide what the model claims about RFC 6842** and act on it, which is item 3 of
   [`conformance.md`](conformance.md#headlines-for-the-next-pass). Either implement both halves or
   say in both NED files that the model implements RFC 2131 without its updates.
8. **Name RFC 2132 in both NED files.** A documentation change with no code behind it; see item 4
   of [`conformance.md`](conformance.md#headlines-for-the-next-pass).
9. **Correct the misleading warning** at `DhcpServer.cc:276`: the message type is unhandled, not
   unknown.
