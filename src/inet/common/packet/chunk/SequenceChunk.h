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

#ifndef __INET_SEQUENCECHUNK_H_
#define __INET_SEQUENCECHUNK_H_

#include <deque>

#include "inet/common/packet/chunk/SliceChunk.h"

namespace inet {

/**
 * This class represents data with an ordered list of consecutive chunks. It's
 * used by the Chunk API implementation internally to manage compound data.
 * User code should not directly instantiate this class.
 */
class INET_API SequenceChunk : public Chunk
{
  friend class Chunk;
  friend class SequenceChunkDescriptor;

  protected:
    /**
     * The list of chunks that make up this chunk.
     */
    std::deque<Ptr<const Chunk>> chunks;

  protected:
    int getNumChunks() const { return chunks.size(); } // only for class descriptor
    const Chunk *getChunk(int i) const { return chunks[i].get(); } // only for class descriptor

    int getElementIndex(bool isForward, int index) const { return isForward ? index : chunks.size() - index - 1; }
    const Ptr<const Chunk>& getElementChunk(const Iterator& iterator) const { return chunks[getElementIndex(iterator.isForward(), iterator.getIndex())]; }

    void doInsertChunkAtFront(const Ptr<const Chunk>& chunk);
    void doInsertSliceChunkAtFront(const Ptr<const SliceChunk>& chunk);
    void doInsertSequenceChunkAtFront(const Ptr<const SequenceChunk>& chunk);

    void doInsertChunkAtBack(const Ptr<const Chunk>& chunk);
    void doInsertSliceChunkAtBack(const Ptr<const SliceChunk>& chunk);
    void doInsertSequenceChunkAtBack(const Ptr<const SequenceChunk>& chunk);

    std::deque<Ptr<const Chunk>> dupChunks() const;

    virtual const Ptr<Chunk> peekUnchecked(PeekPredicate predicate, PeekConverter converter, const Iterator& iterator, b length, int flags) const override;

    static const Ptr<Chunk> convertChunk(const std::type_info& typeInfo, const Ptr<Chunk>& chunk, b offset, b length, int flags);

    virtual void doInsertAtFront(const Ptr<const Chunk>& chunk) override;
    virtual void doInsertAtBack(const Ptr<const Chunk>& chunk) override;

    virtual void doRemoveAtFront(b length) override;
    virtual void doRemoveAtBack(b length) override;

  public:
    /** @name Constructors, destructors and duplication related functions */
    //@{
    SequenceChunk();
    SequenceChunk(const SequenceChunk& other);
    SequenceChunk(const std::deque<Ptr<const Chunk>>& chunks);

    virtual SequenceChunk *dup() const override { return new SequenceChunk(*this); }
    virtual const Ptr<Chunk> dupShared() const override { return makeShared<SequenceChunk>(*this); }
    //@}

    virtual void forEachChild(cVisitor *v) override;

    /** @name Field accessor functions */
    const std::deque<Ptr<const Chunk>>& getChunks() const { return chunks; }
    void setChunks(const std::deque<Ptr<const Chunk>>& chunks);
    //@}

    /** @name Overridden flag functions */
    //@{
    virtual bool isComplete() const override { return !isIncomplete(); }
    virtual bool isIncomplete() const override;

    virtual bool isCorrect() const override { return !isIncorrect(); }
    virtual bool isIncorrect() const override;

    virtual bool isProperlyRepresented() const override { return !isImproperlyRepresented(); }
    virtual bool isImproperlyRepresented() const override;
    //@}

    /** @name Iteration related functions */
    //@{
    virtual void moveIterator(Iterator& iterator, b length) const override;
    virtual void seekIterator(Iterator& iterator, b offset) const override;
    //@}

    /** @name Filling with data related functions */
    //@{
    virtual bool canInsertAtFront(const Ptr<const Chunk>& chunk) const override { return true; }
    virtual bool canInsertAtBack(const Ptr<const Chunk>& chunk) const override { return true; }
    //@}

    /** @name Removing data related functions */
    //@{
    virtual bool canRemoveAtFront(b length) const override { return true; }
    virtual bool canRemoveAtBack(b length) const override { return true; }
    //@}

    /** @name Querying data related functions */
    //@{
    virtual ChunkType getChunkType() const override { return CT_SEQUENCE; }
    virtual b getChunkLength() const override;
    virtual bool isEmpty() const override { return chunks.size() != 0; }
    //@}

    virtual std::string str() const override;
};

} // namespace

#endif // #ifndef __INET_SEQUENCECHUNK_H_

