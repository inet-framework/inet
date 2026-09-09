# RFC 791 (IPv4) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC791-*` · **Stands on:** [standards.md](../../protocol/ipv4/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow, for one document of
the in-scope set: RFC 791. It lists statements of that document that a test can check. The
catalog comes from the RFC text only. It contains no simulation model names and no code
references — that mapping happens in later steps.

Source, cached in this folder:

- `rfc791.txt` — Internet Protocol, September 1981. Downloaded 2026-09-02 from
  <https://www.rfc-editor.org/rfc/rfc791.txt>.

RFC 791 delegates error reports to ICMP. Those statements belong to RFC 792 and live in
[`rfc792/catalog.md`](../rfc792/catalog.md). The documents of the in-scope set, their relatives, and the
override rows are in [`standards.md`](../../protocol/ipv4/standards.md). A feature that spans both documents is
in [`features.md`](../../protocol/ipv4/features.md).

Quotes are verbatim. A reference such as `rfc791.txt:1012` points to a line of the cached
file in this folder.

The state of the workflow — which statement a check targets, which test carries it, and
what the run said — is **not** in this document. It lives in the coverage ledger,
[`ipv4/coverage.md`](../../model/ipv4/coverage.md). Keeping it out is deliberate: this catalog states what the standard says, so a
new test or a new run must never force an edit here.

## Index

| ID | Statement |
| --- | --- |
| [RFC791-TTL-1](#rfc791-ttl-1) | Each module that processes a datagram decreases the TTL. |
| [RFC791-TTL-2](#rfc791-ttl-2) | A datagram with TTL zero is destroyed. |
| [RFC791-TTL-3](#rfc791-ttl-3) | The sender sets the TTL. |
| [RFC791-FRAG-1](#rfc791-frag-1) | An unfragmented datagram carries zero fragmentation information. |
| [RFC791-FRAG-2](#rfc791-frag-2) | Fragment data splits on 8-octet boundaries. |
| [RFC791-FRAG-3](#rfc791-frag-3) | Each fragment keeps the identification of the original datagram. |
| [RFC791-FRAG-4](#rfc791-frag-4) | The first fragment has offset 0 with MF = 1; the last has MF = 0. |
| [RFC791-FRAG-5](#rfc791-frag-5) | The don't fragment flag prohibits fragmentation. |
| [RFC791-FRAG-6](#rfc791-frag-6) | A 68-octet datagram passes every module without fragmentation. |
| [RFC791-REASM-1](#rfc791-reasm-1) | The destination reassembles the original datagram. |
| [RFC791-REASM-2](#rfc791-reasm-2) | Every destination accepts a 576-octet datagram. |
| [RFC791-REASM-3](#rfc791-reasm-3) | Fragments of different datagrams do not mix. |
| [RFC791-CKSUM-1](#rfc791-cksum-1) | The header checksum is recomputed at each hop. |
| [RFC791-CKSUM-2](#rfc791-cksum-2) | A datagram with a bad header checksum is discarded. |
| [RFC791-ID-1](#rfc791-id-1) | Identification is unique per source, destination, and protocol. |
| [RFC791-HDR-1](#rfc791-hdr-1) | The version field is 4. |
| [RFC791-HDR-2](#rfc791-hdr-2) | The header length counts 32-bit words and is at least 5. |
| [RFC791-HDR-3](#rfc791-hdr-3) | The total length counts header and data together, in octets. |
| [RFC791-PROTO-1](#rfc791-proto-1) | The protocol field names the next level protocol of the data. |
| [RFC791-FWD-1](#rfc791-fwd-1) | A gateway forwards a datagram addressed to another network. |
| [RFC791-DLV-1](#rfc791-dlv-1) | The destination host passes the data to the addressed program. |

## How to read an entry

- **Strength** — the word the RFC uses: `must`, `may`, or `description` (normative prose
  without a keyword). RFC 791 predates RFC 2119, so the words carry their plain meaning.
- **Class** — how a test can observe the statement:
  - `wire` — fields of datagrams on a link between two nodes.
  - `end-to-end` — what the destination accepts and delivers upward.
  - `error-signal` — an ICMP message that reports a failure.
  - `internal` — state inside a module; not visible from outside.
  - `encoding` — the exact bit layout of a field; a serializer concern.
- **Overridden by** — appears only when a later document of the in-scope set changes the
  statement. Five entries carry it since RFC 1122 and RFC 6864 entered the in-scope set:
  the identification rule, the don't-fragment rule, the checksum discard, and the two
  reassembly statements. The entry and its ID stay; a test targets the entry that governs.
  The rows are in the override table of
  [`standards.md`](../../protocol/ipv4/standards.md#override-table).

## Time to live

### RFC791-TTL-1

**Each module that processes a datagram decreases the TTL.**

> "This field must be decreased at each point that the internet header is processed to
> reflect the time spent processing the datagram. Even if no local information is available
> on the time actually spent, the field must be decremented by 1."
> — §3.2 Time to Live, `rfc791.txt:1981-1984`

- Strength: must. Class: wire.
- Check idea: send one datagram through a gateway. Compare the TTL on the link before the
  gateway with the TTL on the link after the gateway. The value must decrease by 1.

### RFC791-TTL-2

**A datagram with TTL zero is destroyed.**

> "If this field contains the value zero, then the datagram must be destroyed."
> — §3.2 Time to Live, `rfc791.txt:1013-1014`

- Strength: must. Class: wire (absence after the gateway) plus error-signal
  ([RFC792-TE-1](../rfc792/catalog.md#rfc792-te-1)).
- Check idea: send a datagram whose TTL is too small for the path. The destination must not
  receive it.

### RFC791-TTL-3

**The sender sets the TTL.**

> "The time to live is set by the sender to the maximum time the datagram is allowed to be
> in the internet system." — §3.2, `rfc791.txt:1976-1977`

- Strength: description. Class: wire.
- Check idea: the first observed hop shows the value that the sender selected, and the value
  is greater than zero.

## Fragmentation

### RFC791-FRAG-1

**An unfragmented datagram carries zero fragmentation information.**

> "The originating protocol module of a complete datagram sets the more-fragments flag to
> zero and the fragment offset to zero." — §2.3, `rfc791.txt:688-689`
> "an unfragmented datagram has all zero fragmentation information (MF = 0, fragment offset
> = 0)" — §3.2, `rfc791.txt:1656-1658`

- Strength: description. Class: wire.
- Check idea: a datagram that fits the first link must leave the source with MF = 0 and
  offset = 0.

### RFC791-FRAG-2

**Fragment data splits on 8-octet boundaries.**

> "If an internet datagram is fragmented, its data portion must be broken on 8 octet
> boundaries." — §3.2, `rfc791.txt:1658-1659`
> "The fragment offset is measured in units of 8 octets (64 bits). The first fragment has
> offset zero." — §3.1, `rfc791.txt:1007-1008`

- Strength: must. Class: wire.
- Check idea: with a known datagram size and a known MTU, the fragment sizes and offsets
  follow from arithmetic. Compare the observed offsets with the computed values.

### RFC791-FRAG-3

**Each fragment keeps the identification of the original datagram.**

> "creates two new internet datagrams and copies the contents of the internet header fields
> from the long datagram into both new internet headers" — §2.3, `rfc791.txt:692-694`
> "The identification field is used to distinguish the fragments of one datagram from those
> of another." — §2.3, `rfc791.txt:683-684`

- Strength: description. Class: wire.
- Check idea: all fragments of one datagram must show one identification value.

### RFC791-FRAG-4

**The first fragment has offset 0 with MF = 1; the last fragment has MF = 0.**

> "The more-fragments flag is set to one." (first new datagram) — §2.3, `rfc791.txt:712`
> "The first fragment will have the fragment offset zero, and the last fragment will have
> the more-fragments flag reset to zero." — §2.3, `rfc791.txt:729-731`

- Strength: description. Class: wire.
- Check idea: the first fragment shows offset 0 with MF = 1; the last fragment shows a
  non-zero offset with MF = 0.

### RFC791-FRAG-5

**The don't fragment flag prohibits fragmentation.**

> "An internet datagram can be marked 'don't fragment.' Any internet datagram so marked is
> not to be internet fragmented under any circumstances. If internet datagram marked don't
> fragment cannot be delivered to its destination without fragmenting it, it is to be
> discarded instead." — §2.3, `rfc791.txt:662-666`

- Strength: must. Class: wire (no fragments appear; the destination receives nothing) plus
  error-signal ([RFC792-DU-4](../rfc792/catalog.md#rfc792-du-4)).
- Check idea: send a datagram with DF = 1 that is larger than the MTU of the second link.
  No fragment of it may appear after the gateway, and the destination must not receive it.
- Overridden by: [RFC6864-ID-6](../rfc6864/catalog.md#rfc6864-id-6), the same rule with a
  keyword; [RFC6864-ID-7](../rfc6864/catalog.md#rfc6864-id-7) adds that a transit device
  does not clear the bit.

### RFC791-FRAG-6

**A 68-octet datagram passes every module without fragmentation.**

> "Every internet module must be able to forward a datagram of 68 octets without further
> fragmentation." — §3.2, `rfc791.txt:1669-1671`

- Strength: must. Class: wire.

## Reassembly

### RFC791-REASM-1

**The destination reassembles the original datagram.**

> "To assemble the fragments of an internet datagram, an internet protocol module (for
> example at a destination host) combines internet datagrams that all have the same value
> for the four fields: identification, source, destination, and protocol. The combination is
> done by placing the data portion of each fragment in the relative position indicated by
> the fragment offset in that fragment's internet header." — §2.3, `rfc791.txt:723-729`

- Strength: description. Class: end-to-end.
- Check idea: after fragmentation on the path, the destination must deliver the complete
  original data to the next protocol layer in one piece.
- Overridden by: [RFC1122-REASM-1](../rfc1122/catalog.md#rfc1122-reasm-1), which turns
  the procedure into an obligation of every host.

### RFC791-REASM-2

**Every destination accepts a 576-octet datagram.**

> "All hosts must be prepared to accept datagrams of up to 576 octets (whether they arrive
> whole or in fragments)." — §3.1, `rfc791.txt:961-963`

- Strength: must. Class: end-to-end.
- Overridden by: [RFC1122-REASM-2](../rfc1122/catalog.md#rfc1122-reasm-2), which names the
  quantity (EMTU_R) and adds two should clauses.

### RFC791-REASM-3

**Fragments of different datagrams do not mix.**

> "The receiver of the fragments uses the identification field to ensure that fragments of
> different datagrams are not mixed." — §2.3, `rfc791.txt:674-676`

- Strength: description. Class: end-to-end.

## Header checksum

### RFC791-CKSUM-1

**The header checksum is recomputed at each hop.**

> "Since some header fields change (e.g., time to live), this is recomputed and verified at
> each point that the internet header is processed." — §3.1, `rfc791.txt:1031-1033`

- Strength: description. Class: wire plus encoding.
- Note: the field comparison before and after a gateway is a wire check. The check of the
  checksum algorithm itself is an encoding concern and points to a different test category.

### RFC791-CKSUM-2

**A datagram with a bad header checksum is discarded.**

> "If the header checksum fails, the internet datagram is discarded at once by the entity
> which detects the error." — §1.4, `rfc791.txt:365-366`

- Strength: description (with "is discarded" as plain fact). Class: wire (absence).
- Overridden by: [RFC1122-CKSUM-1](../rfc1122/catalog.md#rfc1122-cksum-1), which states
  the verification and the silent discard as a must for every received datagram.

## Identification

### RFC791-ID-1

**Identification is unique per source, destination, and protocol while the datagram is
active.**

> "The originating protocol module of an internet datagram sets the identification field to
> a value that must be unique for that source-destination pair and protocol for the time the
> datagram will be active in the internet system." — §2.3, `rfc791.txt:684-687`

- Strength: must. Class: wire.
- Check idea: two datagrams of one flow, sent close together, must show two different
  identification values.
- Overridden by: [RFC6864-ID-5](../rfc6864/catalog.md#rfc6864-id-5). The rule now applies
  to non-atomic datagrams only, within one maximum datagram lifetime;
  [RFC6864-ID-3](../rfc6864/catalog.md#rfc6864-id-3) tells every device to ignore the
  field of an atomic datagram.

## Header format

### RFC791-HDR-1

**The version field is 4.**

> "The Version field indicates the format of the internet header. This document describes
> version 4." — §3.1, `rfc791.txt:860-861`

- Strength: description. Class: wire.
- Check idea: every datagram on every link carries version 4.

### RFC791-HDR-2

**The header length counts 32-bit words and is at least 5.**

> "Internet Header Length is the length of the internet header in 32 bit words, and thus
> points to the beginning of the data. Note that the minimum value for a correct header is
> 5." — §3.1, `rfc791.txt:865-867`

- Strength: description. Class: wire.
- Check idea: a datagram without options carries a header of exactly 20 octets, and no
  datagram carries less.

### RFC791-HDR-3

**The total length counts the header and the data together, in octets.**

> "Total Length is the length of the datagram, measured in octets, including internet
> header and data." — §3.1, `rfc791.txt:958-959`

- Strength: description. Class: wire.
- Check idea: for a datagram with a known data size and no options, the total length equals
  20 plus the size of the data.

### RFC791-PROTO-1

**The protocol field names the next level protocol of the data.**

> "This field indicates the next level protocol used in the data portion of the internet
> datagram." — §3.1, `rfc791.txt:1025-1026`

- Strength: description. Class: wire plus end-to-end.
- Check idea: a datagram that carries UDP shows the protocol number of UDP on the wire, and
  the destination hands the data to UDP and to nothing else.

## Delivery and forwarding

### RFC791-FWD-1

**A gateway forwards a datagram addressed to a host in another network.**

> "The internet module determines from the internet address that the datagram is to be
> forwarded to another host in a second network. The internet module determines a local
> net address for the destination host. It calls on the local network interface for that
> network to send the datagram." — §2.2, `rfc791.txt:544-549`
>
> "Gateways implement internet protocol to forward datagrams between networks." — §2.4,
> `rfc791.txt:735-736`

- Strength: description. Class: wire.
- Check idea: a datagram sent from host A to host B across gateway R appears on link 2 with
  the same source and destination addresses it had on link 1.

### RFC791-DLV-1

**The destination host passes the data to the addressed program, with the source address.**

> "The internet module determines that the datagram is for an application program in this
> host. It passes the data to the application program in response to a system call,
> passing the source address and other parameters as results of the call." — §2.2,
> `rfc791.txt:558-561`

- Strength: description. Class: end-to-end.
- Check idea: the program on host B receives exactly the data that the program on host A
  sent, and learns host A's address as the source.

## Out of scope in this catalog

Options (§3.1 Options), type of service and precedence, the security annex, and the
reassembly timer details. Each becomes a catalog entry in a later pass. The type of service
area needs RFC 2474 in the in-scope set first; see
[`standards.md`](../../protocol/ipv4/standards.md#override-table).

The error reports that RFC 791 delegates to ICMP are in
[`rfc792/catalog.md`](../rfc792/catalog.md).
