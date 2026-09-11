# RFC 2132 (DHCP options) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC2132-*` · **Stands on:** [standards.md](../../protocol/dhcp/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow, for one document of the
in-scope set: RFC 2132, DHCP Options and BOOTP Vendor Extensions, of March 1997. The catalog
comes from the RFC text only. It contains no simulation model names and no code references.

Source, cached in this folder:

- `rfc2132.txt` — DHCP Options and BOOTP Vendor Extensions, March 1997. Downloaded
  2026-09-11 from <https://www.rfc-editor.org/rfc/rfc2132.txt>.

RFC 2131 §3 delegates its whole option set to this document, so no DHCP message is legal
without it: the `DHCP message type` option that RFC 2131 demands in every message is
defined here, in §9.6. The behavior that uses each option is in
[`rfc2131/catalog.md`](../rfc2131/catalog.md). The family of documents and the in-scope set
are in [`standards.md`](../../protocol/dhcp/standards.md). The features these statements
build are in [`features.md`](../../protocol/dhcp/features.md).

The state of the workflow — which statement a check targets, which test carries it, and
what the run said — is **not** in this document. It lives in the coverage ledger,
[`dhcp/coverage.md`](../../model/dhcp/coverage.md).

Quotes are verbatim. A reference such as `rfc2132.txt:1478` points to a line of the cached
file in this folder.

## How to read an entry

The conventions of an entry — strength, class and the `Overridden by` field — are the ones
of [`rfc2131/catalog.md`](../rfc2131/catalog.md#how-to-read-an-entry).

Most of this document is a list of definitions, one section per option, and a definition is
an `encoding` statement: the code, the length and the layout of the octets. Only a few
sections add a rule about behavior, and those are the entries with a keyword.

## Index

| ID | Statement |
| --- | --- |
| [RFC2132-FMT-1](#rfc2132-fmt-1) | An option is a tag octet, a length octet and the data; only the codes 0 and 255 carry no length. |
| [RFC2132-FMT-2](#rfc2132-fmt-2) | A text option should carry no trailing null, and a receiver must accept one and must not demand one. |
| [RFC2132-FMT-3](#rfc2132-fmt-3) | Every multi-octet value is in network byte order. |
| [RFC2132-FMT-4](#rfc2132-fmt-4) | The magic cookie is 99.130.83.99. |
| [RFC2132-FMT-5](#rfc2132-fmt-5) | The codes 128 to 254 are reserved for site-specific options. |
| [RFC2132-PAD-1](#rfc2132-pad-1) | The `pad` option is code 0 and one octet long. |
| [RFC2132-END-1](#rfc2132-end-1) | The `end` option is code 255, one octet long, and it marks the end of the valid data. |
| [RFC2132-MASK-1](#rfc2132-mask-1) | The `subnet mask` option is code 1, four octets, and it comes before the `router` option. |
| [RFC2132-ROUTER-1](#rfc2132-router-1) | The `router` option is code 3 and a list of addresses whose length is a multiple of four. |
| [RFC2132-REQIP-1](#rfc2132-reqip-1) | The `requested IP address` option is code 50 and four octets. |
| [RFC2132-LEASE-1](#rfc2132-lease-1) | The `IP address lease time` option is code 51 and a 32-bit count of seconds. |
| [RFC2132-OVER-1](#rfc2132-over-1) | The `option overload` option is code 52 with the values 1, 2 or 3. |
| [RFC2132-TYPE-1](#rfc2132-type-1) | The `DHCP message type` option is code 53 with the values 1 to 8. |
| [RFC2132-SRVID-1](#rfc2132-srvid-1) | The `server identifier` option is code 54 and holds the address of the selected server. |
| [RFC2132-PRL-1](#rfc2132-prl-1) | The `parameter request list` option is code 55, and the server tries to keep the order the client asked for. |
| [RFC2132-MSGOPT-1](#rfc2132-msgopt-1) | The `message` option is code 56 and holds text for a person to read. |
| [RFC2132-MAXSZ-1](#rfc2132-maxsz-1) | The `maximum DHCP message size` option is code 57, two octets, and its smallest legal value is 576. |
| [RFC2132-T1-1](#rfc2132-t1-1) | The `renewal (T1) time value` option is code 58 and a 32-bit count of seconds. |
| [RFC2132-T2-1](#rfc2132-t2-1) | The `rebinding (T2) time value` option is code 59 and a 32-bit count of seconds. |
| [RFC2132-VCLASS-1](#rfc2132-vclass-1) | The `vendor class identifier` option is code 60, and a server that cannot read it ignores it. |
| [RFC2132-CLID-1](#rfc2132-clid-1) | The `client identifier` option is code 61, a type octet and an identifier that is unique on the subnet. |
| [RFC2132-TFTP-1](#rfc2132-tftp-1) | The `TFTP server name` option is code 66, for use when `sname` holds options. |
| [RFC2132-BOOTF-1](#rfc2132-bootf-1) | The `bootfile name` option is code 67, for use when `file` holds options. |

## Option field format

### RFC2132-FMT-1

**An option is a tag octet, a length octet and the data; only the codes 0 and 255 carry no
length.**

> "All options begin with a tag octet, which uniquely identifies the option. Fixed-length
> options without data consist of only a tag octet. Only options 0 and 255 are fixed
> length. All other options are variable-length with a length octet following the tag
> octet. The value of the length octet does not include the two octets specifying the tag
> and length." — §2, `rfc2132.txt:191-196`

- Strength: description. Class: encoding.
- Check idea: a serializer test. On a link, the visible consequence is that a receiver
  parsed the option area and found the options that the sender put in it.

### RFC2132-FMT-2

**A text option should carry no trailing null, and a receiver must accept one and must not
demand one.**

> "Options containing NVT ASCII data SHOULD NOT include a trailing NULL; however, the
> receiver of such options MUST be prepared to delete trailing nulls if they exist. The
> receiver MUST NOT require that a trailing null be included in the data." — §2,
> `rfc2132.txt:197-201`

- Strength: should not, must, must not. Class: encoding for the send half, end-to-end for
  the receive half.
- Check idea: a crafted reply whose text option ends with a null octet configures the
  client, and so does one whose text option does not.

### RFC2132-FMT-3

**Every multi-octet value is in network byte order.**

> "All multi-octet quantities are in network byte-order." — §2, `rfc2132.txt:207`

- Strength: description. Class: encoding.
- Check idea: a serializer test. A lease time of 100 seconds appears on the wire as the four
  octets 00 00 00 64.

### RFC2132-FMT-4

**The magic cookie is 99.130.83.99.**

> "When used with BOOTP, the first four octets of the vendor information field have been
> assigned to the "magic cookie" (as suggested in RFC 951). This field identifies the mode
> in which the succeeding data is to be interpreted. The value of the magic cookie is the 4
> octet dotted decimal 99.130.83.99 (or hexadecimal number 63.82.53.63) in network byte
> order." — §2, `rfc2132.txt:209-214`

- Strength: description. Class: encoding.
- Check idea: the same value as [RFC2131-MSG-2](../rfc2131/catalog.md#rfc2131-msg-2), stated
  here from the side of the option format.

### RFC2132-FMT-5

**The codes 128 to 254 are reserved for site-specific options.**

> "Option codes 128 to 254 (decimal) are reserved for site-specific options." — §2,
> `rfc2132.txt:219-220`

- Strength: description. Class: encoding.
- Check idea: no message carries a code in that range unless the scenario put it there.

## RFC 1497 extensions that DHCP needs

### RFC2132-PAD-1

**The `pad` option is code 0 and one octet long.**

> "The pad option can be used to cause subsequent fields to align on word boundaries. The
> code for the pad option is 0, and its length is 1 octet." — §3.1, `rfc2132.txt:250-253`

- Strength: description. Class: encoding.
- Check idea: a serializer test. The option carries no length octet, which is the exception
  that [RFC2132-FMT-1](#rfc2132-fmt-1) names.

### RFC2132-END-1

**The `end` option is code 255, one octet long, and it marks the end of the valid data.**

> "The end option marks the end of valid information in the vendor field. Subsequent octets
> should be filled with pad options. The code for the end option is 255, and its length is 1
> octet." — §3.2, `rfc2132.txt:262-265`

- Strength: description, with a should for the padding. Class: encoding.
- Check idea: the last option of every message is code 255; see also
  [RFC2131-MSG-4](../rfc2131/catalog.md#rfc2131-msg-4), which makes it a rule of the
  protocol and not only of the format.

### RFC2132-MASK-1

**The `subnet mask` option is code 1, four octets, and it comes before the `router` option.**

> "The subnet mask option specifies the client's subnet mask as per RFC 950 [5]. If both the
> subnet mask and the router option are specified in a DHCP reply, the subnet mask option
> MUST be first. The code for the subnet mask option is 1, and its length is 4 octets."
> — §3.3, `rfc2132.txt:274-287`

- Strength: must for the order, description for the code and the length. Class: wire.
- Check idea: a reply that carries both options carries option 1 at a lower offset than
  option 3, and the client takes its subnet mask from option 1.

### RFC2132-ROUTER-1

**The `router` option is code 3 and a list of addresses whose length is a multiple of four.**

> "The router option specifies a list of IP addresses for routers on the client's subnet.
> Routers SHOULD be listed in order of preference. The code for the router option is 3. The
> minimum length for the router option is 4 octets, and the length MUST always be a multiple
> of 4." — §3.5, `rfc2132.txt:311-316`

- Strength: must for the length, should for the order. Class: wire.
- Check idea: the length octet of option 3 is 4, 8 or another multiple of four, and the
  client installs a default route through the first address in it.

## DHCP extensions

### RFC2132-REQIP-1

**The `requested IP address` option is code 50 and four octets.**

> "This option is used in a client request (DHCPDISCOVER) to allow the client to request
> that a particular IP address be assigned. The code for this option is 50, and its length
> is 4." — §9.1, `rfc2132.txt:1381-1384`

- Strength: description. Class: wire.
- Check idea: option 50 holds one address in four octets. Which message may carry it is a
  rule of RFC 2131 table 5.

### RFC2132-LEASE-1

**The `IP address lease time` option is code 51 and a 32-bit count of seconds.**

> "The time is in units of seconds, and is specified as a 32-bit unsigned integer. The code
> for this option is 51, and its length is 4." — §9.2, `rfc2132.txt:1407-1410`

- Strength: description. Class: wire.
- Check idea: option 51 of a DHCPOFFER holds the lease in seconds as four octets.

### RFC2132-OVER-1

**The `option overload` option is code 52 with the values 1, 2 or 3.**

> "This option is used to indicate that the DHCP 'sname' or 'file' fields are being
> overloaded by using them to carry DHCP options. ... The code for this option is 52, and
> its length is 1. Legal values for this option are: 1 the 'file' field is used to hold
> options 2 the 'sname' field is used to hold options 3 both fields are used to hold
> options" — §9.3, `rfc2132.txt:1419-1435`

- Strength: description. Class: encoding.
- Check idea: a serializer test, paired with
  [RFC2131-MSG-10](../rfc2131/catalog.md#rfc2131-msg-10).

### RFC2132-TYPE-1

**The `DHCP message type` option is code 53 with the values 1 to 8.**

> "This option is used to convey the type of the DHCP message. The code for this option is
> 53, and its length is 1. Legal values for this option are: 1 DHCPDISCOVER 2 DHCPOFFER 3
> DHCPREQUEST 4 DHCPDECLINE 5 DHCPACK 6 DHCPNAK 7 DHCPRELEASE 8 DHCPINFORM" — §9.6,
> `rfc2132.txt:1477-1490`

- Strength: description. Class: wire.
- Check idea: every message carries option 53, and its value is the number of the type of
  that message. This is the one option that
  [RFC2131-MSG-3](../rfc2131/catalog.md#rfc2131-msg-3) makes mandatory everywhere, and
  every other check of this protocol uses it to name the message it looks for.

### RFC2132-SRVID-1

**The `server identifier` option is code 54 and holds the address of the selected server.**

> "This option is used in DHCPOFFER and DHCPREQUEST messages, and may optionally be included
> in the DHCPACK and DHCPNAK messages. ... DHCP clients use the contents of the 'server
> identifier' field as the destination address for any DHCP messages unicast to the DHCP
> server. ... The identifier is the IP address of the selected server. The code for this
> option is 54, and its length is 4." — §9.7, `rfc2132.txt:1499-1510`

- Strength: description. Class: wire.
- Check idea: option 54 holds one address in four octets. RFC 2131 table 3 is stricter than
  the "may optionally be included" of this section: it makes the option a `MUST` in
  DHCPACK and DHCPNAK. Both citations stand, and the RFC 2131 one governs, because it
  states the behavior of the message; see
  [RFC2131-ACK-4](../rfc2131/catalog.md#rfc2131-ack-4) and
  [RFC2131-NAK-6](../rfc2131/catalog.md#rfc2131-nak-6).

### RFC2132-PRL-1

**The `parameter request list` option is code 55, and the server tries to keep the order the
client asked for.**

> "This option is used by a DHCP client to request values for specified configuration
> parameters. The list of requested parameters is specified as n octets, where each octet is
> a valid DHCP option code as defined in this document. The client MAY list the options in
> order of preference. The DHCP server is not required to return the options in the
> requested order, but MUST try to insert the requested options in the order requested by
> the client. The code for this option is 55. Its minimum length is 1." — §9.8,
> `rfc2132.txt:1526-1536`

- Strength: must for the attempt at the order, may for the order of preference. Class: wire.
- Check idea: option 55 of a DHCPDISCOVER holds the codes the client wants, and the reply
  carries those option codes. The "MUST try" is not checkable as written: a failed attempt
  and no attempt look the same on a link.

### RFC2132-MSGOPT-1

**The `message` option is code 56 and holds text for a person to read.**

> "This option is used by a DHCP server to provide an error message to a DHCP client in a
> DHCPNAK message in the event of a failure. A client may use this option in a DHCPDECLINE
> message to indicate the why the client declined the offered parameters. ... The code for
> this option is 56 and its minimum length is 1." — §9.9, `rfc2132.txt:1545-1552`

- Strength: description, with a may for the client. Class: wire.
- Check idea: a DHCPNAK may carry option 56. RFC 2131 table 3 makes it a `SHOULD` for all
  three server messages, `rfc2131.txt:1583`.

### RFC2132-MAXSZ-1

**The `maximum DHCP message size` option is code 57, two octets, and its smallest legal
value is 576.**

> "This option specifies the maximum length DHCP message that it is willing to accept. The
> length is specified as an unsigned 16-bit integer. A client may use the maximum DHCP
> message size option in DHCPDISCOVER or DHCPREQUEST messages, but should not use the option
> in DHCPDECLINE messages. ... The code for this option is 57, and its length is 2. The
> minimum legal value is 576 octets." — §9.10, `rfc2132.txt:1561-1576`

- Strength: description, with a may and a should not. Class: wire.
- Check idea: when a client sends option 57, its value is at least 576, and the reply of the
  server is no longer than that value.

### RFC2132-T1-1

**The `renewal (T1) time value` option is code 58 and a 32-bit count of seconds.**

> "This option specifies the time interval from address assignment until the client
> transitions to the RENEWING state. The value is in units of seconds, and is specified as a
> 32-bit unsigned integer. The code for this option is 58, and its length is 4." — §9.11,
> `rfc2132.txt:1585-1591`

- Strength: description. Class: wire.
- Check idea: option 58 of the DHCPACK holds a number of seconds, and the client renews that
  many seconds after the assignment.

### RFC2132-T2-1

**The `rebinding (T2) time value` option is code 59 and a 32-bit count of seconds.**

> "This option specifies the time interval from address assignment until the client
> transitions to the REBINDING state. The value is in units of seconds, and is specified as
> a 32-bit unsigned integer. The code for this option is 59, and its length is 4." — §9.12,
> `rfc2132.txt:1600-1606`

- Strength: description. Class: wire.
- Check idea: option 59 of the DHCPACK holds a number of seconds, and the client rebinds
  that many seconds after the assignment.

### RFC2132-VCLASS-1

**The `vendor class identifier` option is code 60, and a server that cannot read it ignores
it.**

> "This option is used by DHCP clients to optionally identify the vendor type and
> configuration of a DHCP client. ... Servers not equipped to interpret the class-specific
> information sent by a client MUST ignore it (although it may be reported). Servers that
> respond SHOULD only use option 43 to return the vendor-specific information to the client.
> The code for this option is 60, and its minimum length is 1." — §9.13,
> `rfc2132.txt:1615-1634`

- Strength: must for the ignore rule, should for option 43. Class: end-to-end.
- Check idea: a DHCPDISCOVER that carries an option 60 the server does not know still draws
  a DHCPOFFER, and that answer is the same one the client would get without the option.

### RFC2132-CLID-1

**The `client identifier` option is code 61, a type octet and an identifier that is unique on
the subnet.**

> "This option is used by DHCP clients to specify their unique identifier. DHCP servers use
> this value to index their database of address bindings. ... Identifiers SHOULD be treated
> as opaque objects by DHCP servers. The client identifier MAY consist of type-value pairs
> similar to the 'htype'/'chaddr' fields defined in [3]. ... For correct identification of
> clients, each client's client-identifier MUST be unique among the client-identifiers used
> on the subnet to which the client is attached. ... The code for this option is 61, and its
> minimum length is 2." — §9.14, `rfc2132.txt:1643-1664`

- Strength: must for the uniqueness, should for the opacity, may for the form. Class: wire.
- Check idea: option 61 holds at least two octets, the first one a hardware type or zero.
  Two clients on one subnet carry two different values; see also
  [RFC2131-ID-1](../rfc2131/catalog.md#rfc2131-id-1). RFC 4361 replaces the form of the
  value and is out of scope.

### RFC2132-TFTP-1

**The `TFTP server name` option is code 66, for use when `sname` holds options.**

> "This option is used to identify a TFTP server when the 'sname' field in the DHCP header
> has been used for DHCP options. The code for this option is 66, and its minimum length is
> 1." — §9.4, `rfc2132.txt:1444-1447`

- Strength: description. Class: encoding.
- Check idea: a serializer test, paired with
  [RFC2131-MSG-10](../rfc2131/catalog.md#rfc2131-msg-10). The option exists because the
  field that would carry the name holds options instead.

### RFC2132-BOOTF-1

**The `bootfile name` option is code 67, for use when `file` holds options.**

> "This option is used to identify a bootfile when the 'file' field in the DHCP header has
> been used for DHCP options. The code for this option is 67, and its minimum length is 1."
> — §9.5, `rfc2132.txt:1465-1468`

- Strength: description. Class: encoding.
- Check idea: the same as [RFC2132-TFTP-1](#rfc2132-tftp-1), for the other field.

## Out of scope in this catalog

This catalog holds §2, the two RFC 1497 extensions that a DHCP reply needs for an IPv4 host
to work (`subnet mask` and `router`), and the whole of §9, the options that are specific to
DHCP.

Out of scope, with the reason:

- **§3.4 and §3.6 to §3.20, §4, §5, §6, §7 and §8** — about 60 sections, each one the
  definition of one configuration parameter: time servers, name servers, the path MTU
  plateau table, the TCP keepalive interval, NetBIOS, the X Window System, and more. Each
  one is an `encoding` statement of the same shape as
  [RFC2132-MASK-1](#rfc2132-mask-1): a code, a length and a layout. None of them states a
  behavior of DHCP, and their defaults live in RFC 1122 and RFC 1123. They are the level 5
  area of this document, and a serializer unit test is their home, not a protocol check.
  Two of them are in scope, and only because a client cannot use an address without them.
- **§8.4, `vendor specific information` (option 43)**, `rfc2132.txt:1020-1078`. It defines a
  container whose content is the vendor's, and the encapsulated option space inside it is a
  level 5 matter. The one rule that reaches outside it is in
  [RFC2132-VCLASS-1](#rfc2132-vclass-1).
- **§10, defining new extensions**, `rfc2132.txt:1687-1714`. It states a procedure for a
  person who writes a new option, not a behavior of a node.
- **§13, security considerations**. It demands nothing.
