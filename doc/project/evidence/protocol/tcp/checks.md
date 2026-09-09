# TCP — English check procedures

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [catalog.md](../../standard/rfc9293/catalog.md), [features.md](features.md)

Step 5 artifact of the standards test workflow. This file holds the common mockups, the
scenario constants, the rules every check obeys, and the index. The checks themselves live
in one file per feature under [`checks/`](checks); the section names are the anchors that
the coverage ledger links to. The procedures come from the specification only.
They name no simulation model and no code. The catalog entries are in
[`rfc9293/catalog.md`](../../standard/rfc9293/catalog.md).

The governing document is RFC 9293, not RFC 793. The reason is in
[`standards.md`](standards.md#override-table).

## Index

| Check | File | Statements |
| --- | --- | --- |
| [Connection establishment](checks/establishment.md#connection-establishment) | `checks/establishment.md` | RFC9293-EST-1, EST-2, ISS-1, SEQ-1, ACK-1, OPT-1, HDR-1, CKSUM-1 |
| [Data transfer](checks/data-transfer.md#data-transfer) | `checks/data-transfer.md` | RFC9293-DATA-1, SEG-1, ACK-2, SEQ-2 |
| [Push on the last segment](checks/data-transfer.md#push-on-the-last-segment) | `checks/data-transfer.md` | RFC9293-PSH-1 |
| [Connection termination](checks/termination.md#connection-termination) | `checks/termination.md` | RFC9293-FIN-1, FIN-2 |
| [Flow control](checks/flow-control.md#flow-control) | `checks/flow-control.md` | RFC9293-WND-1, WND-2 |
| [Connection reset](checks/reset.md#connection-reset) | `checks/reset.md` | RFC9293-RST-1 |
| [Blind reset](checks/reset.md#blind-reset) | `checks/reset.md` | RFC9293-RSTP-1 |
| [Valid reset](checks/reset.md#valid-reset) | `checks/reset.md` | RFC9293-RSTP-2; covers RST-1 |
| [No reset for a reset](checks/reset.md#no-reset-for-a-reset) | `checks/reset.md` | RFC9293-RST-3 |
| [Checksum discard](checks/checksum.md#checksum-discard) | `checks/checksum.md` | RFC9293-CKSUM-2 |
| [Checksum by default](checks/checksum.md#checksum-by-default) | `checks/checksum.md` | RFC9293-CKSUM-1 (the value) |
| [Out-of-window segment](checks/segment-acceptance.md#out-of-window-segment) | `checks/segment-acceptance.md` | RFC9293-SEGA-1, SEGA-2, RST-2 |
| [Shrunk window](checks/window.md#shrunk-window) | `checks/window.md` | RFC9293-WND-4 |
| [No new data past a shrunk edge](checks/window.md#no-new-data-past-a-shrunk-edge) | `checks/window.md` | RFC9293-WND-5 |
| [No window shrink](checks/window.md#no-window-shrink) | `checks/window.md` | RFC9293-WND-3 |
| [Soft ICMP error](checks/icmp.md#soft-icmp-error) | `checks/icmp.md` | RFC9293-ICMP-3; covers ICMP-1 |
| [Source Quench](checks/icmp.md#source-quench) | `checks/icmp.md` | RFC9293-ICMP-2 |

Seventeen checks: six from the level 2 pass, eleven added at level 3.

## Statements that no check carries

Two statements of the in-scope set have no check of their own.

| Statement | What it demands | Why no check |
| --- | --- | --- |
| RFC9293-ICMP-1 | the report reaches the connection that caused it | no check of its own; it is covered by Soft ICMP error, which can only pass if the report reached the right connection |
| RFC9293-ICMP-4 | a hard error should abort the connection | the document itself notes that many implementations do not abort in a synchronized state, so the outcome is a `declined` either way; the check that would settle it needs a hard error, which the same relay could produce, and a decision about which behaviour the model intends |

## Common mockups

Three mockups serve the seventeen checks. The first is the one the level 2 pass used. The
other two are what the level 3 checks need, because a segment can only be changed while it
travels, and a report from a gateway needs a gateway.

**The plain mockup.** Two hosts on one link. Host A opens a connection to host B, sends a
block of data, and closes. Host B accepts, receives what arrives, and closes when host A
closes.

```
   host A  ------------  host B
           one link L
    (client)              (server)
```

**The mockup with a relay.** A relay T sits on the link and holds each frame for a moment.

```
   host A  ------  relay T  ------  host B
    (client)                        (server)
```

- The relay changes one segment and passes every other frame on without a change. It is not
  a node of the protocol: it has no address, it answers nothing, and neither host can see
  it.
- A relay holds one rule at a time. A check that needs two changes uses two relays in
  series, and both see both directions.
- What the relay does is what a fault or a third party on the path would do. No program can
  produce it, and that is why these checks are level 3.

**The mockup with a relay and a gateway.** A gateway R stands behind the relay, so that a
real node produces the ICMP report the check needs.

```
   host A  ------  relay T  ------  gateway R  ------  host B
```

- The relay sits before the gateway, so that it can change a segment on its way out and see
  the report the gateway sends back.
- No report in these checks is forged. The relay makes a condition the gateway must report,
  and the gateway writes the message.

Scenario constants, shared unless a check says otherwise:

| Constant | Value | Why |
| --- | --- | --- |
| Open time | 0.1 s | after the link and the address resolution settle |
| Send time | 0.2 s | clearly after the handshake, so a data segment cannot be confused with a handshake segment |
| Close time | 0.4 s | clearly after the data exchange |
| Maximum segment size | 536 octets | the default send MSS over IPv4 (RFC 9293 §3.7.1, MUST-15); no check sends a larger MSS option |
| Checksums | computed by every node | the TCP checksum is never optional (MUST-2); a model that offers a mode that assumes checksums correct runs with that mode off |
| Observation limit | 2 s at level 2, 10 s where a loss must be recovered | a check that takes a segment away waits for the retransmission timeout, which starts at three seconds |
| The receiving program | echoes nothing at level 3 | the reverse direction then carries acknowledgments only, so an observation about a segment from host B cannot be confused with an echo |

No check assumes a value for any sequence number. Each side picks its own initial sequence
number (RFC9293-ISS-1), so every expectation is **relative**: a captured value plus a
computed offset.

A check that changes a segment must leave the checksum right for what it wrote. A relay that
changes a header computes the checksum again, as a deliberate rewriter on the path would;
otherwise the receiver would object to the checksum and never reach the rule under test.
