# DHCP — English check procedures: the address allocation exchange

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc2131/catalog.md](../../../standard/rfc2131/catalog.md), [rfc2132/catalog.md](../../../standard/rfc2132/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

Seven checks share one scenario: a client with no address gets one. The first check watches
the four messages as a sequence, the second follows the transaction identifier through them,
and the others read the contents of one message or one group of fields each. They are
separate checks because a failure in one message must not hide the others.

The transaction identifier has a check of its own and is not part of the sequence check. The
reason is the rule that no check may hide another: a step that demanded both the right
message type and the right transaction identifier would stop at the first message where
either one was wrong, and the two statements would then share one verdict.

## Address allocation exchange

Checks: **RFC2131-MSG-1** (description), **RFC2131-MSG-3** (must), **RFC2131-MSG-5**
(description), **RFC2131-OFF-1** (may), **RFC2131-ACK-1** (description), **RFC2131-ACK-2**
(description), **RFC2132-TYPE-1** (description).

### Requirement

RFC 2131 §3.1: the client broadcasts a DHCPDISCOVER; a server may answer with a DHCPOFFER
that carries a free address in `yiaddr`; the client broadcasts a DHCPREQUEST that names the
chosen server; the chosen server commits the binding and answers with a DHCPACK whose
`yiaddr` is the assigned address. §3: `op` is BOOTREQUEST from the client and BOOTREPLY from
the server. §3: every message carries the `DHCP message type` option, whose values RFC 2132
§9.6 defines. §4.1: a client sends to UDP port 67 and a server to UDP port 68.

### Scenario constants

- The plain mockup. The server holds 192.168.1.1 and gives out addresses from 192.168.1.100.
  The lease is 300 seconds.
- Observation stops one second after the start. The whole exchange needs less than a
  millisecond on a 100 Mbit link.

### Procedure

1. Build the plain mockup with the server and one client.
2. Let the client start. Nothing else happens on the link.
3. Observe every frame that leaves the client and every frame that leaves the server, and
   read the type option, the `op` field, the `xid` field, the UDP ports and the `yiaddr`
   field of each one.

### Expected observations

1. The client sends a message whose type option holds 1, DHCPDISCOVER, whose `op` is 1,
   BOOTREQUEST, and whose UDP destination port is 67. This confirms the stimulus
   (RFC2131-MSG-1, RFC2131-MSG-3, RFC2131-MSG-5, RFC2132-TYPE-1).
2. The server sends a message whose type option holds 2, DHCPOFFER, whose `op` is 2,
   BOOTREPLY, and whose UDP destination port is 68. Its `yiaddr` is an address of the subnet
   and is not 0.0.0.0. Record that address (RFC2131-OFF-1).
3. The client sends a message whose type option holds 3, DHCPREQUEST, whose `op` is 1 and
   whose UDP destination port is 67.
4. The server sends a message whose type option holds 5, DHCPACK, whose `op` is 2, whose UDP
   destination port is 68, and whose `yiaddr` is the address of observation 2
   (RFC2131-ACK-1, RFC2131-ACK-2).
5. No message of type 6, DHCPNAK, leaves the server in the window.

### Notes

- Observation 5 is the negative half of the check. An exchange that ends in a DHCPACK and a
  DHCPNAK together has not allocated an address, and the four positive observations alone
  would not see the difference.
- Observation 4 carries the address of observation 2 forward, and that is the one relation
  this check follows across messages. It is the relation RFC2131-ACK-2 states: the address
  the server commits is the address it offered.
- RFC2131-OFF-1 is a `MAY`: a server need not answer. So this check does not establish that
  a server must answer; it establishes what the answer looks like when it comes. A server
  that stayed silent would fail observation 2, and the results would have to read that
  failure against §4.2, which lets an administrator configure silence.

## Transaction identifier through the exchange

Checks: **RFC2131-XID-3** (description). Also covers: **RFC2131-XID-1** (description).

### Requirement

RFC 2131 table 1: `xid` is a transaction identifier, a random number chosen by the client,
used by the client and the server to associate messages and responses between a client and a
server. §4.3.1: the server inserts the `xid` field from the DHCPDISCOVER message into the
`xid` field of the DHCPOFFER message. §4.4.1: the DHCPREQUEST message contains the same `xid`
as the DHCPOFFER message. Table 3: the `xid` of a DHCPACK is the `xid` of the DHCPREQUEST.

### Scenario constants

- The plain mockup. One client, so that only one transaction is open on the link.

### Procedure

1. Build the plain mockup.
2. Let the client start, and record the `xid` of its DHCPDISCOVER.
3. Read the `xid` of the DHCPOFFER, the DHCPREQUEST and the DHCPACK that follow it.

### Expected observations

1. A DHCPDISCOVER leaves the client. Record its `xid`. This confirms the stimulus.
2. The `xid` of the DHCPOFFER equals the recorded one.
3. The `xid` of the DHCPREQUEST equals the recorded one (RFC2131-XID-3).
4. The `xid` of the DHCPACK equals the recorded one.

### Notes

- The value is recorded and not predicted. RFC2131-XID-1 says it is random, so a check that
  named a number would check the wrong thing. That one value runs through four messages is
  the observable part, and observation 1 covers RFC2131-XID-1 only to that extent: a client
  that used the same constant on every run would pass this check.
- Observation 3 is the one that can fail on its own. Observations 2 and 4 are the server
  copying a value out of the message it answers, which is a table cell; observation 3 is the
  client choosing to keep the value it started with, which a client that generates a fresh
  identifier for every message it sends would not do.
- A client that changes the identifier between the DHCPOFFER and the DHCPREQUEST still
  completes an exchange on a quiet subnet, because the server answers whatever it receives.
  What it loses is the ability of the other servers of the subnet to connect the request to
  the offer they made, and the ability of a client with two transactions open to tell two
  answers apart. That is why the statement matters although a simple exchange works without
  it.

## Discover contents

Checks: **RFC2131-DISC-2** (description), **RFC2131-DISC-3** (must), **RFC2131-DISC-5**
(must not), **RFC2131-MSG-9** (must). Also covers: **RFC2131-DISC-1** (the destination).

### Requirement

RFC 2131 §4.4.1: the client sets `ciaddr` to 0, includes its hardware address in `chaddr`,
and broadcasts the message to 255.255.255.255 and port 67. Table 5: a DHCPDISCOVER carries
no `server identifier` option. §4.1: a message that a client broadcasts before it has an
address carries IP source address 0.

### Scenario constants

- The plain mockup. The client has no address and no configured server.

### Procedure

1. Build the plain mockup.
2. Let the client start.
3. Read the first DHCPDISCOVER on the link: its IP source and destination addresses, its
   `ciaddr` field, its `chaddr` field, and the codes in its option area.

### Expected observations

1. A message of type 1, DHCPDISCOVER, leaves the client. This confirms the stimulus.
2. Its IP destination address is 255.255.255.255 and its link-layer destination address is
   the broadcast address (RFC2131-DISC-1).
3. Its IP source address is 0.0.0.0 (RFC2131-MSG-9).
4. Its `ciaddr` field is 0.0.0.0 (RFC2131-DISC-2).
5. Its `chaddr` field is the hardware address of the interface it left, and its `hlen` field
   is the length of that address in octets (RFC2131-DISC-3).
6. Its option area does not hold code 54, `server identifier` (RFC2131-DISC-5).

### Notes

- Observation 3 and observation 4 look alike and are two different rules. One is about the
  IP header that carries the message; the other is about a field inside the message. A
  sender can get one right and the other wrong.
- Observation 5 is an absence. A client cannot name a server before it has heard from one,
  so the prohibition is the protocol saying: you do not know yet.

## Offer contents

Checks: **RFC2131-OFF-2** (description), **RFC2131-OFF-3** (must), **RFC2131-OFF-4** (must),
**RFC2131-OFF-5** (must not, four times), **RFC2131-OFF-6** (description),
**RFC2131-OFF-8** (description), **RFC2132-LEASE-1** (description), **RFC2132-SRVID-1**
(description).

### Requirement

RFC 2131 table 3, DHCPOFFER column: `server identifier` and `IP address lease time` are a
`MUST`; `requested IP address`, `parameter request list`, `client identifier` and `maximum
message size` are a `MUST NOT`; `hops`, `secs` and `ciaddr` are 0, and `xid`, `flags`,
`giaddr` and `chaddr` come from the DHCPDISCOVER. §2: a server always returns its own
address in the `server identifier` option.

The `client identifier` row of the `MUST NOT` list is overridden by RFC 6842 and is not part
of this check; see [Client identifier
echoed](client-identity.md#client-identifier-echoed).

### Scenario constants

- The plain mockup. The lease is 300 seconds.

### Procedure

1. Build the plain mockup.
2. Let the client start, and record the `xid`, the `flags`, the `giaddr` and the `chaddr` of
   its DHCPDISCOVER.
3. Read the DHCPOFFER that answers it: every field of the fixed part and every code in the
   option area.

### Expected observations

1. A DHCPDISCOVER leaves the client. This confirms the stimulus, and it is the source of the
   four recorded values.
2. A DHCPOFFER answers it, and its `xid` equals the recorded `xid` (RFC2131-OFF-2).
3. Its option area holds code 54, `server identifier`, and the address inside it equals the
   IP source address of the datagram that carried the DHCPOFFER (RFC2131-OFF-3,
   RFC2131-OFF-8, RFC2132-SRVID-1).
4. Its option area holds code 51, `IP address lease time`, and its value is 300
   (RFC2131-OFF-4, RFC2132-LEASE-1).
5. Its option area holds neither code 50, `requested IP address`, nor code 55, `parameter
   request list`, nor code 57, `maximum DHCP message size` (RFC2131-OFF-5).
6. Its `hops` is 0 and its `secs` is 0; its `flags` and its `chaddr` equal the recorded ones
   (RFC2131-OFF-6, in part).

### Notes

- Observation 3 makes two statements at once, and it is worth separating them in the mind:
  the option is present, which is RFC2131-OFF-3, and it holds the server's own address,
  which is RFC2131-OFF-8. The second one is what makes the option useful; a server that put
  any other address there would send the client's later unicasts to the wrong place.
- Observation 4 states the value 300 and not "a value greater than zero". The scenario sets
  the lease, so the check can say what the number must be, and a server that returned its
  own default instead of the configured one would be caught.
- The lease time is in seconds by RFC2131-LEASE-1, so the value 300 means 300 seconds and
  the check does not need a unit.
- Observation 6 covers part of RFC2131-OFF-6 and not all of it. The two address fields of the
  same table row, `ciaddr` and `giaddr`, are the subject of [Address fields of a server
  reply](#address-fields-of-a-server-reply). They are separate because a wrong address field
  would otherwise hide the verdict on the four options of observations 3 to 5.

## Request contents

Checks: **RFC2131-REQ-1** (must), **RFC2131-REQ-2** (must), **RFC2131-REQ-3** (must),
**RFC2131-REQ-4** (must), **RFC2131-REQ-8** (must), **RFC2131-REQ-9** (must),
**RFC2132-REQIP-1** (description).

### Requirement

RFC 2131 §3.1: the DHCPREQUEST of a client in SELECTING must include the `server identifier`
option to name the chosen server, and the `requested IP address` option must hold the
`yiaddr` of the chosen DHCPOFFER; the message is broadcast and must use the same `secs` value
and the same IP broadcast address as the DHCPDISCOVER. §4.3.2: `ciaddr` must be zero. §3.5
and §4.3.2: a client that listed parameters in the DHCPDISCOVER must list them again, and a
client that used a `client identifier` must use the same one.

### Scenario constants

- The plain mockup.

### Procedure

1. Build the plain mockup.
2. Let the client start. Record the `secs` value, the `parameter request list` and the
   `client identifier` of the DHCPDISCOVER, and the `yiaddr` and the `server identifier` of
   the DHCPOFFER that answers it.
3. Read the DHCPREQUEST that follows.

### Expected observations

1. A DHCPOFFER reaches the client. This confirms the stimulus: without an offer there is
   nothing for the client to select.
2. A DHCPREQUEST leaves the client, and its IP destination address is 255.255.255.255, the
   same broadcast address the DHCPDISCOVER used (RFC2131-REQ-4).
3. Its option area holds code 54, `server identifier`, and the address inside it equals the
   `server identifier` of the DHCPOFFER (RFC2131-REQ-1).
4. Its option area holds code 50, `requested IP address`, and the address inside it equals
   the `yiaddr` of the DHCPOFFER (RFC2131-REQ-2, RFC2132-REQIP-1).
5. Its `ciaddr` field is 0.0.0.0 (RFC2131-REQ-3).
6. Its `secs` field equals the recorded `secs` of the DHCPDISCOVER (RFC2131-REQ-4).
7. Its option area holds code 55, `parameter request list`, with the same option codes in
   the same order as the DHCPDISCOVER, and code 61, `client identifier`, with the same
   octets (RFC2131-REQ-8, RFC2131-REQ-9).

### Notes

- Observation 3 and observation 4 are the two halves of "I accept this offer". A DHCPREQUEST
  that named a server but not an address, or an address but not a server, would leave the
  other servers of the subnet unable to tell whether the offer was refused.
- Observation 6 is easy to dismiss and has a purpose that RFC 2131 states: a relay agent may
  use `secs` to decide where to forward, so a changed value could send the DHCPREQUEST to a
  different set of servers than the DHCPDISCOVER reached. In a mockup with no relay agent the
  observation still stands as a field rule.
- Observation 7 checks both `MUST` statements of repetition in one place, because both are
  about the same message and both compare against the same earlier message. A client that
  sends no `parameter request list` at all and no `client identifier` at all satisfies
  neither statement nor breaks it: the two rules are conditional. The check states which
  case it observed.

## Acknowledgement contents

Checks: **RFC2131-ACK-3** (must), **RFC2131-ACK-4** (must), **RFC2131-ACK-5** (must not,
four times), **RFC2131-ACK-8** (description). Also covers: **RFC2131-ACK-6** (should not).

### Requirement

RFC 2131 table 3, DHCPACK column: `IP address lease time` is a `MUST` when the DHCPACK
answers a DHCPREQUEST; `server identifier` is a `MUST`; `requested IP address`, `parameter
request list`, `client identifier` and `maximum message size` are a `MUST NOT`; `hops` is 0,
`secs` is 0, and `xid`, `flags`, `giaddr` and `chaddr` come from the DHCPREQUEST. §3.1: the
parameters of the DHCPACK should not conflict with those of the earlier DHCPOFFER.

The `client identifier` row is overridden by RFC 6842 and is not part of this check. The two
address fields, `ciaddr` and `giaddr`, are the subject of [Address fields of a server
reply](#address-fields-of-a-server-reply).

### Scenario constants

- The plain mockup. The lease is 300 seconds.

### Procedure

1. Build the plain mockup.
2. Let the client start. Record the `yiaddr`, the `server identifier` and the lease time of
   the DHCPOFFER, and the `xid`, `flags`, `giaddr` and `chaddr` of the DHCPREQUEST.
3. Read the DHCPACK that ends the exchange.

### Expected observations

1. A DHCPREQUEST leaves the client. This confirms the stimulus.
2. A DHCPACK answers it, and its `xid`, `flags` and `chaddr` equal the recorded ones; its
   `hops` is 0 and its `secs` is 0 (RFC2131-ACK-8, in part).
3. Its option area holds code 51, `IP address lease time`, with the value 300
   (RFC2131-ACK-3).
4. Its option area holds code 54, `server identifier` (RFC2131-ACK-4).
5. Its option area holds neither code 50 nor code 55 nor code 57 (RFC2131-ACK-5).
6. Its `yiaddr`, its `server identifier` and its lease time equal the recorded values of the
   DHCPOFFER (RFC2131-ACK-6).

### Notes

- Observation 6 covers a `SHOULD NOT`, and it covers only three parameters of it. "Any
  configuration parameters" is wider than three, and a check cannot enumerate a set the
  standard leaves open. The three chosen are the ones every exchange carries.
- Observation 3 is the `MUST` half of RFC2131-ACK-3. The `MUST NOT` half — no lease time in
  the answer to a DHCPINFORM — is in [Inform answered without a
  lease](inform.md#inform-answered-without-a-lease), because it needs a different stimulus.

## Address fields of a server reply

Checks: **RFC2131-OFF-6** (description, the `ciaddr` and `giaddr` half), **RFC2131-ACK-8**
(description, the `ciaddr` and `giaddr` half).

### Requirement

RFC 2131 table 3, DHCPOFFER column: `ciaddr` is 0 and `giaddr` is the `giaddr` from the client
DHCPDISCOVER message. DHCPACK column: `ciaddr` is the `ciaddr` from the DHCPREQUEST or 0, and
`giaddr` is the `giaddr` from the client DHCPREQUEST message. Table 1: `ciaddr` is the client IP
address, filled in only if the client is in BOUND, RENEW or REBINDING state; `giaddr` is the
relay agent IP address, used in booting via a relay agent.

### Scenario constants

- The plain mockup. Neither the DHCPDISCOVER nor the DHCPREQUEST of a client in SELECTING
  carries an address: `ciaddr` is 0.0.0.0 in both by RFC2131-DISC-2 and RFC2131-REQ-3, and
  `giaddr` is 0.0.0.0 in both because no relay agent is in the path.
- So both replies must carry 0.0.0.0 in both fields, and the check can name the value.

### Procedure

1. Build the plain mockup.
2. Let the client start. Record the `ciaddr` and the `giaddr` of the DHCPDISCOVER and of the
   DHCPREQUEST.
3. Read the `ciaddr` and the `giaddr` of the DHCPOFFER and of the DHCPACK.

### Expected observations

1. The DHCPDISCOVER and the DHCPREQUEST both carry `ciaddr` 0.0.0.0 and `giaddr` 0.0.0.0. This
   confirms the stimulus: the two values the replies must copy are both zero.
2. The `ciaddr` of the DHCPOFFER is 0.0.0.0 (RFC2131-OFF-6).
3. The `giaddr` of the DHCPOFFER is 0.0.0.0, the value the DHCPDISCOVER carried
   (RFC2131-OFF-6).
4. The `ciaddr` of the DHCPACK is 0.0.0.0, which is both the value the DHCPREQUEST carried and
   the alternative the table allows (RFC2131-ACK-8).
5. The `giaddr` of the DHCPACK is 0.0.0.0, the value the DHCPREQUEST carried (RFC2131-ACK-8).

### Notes

- These four observations were part of [Offer contents](#offer-contents) and
  [Acknowledgement contents](#acknowledgement-contents) and are a check of their own now,
  because a wrong value in one field would stop those programs before they read the options.
  The statements and their IDs do not change; only which check carries them.
- `giaddr` is the one field of a DHCP message whose meaning a reader can easily mistake. It
  is not "the gateway of the client" and not "the next server"; RFC 2131 table 1 says it is
  the address of the relay agent that forwarded the message. The router address a client needs
  travels in option 3, and the next bootstrap server in `siaddr`. A non-zero `giaddr` in a
  reply tells the client, and any relay agent on the path, that the exchange passed through a
  relay agent at that address, and RFC2131-NAK-3, RFC2131-NAK-4 and RFC2131-BCAST-2 all branch
  on the value.
- A non-zero `ciaddr` in a reply is inert for a conforming client, which reads its address from
  `yiaddr`. The rule still has a reason: `ciaddr` in a message from a client means "I hold this
  address and I answer for it", and a reply that echoes an address the client does not hold yet
  states something that is not true.
