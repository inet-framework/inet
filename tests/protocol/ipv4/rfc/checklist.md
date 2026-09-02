# RFC 791 (IPv4) — catalog of checkable statements

This document is the step 2 artifact of the RFC test workflow (see
[`../../RFC-WORKFLOW.md`](../../RFC-WORKFLOW.md)). It lists statements from the RFC that a
test can check. The catalog comes from the RFC text only. It contains no simulation model
names and no code references — that mapping happens in later steps.

Sources, cached in this folder:

- `rfc791.txt` — Internet Protocol, September 1981. Downloaded 2026-09-02 from
  <https://www.rfc-editor.org/rfc/rfc791.txt>.
- `rfc792.txt` — Internet Control Message Protocol, September 1981. Downloaded 2026-09-02
  from <https://www.rfc-editor.org/rfc/rfc792.txt>. RFC 791 delegates error reports to ICMP,
  so the two documents pair for error-signal checks.

Quotes are verbatim. A reference such as `rfc791.txt:1012` points to a line of the cached
file in this folder.

## How to read an entry

- **Strength** — the word the RFC uses: `must`, `may`, or `description` (normative prose
  without a keyword). RFC 791 predates RFC 2119, so the words carry their plain meaning.
- **Class** — how a test can observe the statement:
  - `wire` — fields of datagrams on a link between two nodes.
  - `end-to-end` — what the destination accepts and delivers upward.
  - `error-signal` — an ICMP message that reports a failure.
  - `internal` — state inside a module; not visible from outside.
  - `encoding` — the exact bit layout of a field; a serializer concern.
- **Status** — `selected` (this iteration), `covered` (part of a selected check),
  `candidate` (practical, not yet selected), `later` (needs injection, faults, or several
  flows).

## Time to live

### R791-TTL-1 — each module that processes a datagram decreases TTL

> "This field must be decreased at each point that the internet header is processed to
> reflect the time spent processing the datagram. Even if no local information is available
> on the time actually spent, the field must be decremented by 1."
> — §3.2 Time to Live, `rfc791.txt:1981-1984`

- Strength: must. Class: wire.
- Check idea: send one datagram through a gateway. Compare the TTL on the link before the
  gateway with the TTL on the link after the gateway. The value must decrease by 1.
- Status: **selected** → [`checks/ttl-decrement.md`](checks/ttl-decrement.md)

### R791-TTL-2 — a datagram with TTL zero is destroyed

> "If this field contains the value zero, then the datagram must be destroyed."
> — §3.2 Time to Live, `rfc791.txt:1013-1014`

- Strength: must. Class: wire (absence after the gateway) plus error-signal (R792-TE-1).
- Check idea: send a datagram whose TTL is too small for the path. The destination must not
  receive it.
- Status: candidate

### R791-TTL-3 — the sender sets the TTL

> "The time to live is set by the sender to the maximum time the datagram is allowed to be
> in the internet system." — §3.2, `rfc791.txt:1976-1977`

- Strength: description. Class: wire.
- Check idea: the first observed hop shows the value that the sender selected, and the value
  is greater than zero.
- Status: **covered** by [`checks/ttl-decrement.md`](checks/ttl-decrement.md) (its first step)

## Fragmentation

### R791-FRAG-1 — an unfragmented datagram carries zero fragmentation information

> "The originating protocol module of a complete datagram sets the more-fragments flag to
> zero and the fragment offset to zero." — §2.3, `rfc791.txt:688-689`
> "an unfragmented datagram has all zero fragmentation information (MF = 0, fragment offset
> = 0)" — §3.2, `rfc791.txt:1656-1658`

- Strength: description. Class: wire.
- Check idea: a datagram that fits the first link must leave the source with MF = 0 and
  offset = 0.
- Status: **selected** → [`checks/fragment-reassembly.md`](checks/fragment-reassembly.md)

### R791-FRAG-2 — fragment data splits on 8-octet boundaries

> "If an internet datagram is fragmented, its data portion must be broken on 8 octet
> boundaries." — §3.2, `rfc791.txt:1658-1659`
> "The fragment offset is measured in units of 8 octets (64 bits). The first fragment has
> offset zero." — §3.1, `rfc791.txt:1007-1008`

- Strength: must. Class: wire.
- Check idea: with a known datagram size and a known MTU, the fragment sizes and offsets
  follow from arithmetic. Compare the observed offsets with the computed values.
- Status: **selected** → [`checks/fragment-reassembly.md`](checks/fragment-reassembly.md)

### R791-FRAG-3 — each fragment keeps the identification of the original datagram

> "creates two new internet datagrams and copies the contents of the internet header fields
> from the long datagram into both new internet headers" — §2.3, `rfc791.txt:692-694`
> "The identification field is used to distinguish the fragments of one datagram from those
> of another." — §2.3, `rfc791.txt:683-684`

- Strength: description. Class: wire.
- Check idea: all fragments of one datagram must show one identification value.
- Status: **selected** → [`checks/fragment-reassembly.md`](checks/fragment-reassembly.md)

### R791-FRAG-4 — the flag pattern across a fragment train

> "The more-fragments flag is set to one." (first new datagram) — §2.3, `rfc791.txt:712`
> "The first fragment will have the fragment offset zero, and the last fragment will have
> the more-fragments flag reset to zero." — §2.3, `rfc791.txt:729-731`

- Strength: description. Class: wire.
- Check idea: the first fragment shows offset 0 with MF = 1; the last fragment shows a
  non-zero offset with MF = 0.
- Status: **selected** → [`checks/fragment-reassembly.md`](checks/fragment-reassembly.md)

### R791-FRAG-5 — the don't fragment flag prohibits fragmentation

> "An internet datagram can be marked 'don't fragment.' Any internet datagram so marked is
> not to be internet fragmented under any circumstances. If internet datagram marked don't
> fragment cannot be delivered to its destination without fragmenting it, it is to be
> discarded instead." — §2.3, `rfc791.txt:662-666`

- Strength: must. Class: wire (no fragments appear; the destination receives nothing) plus
  error-signal (R792-DU-4).
- Check idea: send a datagram with DF = 1 that is larger than the MTU of the second link.
  No fragment of it may appear after the gateway, and the destination must not receive it.
- Status: **selected** → [`checks/dont-fragment.md`](checks/dont-fragment.md)

### R791-FRAG-6 — a 68-octet datagram passes every module without fragmentation

> "Every internet module must be able to forward a datagram of 68 octets without further
> fragmentation." — §3.2, `rfc791.txt:1669-1671`

- Strength: must. Class: wire.
- Status: candidate

## Reassembly

### R791-REASM-1 — the destination reassembles the original datagram

> "To assemble the fragments of an internet datagram, an internet protocol module (for
> example at a destination host) combines internet datagrams that all have the same value
> for the four fields: identification, source, destination, and protocol. The combination is
> done by placing the data portion of each fragment in the relative position indicated by
> the fragment offset in that fragment's internet header." — §2.3, `rfc791.txt:723-729`

- Strength: description. Class: end-to-end.
- Check idea: after fragmentation on the path, the destination must deliver the complete
  original data to the next protocol layer in one piece.
- Status: **selected** → [`checks/fragment-reassembly.md`](checks/fragment-reassembly.md)

### R791-REASM-2 — every destination accepts a 576-octet datagram

> "All hosts must be prepared to accept datagrams of up to 576 octets (whether they arrive
> whole or in fragments)." — §3.1, `rfc791.txt:961-963`

- Strength: must. Class: end-to-end.
- Status: candidate

### R791-REASM-3 — fragments of different datagrams do not mix

> "The receiver of the fragments uses the identification field to ensure that fragments of
> different datagrams are not mixed." — §2.3, `rfc791.txt:674-676`

- Strength: description. Class: end-to-end.
- Status: later (needs two interleaved fragment trains, or crafted fragments)

## Header checksum

### R791-CKSUM-1 — the checksum is recomputed at each hop

> "Since some header fields change (e.g., time to live), this is recomputed and verified at
> each point that the internet header is processed." — §3.1, `rfc791.txt:1031-1033`

- Strength: description. Class: wire plus encoding.
- Note: the field comparison before and after a gateway is a wire check. The check of the
  checksum algorithm itself is an encoding concern and points to a different test category.
- Status: candidate

### R791-CKSUM-2 — a datagram with a bad header checksum is discarded

> "If the header checksum fails, the internet datagram is discarded at once by the entity
> which detects the error." — §1.4, `rfc791.txt:365-366`

- Strength: description (with "is discarded" as plain fact). Class: wire (absence).
- Status: later (needs corruption of a datagram in flight)

## Identification

### R791-ID-1 — identification is unique per source, destination, and protocol

> "The originating protocol module of an internet datagram sets the identification field to
> a value that must be unique for that source-destination pair and protocol for the time the
> datagram will be active in the internet system." — §2.3, `rfc791.txt:684-687`

- Strength: must. Class: wire.
- Check idea: two datagrams of one flow, sent close together, must show two different
  identification values.
- Status: candidate

## Error signals (RFC 792 companions)

### R792-TE-1 — TTL zero at a gateway: discard, and possibly a Time Exceeded message

> "If the gateway processing a datagram finds the time to live field is zero it must discard
> the datagram. The gateway may also notify the source host via the time exceeded message."
> — Time Exceeded Message, `rfc792.txt:344-356`

- Strength: discard is must; the message is may. Class: error-signal.
- Pairs with R791-TTL-2.
- Status: candidate

### R792-DU-4 — DF drop: destination unreachable, code 4

> "Another case is when a datagram must be fragmented to be forwarded by a gateway yet the
> Don't Fragment flag is on. In this case the gateway must discard the datagram and may
> return a destination unreachable message." — Destination Unreachable Message,
> `rfc792.txt:261-264`; code list: "4 = fragmentation needed and DF set",
> `rfc792.txt:215`

- Strength: discard is must; the message is may. Class: error-signal.
- Pairs with R791-FRAG-5. A gateway that sends the message shows the expected cooperative
  behavior; the RFC permits silence.
- Status: **selected** → [`checks/dont-fragment.md`](checks/dont-fragment.md)

## Coverage ledger

The ledger connects catalog entries to check documents and test files. Later steps and later
iterations extend this table; the catalog above stays stable.

| ID | Strength | Class | Status | Check document | Test file |
| --- | --- | --- | --- | --- | --- |
| R791-TTL-1 | must | wire | selected | checks/ttl-decrement.md | Rfc791TtlDecrement.test |
| R791-TTL-2 | must | wire, error-signal | candidate | — | — |
| R791-TTL-3 | description | wire | covered | checks/ttl-decrement.md | Rfc791TtlDecrement.test |
| R791-FRAG-1 | description | wire | selected | checks/fragment-reassembly.md | Rfc791FragmentReassembly.test |
| R791-FRAG-2 | must | wire | selected | checks/fragment-reassembly.md | Rfc791FragmentReassembly.test |
| R791-FRAG-3 | description | wire | selected | checks/fragment-reassembly.md | Rfc791FragmentReassembly.test |
| R791-FRAG-4 | description | wire | selected | checks/fragment-reassembly.md | Rfc791FragmentReassembly.test |
| R791-FRAG-5 | must | wire, error-signal | selected | checks/dont-fragment.md | Rfc791DontFragment.test |
| R791-FRAG-6 | must | wire | candidate | — | — |
| R791-REASM-1 | description | end-to-end | selected | checks/fragment-reassembly.md | Rfc791FragmentReassembly.test |
| R791-REASM-2 | must | end-to-end | candidate | — | — |
| R791-REASM-3 | description | end-to-end | later | — | — |
| R791-CKSUM-1 | description | wire, encoding | candidate | — | — |
| R791-CKSUM-2 | description | wire | later | — | — |
| R791-ID-1 | must | wire | candidate | — | — |
| R792-TE-1 | must + may | error-signal | candidate | — | — |
| R792-DU-4 | must + may | error-signal | selected | checks/dont-fragment.md | Rfc791DontFragment.test |

Out of scope in this iteration: options (§3.1 Options), type of service and precedence,
security annex, and the reassembly timer details. Add them in a later pass.
