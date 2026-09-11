# DHCP — English check procedures: who a reply belongs to

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc2131/catalog.md](../../../standard/rfc2131/catalog.md), [rfc2132/catalog.md](../../../standard/rfc2132/catalog.md), [rfc6842/catalog.md](../../../standard/rfc6842/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

Three checks ask how a client decides that a reply is meant for it. The first one is the rule
RFC 6842 changed; the second and the third put a reply on the link that is meant for somebody
else.

## Client identifier echoed

Checks: **RFC6842-CLID-1** (must), **RFC6842-CLID-2** (must not). Also covers:
**RFC2132-CLID-1** (must).

### Requirement

RFC 6842 §3: if the `client identifier` option is present in a message received from a
client, the server must return the `client identifier` option, unaltered, in its response
message — in a DHCPOFFER, a DHCPACK and a DHCPNAK alike. If the client sent none, the server
must not return one. RFC 2132 §9.14: the option is code 61, at least two octets, the first one
a hardware type or zero, and each client's identifier is unique on its subnet.

This reverses RFC 2131 table 3, which made the option a `MUST NOT` in a DHCPOFFER and a
DHCPACK; see [`standards.md`](../standards.md#override-table).

### Scenario constants

- The mockup with two clients. Both clients send a `client identifier` option, and the two
  values differ.
- Observation stops one second after the start.

### Procedure

1. Build the mockup with the server and two clients.
2. Let both clients start. For each one, record the octets of the `client identifier` option of
   its DHCPDISCOVER.
3. Read the `client identifier` option of the DHCPOFFER and of the DHCPACK that each client
   receives.

### Expected observations

1. Each client sends a DHCPDISCOVER that carries code 61, `client identifier`, and the two
   values differ from each other. This confirms the stimulus and covers RFC2132-CLID-1.
2. The DHCPOFFER that answers the first client carries code 61, and its octets equal the
   recorded octets of that client, unaltered (RFC6842-CLID-1).
3. The DHCPACK that answers the first client carries code 61 with the same octets
   (RFC6842-CLID-1).
4. The same holds for the second client: its DHCPOFFER and its DHCPACK carry its own
   identifier and not the first client's (RFC6842-CLID-1).
5. Each identifier is at least two octets long (RFC2132-CLID-1).

### Notes

- RFC6842-CLID-2, the `MUST NOT` half, needs a client that sends no `client identifier` option.
  Where every client of the mockup sends one, that half cannot be observed, and the ledger
  records it as untested rather than passed. A client that can be configured to leave the
  option out would reach it, and that is a sharpening candidate for the next pass.
- Observation 4 is not a repetition of observations 2 and 3. With one client, a server that
  echoed a constant, or echoed the wrong client's value, would pass; with two clients whose
  values differ, only a server that echoes each client's own value passes.
- A failure here is a failure against RFC 6842 and a pass against RFC 2131, and the two
  documents cannot both be satisfied. A server that omits the option follows the older text.
  The results must say which document the behavior matches, because "wrong" is the wrong word
  for it: the behavior is sixteen years out of date, not incorrect as of 1997.

## Foreign client identifier discarded

Checks: **RFC6842-CLID-3** (must).

### Requirement

RFC 6842 §3: when a client receives a DHCP message containing a `client identifier` option,
the client must compare that client identifier to the one it is configured to send, and if the
two do not match, the client must silently discard the message. RFC 6842 §2 gives the reason:
the `xid` alone cannot decide, because the values that two clients pick need not differ.

### Scenario constants

- The mockup with an injector, **and no server**. A client with no answer stays in SELECTING
  for as long as it waits for offers, so the crafted message is the only answer it can get and
  it arrives in the state the statement is about. With a server in the mockup the real
  DHCPOFFER would arrive within microseconds of the DHCPDISCOVER, and no crafted message could
  be placed before it.
- The crafted message is a DHCPOFFER: `op` BOOTREPLY, the transaction identifier **of the
  client's own pending DHCPDISCOVER**, `yiaddr` 192.168.1.250, a `server identifier` option
  holding 192.168.1.9, an `IP address lease time` option of 600, and a `client identifier`
  option whose octets belong to **another** client.
- The message is addressed to the client, to UDP port 68.
- Observation stops one second after the injection.

### Procedure

1. Build the mockup with an injector, and let the client start.
2. Record the transaction identifier and the `client identifier` of the client's DHCPDISCOVER.
3. Put the crafted DHCPOFFER onto the link, with the recorded transaction identifier and a
   different `client identifier`.
4. Observe what the client does.

### Expected observations

1. The crafted DHCPOFFER reaches the client: a message of type 2, DHCPOFFER, arrives with the
   recorded transaction identifier and with a `client identifier` option that differs from the
   client's own. This confirms the stimulus, and it also confirms that the transaction
   identifier is the right one, so that only the identifier can be a reason to discard.
2. The client sends no DHCPREQUEST that names 192.168.1.9 in its `server identifier` option
   (RFC6842-CLID-3).
3. The client sends no DHCPREQUEST that names 192.168.1.250 in its `requested IP address`
   option (RFC6842-CLID-3).
4. No frame leaves the client with 192.168.1.250 as its IP source address.

### Notes

- Observation 1 carries the whole design of the check. The crafted message is correct in every
  way but one: the transaction identifier matches, the type is right, the fields are
  well-formed. So a client that ignores it can be ignoring it for one reason only. A crafted
  message with a wrong transaction identifier would be discarded by
  [Foreign transaction identifier
  discarded](#foreign-transaction-identifier-discarded) instead, and the check would pass
  without establishing anything.
- The address 192.168.1.250 and the server address 192.168.1.9 belong to no node of the mockup.
  That is deliberate: if the client acts on the crafted offer, the values it then uses could
  have come from nowhere else.
- "Silently" means the client sends nothing about the message. Observations 2 and 3 are
  absences of the two things a client would send if it had accepted the offer.

## Foreign transaction identifier discarded

Checks: **RFC2131-XID-4** (must).

### Requirement

RFC 2131 §4.4.1: if the `xid` of an arriving DHCPOFFER message does not match the `xid` of the
most recent DHCPDISCOVER message, the DHCPOFFER message must be silently discarded. Any
arriving DHCPACK messages must be silently discarded.

### Scenario constants

- The mockup with an injector, **and no server**, for the reason the check above gives: a
  client with no answer stays in SELECTING, and the crafted messages then arrive in the state
  the statement is about.
- Two crafted messages. The first is a DHCPOFFER with a transaction identifier that differs
  from the client's own, `yiaddr` 192.168.1.251, a `server identifier` option holding
  192.168.1.8, and a lease time of 600. The second is a DHCPACK with the client's **correct**
  transaction identifier, `yiaddr` 192.168.1.252 and the same `server identifier`.
- Both are addressed to the client, to UDP port 68.
- Observation runs long enough for the client to give up waiting for an offer and ask again,
  which is what observation 5 reads.

### Procedure

1. Build the mockup with an injector, and let the client start.
2. Record the transaction identifier of the client's DHCPDISCOVER.
3. Put the crafted DHCPOFFER on the link, with a transaction identifier that differs from the
   recorded one.
4. Put the crafted DHCPACK on the link, with the recorded transaction identifier.
5. Observe what the client does.

### Expected observations

1. The crafted DHCPOFFER reaches the client, with a transaction identifier that differs from
   the recorded one. This confirms the first stimulus.
2. The crafted DHCPACK reaches the client, with the recorded transaction identifier. This
   confirms the second stimulus.
3. The client sends no DHCPREQUEST that names 192.168.1.8 in its `server identifier` option,
   and none that names 192.168.1.251 in its `requested IP address` option: the DHCPOFFER with
   the foreign transaction identifier was discarded (RFC2131-XID-4).
4. No frame leaves the client with 192.168.1.252 as its IP source address: the DHCPACK was
   discarded although its transaction identifier was right (RFC2131-XID-4).
5. A second DHCPDISCOVER leaves the client: it never left SELECTING, gave up waiting, and
   started again. A client that had accepted either crafted message would be configured instead.

### Notes

- The statement has two halves and the check has two crafted messages, one for each. The second
  half is the sharper one: a DHCPACK is discarded in SELECTING **even when its transaction
  identifier is right**, because a client in SELECTING has not asked for anything yet and an
  unrequested DHCPACK cannot be an answer. A check with the DHCPOFFER alone would leave that
  half untouched.
- Observation 5 is the positive counterpart of the two absences. A check made only of absences
  cannot tell a client that discarded the two messages from a client that stopped working, and
  the second DHCPDISCOVER shows that the client is alive and still looking.
- The two crafted messages carry two different addresses so that observation 4 can tell them
  apart. With one address, a client that took the first and refused the second would look the
  same as a client that refused both.
