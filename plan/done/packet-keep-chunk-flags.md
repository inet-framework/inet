# Implementation plan: chunk flags in parts, conversions, and merges

> **Kind:** procedure · **Status:** done · **Seal:** none · **Owns:** — · **Stands on:** [implementation plan procedure](../../doc/project/guide/write-an-implementation-plan.md), [Packet design](../../doc/project/design/packet-anatomy.md), [sealing.md](../../doc/project/rule/sealing.md)

Checkout and revision: worktree `inet-packet-keep-chunk-flags`, branch `topic/packet-keep-chunk-flags`, base `origin/master` at `cc574d9154`. The checkout has no other local changes.

Plan revision: draft 1, 2026-11-05.

Approval: the maintainer approved draft 1 in the conversation of 2026-11-05. `src/inet/common/packet/` is sealed. The maintainer gave permission for this repair in the conversation of 2026-11-05: the flags of chunks must survive parts, conversions, and merges. The permission covers the files that [section 6](#6-future-commits-and-dependencies) names. It covers no other change in the sealed folder.

## 1. Problem and intended behavior

A chunk has three error flags: `CF_INCORRECT`, `CF_INCOMPLETE`, and `CF_IMPROPERLY_REPRESENTED`. A flag says that the content of the chunk has errors, misses data, or has an improper representation. It does not say where. Thus a part of a chunk can hold the error, and a part must have the flags of the chunk. `SliceChunk::isIncorrect()` already follows this rule, because it reads the flag of its source.

Four operations break the rule. In a hypothetical case, a radio receives raw bytes. Its PHY marks the 1000-byte `BytesChunk` of the PSDU incorrect. The MAC then reads the 24-byte MAC header with `peekAtFront<Ieee80211MacHeader>()`. The conversion gives a correct header. A peek of the first 100 bytes gives a correct `BytesChunk`. In a second case, a packet gets a correct `BytesChunk` and then an incorrect `BytesChunk`. The insertion merges them into one correct chunk, and `Packet::hasBitError()` becomes false.

The plan of the branch `topic/physicallayer-content-errors` gives a received packet with correct and incorrect chunks side by side. That design and the A-MPDU work of PR #1363 need this repair, so that raw content keeps its flags.

The intended behavior:

1. A chunk that a peek returns has the flags of every chunk that it covers.
2. The result of a conversion is incomplete or incorrect if the converted range is.
3. Two chunks with different flags do not merge.

## 2. Current implementation and evidence

The facts below come from the source at the base revision.

| Source | Current behavior | Consequence |
| --- | --- | --- |
| `peekUnchecked()` case 3 of [BitCountChunk.cc](../../src/inet/common/packet/chunk/BitCountChunk.cc), [BitsChunk.cc](../../src/inet/common/packet/chunk/BitsChunk.cc), [ByteCountChunk.cc](../../src/inet/common/packet/chunk/ByteCountChunk.cc), [BytesChunk.cc](../../src/inet/common/packet/chunk/BytesChunk.cc) | Makes a new chunk for a sub-range. It copies the region tags but not the flags. | A part of a flagged raw chunk is clean. |
| [Chunk.cc](../../src/inet/common/packet/chunk/Chunk.cc) `convertChunk()` | Serializes the range and deserializes the result. It copies the region tags. The result has only the flags that the deserializer sets. | A typed header from flagged bytes, or bytes from a flagged header, is clean. |
| `canInsertAtFront()`, `canInsertAtBack()`, `canInsertAt()` of the four raw types and [SliceChunk.cc](../../src/inet/common/packet/chunk/SliceChunk.cc) | Test only the type. `ByteCountChunk` also tests the fill byte. `SliceChunk` tests the source and the offsets. The callers are `SequenceChunk`, `Packet::insertAt()`, `ChunkQueue`, `ChunkBuffer`, and `PacketDissector`. | A merge keeps the flags of the first chunk only. |
| [SliceChunk.cc](../../src/inet/common/packet/chunk/SliceChunk.cc) `peekUnchecked()` cases 2.1 and 3 | Return the source, or a part of the source. `Chunk::simplify()` is a peek of the whole chunk. | The own flags of a slice are lost. |
| [SequenceChunk.cc](../../src/inet/common/packet/chunk/SequenceChunk.cc) `peekUnchecked()` cases 3 and 4, `doInsertSequenceChunkAtBack()`, `doInsertSliceChunkAtBack()` | Return an element or a part of it. An inserted sequence or slice of a sequence becomes its elements. | The own flags of a sequence or a slice are lost. |
| Internal peeks without `PF_ALLOW_*` in `SequenceChunk.cc` (insertion of slices, removal at both ends), `Packet.cc` (erasure), `Packet.h` (`replaceAt()`, `updateAt()`), `ChunkBuffer.cc`, `ChunkQueue.cc`, `ReorderBuffer.cc`, and the PCAP writers `recorder/PcapWriter.cc` and `recorder/PcapngWriter.cc` | They only move or record data. The PCAP recorder writes bad frames on purpose when `dumpBadFrames` is set. They pass today because a part of a raw chunk is clean. | After the repair of row 1, they would throw for flagged content. They need the three error allowances that `Chunk::simplify()` already uses. |
| [tests/packet/UnitTest.cc](../../tests/packet/UnitTest.cc) | `testIncorrect()` checks only `markIncorrect()`. `testCorruption()` checks flags of chunks of three different types, which do not merge. | No test covers parts, conversions, or merges of flagged chunks. |

The commit `packet: fix: keep the fill byte when splitting a BitCountChunk or ByteCountChunk` repaired the same case 3 for the fill byte. This plan follows that example.

## 3. Scope and acceptance criteria

1. **A1:** A part of a flagged `BitCountChunk`, `BitsChunk`, `ByteCountChunk`, or `BytesChunk` has the flags of the chunk.
2. **A2:** A conversion result is incomplete or incorrect if the converted range is. For a `SequenceChunk` source, the flags come from the elements that the range covers. The deserializer of the result alone decides whether the result is improperly represented (D2).
3. **A3:** Two chunks with different flags do not merge, for all five callers.
4. **A4:** A slice or a sequence with its own flags gives those flags to each part that a peek or an insertion takes from it.
5. **A5:** A clean chunk keeps its current representation. A peek, a merge, and a conversion of clean chunks do the same work as now, plus one integer test.
6. **A6:** The internal peeks of the packet API work with flagged content.
7. **A7:** All fingerprint tests keep their values. A difference in a configuration with corrupted chunks needs an explanation in the commit.

Position-exact error flags, for example as region tags, are outside this plan.

## 4. Proposed design and reasons

**The rule:** a chunk that a peek, a conversion, or a simplification returns has the flags of every chunk that it covers. Two chunks with different flags stay separate. `CF_IMMUTABLE` is not one of these flags.

The changes follow the places where the code already copies region tags:

- **Parts of raw chunks:** case 3 of the four raw types copies the three flags to the new chunk before it becomes immutable.
- **Conversions:** every conversion goes through `Chunk::peekConverted()`. Before the result becomes immutable, the new function `copyConvertedFlags()` marks it incomplete or incorrect if the converted range is. A clean source needs no further work. For a `SequenceChunk` source, it reads a peek of the range, which covers only the overlapping elements. A slice or a sequence result reports the flags of its content and needs no mark; this also prevents a recursion.
- **Merges:** a new protected non-virtual function of `Chunk` compares `isIncorrect()`, `isIncomplete()`, and `isImproperlyRepresented()` of two chunks. The `canInsertAt*()` functions of the four raw types and of `SliceChunk` call it. The virtual functions of `SliceChunk` include the flags of its source, so the comparison is also correct for slices.
- **Containers:** `SequenceChunk` overrides `markIncorrect()`, `markIncomplete()`, and `markImproperlyRepresented()`. The override replaces each element with a flagged copy. A sequence thus never has its own flags, and its peeks and insertions need no change. A `SliceChunk` shares its source, so it keeps its own flags. Its peek cases 2.1 and 3 then return a flagged copy of the result. `SequenceChunk::doInsertSliceChunkAtFront()` and `doInsertSliceChunkAtBack()` give the flags of the slice to the parts that they insert.
- **Internal peeks:** the peeks of row 6 in section 2 pass `PF_ALLOW_INCOMPLETE | PF_ALLOW_INCORRECT | PF_ALLOW_IMPROPERLY_REPRESENTED`, as `Chunk::simplify()` does. `PF_ALLOW_ALL` is not usable, because it contains `PF_ALLOW_EMPTY`, which a typed peek refuses.

These alternatives are not part of the plan:

- **A merge that combines the flags:** the merged chunk would mark correct data as incorrect. The position of the error would become less exact.
- **Error positions as region tags:** this needs a new API and a new representation. The rule above is enough for the current consumers.

## 5. Affected components and compatibility

The behavior changes only for flagged chunks. Today these chunks come from the `chunk` corruption mode of the physical layer and from deserializers that find an error.

Code that peeks a part of a flagged chunk without `PF_ALLOW_INCORRECT` gets a runtime error after the repair. Before the repair, it got a clean part. Such code reads data that it must not trust. Each case that the tests find gets a repair in the commit that exposes it, with a sentence in the commit message.

The release note `doc/whatsnew/fixes/packet-chunk-flags.txt` describes the rule. The packet API keeps its functions and signatures, so the migration guide needs no section.

## 6. Future commits and dependencies

| Commit and proposed subject | Dependencies |
| --- | --- |
| `C0: plan: add: keep chunk flags in parts, conversions, and merges` | none |
| `C1: packet: fix: keep the flags in a part of a raw chunk` | C0 |
| `C2: packet: fix: keep the flags of the converted range` | C1 |
| `C3: packet: fix: do not merge chunks with different flags` | C1 |
| `C4: packet: fix: give the flags of a slice or a sequence to its parts` | C3 |
| `C5: plan: change: record the results of the chunk flag repair` | C4 |
| `C6: plan: location: move the chunk flag plan to done` | C5 |

These commits change the packet API. They do not change a protocol. Thus protocol tests and protocol reviewer documents do not apply. Each commit message says that `src/inet/common/packet/` is sealed, and it names the permission of 2026-11-05. C5 writes the audit of the changed files under `audit/` at the repository root and names its path ([SR-VIOLATION-IN-SEALED](../../doc/project/rule/sealing.md#sr-violation-in-sealed)).

**C1** (done) adds the protected function `Chunk::copyFlags()`, which marks a chunk with the flags of another chunk, as `regionTags.copyTags()` does for region tags. Case 3 of `BitCountChunk.cc`, `BitsChunk.cc`, `ByteCountChunk.cc`, and `BytesChunk.cc` calls it. C1 adds the three error allowances to the internal peeks of row 6 in section 2. A new function `testIncorrectPart()` in `tests/packet/UnitTest.cc` marks each raw type incorrect, incomplete, and improperly represented. It peeks a front part, a middle part, and a back part. Each part must have the flags. It also erases the middle of a flagged chunk in a packet, inserts a slice of a sequence into a sequence, pops parts from a `ChunkQueue`, replaces the middle of a flagged chunk in a `ChunkBuffer`, and pops a part from a `ReorderBuffer`. These operations must not throw, and the parts must keep their flags. A `ChunkBuffer` joins consecutive regions into one region, so the test reads the parts of that region. Before C1, the flag checks fail.

**C2** (done) adds `Chunk::copyConvertedFlags()` and calls it in `Chunk::peekConverted()`. A new function `testIncorrectConversion()` converts an incorrect `BytesChunk` to a typed header, and a flagged typed header to bytes and to bits. It converts an improperly represented header to bytes, which stay proper. It converts a range of a sequence that covers only a correct element, and ranges that cover a flagged element. Only the second kind of result has the flag. Before C2, the first flag check fails. The first form of C2 also copied the improper representation flag. `Ieee80211RadiotapHeaderSerializer_1.test` then failed, because the bytes of an improperly represented Radiotap header are its original bytes (D2).

**C3** (done) adds the comparison function `Chunk::hasSameFlags()` to `Chunk.h` and calls it in the `canInsertAt*()` functions of the four raw types and `SliceChunk`. A new function `testIncorrectMerge()` inserts a correct and an incorrect chunk of each raw type into a packet. The two chunks stay separate: `Packet::hasBitError()` is true, and a peek of each chunk gives its own flag. Two chunks with equal flags still merge. It also checks a `ChunkQueue`, a `ChunkBuffer`, and two adjacent slices of one source with different flags. Before C3, the first check fails, because the merge keeps only the flags of the correct chunk.

**C4** (done) adds the protected function `Chunk::copyOwnFlags()`, which returns a part, or a copy of it with the own flags of the chunk. The overrides of the three mark functions of `SequenceChunk`, the peek cases 2.1 and 3 of `SliceChunk`, and the slice insertion of `SequenceChunk` call it. A new function `testIncorrectContainer()` marks a slice and a sequence incorrect. It peeks parts of them, simplifies them, and inserts them into another sequence. Each result is incorrect. Before C4, these checks fail.

**C5** (done) records the executed results, the audit, and the remaining gaps in section 9. **C6** moves this plan to `plan/done/` without a content change.

## 7. Verification methods and expected results

Follow [run-the-gates.md](../../doc/project/guide/run-the-gates.md). Build in this worktree with its own environment. Before each test run, make sure that `INET_ROOT` points to this worktree. The proposed commands run from the worktree root:

```sh
make MODE=debug -j8
(cd tests/packet && make MODE=debug && ./packet_test_dbg -u Cmdenv -c UnitTest)
opp_run_unit_tests -m debug
opp_run_module_tests -m debug
opp_run_protocol_tests -m debug
opp_run_fingerprint_tests -m debug
```

The packet test checks A1 to A6 on each commit. The other suites check that the repair breaks no consumer. The protocol suite has 27 unexpected failures on master. The repair must keep that set. The fingerprint run checks A7. None of these results exist yet.

## 8. Risks, unresolved decisions, and approval scope

- **R1:** a consumer outside the packet API can peek a part of flagged content without `PF_ALLOW_INCORRECT`. The full test suites show such cases. Each case gets a repair or an explanation.
- **R2:** a `SequenceChunk` with many elements makes `markIncorrect()` copy every element. The PHY marks single chunks, not sequences, so this cost is rare.
- **D1, conservative flags:** a part of a flagged chunk has the flag, although the error can be in another part. Recommended, because a flag has no position.
- **D2, the improper representation flag in conversions (found in C2):** the incomplete and incorrect flags describe the data, so a conversion keeps them. The improper representation flag describes one representation of the data. For example, an improperly represented Radiotap header serializes to its original bytes, which are a proper representation. Thus a conversion does not copy that flag, and the deserializer of the result decides it. A part of a raw chunk (C1) keeps the flag, because it has the same representation, as a `SliceChunk` already does.

The approval covers draft 1 of this plan, the commits C0 to C6, and the sealed files that they name.

## 9. Results

Status: done on the local branch `topic/packet-keep-chunk-flags`, not merged and not pushed. The branch `topic/physicallayer-content-errors` stands on it.

| Commit | Subject |
| --- | --- |
| `e9bacebd5d` | `plan: add: keep chunk flags in parts, conversions, and merges` |
| `74b55c09ec` | `packet: fix: keep the flags in a part of a raw chunk` |
| `ab28986659` | `packet: fix: keep the flags of the converted range` |
| `17f37347fd` | `packet: fix: do not merge chunks with different flags` |
| `b6346a50fd` | `packet: fix: give the flags of a slice or a sequence to its parts` |

Executed checks, all in debug mode, with `omnetpp-6.x` and the commands of section 7:

- `tests/packet`: passes on C1 to C4. Each new test function fails on the base library at its first check: `testIncorrectPart()` at the flags of a part, `testIncorrectConversion()` at the flag of a converted header, `testIncorrectMerge()` at the bit error of the packet, `testIncorrectContainer()` at the first element of a flagged sequence.
- Unit suite: 148 PASS on C1, C2, C3, and C4. The first form of C2 failed `Ieee80211RadiotapHeaderSerializer_1.test`, which led to decision D2.
- Module suite on C4: 823 PASS.
- Protocol suite on C4: 765 PASS, 306 expected skips, 47 expected failures, and 67 unexpected failures. The base `cc574d9154` gives the same 67 unexpected failures, test by test (RIP, ND, MLD and IGMP standards tests, and `WifiBlockAckDataRetry`).
- Fingerprint suite on C4: 1767 PASS. `examples/ipv6/mipv6 -c Handover` fails with the same values on the base. The 22 errors are configurations that need disabled features (lwIP, VoIP streams, Z3) and one Ethernet setup.
- Each commit builds alone in a detached worktree.
- The re-audit of the changed sealed files is `audit/subsystem/common-packet-chunk-flags.md`: the architecture and naming gates give the same output as on master. The series gates pass.

Gaps: `check-cpp.sh` and the release build did not run. `check-source-seals.sh` reports the sealed files, as it must; a pull request needs the protected approval of SR-PR-APPROVAL.
