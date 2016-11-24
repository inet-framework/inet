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
#include "inet/common/packet/SliceChunk.h"

namespace inet {

/**
 * This class represents data with an ordered list of consecutive chunks. It's
 * used by the Chunk API implementation internally to manage compound data.
 * User code should not directly instantiate this class.
 */
class SequenceChunk : public Chunk
{
  protected:
    /**
     * The list of chunks that make up this chunk.
     */
    std::deque<std::shared_ptr<Chunk>> chunks;

  protected:
    virtual const char *getSerializerClassName() const override { return "inet::SequenceChunkSerializer"; }

    int getStartIndex(const Iterator& iterator) const { return iterator.isForward() ? 0 : chunks.size() - 1; }
    int getEndIndex(const Iterator& iterator) const { return iterator.isForward() ? chunks.size() - 1 : 0; }
    int getIndexIncrement(const Iterator& iterator) const { return iterator.isForward() ? 1 : -1; }
    const std::shared_ptr<Chunk>& getElementChunk(const Iterator& iterator) const { return iterator.isForward() ? chunks[iterator.getIndex()] : chunks[chunks.size() - iterator.getIndex() - 1]; }

    virtual std::shared_ptr<Chunk> peekWithIterator(const Iterator& iterator, int64_t length = -1) const override;
    virtual std::shared_ptr<Chunk> peekWithLinearSearch(const Iterator& iterator, int64_t length = -1) const override;

    bool mergeToBeginning(const std::shared_ptr<Chunk>& chunk);
    bool mergeToEnd(const std::shared_ptr<Chunk>& chunk);

    void doInsertToBeginning(const std::shared_ptr<Chunk>& chunk);
    void doInsertToBeginning(const std::shared_ptr<SliceChunk>& chunk);
    void doInsertToBeginning(const std::shared_ptr<SequenceChunk>& chunk);

    void doInsertToEnd(const std::shared_ptr<Chunk>& chunk);
    void doInsertToEnd(const std::shared_ptr<SliceChunk>& chunk);
    void doInsertToEnd(const std::shared_ptr<SequenceChunk>& chunk);

    std::deque<std::shared_ptr<Chunk> > dupChunks() const;

  protected:
    static std::shared_ptr<Chunk> createChunk(const std::type_info& typeInfo, const std::shared_ptr<Chunk>& chunk, int64_t offset = -1, int64_t length = -1);

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

    /** @name Mutability related functions */
    //@{
    virtual void makeImmutable() override;
    //@}

    /** @name Iteration related functions */
    //@{
    virtual void moveIterator(Iterator& iterator, int64_t length) const override;
    virtual void seekIterator(Iterator& iterator, int64_t offset) const override;
    //@}

    /** @name Filling with data related functions */
    //@{
    virtual bool insertToBeginning(const std::shared_ptr<Chunk>& chunk) override;
    virtual bool insertToEnd(const std::shared_ptr<Chunk>& chunk) override;
    //@}

    /** @name Removing data related functions */
    //@{
    virtual bool removeFromBeginning(int64_t length) override;
    virtual bool removeFromEnd(int64_t length) override;
    //@}

    /** @name Querying data related functions */
    //@{
    virtual Type getChunkType() const override { return TYPE_SEQUENCE; }

    virtual int64_t getChunkLength() const override;

    virtual std::shared_ptr<Chunk> peek(const Iterator& iterator, int64_t length) const override;
    //@}

    virtual std::string str() const override;
};

} // namespace

#endif // #ifndef __INET_SEQUENCECHUNK_H_

