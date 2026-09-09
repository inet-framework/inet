# RFC 768 (UDP) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC768-*` · **Stands on:** [standards.md](../../protocol/udp/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow, for one document of
the in-scope set: RFC 768, the User Datagram Protocol of August 1980, still the Internet
Standard. The document is three pages long, and nearly all of it is checkable. The catalog
comes from the RFC text only. It contains no simulation model names and no code references.

Source, cached in this folder:

- `rfc768.txt` — User Datagram Protocol, 28 August 1980. Downloaded 2026-09-08 from
  <https://www.rfc-editor.org/rfc/rfc768.txt>.

The error report that answers a datagram to a closed port belongs to RFC 792 and lives in
[`rfc792/catalog.md`](../rfc792/catalog.md). The family of documents and the in-scope set
are in [`standards.md`](../../protocol/udp/standards.md). The features these statements
build are in [`features.md`](../../protocol/udp/features.md).

The state of the workflow — which statement a check targets, which test carries it, and
what the run said — is **not** in this document. It lives in the coverage ledger,
[`udp/coverage.md`](../../model/udp/coverage.md). Keeping it out is deliberate: this
catalog states what the standard says, so a new test or a new run must never force an edit
here.

Quotes are verbatim. A reference such as `rfc768.txt:69` points to a line of the cached
file in this folder.

## Index

| ID | Statement |
| --- | --- |
| [RFC768-HDR-1](#rfc768-hdr-1) | The source port is optional; it is zero when unused. |
| [RFC768-HDR-2](#rfc768-hdr-2) | The destination port selects the receiver within the destination address. |
| [RFC768-HDR-3](#rfc768-hdr-3) | The length counts header and data in octets; the minimum is eight. |
| [RFC768-CKSUM-1](#rfc768-cksum-1) | The checksum covers a pseudo header, the UDP header and the data. |
| [RFC768-CKSUM-2](#rfc768-cksum-2) | A computed zero is sent as all ones; an all-zero checksum means none was generated. |
| [RFC768-UI-1](#rfc768-ui-1) | A receive operation returns the data, the source port and the source address. |
| [RFC768-IP-1](#rfc768-ip-1) | The UDP module determines the addresses and the protocol from the IP header. |
| [RFC768-PROTO-1](#rfc768-proto-1) | UDP is protocol 17 in IP. |

## How to read an entry

The conventions of an entry — strength, class, and the `Overridden by` field — are the
ones of [`rfc791/catalog.md`](../rfc791/catalog.md#how-to-read-an-entry). RFC 768 predates
RFC 2119 and uses one `must` and one `should`; everything else is description.

Two entries point into [`rfc1122/catalog.md`](../rfc1122/catalog.md). RFC 1122 §4.1 states
the host requirements for UDP with the keywords of RFC 2119. Where it raises the strength
of a statement of this document, the entry here carries `Overridden by`; where it only
says the same thing again, the entry says `Restated by`.

## Header

### RFC768-HDR-1

**The source port is optional; it is zero when unused.**

> "Source Port is an optional field, when meaningful, it indicates the port of the sending
> process, and may be assumed to be the port to which a reply should be addressed in the
> absence of any other information. If not used, a value of zero is inserted."
> — Fields, `rfc768.txt:48-51`

- Strength: description (the field is "optional"; the zero rule is plain statement).
  Class: wire.
- Check idea: a datagram from a program that expects replies carries a nonzero source
  port, and a reply to that port reaches the program.

### RFC768-HDR-2

**The destination port selects the receiver within the destination address.**

> "Destination Port has a meaning within the context of a particular internet destination
> address." — Fields, `rfc768.txt:66-67`

- Strength: description. Class: wire plus end-to-end.
- Check idea: two datagrams to two ports at one address reach two different programs, and
  a datagram to a port that no program opened reaches none.

### RFC768-HDR-3

**The length counts header and data in octets; the minimum is eight.**

> "Length is the length in octets of this user datagram including this header and the
> data. (This means the minimum value of the length is eight.)" — Fields,
> `rfc768.txt:69-71`

- Strength: description. Class: wire.
- Check idea: the length field equals 8 plus the size of the data; a datagram with no data
  carries the value 8.

## Checksum

### RFC768-CKSUM-1

**The checksum covers a pseudo header, the UDP header and the data.**

> "Checksum is the 16-bit one's complement of the one's complement sum of a pseudo header
> of information from the IP header, the UDP header, and the data, padded with zero octets
> at the end (if necessary) to make a multiple of two octets." — Fields,
> `rfc768.txt:73-76`
>
> "The pseudo header conceptually prefixed to the UDP header contains the source address,
> the destination address, the protocol, and the UDP length. This information gives
> protection against misrouted datagrams." — Fields, `rfc768.txt:78-80`

- Strength: description. Class: encoding.
- Overridden by [RFC1122-UCK-1](../rfc1122/catalog.md#rfc1122-uck-1): for a host, RFC 1122
  §4.1.3.4 makes the generation and the check of this value mandatory.
- Check idea: a check on the wire alone can establish only that a checksum is present. A
  check that can change a datagram in flight establishes the coverage: change one octet of
  the data, or one address of the pseudo header, keep the checksum field, and the receiver
  must reject the datagram.

### RFC768-CKSUM-2

**A computed zero is sent as all ones; an all-zero checksum means none was generated.**

> "If the computed checksum is zero, it is transmitted as all ones (the equivalent in one's
> complement arithmetic). An all zero transmitted checksum value means that the
> transmitter generated no checksum (for debugging or for higher level protocols that
> don't care)." — Fields, `rfc768.txt:92-95`

- Strength: description; the second sentence is a permission for the transmitter. Class:
  wire plus end-to-end.
- Restated by [RFC1122-UCK-6](../rfc1122/catalog.md#rfc1122-uck-6), an IMPLEMENTATION note
  of RFC 1122 §4.1.3.4 with the same strength. The receive half of the entry, that an
  all-zero checksum is not a failed checksum, is the subject of
  [RFC1122-UCK-5](../rfc1122/catalog.md#rfc1122-uck-5).
- Check idea: a sender that generates a checksum transmits a nonzero value; a sender that
  generates none transmits zero; the receiver accepts both.

## Interface

### RFC768-UI-1

**A receive operation returns the data, the source port and the source address.**

> "A user interface should allow the creation of new receive ports, receive operations on
> the receive ports that return the data octets and an indication of source port and
> source address, and an operation that allows a datagram to be sent, specifying the data,
> source and destination ports and addresses to be sent." — User Interface,
> `rfc768.txt:100-108`

- Strength: should. Class: end-to-end.
- Check idea: the program that opened the port receives the data; the source port and the
  source address that it learns are the ones the datagram carried.

### RFC768-IP-1

**The UDP module determines the addresses and the protocol from the IP header.**

> "The UDP module must be able to determine the source and destination internet addresses
> and the protocol field from the internet header." — IP Interface, `rfc768.txt:127-128`

- Strength: must. Class: internal. The requirement is on the interface between UDP and IP;
  its visible effects are the source address in RFC768-UI-1 and the pseudo header of
  RFC768-CKSUM-1.
- Check idea: a correct checksum over a pseudo header proves that the module had the
  addresses and the protocol; a unit test of the serializer establishes it directly.

## Protocol number

### RFC768-PROTO-1

**UDP is protocol 17 in IP.**

> "This is protocol 17 (21 octal) when used in the Internet Protocol." — Protocol Number,
> `rfc768.txt:144`

- Strength: description. Class: wire.
- Check idea: the IP header of every UDP datagram carries protocol 17.

## Out of scope in this catalog

RFC 768 is exhausted by the entries above, except for two things that no wire check can
reach: the checksum arithmetic of RFC768-CKSUM-1 and the interface rule of RFC768-IP-1,
which are unit-test material. The options that RFC 9868 adds beyond the length field are
that document's, not this one's, and level 5 work. The introduction's statement that
"delivery and duplicate protection are not guaranteed" (`rfc768.txt:23-24`) is a
non-requirement and has no entry.
