//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_SLICECHUNK_H_
#define __INET_SLICECHUNK_H_

#include "inet/common/packet/chunk/Chunk.h"

namespace inet {

/**
 * This class represents data using a slice of another chunk. The slice is
 * designated with the sliced chunk, an offset, and a length field, both
 * measured in bits. It's used by the Chunk API implementation internally to
 * efficiently represent slices of other chunks. User code should not directly
 * instantiate this class.
 */
class INET_API SliceChunk : public Chunk
{
  friend class Chunk;
  friend class SliceChunkDescriptor;

  protected:
    /**
     * The chunk of which this chunk is a slice, or nullptr if not yet specified.
     */
    Ptr<Chunk> chunk;
    /**
     * The offset measured in bits, or -1 if not yet specified.
     */
    b offset;
    /**
     * The length measured in bits, or -1 if not yet specified.
     */
    b length;

  protected:
    Chunk *_getChunk() const { return chunk.get(); } // only for class descriptor

    virtual const Ptr<Chunk> peekUnchecked(PeekPredicate predicate, PeekConverter converter, const Iterator& iterator, b length, int flags) const override;

    static const Ptr<Chunk> convertChunk(const std::type_info& typeInfo, const Ptr<Chunk>& chunk, b offset, b length, int flags);

    virtual void doInsertAtFront(const Ptr<const Chunk>& chunk) override;
    virtual void doInsertAtBack(const Ptr<const Chunk>& chunk) override;

    virtual void doRemoveAtFront(b length) override;
    virtual void doRemoveAtBack(b length) override;

  public:
    /** @name Constructors, destructors and duplication related functions */
    //@{
    SliceChunk();
    SliceChunk(const SliceChunk& other);
    SliceChunk(const Ptr<Chunk>& chunk, b offset, b length);

    virtual SliceChunk *dup() const override { return new SliceChunk(*this); }
    virtual const Ptr<Chunk> dupShared() const override { return makeShared<SliceChunk>(*this); }
    //@}

    virtual void forEachChild(cVisitor *v) override;

    /** @name Field accessor functions */
    //@{
    const Ptr<Chunk>& getChunk() const { return chunk; }
    void setChunk(const Ptr<Chunk>& chunk) { CHUNK_CHECK_USAGE(chunk->isImmutable(), "chunk is mutable"); this->chunk = chunk; }

    b getOffset() const { return offset; }
    void setOffset(b offset);

    b getLength() const { return length; }
    void setLength(b length);
    //@}

    /** @name Overridden flag functions */
    //@{
    virtual bool isMutable() const override { return Chunk::isMutable() || chunk->isMutable(); }
    virtual bool isImmutable() const override { return Chunk::isImmutable() && chunk->isImmutable(); }

    virtual bool isComplete() const override { return Chunk::isComplete() && chunk->isComplete(); }
    virtual bool isIncomplete() const override { return Chunk::isIncomplete() || chunk->isIncomplete(); }

    virtual bool isCorrect() const override { return Chunk::isCorrect() && chunk->isCorrect(); }
    virtual bool isIncorrect() const override { return Chunk::isIncorrect() || chunk->isIncorrect(); }

    virtual bool isProperlyRepresented() const override { return Chunk::isProperlyRepresented() && chunk->isProperlyRepresented(); }
    virtual bool isImproperlyRepresented() const override { return Chunk::isImproperlyRepresented() || chunk->isImproperlyRepresented(); }
    //@}

    /** @name Overridden chunk functions */
    //@{
    virtual ChunkType getChunkType() const override { return CT_SLICE; }
    virtual b getChunkLength() const override { CHUNK_CHECK_IMPLEMENTATION(length >= b(0)); return length; }

    virtual bool canInsertAtFront(const Ptr<const Chunk>& chunk) const override;
    virtual bool canInsertAtBack(const Ptr<const Chunk>& chunk) const override;

    virtual bool canRemoveAtFront(b length) const override { return false; }
    virtual bool canRemoveAtBack(b length) const override { return false; }

    virtual std::string str() const override;
    //@}
};

} // namespace

#endif // #ifndef __INET_SLICECHUNK_H_

