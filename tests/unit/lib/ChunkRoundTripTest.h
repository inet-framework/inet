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
 * A per-type preparation recipe for the round-trip engine. Every chunk type that
 * needs anything beyond the generic scalar fill (an explicit or measured
 * chunkLength, a specific discriminator/field value, an array element, ...) is
 * described by one or more recipes. A type may appear multiple times: each recipe
 * is a separate instantiate + fill + round-trip, so a class that serializes
 * differently per discriminator value (e.g. AODV Rreq as RREQ vs RREQ_IPv6) is
 * tested for each.
 */
struct ChunkRecipe
{
    /** exact registered type name, e.g. "inet::aodv::Rreq". */
    std::string typeName;

    /** reporting suffix distinguishing variants of the same type ("" if single). */
    std::string label;

    /**
     * Prepares the freshly instantiated, generically filled chunk for the round
     * trip (set the discriminator/fields, resize arrays, establish the chunkLength,
     * whatever the chunk needs). Build the common declarative case with
     * SimpleFiller(); a truly special chunk can use any lambda.
     */
    std::function<void(Chunk *)> fill;
};

/**
 * The common, declarative filler for a ChunkRecipe: sets named fields through the
 * class descriptor and establishes the chunkLength. Returns a fill function for
 * ChunkRecipe::fill. Chunks that need more (array elements, cross-field logic, ...)
 * use a custom fill lambda instead.
 *
 * @param fields  field name -> value; editable fields only, enums by symbol (e.g.
 *                "RREQ_IPv6"). Non-editable meta fields (checksum/FCS mode) cannot
 *                be set this way -- use ChunkRoundTripConfig::commonSetup.
 * @param length  chunkLength to set: b(-1) (default) -> measure from the serializer;
 *                >= b(0) -> explicit (bit granularity; pass B(n) for bytes).
 */
std::function<void(Chunk *)> SimpleFiller(
        std::vector<std::pair<std::string, std::string>> fields = {},
        b length = b(-1));

/**
 * Like SimpleFiller, but first fills the chunk recursively -- descending into nested
 * compound members that the default flat fill leaves at their defaults -- so a serializer
 * that walks a sub-struct's fields is exercised with real values. Opt-in per chunk,
 * because deep filling changes the serialized size and can reach a discriminator buried
 * in a sub-object; use it only where the nested content is pure data. Any @p fields are
 * applied after the deep fill (pin a discriminator the recursion would otherwise clobber);
 * @p length defaults to b(-1) -> measure the (now larger) size from the serializer.
 */
std::function<void(Chunk *)> RecursiveFiller(
        std::vector<std::pair<std::string, std::string>> fields = {},
        b length = b(-1));

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
     * Applied (typed) to every chunk after the generic fill, before any recipe. For
     * non-editable meta fields the descriptor cannot reach -- the checksum/FCS mode,
     * set with a dynamic_cast so one call covers a whole family. May be empty.
     */
    std::function<void(Chunk *)> commonSetup;

    /** Per-type recipes; a type may appear more than once (variants). */
    std::vector<ChunkRecipe> recipes;

    /**
     * Coverage gate allow-list: the wire fields known to be left unfilled, one
     * "Class::fieldpath" per line (blank lines and surrounding whitespace ignored).
     * After the run the engine compares the actual unfilled wire fields against this
     * list: any not listed is a coverage regression (a newly added field, or one a
     * change stopped filling) and is reported as UNEXPECTED, failing the test; listed
     * entries no longer seen are reported as STALE so the list can be pruned. Empty =>
     * every gap is unexpected.
     */
    std::string knownUnfilled;
};

/**
 * Constructed per-chunk round-trip test (WP8).
 *
 * Enumerates every registered chunk-serializer type, instantiates it, fills its
 * simple scalar fields with distinct, byte-asymmetric values (so a swapped,
 * shifted or wrong-endian serialization is most likely to be caught), then
 * checks the byte round-trip serialize -> deserialize -> serialize and the
 * length invariant. Prints a coverage report (OK / FAILED / CRASHED / SKIPPED).
 *
 * The engine only auto-fills flat scalar fields; compound/pointer/array fields
 * are left at their defaults. All chunk-type-specific knowledge (which types to
 * skip, which meta fields to set) is supplied through @p config.
 */
void testChunkRoundTrips(const ChunkRoundTripConfig& config = {});

/**
 * Measure a FieldsChunk's natural serialized length and set its chunkLength to it.
 *
 * Serializes the chunk once with an over-large declared length; the serializer
 * writes its natural number of bytes and FieldsChunkSerializer reports the actual
 * size ("serialized=N"), which is read back and set as the chunkLength. Applied by
 * the engine for a recipe with length == b(-1). No-op for non-FieldsChunk and for
 * self-sizing/padding chunks (which accept any length and so report none) -- those
 * need an explicit length in the recipe instead.
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
