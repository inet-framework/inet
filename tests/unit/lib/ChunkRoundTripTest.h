//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CHUNKROUNDTRIPTEST_H
#define __INET_CHUNKROUNDTRIPTEST_H

#include <functional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "inet/common/INETDefs.h"
#include "inet/common/packet/chunk/Chunk.h"

namespace inet {

/**
 * One filled instance of a chunk type, prepared for the round-trip engine by
 * hand-written, typed C++ code (see lib/fillers/). Every non-skipped registered
 * chunk type must have at least one; a type with none is reported as a failure,
 * so a newly added chunk cannot enter INET untested.
 *
 * A type may appear several times: each filler is a separate instantiate + fill +
 * round-trip, so a chunk whose serializer branches on its content (an address
 * family, a proxy flag, a discriminator, an optional trailer) is tested once per
 * branch. That is where the real bug-finding power is -- prefer adding a variant
 * over widening an existing one.
 */
struct ChunkFiller
{
    /** exact registered type name, e.g. "inet::aodv::Rreq". */
    std::string typeName;

    /** reporting suffix distinguishing variants of the same type ("" if single). */
    std::string label;

    /**
     * Fills the freshly instantiated chunk through its own typed setters: every wire
     * field gets a distinct, byte-asymmetric value (use FillValues, see ChunkFillers.h),
     * arrays get elements, owned pointers get objects. Fields left at their default
     * are reported by the coverage gate, so leaving one is an explicit decision that
     * must be listed in ChunkRoundTripConfig::knownUnfilled.
     *
     * Set the chunkLength only when it depends on the content (a variable-length
     * chunk): call measureAndSetChunkLength() or setChunkLength(). A fixed-size
     * header keeps the length its .msg declares -- so that the engine's
     * serialized-length invariant still checks the serializer against the model.
     */
    std::function<void(Chunk *)> fill;
};

/**
 * Chunk-type-specific configuration for the round-trip engine, supplied by the
 * .test so the engine itself stays chunk-agnostic.
 */
struct ChunkRoundTripConfig
{
    /** Type names not to round-trip: generic chunk representations, dispatch bases. */
    std::set<std::string> skipTypes;

    /**
     * Serializer *class* names that need not be tested -- structural chunks, and
     * abstract-base serializers whose concrete subclasses use a different class. A
     * serializer class no round-tripped type invoked and that is NOT on this list is
     * an untested serializer (a coverage regression) and fails the test.
     */
    std::set<std::string> skipSerializerClasses;

    /**
     * Applied to every chunk before its filler runs, for the settings a whole family
     * shares -- the checksum/FCS mode, set with a dynamic_cast so one call covers all
     * subtypes. A filler may override it afterwards. May be empty.
     */
    std::function<void(Chunk *)> commonSetup;

    /** Per-type fillers; a type may appear more than once (variants). */
    std::vector<ChunkFiller> fillers;

    /**
     * Coverage gate allow-list: the wire fields knowingly left at their default
     * value, one "Class::fieldpath" per line (blank lines and surrounding whitespace
     * ignored). After the run the engine compares the fields the fillers actually
     * left unset against this list: any not listed is a coverage gap (a newly added
     * field, or one a change stopped filling) and is reported as UNEXPECTED, failing
     * the test; listed entries no longer seen are reported as STALE so the list can
     * be pruned. Empty => every unset wire field is unexpected.
     *
     * The check reads the filled chunk through its class descriptor and compares it
     * against a fresh default instance -- reading needs no @editable/@fromString
     * annotation on the model, so a field that cannot be written through the
     * descriptor is still audited here.
     */
    std::string knownUnfilled;
};

/**
 * Constructed per-chunk round-trip test (WP8).
 *
 * Enumerates every registered chunk-serializer type, instantiates it, fills it
 * with its ChunkFiller(s) -- hand-written typed code giving every wire field a
 * distinct, byte-asymmetric value, so a swapped, shifted or wrong-endian
 * serialization is most likely to be caught -- then checks the byte round trip
 * serialize -> deserialize -> serialize and the length invariant. Prints a
 * coverage report (OK / FAILED / CRASHED / SKIPPED).
 *
 * The engine is chunk-agnostic: which types to skip, how to fill them, and which
 * unset fields are tolerated all come from @p config. It writes nothing through
 * the class descriptors -- only the read-only coverage audit uses them -- so the
 * test does not depend on the model's descriptor annotations.
 */
void testChunkRoundTrips(const ChunkRoundTripConfig& config = {});

/**
 * Measure a FieldsChunk's natural serialized length and set its chunkLength to it.
 *
 * Serializes the chunk once with an over-large declared length; the serializer
 * writes its natural number of bytes and FieldsChunkSerializer reports the actual
 * size ("serialized=N"), which is read back and set as the chunkLength. Call it from
 * a filler whose chunk has a content-dependent length. No-op for non-FieldsChunk and
 * for self-sizing/padding chunks (which accept any length and so report none) -- those
 * need an explicit setChunkLength() instead.
 */
void measureAndSetChunkLength(Chunk *chunk);

/**
 * Set a chunk's chunkLength (bit granularity; pass B(n) for bytes). Hides the
 * FieldsChunk cast -- setChunkLength() lives on FieldsChunk, not the base Chunk.
 * No-op for chunks that are not FieldsChunk.
 */
void setChunkLength(Chunk *chunk, b bits);

} // namespace inet

#endif
