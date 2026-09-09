# RFC 792 (ICMP) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC792-*` · **Stands on:** [standards.md](../../protocol/ipv4/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow, for one document of
the in-scope set: RFC 792. RFC 791 delegates its error reports to ICMP, so every entry here
pairs with an entry of [`rfc791/catalog.md`](../rfc791/catalog.md). The catalog comes from
the RFC text only. It contains no simulation model names and no code references.

Source, cached in this folder:

- `rfc792.txt` — Internet Control Message Protocol, September 1981. Downloaded 2026-09-02
  from <https://www.rfc-editor.org/rfc/rfc792.txt>.

The scope of this catalog is narrow by intent: only the two messages that report the
failures of the RFC 791 checks. The other ICMP messages (echo, source quench, redirect,
timestamp, information request) are out of scope in this pass.

Quotes are verbatim. A reference such as `rfc792.txt:215` points to a line of the cached
file in this folder.

The state of the workflow — which statement a check targets, which test carries it, and
what the run said — is **not** in this document. It lives in the coverage ledger,
[`ipv4/coverage.md`](../../model/ipv4/coverage.md). Keeping it out is deliberate: this catalog states what the standard says, so a
new test or a new run must never force an edit here.

## Index

| ID | Statement |
| --- | --- |
| [RFC792-TE-1](#rfc792-te-1) | TTL zero at a gateway: discard, and possibly a Time Exceeded message. |
| [RFC792-DU-4](#rfc792-du-4) | DF drop: destination unreachable, code 4. |

The conventions of an entry — strength, class, and the `Overridden by` field — are
the ones of [`rfc791/catalog.md`](../rfc791/catalog.md#how-to-read-an-entry). No entry
here carries `Overridden by`: RFC 1122 governs hosts, and both messages are gateway
reports in the RFC 791 checks.

## Error signals for the RFC 791 checks

### RFC792-TE-1

**TTL zero at a gateway: the gateway discards the datagram, and it may send a Time Exceeded
message.**

> "If the gateway processing a datagram finds the time to live field is zero it must discard
> the datagram. The gateway may also notify the source host via the time exceeded message."
> — Time Exceeded Message, `rfc792.txt:344-356`

- Strength: discard is must; the message is may. Class: error-signal.
- Pairs with [RFC791-TTL-2](../rfc791/catalog.md#rfc791-ttl-2).
- Note on the strength: this is a gateway rule. For a host, RFC 1122 §3.3.8 turns the may
  into a must "wherever practical" ([RFC1122-ERR-1](../rfc1122/catalog.md#rfc1122-err-1));
  the gateway stays under this text until RFC 1812 enters the in-scope set. RFC 1122
  §3.2.2 also lists five cases in which the message must not be sent
  ([RFC1122-ICMP-5](../rfc1122/catalog.md#rfc1122-icmp-5) to
  [RFC1122-ICMP-9](../rfc1122/catalog.md#rfc1122-icmp-9)); they take precedence.

### RFC792-DU-4

**DF drop: the gateway discards the datagram, and it may send destination unreachable,
code 4.**

> "Another case is when a datagram must be fragmented to be forwarded by a gateway yet the
> Don't Fragment flag is on. In this case the gateway must discard the datagram and may
> return a destination unreachable message." — Destination Unreachable Message,
> `rfc792.txt:261-264`; code list: "4 = fragmentation needed and DF set",
> `rfc792.txt:215`

- Strength: discard is must; the message is may. Class: error-signal.
- Pairs with [RFC791-FRAG-5](../rfc791/catalog.md#rfc791-frag-5). A gateway that sends the
  message shows the expected cooperative behavior; the RFC permits silence. The note on
  the strength under [RFC792-TE-1](#rfc792-te-1) applies here too.
- Note for a later pass: RFC 1191 gives the word that RFC 792 labels "unused" in this
  message a meaning, the next-hop MTU. The row waits in
  [`standards.md`](../../protocol/ipv4/standards.md#override-table); the run of pass 1 already saw the model
  send that value.

## Out of scope in this catalog

Echo and echo reply, source quench (RFC 6633 deprecates it), redirect, parameter problem,
timestamp, and information request. The other destination unreachable codes (0 to 3, and 5)
are also out of scope.

The scope of this catalog is narrow by intent: only the two messages that report the
failures of the RFC 791 checks.
