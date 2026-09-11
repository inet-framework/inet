# RFC 2131 (DHCP) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC2131-*` · **Stands on:** [standards.md](../../protocol/dhcp/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow, for the base document
of the in-scope set: RFC 2131, the Dynamic Host Configuration Protocol of March 1997. The
catalog comes from the RFC text only. It contains no simulation model names and no code
references.

Source, cached in this folder:

- `rfc2131.txt` — Dynamic Host Configuration Protocol, March 1997. Downloaded 2026-09-11
  from <https://www.rfc-editor.org/rfc/rfc2131.txt>.

The options that every DHCP message carries belong to RFC 2132 and live in
[`rfc2132/catalog.md`](../rfc2132/catalog.md). The one later document that changes a rule
of this text is RFC 6842, in [`rfc6842/catalog.md`](../rfc6842/catalog.md). The family of
documents and the in-scope set are in
[`standards.md`](../../protocol/dhcp/standards.md). The features these statements build
are in [`features.md`](../../protocol/dhcp/features.md).

The state of the workflow — which statement a check targets, which test carries it, and
what the run said — is **not** in this document. It lives in the coverage ledger,
[`dhcp/coverage.md`](../../model/dhcp/coverage.md). Keeping it out is deliberate: this
catalog states what the standard says, so a new test or a new run must never force an edit
here.

Quotes are verbatim. A reference such as `rfc2131.txt:1241` points to a line of the cached
file in this folder. Where a quote crosses a page break of the RFC, the reference names
both line groups.

## How to read an entry

- **ID** — `RFC2131-AREA-n`. The ID is stable forever. New entries go at the end of an area.
- **Quote** — the verbatim sentence, with a line reference into the cached file.
- **Strength** — the word the document uses. RFC 2131 §1.4 defines `MUST`, `MUST NOT`,
  `SHOULD`, `SHOULD NOT` and `MAY` in capitals, `rfc2131.txt:243-293`. The document also
  states rules in lowercase prose and in two tables. The entry records what is written:
  `must`, `must not`, `should`, `should not`, `may`, or `description` for normative prose
  with no keyword. Where the keyword is lowercase, the entry says so.
- **Class** — how a test can observe the statement: `wire` (fields of messages on a link),
  `end-to-end` (what a node accepts and acts on), `error-signal` (a report message),
  `internal` (state inside a node), `encoding` (the exact bit layout).
- **Check idea** — one or two sentences, still without implementation names.
- **Overridden by** — present only when RFC 6842 changes the statement.

Two tables of this document carry many of its rules. Table 3, `rfc2131.txt:1537-1590`,
states which field and which option each server message uses. Table 5,
`rfc2131.txt:2023-2109`, does the same for the client messages. Table 4,
`rfc2131.txt:1833-1842`, states the field rules of a DHCPREQUEST per client state. A cell
of a table is a statement, and this catalog gives each group of cells an entry.

## Index

| ID | Statement |
| --- | --- |
| [RFC2131-MSG-1](#rfc2131-msg-1) | A client message carries BOOTREQUEST in `op` and a server message carries BOOTREPLY. |
| [RFC2131-MSG-2](#rfc2131-msg-2) | The first four octets of the `options` field are the magic cookie 99, 130, 83, 99. |
| [RFC2131-MSG-3](#rfc2131-msg-3) | Every DHCP message carries the `DHCP message type` option. |
| [RFC2131-MSG-4](#rfc2131-msg-4) | The last option is always the `end` option. |
| [RFC2131-MSG-5](#rfc2131-msg-5) | DHCP runs over UDP: a client sends to port 67 and a server sends to port 68. |
| [RFC2131-MSG-6](#rfc2131-msg-6) | A client must accept an `options` field of at least 312 octets, that is a message of 576 octets. |
| [RFC2131-MSG-7](#rfc2131-msg-7) | The reserved bits of `flags` are zero from the client and ignored by the server. |
| [RFC2131-MSG-8](#rfc2131-msg-8) | The client sets `hops` to zero. |
| [RFC2131-MSG-9](#rfc2131-msg-9) | A message that a client broadcasts before it has an address carries IP source address 0. |
| [RFC2131-MSG-10](#rfc2131-msg-10) | Options that extend into `sname` or `file` need the `option overload` option and follow strict framing rules. |
| [RFC2131-MSG-11](#rfc2131-msg-11) | An option appears only once, and the client concatenates the values of several instances. |
| [RFC2131-XID-1](#rfc2131-xid-1) | `xid` is a random number of the client that binds a response to a request. |
| [RFC2131-XID-2](#rfc2131-xid-2) | A client chooses an `xid` that is unlikely to collide with another client's. |
| [RFC2131-XID-3](#rfc2131-xid-3) | A server copies the `xid` of the request into its reply, and the client reuses it in the DHCPREQUEST. |
| [RFC2131-XID-4](#rfc2131-xid-4) | In SELECTING, a DHCPOFFER with a foreign `xid`, and any DHCPACK, is discarded in silence. |
| [RFC2131-XID-5](#rfc2131-xid-5) | In RENEWING, a DHCPACK with a foreign `xid` is discarded in silence. |
| [RFC2131-DISC-1](#rfc2131-disc-1) | The client broadcasts DHCPDISCOVER to 255.255.255.255 and the server port. |
| [RFC2131-DISC-2](#rfc2131-disc-2) | `ciaddr` is zero in a DHCPDISCOVER. |
| [RFC2131-DISC-3](#rfc2131-disc-3) | The client puts its hardware address in `chaddr`. |
| [RFC2131-DISC-4](#rfc2131-disc-4) | The client waits a random time of one to ten seconds before the first DHCPDISCOVER. |
| [RFC2131-DISC-5](#rfc2131-disc-5) | A DHCPDISCOVER carries no `server identifier` option. |
| [RFC2131-DISC-6](#rfc2131-disc-6) | A DHCPDISCOVER may suggest an address and a lease time. |
| [RFC2131-OFF-1](#rfc2131-off-1) | A server may answer a DHCPDISCOVER with a DHCPOFFER that carries a free address in `yiaddr`. |
| [RFC2131-OFF-2](#rfc2131-off-2) | The DHCPOFFER carries the `xid` of the DHCPDISCOVER. |
| [RFC2131-OFF-3](#rfc2131-off-3) | A DHCPOFFER carries the `server identifier` option. |
| [RFC2131-OFF-4](#rfc2131-off-4) | A DHCPOFFER carries the `IP address lease time` option. |
| [RFC2131-OFF-5](#rfc2131-off-5) | A DHCPOFFER carries no `requested IP address`, `parameter request list`, `client identifier` or `maximum message size` option. |
| [RFC2131-OFF-6](#rfc2131-off-6) | A DHCPOFFER carries `hops` 0, `secs` 0, `ciaddr` 0, and the `flags`, `giaddr` and `chaddr` of the DHCPDISCOVER. |
| [RFC2131-OFF-7](#rfc2131-off-7) | A server should check that the address it offers is not already in use. |
| [RFC2131-OFF-8](#rfc2131-off-8) | A server always puts its own address in the `server identifier` option. |
| [RFC2131-REQ-1](#rfc2131-req-1) | A DHCPREQUEST from SELECTING carries the `server identifier` option of the chosen server. |
| [RFC2131-REQ-2](#rfc2131-req-2) | A DHCPREQUEST from SELECTING carries the offered `yiaddr` in the `requested IP address` option. |
| [RFC2131-REQ-3](#rfc2131-req-3) | `ciaddr` is zero in a DHCPREQUEST from SELECTING and from INIT-REBOOT. |
| [RFC2131-REQ-4](#rfc2131-req-4) | The DHCPREQUEST from SELECTING is broadcast, and it repeats the `secs` value and the broadcast address of the DHCPDISCOVER. |
| [RFC2131-REQ-5](#rfc2131-req-5) | A DHCPREQUEST from INIT-REBOOT carries the remembered address and no `server identifier`. |
| [RFC2131-REQ-6](#rfc2131-req-6) | A DHCPREQUEST from RENEWING is unicast, carries `ciaddr`, and carries neither `server identifier` nor `requested IP address`. |
| [RFC2131-REQ-7](#rfc2131-req-7) | A DHCPREQUEST from REBINDING is broadcast, and carries the same fields as the one from RENEWING. |
| [RFC2131-REQ-8](#rfc2131-req-8) | A client that asked for a parameter list repeats that list in every later message. |
| [RFC2131-REQ-9](#rfc2131-req-9) | A client that used a `client identifier` uses the same one in every later message. |
| [RFC2131-ACK-1](#rfc2131-ack-1) | The chosen server commits the binding and answers with a DHCPACK. |
| [RFC2131-ACK-2](#rfc2131-ack-2) | The `yiaddr` of the DHCPACK is the assigned address. |
| [RFC2131-ACK-3](#rfc2131-ack-3) | A DHCPACK that answers a DHCPREQUEST carries the `IP address lease time` option; one that answers a DHCPINFORM carries none. |
| [RFC2131-ACK-4](#rfc2131-ack-4) | A DHCPACK carries the `server identifier` option. |
| [RFC2131-ACK-5](#rfc2131-ack-5) | A DHCPACK carries no `requested IP address`, `parameter request list`, `client identifier` or `maximum message size` option. |
| [RFC2131-ACK-6](#rfc2131-ack-6) | The parameters of the DHCPACK should not conflict with those of the DHCPOFFER. |
| [RFC2131-ACK-7](#rfc2131-ack-7) | The server should not probe the address again when it answers the DHCPREQUEST. |
| [RFC2131-ACK-8](#rfc2131-ack-8) | A DHCPACK carries the `xid`, `flags`, `giaddr` and `chaddr` of the DHCPREQUEST, `secs` 0 and `hops` 0. |
| [RFC2131-NAK-1](#rfc2131-nak-1) | A chosen server that cannot satisfy the DHCPREQUEST should answer with a DHCPNAK. |
| [RFC2131-NAK-2](#rfc2131-nak-2) | A server should answer a request for a wrong address or a wrong subnet with a DHCPNAK. |
| [RFC2131-NAK-3](#rfc2131-nak-3) | When `giaddr` is zero, the server broadcasts the DHCPNAK to 255.255.255.255. |
| [RFC2131-NAK-4](#rfc2131-nak-4) | When `giaddr` is set, the server sends the DHCPNAK to the relay agent and sets the broadcast bit. |
| [RFC2131-NAK-5](#rfc2131-nak-5) | A DHCPNAK carries `yiaddr` 0, `ciaddr` 0, `siaddr` 0, and no lease time and no other configuration option. |
| [RFC2131-NAK-6](#rfc2131-nak-6) | A DHCPNAK carries the `server identifier` option. |
| [RFC2131-NAK-7](#rfc2131-nak-7) | A server with no record of a client in INIT-REBOOT stays silent. |
| [RFC2131-NAK-8](#rfc2131-nak-8) | A client that gets a DHCPNAK restarts the configuration process. |
| [RFC2131-LEASE-1](#rfc2131-lease-1) | Times are in seconds, and 0xffffffff means infinity. |
| [RFC2131-LEASE-2](#rfc2131-lease-2) | The client computes the expiry as the send time of its DHCPREQUEST plus the lease of the DHCPACK. |
| [RFC2131-LEASE-3](#rfc2131-lease-3) | T1 is earlier than T2, and T2 is earlier than the expiry. |
| [RFC2131-LEASE-4](#rfc2131-lease-4) | T1 defaults to half the lease and T2 to 0.875 of the lease. |
| [RFC2131-LEASE-5](#rfc2131-lease-5) | At T1 the client unicasts a DHCPREQUEST to the leasing server. |
| [RFC2131-LEASE-6](#rfc2131-lease-6) | At T2 the client broadcasts a DHCPREQUEST to any server. |
| [RFC2131-LEASE-7](#rfc2131-lease-7) | Without an answer the client waits half of the remaining time, at least 60 seconds, before it repeats. |
| [RFC2131-LEASE-8](#rfc2131-lease-8) | When the lease expires the client goes to INIT and stops all other network processing at once. |
| [RFC2131-LEASE-9](#rfc2131-lease-9) | A client that gets a new address does not continue to use the old one. |
| [RFC2131-LEASE-10](#rfc2131-lease-10) | T1 and T2 should carry a random fuzz. |
| [RFC2131-LEASE-11](#rfc2131-lease-11) | The server should return T1 and T2, adjusted for the time that is left. |
| [RFC2131-RETX-1](#rfc2131-retx-1) | The client repeats an unanswered message with a randomized exponential backoff. |
| [RFC2131-RETX-2](#rfc2131-retx-2) | The first delay should be 4 seconds with a fuzz of one second, then 8, doubling to at most 64. |
| [RFC2131-RETX-3](#rfc2131-retx-3) | Without a DHCPOFFER the client repeats the DHCPDISCOVER. |
| [RFC2131-RETX-4](#rfc2131-retx-4) | Without a DHCPACK and without a DHCPNAK the client repeats the DHCPREQUEST and then returns to INIT. |
| [RFC2131-DECL-1](#rfc2131-decl-1) | The client should check the address of the DHCPACK before it uses it. |
| [RFC2131-DECL-2](#rfc2131-decl-2) | A client that finds the address in use sends a DHCPDECLINE and restarts. |
| [RFC2131-DECL-3](#rfc2131-decl-3) | The client should wait at least ten seconds before it restarts after a DHCPDECLINE. |
| [RFC2131-DECL-4](#rfc2131-decl-4) | A server that gets a DHCPDECLINE marks the address as not available. |
| [RFC2131-DECL-5](#rfc2131-decl-5) | The client broadcasts DHCPDECLINE messages. |
| [RFC2131-DECL-6](#rfc2131-decl-6) | A DHCPDECLINE carries the `requested IP address` and `server identifier` options and no lease time. |
| [RFC2131-DECL-7](#rfc2131-decl-7) | The ARP probe of the offered address carries the client's hardware address and sender address 0. |
| [RFC2131-DECL-8](#rfc2131-decl-8) | The client should broadcast an ARP reply to announce its new address. |
| [RFC2131-REL-1](#rfc2131-rel-1) | A client may give its lease back with a DHCPRELEASE. |
| [RFC2131-REL-2](#rfc2131-rel-2) | A DHCPRELEASE repeats the `client identifier` that obtained the lease. |
| [RFC2131-REL-3](#rfc2131-rel-3) | The client unicasts DHCPRELEASE messages to the server. |
| [RFC2131-REL-4](#rfc2131-rel-4) | A server that gets a DHCPRELEASE marks the address as not allocated. |
| [RFC2131-REL-5](#rfc2131-rel-5) | A DHCPRELEASE carries the address in `ciaddr` and the `server identifier` option, and no `requested IP address` and no lease time. |
| [RFC2131-INF-1](#rfc2131-inf-1) | A client with its own address may ask for parameters alone with a DHCPINFORM. |
| [RFC2131-INF-2](#rfc2131-inf-2) | The server answers a DHCPINFORM with a DHCPACK to the address in `ciaddr`. |
| [RFC2131-INF-3](#rfc2131-inf-3) | The answer to a DHCPINFORM carries no lease time and no `yiaddr`. |
| [RFC2131-INF-4](#rfc2131-inf-4) | The server does not look for a lease when it answers a DHCPINFORM. |
| [RFC2131-INF-5](#rfc2131-inf-5) | A DHCPINFORM carries no `requested IP address` and no lease time option. |
| [RFC2131-INF-6](#rfc2131-inf-6) | A DHCPINFORM goes to the DHCP server port. |
| [RFC2131-BCAST-1](#rfc2131-bcast-1) | A client that cannot receive a unicast before it is configured sets the BROADCAST bit. |
| [RFC2131-BCAST-2](#rfc2131-bcast-2) | With a non-zero `giaddr` the server answers to the server port of the relay agent. |
| [RFC2131-BCAST-3](#rfc2131-bcast-3) | With `giaddr` zero and `ciaddr` non-zero the server unicasts to `ciaddr`. |
| [RFC2131-BCAST-4](#rfc2131-bcast-4) | With both zero and the broadcast bit set the server broadcasts to 255.255.255.255. |
| [RFC2131-BCAST-5](#rfc2131-bcast-5) | With both zero and the broadcast bit clear the server unicasts to `chaddr` and `yiaddr`. |
| [RFC2131-BCAST-6](#rfc2131-bcast-6) | A server that answers a client directly should read the BROADCAST bit and obey it. |
| [RFC2131-SRVID-1](#rfc2131-srvid-1) | A server accepts any of its own addresses as its identifier. |
| [RFC2131-SRVID-2](#rfc2131-srvid-2) | A server chooses a `server identifier` that the client can reach. |
| [RFC2131-SRVID-3](#rfc2131-srvid-3) | A client unicasts to the address of the `server identifier` option. |
| [RFC2131-ID-1](#rfc2131-id-1) | A `client identifier` is unique to its client within the subnet. |
| [RFC2131-ID-2](#rfc2131-id-2) | The server identifies the client by the `client identifier`, or by `chaddr` when there is none. |
| [RFC2131-SEL-1](#rfc2131-sel-1) | The server chooses the address by a stated order of preference. |
| [RFC2131-SEL-2](#rfc2131-sel-2) | The server should not give the offered address to another client before the answer comes. |
| [RFC2131-SEL-3](#rfc2131-sel-3) | The server chooses the lease duration by a stated order of preference. |
| [RFC2131-SEL-4](#rfc2131-sel-4) | The server returns the address, the lease expiry and the requested parameters. |
| [RFC2131-SEL-5](#rfc2131-sel-5) | The server omits a parameter it has no value for, and returns each requested parameter once. |
| [RFC2131-MISC-1](#rfc2131-misc-1) | A client with several interfaces runs DHCP on each one on its own. |

## Message format and transport

### RFC2131-MSG-1

**A client message carries BOOTREQUEST in `op` and a server message carries BOOTREPLY.**

> "The 'op' field of each DHCP message sent from a client to a server contains BOOTREQUEST.
> BOOTREPLY is used in the 'op' field of each DHCP message sent from a server to a client."
> — §3, `rfc2131.txt:682-684`

- Strength: description. Class: wire.
- Check idea: every message from the client carries `op` = 1 and every message from the
  server carries `op` = 2.

### RFC2131-MSG-2

**The first four octets of the `options` field are the magic cookie 99, 130, 83, 99.**

> "The first four octets of the 'options' field of the DHCP message contain the (decimal)
> values 99, 130, 83 and 99, respectively (this is the same magic cookie as is defined in
> RFC 1497 [17])." — §3, `rfc2131.txt:686-688`

- Strength: description. Class: encoding.
- Check idea: the four octets at the start of the option area of every message are
  99.130.83.99. The value is what tells a receiver that the area holds DHCP options.

### RFC2131-MSG-3

**Every DHCP message carries the `DHCP message type` option.**

> "One particular option - the "DHCP message type" option - must be included in every DHCP
> message. This option defines the "type" of the DHCP message." — §3,
> `rfc2131.txt:694-696`

- Strength: must (lowercase in the text; the rule is absolute). Class: wire.
- Check idea: each of the eight messages of table 2 carries option 53 with the value that
  names it. The definition of the option is
  [RFC2132-TYPE-1](../rfc2132/catalog.md#rfc2132-type-1).

### RFC2131-MSG-4

**The last option is always the `end` option.**

> "The last option must always be the 'end' option." — §4.1, `rfc2131.txt:1230,1239`
> (the sentence crosses a page break)

- Strength: must (lowercase). Class: encoding.
- Check idea: the option area of every message ends with option 255.

### RFC2131-MSG-5

**DHCP runs over UDP: a client sends to port 67 and a server sends to port 68.**

> "DHCP uses UDP as its transport protocol. DHCP messages from a client to a server are
> sent to the 'DHCP server' port (67), and DHCP messages from a server to a client are
> sent to the 'DHCP client' port (68)." — §4.1, `rfc2131.txt:1241-1244`

- Strength: description. Class: wire.
- Check idea: every message from a client carries UDP destination port 67 and every
  message from a server carries UDP destination port 68.

### RFC2131-MSG-6

**A client must accept an `options` field of at least 312 octets, that is a message of 576
octets.**

> "A DHCP client must be prepared to receive DHCP messages with an 'options' field of at
> least length 312 octets. This requirement implies that a DHCP client must be prepared to
> receive a message of up to 576 octets, the minimum IP datagram size an IP host must be
> prepared to accept [3]." — §2, `rfc2131.txt:555-559,567`

- Strength: must (lowercase, twice). Class: end-to-end.
- Check idea: a server reply padded out to 576 octets reaches the client and configures it.
  A client that drops the message fails the statement.

### RFC2131-MSG-7

**The reserved bits of `flags` are zero from the client and ignored by the server.**

> "The remaining bits of the flags field are reserved for future use. They MUST be set to
> zero by clients and ignored by servers and relay agents." — §2, `rfc2131.txt:586-588`

- Strength: must. Class: wire for the send half, end-to-end for the ignore half.
- Check idea: the 15 low bits of `flags` are zero in every client message; a server that
  gets a message with one of them set answers it as if the bit were not there.

### RFC2131-MSG-8

**The client sets `hops` to zero.**

> "hops 1 Client sets to zero, optionally used by relay agents when booting via a relay
> agent." — table 1, `rfc2131.txt:528-529`

- Strength: description. Class: wire.
- Check idea: `hops` is 0 in every message that a client sends. Table 3 and table 5 repeat
  the value 0 for every message of both sides, `rfc2131.txt:1542` and
  `rfc2131.txt:2029`.

### RFC2131-MSG-9

**A message that a client broadcasts before it has an address carries IP source address 0.**

> "DHCP messages broadcast by a client prior to that client obtaining its IP address must
> have the source address field in the IP header set to 0." — §4.1, `rfc2131.txt:1267-1269`

- Strength: must (lowercase). Class: wire.
- Check idea: the IP header of the DHCPDISCOVER and of the DHCPREQUEST that follows it
  carries source address 0.0.0.0.

### RFC2131-MSG-10

**Options that extend into `sname` or `file` need the `option overload` option and follow
strict framing rules.**

> "If the options in a DHCP message extend into the 'sname' and 'file' fields, the 'option
> overload' option MUST appear in the 'options' field, with value 1, 2 or 3, as specified
> in RFC 1533." — §4.1, `rfc2131.txt:1284-1286`
>
> "The options in the 'sname' and 'file' fields (if in use as indicated by the 'options
> overload' option) MUST begin with the first octet of the field, MUST be terminated by an
> 'end' option, and MUST be followed by 'pad' options to fill the remainder of the field.
> Any individual option in the 'options', 'sname' and 'file' fields MUST be entirely
> contained in that field. The options in the 'options' field MUST be interpreted first"
> — §4.1, `rfc2131.txt:1298-1304`

- Strength: must, seven times. Class: encoding.
- Check idea: a serializer test. The rules state how the octets sit in three fields of one
  message, and a check that watches a link can see only that the option area parsed.

### RFC2131-MSG-11

**An option appears only once, and the client concatenates the values of several instances.**

> "Options may appear only once, unless otherwise specified in the options document. The
> client concatenates the values of multiple instances of the same option into a single
> parameter list for configuration." — §4.1, `rfc2131.txt:1311-1314`

- Strength: description. Class: encoding.
- Check idea: no option code occurs twice in a message. RFC 3396 replaces the
  concatenation half of this statement, and RFC 3396 is out of scope; see
  [`standards.md`](../../protocol/dhcp/standards.md#override-table).

## Transaction identifier

### RFC2131-XID-1

**`xid` is a random number of the client that binds a response to a request.**

> "xid 4 Transaction ID, a random number chosen by the client, used by the client and
> server to associate messages and responses between a client and a server." — table 1,
> `rfc2131.txt:530-533`

- Strength: description. Class: wire.
- Check idea: the `xid` of a reply equals the `xid` of the request it answers, and the
  value is not a fixed constant across two runs of one client.

### RFC2131-XID-2

**A client chooses an `xid` that is unlikely to collide with another client's.**

> "A DHCP client MUST choose 'xid's in such a way as to minimize the chance of using an
> 'xid' identical to one used by another client." — §4.1, `rfc2131.txt:1335-1337`

- Strength: must. Class: wire.
- Check idea: two clients that start at the same moment on one subnet use two different
  `xid` values.

### RFC2131-XID-3

**A server copies the `xid` of the request into its reply, and the client reuses it in the
DHCPREQUEST.**

> "The server inserts the 'xid' field from the DHCPDISCOVER message into the 'xid' field of
> the DHCPOFFER message and sends the DHCPOFFER message to the requesting client." — §4.3.1,
> `rfc2131.txt:1657-1660`
>
> "The DHCPREQUEST message contains the same 'xid' as the DHCPOFFER message." — §4.4.1,
> `rfc2131.txt:2116-2117`

- Strength: description. Class: wire.
- Check idea: one value of `xid` runs through the whole DHCPDISCOVER, DHCPOFFER,
  DHCPREQUEST and DHCPACK exchange.

### RFC2131-XID-4

**In SELECTING, a DHCPOFFER with a foreign `xid`, and any DHCPACK, is discarded in silence.**

> "If the 'xid' of an arriving DHCPOFFER message does not match the 'xid' of the most
> recent DHCPDISCOVER message, the DHCPOFFER message must be silently discarded. Any
> arriving DHCPACK messages must be silently discarded." — §4.4.1, `rfc2131.txt:1990-1993`

- Strength: must (lowercase, twice). Class: end-to-end.
- Check idea: a crafted DHCPOFFER with a wrong `xid` reaches a client in SELECTING; the
  client does not answer it and does not take the address it carries.

### RFC2131-XID-5

**In RENEWING, a DHCPACK with a foreign `xid` is discarded in silence.**

> "Any DHCPACK messages that arrive with an 'xid' that does not match the 'xid' of the
> client's DHCPREQUEST message are silently discarded." — §4.4.5, `rfc2131.txt:2231-2232`

- Strength: description with the word "silently"; the rule is absolute. Class: end-to-end.
- Check idea: a crafted DHCPACK with a wrong `xid` reaches a client that renews; the client
  stays in RENEWING and repeats its DHCPREQUEST.

## Discovery

### RFC2131-DISC-1

**The client broadcasts DHCPDISCOVER to 255.255.255.255 and the server port.**

> "The client broadcasts a DHCPDISCOVER message on its local physical subnet." — §3.1,
> `rfc2131.txt:714-715`
>
> "The client then broadcasts the DHCPDISCOVER on the local hardware broadcast address to
> the 0xffffffff IP broadcast address and 'DHCP server' UDP port." — §4.4.1,
> `rfc2131.txt:1986-1988`

- Strength: description. Class: wire.
- Check idea: the first message of a client that has no address is a DHCPDISCOVER with IP
  destination 255.255.255.255, a broadcast link-layer destination address, and UDP
  destination port 67.

### RFC2131-DISC-2

**`ciaddr` is zero in a DHCPDISCOVER.**

> "The client sets 'ciaddr' to 0x00000000." — §4.4.1, `rfc2131.txt:1971`
>
> "'ciaddr' 0 (DHCPDISCOVER)" — table 5, `rfc2131.txt:2038`

- Strength: description. Class: wire.
- Check idea: the `ciaddr` field of a DHCPDISCOVER is 0.0.0.0.

### RFC2131-DISC-3

**The client puts its hardware address in `chaddr`.**

> "The client MUST include its hardware address in the 'chaddr' field, if necessary for
> delivery of DHCP reply messages." — §4.4.1, `rfc2131.txt:1976-1977`

- Strength: must, with a condition. Class: wire.
- Check idea: the `chaddr` field of the DHCPDISCOVER equals the MAC address of the
  interface that sent it, and the `hlen` field is the length of that address.

### RFC2131-DISC-4

**The client waits a random time of one to ten seconds before the first DHCPDISCOVER.**

> "The client SHOULD wait a random time between one and ten seconds to desynchronize the
> use of DHCP at startup." — §4.4.1, `rfc2131.txt:1970-1971`

- Strength: should. Class: wire, as a time.
- Check idea: the first DHCPDISCOVER of a client leaves between one and ten seconds after
  the client starts, and two clients that start together do not send at the same instant.

### RFC2131-DISC-5

**A DHCPDISCOVER carries no `server identifier` option.**

> "Server identifier MUST NOT (DHCPDISCOVER)" — table 5, `rfc2131.txt:2097`

- Strength: must not. Class: wire.
- Check idea: option 54 is absent from every DHCPDISCOVER.

### RFC2131-DISC-6

**A DHCPDISCOVER may suggest an address and a lease time.**

> "The client MAY suggest a network address and/or lease time by including the 'requested
> IP address' and 'IP address lease time' options." — §4.4.1, `rfc2131.txt:1973-1975`

- Strength: may. Class: wire.
- Check idea: a client that is configured to ask for a named address carries option 50 with
  that address in its DHCPDISCOVER. The absence of the option is not a violation.

## Offer

### RFC2131-OFF-1

**A server may answer a DHCPDISCOVER with a DHCPOFFER that carries a free address in
`yiaddr`.**

> "Each server may respond with a DHCPOFFER message that includes an available network
> address in the 'yiaddr' field (and other configuration parameters in DHCP options)."
> — §3.1, `rfc2131.txt:720-722`

- Strength: may for the answer; the content of the answer is description. Class: wire.
- Check idea: a DHCPOFFER follows the DHCPDISCOVER, and its `yiaddr` is an address of the
  subnet of the client that no other client holds. The permission is real: §4.2 says a
  server need not answer at all, `rfc2131.txt:1389-1393`.

### RFC2131-OFF-2

**The DHCPOFFER carries the `xid` of the DHCPDISCOVER.**

> "'xid' 'xid' from client DHCPDISCOVER message" — table 3, `rfc2131.txt:1543-1545`

- Strength: description. Class: wire. The general rule is
  [RFC2131-XID-3](#rfc2131-xid-3); this entry is the table cell for the DHCPOFFER.
- Check idea: the `xid` of the DHCPOFFER equals the `xid` of the DHCPDISCOVER it answers.

### RFC2131-OFF-3

**A DHCPOFFER carries the `server identifier` option.**

> "Server identifier MUST" — table 3, DHCPOFFER column, `rfc2131.txt:1586`

- Strength: must. Class: wire.
- Check idea: option 54 is present in the DHCPOFFER, and it holds an address of the server
  that sent the message.

### RFC2131-OFF-4

**A DHCPOFFER carries the `IP address lease time` option.**

> "IP address lease time MUST" — table 3, DHCPOFFER column, `rfc2131.txt:1578`

- Strength: must. Class: wire.
- Check idea: option 51 is present in the DHCPOFFER and its value is greater than zero.

### RFC2131-OFF-5

**A DHCPOFFER carries no `requested IP address`, `parameter request list`, `client
identifier` or `maximum message size` option.**

> "Requested IP address MUST NOT ... Parameter request list MUST NOT ... Client identifier
> MUST NOT ... Maximum message size MUST NOT" — table 3, DHCPOFFER column,
> `rfc2131.txt:1577,1582,1584,1587`

- Strength: must not, four times. Class: wire.
- Overridden by [RFC6842-CLID-1](../rfc6842/catalog.md#rfc6842-clid-1), for the `client
  identifier` row only: when the client sent one, the server must now return it. The other
  three rows stand.
- Check idea: options 50, 55 and 57 are absent from the DHCPOFFER. Option 61 is absent
  when the client sent none, and present when the client sent one.

### RFC2131-OFF-6

**A DHCPOFFER carries `hops` 0, `secs` 0, `ciaddr` 0, and the `flags`, `giaddr` and
`chaddr` of the DHCPDISCOVER.**

> "'hops' 0 ... 'secs' 0 ... 'ciaddr' 0 ... 'flags' 'flags' from client DHCPDISCOVER
> message ... 'giaddr' 'giaddr' from client DHCPDISCOVER message ... 'chaddr' 'chaddr'
> from client DHCPDISCOVER message" — table 3, DHCPOFFER column,
> `rfc2131.txt:1542,1546,1547,1553-1561`

- Strength: description, as a table of values. Class: wire.
- Check idea: the six fields of the DHCPOFFER hold the stated values, and the three that
  come from the request equal the fields of the DHCPDISCOVER.

### RFC2131-OFF-7

**A server should check that the address it offers is not already in use.**

> "When allocating a new address, servers SHOULD check that the offered network address is
> not already in use; e.g., the server may probe the offered address with an ICMP Echo
> Request. Servers SHOULD be implemented so that network administrators MAY choose to
> disable probes of newly allocated addresses." — §3.1, `rfc2131.txt:726-727,735-738`
> (the sentence crosses a page break)

- Strength: should. Class: wire, as an ICMP echo request.
- Check idea: an ICMP echo request for the address of `yiaddr` leaves the server between
  the DHCPDISCOVER and the DHCPOFFER. Silence is a `declined`, not a defect.

### RFC2131-OFF-8

**A server always puts its own address in the `server identifier` option.**

> "A DHCP server always returns its own address in the 'server identifier' option." — §2,
> `rfc2131.txt:516-517`

- Strength: description, with the word "always". Class: wire.
- Check idea: the address inside option 54 of a DHCPOFFER equals the IP source address of
  the datagram that carries it, when the server and the client share a subnet.

## Request

### RFC2131-REQ-1

**A DHCPREQUEST from SELECTING carries the `server identifier` option of the chosen server.**

> "The client broadcasts a DHCPREQUEST message that MUST include the 'server identifier'
> option to indicate which server it has selected" — §3.1, `rfc2131.txt:851-853`
>
> "Client inserts the address of the selected server in 'server identifier'" — §4.3.2,
> `rfc2131.txt:1696-1697`

- Strength: must. Class: wire.
- Check idea: option 54 of the DHCPREQUEST holds the address that option 54 of the chosen
  DHCPOFFER held.

### RFC2131-REQ-2

**A DHCPREQUEST from SELECTING carries the offered `yiaddr` in the `requested IP address`
option.**

> "The 'requested IP address' option MUST be set to the value of 'yiaddr' in the DHCPOFFER
> message from the server." — §3.1, `rfc2131.txt:854-856`

- Strength: must. Class: wire.
- Check idea: option 50 of the DHCPREQUEST equals the `yiaddr` field of the DHCPOFFER.

### RFC2131-REQ-3

**`ciaddr` is zero in a DHCPREQUEST from SELECTING and from INIT-REBOOT.**

> "'ciaddr' MUST be zero" — §4.3.2, SELECTING, `rfc2131.txt:1697`, and INIT-REBOOT,
> `rfc2131.txt:1718`
>
> "ciaddr zero zero IP address IP address" — table 4, `rfc2131.txt:1839`

- Strength: must. Class: wire.
- Check idea: the `ciaddr` field of the DHCPREQUEST that answers a DHCPOFFER is 0.0.0.0.
  The client has no address yet, so a non-zero value would be a claim it cannot make.

### RFC2131-REQ-4

**The DHCPREQUEST from SELECTING is broadcast, and it repeats the `secs` value and the
broadcast address of the DHCPDISCOVER.**

> "This DHCPREQUEST message is broadcast and relayed through DHCP/BOOTP relay agents. To
> help ensure that any BOOTP relay agents forward the DHCPREQUEST message to the same set
> of DHCP servers that received the original DHCPDISCOVER message, the DHCPREQUEST message
> MUST use the same value in the DHCP message header's 'secs' field and be sent to the same
> IP broadcast address as the original DHCPDISCOVER message." — §3.1, `rfc2131.txt:856-862`

- Strength: must. Class: wire.
- Check idea: the DHCPREQUEST goes to 255.255.255.255, and its `secs` field holds the same
  value as the `secs` field of the DHCPDISCOVER of the same transaction.

### RFC2131-REQ-5

**A DHCPREQUEST from INIT-REBOOT carries the remembered address and no `server identifier`.**

> "'server identifier' MUST NOT be filled in, 'requested IP address' option MUST be filled
> in with client's notion of its previously assigned address. 'ciaddr' MUST be zero."
> — §4.3.2, `rfc2131.txt:1716-1718`
>
> "The client MUST insert its known network address as a 'requested IP address' option in
> the DHCPREQUEST message. ... The client MUST NOT include a 'server identifier' in the
> DHCPREQUEST message." — §4.4.2, `rfc2131.txt:2143-2150`

- Strength: must and must not. Class: wire.
- Check idea: a client that restarts with a remembered address broadcasts a DHCPREQUEST
  that carries option 50 with that address, carries no option 54, and has `ciaddr` 0.

### RFC2131-REQ-6

**A DHCPREQUEST from RENEWING is unicast, carries `ciaddr`, and carries neither `server
identifier` nor `requested IP address`.**

> "'server identifier' MUST NOT be filled in, 'requested IP address' option MUST NOT be
> filled in, 'ciaddr' MUST be filled in with client's IP address. ... This message will be
> unicast, so no relay agents will be involved in its transmission." — §4.3.2,
> `rfc2131.txt:1765-1770`
>
> "At time T1 the client moves to RENEWING state and sends (via unicast) a DHCPREQUEST
> message to the server to extend its lease. The client sets the 'ciaddr' field in the
> DHCPREQUEST to its current network address." — §4.4.5, `rfc2131.txt:2223-2226`

- Strength: must and must not. Class: wire.
- Check idea: the renewal message is a unicast to the leasing server whose `ciaddr` holds
  the leased address, and it carries neither option 50 nor option 54.

### RFC2131-REQ-7

**A DHCPREQUEST from REBINDING is broadcast, and carries the same fields as the one from
RENEWING.**

> "'server identifier' MUST NOT be filled in, 'requested IP address' option MUST NOT be
> filled in, 'ciaddr' MUST be filled in with client's IP address. ... This message MUST be
> broadcast to the 0xffffffff IP broadcast address." — §4.3.2, `rfc2131.txt:1781-1785`

- Strength: must and must not. Class: wire.
- Check idea: the rebinding message goes to 255.255.255.255, its `ciaddr` holds the leased
  address, and it carries neither option 50 nor option 54.

### RFC2131-REQ-8

**A client that asked for a parameter list repeats that list in every later message.**

> "If the client includes a list of parameters in a DHCPDISCOVER message, it MUST include
> that list in any subsequent DHCPREQUEST messages." — §3.5, `rfc2131.txt:1151-1153`
>
> "If the client included a list of requested parameters in a DHCPDISCOVER message, it MUST
> include that list in all subsequent messages." — §4.3.2, `rfc2131.txt:1672-1674`

- Strength: must. Class: wire.
- Check idea: option 55 of the DHCPREQUEST holds the same option codes as option 55 of the
  DHCPDISCOVER of the same transaction.

### RFC2131-REQ-9

**A client that used a `client identifier` uses the same one in every later message.**

> "If the client uses a 'client identifier' in one message, it MUST use that same identifier
> in all subsequent messages, to ensure that all servers correctly identify the client."
> — §2, `rfc2131.txt:499-501`
>
> "If the client supplies a 'client identifier', the client MUST use the same 'client
> identifier' in all subsequent messages" — §4.2, `rfc2131.txt:1417-1419`

- Strength: must. Class: wire.
- Check idea: option 61 holds the same octets in the DHCPDISCOVER, the DHCPREQUEST and
  every later message of one client.

## Acknowledgement

### RFC2131-ACK-1

**The chosen server commits the binding and answers with a DHCPACK.**

> "The server selected in the DHCPREQUEST message commits the binding for the client to
> persistent storage and responds with a DHCPACK message containing the configuration
> parameters for the requesting client." — §3.1, `rfc2131.txt:869-872`

- Strength: description; it is the only path from a DHCPREQUEST to a configured client.
  Class: wire.
- Check idea: a DHCPACK follows the DHCPREQUEST, and it comes from the server that option
  54 of the DHCPREQUEST named.

### RFC2131-ACK-2

**The `yiaddr` of the DHCPACK is the assigned address.**

> "The 'yiaddr' field in the DHCPACK messages is filled in with the selected network
> address." — §3.1, `rfc2131.txt:879-880`

- Strength: description. Class: wire and end-to-end.
- Check idea: the `yiaddr` of the DHCPACK equals the address the client asked for in option
  50 of its DHCPREQUEST, and the client uses that address afterwards.

### RFC2131-ACK-3

**A DHCPACK that answers a DHCPREQUEST carries the `IP address lease time` option; one that
answers a DHCPINFORM carries none.**

> "IP address lease time ... MUST (DHCPREQUEST) MUST NOT (DHCPINFORM)" — table 3, DHCPACK
> column, `rfc2131.txt:1578-1579`

- Strength: must and must not. Class: wire.
- Check idea: option 51 is present in the DHCPACK that ends the four-message exchange, and
  absent from the DHCPACK that answers a DHCPINFORM.

### RFC2131-ACK-4

**A DHCPACK carries the `server identifier` option.**

> "Server identifier MUST" — table 3, DHCPACK column, `rfc2131.txt:1586`

- Strength: must. Class: wire.
- Check idea: option 54 is present in the DHCPACK.

### RFC2131-ACK-5

**A DHCPACK carries no `requested IP address`, `parameter request list`, `client identifier`
or `maximum message size` option.**

> "Requested IP address MUST NOT ... Parameter request list MUST NOT ... Client identifier
> MUST NOT ... Maximum message size MUST NOT" — table 3, DHCPACK column,
> `rfc2131.txt:1577,1582,1584,1587`

- Strength: must not, four times. Class: wire.
- Overridden by [RFC6842-CLID-1](../rfc6842/catalog.md#rfc6842-clid-1), for the `client
  identifier` row only. The other three rows stand.
- Check idea: options 50, 55 and 57 are absent from the DHCPACK.

### RFC2131-ACK-6

**The parameters of the DHCPACK should not conflict with those of the DHCPOFFER.**

> "Any configuration parameters in the DHCPACK message SHOULD NOT conflict with those in
> the earlier DHCPOFFER message to which the client is responding." — §3.1,
> `rfc2131.txt:876-878`

- Strength: should not. Class: wire.
- Check idea: the `yiaddr`, the lease time and the subnet mask of the DHCPACK equal the
  ones of the DHCPOFFER of the same transaction.

### RFC2131-ACK-7

**The server should not probe the address again when it answers the DHCPREQUEST.**

> "The server SHOULD NOT check the offered network address at this point." — §3.1,
> `rfc2131.txt:878-879`

- Strength: should not. Class: wire, as the absence of an ICMP echo request.
- Check idea: no ICMP echo request for `yiaddr` leaves the server between the DHCPREQUEST
  and the DHCPACK. The check needs the probe of
  [RFC2131-OFF-7](#rfc2131-off-7) to be visible first, or it passes for the wrong reason.

### RFC2131-ACK-8

**A DHCPACK carries the `xid`, `flags`, `giaddr` and `chaddr` of the DHCPREQUEST, `secs` 0
and `hops` 0.**

> "'hops' 0 ... 'xid' 'xid' from client DHCPREQUEST message ... 'secs' 0 ... 'ciaddr'
> 'ciaddr' from DHCPREQUEST or 0 ... 'flags' 'flags' from client DHCPREQUEST message"
> — table 3, DHCPACK column, `rfc2131.txt:1542-1561`

- Strength: description, as a table of values. Class: wire.
- Check idea: the six fields of the DHCPACK hold the stated values.

## Negative acknowledgement

### RFC2131-NAK-1

**A chosen server that cannot satisfy the DHCPREQUEST should answer with a DHCPNAK.**

> "If the selected server is unable to satisfy the DHCPREQUEST message (e.g., the requested
> network address has been allocated), the server SHOULD respond with a DHCPNAK message."
> — §3.1, `rfc2131.txt:882-884`

- Strength: should. Class: wire.
- Check idea: a DHCPREQUEST for an address that the server has already given to another
  client draws a DHCPNAK and no DHCPACK.

### RFC2131-NAK-2

**A server should answer a request for a wrong address or a wrong subnet with a DHCPNAK.**

> "If the client's request is invalid (e.g., the client has moved to a new subnet), servers
> SHOULD respond with a DHCPNAK message to the client. Servers SHOULD NOT respond if their
> information is not guaranteed to be accurate." — §3.2, `rfc2131.txt:1015-1017`
>
> "If the DHCP server detects that the client is on the wrong net (i.e., the result of
> applying the local subnet mask or remote subnet mask (if 'giaddr' is not zero) to
> 'requested IP address' option value doesn't match reality), then the server SHOULD send a
> DHCPNAK message to the client." — §4.3.2, `rfc2131.txt:1725-1730`

- Strength: should. Class: wire.
- Check idea: a DHCPREQUEST from INIT-REBOOT that asks for an address of a foreign subnet
  draws a DHCPNAK.

### RFC2131-NAK-3

**When `giaddr` is zero, the server broadcasts the DHCPNAK to 255.255.255.255.**

> "If 'giaddr' is 0x0 in the DHCPREQUEST message, the client is on the same subnet as the
> server. The server MUST broadcast the DHCPNAK message to the 0xffffffff broadcast address
> because the client may not have a correct network address or subnet mask, and the client
> may not be answering ARP requests." — §4.3.2, `rfc2131.txt:1751-1755`
>
> "In all cases, when 'giaddr' is zero, the server broadcasts any DHCPNAK messages to
> 0xffffffff." — §4.1, `rfc2131.txt:1281-1282`

- Strength: must. Class: wire.
- Check idea: the IP destination address of the DHCPNAK is 255.255.255.255 and its
  link-layer destination address is the broadcast address, whatever the BROADCAST bit of
  the request said.

### RFC2131-NAK-4

**When `giaddr` is set, the server sends the DHCPNAK to the relay agent and sets the
broadcast bit.**

> "Otherwise, the server MUST send the DHCPNAK message to the IP address of the BOOTP relay
> agent, as recorded in 'giaddr'." — §3.2, `rfc2131.txt:1028-1029`
>
> "If 'giaddr' is set in the DHCPREQUEST message, the client is on a different subnet. The
> server MUST set the broadcast bit in the DHCPNAK, so that the relay agent will broadcast
> the DHCPNAK to the client" — §4.3.2, `rfc2131.txt:1757-1760`

- Strength: must. Class: wire.
- Check idea: the statement needs a relay agent on the path. Without one, `giaddr` is
  always zero and the condition never holds.

### RFC2131-NAK-5

**A DHCPNAK carries `yiaddr` 0, `ciaddr` 0, `siaddr` 0, and no lease time and no other
configuration option.**

> "'ciaddr' ... 0 ... 'yiaddr' ... 0 ... 'siaddr' ... 0" — table 3, DHCPNAK column,
> `rfc2131.txt:1547-1552`
>
> "Requested IP address MUST NOT ... IP address lease time ... MUST NOT ... Use
> 'file'/'sname' fields ... MUST NOT ... Parameter request list ... MUST NOT ... Maximum
> message size ... MUST NOT ... All others ... MUST NOT" — table 3, DHCPNAK column,
> `rfc2131.txt:1577-1588`

- Strength: must not, six times, plus three field values as description. Class: wire.
- Check idea: the DHCPNAK carries no option 50, 51, 52, 55 or 57, no configuration option
  such as the subnet mask, and its three address fields are 0.0.0.0. The `All others MUST
  NOT` row is the widest prohibition in the document.

### RFC2131-NAK-6

**A DHCPNAK carries the `server identifier` option.**

> "Server identifier MUST" — table 3, DHCPNAK column, `rfc2131.txt:1586`

- Strength: must. Class: wire.
- Check idea: option 54 is present in the DHCPNAK. It is the one option that the `All
  others MUST NOT` row of [RFC2131-NAK-5](#rfc2131-nak-5) does not remove.

### RFC2131-NAK-7

**A server with no record of a client in INIT-REBOOT stays silent.**

> "If the DHCP server has no record of this client, then it MUST remain silent, and MAY
> output a warning to the network administrator. This behavior is necessary for peaceful
> coexistence of non-communicating DHCP servers on the same wire." — §4.3.2,
> `rfc2131.txt:1746-1749`

- Strength: must. Class: wire, as an absence.
- Check idea: a DHCPREQUEST from INIT-REBOOT that names an address of a client the server
  never served draws no message at all: no DHCPNAK and no DHCPACK.

### RFC2131-NAK-8

**A client that gets a DHCPNAK restarts the configuration process.**

> "If the client receives a DHCPNAK message, the client restarts the configuration
> process." — §3.1, `rfc2131.txt:910-911`
>
> "If the client receives a DHCPNAK message, it cannot reuse its remembered network
> address. It must instead request a new address by restarting the configuration process,
> this time using the (non-abbreviated) procedure described in section 3.1." — §3.2,
> `rfc2131.txt:1048-1051`

- Strength: description in §3.1, must (lowercase) in §3.2; it is the only path out of the
  state. Class: wire.
- Check idea: a DHCPNAK is followed by a DHCPDISCOVER from the same client, and the client
  does not use the address it asked for.

## Lease and reacquisition

### RFC2131-LEASE-1

**Times are in seconds, and 0xffffffff means infinity.**

> "Throughout the protocol, times are to be represented in units of seconds. The time value
> of 0xffffffff is reserved to represent "infinity"." — §3.3, `rfc2131.txt:1096-1098`

- Strength: description. Class: wire.
- Check idea: the lease time in option 51 counts seconds; a lease that never ends carries
  the value 4294967295.

### RFC2131-LEASE-2

**The client computes the expiry as the send time of its DHCPREQUEST plus the lease of the
DHCPACK.**

> "The client records the lease expiration time as the sum of the time at which the
> original request was sent and the duration of the lease from the DHCPACK message."
> — §4.4.1, `rfc2131.txt:2117-2119`

- Strength: description. Class: internal.
- Check idea: the instant at which the client gives the address up equals the send time of
  the DHCPREQUEST plus the lease, and not the arrival time of the DHCPACK plus the lease.

### RFC2131-LEASE-3

**T1 is earlier than T2, and T2 is earlier than the expiry.**

> "T1 MUST be earlier than T2, which, in turn, MUST be earlier than the time at which the
> client's lease will expire." — §4.4.5, `rfc2131.txt:2216-2218`

- Strength: must. Class: wire for the values in the DHCPACK, internal for the times.
- Check idea: options 58 and 59 of the DHCPACK hold values in the order T1 < T2 < lease,
  and the renewal message leaves before the rebinding message, which leaves before the
  expiry.

### RFC2131-LEASE-4

**T1 defaults to half the lease and T2 to 0.875 of the lease.**

> "Times T1 and T2 are configurable by the server through options. T1 defaults to (0.5 *
> duration_of_lease). T2 defaults to (0.875 * duration_of_lease)." — §4.4.5,
> `rfc2131.txt:2253-2255`

- Strength: description. Class: wire and internal.
- Check idea: with a DHCPACK that carries no option 58 and no option 59, the renewal
  message leaves at half the lease and the rebinding message at 0.875 of it.

### RFC2131-LEASE-5

**At T1 the client unicasts a DHCPREQUEST to the leasing server.**

> "At time T1 the client moves to RENEWING state and sends (via unicast) a DHCPREQUEST
> message to the server to extend its lease." — §4.4.5, `rfc2131.txt:2223-2225`

- Strength: description; it is the only path out of BOUND at T1. Class: wire.
- Check idea: a unicast DHCPREQUEST leaves the client at T1 after the DHCPACK, addressed to
  the server that gave the lease.

### RFC2131-LEASE-6

**At T2 the client broadcasts a DHCPREQUEST to any server.**

> "If no DHCPACK arrives before time T2, the client moves to REBINDING state and sends (via
> broadcast) a DHCPREQUEST message to extend its lease." — §4.4.5, `rfc2131.txt:2247-2249`

- Strength: description; the only path. Class: wire.
- Check idea: when the renewal at T1 stays unanswered, a broadcast DHCPREQUEST leaves the
  client at T2.

### RFC2131-LEASE-7

**Without an answer the client waits half of the remaining time, at least 60 seconds,
before it repeats.**

> "In both RENEWING and REBINDING states, if the client receives no response to its
> DHCPREQUEST message, the client SHOULD wait one-half of the remaining time until T2 (in
> RENEWING state) and one-half of the remaining lease time (in REBINDING state), down to a
> minimum of 60 seconds, before retransmitting the DHCPREQUEST message." — §4.4.5,
> `rfc2131.txt:2265-2269`

- Strength: should. Class: wire, as a time.
- Check idea: the interval between two renewal messages is half of the time that is left
  until T2, and it never falls below 60 seconds.

### RFC2131-LEASE-8

**When the lease expires the client goes to INIT and stops all other network processing at
once.**

> "If the lease expires before the client receives a DHCPACK, the client moves to INIT
> state, MUST immediately stop any other network processing and requests network
> initialization parameters as if the client were uninitialized." — §4.4.5,
> `rfc2131.txt:2271-2273`
>
> "If the lease expires before the client can contact a DHCP server, the client must
> immediately discontinue use of the previous network address" — §3.7,
> `rfc2131.txt:1213-1215`

- Strength: must. Class: end-to-end and wire.
- Check idea: after the expiry the client sends no datagram from the leased address, and a
  DHCPDISCOVER leaves it.

### RFC2131-LEASE-9

**A client that gets a new address does not continue to use the old one.**

> "If the client is given a new network address, it MUST NOT continue using the previous
> network address and SHOULD notify the local users of the problem." — §4.4.5,
> `rfc2131.txt:2276-2278`

- Strength: must not. Class: wire.
- Check idea: after a DHCPACK that carries a different `yiaddr`, no datagram leaves the
  client with the earlier address as its IP source address.

### RFC2131-LEASE-10

**T1 and T2 should carry a random fuzz.**

> "Times T1 and T2 SHOULD be chosen with some random "fuzz" around a fixed value, to avoid
> synchronization of client reacquisition." — §4.4.5, `rfc2131.txt:2255-2257`

- Strength: should. Class: wire, as a time.
- Check idea: two clients with equal leases that were configured at the same instant do not
  renew at the same instant.

### RFC2131-LEASE-11

**The server should return T1 and T2, adjusted for the time that is left.**

> "The server SHOULD return T1 and T2, and their values SHOULD be adjusted from their
> original values to take account of the time remaining on the lease." — §4.4.5,
> `rfc2131.txt:2261-2263`

- Strength: should. Class: wire.
- Check idea: options 58 and 59 are present in the DHCPACK, and the value of a renewal
  DHCPACK is not simply a copy of the value of the first one.

## Retransmission

### RFC2131-RETX-1

**The client repeats an unanswered message with a randomized exponential backoff.**

> "DHCP clients are responsible for all message retransmission. The client MUST adopt a
> retransmission strategy that incorporates a randomized exponential backoff algorithm to
> determine the delay between retransmissions." — §4.1, `rfc2131.txt:1316-1319`

- Strength: must. Class: wire, as a time.
- Check idea: with no server on the subnet, the intervals between the DHCPDISCOVER messages
  of one client grow, and two clients do not repeat at the same instant.

### RFC2131-RETX-2

**The first delay should be 4 seconds with a fuzz of one second, then 8, doubling to at
most 64.**

> "For example, in a 10Mb/sec Ethernet internetwork, the delay before the first
> retransmission SHOULD be 4 seconds randomized by the value of a uniform random number
> chosen from the range -1 to +1. ... The delay before the next retransmission SHOULD be 8
> seconds randomized by the value of a uniform number chosen from the range -1 to +1. The
> retransmission delay SHOULD be doubled with subsequent retransmissions up to a maximum of
> 64 seconds." — §4.1, `rfc2131.txt:1322-1330`

- Strength: should. Class: wire, as a time.
- Check idea: the first interval is in the range 3 to 5 seconds, the second in 7 to 9, and
  no interval passes 64 seconds. The tolerance belongs to a statistical check, and that is
  level 4 work.

### RFC2131-RETX-3

**Without a DHCPOFFER the client repeats the DHCPDISCOVER.**

> "The client times out and retransmits the DHCPDISCOVER message if the client receives no
> DHCPOFFER messages." — §3.1, `rfc2131.txt:863-864`

- Strength: description. Class: wire.
- Check idea: on a subnet with no server, a second DHCPDISCOVER follows the first one.

### RFC2131-RETX-4

**Without a DHCPACK and without a DHCPNAK the client repeats the DHCPREQUEST and then
returns to INIT.**

> "The client times out and retransmits the DHCPREQUEST message if the client receives
> neither a DHCPACK or a DHCPNAK message. ... If the client receives neither a DHCPACK or a
> DHCPNAK message after employing the retransmission algorithm, the client reverts to INIT
> state and restarts the initialization process." — §3.1, `rfc2131.txt:913-925`

- Strength: description. Class: wire.
- Check idea: when the DHCPACK is removed from the path, a second DHCPREQUEST follows, and
  after the repetitions a DHCPDISCOVER follows.

## Decline

### RFC2131-DECL-1

**The client should check the address of the DHCPACK before it uses it.**

> "The client SHOULD perform a final check on the parameters (e.g., ARP for allocated
> network address), and notes the duration of the lease specified in the DHCPACK message."
> — §3.1, `rfc2131.txt:891-894`
>
> "The client SHOULD perform a check on the suggested address to ensure that the address is
> not already in use. For example, if the client is on a network that supports ARP, the
> client may issue an ARP request for the suggested request." — §4.4.1,
> `rfc2131.txt:2119-2123`

- Strength: should. Class: wire, as an ARP request.
- Check idea: an ARP request for the address of `yiaddr` leaves the client after the
  DHCPACK and before the first datagram from that address.

### RFC2131-DECL-2

**A client that finds the address in use sends a DHCPDECLINE and restarts.**

> "If the client detects that the address is already in use (e.g., through the use of ARP),
> the client MUST send a DHCPDECLINE message to the server and restarts the configuration
> process." — §3.1, `rfc2131.txt:903-906`
>
> "If the network address appears to be in use, the client MUST send a DHCPDECLINE message
> to the server." — §4.4.1, `rfc2131.txt:2135-2136`

- Strength: must. Class: wire.
- Check idea: when another node answers the ARP probe for `yiaddr`, a DHCPDECLINE leaves
  the client and a DHCPDISCOVER follows it.

### RFC2131-DECL-3

**The client should wait at least ten seconds before it restarts after a DHCPDECLINE.**

> "The client SHOULD wait a minimum of ten seconds before restarting the configuration
> process to avoid excessive network traffic in case of looping." — §3.1,
> `rfc2131.txt:906-908`

- Strength: should. Class: wire, as a time.
- Check idea: at least ten seconds pass between the DHCPDECLINE and the DHCPDISCOVER that
  follows it.

### RFC2131-DECL-4

**A server that gets a DHCPDECLINE marks the address as not available.**

> "If the server receives a DHCPDECLINE message, the client has discovered through some
> other means that the suggested network address is already in use. The server MUST mark
> the network address as not available and SHOULD notify the local system administrator of
> a possible configuration problem." — §4.3.3, `rfc2131.txt:1807-1811`

- Strength: must. Class: internal, with an end-to-end consequence.
- Check idea: after a DHCPDECLINE for one address, the next DHCPOFFER of that server
  carries a different address.

### RFC2131-DECL-5

**The client broadcasts DHCPDECLINE messages.**

> "Because the client is declining the use of the IP address supplied by the server, the
> client broadcasts DHCPDECLINE messages." — §4.4.4, `rfc2131.txt:2197-2199`

- Strength: description. Class: wire.
- Check idea: the IP destination address of the DHCPDECLINE is 255.255.255.255.

### RFC2131-DECL-6

**A DHCPDECLINE carries the `requested IP address` and `server identifier` options and no
lease time.**

> "Requested IP address ... MUST (DHCPDECLINE) ... IP address lease time ... MUST NOT ...
> Vendor class identifier ... MUST NOT ... Server identifier ... MUST ... Parameter request
> list ... MUST NOT ... Maximum message size ... MUST NOT ... All others ... MUST NOT"
> — table 5, DHCPDECLINE column, `rfc2131.txt:2082-2107`
>
> "'ciaddr' ... 0 (DHCPDECLINE)" — table 5, `rfc2131.txt:2038`

- Strength: must and must not. Class: wire.
- Check idea: the DHCPDECLINE carries option 50 with the refused address and option 54 with
  the server, and carries no option 51, 55, 57 or 60.

### RFC2131-DECL-7

**The ARP probe of the offered address carries the client's hardware address and sender
address 0.**

> "When broadcasting an ARP request for the suggested address, the client must fill in its
> own hardware address as the sender's hardware address, and 0 as the sender's IP address,
> to avoid confusing ARP caches in other hosts on the same subnet." — §4.4.1,
> `rfc2131.txt:2123-2126`

- Strength: must (lowercase). Class: wire.
- Check idea: the ARP request of [RFC2131-DECL-1](#rfc2131-decl-1) carries sender protocol
  address 0.0.0.0 and the sender hardware address of the client.

### RFC2131-DECL-8

**The client should broadcast an ARP reply to announce its new address.**

> "The client SHOULD broadcast an ARP reply to announce the client's new IP address and
> clear any outdated ARP cache entries in hosts on the client's subnet." — §4.4.1,
> `rfc2131.txt:2136-2138`

- Strength: should. Class: wire.
- Check idea: an ARP reply with the new address leaves the client after the DHCPACK, to the
  broadcast address.

## Release

### RFC2131-REL-1

**A client may give its lease back with a DHCPRELEASE.**

> "The client may choose to relinquish its lease on a network address by sending a
> DHCPRELEASE message to the server." — §3.1, `rfc2131.txt:928-930`

- Strength: may. Class: wire.
- Check idea: a client that shuts down while it holds a lease sends a DHCPRELEASE. Silence
  conforms as well; §4.4.6 says so, `rfc2131.txt:2284-2285`.

### RFC2131-REL-2

**A DHCPRELEASE repeats the `client identifier` that obtained the lease.**

> "If the client used a 'client identifier' when it obtained the lease, it MUST use the
> same 'client identifier' in the DHCPRELEASE message." — §3.1, `rfc2131.txt:931-933`

- Strength: must. Class: wire.
- Check idea: option 61 of the DHCPRELEASE holds the same octets as option 61 of the
  DHCPREQUEST that obtained the lease.

### RFC2131-REL-3

**The client unicasts DHCPRELEASE messages to the server.**

> "The client unicasts DHCPRELEASE messages to the server." — §4.4.4, `rfc2131.txt:2197`

- Strength: description. Class: wire.
- Check idea: the IP destination address of the DHCPRELEASE is the address of the server,
  not the broadcast address.

### RFC2131-REL-4

**A server that gets a DHCPRELEASE marks the address as not allocated.**

> "Upon receipt of a DHCPRELEASE message, the server marks the network address as not
> allocated. The server SHOULD retain a record of the client's initialization parameters
> for possible reuse in response to subsequent requests from the client." — §4.3.4,
> `rfc2131.txt:1815-1818`

- Strength: description for the first sentence, should for the second; the first is the
  only path. Class: internal, with an end-to-end consequence.
- Check idea: after a DHCPRELEASE, the server offers that same address to the next client
  that asks.

### RFC2131-REL-5

**A DHCPRELEASE carries the address in `ciaddr` and the `server identifier` option, and no
`requested IP address` and no lease time.**

> "Requested IP address ... MUST NOT (DHCPRELEASE) ... IP address lease time ... MUST NOT
> ... Server identifier ... MUST" — table 5, DHCPRELEASE column, `rfc2131.txt:2082-2098`
>
> "'ciaddr' ... client's network address (DHCPRELEASE)" — table 5, `rfc2131.txt:2038-2041`

- Strength: must and must not. Class: wire.
- Check idea: the `ciaddr` of the DHCPRELEASE holds the leased address, option 54 is
  present, and options 50 and 51 are absent.

## Inform

### RFC2131-INF-1

**A client with its own address may ask for parameters alone with a DHCPINFORM.**

> "If a client has obtained a network address through some other means (e.g., manual
> configuration), it may use a DHCPINFORM request message to obtain other local
> configuration parameters." — §3.4, `rfc2131.txt:1117-1119`
>
> "The client places its own network address in the 'ciaddr' field. The client SHOULD NOT
> request lease time parameters." — §4.4.3, `rfc2131.txt:2167-2168`

- Strength: may for the message, should not for the lease request. Class: wire.
- Check idea: a client with a statically configured address sends a DHCPINFORM whose
  `ciaddr` holds that address.

### RFC2131-INF-2

**The server answers a DHCPINFORM with a DHCPACK to the address in `ciaddr`.**

> "The server responds to a DHCPINFORM message by sending a DHCPACK message directly to the
> address given in the 'ciaddr' field of the DHCPINFORM message." — §4.3.5,
> `rfc2131.txt:1822-1823`
>
> "The servers SHOULD unicast the DHCPACK reply to the address given in the 'ciaddr' field
> of the DHCPINFORM message." — §3.4, `rfc2131.txt:1131-1133`

- Strength: description in §4.3.5, should in §3.4. Class: wire.
- Check idea: the DHCPACK that answers the DHCPINFORM is a unicast to the `ciaddr` of that
  message.

### RFC2131-INF-3

**The answer to a DHCPINFORM carries no lease time and no `yiaddr`.**

> "The server MUST NOT send a lease expiration time to the client and SHOULD NOT fill in
> 'yiaddr'." — §4.3.5, `rfc2131.txt:1824-1825`

- Strength: must not and should not. Class: wire.
- Check idea: the DHCPACK that answers a DHCPINFORM carries no option 51, and its `yiaddr`
  is 0.0.0.0.

### RFC2131-INF-4

**The server does not look for a lease when it answers a DHCPINFORM.**

> "The server SHOULD check the network address in a DHCPINFORM message for consistency, but
> MUST NOT check for an existing lease." — §3.4, `rfc2131.txt:1135-1136`
>
> "Servers receiving a DHCPINFORM message construct a DHCPACK message with any local
> configuration parameters appropriate for the client without: allocating a new address,
> checking for an existing binding, filling in 'yiaddr' or including lease time
> parameters." — §3.4, `rfc2131.txt:1127-1131`

- Strength: must not. Class: internal, with an end-to-end consequence.
- Check idea: a DHCPINFORM from a client that holds no lease still draws a DHCPACK, and the
  server's pool of free addresses does not shrink.

### RFC2131-INF-5

**A DHCPINFORM carries no `requested IP address` and no lease time option.**

> "Requested IP address ... MUST NOT (INFORM) ... IP address lease time ... MUST NOT
> (INFORM)" — table 5, DHCPINFORM column, `rfc2131.txt:2082-2091`

- Strength: must not. Class: wire.
- Check idea: options 50 and 51 are absent from the DHCPINFORM.

### RFC2131-INF-6

**A DHCPINFORM goes to the DHCP server port.**

> "The client then unicasts the DHCPINFORM to the DHCP server if it knows the server's
> address, otherwise it broadcasts the message to the limited (all 1s) broadcast address.
> DHCPINFORM messages MUST be directed to the 'DHCP server' UDP port." — §4.4.3,
> `rfc2131.txt:2170-2173`

- Strength: must. Class: wire.
- Check idea: the UDP destination port of the DHCPINFORM is 67.

## Broadcast bit and reply delivery

### RFC2131-BCAST-1

**A client that cannot receive a unicast before it is configured sets the BROADCAST bit.**

> "A client that cannot receive unicast IP datagrams until its protocol software has been
> configured with an IP address SHOULD set the BROADCAST bit in the 'flags' field to 1 in
> any DHCPDISCOVER or DHCPREQUEST messages that client sends. ... A client that can receive
> unicast IP datagrams before its protocol software has been configured SHOULD clear the
> BROADCAST bit to 0." — §4.1, `rfc2131.txt:1362-1369`

- Strength: should, both halves. Class: wire.
- Check idea: the top bit of the `flags` field of the DHCPDISCOVER and of the DHCPREQUEST
  carries the value that matches what the client can receive.

### RFC2131-BCAST-2

**With a non-zero `giaddr` the server answers to the server port of the relay agent.**

> "If the 'giaddr' field in a DHCP message from a client is non-zero, the server sends any
> return messages to the 'DHCP server' port on the BOOTP relay agent whose address appears
> in 'giaddr'." — §4.1, `rfc2131.txt:1271-1273`

- Strength: description. Class: wire.
- Check idea: needs a relay agent on the path; without one, `giaddr` is always zero.

### RFC2131-BCAST-3

**With `giaddr` zero and `ciaddr` non-zero the server unicasts to `ciaddr`.**

> "If the 'giaddr' field is zero and the 'ciaddr' field is nonzero, then the server unicasts
> DHCPOFFER and DHCPACK messages to the address in 'ciaddr'." — §4.1, `rfc2131.txt:1273-1275`

- Strength: description. Class: wire.
- Check idea: the DHCPACK that answers a renewal is a unicast to the address in the
  `ciaddr` of the renewal message.

### RFC2131-BCAST-4

**With both zero and the broadcast bit set the server broadcasts to 255.255.255.255.**

> "If 'giaddr' is zero and 'ciaddr' is zero, and the broadcast bit is set, then the server
> broadcasts DHCPOFFER and DHCPACK messages to 0xffffffff." — §4.1, `rfc2131.txt:1276-1278`

- Strength: description. Class: wire.
- Check idea: with the BROADCAST bit set in the DHCPDISCOVER, the IP destination address of
  the DHCPOFFER is 255.255.255.255.

### RFC2131-BCAST-5

**With both zero and the broadcast bit clear the server unicasts to `chaddr` and `yiaddr`.**

> "If the broadcast bit is not set and 'giaddr' is zero and 'ciaddr' is zero, then the
> server unicasts DHCPOFFER and DHCPACK messages to the client's hardware address and
> 'yiaddr' address." — §4.1, `rfc2131.txt:1278-1280`

- Strength: description. Class: wire.
- Check idea: with the BROADCAST bit clear, the IP destination address of the DHCPOFFER is
  the `yiaddr` it carries and the link-layer destination address is the `chaddr`.

### RFC2131-BCAST-6

**A server that answers a client directly should read the BROADCAST bit and obey it.**

> "A server or relay agent sending or relaying a DHCP message directly to a DHCP client
> (i.e., not to a relay agent specified in the 'giaddr' field) SHOULD examine the BROADCAST
> bit in the 'flags' field. If this bit is set to 1, the DHCP message SHOULD be sent as an
> IP broadcast using an IP broadcast address (preferably 0xffffffff) as the IP destination
> address and the link-layer broadcast address as the link-layer destination address. If
> the BROADCAST bit is cleared to 0, the message SHOULD be sent as an IP unicast to the IP
> address specified in the 'yiaddr' field and the link-layer address specified in the
> 'chaddr' field." — §4.1, `rfc2131.txt:1373-1382`

- Strength: should. Class: wire.
- Check idea: two clients on one subnet, one with the bit set and one with it clear, draw
  two answers with two different destination addresses.

## Server identifier

### RFC2131-SRVID-1

**A server accepts any of its own addresses as its identifier.**

> "A server with multiple network addresses MUST be prepared to to accept any of its
> network addresses as identifying that server in a DHCP message." — §4.1,
> `rfc2131.txt:1249-1251`

- Strength: must. Class: internal, with an end-to-end consequence.
- Check idea: a server with two addresses answers a DHCPREQUEST whose option 54 names
  either one of them.

### RFC2131-SRVID-2

**A server chooses a `server identifier` that the client can reach.**

> "To accommodate potentially incomplete network connectivity, a server MUST choose an
> address as a 'server identifier' that, to the best of the server's knowledge, is
> reachable from the client. For example, if the DHCP server and the DHCP client are
> connected to the same subnet (i.e., the 'giaddr' field in the message from the client is
> zero), the server SHOULD select the IP address the server is using for communication on
> that subnet as the 'server identifier'." — §4.1, `rfc2131.txt:1251-1258`

- Strength: must, with a should for the common case. Class: wire.
- Check idea: with a server that has two interfaces, option 54 of the DHCPOFFER holds the
  address of the interface that faces the client.

### RFC2131-SRVID-3

**A client unicasts to the address of the `server identifier` option.**

> "DHCP clients MUST use the IP address provided in the 'server identifier' option for any
> unicast requests to the DHCP server." — §4.1, `rfc2131.txt:1263-1265`

- Strength: must. Class: wire.
- Check idea: the IP destination address of the renewal DHCPREQUEST and of the DHCPRELEASE
  equals the address that option 54 of the DHCPACK carried.

## Client identity

### RFC2131-ID-1

**A `client identifier` is unique to its client within the subnet.**

> "The 'client identifier' chosen by a DHCP client MUST be unique to that client within the
> subnet to which the client is attached." — §2, `rfc2131.txt:497-499`

- Strength: must. Class: internal.
- Check idea: two clients on one subnet carry two different values in option 61.

### RFC2131-ID-2

**The server identifies the client by the `client identifier`, or by `chaddr` when there is
none.**

> "If the client supplies a 'client identifier', the client MUST use the same 'client
> identifier' in all subsequent messages, and the server MUST use that identifier to
> identify the client. If the client does not provide a 'client identifier' option, the
> server MUST use the contents of the 'chaddr' field to identify the client." — §4.2,
> `rfc2131.txt:1417-1422`

- Strength: must. Class: internal, with an end-to-end consequence.
- Check idea: a client that keeps its `client identifier` and changes its hardware address
  gets the same address back; a client that carries no identifier and changes its hardware
  address gets a new one.

## Server selection rules

### RFC2131-SEL-1

**The server chooses the address by a stated order of preference.**

> "If an address is available, the new address SHOULD be chosen as follows: o The client's
> current address as recorded in the client's current binding, ELSE o The client's previous
> address as recorded in the client's (now expired or released) binding, if that address is
> in the server's pool of available addresses and not already allocated, ELSE o The address
> requested in the 'Requested IP Address' option, if that address is valid and not already
> allocated, ELSE o A new address allocated from the server's pool of available addresses"
> — §4.3.1, `rfc2131.txt:1472-1486`

- Strength: should. Class: end-to-end.
- Check idea: a client that asks again after a release gets its earlier address back, while
  a client that never asked before gets a fresh one.

### RFC2131-SEL-2

**The server should not give the offered address to another client before the answer comes.**

> "While not required for correct operation of DHCP, the server SHOULD NOT reuse the
> selected network address before the client responds to the server's DHCPOFFER message."
> — §4.3.1, `rfc2131.txt:1504-1506`

- Strength: should not. Class: end-to-end.
- Check idea: two clients send a DHCPDISCOVER one after the other and neither one answers;
  the two DHCPOFFER messages carry two different addresses.

### RFC2131-SEL-3

**The server chooses the lease duration by a stated order of preference.**

> "IF the client has not requested a specific lease in the DHCPDISCOVER message and the
> client does not have an assigned network address, the server assigns a locally configured
> default lease time, ELSE IF the client has requested a specific lease in the DHCPDISCOVER
> message (regardless of whether the client has an assigned network address), the server
> may choose either to return the requested lease (if the lease is acceptable to local
> policy) or select another lease." — §4.3.1, `rfc2131.txt:1526-1535`

- Strength: description with a `may` for the last branch. Class: wire.
- Check idea: with no option 51 in the DHCPDISCOVER, option 51 of the DHCPOFFER holds the
  configured default of the server.

### RFC2131-SEL-4

**The server returns the address, the lease expiry and the requested parameters.**

> "The server MUST return to the client: o The client's network address, as determined by
> the rules given earlier in this section, o The expiration time for the client's lease, as
> determined by the rules given earlier in this section, o Parameters requested by the
> client, according to the following rules: -- IF the server has been explicitly configured
> with a default value for the parameter, the server MUST include that value in an
> appropriate option in the 'option' field" — §4.3.1, `rfc2131.txt:1601-1614`

- Strength: must. Class: wire.
- Check idea: a client that asks for the subnet mask and the router in option 55 gets
  option 1 and option 3 in the DHCPOFFER and in the DHCPACK.

### RFC2131-SEL-5

**The server omits a parameter it has no value for, and returns each requested parameter
once.**

> "-- The server MUST NOT return a value for that parameter, The server MUST supply as many
> of the requested parameters as possible and MUST omit any parameters it cannot provide.
> The server MUST include each requested parameter only once unless explicitly allowed in
> the DHCP Options and BOOTP Vendor Extensions document." — §4.3.1, `rfc2131.txt:1622-1635`

- Strength: must not and must. Class: wire.
- Check idea: a client that asks for an option the server has no value for gets a reply
  without that option code, and no option code occurs twice in the reply.

## Other

### RFC2131-MISC-1

**A client with several interfaces runs DHCP on each one on its own.**

> "A client with multiple network interfaces must use DHCP through each interface
> independently to obtain configuration information parameters for those separate
> interfaces." — §3.6, `rfc2131.txt:1198-1200`

- Strength: must (lowercase). Class: wire.
- Check idea: a client with two interfaces sends one DHCPDISCOVER on each one, with a
  `chaddr` of that interface, and configures the two interfaces from two separate
  exchanges.

## Out of scope in this catalog

The entries above hold every `MUST`, `MUST NOT`, `SHOULD`, `SHOULD NOT` and `MAY` of
RFC 2131 that states a behavior of a client or of a server, and every field value of
tables 1, 3, 4 and 5. What is left out, and why:

- **The design goals of §1.6**, `rfc2131.txt:322-389`. They use the word "must" about the
  protocol as a whole ("DHCP must work across routers"), not about a message. Each one
  becomes a real requirement further down the document, and the entry is there.
- **The administrative statements**: a host should not act as a server unless configured
  to, `rfc2131.txt:102-103`; a server need not answer every message, `rfc2131.txt:1389-1393`;
  a server may refuse an address for administrative reasons, `rfc2131.txt:1490-1493`; the
  vendor class rules of §4.2, `rfc2131.txt:1410-1437`. These say what an administrator may
  configure, not what the protocol does. The permission half is recorded where it weakens
  a statement above, for example in [RFC2131-OFF-1](#rfc2131-off-1).
- **The non-requirements**: "the correct operation of DHCP does not depend on the
  transmission of DHCPRELEASE messages", `rfc2131.txt:2284-2285`, and "Servers need not
  reserve the offered network address", `rfc2131.txt:722-724`. A statement that nothing is
  required cannot fail.
- **The three allocation mechanisms of §1**, `rfc2131.txt:124-147`. They name a policy of
  the administrator, and the protocol is the same for all three.
- **Appendix A**, the list of host configuration parameters. It points into RFC 1122 and
  RFC 2132 and defines nothing itself.
- **The security considerations of §7**, `rfc2131.txt:2398-2427`. The section states that
  DHCP is insecure; it demands nothing.
- **Everything about a relay agent** beyond the two entries that record it,
  [RFC2131-NAK-4](#rfc2131-nak-4) and [RFC2131-BCAST-2](#rfc2131-bcast-2). RFC 2131
  delegates the relay agent to RFC 1542, which is out of the in-scope set; see
  [`standards.md`](../../protocol/dhcp/standards.md#in-scope-set).
