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
// TODO: consider turning some assert into if/throw, consider potential performance penalty, make it optional with compile time macro?
class INET_API Packet : public cPacket
{
  friend class PacketDescriptor;

  protected:
    /**
     * While this chunk is mutable, it allows efficient merging with newly added chunks.
     */
    std::shared_ptr<Chunk> contents;
    Chunk::ForwardIterator headerIterator;
    Chunk::BackwardIterator trailerIterator;

  protected:
    int getNumChunks() const;
    Chunk *getChunk(int i) const;

    void makeContentsMutable() {
        if (contents.use_count() <= 2)
            contents->markMutableIfExclusivelyOwned();
        else
            contents = contents->dupShared();
    }

  public:
    explicit Packet(const char *name = nullptr, short kind = 0);
    Packet(const char *name, const std::shared_ptr<Chunk>& contents);
    Packet(const Packet& other);

    virtual Packet *dup() const override { return new Packet(*this); }

    /** @name Mutability related functions */
    //@{
    bool isContentsImmutable() const { return contents != nullptr && contents->isImmutable(); }
    bool isContentsMutable() const { return contents == nullptr || contents->isMutable(); }
    void assertContentsMutable() const { assert(isContentsMutable()); }
    void assertContentsImmutable() const { assert(isContentsImmutable()); }
    void markContentsImmutable() { contents->markImmutable(); }
    //@}

    /** @name Length querying related functions */
    //@{
    /**
     * Returns the packet length in bytes ignoring header and trailer iterators.
     * The returned value is in the range [0, +infinity).
     */
    virtual int64_t getPacketLength() const { return contents == nullptr ? 0 : contents->getChunkLength(); }

    /**
     * Returns the length in bits between the header and trailer iterators.
     * The returned value is in the range [0, +infinity).
     */
    virtual int64_t getBitLength() const override { return getDataLength() << 3; }
    //@}

    /** @name Unsupported cPacket functions */
    //@{
    virtual void encapsulate(cPacket *packet) override { throw cRuntimeError("Invalid operation"); }

    virtual cPacket *decapsulate() override { throw cRuntimeError("Invalid operation"); }
    //@}

    /** @name Header querying related functions */
    //@{
    /**
     * Returns the header pop offset measured in bytes from the beginning of
     * the packet. The returned value is in the range [0, getPacketLength()].
     */
    int64_t getHeaderPopOffset() const { return headerIterator.getPosition(); }

    /**
     * Changes the header pop offset measured in bytes from the beginning of
     * the packet. The value must be in the range [0, getPacketLength()].
     */
    void setHeaderPopOffset(int64_t offset);

    /**
     * Returns the total byte count of popped packet headers.
     */
    int64_t getHeaderPoppedByteCount() const { return headerIterator.getPosition(); }

    std::shared_ptr<Chunk> peekHeader(int64_t length = -1) const;

    std::shared_ptr<Chunk> popHeader(int64_t length = -1);

    void pushHeader(const std::shared_ptr<Chunk>& chunk);

    template <typename T>
    bool hasHeader(int64_t length = -1) const {
        assert(-1 <= length && length <= getDataLength());
        return contents == nullptr ? false : contents->has<T>(headerIterator, length);
    }

    template <typename T>
    std::shared_ptr<T> peekHeader(int64_t length = -1) const {
        assert(-1 <= length && length <= getDataLength());
        return contents == nullptr ? nullptr : contents->peek<T>(headerIterator, length);
    }

    template <typename T>
    std::shared_ptr<T> popHeader(int64_t length = -1) {
        assert(-1 <= length && length <= getDataLength());
        const auto& chunk = peekHeader<T>(length);
        if (chunk != nullptr)
            contents->moveIterator(headerIterator, chunk->getChunkLength());
        return chunk;
    }
    //@}

    /** @name Trailer querying related functions */
    //@{
    /**
     * Returns the trailer pop offset measured in bytes from the beginning of
     * the packet. The returned value is in the range [0, getPacketLength()].
     */
    int64_t getTrailerPopOffset() const { return getPacketLength() - trailerIterator.getPosition(); }

    /**
     * Changes the trailer pop offset measured in bytes from the beginning of
     * the packet. The value must be in the range [0, getPacketLength()].
     */
    void setTrailerPopOffset(int64_t offset);

    /**
     * Returns the total byte count of popped packet trailers.
     */
    int64_t getTrailerPoppedByteCount() const { return trailerIterator.getPosition(); }

    std::shared_ptr<Chunk> peekTrailer(int64_t length = -1) const;

    std::shared_ptr<Chunk> popTrailer(int64_t length = -1);

    void pushTrailer(const std::shared_ptr<Chunk>& chunk);

    template <typename T>
    bool hasTrailer(int64_t length = -1) const {
        assert(-1 <= length && length <= getDataLength());
        return contents == nullptr ? nullptr : contents->has<T>(trailerIterator, length);
    }

    template <typename T>
    std::shared_ptr<T> peekTrailer(int64_t length = -1) const {
        assert(-1 <= length && length <= getDataLength());
        return contents == nullptr ? nullptr : contents->peek<T>(trailerIterator, length);
    }

    template <typename T>
    std::shared_ptr<T> popTrailer(int64_t length = -1) {
        assert(-1 <= length && length <= getDataLength());
        const auto& chunk = peekTrailer<T>(length);
        if (chunk != nullptr)
            contents->moveIterator(trailerIterator, chunk->getChunkLength());
        return chunk;
    }
    //@}

    /** @name Data querying related functions */
    //@{
    /**
     * Returns the current data size measured in bytes. The returned value is
     * in the range [0, getPacketLength()].
     */
    int64_t getDataLength() const { return getPacketLength() - headerIterator.getPosition() - trailerIterator.getPosition(); }

    std::shared_ptr<Chunk> peekDataAt(int64_t offset, int64_t length = -1) const;

    template <typename T>
    bool hasDataAt(int64_t offset, int64_t length = -1) const {
        return peekDataAt<T>(offset, length) != nullptr;
    }

    template <typename T>
    std::shared_ptr<T> peekDataAt(int64_t offset, int64_t length = -1) const {
        return contents == nullptr ? nullptr : contents->peek<T>(Chunk::Iterator(true, headerIterator.getPosition() + offset, -1), length);
    }
    //@}

    /** @name Querying related functions */
    //@{
    std::shared_ptr<Chunk> peekAt(int64_t offset, int64_t length = -1) const;

    template <typename T>
    bool hasAt(int64_t offset, int64_t length = -1) const {
        assert(0 <= offset && offset <= getPacketLength());
        assert(-1 <= length && length <= getPacketLength());
        return peekAt<T>(offset, length) != nullptr;
    }

    template <typename T>
    std::shared_ptr<T> peekAt(int64_t offset, int64_t length = -1) const {
        assert(0 <= offset && offset <= getPacketLength());
        assert(-1 <= length && length <= getPacketLength());
        return contents == nullptr ? nullptr : contents->peek<T>(Chunk::Iterator(true, offset, -1), length);
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

    virtual std::string str() const override { return contents == nullptr ? "" : contents->str(); }
};

inline std::ostream& operator<<(std::ostream& os, const Packet *packet) { return os << packet->str(); }

inline std::ostream& operator<<(std::ostream& os, const Packet& packet) { return os << packet.str(); }

} // namespace

#endif // #ifndef __INET_PACKET_H_

