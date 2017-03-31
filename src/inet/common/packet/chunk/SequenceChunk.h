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
    std::deque<std::shared_ptr<Chunk>> chunks;

  protected:
    int getNumChunks() const { return chunks.size(); } // only for class descriptor
    Chunk *getChunk(int i) const { return chunks[i].get(); } // only for class descriptor

    int getElementIndex(bool isForward, int index) const { return isForward ? index : chunks.size() - index - 1; }
    const std::shared_ptr<Chunk>& getElementChunk(const Iterator& iterator) const { return chunks[getElementIndex(iterator.isForward(), iterator.getIndex())]; }

    void doInsertToBeginning(const std::shared_ptr<Chunk>& chunk);
    void doInsertToBeginning(const std::shared_ptr<SliceChunk>& chunk);
    void doInsertToBeginning(const std::shared_ptr<SequenceChunk>& chunk);

    void doInsertToEnd(const std::shared_ptr<Chunk>& chunk);
    void doInsertToEnd(const std::shared_ptr<SliceChunk>& chunk);
    void doInsertToEnd(const std::shared_ptr<SequenceChunk>& chunk);

    std::deque<std::shared_ptr<Chunk>> dupChunks() const;

    virtual std::shared_ptr<Chunk> peekUnchecked(PeekPredicate predicate, PeekConverter converter, const Iterator& iterator, bit length, int flags) const override;

    static std::shared_ptr<Chunk> convertChunk(const std::type_info& typeInfo, const std::shared_ptr<Chunk>& chunk, bit offset, bit length, int flags);

  public:
    /** @name Constructors, destructors and duplication related functions */
    //@{
    SequenceChunk();
    SequenceChunk(const SequenceChunk& other);
    SequenceChunk(const std::deque<std::shared_ptr<Chunk>>& chunks);

    virtual SequenceChunk *dup() const override { return new SequenceChunk(*this); }
    virtual std::shared_ptr<Chunk> dupShared() const override { return std::make_shared<SequenceChunk>(*this); }
    //@}

    /** @name Field accessor functions */
    const std::deque<std::shared_ptr<Chunk>>& getChunks() const { return chunks; }
    void setChunks(const std::deque<std::shared_ptr<Chunk>>& chunks);
    //@}

    /** @name Overridden flag functions */
    //@{
    virtual bool isMutable() const override;
    virtual bool isImmutable() const override { return !isMutable(); }
    virtual void markImmutable() override;

    virtual bool isComplete() const override { return !isIncomplete(); }
    virtual bool isIncomplete() const override;

    virtual bool isCorrect() const override { return !isIncorrect(); }
    virtual bool isIncorrect() const override;

    virtual bool isProperlyRepresented() const override { return !isImproperlyRepresented(); }
    virtual bool isImproperlyRepresented() const override;
    //@}

    /** @name Iteration related functions */
    //@{
    virtual void moveIterator(Iterator& iterator, bit length) const override;
    virtual void seekIterator(Iterator& iterator, bit offset) const override;
    //@}

    /** @name Filling with data related functions */
    //@{
    virtual bool canInsertAtBeginning(const std::shared_ptr<Chunk>& chunk) override { return true; }
    virtual bool canInsertAtEnd(const std::shared_ptr<Chunk>& chunk) override { return true; }

    virtual void insertAtBeginning(const std::shared_ptr<Chunk>& chunk) override;
    virtual void insertAtEnd(const std::shared_ptr<Chunk>& chunk) override;
    //@}

    /** @name Removing data related functions */
    //@{
    virtual bool canRemoveFromBeginning(bit length) override { return true; }
    virtual bool canRemoveFromEnd(bit length) override { return true; }

    virtual void removeFromBeginning(bit length) override;
    virtual void removeFromEnd(bit length) override;
    //@}

    /** @name Querying data related functions */
    //@{
    virtual ChunkType getChunkType() const override { return CT_SEQUENCE; }
    virtual bit getChunkLength() const override;
    //@}

    virtual std::string str() const override;
};

} // namespace

#endif // #ifndef __INET_SEQUENCECHUNK_H_

