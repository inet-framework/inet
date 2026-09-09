# RFC 792 (ICMP) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC792-*` · **Stands on:** [standards.md](../../protocol/ipv4/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow, for one document of
the in-scope set: RFC 792. RFC 791 delegates its error reports to ICMP, and so does every
protocol above IP, so this catalog serves more than one protocol: the entries for IPv4 pair
with [`rfc791/catalog.md`](../rfc791/catalog.md), and the port unreachable entry serves
UDP, [`rfc768/catalog.md`](../rfc768/catalog.md). The catalog comes from
the RFC text only. It contains no simulation model names and no code references.

Source, cached in this folder:

- `rfc792.txt` — Internet Control Message Protocol, September 1981. Downloaded 2026-09-02
  from <https://www.rfc-editor.org/rfc/rfc792.txt>.

The scope of this catalog is narrow by intent: the two messages that report the failures of
the RFC 791 checks, and the one that answers a UDP datagram to a closed port. The other
ICMP messages (echo, source quench, redirect, timestamp, information request) are out of
scope in this pass.

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
| [RFC792-DU-3](#rfc792-du-3) | Closed port: destination unreachable, code 3. |

The conventions of an entry — strength, class, and the `Overridden by` field — are
the ones of [`rfc791/catalog.md`](../rfc791/catalog.md#how-to-read-an-entry). One entry of
the three carries `Overridden by`. The two messages of the RFC 791 checks are gateway
reports, and RFC 1122 governs hosts, so the gateway stays under this text until RFC 1812
enters an in-scope set. The closed-port message is a host report, and RFC 1122 raises its
strength to a should; both protocols that share this catalog hold RFC 1122 in their in-scope
set, IPv4 from level 3 and UDP from level 3, so the entry carries the override. The note
under the entry says which section governs which protocol, because a check written for the
UDP set at level 2 still reads the entry as a may.

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

### RFC792-DU-3

**Closed port: the destination host discards the datagram, and it may send destination
unreachable, code 3.**

> "If, in the destination host, the IP module cannot deliver the datagram because the
> indicated protocol module or process port is not active, the destination host may send a
> destination unreachable message to the source host." — Destination Unreachable Message,
> `rfc792.txt:256-259`; code list: "3 = port unreachable", `rfc792.txt:213`

- Strength: the message is may; the non-delivery is a consequence of RFC 768, not a
  statement of this document. Class: error-signal.
- Pairs with [RFC768-HDR-2](../rfc768/catalog.md#rfc768-hdr-2): a port that no program
  opened selects no receiver. A host that sends the message shows the cooperative behavior;
  the RFC permits silence.
- Overridden by [RFC1122-UPORT-1](../rfc1122/catalog.md#rfc1122-uport-1): RFC 1122 §4.1.3.1
  states the same report as a **should**, for UDP itself.
- Note on the strength: this is a host report, unlike the two above. Three entries of
  RFC 1122 raise the may to a should, from two directions. §4.1.3.1 says that UDP should
  send the message (RFC1122-UPORT-1); §3.2.2.1 says the same of the IP layer
  ([RFC1122-DU-1](../rfc1122/catalog.md#rfc1122-du-1)); and §3.3.8 asks a host to report
  wherever practical ([RFC1122-ERR-1](../rfc1122/catalog.md#rfc1122-err-1)). The five
  prohibitions of §3.2.2 take precedence over all three. RFC 1122 is in the IPv4 in-scope
  set and, from level 3 on, in the UDP one; a UDP check at level 2 reads this entry as a
  may, and a UDP check at level 3 reads it as a should.

## Out of scope in this catalog

Echo and echo reply, source quench (RFC 6633 deprecates it), redirect, parameter problem,
timestamp, and information request. The destination unreachable codes other than 3 and 4
— that is, 0, 1, 2 and 5 — are also out of scope.

The scope of this catalog is narrow by intent: the two messages that report the failures
of the RFC 791 checks, and the one that answers a UDP datagram to a closed port.
