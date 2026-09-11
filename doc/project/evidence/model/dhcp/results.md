# DHCP — run results and model analysis

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../../protocol/dhcp/checks.md), [features.md](../../protocol/dhcp/features.md), [coverage.md](coverage.md)

Step 7 artifact of the standards test workflow. This is the first document of the DHCP pass
that may name the simulation model and reference code, and it does both. The verdicts of
the run, the class of every failure, and where the model implements or fails to implement
each checked behavior.

## Run record

- Date: 2026-09-11 11:25 +0200
- INET: branch `topic/rfc-tests-dhcp-level3`, commit `4e20c74e82`, tree clean
- OMNeT++: 6.4.0
- Build: debug, built from this commit
- Compiler: Ubuntu clang version 23.0.0 (++20260325083105+68994554ea12-1~exp1~20260325203127.404)
- Platform: Ubuntu 26.04.1 LTS, Linux 7.0.0-31-generic x86_64
- Command: `inet_run_protocol_tests -p inet -w dhcp`
- Target level: 3

The build is newer than every source file under `src`, so the library the run linked was
built from this commit. The worktree holds no uncommitted change.

## Verdicts

26 tests. 13 PASS and 13 declared FAIL. No unexpected verdict, so the suite as a whole
reports PASS.

| Test | Verdict | Check |
| --- | --- | --- |
| `Rfc2131AddressAllocation.test` | PASS | [address-allocation-exchange](../../protocol/dhcp/checks/exchange.md#address-allocation-exchange) |
| `Rfc2131TransactionIdentifier.test` | **FAIL**, gap 1 | [transaction-identifier-through-the-exchange](../../protocol/dhcp/checks/exchange.md#transaction-identifier-through-the-exchange) |
| `Rfc2131DiscoverContents.test` | PASS | [discover-contents](../../protocol/dhcp/checks/exchange.md#discover-contents) |
| `Rfc2131OfferContents.test` | PASS | [offer-contents](../../protocol/dhcp/checks/exchange.md#offer-contents) |
| `Rfc2131RequestContents.test` | PASS | [request-contents](../../protocol/dhcp/checks/exchange.md#request-contents) |
| `Rfc2131AckContents.test` | PASS | [acknowledgement-contents](../../protocol/dhcp/checks/exchange.md#acknowledgement-contents) |
| `Rfc2131ReplyAddressFields.test` | **FAIL**, gaps 2 and 3 | [address-fields-of-a-server-reply](../../protocol/dhcp/checks/exchange.md#address-fields-of-a-server-reply) |
| `Rfc2131MessageFraming.test` | PASS | [message-framing-on-the-wire](../../protocol/dhcp/checks/message-format.md#message-framing-on-the-wire) |
| `Rfc2131RenewAtT1.test` | PASS | [renewal-at-t1](../../protocol/dhcp/checks/lease.md#renewal-at-t1) |
| `Rfc2131RebindAtT2.test` | PASS | [rebinding-at-t2](../../protocol/dhcp/checks/lease.md#rebinding-at-t2) |
| `Rfc2131LeaseExpiry.test` | PASS | [lease-expiry](../../protocol/dhcp/checks/lease.md#lease-expiry) |
| `Rfc2131NakWrongSubnet.test` | **FAIL**, gap 10 | [negative-acknowledgement-for-a-wrong-subnet](../../protocol/dhcp/checks/nak.md#negative-acknowledgement-for-a-wrong-subnet) |
| `Rfc2131SilentUnknownClient.test` | PASS | [silence-for-an-unknown-client](../../protocol/dhcp/checks/nak.md#silence-for-an-unknown-client) |
| `Rfc6842ClientIdentifierEchoed.test` | **FAIL**, gap 5 | [client-identifier-echoed](../../protocol/dhcp/checks/client-identity.md#client-identifier-echoed) |
| `Rfc6842ForeignClientIdentifier.test` | **FAIL**, gap 6 | [foreign-client-identifier-discarded](../../protocol/dhcp/checks/client-identity.md#foreign-client-identifier-discarded) |
| `Rfc2131ForeignTransactionId.test` | PASS | [foreign-transaction-identifier-discarded](../../protocol/dhcp/checks/client-identity.md#foreign-transaction-identifier-discarded) |
| `Rfc2131DuplicateAddressDeclined.test` | **FAIL**, gap 11 | [duplicate-address-declined](../../protocol/dhcp/checks/decline.md#duplicate-address-declined) |
| `Rfc2131InformWithoutLease.test` | **FAIL**, gap 9 | [inform-answered-without-a-lease](../../protocol/dhcp/checks/inform.md#inform-answered-without-a-lease) |
| `Rfc2131ReleaseOnShutdown.test` | **FAIL**, gap 12 | [release-on-shutdown](../../protocol/dhcp/checks/release.md#release-on-shutdown) |
| `Rfc2131BroadcastBitClear.test` | **FAIL**, gap 7 | [reply-to-a-client-that-clears-the-broadcast-bit](../../protocol/dhcp/checks/reply-delivery.md#reply-to-a-client-that-clears-the-broadcast-bit) |
| `Rfc2131BroadcastBitSet.test` | PASS | [reply-to-a-client-that-sets-the-broadcast-bit](../../protocol/dhcp/checks/reply-delivery.md#reply-to-a-client-that-sets-the-broadcast-bit) |
| `Rfc2131ReplyFlagsField.test` | **FAIL**, gap 8 | [the-flags-field-of-a-server-reply](../../protocol/dhcp/checks/reply-delivery.md#the-flags-field-of-a-server-reply) |
| `Rfc2131DiscoverRetransmission.test` | **FAIL**, gap 13 | [discover-repeated-without-a-server](../../protocol/dhcp/checks/retransmission.md#discover-repeated-without-a-server) |
| `Rfc2131RequestRetransmission.test` | **FAIL**, gap 14 | [request-repeated-when-the-reply-is-lost](../../protocol/dhcp/checks/retransmission.md#request-repeated-when-the-reply-is-lost) |
| `Rfc2131RequestedParameters.test` | PASS | [requested-parameters-returned](../../protocol/dhcp/checks/parameters.md#requested-parameters-returned) |
| `Rfc2131ParameterWithoutValue.test` | **FAIL**, gap 4 | [a-parameter-the-server-has-no-value-for](../../protocol/dhcp/checks/parameters.md#a-parameter-the-server-has-no-value-for) |

## The class of every failure

All thirteen failures are **model gaps**. Not one is a test error and not one is a
specification misread. Every failing test keeps the assertion the standard states and
declares `%# expected-result: FAIL`; no assertion is inverted and no expectation is weakened.

That every failure falls in one class is worth a sentence, because it is not the usual
outcome of a first deep pass. The reason is that step 6 was written against the check
documents and every mismatch was traced into the source before the test was declared. Four
of the thirteen were found the other way round — the test was written, it failed, and the
source then showed why — and in each of those four the check was **split** so that the
failing statement did not take the verdict of its neighbours with it. Those four splits are
named in the pass log of the ledger.

## The model gaps

Fourteen gaps in thirteen tests; `Rfc2131ReplyAddressFields.test` carries two.

### Gap 1 — the client changes the transaction identifier between the offer and the request

`DhcpClient::sendRequest` starts by drawing a new value:

```cpp
    // setting the xid
    xid = intuniform(0, RAND_MAX); // generating a new xid for each transmission
```

— `src/inet/applications/dhcp/DhcpClient.cc:514-515`

RFC 2131 §4.4.1 says "The DHCPREQUEST message contains the same 'xid' as the DHCPOFFER
message", `rfc2131.txt:2116-2117`, which is
[RFC2131-XID-3](../../standard/rfc2131/catalog.md#rfc2131-xid-3). The comment states the
intent plainly, so this is a decision and not an oversight.

The server side is right: it copies the value of the message it answers, so the DHCPOFFER
carries the identifier of the DHCPDISCOVER and the DHCPACK that of the DHCPREQUEST. The run
shows exactly that: the test matched its first two steps and missed the third.

An exchange on a quiet subnet still completes, because the server answers whatever it
receives. What breaks is the purpose of the field: the servers that were not chosen cannot
connect the DHCPREQUEST to the offer they made, and a client with two transactions open
cannot tell two answers apart.

### Gap 2 — the server writes the gateway address into `giaddr` of a DHCPOFFER

```cpp
    offer->setGiaddr(lease->gateway); // next server ip
```

— `src/inet/applications/dhcp/DhcpServer.cc:422`

RFC 2131 table 1 defines `giaddr` as "Relay agent IP address, used in booting via a relay
agent", `rfc2131.txt:543-544`, and table 3 says the `giaddr` of a DHCPOFFER is the `giaddr`
of the client DHCPDISCOVER, `rfc2131.txt:1556-1558` —
[RFC2131-OFF-6](../../standard/rfc2131/catalog.md#rfc2131-off-6). On a subnet with no relay
agent that value is 0.0.0.0.

There are two errors in the one line, and the comment shows the first: "next server ip" is
the meaning of `siaddr`, not of `giaddr`. The router address a client needs travels in option
3, which the same function also sets correctly at `DhcpServer.cc:439-440`.

The consequence is not cosmetic. `giaddr` is the first value §4.1 tests when it decides where
a reply goes, and
[RFC2131-NAK-3](../../standard/rfc2131/catalog.md#rfc2131-nak-3),
[RFC2131-NAK-4](../../standard/rfc2131/catalog.md#rfc2131-nak-4) and
[RFC2131-BCAST-2](../../standard/rfc2131/catalog.md#rfc2131-bcast-2) all branch on it. A
client or a relay agent that read the field would conclude that the exchange passed through
a relay agent at the server's own address.

`DhcpServer::sendAck` does not set the field at all, so the DHCPACK carries 0.0.0.0 and the
two replies of one exchange disagree with each other.

### Gap 3 — the server writes the leased address into `ciaddr` of a DHCPACK

```cpp
    ack->setCiaddr(lease->ip); // client IP addr.
```

— `src/inet/applications/dhcp/DhcpServer.cc:339`

RFC 2131 table 3 gives two permitted values for that field: "'ciaddr' from DHCPREQUEST or 0",
`rfc2131.txt:1547-1548` — [RFC2131-ACK-8](../../standard/rfc2131/catalog.md#rfc2131-ack-8).
The `ciaddr` of a DHCPREQUEST from SELECTING is 0.0.0.0 by
[RFC2131-REQ-3](../../standard/rfc2131/catalog.md#rfc2131-req-3), so the leased address is a
third value that the table does not allow.

This is the mildest of the fourteen. A conforming client reads its address from `yiaddr` and
never looks at `ciaddr` in a reply, so nothing observable follows. What the rule guards is
the meaning of the field: `ciaddr` in a message from a client says "I hold this address and I
answer for it", and a reply that echoes an address the client does not hold yet states
something that is not true.

### Gap 4 — the server returns a domain name server option it has no value for

```cpp
    ack->getOptionsForUpdate().setDnsArraySize(1);
    ack->getOptionsForUpdate().setDns(0, lease->dns);
```

— `src/inet/applications/dhcp/DhcpServer.cc:360-361`, and the same pair in `sendOffer` at
`:442-443`

Nothing in the model ever assigns `DhcpLease::dns` (`src/inet/applications/dhcp/DhcpLease.h:28`);
a search of both source files finds only the two reads above. The field therefore keeps its
default, an unspecified address. `DhcpMessageSerializer` writes the option because the array
is not empty and not because the address is a real one
(`src/inet/applications/dhcp/DhcpMessageSerializer.cc:158-167`), so the octets on the wire
carry code 6 with the value 0.0.0.0.

RFC 2131 §4.3.1 reaches its last branch for a parameter the server has no value for and no
Host Requirements default for: "The server MUST NOT return a value for that parameter", and
"MUST omit any parameters it cannot provide", `rfc2131.txt:1622-1632` —
[RFC2131-SEL-5](../../standard/rfc2131/catalog.md#rfc2131-sel-5).

The run separates the two halves of the check neatly. The server returns **no** option 42, so
observation 2 passes; it returns option 6, so observation 3 fails. The server also has no
parameter for a name server in its NED file, so there is no configuration that could fix
this: the option is unconditional.

### Gap 5 — the server never returns the client identifier option

`DhcpServer::sendOffer` and `DhcpServer::sendAck` write six options — subnet mask, the two
lease timers, the lease time, the router, the domain name server — and the server identifier
(`DhcpServer.cc:431-447` and `:349-365`). `DhcpServer::sendNak` writes one, the server
identifier (`:303`). None of the three writes option 61.

RFC 6842 §3: "If the 'client identifier' option is present in a message received from a
client, the server MUST return the 'client identifier' option, unaltered, in its response
message", `rfc6842.txt:158-160` —
[RFC6842-CLID-1](../../standard/rfc6842/catalog.md#rfc6842-clid-1).

**This behavior follows RFC 2131 and contradicts RFC 6842, and the two cannot both be
satisfied.** RFC 2131 table 3 makes the option a `MUST NOT` in a DHCPOFFER and a DHCPACK,
`rfc2131.txt:1584`, and the model does what that table says. So "wrong" is the wrong word:
the behavior is sixteen years out of date. RFC 6842 §2 explains what it costs — a client
whose `chaddr` is all zeroes, and two clients that share one hardware address, cannot tell
whether a reply is theirs, because the transaction identifier is not unique across clients
either, `rfc6842.txt:128-139`.

The model's client always sends option 61 (`DhcpClient.cc:534` and `:608`), so the condition
of the `MUST` holds in every exchange of every test.

### Gap 6 — the client never compares the client identifier of a reply

`DhcpClient::handleDhcpMessage` tests two things about an arriving reply, the opcode and the
transaction identifier:

```cpp
    if (msg->getXid() != xid) {
        EV_WARN << "Message transaction ID is not valid, dropping." << endl;
```

— `src/inet/applications/dhcp/DhcpClient.cc:394-395`

and then dispatches on the client state. Nothing reads
`msg->getOptions().getClientIdentifier()` anywhere in the file.

RFC 6842 §3: "When a client receives a DHCP message containing a 'client identifier' option,
the client MUST compare that client identifier to the one it is configured to send. If the
two client identifiers do not match, the client MUST silently discard the message",
`rfc6842.txt:183-186` —
[RFC6842-CLID-3](../../standard/rfc6842/catalog.md#rfc6842-clid-3).

The run shows the consequence and not only the absence. The crafted DHCPOFFER was correct in
every respect but its identifier, and the client accepted it: the forbidden event of the
test fired 0.05 seconds after the injection, a DHCPREQUEST naming a server at 192.168.1.9 and
an address of 192.168.1.250, neither of which belongs to any node of the mockup. A client on a
subnet where two clients share a hardware address would take another client's lease this way.

This gap and gap 5 are the two halves of RFC 6842, and neither one is implemented.

### Gap 7 — the server broadcasts a reply when the broadcast bit is clear

The fourth branch of the delivery rule in both `sendOffer` and `sendAck`:

```cpp
    else {
        // TODO should send it to client's hardware address and yiaddr address, but the application can not set the destination MacAddress.
//        destAddr = lease->ip;
        destAddr = Ipv4Address::ALLONES_ADDRESS;
    }
```

— `src/inet/applications/dhcp/DhcpServer.cc:478-482`

The three branches above it are right, and the quotation of §4.1 that precedes them is
complete and correct (`:459-470`). So the model knows the rule, states it, and then does not
follow its last case, for a stated reason: the application cannot set a destination hardware
address.

RFC 2131 §4.1: "If the broadcast bit is not set and 'giaddr' is zero and 'ciaddr' is zero,
then the server unicasts DHCPOFFER and DHCPACK messages to the client's hardware address and
'yiaddr' address", `rfc2131.txt:1278-1280` —
[RFC2131-BCAST-5](../../standard/rfc2131/catalog.md#rfc2131-bcast-5).

The pair of checks is what makes this legible. `Rfc2131BroadcastBitSet.test` passes and
`Rfc2131BroadcastBitClear.test` fails, and together they say that the server is not reading
the bit at all: it broadcasts whichever value the bit carries. One check for the pair would
have reported one verdict and hidden that.

The reply still reaches the client, so nothing stops working. What the rule buys, and what is
lost, is that a message addressed to one client does not reach every host of the subnet.

### Gap 8 — the server writes a constant into the `flags` field of a reply

```cpp
    offer->setBroadcast(false); // unicast
```

— `src/inet/applications/dhcp/DhcpServer.cc:419`, and `ack->setBroadcast(false);` at `:338`

RFC 2131 table 3 says the `flags` of a DHCPOFFER are the `flags` of the client DHCPDISCOVER
and the `flags` of a DHCPACK those of the client DHCPREQUEST, `rfc2131.txt:1553-1555` — the
`flags` half of [RFC2131-OFF-6](../../standard/rfc2131/catalog.md#rfc2131-off-6) and of
[RFC2131-ACK-8](../../standard/rfc2131/catalog.md#rfc2131-ack-8).

`DhcpServer::sendNak` is the one reply that copies the value:

```cpp
    nak->setBroadcast(msg->getBroadcast());
```

— `DhcpServer.cc:300`

so the three replies of one server do not agree with each other, and the DHCPNAK is the one
that is right.

The gap is invisible while every client clears the bit, which is why
`Rfc2131OfferContents.test` can cover the field only for the value zero and passes. The
crafted request with the bit set is what exposes it, and that is the whole reason
`Rfc2131ReplyFlagsField.test` exists.

The field matters to a relay agent and not to the client: §4.1 says a relay agent that
forwards a reply to a client should examine the bit of the message it forwards,
`rfc2131.txt:1373-1379`, so a reply that lost the bit tells the relay agent to unicast to a
client that asked for a broadcast. With no relay agent in the mockup there is no observable
consequence.

### Gap 9 — the server handles two message types and drops the other three

`DhcpServer::processDhcpMessage` (`DhcpServer.cc:157`) branches on the message type, handles
DHCPDISCOVER and DHCPREQUEST, and ends with:

```cpp
            EV_WARN << "BOOTREQUEST arrived, but DHCP message type is unknown. Dropping it." << endl;
```

— `src/inet/applications/dhcp/DhcpServer.cc:276`

DHCPINFORM, DHCPDECLINE and DHCPRELEASE all fall into that branch. So three of the five
messages a server can receive — RFC 2131 §4.3 lists all five, `rfc2131.txt:1445-1453` — are
dropped with a warning.

The run reaches the DHCPINFORM case: the crafted message arrived at the server's application
and the server answered nothing, so `Rfc2131InformWithoutLease.test` missed the deadline of
its first step. The statements that fail are
[RFC2131-INF-2](../../standard/rfc2131/catalog.md#rfc2131-inf-2) and, by consequence,
[RFC2131-INF-3](../../standard/rfc2131/catalog.md#rfc2131-inf-3) and
[RFC2131-INF-5](../../standard/rfc2131/catalog.md#rfc2131-inf-5), which have nothing to be
read on.

The other two dropped types close two more statements that no check of this pass could reach
anyway: [RFC2131-DECL-4](../../standard/rfc2131/catalog.md#rfc2131-decl-4), the server marks a
declined address as unavailable, and
[RFC2131-REL-4](../../standard/rfc2131/catalog.md#rfc2131-rel-4), the server frees a released
one. Both are `internal` statements whose consequence needs a second exchange to observe, and
the ledger records them as `no check`. This analysis is what tells a reader that neither one
is implemented, which the ledger could not say from a run.

### Gap 10 — the server is silent where it should answer with a DHCPNAK

The INIT-REBOOT branch asks its two questions in the wrong order:

```cpp
                    auto it = leased.find(requestedAddress);
                    if (it == leased.end()) {
                        // if DHCP server has no record of the requested IP, then it must remain silent
```

— `src/inet/applications/dhcp/DhcpServer.cc:235-237`

and only for an address it finds does it then test the subnet, at `:241`, with the wrong-net
DHCPNAK at `:252`. An address of a foreign subnet is in no table of leases, so it takes the
first path and the server stays silent. The run logged exactly that: "DHCP server has no
record of IP 10.0.0.7."

RFC 2131 §4.3.2 orders the two the other way: "Determining whether a client in the INIT-REBOOT
state is on the correct network is done by examining the contents of 'giaddr', the 'requested
IP address' option, and a database lookup. If the DHCP server detects that the client is on
the wrong net ... then the server SHOULD send a DHCPNAK message to the client",
`rfc2131.txt:1723-1730` —
[RFC2131-NAK-2](../../standard/rfc2131/catalog.md#rfc2131-nak-2). The silence rule of
[RFC2131-NAK-7](../../standard/rfc2131/catalog.md#rfc2131-nak-7) applies to a client the
server has no record of **on the correct network**, `rfc2131.txt:1743-1749`.

The statement is a `SHOULD`, so the matrix of step 8 reads this as a declined behavior and
not as a defect. What makes it worth recording is the pair of checks:
`Rfc2131SilentUnknownClient.test` passes and `Rfc2131NakWrongSubnet.test` fails, and together
they say the model is silent in both cases where the standard asks for silence in one of them
only. Neither check alone could establish that.

### Gap 11 — the client never probes the address it was given

`DhcpClient::handleDhcpAck` (`DhcpClient.cc:665`) records the lease and calls `bindLease`
(`:306`), which configures the interface. Neither one sends an address resolution request for
the new address and neither waits for an answer to one. What `bindLease` has instead is the
whole procedure, written out as a comment:

```cpp
    /*
        The client SHOULD perform a final check on the parameters (ping, Arp).
        If the client detects that the address is already in use:
        EV_INFO << "The offered IP " << lease->ip << " is not available." << endl;
        sendDecline(lease->ip);
        initClient();
     */
```

— `src/inet/applications/dhcp/DhcpClient.cc:315-320`

RFC 2131 §4.4.1: "The client SHOULD perform a check on the suggested address to ensure that
the address is not already in use. For example, if the client is on a network that supports
ARP, the client may issue an ARP request for the suggested request",
`rfc2131.txt:2119-2123` —
[RFC2131-DECL-1](../../standard/rfc2131/catalog.md#rfc2131-decl-1).

The run confirms it: in a mockup where another host already held 192.168.1.100, the client
took the DHCPACK for that address and sent no probe, so the test missed the deadline of its
second step.

**The failure does not say whether the client would obey
[RFC2131-DECL-2](../../standard/rfc2131/catalog.md#rfc2131-decl-2) if it ever found a
conflict.** It says that it cannot find one. The distinction matters, and the code makes it
sharply: `DhcpClient::sendDecline` exists and is complete
(`DhcpClient.cc:632-662`), it builds a broadcast DHCPDECLINE with the refused address in
option 50 and the server in option 54, and its **only** call site is the one inside the
comment above. A search for `sendDecline` over the two source files finds the declaration at
`DhcpClient.h:115`, the definition at `DhcpClient.cc:632`, and that commented line at `:319`.
So the message is written and unreachable, the probe that would trigger it is a comment, and
the four statements the check also carried — DECL-2, DECL-5, DECL-6 and DECL-7 — are untested
rather than failed. The ledger records that difference.

### Gap 12 — the client sends no DHCPRELEASE

```cpp
    // TODO Client should send DHCPRELEASE to the server. However, the correct operation
    // of DHCP does not depend on the transmission of DHCPRELEASE messages.
```

— `src/inet/applications/dhcp/DhcpClient.cc:724-725`

The comment quotes RFC 2131 §4.4.6 correctly, `rfc2131.txt:2284-2285`, and the statement the
check targets, [RFC2131-REL-1](../../standard/rfc2131/catalog.md#rfc2131-rel-1), is a `MAY`.
So the matrix reads this as a declined behavior and not as a defect, and the comment is a
fair statement of why.

The run shut the client's application down at 60 seconds, inside a lease of 300 seconds and
before T1 at 150. The scenario manager logged the shutdown and no DHCPRELEASE left the
client.

What the model loses is the server's chance to free the address early; the lease runs out
instead. The four field rules of the message — REL-2, REL-3 and REL-5 — have nothing to be
read on and are untested rather than failed.

### Gap 13 — the client's retransmission delay is a constant

```cpp
        responseTimeout = 60; // response timeout in seconds RFC 2131, 4.4.3
```

— `src/inet/applications/dhcp/DhcpClient.cc:45`

and `scheduleTimerTO` reschedules that same value every time it is called:

```cpp
    rescheduleAfter(responseTimeout, timerTo);
```

— `src/inet/applications/dhcp/DhcpClient.cc:678`

RFC 2131 §4.1: "The client MUST adopt a retransmission strategy that incorporates a
randomized exponential backoff algorithm to determine the delay between retransmissions",
`rfc2131.txt:1317-1319` —
[RFC2131-RETX-1](../../standard/rfc2131/catalog.md#rfc2131-retx-1). Neither half is there:
the delay does not grow and it carries no random part.

The run on a subnet with no server produced DHCPDISCOVER messages at 0, 60, 120, 180 and so
on, so the interval is exactly 60 seconds each time. The repetition itself works —
[RFC2131-RETX-3](../../standard/rfc2131/catalog.md#rfc2131-retx-3) holds — and only the
timing rule fails.

The figure the model uses is the right number for a different quantity. §4.4.3 names "60
seconds or 4 tries" as the total a client should wait before it tells the user,
`rfc2131.txt:2179-2181`; the comment on the line cites that section. §4.1 is about the delay
between two retransmissions, and it suggests 4 seconds, then 8, doubling to at most 64,
`rfc2131.txt:1322-1330`. The two were conflated.

A note on how this gap was found, because it bears on the reliability of the other twelve.
The first version of the growth test compared the newest interval against the interval it had
last **recorded**, and a record that advances only on a match compares against a stale value:
with a constant stream of 60-second repetitions the test passed, at t=0, 60 and 180. The
predicate now advances its record on every DHCPDISCOVER it sees, and the test fails as it
should. A check whose own bookkeeping is wrong reports a pass that means nothing, and this one
did until it was corrected.

### Gap 14 — the client does not retransmit a DHCPREQUEST at all

```cpp
        else if (category == WAIT_ACK) {
            EV_DETAIL << "No DHCP ACK received within timeout. Restarting." << endl;
            initClient();
        }
```

— `src/inet/applications/dhcp/DhcpClient.cc:226-228`

`initClient` (`:363`) returns the client to INIT and sends a fresh DHCPDISCOVER. So the
unanswered DHCPREQUEST is never repeated; the client abandons the transaction at the first
timeout.

RFC 2131 §3.1 asks for the repetition first and the restart only after it: "The client times
out and retransmits the DHCPREQUEST message if the client receives neither a DHCPACK or a
DHCPNAK message. ... If the client receives neither a DHCPACK or a DHCPNAK message after
employing the retransmission algorithm, the client reverts to INIT state and restarts the
initialization process", `rfc2131.txt:913-925` —
[RFC2131-RETX-4](../../standard/rfc2131/catalog.md#rfc2131-retx-4).

The run dropped every DHCPACK on the path and the forbidden event fired at 60.0001 seconds: a
DHCPDISCOVER where a repeated DHCPREQUEST should have been. The absence of a DHCPDISCOVER
between the two DHCPREQUEST messages is the observation that separates a retransmission from a
restart, and it is the one that caught this. Without it the test would have passed on the
second DHCPREQUEST of the restarted exchange, which asks for the same address because the
server offers the same address again.

This gap and gap 13 are one shape of problem: the client has no retransmission strategy at
all, only a single timeout that restarts everything.

## What the model does well

Thirteen checks pass, and five of them are worth naming, because they are the ones a weaker
test would have missed.

- **The lease and both timers.** `Rfc2131RenewAtT1.test`, `Rfc2131RebindAtT2.test` and
  `Rfc2131LeaseExpiry.test` all pass. T1 falls at exactly half the lease and T2 at 0.875 of
  it (`DhcpServer.cc:351-353` computes both and `DhcpClient::scheduleTimerT1` and
  `scheduleTimerT2` at `:681` and `:687` read them back), the renewal is a unicast to the
  `server identifier` address with `ciaddr` set and neither option 50 nor option 54, the
  rebinding is a broadcast with the same fields, and at the expiry the client gives the
  address up and asks again as an uninitialized client with IP source 0.0.0.0. The last two
  of those three needed a relay on the path to reach at all.
- **The transaction identifier at the receiving end.** `Rfc2131ForeignTransactionId.test`
  passes on both halves of
  [RFC2131-XID-4](../../standard/rfc2131/catalog.md#rfc2131-xid-4): a crafted DHCPOFFER with
  a foreign identifier is discarded, and a crafted DHCPACK with the **right** identifier is
  discarded too, because a client in SELECTING has asked for nothing. `DhcpClient.cc:394` does
  the first and the SELECTING branch at `:405-414` the second. The client then gave up waiting
  and asked again at t=60, which shows it never left the state.
- **Silence for a client the server does not know.** `Rfc2131SilentUnknownClient.test` passes.
  The server sent nothing at all for a crafted INIT-REBOOT request naming an address inside
  its subnet that it had never leased, which is what
  [RFC2131-NAK-7](../../standard/rfc2131/catalog.md#rfc2131-nak-7) demands and what lets two
  non-communicating servers share one wire.
- **The framing of the octets.** `Rfc2131MessageFraming.test` passes for all four messages of
  the exchange: the magic cookie, an option area that parses by "tag, length, value" from its
  first octet to an `end` option with nothing left over, no code twice, and a lease time of
  300 written as the four octets `00 00 01 2C`. The serializer and the length arithmetic of
  the two applications agree; they have to, because `DhcpMessageSerializer` asserts it
  (`DhcpMessageSerializer.cc:216`).
- **The mandatory and prohibited options of each message.** `Rfc2131OfferContents.test`,
  `Rfc2131RequestContents.test` and `Rfc2131AckContents.test` pass. Every option RFC 2131
  table 3 and table 5 mark `MUST` is present and every one they mark `MUST NOT` is absent,
  with the two exceptions this document records as gaps 4 and 5.

## Sharpening candidates for the next pass

In rough order of what each one would buy.

1. **A second client in the mockup**, which unlocks the largest group of statements the
   ledger records as `no check`: the address selection rules
   ([RFC2131-SEL-1](../../standard/rfc2131/catalog.md#rfc2131-sel-1),
   [SEL-2](../../standard/rfc2131/catalog.md#rfc2131-sel-2)), the server's half of the decline
   and the release ([DECL-4](../../standard/rfc2131/catalog.md#rfc2131-decl-4),
   [REL-4](../../standard/rfc2131/catalog.md#rfc2131-rel-4)), and the server's use of the
   `client identifier` as a key ([ID-2](../../standard/rfc2131/catalog.md#rfc2131-id-2)). Each
   one becomes observable as "what the next client is offered".
2. **A statistical check with a stated tolerance**, which is level 4 and which the five
   distribution statements need:
   [RETX-2](../../standard/rfc2131/catalog.md#rfc2131-retx-2),
   [DISC-4](../../standard/rfc2131/catalog.md#rfc2131-disc-4),
   [LEASE-7](../../standard/rfc2131/catalog.md#rfc2131-lease-7),
   [LEASE-10](../../standard/rfc2131/catalog.md#rfc2131-lease-10) and
   [DECL-3](../../standard/rfc2131/catalog.md#rfc2131-decl-3). Gap 13 already tells what the
   answer will be for the first two.
3. **A relay agent**, which brings RFC 1542 into the in-scope set and reaches
   [NAK-4](../../standard/rfc2131/catalog.md#rfc2131-nak-4) and
   [BCAST-2](../../standard/rfc2131/catalog.md#rfc2131-bcast-2), and which would make gaps 2
   and 8 observable rather than merely wrong.
4. **A client that can be told to send no `client identifier` option**, which is the one
   thing [RFC6842-CLID-2](../../standard/rfc6842/catalog.md#rfc6842-clid-2) needs. The model's
   client always sends one, so the `MUST NOT` half of RFC 6842 stays untested.
5. **A slower link or a slower server**, which would separate the two anchors of
   [LEASE-2](../../standard/rfc2131/catalog.md#rfc2131-lease-2): the expiry is the send instant
   of the DHCPREQUEST plus the lease, and on a 100 Mbit link that differs from the arrival
   instant of the DHCPACK by tens of microseconds, which the one-second tolerance of the
   expiry check cannot see.
6. **A DHCPINFORM for an address that is bound to another client**, which is what
   [INF-4](../../standard/rfc2131/catalog.md#rfc2131-inf-4) needs: a server that looked for a
   lease and found none behaves exactly like a server that did not look. Gap 9 makes this
   moot until the server handles the message at all.
7. **A serializer unit test suite**, the home of the ten `encoding` statements the ledger
   files as `later`: the option overload rules, the pad option, the trailing-null rule and the
   options that only a long reply would carry.
