# QUIC standards tests — level 3

> **Kind:** how · **Status:** done · **Stands on:**
> [derive-tests-from-a-standard.md](../../doc/project/guide/derive-tests-from-a-standard.md),
> [quic/standards.md](../../doc/project/evidence/protocol/quic/standards.md)

The QUIC pass reached **level 2 partial**. Two things must happen before level 3 can be
claimed, and the ledger names both. This plan does the level 2 work first and then the
level 3 work, in one pass on `master`, in the worktree `inet-master`.

## What the level 2 pass left open

1. `QUIC-F-STREAMS` rests on a check that cannot fail: the model always uses stream 0.
2. `QUIC-F-INITIAL-SIZE` has no check for the server half of the requirement.

Two probes settled both, before any test was written.

- **The stream identifier.** With two generators the client's data goes on **one** stream,
  and the second generator's bytes continue the first one's offsets. With a single
  generator asked for identifier 1, the client still sends on stream 0. The model has one
  identifier and no path that assigns another, so no check on the wire can fail. The
  statement becomes `no check` with that reason, and the feature keeps its `partial` value
  for a stated cause rather than for a missing test.
- **The server's Initial datagram.** The server sends an Initial packet of 27 octets that
  carries a CRYPTO frame, which is ack-eliciting. RFC 9000 §14.1 demands 1200. This is the
  undeclared gap the level 2 notes suspected, and it becomes a test.

## What level 3 adds

RFC 9000 holds the text for all of it; no new document is needed. The catalog's own
out-of-scope list names the four subjects this plan takes:

- §2.2 — an endpoint buffers data received out of order and still delivers an ordered
  stream. The statement is in the catalog already; only its edge is new.
- §6.1 — a server that does not accept the client's version answers with a Version
  Negotiation packet, and no endpoint answers such a packet with another.
- §8.1 — before the client's address is validated, a server sends at most three times the
  bytes it received.
- §12.4 — a frame of unknown type is a connection error of type FRAME_ENCODING_ERROR.

## The toolset, and what is different about QUIC

QUIC has **no protocol dissector** in this tree, so `quic.<field>` filters do not work at
all. The level 2 pass built `QuicChunks.h`, which walks the packet's chunk list directly.
Level 3 keeps that and adds mutation:

- A relay selects frames by `udp.destPort`, which the filter can read, and the mutator then
  decides by looking at the chunks. The filter's blindness costs nothing.
- A relay that changes a chunk rebuilds the packet content from its chunk list.
- One check needs no mutation at all: reordering is a `delay` on the relay.

## Steps

- [x] **Step 3 — catalog.** Add the four statements to `standard/rfc9000/catalog.md`.
- [x] **Step 4 — features.** Three features: version negotiation, address validation,
      frame validation.
- [x] **Step 5 — checks.** Split `protocol/quic/checks.md` per feature and add five
      procedures.
- [x] **Step 6 — tests.** Extend `QuicChunks.h` with mutation, and write five tests.
- [x] **Step 7 — results.** Record every verdict and both probes in `model/quic/results.md`.
- [x] **Step 8 — conformance and ledger.** Update the four model documents.
- [x] **Step 9 — run and commit.** Build, run every suite, check the links, commit.

## The tests

Five tests, and the verdict each one reached:

| Test | Checks | Verdict |
| --- | --- | --- |
| Rfc9000ReorderedDelivery | RFC9000-STR-2 at its edge | PASS |
| Rfc9000AntiAmplification | RFC9000-AMP-1 | PASS |
| Rfc9000ServerInitialSize | RFC9000-SIZE-1, the server half | FAIL, gap 1: the datagram is 27 octets |
| Rfc9000VersionNegotiation | RFC9000-VER-1 | FAIL, gap 2: a version nobody speaks buys a full connection |
| Rfc9000UnknownFrameType | RFC9000-ERR-1 | FAIL, gap 3: the run stops |

## Facts found before the work

- `ConnectionState.cc` dispatches an incoming frame on `getFrameType()`, and its default
  branch throws `cRuntimeError("Unknown Frame Header Type")`. The unknown-frame check
  therefore has a predictable outcome, and it is not the one RFC 9000 asks for.
- The server's handshake datagrams are 27, 38, 24 and 22 octets. None of them is padded.

## Result

**Level 2 reached, level 3 partial.** 11 tests in the suite: 8 PASS and 3 declared FAIL. The
two level 2 blockers are closed, one by a check that fails and one by a probe that proves no
check can succeed. Four of the nine level 3 subjects of the catalog are done. The full
account is in `doc/project/evidence/model/quic/`.

The tooling prediction of pass 1 was wrong in a useful way. A dissector was thought to gate
level 3; it does not. A relay selects a datagram by a UDP field or its size, and the mutator
reads and writes the QUIC chunks directly.

## Facts found during the work

- A size threshold of 1000 octets selects the client's Initial datagram, which is padded to
  exactly 1200. Use 1300 to mean "a datagram carrying data".
- Holding a packet makes the sender retransmit it, so the gap is filled by the copy and the
  held original arrives afterwards as a duplicate.
- `mutateFirstQuicChunk` returning false lets one relay be pointed at a whole direction and
  still change only the packets that carry the chunk it wants.

