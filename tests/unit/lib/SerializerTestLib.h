//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SERIALIZERTESTLIB_H
#define __INET_SERIALIZERTESTLIB_H

#include <set>
#include <string>

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
void reportSerializerCoverage(const std::set<std::string>& skipTypes = {});

} // namespace inet

#endif
