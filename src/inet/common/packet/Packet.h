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
 * data used by communication protocols. Packets store their data in different
 * kind of chunks.
 *
 * Packets are initially mutable, then may become immutable (but never the
 * other way around). All chunks are immutable in an immutable packet.
 * Immutable chunks are automatically shared among immutable packets when
 * duplicating.
 *
 * A packet is conceptually divided into three parts during processing: headers,
 * data, and trailers. These parts are separated by iterators maintained by
 * the packet.
 *
 * In general, packets support the following operations:
 *  - insert to the beginning or end
 *  - remove from the beginning or end
 *  - query length and peek an arbitrary part
 *  - serialize to and deserialize from a sequence of bytes
 *  - copying to a new mutable packet
 */
class Packet : public cPacket
{
  friend class PacketDescriptor;

  protected:
    std::shared_ptr<Chunk> data;
    Chunk::Iterator headerIterator;
    Chunk::Iterator trailerIterator;

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

    /** @name Header querying related functions */
    //@{
    /**
     * Returns the current header iterator position measured in bytes from the
     * beginning of the packet. The returned value is in the range [0, getByteLength()].
     */
    int64_t getHeaderPosition() const { return headerIterator.getPosition(); }

    /**
     * Changes the current header iterator position measured in bytes from the
     * beginning of the packet. The value must be in the range [0, getByteLength()].
     */
    void setHeaderPosition(int64_t offset) { data->seekIterator(headerIterator, offset); }

    /**
     * Returns the total byte length of the packet headers processed so far.
     */
    int64_t getHeaderLength() const { return headerIterator.getPosition(); }      // TODO: rename to getHeaderLength, or rather getProcessedHeaderLength to avoid confusion?

    std::shared_ptr<Chunk> peekHeader(int64_t byteLength = -1) const;

    std::shared_ptr<Chunk> peekHeaderAt(int64_t byteOffset, int64_t byteLength) const;

    std::shared_ptr<Chunk> popHeader(int64_t byteLength = -1);

    template <typename T>
    bool hasHeader(int64_t byteLength = -1) const {
        return peekHeader<T>(byteLength) != nullptr;
    }

    template <typename T>
    std::shared_ptr<T> peekHeader(int64_t byteLength = -1) const {
        return data->peek<T>(headerIterator, byteLength);
    }

    template <typename T>
    std::shared_ptr<T> peekHeaderAt(int64_t byteOffset, int64_t byteLength = -1) const {
        return data->peek<T>(Chunk::Iterator(true, byteOffset), byteLength);
    }

    template <typename T>
    std::shared_ptr<T> popHeader(int64_t byteLength = -1) {
        const auto& chunk = peekHeader<T>(byteLength);
        if (chunk != nullptr)
            data->moveIterator(headerIterator, chunk->getByteLength());
        return chunk;
    }
    //@}

    /** @name Trailer querying related functions */
    //@{
    /**
     * Returns the current trailer iterator position measured in bytes from the
     * end of the packet. The returned value is in the range [0, getByteLength()].
     */
    int64_t getTrailerPosition() const { return trailerIterator.getPosition(); }

    /**
     * Changes the current trailer iterator position measured in bytes from the
     * end of the packet. The value must be in the range [0, getByteLength()].
     */
    void setTrailerPosition(int64_t offset) { data->seekIterator(trailerIterator, offset); }

    /**
     * Returns the total byte length of the packet trailers processed so far.
     */
    int64_t getTrailerLength() const { return trailerIterator.getPosition(); }

    std::shared_ptr<Chunk> peekTrailer(int64_t byteLength = -1) const;

    std::shared_ptr<Chunk> peekTrailerAt(int64_t byteOffset, int64_t byteLength) const;

    std::shared_ptr<Chunk> popTrailer(int64_t byteLength = -1);

    template <typename T>
    bool hasTrailer(int64_t byteLength = -1) const {
        return peekTrailer<T>(byteLength) != nullptr;
    }

    template <typename T>
    std::shared_ptr<T> peekTrailer(int64_t byteLength = -1) const {
        return data->peek<T>(trailerIterator, byteLength);
    }

    template <typename T>
    std::shared_ptr<T> peekTrailerAt(int64_t byteOffset, int64_t byteLength = -1) const {
        return data->peek<T>(Chunk::Iterator(false, byteOffset), byteLength);
    }

    template <typename T>
    std::shared_ptr<T> popTrailer(int64_t byteLength = -1) {
        const auto& chunk = peekTrailer<T>(byteLength);
        if (chunk != nullptr)
            data->moveIterator(trailerIterator, -chunk->getByteLength());
        return chunk;
    }
    //@}

    /** @name Data querying related functions */
    //@{
    /**
     * Returns the current data position measured in bytes from the beginning
     * of the packet. The returned value is in the range [0, getByteLength()].
     */
    int64_t getDataPosition() const { return headerIterator.getPosition(); }

    /**
     * Returns the current data size measured in bytes. The returned value is
     * in the range [0, getByteLength()].
     */
    int64_t getDataLength() const { return getByteLength() - headerIterator.getPosition() - trailerIterator.getPosition(); }

    std::shared_ptr<Chunk> peekData(int64_t byteLength = -1) const;

    std::shared_ptr<Chunk> peekDataAt(int64_t byteOffset = 0, int64_t byteLength = -1) const;

    template <typename T>
    bool hasData(int64_t byteLength = -1) const {
        return peekData<T>(byteLength) != nullptr;
    }

    template <typename T>
    std::shared_ptr<T> peekData(int64_t byteLength = -1) const {
        return data->peek<T>(Chunk::Iterator(true, getDataPosition()), byteLength);
    }

    template <typename T>
    std::shared_ptr<T> peekDataAt(int64_t byteOffset = 0, int64_t byteLength = -1) const {
        return data->peek<T>(Chunk::Iterator(true, getDataPosition() + byteOffset), byteLength);
    }
    //@}

    /** @name Querying related functions */
    //@{
    std::shared_ptr<Chunk> peek(int64_t byteLength = -1) const;

    std::shared_ptr<Chunk> peekAt(int64_t byteOffset = 0, int64_t byteLength = -1) const;

    template <typename T>
    bool has(int64_t byteLength = -1) const {
        return peek<T>(byteLength) != nullptr;
    }

    template <typename T>
    std::shared_ptr<T> peek(int64_t byteLength = -1) const {
        return data->peek<T>(Chunk::Iterator(true, 0), byteLength);
    }

    template <typename T>
    std::shared_ptr<T> peekAt(int64_t byteOffset = 0, int64_t byteLength = -1) const {
        return data->peek<T>(Chunk::Iterator(true, byteOffset), byteLength);
    }
    //@}

    /** @name Filling with data related functions */
    //@{
    void prepend(const std::shared_ptr<Chunk>& chunk, bool flatten = true);
    void prepend(Packet *packet, bool flatten = true);

    void append(const std::shared_ptr<Chunk>& chunk, bool flatten = true);
    void append(Packet *packet, bool flatten = true);
    //@}

    virtual int64_t getBitLength() const override { return data->getByteLength() << 3; }

    virtual std::string str() const override { return data->str(); }
};

inline std::ostream& operator<<(std::ostream& os, const Packet *packet) { return os << packet->str(); }

inline std::ostream& operator<<(std::ostream& os, const Packet& packet) { return os << packet.str(); }

} // namespace

#endif // #ifndef __INET_PACKET_H_

