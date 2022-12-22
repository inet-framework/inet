//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_EMPTYCHUNK_H
#define __INET_EMPTYCHUNK_H

#include "inet/common/packet/chunk/Chunk.h"

namespace inet {

/**
 * This class represents a completely empty chunk.
 */
class INET_API EmptyChunk : public Chunk
{
    friend class Chunk;

  protected:
    virtual const Ptr<Chunk> peekUnchecked(PeekPredicate predicate, PeekConverter converter, const Iterator& iterator, b length, int flags) const override;

    static const Ptr<Chunk> convertChunk(const std::type_info& typeInfo, const Ptr<Chunk>& chunk, b offset, b length, int flags);

  public:
    /** @name Constructors, destructors and duplication related functions */
    //@{
    EmptyChunk();
    EmptyChunk(const EmptyChunk& other);

    virtual EmptyChunk *dup() const override { return new EmptyChunk(*this); }
    virtual const Ptr<Chunk> dupShared() const override { return makeShared<EmptyChunk>(*this); }
    //@}

    /** @name Overridden chunk functions */
    //@{
    virtual ChunkType getChunkType() const override { return CT_EMPTY; }
    virtual b getChunkLength() const override { return b(0); }

    virtual std::ostream& printFieldsToStream(std::ostream& stream, int level, int evFlags = 0) const override { return stream; }
    //@}

    static const Ptr<Chunk> getEmptyChunk(int flags) {
        if (flags & PF_ALLOW_NULLPTR)
            return nullptr;
        else if (flags & PF_ALLOW_EMPTY)
            return makeShared<EmptyChunk>();
        else
            throw cRuntimeError("Returning an empty chunk is not allowed according to the flags: %x", flags);
    }
};

} // namespace

#endif

