//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_STREAMBUFFERCHUNK_H
#define __INET_STREAMBUFFERCHUNK_H

#include "inet/common/packet/chunk/Chunk.h"

namespace inet {

/**
 * This chunk represents a buffer that is being filled up by streaming data from another chunk.
 */
class INET_API StreamBufferChunk : public Chunk
{
    friend class Chunk;
    friend class StreamBufferChunkDescriptor;

  protected:
    /**
     * The chunk of which this chunk is a stream buffer, or nullptr if not yet specified.
     */
    Ptr<Chunk> streamData;
    /**
     * The streaming start time measured in seconds, or -1 if not yet specified.
     */
    simtime_t startTime;
    /**
     * The streaming datarate measured in bits per second, or NaN if not yet specified.
     */
    bps datarate;

  protected:
    Chunk *_getStreamData() const { return streamData.get(); } // only for class descriptor

    const Ptr<Chunk> peekUnchecked(PeekPredicate predicate, PeekConverter converter, const Iterator& iterator, b length, int flags) const override;

  public:
    /** @name Constructors, destructors and duplication related functions */
    //@{
    StreamBufferChunk();
    StreamBufferChunk(const StreamBufferChunk& other) = default;
    StreamBufferChunk(const Ptr<Chunk>& chunk, simtime_t startTime, bps datarate);

    StreamBufferChunk *dup() const override { return new StreamBufferChunk(*this); }
    const Ptr<Chunk> dupShared() const override { return makeShared<StreamBufferChunk>(*this); }

    void parsimPack(cCommBuffer *buffer) const override;
    void parsimUnpack(cCommBuffer *buffer) override;
    //@}

    void forEachChild(cVisitor *v) override;

    /** @name Field accessor functions */
    //@{
    const Ptr<Chunk>& getStreamData() const { return streamData; }
    void setStreamData(const Ptr<Chunk>& streamData) { CHUNK_CHECK_USAGE(streamData->isImmutable(), "chunk is mutable"); this->streamData = streamData; }

    simtime_t getStartTime() const { return startTime; }
    void setStartTime(simtime_t startTime);

    bps getDatarate() const { return datarate; }
    void setDatarate(bps datarate);
    //@}

    /** @name Overridden flag functions */
    //@{
    bool isMutable() const override { return Chunk::isMutable() || streamData->isMutable(); }
    bool isImmutable() const override { return Chunk::isImmutable() && streamData->isImmutable(); }

    bool isComplete() const override { return Chunk::isComplete() && streamData->isComplete(); }
    bool isIncomplete() const override { return Chunk::isIncomplete() || streamData->isIncomplete(); }

    bool isCorrect() const override { return Chunk::isCorrect() && streamData->isCorrect(); }
    bool isIncorrect() const override { return Chunk::isIncorrect() || streamData->isIncorrect(); }

    bool isProperlyRepresented() const override { return Chunk::isProperlyRepresented() && streamData->isProperlyRepresented(); }
    bool isImproperlyRepresented() const override { return Chunk::isImproperlyRepresented() || streamData->isImproperlyRepresented(); }
    //@}

    /** @name Overridden chunk functions */
    //@{
    ChunkType getChunkType() const override { return CT_STREAM; }

    b getChunkLength() const override { return streamData->getChunkLength(); }

    bool containsSameData(const Chunk& other) const override;

    std::ostream& printFieldsToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    //@}
};

} // namespace

#endif

