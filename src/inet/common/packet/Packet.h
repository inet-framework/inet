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

#ifndef __INET_PACKET_H_
#define __INET_PACKET_H_

#include "inet/common/packet/Chunk.h"

namespace inet {

/**
 * This class represents network packets, datagrams, frames and other kinds of
 * data used by communication protocols. The underlying data structure supports
 * efficient construction, duplication, encapsulation, aggregation, fragmentation
 * and serialization. The data structure also supports dual representation by
 * default: data can be accessed as raw bytes and also as field based classes.
 * Internally, packets store their data in different kind of chunks. See the
 * Chunk class and its subclasses for details.
 *
 * Packets are initially mutable, then may become immutable (but never the
 * other way around). All chunks are immutable in an immutable packet.
 * Immutable chunks are automatically shared among immutable packets when
 * duplicating.
 *
 * Packets are conceptually divided into three parts during processing: headers,
 * data, and trailers. These parts are separated by iterators which are stored
 * within the packet. During packet processing, as the packet is passed through
 * the protocol layers at the receiver, headers and trailers are popped from the
 * beginning and the end. This effectively reduces the remaining unprocessed
 * data part, but it doesn't affect the data stored in the chunks.
 *
 * In general, packets support the following operations:
 *  - insert to the beginning or end
 *  - remove from the beginning or end
 *  - query length and peek an arbitrary part
 *  - serialize to and deserialize from a sequence of bytes
 *  - copy to a new mutable packet
 *  - convert to a human readable string
 */
class Packet : public cPacket
{
  friend class PacketDescriptor;

  protected:
    std::shared_ptr<Chunk> data;
    Chunk::ForwardIterator headerIterator;
    Chunk::BackwardIterator trailerIterator;

  protected:
    int getNumChunks() const;
    Chunk *getChunk(int i) const;

  public:
    explicit Packet(const char *name = nullptr, short kind = 0);
    Packet(const Packet& other);
    Packet(const std::shared_ptr<Chunk>& data, const char *name = nullptr, short kind = 0);

    virtual Packet *dup() const override { return new Packet(*this); }

    /** @name Mutability related functions */
    //@{
    bool isImmutable() const { return data != nullptr && data->isImmutable(); }
    bool isMutable() const { return data == nullptr || data->isMutable(); }
    void assertMutable() const { assert(isMutable()); }
    void assertImmutable() const { assert(isImmutable()); }
    void makeImmutable() { data->makeImmutable(); }
    //@}

    /** @name Length querying related functions */
    //@{
    /**
     * Returns the packet length in bytes ignoring header and trailer iterators.
     * The returned value is in the range [0, +infinity).
     */
    virtual int64_t getPacketLength() const { return data->getChunkLength(); }

    /**
     * Returns the length in bits between the header and trailer iterators.
     * The returned value is in the range [0, +infinity).
     */
    virtual int64_t getBitLength() const override { return getDataLength() << 3; }
    //@}

    /** @name Header querying related functions */
    //@{
    /**
     * Returns the current header iterator position measured in bytes from the
     * beginning of the packet. The returned value is in the range [0, getPacketLength()].
     */
    int64_t getHeaderPosition() const { return headerIterator.getPosition(); }

    /**
     * Changes the current header iterator position measured in bytes from the
     * beginning of the packet. The value must be in the range [0, getPacketLength()].
     */
    void setHeaderPosition(int64_t offset) { data->seekIterator(headerIterator, offset); }

    /**
     * Returns the total byte length of the packet headers processed so far.
     */
    int64_t getHeaderLength() const { return headerIterator.getPosition(); }      // TODO: rename getProcessedHeaderLength to avoid confusion?

    std::shared_ptr<Chunk> peekHeader(int64_t length = -1) const;

    std::shared_ptr<Chunk> peekHeaderAt(int64_t offset, int64_t length) const;

    std::shared_ptr<Chunk> popHeader(int64_t length = -1);

    template <typename T>
    bool hasHeader(int64_t length = -1) const {
        return peekHeader<T>(length) != nullptr;
    }

    template <typename T>
    std::shared_ptr<T> peekHeader(int64_t length = -1) const {
        return data->peek<T>(headerIterator, length);
    }

    template <typename T>
    std::shared_ptr<T> peekHeaderAt(int64_t offset, int64_t length = -1) const {
        return data->peek<T>(Chunk::Iterator(true, offset), length);
    }

    template <typename T>
    std::shared_ptr<T> popHeader(int64_t length = -1) {
        const auto& chunk = peekHeader<T>(length);
        if (chunk != nullptr)
            data->moveIterator(headerIterator, chunk->getChunkLength());
        return chunk;
    }
    //@}

    /** @name Trailer querying related functions */
    //@{
    /**
     * Returns the current trailer iterator position measured in bytes from the
     * end of the packet. The returned value is in the range [0, getPacketLength()].
     */
    int64_t getTrailerPosition() const { return trailerIterator.getPosition(); }

    /**
     * Changes the current trailer iterator position measured in bytes from the
     * end of the packet. The value must be in the range [0, getPacketLength()].
     */
    void setTrailerPosition(int64_t offset) { data->seekIterator(trailerIterator, offset); }

    /**
     * Returns the total byte length of the packet trailers processed so far.
     */
    int64_t getTrailerLength() const { return trailerIterator.getPosition(); }

    std::shared_ptr<Chunk> peekTrailer(int64_t length = -1) const;

    std::shared_ptr<Chunk> peekTrailerAt(int64_t offset, int64_t length) const;

    std::shared_ptr<Chunk> popTrailer(int64_t length = -1);

    template <typename T>
    bool hasTrailer(int64_t length = -1) const {
        return peekTrailer<T>(length) != nullptr;
    }

    template <typename T>
    std::shared_ptr<T> peekTrailer(int64_t length = -1) const {
        return data->peek<T>(trailerIterator, length);
    }

    template <typename T>
    std::shared_ptr<T> peekTrailerAt(int64_t offset, int64_t length = -1) const {
        return data->peek<T>(Chunk::Iterator(false, offset), length);
    }

    template <typename T>
    std::shared_ptr<T> popTrailer(int64_t length = -1) {
        const auto& chunk = peekTrailer<T>(length);
        if (chunk != nullptr)
            data->moveIterator(trailerIterator, chunk->getChunkLength());
        return chunk;
    }
    //@}

    /** @name Data querying related functions */
    //@{
    /**
     * Returns the current data position measured in bytes from the beginning
     * of the packet. The returned value is in the range [0, getPacketLength()].
     */
    int64_t getDataPosition() const { return headerIterator.getPosition(); }

    /**
     * Returns the current data size measured in bytes. The returned value is
     * in the range [0, getPacketLength()].
     */
    int64_t getDataLength() const { return getPacketLength() - headerIterator.getPosition() - trailerIterator.getPosition(); }

    std::shared_ptr<Chunk> peekData(int64_t length = -1) const;

    std::shared_ptr<Chunk> peekDataAt(int64_t offset = 0, int64_t length = -1) const;

    template <typename T>
    bool hasData(int64_t length = -1) const {
        return peekData<T>(length) != nullptr;
    }

    template <typename T>
    std::shared_ptr<T> peekData(int64_t length = -1) const {
        return data->peek<T>(Chunk::Iterator(true, getDataPosition()), length);
    }

    template <typename T>
    std::shared_ptr<T> peekDataAt(int64_t offset = 0, int64_t length = -1) const {
        return data->peek<T>(Chunk::Iterator(true, getDataPosition() + offset), length);
    }
    //@}

    /** @name Querying related functions */
    //@{
    std::shared_ptr<Chunk> peek(int64_t length = -1) const;

    std::shared_ptr<Chunk> peekAt(int64_t offset = 0, int64_t length = -1) const;

    template <typename T>
    bool has(int64_t length = -1) const {
        return peek<T>(length) != nullptr;
    }

    template <typename T>
    std::shared_ptr<T> peek(int64_t length = -1) const {
        return data->peek<T>(Chunk::Iterator(true, 0), length);
    }

    template <typename T>
    std::shared_ptr<T> peekAt(int64_t offset = 0, int64_t length = -1) const {
        return data->peek<T>(Chunk::Iterator(true, offset), length);
    }
    //@}

    /** @name Filling with data related functions */
    //@{
    void prepend(const std::shared_ptr<Chunk>& chunk);
    void append(const std::shared_ptr<Chunk>& chunk);
    //@}

    /** @name Removing data related functions */
    //@{
    void removeFromBeginning(int64_t length);
    void removeFromEnd(int64_t length);
    //@}

    virtual std::string str() const override { return data->str(); }
};

inline std::ostream& operator<<(std::ostream& os, const Packet *packet) { return os << packet->str(); }

inline std::ostream& operator<<(std::ostream& os, const Packet& packet) { return os << packet.str(); }

} // namespace

#endif // #ifndef __INET_PACKET_H_

