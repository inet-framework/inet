# DHCP — English check procedures: the address allocation exchange

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc2131/catalog.md](../../../standard/rfc2131/catalog.md), [rfc2132/catalog.md](../../../standard/rfc2132/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

Five checks share one scenario: a client with no address gets one. The first check watches
the four messages as a sequence; the other four read the contents of one message each. They
are separate checks because a failure in one message must not hide the other three.

## Address allocation exchange

Checks: **RFC2131-MSG-1** (description), **RFC2131-MSG-3** (must), **RFC2131-MSG-5**
(description), **RFC2131-XID-1** (description), **RFC2131-XID-3** (description),
**RFC2131-OFF-1** (may), **RFC2131-ACK-1** (description), **RFC2131-ACK-2** (description),
**RFC2132-TYPE-1** (description).

### Requirement

RFC 2131 §3.1: the client broadcasts a DHCPDISCOVER; a server may answer with a DHCPOFFER
that carries a free address in `yiaddr`; the client broadcasts a DHCPREQUEST that names the
chosen server; the chosen server commits the binding and answers with a DHCPACK whose
`yiaddr` is the assigned address. §3: `op` is BOOTREQUEST from the client and BOOTREPLY from
the server. §3: every message carries the `DHCP message type` option, whose values RFC 2132
§9.6 defines. §4.1: a client sends to UDP port 67 and a server to UDP port 68. Table 1 and
§4.3.1: one `xid`, chosen by the client, runs through all four messages.

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
   BOOTREQUEST, and whose UDP destination port is 67. Record its `xid`. This confirms the
   stimulus (RFC2131-DISC-1, RFC2131-MSG-1, RFC2131-MSG-3, RFC2131-MSG-5, RFC2132-TYPE-1).
2. The server sends a message whose type option holds 2, DHCPOFFER, whose `op` is 2,
   BOOTREPLY, whose UDP destination port is 68, and whose `xid` is the recorded one. Its
   `yiaddr` is an address of the subnet and is not 0.0.0.0. Record that address
   (RFC2131-OFF-1, RFC2131-XID-3).
3. The client sends a message whose type option holds 3, DHCPREQUEST, and whose `xid` is the
   recorded one.
4. The server sends a message whose type option holds 5, DHCPACK, whose `xid` is the
   recorded one, and whose `yiaddr` is the address of observation 2 (RFC2131-ACK-1,
   RFC2131-ACK-2).
5. No message of type 6, DHCPNAK, leaves the server in the window.

### Notes

- Observation 5 is the negative half of the check. An exchange that ends in a DHCPACK and a
  DHCPNAK together has not allocated an address, and the four positive observations alone
  would not see the difference.
- The `xid` of observation 1 is recorded and not predicted. RFC2131-XID-1 says the value is
  random, so a check that named a number would check the wrong thing. That the same value
  returns in three later messages is the observable part.
- RFC2131-OFF-1 is a `MAY`: a server need not answer. So this check does not establish that
  a server must answer; it establishes what the answer looks like when it comes. A server
  that stayed silent would fail observation 2, and the results would have to read that
  failure against §4.2, which lets an administrator configure silence.

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
6. Its `hops` is 0, its `secs` is 0 and its `ciaddr` is 0.0.0.0; its `flags`, `giaddr` and
   `chaddr` equal the recorded ones (RFC2131-OFF-6).

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

The `client identifier` row is overridden by RFC 6842 and is not part of this check.

### Scenario constants

- The plain mockup. The lease is 300 seconds.

### Procedure

1. Build the plain mockup.
2. Let the client start. Record the `yiaddr`, the `server identifier` and the lease time of
   the DHCPOFFER, and the `xid`, `flags`, `giaddr` and `chaddr` of the DHCPREQUEST.
3. Read the DHCPACK that ends the exchange.

### Expected observations

1. A DHCPREQUEST leaves the client. This confirms the stimulus.
2. A DHCPACK answers it, and its `xid`, `flags`, `giaddr` and `chaddr` equal the recorded
   ones; its `hops` is 0 and its `secs` is 0 (RFC2131-ACK-8).
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
