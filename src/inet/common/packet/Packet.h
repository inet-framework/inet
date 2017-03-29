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

#include "inet/common/packet/chunk/BitsChunk.h"
#include "inet/common/packet/chunk/BytesChunk.h"

namespace inet {

/**
 * This class represents network packets, datagrams, frames and other kinds of
 * data used by communication protocols. The underlying data structure supports
 * efficient construction, duplication, encapsulation, aggregation, fragmentation
 * and serialization. The data structure also supports dual representation by
 * default: data can be accessed as raw bytes and also as field based classes.
 *
 * Internally, packet stores the data in different kind of chunks. See the
 * Chunk class and its subclasses for details. All chunks are immutable in a
 * packet. Chunks are automatically merged as they are pushed into a packet,
 * and they are also shared among packets when duplicating.
 *
 * Packets are conceptually divided into three parts during processing: headers,
 * data, and trailers. These parts are separated by iterators which are stored
 * within the packet. During packet processing, as the packet is passed through
 * the protocol layers at the receiver, headers and trailers are popped from the
 * beginning and the end. This effectively reduces the remaining unprocessed
 * data part, but it doesn't affect the data stored in the chunks. Popping
 * headers and trailers is an efficient operation, because it doesn't require
 * copying or changing chunks, only updating the header and trailer iterators.
 *
 * In general, this class supports the following operations:
 *  - insert to the beginning or end
 *  - remove from the beginning or end
 *  - query the length of the whole, header, data, and trailer parts
 *  - peek, pop or remove an arbitrary chunk from the parts
 *  - serialize to and deserialize from a sequence of bits or bytes
 *  - copy to a new packet
 *  - convert to a human readable string
 */
class INET_API Packet : public cPacket
{
  friend class PacketDescriptor;

  protected:
    /**
     * This chunk is always immutable to allow arbitrary peeking. Nevertheless
     * it's reused if possible to allow efficient merging with newly added chunks.
     */
    std::shared_ptr<Chunk> contents;
    Chunk::ForwardIterator headerIterator;
    Chunk::BackwardIterator trailerIterator;

  protected:
    Chunk *getContents() const { return contents.get(); } // only for class descriptor

    template <typename T>
    std::shared_ptr<T> makeExclusivelyOwnedMutableChunk(const std::shared_ptr<T>& chunk) const {
        if (chunk.use_count() == 1) {
            chunk->markMutableIfExclusivelyOwned();
            return chunk;
        }
        else
            return std::static_pointer_cast<T>(chunk->dupShared());
    }

  public:
    explicit Packet(const char *name = nullptr, short kind = 0);
    Packet(const char *name, const std::shared_ptr<Chunk>& contents);
    Packet(const Packet& other);

    virtual Packet *dup() const override { return new Packet(*this); }

    /** @name Length querying related functions */
    //@{
    /**
     * Returns the total packet length ignoring header and trailer iterators.
     * The returned value is in the range [0, +infinity).
     */
    bit getTotalLength() const { return contents == nullptr ? bit(0) : contents->getChunkLength(); }

    /**
     * Returns the length in bits between the header and trailer iterators.
     * The returned value is in the range [0, +infinity).
     */
    virtual int64_t getBitLength() const override { return bit(getDataLength()).get(); }

    virtual void setBitLength(int64_t value) override { throw cRuntimeError("Invalid operation"); }
    //@}

    /** @name Other overridden cPacket functions */
    //@{
    virtual bool hasBitError() const override { return cPacket::hasBitError() || contents->isIncorrect(); }
    //@}

    /** @name Unsupported cPacket functions */
    //@{
    virtual void encapsulate(cPacket *packet) override { throw cRuntimeError("Invalid operation"); }

    virtual cPacket *decapsulate() override { throw cRuntimeError("Invalid operation"); }

    virtual cPacket *getEncapsulatedPacket() const override { return nullptr; }
    //@}

    /** @name Header querying related functions */
    //@{
    /**
     * Returns the header pop offset measured from the beginning of the packet.
     * The returned value is in the range [0, getTotalLength()].
     */
    bit getHeaderPopOffset() const { return headerIterator.getPosition(); }

    /**
     * Changes the header pop offset measured from the beginning of the packet.
     * The value must be in the range [0, getTotalLength()].
     */
    void setHeaderPopOffset(bit offset);

    /**
     * Returns the total length of popped headers.
     */
    bit getHeaderPoppedLength() const { return headerIterator.getPosition(); }

    /**
     * Returns the designated header as an immutable chunk in its current
     * representation. If the length is unspecified, then the length of the
     * result is chosen according to the internal representation.
     */
    std::shared_ptr<Chunk> peekHeader(bit length = bit(-1), int flags = 0) const;

    /**
     * Pops the designated header and returns it as an immutable chunk in its
     * current representation. If the length is unspecified, then the length of
     * the result is chosen according to the internal representation.
     */
    std::shared_ptr<Chunk> popHeader(bit length = bit(-1), int flags = 0);

    /**
     * Removes the designated header and returns it as a mutable chunk in its
     * current representation. If the length is unspecified, then the length of
     * the result is chosen according to the internal representation.
     */
    std::shared_ptr<Chunk> removeHeader(bit length = bit(-1), int flags = 0);

    /**
     * Pushes the provided header at the beginning of the packet. The header
     * must be immutable and the header pop offset must point to the beginning
     * of the packet before calling this function.
     */
    void pushHeader(const std::shared_ptr<Chunk>& chunk);

    /**
     * Pushes the provided header at the beginning of the packet. The pushed
     * header is automatically marked immutable. The header pop offset must
     * point to the beginning of the packet before calling this function.
     */
    void insertHeader(const std::shared_ptr<Chunk>& chunk);

    /**
     * Returns true if the designated header is available in the requested
     * representation. If the length is unspecified, then the length of the
     * result is chosen according to the internal representation.
     */
    template <typename T>
    bool hasHeader(bit length = bit(-1)) const {
        assert(bit(-1) <= length && length <= getDataLength());
        return contents == nullptr ? false : contents->has<T>(headerIterator, length);
    }

    /**
     * Returns the designated header as an immutable chunk in the requested
     * representation. If the length is unspecified, then the length of the
     * result is chosen according to the internal representation.
     */
    template <typename T>
    std::shared_ptr<T> peekHeader(bit length = bit(-1), int flags = 0) const {
        assert(bit(-1) <= length && length <= getDataLength());
        return contents == nullptr ? nullptr : contents->peek<T>(headerIterator, length, flags);
    }

    /**
     * Pops the designated header and returns it as an immutable chunk in the
     * requested representation. If the length is unspecified, then the length
     * of the result is chosen according to the internal representation.
     */
    template <typename T>
    std::shared_ptr<T> popHeader(bit length = bit(-1), int flags = 0) {
        assert(bit(-1) <= length && length <= getDataLength());
        const auto& chunk = peekHeader<T>(length, flags);
        if (chunk != nullptr)
            contents->moveIterator(headerIterator, chunk->getChunkLength());
        return chunk;
    }

    /**
     * Removes the designated header and returns it as a mutable chunk in the
     * requested representation. If the length is unspecified, then the length
     * of the result is chosen according to the internal representation.
     */
    template <typename T>
    std::shared_ptr<T> removeHeader(bit length = bit(-1), int flags = 0) {
        const auto& chunk = popHeader<T>(length, flags);
        removePoppedHeaders();
        return makeExclusivelyOwnedMutableChunk(chunk);
    }
    //@}

    /** @name Trailer querying related functions */
    //@{
    /**
     * Returns the trailer pop offset measured from the beginning of the packet.
     * The returned value is in the range [0, getTotalLength()].
     */
    bit getTrailerPopOffset() const { return getTotalLength() - trailerIterator.getPosition(); }

    /**
     * Changes the trailer pop offset measured from the beginning of the packet.
     * The value must be in the range [0, getTotalLength()].
     */
    void setTrailerPopOffset(bit offset);

    /**
     * Returns the total length of popped trailers.
     */
    bit getTrailerPoppedLength() const { return trailerIterator.getPosition(); }

    /**
     * Returns the designated trailer as an immutable chunk in its current
     * representation. If the length is unspecified, then the length of the
     * result is chosen according to the internal representation.
     */
    std::shared_ptr<Chunk> peekTrailer(bit length = bit(-1), int flags = 0) const;

    /**
     * Pops the designated trailer and returns it as an immutable chunk in its
     * current representation. If the length is unspecified, then the length of
     * the result is chosen according to the internal representation.
     */
    std::shared_ptr<Chunk> popTrailer(bit length = bit(-1), int flags = 0);

    /**
     * Removes the designated trailer and returns it as a mutable chunk in its
     * current representation. If the length is unspecified, then the length of
     * the result is chosen according to the internal representation.
     */
    std::shared_ptr<Chunk> removeTrailer(bit length = bit(-1), int flags = 0);

    /**
     * Pushes the provided trailer at the end of the packet. The trailer must be
     * immutable and the trailer pop offset must point to the end of the packet
     * before calling this function.
     */
    void pushTrailer(const std::shared_ptr<Chunk>& chunk);

    /**
     * Pushes the provided trailer at the end of the packet. The pushed trailer
     * is automatically marked immutable. The trailer pop offset must point to
     * the end of the packet before calling this function.
     */
    void insertTrailer(const std::shared_ptr<Chunk>& chunk);

    /**
     * Returns true if the designated trailer is available in the requested
     * representation. If the length is unspecified, then the length of the
     * result is chosen according to the internal representation.
     */
    template <typename T>
    bool hasTrailer(bit length = bit(-1)) const {
        assert(bit(-1) <= length && length <= getDataLength());
        return contents == nullptr ? nullptr : contents->has<T>(trailerIterator, length);
    }

    /**
     * Returns the designated trailer as an immutable chunk in the requested
     * representation. If the length is unspecified, then the length of the
     * result is chosen according to the internal representation.
     */
    template <typename T>
    std::shared_ptr<T> peekTrailer(bit length = bit(-1), int flags = 0) const {
        assert(bit(-1) <= length && length <= getDataLength());
        return contents == nullptr ? nullptr : contents->peek<T>(trailerIterator, length, flags);
    }

    /**
     * Pops the designated trailer and returns it as an immutable chunk in the
     * requested representation. If the length is unspecified, then the length
     * of the result is chosen according to the internal representation.
     */
    template <typename T>
    std::shared_ptr<T> popTrailer(bit length = bit(-1), int flags = 0) {
        assert(bit(-1) <= length && length <= getDataLength());
        const auto& chunk = peekTrailer<T>(length, flags);
        if (chunk != nullptr)
            contents->moveIterator(trailerIterator, chunk->getChunkLength());
        return chunk;
    }

    /**
     * Removes the designated trailer and returns it as a mutable chunk in the
     * requested representation. If the length is unspecified, then the length
     * of the result is chosen according to the internal representation.
     */
    template <typename T>
    std::shared_ptr<T> removeTrailer(bit length = bit(-1), int flags = 0) {
        const auto& chunk = popTrailer<T>(length, flags);
        removePoppedTrailers();
        return makeExclusivelyOwnedMutableChunk(chunk);
    }
    //@}

    /** @name Data querying related functions */
    //@{
    /**
     * Returns the current length of the data part of the packet. This is the
     * same as the difference between the header and trailer pop offsets.
     * The returned value is in the range [0, getTotalLength()].
     */
    bit getDataLength() const { return getTotalLength() - headerIterator.getPosition() - trailerIterator.getPosition(); }

    /**
     * Returns the designated data part as an immutable chunk in its current
     * representation. If the length is unspecified, then the length of the
     * result is chosen according to the internal representation.
     */
    std::shared_ptr<Chunk> peekDataAt(bit offset, bit length = bit(-1), int flags = 0) const;

    /**
     * Returns true if the designated data part is available in the requested
     * representation. If the length is unspecified, then the length of the
     * result is chosen according to the internal representation.
     */
    template <typename T>
    bool hasDataAt(bit offset, bit length = bit(-1)) const {
        return peekDataAt<T>(offset, length) != nullptr;
    }

    /**
     * Returns the designated data part as an immutable chunk in the requested
     * representation. If the length is unspecified, then the length of the
     * result is chosen according to the internal representation.
     */
    template <typename T>
    std::shared_ptr<T> peekDataAt(bit offset, bit length = bit(-1), int flags = 0) const {
        return contents == nullptr ? nullptr : contents->peek<T>(Chunk::Iterator(true, headerIterator.getPosition() + offset, -1), length, flags);
    }

    /**
     * Returns the data part (excluding popped headers and trailers) in the
     * current representation. The length of the returned chunk is the same as
     * the value returned by getDataLength().
     */
    std::shared_ptr<Chunk> peekData(int flags = 0) const {
        return peekDataAt(bit(0), getDataLength(), flags);
    }

    /**
     * Returns the data part (excluding popped headers and trailers) as a
     * sequence of bits. The length of the returned chunk is the same as the
     * value returned by getDataLength().
     */
    std::shared_ptr<BitsChunk> peekDataBits(int flags = 0) const {
        return peekDataAt<BitsChunk>(bit(0), getDataLength(), flags);
    }

    /**
     * Returns the data part (excluding popped headers and trailers) as a
     * sequence of bytes. The length of the returned chunk is the same as the
     * value returned by getDataLength().
     */
    std::shared_ptr<BytesChunk> peekDataBytes(int flags = 0) const {
        return peekDataAt<BytesChunk>(bit(0), getDataLength(), flags);
    }
    //@}

    /** @name Querying related functions */
    //@{
    /**
     * Returns the designated part of the packet as an immutable chunk in its
     * current representation. If the length is unspecified, then the length of
     * the result is chosen according to the internal representation.
     */
    std::shared_ptr<Chunk> peekAt(bit offset, bit length = bit(-1), int flags = 0) const;

    /**
     * Returns true if the designated part of the packet is available in the
     * requested representation. If the length is unspecified, then the length
     * of the result is chosen according to the internal representation.
     */
    template <typename T>
    bool hasAt(bit offset, bit length = bit(-1)) const {
        assert(bit(0) <= offset && offset <= getTotalLength());
        assert(bit(-1) <= length && length <= getTotalLength());
        return peekAt<T>(offset, length) != nullptr;
    }

    /**
     * Returns the designated part of the packet as an immutable chunk in the
     * requested representation. If the length is unspecified, then the length
     * of the result is chosen according to the internal representation.
     */
    template <typename T>
    std::shared_ptr<T> peekAt(bit offset, bit length = bit(-1), int flags = 0) const {
        assert(bit(0) <= offset && offset <= getTotalLength());
        assert(bit(-1) <= length && length <= getTotalLength());
        return contents == nullptr ? nullptr : contents->peek<T>(Chunk::Iterator(true, bit(offset), -1), length, flags);
    }


    /**
     * Returns the whole packet (including popped headers and trailers) in the
     * current representation. The length of the returned chunk is the same as
     * the value returned by getTotalLength().
     */
    std::shared_ptr<Chunk> peekAll(int flags = 0) const {
        return peekAt(bit(0), getTotalLength(), flags);
    }

    /**
     * Returns the whole packet (including popped headers and trailers) as a
     * sequence of bits. The length of the returned chunk is the same as the
     * value returned by getTotalLength().
     */
    std::shared_ptr<BitsChunk> peekAllBits(int flags = 0) const {
        return peekAt<BitsChunk>(bit(0), getTotalLength(), flags);
    }

    /**
     * Returns the whole packet (including popped headers and trailers) as a
     * sequence of bytes. The length of the returned chunk is the same as the
     * value returned by getTotalLength().
     */
    std::shared_ptr<BytesChunk> peekAllBytes(int flags = 0) const {
        return peekAt<BytesChunk>(bit(0), getTotalLength(), flags);
    }
    //@}

    /** @name Filling with data related functions */
    //@{
    /**
     * Inserts the provided chunk at the beginning of the packet. The header
     * pop offset must be zero before calling this function.
     */
    void prepend(const std::shared_ptr<Chunk>& chunk);

    /**
     * Inserts the provided chunk at the end of the packet. The trailer pop
     * offset must be zero before calling this function.
     */
    void append(const std::shared_ptr<Chunk>& chunk);
    //@}

    /** @name Removing data related functions */
    //@{
    /**
     * Removes the requested amount from the beginning of the packet. The header
     * pop offset must be zero before calling this function.
     */
    void removeFromBeginning(bit length);

    /**
     * Removes the requested amount from the end of the packet. The trailer pop
     * offset must be zero before calling this function.
     */
    void removeFromEnd(bit length);

    /**
     * Removes all popped headers, and sets the header pop offset to zero.
     * The popped trailers and the data part of the packet isn't affected.
     */
    void removePoppedHeaders();

    /**
     * Removes all popped trailers, and sets the trailer pop offset to zero.
     * The popped headers and the data part of the packet isn't affected.
     */
    void removePoppedTrailers();

    /**
     * Removes all popped headers and trailers, but the data part isn't affected.
     */
    void removePoppedChunks();
    //@}

    /**
     * Returns a human readable string representation.
     */
    virtual std::string str() const override { return contents == nullptr ? "" : contents->str(); }
};

inline std::ostream& operator<<(std::ostream& os, const Packet *packet) { return os << packet->str(); }

inline std::ostream& operator<<(std::ostream& os, const Packet& packet) { return os << packet.str(); }

} // namespace

#endif // #ifndef __INET_PACKET_H_

