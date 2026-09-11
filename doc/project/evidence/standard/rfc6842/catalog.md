# RFC 6842 (client identifier in server replies) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC6842-*` · **Stands on:** [standards.md](../../protocol/dhcp/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow, for the one later
document of the in-scope set: RFC 6842, Client Identifier Option in DHCP Server Replies, of
January 2013. It updates RFC 2131. The catalog comes from the RFC text only. It contains no
simulation model names and no code references.

Source, cached in this folder:

- `rfc6842.txt` — Client Identifier Option in DHCP Server Replies, January 2013. Downloaded
  2026-09-11 from <https://www.rfc-editor.org/rfc/rfc6842.txt>.

This document is four pages long, and one section of it is normative: §3, `Modification to
RFC 2131`. It reverses a prohibition of the base document. RFC 2131 table 3 says that a
server `MUST NOT` return the `client identifier` option in a DHCPOFFER or a DHCPACK; sixteen
years of operation showed that the absence of the option loses replies and confuses clients
that share a hardware address, and this document turns the prohibition into a requirement.

The statements it changes are in [`rfc2131/catalog.md`](../rfc2131/catalog.md):
[RFC2131-OFF-5](../rfc2131/catalog.md#rfc2131-off-5) and
[RFC2131-ACK-5](../rfc2131/catalog.md#rfc2131-ack-5) carry the `Overridden by` field that
points here. The row of the override table is in
[`standards.md`](../../protocol/dhcp/standards.md#override-table).

The state of the workflow — which statement a check targets, which test carries it, and
what the run said — is **not** in this document. It lives in the coverage ledger,
[`dhcp/coverage.md`](../../model/dhcp/coverage.md).

Quotes are verbatim. A reference such as `rfc6842.txt:159` points to a line of the cached
file in this folder.

## How to read an entry

The conventions of an entry are the ones of
[`rfc2131/catalog.md`](../rfc2131/catalog.md#how-to-read-an-entry). This document uses the
keywords of RFC 2119, `rfc6842.txt:89-91`, and not the local definitions of RFC 2131 §1.4.

## Index

| ID | Statement |
| --- | --- |
| [RFC6842-CLID-1](#rfc6842-clid-1) | A server returns the `client identifier` option unaltered when the client sent one. |
| [RFC6842-CLID-2](#rfc6842-clid-2) | A server returns no `client identifier` option when the client sent none. |
| [RFC6842-CLID-3](#rfc6842-clid-3) | A client discards a message in silence when its `client identifier` is not the client's own. |

## The client identifier in a reply

### RFC6842-CLID-1

**A server returns the `client identifier` option unaltered when the client sent one.**

> "If the 'client identifier' option is present in a message received from a client, the
> server MUST return the 'client identifier' option, unaltered, in its response message."
> — §3, `rfc6842.txt:158-160`
>
> "Client identifier (if sent by client) ... MUST MUST MUST" — §3, the modified table,
> `rfc6842.txt:175-178`

- Strength: must. Class: wire.
- This statement replaces the `client identifier` row of RFC 2131 table 3,
  `rfc2131.txt:1584`, for all three server messages: DHCPOFFER, DHCPACK and DHCPNAK. The
  RFC 2131 entries keep their IDs and stay in the catalog; see
  [RFC2131-OFF-5](../rfc2131/catalog.md#rfc2131-off-5) and
  [RFC2131-ACK-5](../rfc2131/catalog.md#rfc2131-ack-5).
- Check idea: a client that carries option 61 in its DHCPDISCOVER and its DHCPREQUEST gets a
  DHCPOFFER and a DHCPACK that carry option 61 with the same octets.

### RFC6842-CLID-2

**A server returns no `client identifier` option when the client sent none.**

> "Client identifier (if not sent by client) ... MUST NOT MUST NOT MUST NOT" — §3, the
> modified table, `rfc6842.txt:179-180`

- Strength: must not. Class: wire.
- The row keeps the prohibition of RFC 2131 for the case the base document had in mind. So a
  server obeys this document and RFC 2131 at the same time whenever the client sends no
  identifier, and the two documents disagree only in the other case.
- Check idea: a client whose messages carry no option 61 gets a DHCPOFFER and a DHCPACK that
  carry no option 61 either.

### RFC6842-CLID-3

**A client discards a message in silence when its `client identifier` is not the client's
own.**

> "When a client receives a DHCP message containing a 'client identifier' option, the client
> MUST compare that client identifier to the one it is configured to send. If the two client
> identifiers do not match, the client MUST silently discard the message." — §3,
> `rfc6842.txt:183-186`

- Strength: must, twice. Class: end-to-end.
- The statement is new. RFC 2131 gives a client one test for a reply, the `xid`
  ([RFC2131-XID-4](../rfc2131/catalog.md#rfc2131-xid-4)), and §2 of this document explains
  why that test is not enough: two clients on one subnet may pick the same `xid`,
  `rfc6842.txt:128-135`.
- Check idea: a crafted DHCPOFFER whose option 61 holds a foreign identifier reaches a client
  in SELECTING; the client does not answer it and does not take its address. The `xid` of the
  crafted message is the right one, so only the identifier can be the reason for the discard.

## Out of scope in this catalog

RFC 6842 is exhausted by the three entries above. What is left out:

- **§1 and §2**, the introduction and the problem statement, `rfc6842.txt:73-154`. They
  explain why the change was made. The one sentence with operational weight, that a relay
  agent or a server `MAY` drop a message that carries neither a `client identifier` nor a
  `chaddr`, `rfc6842.txt:100-102`, is a report of what RFC 2131 implementations do and not a
  new rule of this document.
- **§4, security considerations**, `rfc6842.txt:188-198`. It notes that the identifier
  reaches every network operator the client meets, and states that this document does nothing
  about it. It demands nothing.
