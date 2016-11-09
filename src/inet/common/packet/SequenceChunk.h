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
  public:
    class SequenceIterator : public Iterator
    {
      protected:
        const std::shared_ptr<const SequenceChunk> chunk = nullptr;
        int index = -1;

      public:
        SequenceIterator(const std::shared_ptr<const SequenceChunk>& chunk, int index = 0, int64_t position = 0);
        SequenceIterator(const SequenceIterator& other);

        int getIndex() const { return index; }

        void move(int64_t byteLength);
        void seek(int64_t byteOffset);

        virtual int getStartIndex() const = 0;
        virtual int getEndIndex() const = 0;
        virtual int getIndexIncrement() const = 0;
        virtual const std::shared_ptr<Chunk>& getElementChunk() const = 0;
    };

    class ForwardIterator : public SequenceIterator
    {
      public:
        ForwardIterator(const std::shared_ptr<const SequenceChunk>& chunk, int index = 0, int64_t position = 0);
        ForwardIterator(const ForwardIterator& other);

        virtual int getStartIndex() const override { return 0; }
        virtual int getEndIndex() const override { return chunk->chunks.size() - 1; }
        virtual int getIndexIncrement() const override { return 1; }
        virtual const std::shared_ptr<Chunk>& getElementChunk() const override { return chunk->chunks[index]; }
    };

    class BackwardIterator : public SequenceIterator
    {
      public:
        BackwardIterator(const std::shared_ptr<const SequenceChunk>& chunk, int index = 0, int64_t position = 0);
        BackwardIterator(const ForwardIterator& other);

        virtual int getStartIndex() const override { return chunk->chunks.size() - 1; }
        virtual int getEndIndex() const override { return 0; }
        virtual int getIndexIncrement() const override { return -1; }
        virtual const std::shared_ptr<Chunk>& getElementChunk() const override { return chunk->chunks[chunk->chunks.size() - index - 1]; }
    };

  protected:
    std::vector<std::shared_ptr<Chunk>> chunks;

  protected:
    std::shared_ptr<Chunk> peekWithIterator(const SequenceIterator& iterator, int64_t byteLength = -1) const;
    std::shared_ptr<Chunk> peekWithLinearSearch(const SequenceIterator& iterator, int64_t byteLength = -1) const;

    void prependChunk(const std::shared_ptr<Chunk>& chunk);
    void prependChunk(const std::shared_ptr<SequenceChunk>& chunk);

    void appendChunk(const std::shared_ptr<Chunk>& chunk);
    void appendChunk(const std::shared_ptr<SliceChunk>& chunk);
    void appendChunk(const std::shared_ptr<SequenceChunk>& chunk);

    std::vector<std::shared_ptr<Chunk>> cloneChunks() const {
        std::vector<std::shared_ptr<Chunk>> clones;
        for (auto& chunk : chunks)
            // TODO: is this the right way to do it?
            clones.push_back(chunk->isImmutable() ? chunk : std::shared_ptr<Chunk>(static_cast<Chunk *>(chunk->dup())));
        return clones;
    }

  public:
    SequenceChunk() { }
    SequenceChunk(const SequenceChunk& other);

    virtual SequenceChunk *dup() const override { return new SequenceChunk(*this); }

    const std::vector<std::shared_ptr<Chunk>>& getChunks() const { return chunks; }

    /** @name Mutability related functions */
    //@{
    void makeImmutable();
    //@}

    /** @name Iteration related functions */
    //@{
    ForwardIterator createForwardIterator() const { return ForwardIterator(std::static_pointer_cast<const SequenceChunk>(shared_from_this())); }
    BackwardIterator createBackwardIterator() const { return BackwardIterator(std::static_pointer_cast<const SequenceChunk>(shared_from_this())); }
    //@}

    /** @name Querying data related functions */
    //@{
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

    /** @name Filling with data related functions */
    //@{
    void prepend(const std::shared_ptr<Chunk>& chunk, bool flatten = true);
    void append(const std::shared_ptr<Chunk>& chunk, bool flatten = true);
    //@}

    /** @name Removing data related functions */
    //@{
    void removeBeginning(int64_t byteLength);
    void removeEnd(int64_t byteLength);
    //@}

    virtual int64_t getByteLength() const override;

    virtual const char *getSerializerClassName() const override { return "inet::SequenceChunkSerializer"; }

    virtual std::string str() const override;

    friend class Packet;
    friend class PacketDescriptor;
};

} // namespace

#endif // #ifndef __INET_SEQUENCECHUNK_H_

