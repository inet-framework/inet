# RFC 9293 (TCP) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC9293-*` · **Stands on:** [standards.md](../../protocol/tcp/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow, for one document of
the in-scope set: RFC 9293, the August 2022 text that obsoletes RFC 793. It lists
statements that a test can check. The catalog comes from the RFC text only. It contains no
simulation model names and no code references.

Source, cached in this folder:

- `rfc9293.txt` — Transmission Control Protocol (TCP), August 2022. Downloaded 2026-09-04
  from <https://www.rfc-editor.org/rfc/rfc9293.txt>.

The family of TCP documents, and the reason this catalog quotes RFC 9293 and not RFC 793,
are in [`standards.md`](../../protocol/tcp/standards.md). The features that these statements
build are in [`features.md`](../../protocol/tcp/features.md).

Quotes are verbatim. A reference such as `rfc9293.txt:337` points to a line of the cached
file in this folder.

The state of the workflow — which statement a check targets, which test carries it, and
what the run said — is **not** in this document. It lives in the coverage ledger,
[`tcp/coverage.md`](../../model/tcp/coverage.md). Keeping it out is deliberate: this catalog states what the standard says, so a
new test or a new run must never force an edit here.

## Index

| ID | Statement |
| --- | --- |
| [RFC9293-EST-1](#rfc9293-est-1) | A connection is established with the three-way handshake. |
| [RFC9293-EST-2](#rfc9293-est-2) | The SYN-ACK acknowledges the initial sequence number plus one. |
| [RFC9293-SEQ-1](#rfc9293-seq-1) | The SYN occupies one sequence number, before any data. |
| [RFC9293-SEQ-2](#rfc9293-seq-2) | A pure ACK occupies no sequence number space. |
| [RFC9293-ACK-1](#rfc9293-ack-1) | The acknowledgment field holds the next sequence number expected. |
| [RFC9293-ACK-2](#rfc9293-ack-2) | Once established, every segment carries an acknowledgment. |
| [RFC9293-FIN-1](#rfc9293-fin-1) | The FIN occupies one sequence number, after the last data octet. |
| [RFC9293-FIN-2](#rfc9293-fin-2) | A normal close acknowledges each FIN, one direction at a time. |
| [RFC9293-DATA-1](#rfc9293-data-1) | TCP provides a reliable, in-order byte-stream service. |
| [RFC9293-SEG-1](#rfc9293-seg-1) | A segment is no larger than the effective send MSS. |
| [RFC9293-OPT-1](#rfc9293-opt-1) | Endpoints implement the MSS option, and should send it in every SYN. |
| [RFC9293-PSH-1](#rfc9293-psh-1) | Without a PUSH flag on SEND, the sender sets PSH on the last buffered segment. |
| [RFC9293-WND-1](#rfc9293-wnd-1) | The window field is the number of octets the receiver is prepared to accept. |
| [RFC9293-WND-2](#rfc9293-wnd-2) | The sender packages data into segments that fit the current window. |
| [RFC9293-ZWP-1](#rfc9293-zwp-1) | A sender with a zero window probes it regularly. |
| [RFC9293-ACKD-1](#rfc9293-ackd-1) | An acknowledgment may be delayed, but by less than 0.5 seconds. |
| [RFC9293-CKSUM-1](#rfc9293-cksum-1) | The sender must generate the checksum. |
| [RFC9293-CKSUM-2](#rfc9293-cksum-2) | The receiver must check the checksum. |
| [RFC9293-HDR-1](#rfc9293-hdr-1) | The data offset counts the header in 32-bit words. |
| [RFC9293-RST-1](#rfc9293-rst-1) | A segment for a connection that does not exist is answered with a reset. |
| [RFC9293-ISS-1](#rfc9293-iss-1) | Each side chooses its own initial sequence number. |
| [RFC9293-ISS-2](#rfc9293-iss-2) | The initial sequence number is clock-driven, and should add a pseudorandom function. |

## How to read an entry

- **Strength** — the word the RFC uses: `must`, `should`, `may`, or `description` for
  normative prose without a keyword. RFC 9293 uses RFC 2119 keywords, and it also carries
  much descriptive prose and figures that a test can check.
- **Class** — `wire`, `end-to-end`, `error-signal`, `internal`, or `encoding`, as in
  [`rfc791-catalog`](../rfc791/catalog.md#how-to-read-an-entry).
- **Overridden by** — appears only when a later document of the in-scope set changes the
  statement. No entry carries the field, because RFC 9293 is alone in the in-scope set. It
  is itself the override of RFC 793; see the override table of
  [`standards.md`](../../protocol/tcp/standards.md#override-table).

## Connection establishment

### RFC9293-EST-1

**A connection is established with the three-way handshake.**

> "The "three-way handshake" is the procedure used to establish a connection. This
> procedure normally is initiated by one TCP peer and responded to by another TCP peer."
> — §3.5, `rfc9293.txt:1223-1225`
>
> ```
> 2.  SYN-SENT    --> <SEQ=100><CTL=SYN>               --> SYN-RECEIVED
> 3.  ESTABLISHED <-- <SEQ=300><ACK=101><CTL=SYN,ACK>  <-- SYN-RECEIVED
> 4.  ESTABLISHED --> <SEQ=101><ACK=301><CTL=ACK>      --> ESTABLISHED
> ```
> — Figure 6, `rfc9293.txt:1260-1268`

- Strength: description with a normative figure. Class: wire.
- Check idea: open one connection and observe three segments in order on the link: SYN
  without ACK, SYN with ACK, then ACK without SYN.

### RFC9293-EST-2

**The SYN-ACK acknowledges the initial sequence number plus one.**

> "In line 3, TCP Peer B sends a SYN and acknowledges the SYN it received from TCP Peer A.
> Note that the acknowledgment field indicates TCP Peer B is now expecting to hear sequence
> 101, acknowledging the SYN that occupied sequence 100." — §3.5, `rfc9293.txt:1272-1277`

- Strength: description. Class: wire.
- Check idea: capture the sequence number of the SYN, then require the acknowledgment field
  of the SYN-ACK to equal that value plus one.

### RFC9293-ISS-1

**Each side chooses its own initial sequence number.**

> "When new connections are created, an initial sequence number (ISN) generator is employed
> that selects a new 32-bit ISN." — §3.4.1, `rfc9293.txt:1014-1016`

- Strength: description. Class: wire.
- Check idea: the SYN and the SYN-ACK carry two sequence numbers that the two sides pick
  on their own. A test may observe both but must not assume a value for either.

### RFC9293-ISS-2

**The initial sequence number is clock-driven, and it should add a pseudorandom function.**

> "A TCP implementation MUST use the above type of "clock" for clock-driven selection of
> initial sequence numbers (MUST-8), and SHOULD generate its initial sequence numbers with
> the expression:
>
> ISN = M + F(localip, localport, remoteip, remoteport, secretkey)
>
> where M is the 4 microsecond timer, and F() is a pseudorandom function (PRF) of the
> connection's identifying parameters" — §3.4.1, `rfc9293.txt:1032-1042`

- Strength: must for the clock (MUST-8); should for the pseudorandom part (SHLD-1).
- Class: internal. A wire observation can see that two initial sequence numbers differ, but
  it cannot establish that a generator is unpredictable. A check of the `SHOULD` needs the
  generator itself, not the link.
- Check idea: read the generator, and observe on the wire that successive connections do
  not start from the same value.
- Note: the two halves have different strengths, so a model may satisfy MUST-8 and not
  SHLD-1 without violating the RFC. Record the two separately when this entry is promoted.

## Sequence space

### RFC9293-SEQ-1

**The SYN occupies one sequence number, before any data.**

> "For sequence number purposes, the SYN is considered to occur before the first actual data
> octet of the segment in which it occurs, while the FIN is considered to occur after the
> last actual data octet in a segment in which it occurs. The segment length (SEG.LEN)
> includes both data and sequence space-occupying controls." — §3.4, `rfc9293.txt:988-992`

- Strength: description. Class: wire.
- Check idea: the acknowledgment of a SYN is the sequence number of that SYN plus one, and
  the first data octet of the sender carries that same plus-one value.

### RFC9293-SEQ-2

**A pure ACK occupies no sequence number space.**

> "Note that the sequence number of the segment in line 5 is the same as in line 4 because
> the ACK does not occupy sequence number space" — §3.5, `rfc9293.txt:1279-1281`

- Strength: description. Class: wire.
- Check idea: the empty ACK that closes the handshake and the first data segment that
  follows carry the same sequence number.

## Acknowledgment

### RFC9293-ACK-1

**The acknowledgment field holds the next sequence number expected.**

> "If the ACK control bit is set, this field contains the value of the next sequence number
> the sender of the segment is expecting to receive." — §3.1, `rfc9293.txt:337-339`

- Strength: description of the field. Class: wire.
- Check idea: after a data segment of N octets at sequence number S arrives, the
  acknowledgment that the receiver returns equals S plus N.

### RFC9293-ACK-2

**Once established, every segment carries an acknowledgment.**

> "Once a connection is established, this is always sent." — §3.1, `rfc9293.txt:339`

- Strength: description with the force of "always". Class: wire.
- Check idea: every segment after the handshake carries the ACK control bit.

## Connection termination

### RFC9293-FIN-1

**The FIN occupies one sequence number, after the last data octet.**

> "FIN — A control bit (finis) occupying one sequence number, which indicates that the
> sender will send no more data or control occupying sequence space." — §4 Glossary,
> `rfc9293.txt:3971-3974`

- Strength: description. Class: wire.
- Check idea: the acknowledgment that answers a FIN equals the sequence number of that FIN
  plus one.

### RFC9293-FIN-2

**A normal close acknowledges each FIN, one direction at a time.**

> ```
> 2.  FIN-WAIT-1  --> <SEQ=100><ACK=300><CTL=FIN,ACK>  --> CLOSE-WAIT
> 3.  FIN-WAIT-2  <-- <SEQ=300><ACK=101><CTL=ACK>      <-- CLOSE-WAIT
> 4.  TIME-WAIT   <-- <SEQ=300><ACK=101><CTL=FIN,ACK>  <-- LAST-ACK
> 5.  TIME-WAIT   --> <SEQ=101><ACK=301><CTL=ACK>      --> CLOSED
> ```
> — Figure 12, Normal Close Sequence, `rfc9293.txt:1590-1606`

- Strength: description with a normative figure. Class: wire.
- Check idea: the closing side sends FIN, the peer acknowledges it, the peer sends its own
  FIN, and the closing side acknowledges that one.

## Data transfer and segment size

### RFC9293-DATA-1

**TCP provides a reliable, in-order byte-stream service.**

> "TCP provides a reliable, in-order, byte-stream service to applications." — §2.1,
> `rfc9293.txt:243-244`
>
> "Once the connection is established, data is communicated by the exchange of segments."
> — §3.8, `rfc9293.txt:1891-1892`

- Strength: description; the service definition of the protocol. Class: end-to-end.
- Check idea: after a sender transmits a block of known size on a path without loss, the
  receiver's cumulative acknowledgment names the octet after the last one sent. That
  acknowledgment cannot occur unless every earlier octet arrived in order.

### RFC9293-SEG-1

**A segment is no larger than the effective send MSS.**

> "The maximum size of a segment that a TCP endpoint really sends, the "effective send
> MSS", MUST be the smaller (MUST-16) of the send MSS (that reflects the available
> reassembly buffer size at the remote host, the EMTU_R [19]) and the largest transmission
> size permitted by the IP layer (EMTU_S [19])" — §3.7.1, `rfc9293.txt:1745-1749`
>
> "If an MSS Option is not received at connection setup, TCP implementations MUST assume a
> default send MSS of 536 (576 - 40) for IPv4 or 1220 (1280 - 60) for IPv6 (MUST-15)."
> — §3.7.1, `rfc9293.txt:1741-1743`

- Strength: must. Class: wire.
- Check idea: a full segment of a long stream carries exactly the effective send MSS of
  data, 536 octets by default, and no segment carries more.

### RFC9293-OPT-1

**Endpoints implement the MSS option, and should send it in every SYN.**

> "TCP endpoints MUST implement both sending and receiving the MSS Option (MUST-14)."
> — §3.7.1, `rfc9293.txt:1734-1735`
>
> "TCP implementations SHOULD send an MSS Option in every SYN segment when its receive MSS
> differs from the default 536 for IPv4 or 1220 for IPv6 (SHLD-5), and MAY send it always
> (MAY-3)." — §3.7.1, `rfc9293.txt:1737-1739`

- Strength: must for the implementation; should for sending it in a SYN. Class: wire.
- Check idea: a SYN with a 24-octet header carries a 4-octet option; when the receive MSS is
  the default, the option is a `may`, and its absence is not a violation.

### RFC9293-PSH-1

**Without a PUSH flag on SEND, the sender sets PSH on the last buffered segment.**

> "A TCP endpoint MAY implement PUSH flags on SEND calls (MAY-15). If PUSH flags are not
> implemented, then the sending TCP peer: (1) MUST NOT buffer data indefinitely (MUST-60),
> and (2) MUST set the PSH bit in the last buffered segment (i.e., when there is no more
> queued data to be sent) (MUST-61)." — §3.9.1.2, `rfc9293.txt:2486-2490`

- Strength: may for the flag on the interface; must, under that condition, for the PSH
  bit on the last segment. Class: wire.
- Check idea: the segment that ends a send — the one whose last octet is the last octet the
  application handed over — carries PSH.

## Window

### RFC9293-WND-1

**The window field is the number of octets the receiver is prepared to accept.**

> "Window: 16 bits. The number of data octets beginning with the one indicated in the
> acknowledgment field that the sender of this segment is willing to accept." — §3.1,
> `rfc9293.txt:391-395`
>
> "The window sent in each segment indicates the range of sequence numbers the sender of
> the window (the data receiver) is currently prepared to accept." — §3.8.6,
> `rfc9293.txt:2116-2118`

- Strength: description of the field. Class: wire.
- Check idea: a receiver configured to accept W octets advertises W in its segments.

### RFC9293-WND-2

**The sender packages data into segments that fit the current window.**

> "The sending TCP endpoint packages the data to be transmitted into segments that fit the
> current window, and may repackage segments on the retransmission queue." — §3.8.6,
> `rfc9293.txt:2120-2122`
>
> "If this happens, the sender SHOULD NOT send new data (SHLD-15), but SHOULD retransmit
> normally the old unacknowledged data between SND.UNA and SND.UNA+SND.WND (SHLD-16)."
> — §3.8.6, on a window that shrank, `rfc9293.txt:2155-2157`

- Strength: description for the general rule; should for the shrunk-window case. Class:
  wire.
- Check idea: with a window smaller than a segment, the sender's segment carries exactly
  the window's worth of data and no new data follows until the window moves.

### RFC9293-ZWP-1

**A sender with a zero window probes it regularly.**

> "The sending TCP peer must regularly transmit at least one octet of new data (if
> available), or retransmit to the receiving TCP peer even if the send window is zero, in
> order to "probe" the window." — §3.8.6.1, `rfc9293.txt:2166-2168`

- Strength: must, written in lower case. Class: wire, with a timer.
- Check idea: level 4; a zero window followed by a probe within the persist timer.

## Acknowledgment delay

### RFC9293-ACKD-1

**An acknowledgment may be delayed, but by less than 0.5 seconds.**

> "A TCP endpoint SHOULD implement a delayed ACK (SHLD-18), but an ACK should not be
> excessively delayed; in particular, the delay MUST be less than 0.5 seconds (MUST-40)."
> — §3.8.6.3, `rfc9293.txt:2326-2328`

- Strength: should for the delay; must for its bound. Class: wire, with a timer.
- Check idea: level 4 for the bound. At level 2 the statement licenses a scenario in which
  the receiver delays its acknowledgment by less than the bound.

## Checksum

### RFC9293-CKSUM-1

**The sender must generate the checksum.**

> "The TCP checksum is never optional. The sender MUST generate it (MUST-2)" — §3.1,
> `rfc9293.txt:458-459`

- Strength: must. Class: wire.
- Check idea: every segment on the wire carries a nonzero checksum. Whether the value is
  right is an encoding statement for a serializer unit test.

### RFC9293-CKSUM-2

**The receiver must check the checksum.**

> "and the receiver MUST check it (MUST-3)." — §3.1, `rfc9293.txt:459`

- Strength: must. Class: wire (absence after a corrupted segment).
- Check idea: a segment with a wrong checksum is not acknowledged and not delivered. Needs a
  corrupted segment in flight: level 3.

## Header

### RFC9293-HDR-1

**The data offset counts the header in 32-bit words.**

> "Data Offset (DOffset): 4 bits. The number of 32-bit words in the TCP header. This
> indicates where the data begins. The TCP header (even one including options) is an
> integer multiple of 32 bits long." — §3.1, `rfc9293.txt:341-345`

- Strength: description. Class: wire.
- Check idea: a segment without options has a header of 20 octets; a SYN with the MSS option
  has 24; no header is shorter than 20.

## Reset

### RFC9293-RST-1

**A segment for a connection that does not exist is answered with a reset.**

> "If the connection does not exist (CLOSED), then a reset is sent in response to any
> incoming segment except another reset. A SYN segment that does not match an existing
> connection is rejected by this means." — §3.5.2, `rfc9293.txt:1468-1471`
>
> "If the ACK bit is off, sequence number zero is used, <SEQ=0><ACK=SEG.SEQ+SEG.LEN>
> <CTL=RST,ACK>" — §3.10.7.1, `rfc9293.txt:3287-3289`

- Strength: description, as a rule of the state machine. Class: wire.
- Check idea: a SYN to a port with no listener is answered by a segment with RST and ACK,
  sequence number zero, and an acknowledgment of the SYN's sequence number plus one; the
  attempt ends.

## Out of scope in this catalog

The state machine as a whole, retransmission and the retransmission timer, congestion
control, a shrunk or zero window beyond the probe statement, the options other than MSS
(window scale, timestamps, SACK), reset handling on a live connection and the acceptance
rules for a received reset, urgent data, keep-alives, simultaneous open and simultaneous
close, and the TIME-WAIT duration.

Every one of those needs either a companion document from
[`standards.md`](../../protocol/tcp/standards.md) or a test class beyond a single wire
observation.
