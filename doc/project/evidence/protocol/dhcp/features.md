# DHCP — feature map

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `DHCP-F-*` · **Stands on:** [standards.md](standards.md), [rfc2131/catalog.md](../../standard/rfc2131/catalog.md), [rfc2132/catalog.md](../../standard/rfc2132/catalog.md), [rfc6842/catalog.md](../../standard/rfc6842/catalog.md)

Step 4 artifact of the standards test workflow. The catalogs are flat, fine-grained, and
per document. This document is the high-level view above them: the capabilities that the
in-scope set of [`standards.md`](standards.md#in-scope-set) defines, the requirement level
of each, and the cross reference into the catalogs.

The map reads in two directions:

- **Down** — which checks tell whether a feature works. A `core` check is one the feature
  cannot work without. A `supporting` check is a detail or an edge case.
- **Up** — which features the simulation model supports. Step 7 writes the answer into
  [`coverage.md`](../../model/dhcp/coverage.md), and step 8 compares it with the claims of
  the model in [`conformance.md`](../../model/dhcp/conformance.md).

The feature list comes from the standard texts only. The support of each feature — what the
run actually showed — is **not** in this document. It lives in the coverage ledger,
[`coverage.md`](../../model/dhcp/coverage.md).

## How this map decides the level

The rule of the guide: `mandatory` when any core statement says `must` or `shall`, or when
the mechanism is the only path the document gives to a state or an outcome; `optional` when
every core statement says `may` or `should`; `unstated` otherwise.

RFC 2131 needs one refinement of that rule, and three features depend on it. The document
often states an optional mechanism and then states, with a `MUST`, how the message of that
mechanism looks. §3.4 says a client "may use a DHCPINFORM request message"; §4.3.5 then says
the server "MUST NOT send a lease expiration time" in the answer. The `MUST NOT` holds only
when a DHCPINFORM arrives.

**A conditional keyword does not raise the level of a feature.** The level comes from the
statement that decides whether the mechanism has to exist at all. So the level of
[DHCP-F-INFORM](#dhcp-f-inform), [DHCP-F-RELEASE](#dhcp-f-release) and
[DHCP-F-INIT-REBOOT](#dhcp-f-init-reboot) is `optional`, and each entry names the permission
that sets it and the conditional `must` statements that it does not let override.

The consequence is a real one, and it is the reason the refinement is written down here
rather than applied in silence: the conformance matrix of step 8 turns a `not supported`
mandatory feature into a `defect` and a `not supported` optional feature into a `declined`.
A model that never sends a DHCPRELEASE has declined a permission; it has not broken a rule.

## Index

| ID | Feature |
| --- | --- |
| [DHCP-F-MESSAGE-FORMAT](#dhcp-f-message-format) | Every DHCP message is a BOOTP message whose option area starts with the magic cookie, names its type, and ends with the `end` option. |
| [DHCP-F-TRANSPORT](#dhcp-f-transport) | DHCP messages travel over UDP between port 67 and port 68, and a client with no address sends from 0.0.0.0. |
| [DHCP-F-OPTION-ENCODING](#dhcp-f-option-encoding) | An option is a code, a length and a value, in network byte order, and it may extend into the `sname` and `file` fields. |
| [DHCP-F-TRANSACTION-ID](#dhcp-f-transaction-id) | The `xid` binds a reply to the request it answers, and a reply with a foreign `xid` is discarded in silence. |
| [DHCP-F-DISCOVERY](#dhcp-f-discovery) | A client with no address broadcasts a DHCPDISCOVER to find the servers of its subnet. |
| [DHCP-F-OFFER](#dhcp-f-offer) | A server answers a DHCPDISCOVER with a DHCPOFFER that carries a free address, a lease time and its own identity. |
| [DHCP-F-SELECTION](#dhcp-f-selection) | The client picks one offer and claims it with a broadcast DHCPREQUEST that names the chosen server and the offered address. |
| [DHCP-F-ACKNOWLEDGEMENT](#dhcp-f-acknowledgement) | The chosen server commits the binding and answers with a DHCPACK that carries the assigned address and the lease. |
| [DHCP-F-LEASE](#dhcp-f-lease) | An address is held for a stated number of seconds, and the client computes the expiry, T1 and T2 from the reply. |
| [DHCP-F-RENEW](#dhcp-f-renew) | At T1 the client unicasts a DHCPREQUEST to the leasing server to extend the lease. |
| [DHCP-F-REBIND](#dhcp-f-rebind) | At T2, with the renewal unanswered, the client broadcasts a DHCPREQUEST to any server. |
| [DHCP-F-EXPIRY](#dhcp-f-expiry) | When the lease ends without a DHCPACK, the client stops using the address at once and starts again. |
| [DHCP-F-INIT-REBOOT](#dhcp-f-init-reboot) | A client that remembers an address confirms it with a DHCPREQUEST that names no server. |
| [DHCP-F-NAK](#dhcp-f-nak) | A server that cannot satisfy a request answers with a DHCPNAK, broadcast when no relay agent is in the path, and the client starts again. |
| [DHCP-F-DECLINE](#dhcp-f-decline) | A client that finds the assigned address in use sends a DHCPDECLINE and starts again, and the server takes the address out of service. |
| [DHCP-F-RELEASE](#dhcp-f-release) | A client gives an address back with a unicast DHCPRELEASE, and the server frees it. |
| [DHCP-F-INFORM](#dhcp-f-inform) | A client that already has an address asks for the other parameters alone with a DHCPINFORM. |
| [DHCP-F-RETRANSMISSION](#dhcp-f-retransmission) | A client repeats an unanswered message with a randomized exponential backoff, and gives up into INIT. |
| [DHCP-F-REPLY-DELIVERY](#dhcp-f-reply-delivery) | Where a reply goes follows from `giaddr`, `ciaddr` and the BROADCAST bit. |
| [DHCP-F-SERVER-IDENTITY](#dhcp-f-server-identity) | A server names itself with the `server identifier` option, and a client unicasts to that address. |
| [DHCP-F-CLIENT-IDENTITY](#dhcp-f-client-identity) | A lease belongs to a `client identifier`, or to `chaddr` when the client sends none, and the server echoes the identifier back. |
| [DHCP-F-PARAMETERS](#dhcp-f-parameters) | A client asks for configuration parameters by option code, and the server returns the ones it has, once each. |
| [DHCP-F-ADDRESS-SELECTION](#dhcp-f-address-selection) | A server prefers the address a client held before, and holds an offered address back until the answer comes. |
| [DHCP-F-DUPLICATE-DETECTION](#dhcp-f-duplicate-detection) | Both sides check that an address is free before it goes into service, and the client announces it afterwards. |

## Summary table

| Feature | Level | Sources | Core checks |
| --- | --- | --- | --- |
| [DHCP-F-MESSAGE-FORMAT](#dhcp-f-message-format) | mandatory | RFC 2131 §2, §3, §4.1; RFC 2132 §9.6, §3.2 | RFC2131-MSG-1, MSG-2, MSG-3, MSG-4, RFC2132-TYPE-1 |
| [DHCP-F-TRANSPORT](#dhcp-f-transport) | mandatory | RFC 2131 §4.1, §4.4.1, §4.4.3 | RFC2131-MSG-5, MSG-9, INF-6 |
| [DHCP-F-OPTION-ENCODING](#dhcp-f-option-encoding) | mandatory | RFC 2132 §2; RFC 2131 §4.1 | RFC2132-FMT-1, FMT-3, RFC2131-MSG-11 |
| [DHCP-F-TRANSACTION-ID](#dhcp-f-transaction-id) | mandatory | RFC 2131 §2 table 1, §4.1, §4.4.1, §4.4.5 | RFC2131-XID-1, XID-3, XID-4 |
| [DHCP-F-DISCOVERY](#dhcp-f-discovery) | mandatory | RFC 2131 §3.1, §4.4.1, table 5 | RFC2131-DISC-1, DISC-2, DISC-3, DISC-5 |
| [DHCP-F-OFFER](#dhcp-f-offer) | mandatory | RFC 2131 §3.1, §4.3.1, table 3 | RFC2131-OFF-1, OFF-2, OFF-3, OFF-4, OFF-5, OFF-6 |
| [DHCP-F-SELECTION](#dhcp-f-selection) | mandatory | RFC 2131 §3.1, §4.3.2, §4.4.1, table 4 | RFC2131-REQ-1, REQ-2, REQ-3, REQ-4 |
| [DHCP-F-ACKNOWLEDGEMENT](#dhcp-f-acknowledgement) | mandatory | RFC 2131 §3.1, table 3 | RFC2131-ACK-1, ACK-2, ACK-3, ACK-4, ACK-5, ACK-8 |
| [DHCP-F-LEASE](#dhcp-f-lease) | mandatory | RFC 2131 §2.2, §3.3, §4.4.5; RFC 2132 §9.2, §9.11, §9.12 | RFC2131-LEASE-1, LEASE-2, LEASE-3, RFC2132-LEASE-1 |
| [DHCP-F-RENEW](#dhcp-f-renew) | mandatory | RFC 2131 §4.3.2, §4.4.5 | RFC2131-LEASE-5, REQ-6 |
| [DHCP-F-REBIND](#dhcp-f-rebind) | mandatory | RFC 2131 §4.3.2, §4.4.5 | RFC2131-LEASE-6, REQ-7 |
| [DHCP-F-EXPIRY](#dhcp-f-expiry) | mandatory | RFC 2131 §3.7, §4.4.5 | RFC2131-LEASE-8, LEASE-9 |
| [DHCP-F-INIT-REBOOT](#dhcp-f-init-reboot) | optional | RFC 2131 §3.2, §4.3.2, §4.4.2 | RFC2131-REQ-5 |
| [DHCP-F-NAK](#dhcp-f-nak) | mandatory | RFC 2131 §3.1, §3.2, §4.1, §4.3.2, table 3 | RFC2131-NAK-3, NAK-5, NAK-6, NAK-8 |
| [DHCP-F-DECLINE](#dhcp-f-decline) | mandatory | RFC 2131 §3.1, §4.3.3, §4.4.1, §4.4.4, table 5 | RFC2131-DECL-2, DECL-4, DECL-5, DECL-6 |
| [DHCP-F-RELEASE](#dhcp-f-release) | optional | RFC 2131 §3.1, §4.3.4, §4.4.4, §4.4.6, table 5 | RFC2131-REL-1, REL-3, REL-4, REL-5 |
| [DHCP-F-INFORM](#dhcp-f-inform) | optional | RFC 2131 §3.4, §4.3.5, §4.4.3, table 5 | RFC2131-INF-1, INF-2, INF-3, INF-5 |
| [DHCP-F-RETRANSMISSION](#dhcp-f-retransmission) | mandatory | RFC 2131 §3.1, §4.1 | RFC2131-RETX-1, RETX-3, RETX-4 |
| [DHCP-F-REPLY-DELIVERY](#dhcp-f-reply-delivery) | mandatory | RFC 2131 §4.1 | RFC2131-BCAST-4, BCAST-5, BCAST-3 |
| [DHCP-F-SERVER-IDENTITY](#dhcp-f-server-identity) | mandatory | RFC 2131 §2, §4.1; RFC 2132 §9.7 | RFC2131-SRVID-2, SRVID-3, OFF-8, RFC2132-SRVID-1 |
| [DHCP-F-CLIENT-IDENTITY](#dhcp-f-client-identity) | mandatory | RFC 2131 §2, §4.2; RFC 2132 §9.14; RFC 6842 §3 | RFC2131-ID-2, REQ-9, RFC6842-CLID-1, RFC6842-CLID-2, RFC6842-CLID-3 |
| [DHCP-F-PARAMETERS](#dhcp-f-parameters) | mandatory | RFC 2131 §3.5, §4.3.1; RFC 2132 §9.8, §3.3, §3.5 | RFC2131-SEL-4, SEL-5, REQ-8, RFC2132-PRL-1 |
| [DHCP-F-ADDRESS-SELECTION](#dhcp-f-address-selection) | optional | RFC 2131 §4.3.1 | RFC2131-SEL-1, SEL-2 |
| [DHCP-F-DUPLICATE-DETECTION](#dhcp-f-duplicate-detection) | optional | RFC 2131 §2.2, §3.1, §4.4.1 | RFC2131-OFF-7, DECL-1 |

Twenty-four features, nineteen mandatory and five optional. The count is high next to UDP's
seven, and the reason is the shape of the protocol: DHCP is a state machine with eight
message types, two roles and three timers, and RFC 2131 states a rule for every cell of
that grid. What a run showed about each feature is in
[`coverage.md`](../../model/dhcp/coverage.md).

## DHCP-F-MESSAGE-FORMAT

**Every DHCP message is a BOOTP message whose option area starts with the magic cookie,
names its type, and ends with the `end` option.**

- **Sources** — RFC 2131 §2 figure 1 and table 1, `rfc2131.txt:455-553`; §3,
  `rfc2131.txt:681-703`; §4.1, `rfc2131.txt:1226-1239`; RFC 2132 §9.6,
  `rfc2132.txt:1475-1495`; §3.2, `rfc2132.txt:260-270`.
- **Level** — mandatory (reason: keyword). "One particular option - the "DHCP message type"
  option - must be included in every DHCP message", `rfc2131.txt:694-696`, and "The last
  option must always be the 'end' option", `rfc2131.txt:1230,1239`. It is also the only
  path: without the type option a receiver cannot tell a DHCPOFFER from a DHCPNAK.
- **Description** — the fixed part is the BOOTP header of RFC 951: `op`, `htype`, `hlen`,
  `hops`, `xid`, `secs`, `flags` and four address fields, then `chaddr`, `sname` and `file`.
  The variable part starts with the four octets 99.130.83.99 and holds tagged options, the
  last one being code 255. `op` is 1 from a client and 2 from a server.
- **Checks** — core: RFC2131-MSG-1 (the `op` direction), RFC2131-MSG-2 (the magic cookie),
  RFC2131-MSG-3 (a type option in every message), RFC2131-MSG-4 (the `end` option last),
  RFC2132-TYPE-1 (what the type option holds). Supporting: RFC2131-MSG-7 (the reserved
  `flags` bits are zero), RFC2131-MSG-8 (`hops` is zero), RFC2131-MSG-6 (a client accepts a
  576-octet message), RFC2132-END-1 and RFC2132-PAD-1 (the two options with no length
  octet).

## DHCP-F-TRANSPORT

**DHCP messages travel over UDP between port 67 and port 68, and a client with no address
sends from 0.0.0.0.**

- **Sources** — RFC 2131 §4.1, `rfc2131.txt:1241-1244` and `rfc2131.txt:1267-1269`; §4.4.1,
  `rfc2131.txt:1986-1988`; §4.4.3, `rfc2131.txt:2172-2173`.
- **Level** — mandatory (reason: only path). §4.1 states one transport and two ports and
  gives no alternative. The source address rule carries a lowercase "must",
  `rfc2131.txt:1267-1269`, and the DHCPINFORM port rule a capital `MUST`,
  `rfc2131.txt:2172-2173`.
- **Description** — a client sends to UDP port 67 and a server to UDP port 68. A client that
  has no address yet cannot name a source address, so it sends from 0.0.0.0 and the server
  cannot answer by the usual route; that is why the delivery rules of
  [DHCP-F-REPLY-DELIVERY](#dhcp-f-reply-delivery) exist.
- **Checks** — core: RFC2131-MSG-5 (the two ports), RFC2131-MSG-9 (source address 0.0.0.0
  before configuration), RFC2131-INF-6 (a DHCPINFORM goes to port 67). Supporting:
  RFC2131-DISC-1, which also fixes the destination of the first message.

## DHCP-F-OPTION-ENCODING

**An option is a code, a length and a value, in network byte order, and it may extend into
the `sname` and `file` fields.**

- **Sources** — RFC 2132 §2, `rfc2132.txt:186-241`; RFC 2131 §4.1,
  `rfc2131.txt:1284-1314`.
- **Level** — mandatory (reason: only path). The option area is the only place DHCP carries
  a parameter, and §2 of RFC 2132 gives one encoding for it.
- **Description** — every option except code 0 and code 255 is a tag octet, a length octet
  that counts only the value, and the value. A multi-octet number is big-endian. When the
  options do not fit, the `option overload` option says that `file`, `sname`, or both, hold
  options too, and then strict framing rules apply to those two fields.
- **Checks** — core: RFC2132-FMT-1 (the tag, length, value shape), RFC2132-FMT-3 (network
  byte order), RFC2131-MSG-11 (an option appears once). Supporting: RFC2132-FMT-2 (trailing
  nulls in a text option), RFC2132-FMT-4 (the cookie, from the option side), RFC2132-FMT-5
  (the site-specific range), RFC2131-MSG-10 and RFC2132-OVER-1 (the overload rules),
  RFC2132-TFTP-1 and RFC2132-BOOTF-1 (the two options that replace the overloaded fields).

  Most of these are `encoding` statements, and the table of step 9 files an `encoding`
  statement under a unit test of the serializer, not under a check of two nodes on a link.
  The feature is here so that no area of the RFC 2132 catalog falls outside the map.

## DHCP-F-TRANSACTION-ID

**The `xid` binds a reply to the request it answers, and a reply with a foreign `xid` is
discarded in silence.**

- **Sources** — RFC 2131 table 1, `rfc2131.txt:530-533`; §4.1, `rfc2131.txt:1334-1342`;
  §4.4.1, `rfc2131.txt:1990-1993`; §4.4.5, `rfc2131.txt:2231-2232`; §4.3.1,
  `rfc2131.txt:1657-1660`.
- **Level** — mandatory (reason: keyword). Two `MUST` statements: the client chooses an
  `xid` that is unlikely to collide, `rfc2131.txt:1335-1337`, and a DHCPOFFER with a foreign
  `xid` "must be silently discarded", `rfc2131.txt:1990-1992`.
- **Description** — the client picks a random `xid`, the server copies it into every reply,
  and the client uses it again in the DHCPREQUEST of the same transaction. A client that
  has several requests open tells the replies apart by it, and a client discards a reply
  that carries another client's value. RFC 6842 §2 notes that the `xid` alone cannot do the
  job, because two clients may pick the same value; see
  [DHCP-F-CLIENT-IDENTITY](#dhcp-f-client-identity).
- **Checks** — core: RFC2131-XID-1 (the field and its purpose), RFC2131-XID-3 (one value
  through the whole exchange), RFC2131-XID-4 (a foreign `xid` is discarded in SELECTING).
  Supporting: RFC2131-XID-2 (two clients pick two values), RFC2131-XID-5 (the same discard
  in RENEWING), RFC2131-OFF-2 and RFC2131-ACK-8 (the table cells that copy the value).

## DHCP-F-DISCOVERY

**A client with no address broadcasts a DHCPDISCOVER to find the servers of its subnet.**

- **Sources** — RFC 2131 §3.1, `rfc2131.txt:714-718`; §4.4.1, `rfc2131.txt:1969-1988`;
  table 5, `rfc2131.txt:2023-2107`.
- **Level** — mandatory (reason: only path). A client that knows nothing has one way to
  start, and §4.4.1 gives it.
- **Description** — the client waits a short random time, picks an `xid`, sets `ciaddr` to
  zero, puts its hardware address in `chaddr`, and broadcasts the message to
  255.255.255.255 and port 67. It may suggest an address and a lease time. It may not name a
  server, because it does not know one yet.
- **Checks** — core: RFC2131-DISC-1 (the broadcast destination), RFC2131-DISC-2 (`ciaddr`
  zero), RFC2131-DISC-3 (`chaddr` holds the hardware address), RFC2131-DISC-5 (no `server
  identifier`). Supporting: RFC2131-DISC-4 (the random start delay), RFC2131-DISC-6 (the two
  hints), RFC2131-MISC-1 (one exchange per interface).

## DHCP-F-OFFER

**A server answers a DHCPDISCOVER with a DHCPOFFER that carries a free address, a lease
time and its own identity.**

- **Sources** — RFC 2131 §3.1, `rfc2131.txt:720-739`; §4.3.1, `rfc2131.txt:1467-1660`;
  table 3, `rfc2131.txt:1537-1590`.
- **Level** — mandatory (reason: keyword). Table 3 marks `IP address lease time` and `server
  identifier` as `MUST` in a DHCPOFFER, `rfc2131.txt:1578,1586`, and four other options as
  `MUST NOT`.
- **Description** — the server picks an address and a lease, builds a DHCPOFFER with the
  address in `yiaddr`, copies six fields from the request, and answers. The answer itself is
  a `may`: §4.2 says a server need not respond to every message, `rfc2131.txt:1389-1393`. So
  the level comes from the content of the answer, not from its existence, and a server that
  answers nothing at all is outside what a check can judge.
- **Checks** — core: RFC2131-OFF-1 (an address in `yiaddr`), RFC2131-OFF-2 (the `xid`),
  RFC2131-OFF-3 (the `server identifier`), RFC2131-OFF-4 (the lease time), RFC2131-OFF-5
  (the four prohibited options), RFC2131-OFF-6 (the six copied or zeroed fields).
  Supporting: RFC2131-OFF-7 (the probe before the offer), RFC2131-OFF-8 (the server's own
  address in the identifier), RFC2131-SEL-3 (how the lease is chosen), RFC2132-LEASE-1 and
  RFC2132-SRVID-1 (the two options the offer must carry).

## DHCP-F-SELECTION

**The client picks one offer and claims it with a broadcast DHCPREQUEST that names the
chosen server and the offered address.**

- **Sources** — RFC 2131 §3.1, `rfc2131.txt:847-864`; §4.3.2, `rfc2131.txt:1694-1712`;
  §4.4.1, `rfc2131.txt:1995-2001` and `rfc2131.txt:2111-2117`; table 4,
  `rfc2131.txt:1833-1842`.
- **Level** — mandatory (reason: keyword). Three `MUST` statements in one paragraph: the
  `server identifier` option, the `requested IP address` option set to `yiaddr`, and the
  same `secs` value and broadcast address as the DHCPDISCOVER, `rfc2131.txt:851-862`.
- **Description** — the client collects offers for a while, chooses one by a rule of its own,
  and broadcasts a DHCPREQUEST. The message is broadcast on purpose: the servers that were
  not chosen read it as a refusal of their own offers. Which offer the client picks is an
  implementation decision, `rfc2131.txt:1999-2001`; that it names the choice is not.
- **Checks** — core: RFC2131-REQ-1 (the chosen server), RFC2131-REQ-2 (the offered address),
  RFC2131-REQ-3 (`ciaddr` zero), RFC2131-REQ-4 (broadcast, and the `secs` value).
  Supporting: RFC2131-REQ-8 (the parameter list is repeated), RFC2131-REQ-9 (the client
  identifier is repeated), RFC2132-REQIP-1 (the option the request carries).

## DHCP-F-ACKNOWLEDGEMENT

**The chosen server commits the binding and answers with a DHCPACK that carries the
assigned address and the lease.**

- **Sources** — RFC 2131 §3.1, `rfc2131.txt:866-894`; table 3, `rfc2131.txt:1537-1590`.
- **Level** — mandatory (reason: keyword, and only path). Table 3 marks `IP address lease
  time` and `server identifier` as `MUST` in a DHCPACK. The DHCPACK is also the one message
  that moves a client to BOUND; figure 5 gives no other edge, `rfc2131.txt:1911-1954`.
- **Description** — the server writes the binding to storage and answers with the
  configuration. The client reads the address from `yiaddr` and the duration from option 51,
  and it is configured. The commit before the answer is what makes the address the client's;
  a DHCPOFFER commits nothing.
- **Checks** — core: RFC2131-ACK-1 (the answer from the chosen server), RFC2131-ACK-2
  (`yiaddr` holds the assigned address), RFC2131-ACK-3 (the lease time option),
  RFC2131-ACK-4 (the `server identifier`), RFC2131-ACK-5 (the four prohibited options),
  RFC2131-ACK-8 (the copied fields). Supporting: RFC2131-ACK-6 (no conflict with the offer),
  RFC2131-ACK-7 (no second probe).

## DHCP-F-LEASE

**An address is held for a stated number of seconds, and the client computes the expiry, T1
and T2 from the reply.**

- **Sources** — RFC 2131 §2.2, `rfc2131.txt:640-668`; §3.3, `rfc2131.txt:1093-1113`;
  §4.4.5, `rfc2131.txt:2211-2263`; RFC 2132 §9.2, §9.11, §9.12,
  `rfc2132.txt:1391-1415,1583-1611`.
- **Level** — mandatory (reason: keyword). "T1 MUST be earlier than T2, which, in turn, MUST
  be earlier than the time at which the client's lease will expire", `rfc2131.txt:2216-2218`.
  The lease is also the one thing that separates DHCP from BOOTP, `rfc2131.txt:416-419`.
- **Description** — the server states a duration in seconds in option 51; 0xffffffff means
  the lease never ends. The client adds the duration to the time it sent its DHCPREQUEST,
  not to the time the reply arrived, so a slow reply does not lengthen the lease. Two
  earlier instants divide the lease: T1, when the client tries the leasing server, and T2,
  when it tries any server. Both come from options 58 and 59 when the server sends them, and
  from the fractions 0.5 and 0.875 when it does not.
- **Checks** — core: RFC2131-LEASE-1 (seconds, and the value for infinity), RFC2131-LEASE-2
  (the expiry arithmetic), RFC2131-LEASE-3 (the order T1 < T2 < expiry), RFC2132-LEASE-1
  (the option). Supporting: RFC2131-LEASE-4 (the two default fractions), RFC2131-LEASE-10
  (the random fuzz), RFC2131-LEASE-11 (the server returns T1 and T2), RFC2132-T1-1 and
  RFC2132-T2-1 (the two options).

## DHCP-F-RENEW

**At T1 the client unicasts a DHCPREQUEST to the leasing server to extend the lease.**

- **Sources** — RFC 2131 §4.3.2, `rfc2131.txt:1763-1777`; §4.4.5, `rfc2131.txt:2223-2238`.
- **Level** — mandatory (reason: keyword, and only path). Three `MUST` statements govern the
  fields of the message, `rfc2131.txt:1765-1767`, and figure 5 gives RENEWING as the only
  edge out of BOUND at T1.
- **Description** — the client is configured, so it can name itself: `ciaddr` holds its
  address, and the message is a unicast to the server that gave the lease. It names no
  server in an option and asks for no address, because both are already settled. A DHCPACK
  returns it to BOUND with a fresh lease.
- **Checks** — core: RFC2131-LEASE-5 (the unicast at T1), RFC2131-REQ-6 (the three field
  rules). Supporting: RFC2131-SRVID-3 (the unicast goes to the `server identifier` address),
  RFC2131-BCAST-3 (the answer is a unicast to `ciaddr`), RFC2131-LEASE-7 (the wait before a
  repeat), RFC2131-XID-5 (a foreign `xid` is discarded).

## DHCP-F-REBIND

**At T2, with the renewal unanswered, the client broadcasts a DHCPREQUEST to any server.**

- **Sources** — RFC 2131 §4.3.2, `rfc2131.txt:1779-1803`; §4.4.5, `rfc2131.txt:2247-2251`.
- **Level** — mandatory (reason: keyword). "This message MUST be broadcast to the 0xffffffff
  IP broadcast address", `rfc2131.txt:1784-1785`, and figure 5 gives REBINDING as the only
  edge out of RENEWING at T2.
- **Description** — the leasing server did not answer, so the client stops addressing it and
  asks the whole subnet. The fields are the ones of the renewal; only the destination
  changes. Any server with authority over the lease may answer.
- **Checks** — core: RFC2131-LEASE-6 (the broadcast at T2), RFC2131-REQ-7 (the field rules
  and the broadcast). Supporting: RFC2131-LEASE-7 (the wait before a repeat).

## DHCP-F-EXPIRY

**When the lease ends without a DHCPACK, the client stops using the address at once and
starts again.**

- **Sources** — RFC 2131 §3.7, `rfc2131.txt:1210-1215`; §4.4.5, `rfc2131.txt:2271-2278`.
- **Level** — mandatory (reason: keyword). "MUST immediately stop any other network
  processing", `rfc2131.txt:2272-2273`, and "it MUST NOT continue using the previous network
  address", `rfc2131.txt:2276-2277`.
- **Description** — the lease is a promise with an end. When the end comes without a renewal,
  the address is no longer the client's, and the client must stop using it in the same
  instant, not at the next convenient moment. It then behaves as if it had just started. If
  the new exchange returns the same address, it may carry on; if it returns a different one,
  the old address is gone for good.
- **Checks** — core: RFC2131-LEASE-8 (stop at once, and restart), RFC2131-LEASE-9 (do not
  use the old address after a new one). Supporting: RFC2131-DISC-1, the DHCPDISCOVER that
  the restart produces.

## DHCP-F-INIT-REBOOT

**A client that remembers an address confirms it with a DHCPREQUEST that names no server.**

- **Sources** — RFC 2131 §3.2, `rfc2131.txt:938-942` and `rfc2131.txt:959-966`; §4.3.2,
  `rfc2131.txt:1714-1761`; §4.4.2, `rfc2131.txt:2140-2159`.
- **Level** — optional (reason: keyword). "If a client remembers and wishes to reuse a
  previously allocated network address, a client may choose to omit some of the steps
  described in the previous section", `rfc2131.txt:938-941`. A client that keeps nothing
  across a restart never enters the state. The field rules of RFC2131-REQ-5 are a `MUST`,
  but a conditional one: they apply once the client chooses this path. See
  [how this map decides the level](#how-this-map-decides-the-level).
- **Description** — the client skips the DHCPDISCOVER and the DHCPOFFER and goes straight to
  a broadcast DHCPREQUEST that carries the remembered address in option 50. It names no
  server, because it wants any server that knows the binding to answer. `ciaddr` stays zero,
  because the client does not yet have the right to the address it is asking about.
- **Checks** — core: RFC2131-REQ-5 (the remembered address, no server identifier, `ciaddr`
  zero). Supporting: RFC2131-NAK-2 (a wrong address or a wrong subnet draws a DHCPNAK),
  RFC2131-NAK-7 (a server that knows nothing stays silent), RFC2131-RETX-4 (what happens
  when nobody answers).

## DHCP-F-NAK

**A server that cannot satisfy a request answers with a DHCPNAK, broadcast when no relay
agent is in the path, and the client starts again.**

- **Sources** — RFC 2131 §3.1, `rfc2131.txt:882-884`; §3.2, `rfc2131.txt:1015-1032` and
  `rfc2131.txt:1048-1053`; §4.1, `rfc2131.txt:1281-1282`; §4.3.2, `rfc2131.txt:1719-1761`;
  table 3, `rfc2131.txt:1537-1590`.
- **Level** — mandatory (reason: keyword, and only path). The delivery rule is a `MUST`:
  "The server MUST broadcast the DHCPNAK message to the 0xffffffff broadcast address",
  `rfc2131.txt:1752-1753`. Table 3 marks `server identifier` as `MUST` and every other
  option as `MUST NOT`. And the client's answer to a DHCPNAK is the only edge back to INIT
  from three states of figure 5.
- **Description** — a DHCPNAK is the one negative message of the protocol, and it says: the
  address you ask about is not yours. It carries no address and no lease, because there is
  nothing to give. It always goes to the broadcast address when the client is on the
  server's own subnet, because a client that gets a DHCPNAK may hold a wrong address or a
  wrong mask and may answer no ARP request. The client abandons the address and starts from
  INIT.
- **Checks** — core: RFC2131-NAK-3 (the broadcast delivery), RFC2131-NAK-5 (the empty
  fields and the prohibited options), RFC2131-NAK-6 (the `server identifier`), RFC2131-NAK-8
  (the client restarts). Supporting: RFC2131-NAK-1 (a DHCPNAK when the address is taken),
  RFC2131-NAK-2 (a DHCPNAK for a wrong address or subnet), RFC2131-NAK-7 (silence when the
  server knows nothing), RFC2131-NAK-4 (the relay agent case), RFC2132-MSGOPT-1 (the text
  option a DHCPNAK may carry).

## DHCP-F-DECLINE

**A client that finds the assigned address in use sends a DHCPDECLINE and starts again, and
the server takes the address out of service.**

- **Sources** — RFC 2131 §3.1, `rfc2131.txt:903-908`; §4.3.3, `rfc2131.txt:1805-1811`;
  §4.4.1, `rfc2131.txt:2135-2136`; §4.4.4, `rfc2131.txt:2197-2199`; table 5,
  `rfc2131.txt:2023-2107`.
- **Level** — mandatory (reason: keyword). "the client MUST send a DHCPDECLINE message to
  the server", `rfc2131.txt:904-905`, and "The server MUST mark the network address as not
  available", `rfc2131.txt:1809-1810`.
- **Description** — DHCP promises that no address serves two clients at once, and the
  DHCPDECLINE is what keeps the promise when the promise has already broken. The client
  found the address in use, so it refuses it, tells the server, and starts over. The server
  must believe the client and stop giving that address out. The message is broadcast,
  because the client is giving up the address it would have answered from.
- **Checks** — core: RFC2131-DECL-2 (send a DHCPDECLINE and restart), RFC2131-DECL-4 (the
  server marks the address unavailable), RFC2131-DECL-5 (the broadcast), RFC2131-DECL-6 (the
  two required options and the prohibited ones). Supporting: RFC2131-DECL-3 (the ten-second
  wait), RFC2131-DECL-1 and RFC2131-DECL-7 (the ARP probe that finds the conflict, which is
  [DHCP-F-DUPLICATE-DETECTION](#dhcp-f-duplicate-detection)).

## DHCP-F-RELEASE

**A client gives an address back with a unicast DHCPRELEASE, and the server frees it.**

- **Sources** — RFC 2131 §3.1, `rfc2131.txt:928-933`; §4.3.4, `rfc2131.txt:1813-1818`;
  §4.4.4, `rfc2131.txt:2197`; §4.4.6, `rfc2131.txt:2280-2285`; table 5,
  `rfc2131.txt:2023-2107`.
- **Level** — optional (reason: keyword, and an explicit non-requirement). "The client may
  choose to relinquish its lease", `rfc2131.txt:928-929`, and §4.4.6 says plainly: "the
  correct operation of DHCP does not depend on the transmission of DHCPRELEASE messages",
  `rfc2131.txt:2284-2285`. The `MUST` statements of RFC2131-REL-2 and RFC2131-REL-5 are
  conditional on the client sending one.
- **Description** — a client that shuts down cleanly may hand the address back, and the
  server may then give it to somebody else at once instead of waiting for the lease to end.
  The message is a unicast, because the client still has its address and knows the server.
  Nothing breaks when the message never comes; the lease runs out instead.
- **Checks** — core: RFC2131-REL-1 (the message exists), RFC2131-REL-3 (the unicast),
  RFC2131-REL-4 (the server frees the address), RFC2131-REL-5 (the field and option rules).
  Supporting: RFC2131-REL-2 (the client identifier is repeated), RFC2131-SRVID-3 (the
  unicast destination), RFC2131-SEL-1 (the released address is preferred for the same client
  next time).

## DHCP-F-INFORM

**A client that already has an address asks for the other parameters alone with a
DHCPINFORM.**

- **Sources** — RFC 2131 §3.4, `rfc2131.txt:1115-1139`; §4.3.5, `rfc2131.txt:1820-1826`;
  §4.4.3, `rfc2131.txt:2161-2183`; table 5, `rfc2131.txt:2023-2107`.
- **Level** — optional (reason: keyword). "If a client has obtained a network address through
  some other means (e.g., manual configuration), it may use a DHCPINFORM request message to
  obtain other local configuration parameters", `rfc2131.txt:1117-1119`. The `MUST NOT`
  statements of §4.3.5 apply once a DHCPINFORM arrives. See
  [how this map decides the level](#how-this-map-decides-the-level).
- **Description** — the message separates the two services of DHCP. A statically addressed
  host needs the second one — the subnet mask, the routers, the name servers — and none of
  the first. So the server answers with a DHCPACK that carries parameters and no address and
  no lease, allocates nothing, and does not even look for a binding. The DHCPINFORM is the
  one message type that RFC 1541 did not have; RFC 2131 §1.1 added it,
  `rfc2131.txt:158-160`.
- **Checks** — core: RFC2131-INF-1 (the client sends one with its address in `ciaddr`),
  RFC2131-INF-2 (the unicast DHCPACK to `ciaddr`), RFC2131-INF-3 (no lease and no `yiaddr`
  in the answer), RFC2131-INF-5 (the two prohibited options). Supporting: RFC2131-INF-4 (the
  server looks for no lease), RFC2131-INF-6 (the destination port), RFC2131-ACK-3 (the
  `MUST NOT` half of the lease time rule).

## DHCP-F-RETRANSMISSION

**A client repeats an unanswered message with a randomized exponential backoff, and gives up
into INIT.**

- **Sources** — RFC 2131 §3.1, `rfc2131.txt:863-864` and `rfc2131.txt:913-926`; §4.1,
  `rfc2131.txt:1316-1332`.
- **Level** — mandatory (reason: keyword). "The client MUST adopt a retransmission strategy
  that incorporates a randomized exponential backoff algorithm", `rfc2131.txt:1317-1319`.
- **Description** — DHCP runs on UDP, so nothing below it repeats a lost message, and §4.1
  puts the whole duty on the client. The delay doubles, which keeps a busy subnet from
  drowning, and it carries a random part, which keeps many clients from repeating in step.
  After enough tries the client gives up on the current state and returns to INIT.
- **Checks** — core: RFC2131-RETX-1 (the backoff), RFC2131-RETX-3 (repeat the
  DHCPDISCOVER), RFC2131-RETX-4 (repeat the DHCPREQUEST, then revert to INIT). Supporting:
  RFC2131-RETX-2 (the 4, 8, 64-second figures), RFC2131-LEASE-7 (the different rule that
  holds in RENEWING and REBINDING), RFC2131-DISC-4 (the random start delay).

  RFC2131-RETX-2 states a distribution, not a value, and the guide files a distribution at
  level 4. The entry stays `later` until a statistical check with a stated tolerance exists.

## DHCP-F-REPLY-DELIVERY

**Where a reply goes follows from `giaddr`, `ciaddr` and the BROADCAST bit.**

- **Sources** — RFC 2131 §2, `rfc2131.txt:582-601`; §4.1, `rfc2131.txt:1271-1282` and
  `rfc2131.txt:1351-1385`.
- **Level** — mandatory (reason: only path). §4.1 gives four rules for the destination of a
  DHCPOFFER or a DHCPACK and a fifth for a DHCPNAK, and no other rule exists. A server that
  followed none of them could not reach a client that has no address.
- **Description** — a client that is not configured yet may be unable to receive a unicast IP
  datagram, which is a deadlock: the address cannot arrive until the address is set. The
  BROADCAST bit is how the client warns the server. The server then reads three values in
  order — `giaddr` first, `ciaddr` next, the bit last — and picks a broadcast, a unicast to
  `ciaddr`, or a unicast to `chaddr` and `yiaddr`.
- **Checks** — core: RFC2131-BCAST-4 (broadcast when the bit is set), RFC2131-BCAST-5
  (unicast to `chaddr` and `yiaddr` when it is clear), RFC2131-BCAST-3 (unicast to `ciaddr`
  for a renewal). Supporting: RFC2131-BCAST-1 (the client sets the bit), RFC2131-BCAST-6
  (the server reads the bit), RFC2131-BCAST-2 and RFC2131-NAK-4 (the two relay agent rules),
  RFC2131-MSG-7 (the other `flags` bits stay zero).

  RFC2131-BCAST-2 and RFC2131-NAK-4 hold only when a relay agent put a value in `giaddr`.
  RFC 1542 is out of the in-scope set, so a test network of this pass has no relay agent and
  the two statements cannot be reached; see
  [`standards.md`](standards.md#in-scope-set).

## DHCP-F-SERVER-IDENTITY

**A server names itself with the `server identifier` option, and a client unicasts to that
address.**

- **Sources** — RFC 2131 §2, `rfc2131.txt:511-517`; §4.1, `rfc2131.txt:1247-1265`;
  RFC 2132 §9.7, `rfc2132.txt:1497-1522`.
- **Level** — mandatory (reason: keyword). Three `MUST` statements in §4.1: a server accepts
  any of its addresses as its identifier, a server chooses a reachable one, and a client uses
  that address for every unicast.
- **Description** — the option does two jobs at once. It tells a client which server made an
  offer, so the client can accept one offer and refuse the others, and it is the address the
  client sends to afterwards. The two jobs are why a multi-homed server must choose the
  address that faces the client: an unreachable identifier makes every later unicast fail.
  The `siaddr` field is a different thing — the next server of a bootstrap — and §2 keeps
  the two apart, `rfc2131.txt:511-517`.
- **Checks** — core: RFC2131-SRVID-2 (a reachable identifier), RFC2131-SRVID-3 (the client
  unicasts to it), RFC2131-OFF-8 (the server returns its own address), RFC2132-SRVID-1 (the
  option). Supporting: RFC2131-SRVID-1 (a server accepts any of its addresses),
  RFC2131-OFF-3, RFC2131-ACK-4 and RFC2131-NAK-6 (the three messages that must carry it),
  RFC2131-REQ-1 (the client names its choice with it).

## DHCP-F-CLIENT-IDENTITY

**A lease belongs to a `client identifier`, or to `chaddr` when the client sends none, and
the server echoes the identifier back.**

- **Sources** — RFC 2131 §2, `rfc2131.txt:489-501`; §3.1, `rfc2131.txt:872-875`; §4.2,
  `rfc2131.txt:1415-1432`; RFC 2132 §9.14, `rfc2132.txt:1641-1669`; RFC 6842 §3,
  `rfc6842.txt:156-186`, which governs the reply.
- **Level** — mandatory (reason: keyword). "the server MUST use that identifier to identify
  the client. If the client does not provide a 'client identifier' option, the server MUST
  use the contents of the 'chaddr' field", `rfc2131.txt:1418-1422`. Whether the client sends
  an option 61 is a `MAY` of table 5, but that the server identifies the client somehow is
  not optional.
- **Description** — a server keeps one binding per client, so it needs a key. The key is the
  `client identifier` option when the client offers one, and `chaddr` otherwise. RFC 2131
  forbade the server to echo the identifier back, to save room in the message. RFC 6842
  reversed that: a client whose `chaddr` is all zeroes, and two clients that share one
  hardware address, cannot tell whether a reply is theirs, because the `xid` is not unique
  across clients either. So the server now returns the option unaltered, and the client
  discards a reply whose identifier is not its own.
- **Checks** — core: RFC2131-ID-2 (the server's key), RFC2131-REQ-9 (the client keeps one
  value), RFC6842-CLID-1 (the server echoes it), RFC6842-CLID-2 (and echoes none when none
  came), RFC6842-CLID-3 (the client discards a foreign one). Supporting: RFC2131-ID-1 and
  RFC2132-CLID-1 (the uniqueness rule), RFC2131-REL-2 (the identifier in a DHCPRELEASE),
  RFC2131-OFF-5 and RFC2131-ACK-5, whose `client identifier` rows RFC 6842 overrides.

## DHCP-F-PARAMETERS

**A client asks for configuration parameters by option code, and the server returns the ones
it has, once each.**

- **Sources** — RFC 2131 §3.5, `rfc2131.txt:1141-1194`; §4.3.1, `rfc2131.txt:1592-1660`;
  RFC 2132 §9.8, `rfc2132.txt:1524-1541`; §3.3 and §3.5, `rfc2132.txt:272-321`.
- **Level** — mandatory (reason: keyword). Six `MUST` statements and one `MUST NOT` in the
  list of §4.3.1, `rfc2131.txt:1601-1635`.
- **Description** — this is the first of the two services of DHCP, and the one BOOTP already
  had. The client lists the option codes it wants in option 55. The server walks a fixed
  order of sources — its own configuration, then the Host Requirements defaults, then
  nothing — and returns what it finds. It must omit what it has no value for rather than
  invent one, and it must not repeat an option code. A client that asked for a list in its
  DHCPDISCOVER must ask for the same list later, so that every server sees the same request.
- **Checks** — core: RFC2131-SEL-4 (the address, the lease and the requested parameters),
  RFC2131-SEL-5 (omit what is missing, no duplicates), RFC2131-REQ-8 (the list is
  repeated), RFC2132-PRL-1 (the option). Supporting: RFC2132-MASK-1 and RFC2132-ROUTER-1
  (the two parameters an IPv4 host cannot work without), RFC2132-VCLASS-1 (a server ignores
  a vendor class it does not know), RFC2132-MAXSZ-1 and RFC2131-MSG-6 (how large a reply may
  be), RFC2131-ACK-6 (the parameters do not change between the offer and the
  acknowledgement).

## DHCP-F-ADDRESS-SELECTION

**A server prefers the address a client held before, and holds an offered address back until
the answer comes.**

- **Sources** — RFC 2131 §4.3.1, `rfc2131.txt:1467-1507`.
- **Level** — optional (reason: keyword, `should` and `should not`). "the new address SHOULD
  be chosen as follows", `rfc2131.txt:1472-1473`, and "the server SHOULD NOT reuse the
  selected network address before the client responds", `rfc2131.txt:1504-1506`.
- **Description** — one of the design goals of §1.6 is that a client keeps its configuration
  across a restart, `rfc2131.txt:376-379`, and this rule is how a server reaches it: the
  current binding first, then the expired one, then what the client asked for, then a fresh
  address. None of it is required, and a server that always gives out the next free address
  still conforms. Which is the point of the `SHOULD`: the behavior is worth having and no
  exchange breaks without it.
- **Checks** — core: RFC2131-SEL-1 (the order of preference), RFC2131-SEL-2 (do not reuse a
  pending offer). Supporting: RFC2131-DISC-6 (the client's hint), RFC2131-SEL-3 (the same
  kind of rule for the lease duration), RFC2131-REL-4 (a released address returns to the
  pool), RFC2131-DECL-4 (a declined address leaves the pool).

## DHCP-F-DUPLICATE-DETECTION

**Both sides check that an address is free before it goes into service, and the client
announces it afterwards.**

- **Sources** — RFC 2131 §2.2, `rfc2131.txt:659-668`; §3.1, `rfc2131.txt:726-738` and
  `rfc2131.txt:891-894`; §4.4.1, `rfc2131.txt:2119-2138`.
- **Level** — optional (reason: keyword, `should` throughout). "the allocating server SHOULD
  probe the reused address before allocating the address, e.g., with an ICMP echo request,
  and the client SHOULD probe the newly received address, e.g., with ARP",
  `rfc2131.txt:666-668`.
- **Description** — DHCP has to guarantee that one address never serves two clients, and its
  own records are not enough for that: a statically configured host that DHCP never heard of
  may hold the address. So both sides look. The server sends an ICMP echo request before it
  offers, the client sends an ARP request before it uses, and the ARP request carries sender
  address 0.0.0.0 so that no other host learns a binding that may not hold. After the
  address is in service the client announces it, which clears stale caches elsewhere on the
  subnet. What happens when the probe finds an answer is
  [DHCP-F-DECLINE](#dhcp-f-decline).
- **Checks** — core: RFC2131-OFF-7 (the server's probe), RFC2131-DECL-1 (the client's
  probe). Supporting: RFC2131-DECL-7 (the fields of the ARP probe), RFC2131-DECL-8 (the
  announcement), RFC2131-ACK-7 (the server does not probe a second time).

## Coverage of the catalogs

Every area of every in-scope catalog appears in at least one feature.

| Catalog area | Feature |
| --- | --- |
| RFC 2131, Message format and transport | DHCP-F-MESSAGE-FORMAT, DHCP-F-TRANSPORT, DHCP-F-OPTION-ENCODING, DHCP-F-REPLY-DELIVERY |
| RFC 2131, Transaction identifier | DHCP-F-TRANSACTION-ID |
| RFC 2131, Discovery | DHCP-F-DISCOVERY |
| RFC 2131, Offer | DHCP-F-OFFER, DHCP-F-SERVER-IDENTITY, DHCP-F-DUPLICATE-DETECTION |
| RFC 2131, Request | DHCP-F-SELECTION, DHCP-F-INIT-REBOOT, DHCP-F-RENEW, DHCP-F-REBIND, DHCP-F-CLIENT-IDENTITY, DHCP-F-PARAMETERS |
| RFC 2131, Acknowledgement | DHCP-F-ACKNOWLEDGEMENT, DHCP-F-INFORM, DHCP-F-DUPLICATE-DETECTION |
| RFC 2131, Negative acknowledgement | DHCP-F-NAK, DHCP-F-INIT-REBOOT |
| RFC 2131, Lease and reacquisition | DHCP-F-LEASE, DHCP-F-RENEW, DHCP-F-REBIND, DHCP-F-EXPIRY |
| RFC 2131, Retransmission | DHCP-F-RETRANSMISSION |
| RFC 2131, Decline | DHCP-F-DECLINE, DHCP-F-DUPLICATE-DETECTION |
| RFC 2131, Release | DHCP-F-RELEASE |
| RFC 2131, Inform | DHCP-F-INFORM |
| RFC 2131, Broadcast bit and reply delivery | DHCP-F-REPLY-DELIVERY |
| RFC 2131, Server identifier | DHCP-F-SERVER-IDENTITY |
| RFC 2131, Client identity | DHCP-F-CLIENT-IDENTITY |
| RFC 2131, Server selection rules | DHCP-F-PARAMETERS, DHCP-F-ADDRESS-SELECTION, DHCP-F-OFFER |
| RFC 2131, Other | DHCP-F-DISCOVERY (one exchange per interface) |
| RFC 2132, Option field format | DHCP-F-OPTION-ENCODING, DHCP-F-MESSAGE-FORMAT |
| RFC 2132, RFC 1497 extensions that DHCP needs | DHCP-F-MESSAGE-FORMAT (pad, end), DHCP-F-PARAMETERS (subnet mask, router) |
| RFC 2132, DHCP extensions | DHCP-F-OFFER, DHCP-F-SELECTION, DHCP-F-LEASE, DHCP-F-SERVER-IDENTITY, DHCP-F-CLIENT-IDENTITY, DHCP-F-PARAMETERS, DHCP-F-NAK, DHCP-F-OPTION-ENCODING |
| RFC 6842, The client identifier in a reply | DHCP-F-CLIENT-IDENTITY |

All 106 entries of the RFC 2131 catalog, all 23 of the RFC 2132 catalog and all 3 of the
RFC 6842 catalog appear in the map. Several entries sit in two places on purpose. The
clearest case is the DHCPDECLINE group: the ARP probe that finds the conflict belongs to
[DHCP-F-DUPLICATE-DETECTION](#dhcp-f-duplicate-detection) and the message that answers it
to [DHCP-F-DECLINE](#dhcp-f-decline), and a check of one needs the other to have happened
first.
