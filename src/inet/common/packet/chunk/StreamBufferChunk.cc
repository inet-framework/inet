//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/packet/chunk/EmptyChunk.h"
#include "inet/common/packet/chunk/StreamBufferChunk.h"

namespace inet {

Register_Class(StreamBufferChunk);

StreamBufferChunk::StreamBufferChunk() :
    Chunk(),
    streamData(nullptr),
    startTime(-1),
    datarate(NaN)
{
}

StreamBufferChunk::StreamBufferChunk(const Ptr<Chunk>& streamData, simtime_t startTime, bps datarate) :
    Chunk(),
    streamData(streamData),
    startTime(startTime),
    datarate(datarate)
{
    CHUNK_CHECK_USAGE(streamData->isImmutable(), "chunk is mutable");
    regionTags.copyTags(streamData->regionTags, b(0), b(0), streamData->getChunkLength());
}

void StreamBufferChunk::parsimPack(cCommBuffer *buffer) const
{
    Chunk::parsimPack(buffer);
    buffer->packObject(streamData.get());
    buffer->pack(startTime);
    buffer->pack(bps(datarate).get());
}

void StreamBufferChunk::parsimUnpack(cCommBuffer *buffer)
{
    Chunk::parsimUnpack(buffer);
    streamData = check_and_cast<Chunk *>(buffer->unpackObject())->shared_from_this();
    simtime_t o;
    buffer->unpack(o);
    startTime = o;
    uint64_t l;
    buffer->unpack(l);
    datarate = bps(l);
}

void StreamBufferChunk::forEachChild(cVisitor *v)
{
    Chunk::forEachChild(v);
    v->visit(const_cast<Chunk *>(streamData.get()));
}

bool StreamBufferChunk::containsSameData(const Chunk& other) const
{
    if (&other == this)
        return true;
    else if (!Chunk::containsSameData(other))
        return false;
    else {
        auto otherStreamBuffer = static_cast<const StreamBufferChunk *>(&other);
        return startTime == otherStreamBuffer->startTime &&
               datarate == otherStreamBuffer->datarate &&
               streamData->containsSameData(*otherStreamBuffer->streamData.get());
    }
}

const Ptr<Chunk> StreamBufferChunk::peekUnchecked(PeekPredicate predicate, PeekConverter converter, const Iterator& iterator, b length, int flags) const
{
    ASSERT(iterator.isForward());
    b currentStreamPosition = b((simTime() - startTime).dbl() * datarate.get());
    if (iterator.getPosition() + length <= currentStreamPosition)
        return streamData->peekUnchecked(predicate, converter, iterator, length, flags);
    else {
        auto chunk = streamData->peekUnchecked(predicate, converter, iterator, length, flags)->dupShared();
        chunk->markIncomplete();
        return chunk;
    }
}

void StreamBufferChunk::setStartTime(simtime_t startTime)
{
    handleChange();
    this->startTime = startTime;
}

void StreamBufferChunk::setDatarate(bps datarate)
{
    handleChange();
    this->datarate = datarate;
}

std::ostream& StreamBufferChunk::printFieldsToStream(std::ostream& stream, int level, int evFlags) const
{
    if (level <= PRINT_LEVEL_DETAIL) {
        stream << EV_FIELD(startTime);
        stream << EV_FIELD(datarate);
        stream << EV_FIELD(streamData, printFieldToString(streamData.get(), level + 1, evFlags));
    }
    return stream;
}

} // namespace

