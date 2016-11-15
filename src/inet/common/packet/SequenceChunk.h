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

#include "inet/common/packet/SliceChunk.h"

namespace inet {

class SequenceChunk : public Chunk
{
  protected:
    std::vector<std::shared_ptr<Chunk>> chunks;

  protected:
    virtual const char *getSerializerClassName() const override { return "inet::SequenceChunkSerializer"; }

    int getStartIndex(const Iterator& iterator) const { return iterator.isForward() ? 0 : chunks.size() - 1; }
    int getEndIndex(const Iterator& iterator) const { return iterator.isForward() ? chunks.size() - 1 : 0; }
    int getIndexIncrement(const Iterator& iterator) const { return iterator.isForward() ? 1 : -1; }
    const std::shared_ptr<Chunk>& getElementChunk(const Iterator& iterator) const { return iterator.isForward() ? chunks[iterator.getIndex()] : chunks[chunks.size() - iterator.getIndex() - 1]; }

    std::shared_ptr<Chunk> peekWithIterator(const SequenceIterator& iterator, int64_t byteLength = -1) const;
    std::shared_ptr<Chunk> peekWithLinearSearch(const SequenceIterator& iterator, int64_t byteLength = -1) const;

    bool mergeToEnd(const std::shared_ptr<Chunk>& chunk);

    void insertToBeginning(const std::shared_ptr<SequenceChunk>& chunk);

    void insertToEnd(const std::shared_ptr<SliceChunk>& chunk);
    void insertToEnd(const std::shared_ptr<SequenceChunk>& chunk);

    std::vector<std::shared_ptr<Chunk> > dupChunks() const;

  public:
    SequenceChunk();
    SequenceChunk(const SequenceChunk& other);
    SequenceChunk(const std::vector<std::shared_ptr<Chunk>>& chunks);

    virtual SequenceChunk *dup() const override { return new SequenceChunk(*this); }
    virtual std::shared_ptr<Chunk> dupShared() const override { return std::make_shared<SequenceChunk>(*this); }

    virtual Type getChunkType() const override { return TYPE_SEQUENCE; }

    const std::vector<std::shared_ptr<Chunk>>& getChunks() const { return chunks; }

    /** @name Mutability related functions */
    //@{
    void makeImmutable();
    //@}

    /** @name Iteration related functions */
    //@{
    virtual void moveIterator(Iterator& iterator, int64_t byteLength) const override;
    virtual void seekIterator(Iterator& iterator, int64_t byteOffset) const override;
    //@}

    /** @name Filling with data related functions */
    //@{
    virtual bool insertToBeginning(const std::shared_ptr<Chunk>& chunk) override;
    virtual bool insertToEnd(const std::shared_ptr<Chunk>& chunk) override;

    void prepend(const std::shared_ptr<Chunk>& chunk, bool flatten = true);
    void append(const std::shared_ptr<Chunk>& chunk, bool flatten = true);
    //@}

    /** @name Removing data related functions */
    //@{
    virtual bool removeFromBeginning(int64_t byteLength) override;
    virtual bool removeFromEnd(int64_t byteLength) override;
    //@}

    /** @name Querying data related functions */
    //@{
    virtual int64_t getByteLength() const override;

    virtual std::shared_ptr<Chunk> peek(const Iterator& iterator, int64_t byteLength = -1) const override;

    template <typename T>
    std::shared_ptr<T> peek(const SequenceIterator& iterator, int64_t byteLength = -1) const {
        if (auto tChunk = std::dynamic_pointer_cast<T>(peekWithIterator(iterator, byteLength)))
            return tChunk;
        if (auto tChunk = std::dynamic_pointer_cast<T>(peekWithLinearSearch(iterator, byteLength)))
            return tChunk;
        return Chunk::peek<T>(iterator, byteLength);
    }
    //@}

    virtual std::string str() const override;
};

} // namespace

#endif // #ifndef __INET_SEQUENCECHUNK_H_

