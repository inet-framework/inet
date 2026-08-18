//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SERIALIZERTESTLIB_H
#define __INET_SERIALIZERTESTLIB_H

#include <set>
#include <string>
#include <vector>

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * Reads each record of a pcap file, deserializes it into typed chunks using the
 * generic PacketDissector, re-serializes the result, and compares it byte-by-byte
 * with the original frame. Progress and per-frame results are written to EV.
 *
 * This replaces the hand-written, per-protocol deserialization chain: the generic
 * dissector automatically covers every registered protocol (falling back to raw
 * bytes for unknown payloads, which round-trip trivially).
 *
 * @param filename  path to the pcap file (relative to the running test's work dir)
 * @param hasFcs    true if the Ethernet frames in the file already contain the FCS
 *                  trailer; false means a computed FCS is appended before the
 *                  round-trip (real captures usually omit the FCS)
 * @return true if every stored (non-truncated) frame round-trips identically
 */
bool testPcapSerialization(const char *filename, bool hasFcs = true);

/**
 * Starts recording serializer coverage: installs a recorder on the
 * ChunkSerializerRegistry and resets the accumulated coverage / raw-bytes state.
 * Call once before the testPcapSerialization() calls whose coverage should be
 * measured together.
 */
void beginSerializerCoverage();

/**
 * One expectation about a frame: the value a named field of the first chunk of a given
 * type must have after the frame has been dissected.
 */
struct ChunkFieldCheck {
    std::string chunkTypeName;  ///< class name of the chunk, e.g. "inet::MrpTest"
    int occurrence;             ///< which chunk of that class in the frame (a stack of tags has several)
    std::string fieldName;      ///< field name as the message file declares it
    std::string expectedValue;  ///< the value as the generated class descriptor renders it
};

/**
 * Starts a run of field checks: resets the counters reportChunkFieldChecks() prints.
 */
void beginChunkFieldChecks();

/**
 * Dissects one frame of a capture and compares fields of the chunks it yields against
 * values taken from an independent decoder (tshark), so that a serializer which reads a
 * field from the wrong place is caught -- something neither round-trip test can do, since
 * both reproduce the bytes whatever the fields mean.
 *
 * @param filename     path to the pcap file (relative to the running test's work dir)
 * @param frameNumber  1-based record number within the file
 * @param hasFcs       as for testPcapSerialization(): whether the frames carry their FCS
 * @param checks       what the fields of that frame must hold
 */
void checkChunkFields(const char *filename, int frameNumber, bool hasFcs, const std::vector<ChunkFieldCheck>& checks);

/**
 * Runs every expectation of the field reference that belongs to a capture: the file the
 * pcap's name gives with a .fields extension, written by tests/unit/tools/mkfieldrefs.py
 * from what tshark reads. Its lines are tab-separated: frame number, chunk class, field,
 * expected value, and the tshark field the value came from.
 *
 * @param filename  path to the pcap file; the reference is looked up next to it
 * @param hasFcs    as for testPcapSerialization()
 */
void checkChunkFieldsFromReference(const char *filename, bool hasFcs);

/**
 * Prints the summary of the field checks: how many were made, how many did not hold, and
 * -- from INET's own class descriptors, so exactly -- which fields of the chunks the
 * captures dissect no expectation names at all. That last list is what the reference does
 * not guard: it is the work list for extending the mapping of mkfieldrefs.py.
 */
void reportChunkFieldChecks();

/**
 * Serializer type names (as opp_typename) that a pcap round-trip can never
 * exercise -- generic chunk representations, abstract dispatch bases, INET-internal
 * formats and physical-layer headers. The default argument of
 * reportSerializerCoverage(); see the definition for the reasoning per group.
 */
extern const std::set<std::string> pcapCoverageSkipTypes;

/**
 * Stops recording and prints the coverage report to EV: the exercised
 * serializers (USED ...), the registered-but-never-invoked ones (UNTESTED ...),
 * and the raw-bytes summary (leaf-region count + any TOPLEVEL-RAWBYTES frames,
 * i.e. a whole frame whose outermost protocol was not parsed at all).
 *
 * @param skipTypes  serializer type names (as opp_typename, e.g. "inet::SliceChunk")
 *                   that are not meaningfully exercised by a pcap round-trip -- the
 *                   generic chunk-representation "plumbing". When registered but not
 *                   exercised they are listed as SKIPPED instead of UNTESTED, so the
 *                   UNTESTED list holds only real protocol serializers still to cover.
 */
void reportSerializerCoverage(const std::set<std::string>& skipTypes = pcapCoverageSkipTypes);

} // namespace inet

#endif
