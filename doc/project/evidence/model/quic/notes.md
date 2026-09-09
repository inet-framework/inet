# QUIC — quirks and follow-ups

> **Kind:** reference · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [results.md](results.md), [coverage.md](coverage.md)

What this document is for. The workflow's artifacts each answer one question: what the
standard says, what a run showed, what the model claims. This one holds what a person
learned while doing the work and would otherwise have to learn again — the behaviors of the
model that surprised the author, the traps in the tooling that cost a debugging cycle, and
the ordered list of what to do next.

The tooling quirks that apply to every protocol are in
[`ipv4/notes.md`](../ipv4/notes.md#tooling-quirks). QUIC added more than any other protocol,
because it is the first one the test framework cannot see.

## The framework cannot see QUIC

This is the finding that shaped the whole pass, and the first thing anyone continuing this
work needs to know.

1. **No protocol dissector is registered for QUIC.** A repository-wide search for
   `Register_Protocol_Dissector` finds none for `Protocol::quic`. `PacketDissector`
   therefore falls back to the default dissector, which reports the whole undissected
   remainder as one chunk, so the class map the framework builds gets a single entry —
   `SequenceChunk` — and never the QUIC classes. Every `quic.<field>` filter and every
   `evalPacketField(pkt, "InitialPacketHeader.version")` call fails with
   `protocol '...' not present in packet`. This is architectural. Without a dissector it
   cannot work.
2. **The Quic module emits none of the signals the framework recognises.** It emits
   `packetSent` and `packetReceived` of type `cPacket`, plus a large family of statistics.
   The framework turns only `packetSentToLower`, `packetReceivedFromLower`,
   `packetSentToUpper` and their relatives into events, so `on("client.quic")` has nothing
   to hook.
3. **The way in.** QUIC rides on UDP, and the UDP module does emit the recognised signals.
   Every check observes UDP datagrams at `client.udp` and `server.udp` and reads QUIC
   fields by walking the packet's own chunk list, which carries the concrete QUIC chunk
   types in wire order. The suite's helper
   [`QuicChunks.h`](../../../../../tests/protocol/quic/QuicChunks.h) does that walk and
   documents why.
4. **A trap inside that helper.** Its first form returned only the **first** chunk of a
   kind, and a QUIC packet carries several frames. A check that must hold for *every* frame
   of a kind therefore silently examined one, and passed. The helper now offers both forms,
   `findQuicChunk` and `findAllQuicChunks`, and says which to use when.
5. **Two statistics that cannot be observed.** The application's `bytesReceived` fires per
   chunk, never as a total. The module's `totalRcvAppData` is a running total, but
   subscribing to it **aborts the run**: `Unsupported signal data type uintval_t`, because
   the tester implements only the signed overload.
6. **A scalar signal cannot carry a predicate.** `EventPattern::selectorMatches` refuses
   `.match()` and `.packet()` on any non-packet event, so a running total cannot be
   accumulated on the signal side. The flow-control check keeps its total in state shared
   between the steps' predicates instead.

## The vacuous pass, and why one test was withdrawn

`Rfc9000StreamIdentifiers.test` asserts that the client's stream identifier is even and
bidirectional. It passes — because the reference application hardcodes stream 0, not
because the model maintains the encoding. Reading the model settles it: `getStreamId`
returns the application's number unchanged
([QuicTrafficgen.cc:234-237](../../../../../src/inet/applications/quicapp/QuicTrafficgen.cc#L234-L237)),
`findOrCreateStream` validates nothing
([Connection.cc:169-178](../../../../../src/inet/transportlayer/quic/Connection.cc#L169-L178)),
and `Quic.ned` declares the deviation itself.

A decisive check was written — two application streams, every client identifier must be a
multiple of four — and **it passed too**, because the application's second stream never
reached the observation point. Rather than keep a green test whose pass could not be
explained, the test was removed and the check left in
[`checks.md`](../../protocol/quic/checks/streams.md#stream-identifier-assignment) without one.

The lesson generalises past QUIC: **a check that cannot fail is not evidence.** Both of
these passed, and neither established anything. The ledger records the feature as `partial`
for that reason, departing from the mechanical rule of step 7 on purpose.

Finding out why the second stream does not appear is the first task of the next pass.

**Pass 2 found out, and the answer changes the conclusion rather than the verdict.** The
second stream does appear. It is stream 0. With two generators the client sends every frame
on stream 0 and the second generator's bytes continue the first one's offsets; with one
generator asked for identifier 1, the client still sends on stream 0. The model has one
identifier and no path that assigns another.

So the withdrawn test could not have worked, and neither can any successor of it. The
statement is now `no check` in the ledger with that proof behind it, instead of a candidate
waiting for a better scenario. The lesson above stands and gains a second half: **a check that
cannot fail is not evidence, and finding out why it cannot fail is worth more than another
attempt at writing it.**

## Model quirks

### A model can stop where a standard says it must answer

Three models in this tree stop the simulation on an input a standard describes: IPv4 on an
unknown ICMP type, TCP on a Source Quench, and QUIC on a frame type no version defines. Each
is the default branch of a switch, in a different module written by different people.

The QUIC one is the widest in reach. Frame types come from a registry that grows by design,
so any peer of a later version can stop this endpoint by using a frame it is entitled to use.
RFC 9000 asks for a connection error, which is an answer to the peer and the end of one
connection; a throw is neither.

### The version field is read and ignored

A packet whose version is 0x0A0A0A0A completes a handshake with this model and transfers
data. The field is the first thing a QUIC endpoint reads and the one decision it can take
with no state at all, and the model takes none.

### A bound can hold by luck

The anti-amplification limit of §8.1 is respected in every run, and nothing in the model
enforces it. The client's Initial datagram is padded to 1200 octets, so the server has a
3600-octet budget from the first packet, and the server's whole handshake is about a hundred
octets. The model's own list of missing bits names amplification mitigation as absent, and
the check agrees with the list rather than with the behaviour. A reader who saw only the
green verdict would draw the wrong conclusion.

### The 1200-octet floor is half implemented, and the missing half is undeclared

`buildClientInitialPacket` pads to 1200 octets
([PacketBuilder.cc:476](../../../../../src/inet/transportlayer/quic/PacketBuilder.cc#L476)).
`buildServerInitialPacket` has no padding at all. RFC 9000 §14.1 binds **both** endpoints,
the server whenever the datagram is ack-eliciting. The model's own "Missing bits" list names
amplification-attack mitigation — the neighbouring §8 rule — but not this. It is the single
undeclared departure in an otherwise exemplary declaration, and the strongest candidate for
a `defect` at the next pass.

### Twelve declared deviations, and what they cost

`Quic.ned` is the best-documented module in this tree: it names its RFCs by number and then
lists twelve things it does not do. Two land on this pass's catalog — stream types are not
distinguished, and the connection identifier is always 0, which makes the identifier half of
RFC9000-PKT-1 degenerate. Eight RFC 9000 frame types are absent from the model's enumeration
(RESET_STREAM, STOP_SENDING, MAX_STREAMS, STREAMS_BLOCKED, NEW_CONNECTION_ID,
RETIRE_CONNECTION_ID, PATH_CHALLENGE, PATH_RESPONSE), each matching a declared deviation.

Against such a model a declared deviation is `declined`, not `defect` — the model never
claimed the behavior. The workflow's value here is not catching it out; it is confirming
whether the declared list is complete. It is, but for the server-side padding above.

### No encryption, which is what makes the checks possible

The model sends everything unencrypted and says so. On a protected connection every check
that reads a frame field would need the keys. That is worth remembering before anyone adds
packet protection: it would silently disable most of this suite.

### A showcase without the caveats

`showcases/quic/linksharing/doc/index.rst` describes QUIC's TLS 1.3 encryption and its
connection migration as properties of QUIC, without saying that this model implements
neither. The module documentation is careful; the showcase does not inherit that care.

## Tooling quirks, added at level 3

### A relay can work without a dissector

The relay's filter cannot see QUIC fields, and that turned out not to matter. Select a
datagram by its UDP port or its size, and let the mutator look at the chunks and decide. A
mutator that returns false for a packet it does not want lets one relay be pointed at a whole
direction.

### Size is a sharp selector, and 1200 is a trap

A QUIC client pads its Initial datagram to exactly 1200 octets. A size threshold below that
selects the handshake rather than the data, which is how the reordering check first delayed
the wrong packet. Use 1300 or more to mean "a datagram carrying data".

### Holding one packet makes the sender retransmit

The reordering check holds the first data packet for 20 ms. The client does not wait: it
retransmits that packet quickly, so the gap is filled by the copy and the held original
arrives afterwards as a duplicate. The check had to be ordered around that, and the discard
question had to be asked of a different offset. A relay that holds a packet is producing a
loss as far as the sender is concerned.

## Follow-ups, in the order I would do them

Items 2, 3 and most of 5 are done. Item 1 turned out not to gate the rest. What follows is
the list as pass 2 leaves it.

1. **Correct the frame type switch** so that a type the module does not know ends the
   connection with FRAME_ENCODING_ERROR instead of ending the run. It is the gap with the
   widest reach, and the same shape waits in the ICMP module for IPv4 and TCP.
2. **Implement version negotiation**, or declare its absence in the module's "Missing bits"
   list. Either would be an improvement; today the model neither does it nor says it does
   not.
3. **Pad the server's ack-eliciting Initial datagram** to 1200 octets, or declare that half
   as an exception the way the eleven others are declared.
4. **A protocol dissector for QUIC.** Not a gate any more: pass 2 read and changed QUIC
   chunks without one. It would make every check of this suite shorter, and it would let the
   relay's own filter select on QUIC fields instead of on size.
5. **Fix the tester's scalar-signal overload** so `totalRcvAppData` can be observed instead
   of aborting the run, and so an application's own byte counters become usable. This is
   what keeps every "the application received" observation a proxy at the transport layer.
6. **Level 3, what remains**: the stateless reset, address validation with Retry and tokens,
   path validation and migration, stream reset, and a packet that cannot be decrypted. Also
   RFC9000-VER-2, which the relay can now reach.
7. **Level 4 needs RFC 9002**: loss detection, congestion control, the idle timeout and the
   acknowledgment delay bound.
8. **Add the model's caveats to the showcase page**, or link it to the module's "Missing
   bits" list.
