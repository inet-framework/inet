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
[`checks.md`](../../protocol/quic/checks.md#stream-identifier-assignment) without one.

The lesson generalises past QUIC: **a check that cannot fail is not evidence.** Both of
these passed, and neither established anything. The ledger records the feature as `partial`
for that reason, departing from the mechanical rule of step 7 on purpose.

Finding out why the second stream does not appear is the first task of the next pass.

## Model quirks

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

## Follow-ups, in the order I would do them

1. **A protocol dissector for QUIC**, or an agreed convention that QUIC checks read chunk
   types. Everything else is gated on this. With a dissector the checks become ordinary
   field filters and stop depending on a hand-written helper.
2. **Find out why a second application stream does not reach the wire**, then run the
   withdrawn stream-identifier assignment check. It is the one check that would settle
   RFC9000-STR-1 rather than coincide with it.
3. **A check for the server's Initial datagram expansion**, on a scenario where the
   server's Initial packet is ack-eliciting. It is the one undeclared gap.
4. **Fix the tester's scalar-signal overload** so `totalRcvAppData` can be observed instead
   of aborting the run, and so an application's own byte counters become usable.
5. **Level 3, all of it available in RFC 9000 already**: ordered delivery under reordering,
   connection errors, version negotiation, the anti-amplification limit, stream reset.
6. **Level 4 needs RFC 9002**: loss detection, congestion control, the idle timeout and the
   acknowledgment delay bound.
7. **Add the model's caveats to the showcase page**, or link it to the module's "Missing
   bits" list.
