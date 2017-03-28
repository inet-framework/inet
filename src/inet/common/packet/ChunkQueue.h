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

#ifndef __INET_CHUNKQUEUE_H_
#define __INET_CHUNKQUEUE_H_

#include "inet/common/packet/chunk/BitsChunk.h"
#include "inet/common/packet/chunk/BytesChunk.h"

namespace inet {

/**
 * This class represents a first in, first out queue of chunks. It is mainly
 * useful for application and protocol buffers where the incoming chunks are
 * guaranteed to be in order.
 */
class INET_API ChunkQueue : public cNamedObject
{
  friend class ChunkQueueDescriptor;

  protected:
    bit pushedLength = bit(0);
    bit poppedLength = bit(0);
    /**
     * This chunk is always immutable to allow arbitrary peeking. Nevertheless
     * it's reused if possible to allow efficient merging with newly added chunks.
     */
    std::shared_ptr<Chunk> contents = nullptr;
    Chunk::Iterator iterator;

  protected:
    Chunk *getContents() const { return contents.get(); } // only for class descriptor

    void remove(bit length);

  public:
    /** @name Constructors, destructors and duplication related functions */
    //@{
    ChunkQueue(const char *name = nullptr, const std::shared_ptr<Chunk>& contents = nullptr);
    ChunkQueue(const ChunkQueue& other);

    virtual ChunkQueue *dup() const override { return new ChunkQueue(*this); }
    //@}

    /** @name Length querying related functions */
    //@{
    /**
     * Returns the number of available bytes.
     */
    bit getQueueLength() const { return contents == nullptr ? bit(0) : contents->getChunkLength() - iterator.getPosition(); }

    bit getPushedLength() const { return pushedLength; }

    bit getPoppedLength() const { return poppedLength; }
    //@}

    /** @name Querying data related functions */
    //@{
    std::shared_ptr<Chunk> peek(bit length = bit(-1), int flags = 0) const;

    std::shared_ptr<Chunk> peekAt(bit offset, bit length, int flags = 0) const;

    template <typename T>
    bool has(bit length = bit(-1)) const {
        return contents == nullptr ? false : contents->has<T>(iterator, length);
    }

    template <typename T>
    std::shared_ptr<T> peek(bit length = bit(-1), int flags = 0) const {
        return contents == nullptr ? nullptr : contents->peek<T>(iterator, length, flags);
    }

    template <typename T>
    std::shared_ptr<T> peekAt(bit offset, bit length = bit(-1), int flags = 0) const {
        return contents == nullptr ? nullptr : contents->peek<T>(Chunk::Iterator(true, iterator.getPosition() + offset, -1), length, flags);
    }

    /**
     * Returns all data in the queue in the current representation. The length
     * of the returned chunk is the same as the value returned by getQueueLength().
     */
    std::shared_ptr<Chunk> peek() const {
        return peekAt(bit(0), getQueueLength());
    }

    /**
     * Returns all data in the queue in the as a sequence of bits. The length
     * of the returned chunk is the same as the value returned by getQueueLength().
     */
    std::shared_ptr<BitsChunk> peekBits() const {
        return peekAt<BitsChunk>(bit(0), getQueueLength());
    }

    /**
     * Returns all data in the queue in the as a sequence of bytes. The length
     * of the returned chunk is the same as the value returned by getQueueLength().
     */
    std::shared_ptr<BytesChunk> peekBytes() const {
        return peekAt<BytesChunk>(bit(0), getQueueLength());
    }
    //@}

    /** @name Removing data related functions */
    //@{
    std::shared_ptr<Chunk> pop(bit length = bit(-1), int flags = 0);

    template <typename T>
    std::shared_ptr<T> pop(bit length = bit(-1), int flags = 0) {
        const auto& chunk = peek<T>(length, flags);
        if (chunk != nullptr)
            remove(chunk->getChunkLength());
        return chunk;
    }

    void clear();
    //@}

    /** @name Filling with data related functions */
    //@{
    void push(const std::shared_ptr<Chunk>& chunk);
    //@}

    virtual std::string str() const override { return contents == nullptr ? "" : contents->str(); }
};

inline std::ostream& operator<<(std::ostream& os, const ChunkQueue *queue) { return os << queue->str(); }

inline std::ostream& operator<<(std::ostream& os, const ChunkQueue& queue) { return os << queue.str(); }

} // namespace

#endif // #ifndef __INET_CHUNKQUEUE_H_

