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
| [RFC9293-SEGA-1](#rfc9293-sega-1) | A segment is acceptable when its sequence number falls in the receive window. |
| [RFC9293-SEGA-2](#rfc9293-sega-2) | An unacceptable segment draws an empty acknowledgment, and the state does not change. |
| [RFC9293-RST-2](#rfc9293-rst-2) | A reset is not sent when it is not clear that the segment does not belong to the connection. |
| [RFC9293-RST-3](#rfc9293-rst-3) | A reset is not sent in answer to a reset. |
| [RFC9293-RSTP-1](#rfc9293-rstp-1) | A reset is valid only when its sequence number is in the window. |
| [RFC9293-RSTP-2](#rfc9293-rstp-2) | A valid reset on a synchronized connection aborts it. |
| [RFC9293-WND-3](#rfc9293-wnd-3) | A receiver should not shrink the window. |
| [RFC9293-WND-4](#rfc9293-wnd-4) | A sender is robust when the peer shrinks the window. |
| [RFC9293-WND-5](#rfc9293-wnd-5) | With a negative usable window, the sender sends no new data. |
| [RFC9293-ICMP-1](#rfc9293-icmp-1) | An ICMP error message is acted on, and directed to the connection that caused it. |
| [RFC9293-ICMP-2](#rfc9293-icmp-2) | A received ICMP Source Quench is silently discarded. |
| [RFC9293-ICMP-3](#rfc9293-icmp-3) | A soft ICMP error does not abort the connection. |
| [RFC9293-ICMP-4](#rfc9293-icmp-4) | A hard ICMP error should abort the connection. |

The rows above the line come from the level 2 pass, which read the document for the normal
path. The rows below come from the level 3 pass, which read it for the edges: the segment
that does not fit, the reset a third party could forge, the window that moves backward, and
the error the layer below reports.

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

Some rules of this document appear twice, once in the prose of §3.5 or §3.8 and once in the
event processing of §3.10. Where that happens the entry quotes both places. Where the two
carry different keywords, the weaker one governs and the entry says so.

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

## Segment acceptance

### RFC9293-SEGA-1

**A segment is acceptable when its sequence number falls in the receive window.**

> "There are four cases for the acceptability test for an incoming segment" — §3.10.7.4,
> `rfc9293.txt:3491-3492`, followed by Table 6, `rfc9293.txt:3494-3514`. For a segment that
> carries data and a receive window above zero the test is
> "RCV.NXT =< SEG.SEQ < RCV.NXT+RCV.WND or RCV.NXT =< SEG.SEQ+SEG.LEN-1 < RCV.NXT+RCV.WND".

- Strength: description, as a rule of the state machine. Class: end-to-end.
- Check idea: give a segment a sequence number far above the right edge of the window. The
  receiver does not deliver its data to the program.

### RFC9293-SEGA-2

**An unacceptable segment draws an empty acknowledgment, and the connection does not
change state.**

> "any unacceptable segment (out-of-window sequence number or unacceptable acknowledgment
> number) must be responded to with an empty acknowledgment segment (without any user data)
> containing the current send sequence number and an acknowledgment indicating the next
> sequence number expected to be received, and the connection remains in the same state."
> — §3.5.2, `rfc9293.txt:1494-1499`
>
> "If an incoming segment is not acceptable, an acknowledgment should be sent in reply
> (unless the RST bit is set, if so drop the segment and return): <SEQ=SND.NXT>
> <ACK=RCV.NXT><CTL=ACK>" — §3.10.7.4, `rfc9293.txt:3523-3527`

- Strength: description in §3.5.2, `should` in §3.10.7.4. The two say the same thing about
  the same event; the strength of the weaker one governs.
- Class: wire plus end-to-end (the connection lives on).
- Check idea: send an out-of-window segment on a live connection. The answer is a segment
  with ACK set, no data, and the acknowledgment field unchanged at the sequence number the
  receiver still expects. The connection carries on afterwards.

## Reset generation

### RFC9293-RST-2

**A reset is not sent when it is not clear that the segment does not belong to the
connection.**

> "As a general rule, reset (RST) is sent whenever a segment arrives that apparently is not
> intended for the current connection. A reset must not be sent if it is not clear that
> this is the case." — §3.5.2, `rfc9293.txt:1462-1464`

- Strength: must not. Class: wire (absence).
- Check idea: an out-of-window segment on a live connection is answered, but never with a
  reset. The rule guards a connection against a segment that a third party crafted.

### RFC9293-RST-3

**A reset is not sent in answer to a reset.**

> "If the connection does not exist (CLOSED), then a reset is sent in response to any
> incoming segment except another reset." — §3.5.2, `rfc9293.txt:1468-1469`
>
> "If an incoming segment is not acceptable, an acknowledgment should be sent in reply
> (unless the RST bit is set, if so drop the segment and return)" — §3.10.7.4,
> `rfc9293.txt:3523-3525`

- Strength: description, as a rule of the state machine. Class: wire (absence).
- Check idea: deliver a reset to a port on which nothing listens. Nothing comes back. Two
  hosts that both answered a reset with a reset would exchange them without end.

## Reset processing

### RFC9293-RSTP-1

**A reset is valid only when its sequence number is in the window.**

> "In all states except SYN-SENT, all reset (RST) segments are validated by checking their
> SEQ fields. A reset is valid if its sequence number is in the window." — §3.5.3,
> `rfc9293.txt:1509-1511`
>
> "If the RST bit is set and the sequence number is outside the current receive window,
> silently drop the segment." — §3.10.7.4, `rfc9293.txt:3559-3561`. This sentence is one of
> three checks that apply to a stack which implements the mitigation of RFC 5961; the
> §3.5.3 rule above holds for every stack.

- Strength: description, as a rule of the state machine. Class: end-to-end (the connection
  lives on).
- Check idea: deliver a reset whose sequence number lies far outside the window of a live
  connection. The connection carries on and the data that follows still arrives. This is
  what stops a blind reset from a third party who cannot see the sequence numbers.

### RFC9293-RSTP-2

**A valid reset on a synchronized connection aborts it.**

> "The receiver of a RST first validates it, then changes state. ... otherwise, the
> receiver aborts the connection and goes to the CLOSED state. If the receiver was in any
> other state, it aborts the connection and advises the user and goes to the CLOSED state."
> — §3.5.3, `rfc9293.txt:1515-1521`

- Strength: description, as a rule of the state machine. Class: end-to-end.
- Check idea: deliver a reset whose sequence number is the next one the receiver expects.
  The connection ends, and no more data crosses it.

## Window shrinking

### RFC9293-WND-3

**A receiver should not shrink the window.**

> "A TCP receiver SHOULD NOT shrink the window, i.e., move the right window edge to the
> left (SHLD-14)." — §3.8.6, `rfc9293.txt:2150-2151`

- Strength: should not. Class: wire.
- Check idea: read the right edge of the window, the acknowledgment number plus the window
  field, in every segment a receiver sends. It never moves backward.

### RFC9293-WND-4

**A sender is robust when the peer shrinks the window.**

> "However, a sending TCP peer MUST be robust against window shrinking, which may cause the
> 'usable window' (see Section 3.8.6.2.1) to become negative (MUST-34)." — §3.8.6,
> `rfc9293.txt:2151-2153`

- Strength: must. Class: end-to-end.
- Check idea: move the right edge of the window backward on a live connection, behind the
  sequence number the sender has already reached. The sender carries on: it does not stop,
  it does not fail, and the connection still finishes.

### RFC9293-WND-5

**With a negative usable window, the sender sends no new data.**

> "If this happens, the sender SHOULD NOT send new data (SHLD-15), but SHOULD retransmit
> normally the old unacknowledged data between SND.UNA and SND.UNA+SND.WND (SHLD-16)."
> — §3.8.6, `rfc9293.txt:2155-2157`

- Strength: should not. Class: wire (absence).
- Check idea: after the right edge moves backward, no segment carrying new data goes past
  the new edge. A retransmission of data already sent is allowed and is not new data.

## ICMP

### RFC9293-ICMP-1

**An ICMP error message is acted on, and directed to the connection that caused it.**

> "TCP implementations MUST act on an ICMP error message passed up from the IP layer,
> directing it to the connection that created the error (MUST-54). The necessary
> demultiplexing information can be found in the IP header contained within the ICMP
> message." — §3.9.2.2, `rfc9293.txt:2828-2831`

- Strength: must. Class: internal.
- Check idea: what "act on" means is the subject of the three entries below. The choice of
  connection happens inside the module, at the interface between the ICMP module and TCP.

### RFC9293-ICMP-2

**A received ICMP Source Quench is silently discarded.**

> "TCP implementations MUST silently discard any received ICMP Source Quench messages
> (MUST-55)." — §3.9.2.2, `rfc9293.txt:2841-2842`

- Strength: must. Class: end-to-end (absence of a change) plus error-signal (absence).
- Check idea: deliver a Source Quench for a live connection. Nothing changes: no answer, no
  break in the flow.

### RFC9293-ICMP-3

**A soft ICMP error does not abort the connection.**

> "Since these Unreachable messages indicate soft error conditions, a TCP implementation
> MUST NOT abort the connection (MUST-56), and it SHOULD make the information available to
> the application (SHLD-25)." — §3.9.2.2, `rfc9293.txt:2852-2854`

The document lists the soft errors for IPv4 as "Destination Unreachable -- codes 0, 1, 5;
Time Exceeded -- codes 0, 1; and Parameter Problem", §3.9.2.2, `rfc9293.txt:2845-2846`.

- Strength: must not. Class: end-to-end.
- Check idea: deliver a Destination Unreachable with code 0 for a live connection. The
  connection stays open and the transfer finishes.

### RFC9293-ICMP-4

**A hard ICMP error should abort the connection.**

> "These are hard error conditions, so TCP implementations SHOULD abort the connection
> (SHLD-26)." — §3.9.2.2, `rfc9293.txt:2859-2860`

The document lists the hard errors as "Destination Unreachable -- codes 2-4", §3.9.2.2,
`rfc9293.txt:2857`.

- Strength: should. Class: end-to-end.
- Check idea: deliver a Destination Unreachable with code 3 for a live connection. The
  connection ends. The document itself notes that many implementations do not do this in a
  synchronized state, so silence here is a `declined` and not a defect.

## Out of scope in this catalog

The state machine as a whole, retransmission and the retransmission timer, congestion
control, the zero window beyond the probe statement, the options other than MSS (window
scale, timestamps, SACK), urgent data, keep-alives, simultaneous open and simultaneous
close, silly window avoidance, and the TIME-WAIT duration.

Every one of those needs either a companion document from
[`standards.md`](../../protocol/tcp/standards.md) or a test class beyond a single wire
observation.

The level 3 pass took four subjects off this list, because RFC 9293 states each one and a
relay on the path can produce the event: the acceptance rules for a received segment, reset
handling on a live connection, the shrinking of the window, and the ICMP messages that reach
a connection. They are entries of this catalog now.
