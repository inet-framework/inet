# RFC 792 (ICMP) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC792-*` · **Stands on:** [standards.md](standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow, for one document of
the in-scope set: RFC 792. RFC 791 delegates its error reports to ICMP, so every entry here
pairs with an entry of [`rfc791-checklist.md`](rfc791-checklist.md). The catalog comes from
the RFC text only. It contains no simulation model names and no code references.

Source, cached in this folder:

- `rfc792.txt` — Internet Control Message Protocol, September 1981. Downloaded 2026-09-02
  from <https://www.rfc-editor.org/rfc/rfc792.txt>.

The scope of this catalog is narrow by intent: only the two messages that report the
failures of the RFC 791 checks. The other ICMP messages (echo, source quench, redirect,
timestamp, information request) are out of scope in this pass.

Quotes are verbatim. A reference such as `rfc792.txt:215` points to a line of the cached
file in this folder.

## Index

| ID | Statement |
| --- | --- |
| [RFC792-TE-1](#rfc792-te-1) | TTL zero at a gateway: discard, and possibly a Time Exceeded message. |
| [RFC792-DU-4](#rfc792-du-4) | DF drop: destination unreachable, code 4. |

The conventions of an entry — strength, class, status, and the `Overridden by` field — are
the ones of [`rfc791-checklist.md`](rfc791-checklist.md#how-to-read-an-entry).

## Error signals for the RFC 791 checks

### RFC792-TE-1

**TTL zero at a gateway: the gateway discards the datagram, and it may send a Time Exceeded
message.**

> "If the gateway processing a datagram finds the time to live field is zero it must discard
> the datagram. The gateway may also notify the source host via the time exceeded message."
> — Time Exceeded Message, `rfc792.txt:344-356`

- Strength: discard is must; the message is may. Class: error-signal.
- Pairs with [RFC791-TTL-2](rfc791-checklist.md#rfc791-ttl-2).
- Status: candidate

### RFC792-DU-4

**DF drop: the gateway discards the datagram, and it may send destination unreachable,
code 4.**

> "Another case is when a datagram must be fragmented to be forwarded by a gateway yet the
> Don't Fragment flag is on. In this case the gateway must discard the datagram and may
> return a destination unreachable message." — Destination Unreachable Message,
> `rfc792.txt:261-264`; code list: "4 = fragmentation needed and DF set",
> `rfc792.txt:215`

- Strength: discard is must; the message is may. Class: error-signal.
- Pairs with [RFC791-FRAG-5](rfc791-checklist.md#rfc791-frag-5). A gateway that sends the
  message shows the expected cooperative behavior; the RFC permits silence.
- Status: **selected** → [rfc791-checks.md#dont-fragment](rfc791-checks.md#dont-fragment)
- Note for a later pass: RFC 1191 gives the word that RFC 792 labels "unused" in this
  message a meaning, the next-hop MTU. The row waits in
  [`standards.md`](standards.md#override-table); the run of pass 1 already saw the model
  send that value.

## Coverage ledger

| ID | Strength | Class | Status | Check section | Test file |
| --- | --- | --- | --- | --- | --- |
| RFC792-TE-1 | must + may | error-signal | candidate | — | — |
| RFC792-DU-4 | must + may | error-signal | selected | [dont-fragment](rfc791-checks.md#dont-fragment) | Rfc791DontFragment.test |

The check sections and the test files stay under the name of the primary document,
`rfc791-checks.md` and `Rfc791DontFragment.test`, because one scenario shows the RFC 791
behavior and the RFC 792 report together. The catalogs split per document; the checks do
not have to.

Out of scope in this pass: echo and echo reply, source quench (RFC 6633 deprecates it),
redirect, parameter problem, timestamp, and information request. The other destination
unreachable codes (0 to 3, 5) are also out of scope.

Both entries of this catalog appear in [`features.md`](features.md), as step 4 requires.
