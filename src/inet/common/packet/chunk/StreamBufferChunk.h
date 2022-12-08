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

    virtual const Ptr<Chunk> peekUnchecked(PeekPredicate predicate, PeekConverter converter, const Iterator& iterator, b length, int flags) const override;

  public:
    /** @name Constructors, destructors and duplication related functions */
    //@{
    StreamBufferChunk();
    StreamBufferChunk(const StreamBufferChunk& other) = default;
    StreamBufferChunk(const Ptr<Chunk>& chunk, simtime_t startTime, bps datarate);

    virtual StreamBufferChunk *dup() const override { return new StreamBufferChunk(*this); }
    virtual const Ptr<Chunk> dupShared() const override { return makeShared<StreamBufferChunk>(*this); }

    virtual void parsimPack(cCommBuffer *buffer) const override;
    virtual void parsimUnpack(cCommBuffer *buffer) override;
    //@}

    virtual void forEachChild(cVisitor *v) override;

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
    virtual bool isMutable() const override { return Chunk::isMutable() || streamData->isMutable(); }
    virtual bool isImmutable() const override { return Chunk::isImmutable() && streamData->isImmutable(); }

    virtual bool isComplete() const override { return Chunk::isComplete() && streamData->isComplete(); }
    virtual bool isIncomplete() const override { return Chunk::isIncomplete() || streamData->isIncomplete(); }

    virtual bool isCorrect() const override { return Chunk::isCorrect() && streamData->isCorrect(); }
    virtual bool isIncorrect() const override { return Chunk::isIncorrect() || streamData->isIncorrect(); }

    virtual bool isProperlyRepresented() const override { return Chunk::isProperlyRepresented() && streamData->isProperlyRepresented(); }
    virtual bool isImproperlyRepresented() const override { return Chunk::isImproperlyRepresented() || streamData->isImproperlyRepresented(); }
    //@}

    /** @name Overridden chunk functions */
    //@{
    virtual ChunkType getChunkType() const override { return CT_STREAM; }

    virtual b getChunkLength() const override { return streamData->getChunkLength(); }

    virtual bool containsSameData(const Chunk& other) const override;

    virtual std::ostream& printFieldsToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    //@}
};

} // namespace

#endif

