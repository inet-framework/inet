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
    Ptr<const Chunk> contents;
    Chunk::ForwardIterator headerIterator;
    Chunk::BackwardIterator trailerIterator;
    b totalLength;

#if 0
    //FIXME next cache variables needed for getHeader/Data/TrailerPart() functions
    Ptr<const Chunk> __headerCache;
    Ptr<const Chunk> __dataCache;
    Ptr<const Chunk> __trailerCache;
#endif

  protected:
    const Chunk *getContents() const { return contents.get(); } // only for class descriptor

#if 0
    //FIXME next functions generates a new shared pointer, cache variable needed
    const Chunk *getHeaderPart() const; // only for class descriptor
    const Chunk *getDataPart() const; // only for class descriptor
    const Chunk *getTrailerPart() const; // only for class descriptor
#endif

    bool isIteratorConsistent(const Chunk::Iterator& iterator) {
        Chunk::Iterator copy(iterator);
        contents->seekIterator(copy, iterator.getPosition());
        return iterator.getPosition() == copy.getPosition() && (iterator.getIndex() == -1 || iterator.getIndex() == copy.getIndex());
    }

  public:
    explicit Packet(const char *name = nullptr, short kind = 0);
    Packet(const char *name, const Ptr<const Chunk>& contents);
    Packet(const Packet& other);

    virtual Packet *dup() const override { return new Packet(*this); }
    virtual void forEachChild(cVisitor *v) override;

    /** @name Length querying related functions */
    //@{
    /**
     * Returns the total packet length ignoring header and trailer iterators.
     * The returned value is in the range [0, +infinity).
     */
    b getTotalLength() const { return totalLength; }

    /**
     * Returns the length in bits between the header and trailer iterators.
     * The returned value is in the range [0, +infinity).
     */
    virtual int64_t getBitLength() const override { return b(getDataLength()).get(); }

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

    virtual void setControlInfo(cObject *p) override { throw cRuntimeError("Invalid operation"); }
    //@}

    /** @name Header querying related functions */
    //@{
    /**
     * Returns the header pop offset measured from the beginning of the packet.
     * The returned value is in the range [0, getTotalLength()].
     */
    b getHeaderPopOffset() const { return headerIterator.getPosition(); }

    /**
     * Changes the header pop offset measured from the beginning of the packet.
     * The value must be in the range [0, getTotalLength()].
     */
    void setHeaderPopOffset(b offset);

    /**
     * Returns the total length of popped headers.
     */
    b getHeaderPoppedLength() const { return headerIterator.getPosition(); }

    /**
     * Returns the designated header as an immutable chunk in its current
     * representation. If the length is unspecified, then the length of the
     * result is chosen according to the internal representation.
     */
    const Ptr<const Chunk> peekHeader(b length = b(-1), int flags = 0) const;

    /**
     * Pops the designated header and returns it as an immutable chunk in its
     * current representation. If the length is unspecified, then the length of
     * the result is chosen according to the internal representation.
     */
    const Ptr<const Chunk> popHeader(b length = b(-1), int flags = 0);

    /**
     * Removes the designated header and returns it as a mutable chunk in its
     * current representation. If the length is unspecified, then the length of
     * the result is chosen according to the internal representation. The popped
     * header length must be zero before calling this function.
     */
    const Ptr<Chunk> removeHeader(b length = b(-1), int flags = 0);

    /**
     * Pushes the provided header at the beginning of the packet. The header
     * must be immutable and the popped header length must be zero before
     * calling this function.
     */
    void pushHeader(const Ptr<const Chunk>& chunk);

    /**
     * Pushes the provided header at the beginning of the packet. The pushed
     * header is automatically marked immutable. The popped header length must
     * be zero before calling this function.
     */
    void insertHeader(const Ptr<const Chunk>& chunk);

    /**
     * Returns true if the designated header is available in the requested
     * representation. If the length is unspecified, then the length of the
     * result is chosen according to the internal representation.
     */
    template <typename T>
    bool hasHeader(b length = b(-1)) const {
        CHUNK_CHECK_USAGE(b(-1) <= length && length <= getDataLength(), "length is invalid");
        return contents->has<T>(headerIterator, length);
    }

    /**
     * Returns the designated header as an immutable chunk in the requested
     * representation. If the length is unspecified, then the length of the
     * result is chosen according to the internal representation.
     */
    template <typename T>
    const Ptr<const T> peekHeader(b length = b(-1), int flags = 0) const {
        auto dataLength = getDataLength();
        CHUNK_CHECK_USAGE(b(-1) <= length && length <= dataLength, "length is invalid");
        const auto& chunk = contents->peek<T>(headerIterator, length, flags);
        if (chunk == nullptr || chunk->getChunkLength() <= dataLength)
            return chunk;
        else
            return contents->peek<T>(headerIterator, dataLength, flags);
    }

    /**
     * Pops the designated header and returns it as an immutable chunk in the
     * requested representation. If the length is unspecified, then the length
     * of the result is chosen according to the internal representation.
     */
    template <typename T>
    const Ptr<const T> popHeader(b length = b(-1), int flags = 0) {
        CHUNK_CHECK_USAGE(b(-1) <= length && length <= getDataLength(), "length is invalid");
        const auto& chunk = peekHeader<T>(length, flags);
        if (chunk != nullptr) {
            contents->moveIterator(headerIterator, chunk->getChunkLength());
            CHUNK_CHECK_IMPLEMENTATION(getDataLength() >= b(0));
        }
        return chunk;
    }

    /**
     * Removes the designated header and returns it as a mutable chunk in the
     * requested representation. If the length is unspecified, then the length
     * of the result is chosen according to the internal representation. The
     * popped header length must be zero before calling this function.
     */
    template <typename T>
    const Ptr<T> removeHeader(b length = b(-1), int flags = 0) {
        CHUNK_CHECK_USAGE(b(-1) <= length && length <= getDataLength(), "length is invalid");
        CHUNK_CHECK_USAGE(headerIterator.getPosition() == b(0), "popped header length is non-zero");
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
    b getTrailerPopOffset() const { return getTotalLength() - trailerIterator.getPosition(); }

    /**
     * Changes the trailer pop offset measured from the beginning of the packet.
     * The value must be in the range [0, getTotalLength()].
     */
    void setTrailerPopOffset(b offset);

    /**
     * Returns the total length of popped trailers.
     */
    b getTrailerPoppedLength() const { return trailerIterator.getPosition(); }

    /**
     * Returns the designated trailer as an immutable chunk in its current
     * representation. If the length is unspecified, then the length of the
     * result is chosen according to the internal representation.
     */
    const Ptr<const Chunk> peekTrailer(b length = b(-1), int flags = 0) const;

    /**
     * Pops the designated trailer and returns it as an immutable chunk in its
     * current representation. If the length is unspecified, then the length of
     * the result is chosen according to the internal representation.
     */
    const Ptr<const Chunk> popTrailer(b length = b(-1), int flags = 0);

    /**
     * Removes the designated trailer and returns it as a mutable chunk in its
     * current representation. If the length is unspecified, then the length of
     * the result is chosen according to the internal representation. The popped
     * trailer length must be zero before calling this function.
     */
    const Ptr<Chunk> removeTrailer(b length = b(-1), int flags = 0);

    /**
     * Pushes the provided trailer at the end of the packet. The trailer must be
     * immutable and the popped trailer length must be zero before calling this
     * function.
     */
    void pushTrailer(const Ptr<const Chunk>& chunk);

    /**
     * Pushes the provided trailer at the end of the packet. The pushed trailer
     * is automatically marked immutable. The popped trailer length must be zero
     * before calling this function.
     */
    void insertTrailer(const Ptr<const Chunk>& chunk);

    /**
     * Returns true if the designated trailer is available in the requested
     * representation. If the length is unspecified, then the length of the
     * result is chosen according to the internal representation.
     */
    template <typename T>
    bool hasTrailer(b length = b(-1)) const {
        CHUNK_CHECK_USAGE(b(-1) <= length && length <= getDataLength(), "length is invalid");
        return contents->has<T>(trailerIterator, length);
    }

    /**
     * Returns the designated trailer as an immutable chunk in the requested
     * representation. If the length is unspecified, then the length of the
     * result is chosen according to the internal representation.
     */
    template <typename T>
    const Ptr<const T> peekTrailer(b length = b(-1), int flags = 0) const {
        auto dataLength = getDataLength();
        CHUNK_CHECK_USAGE(b(-1) <= length && length <= dataLength, "length is invalid");
        const auto& chunk = contents->peek<T>(trailerIterator, length, flags);
        if (chunk == nullptr || chunk->getChunkLength() <= dataLength)
            return chunk;
        else
            return contents->peek<T>(trailerIterator, dataLength, flags);
    }

    /**
     * Pops the designated trailer and returns it as an immutable chunk in the
     * requested representation. If the length is unspecified, then the length
     * of the result is chosen according to the internal representation.
     */
    template <typename T>
    const Ptr<const T> popTrailer(b length = b(-1), int flags = 0) {
        CHUNK_CHECK_USAGE(b(-1) <= length && length <= getDataLength(), "length is invalid");
        const auto& chunk = peekTrailer<T>(length, flags);
        if (chunk != nullptr) {
            contents->moveIterator(trailerIterator, chunk->getChunkLength());
            CHUNK_CHECK_IMPLEMENTATION(getDataLength() >= b(0));
        }
        return chunk;
    }

    /**
     * Removes the designated trailer and returns it as a mutable chunk in the
     * requested representation. If the length is unspecified, then the length
     * of the result is chosen according to the internal representation. The
     * popped trailer length must be zero before calling this function.
     */
    template <typename T>
    const Ptr<T> removeTrailer(b length = b(-1), int flags = 0) {
        CHUNK_CHECK_USAGE(b(-1) <= length && length <= getDataLength(), "length is invalid");
        CHUNK_CHECK_USAGE(trailerIterator.getPosition() == b(0), "popped trailer length is non-zero");
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
    b getDataLength() const { return getTotalLength() - headerIterator.getPosition() - trailerIterator.getPosition(); }

    /**
     * Returns the designated data part as an immutable chunk in its current
     * representation. If the length is unspecified, then the length of the
     * result is chosen according to the internal representation.
     */
    const Ptr<const Chunk> peekDataAt(b offset, b length = b(-1), int flags = 0) const;

    /**
     * Returns true if the designated data part is available in the requested
     * representation. If the length is unspecified, then the length of the
     * result is chosen according to the internal representation.
     */
    template <typename T>
    bool hasDataAt(b offset, b length = b(-1)) const {
        CHUNK_CHECK_USAGE(b(0) <= offset && offset <= getDataLength(), "offset is out of range");
        CHUNK_CHECK_USAGE(b(-1) <= length && offset + length <= getDataLength(), "length is invalid");
        return contents->has<T>(Chunk::Iterator(true, headerIterator.getPosition() + offset, -1), length);
    }

    /**
     * Returns the designated data part as an immutable chunk in the requested
     * representation. If the length is unspecified, then the length of the
     * result is chosen according to the internal representation.
     */
    template <typename T>
    const Ptr<const T> peekDataAt(b offset, b length = b(-1), int flags = 0) const {
        CHUNK_CHECK_USAGE(b(0) <= offset && offset <= getDataLength(), "offset is out of range");
        CHUNK_CHECK_USAGE(b(-1) <= length && offset + length <= getDataLength(), "length is invalid");
        return contents->peek<T>(Chunk::Iterator(true, headerIterator.getPosition() + offset, -1), length, flags);
    }

    /**
     * Returns the data part (excluding popped headers and trailers) in the
     * current representation. The length of the returned chunk is the same as
     * the value returned by getDataLength().
     */
    const Ptr<const Chunk> peekData(int flags = 0) const {
        return peekDataAt(b(0), getDataLength(), flags);
    }

    /**
     * Returns the data part (excluding popped headers and trailers) as a
     * sequence of bits. The length of the returned chunk is the same as the
     * value returned by getDataLength().
     */
    const Ptr<const BitsChunk> peekDataBits(int flags = 0) const {
        return peekDataAt<BitsChunk>(b(0), getDataLength(), flags);
    }

    /**
     * Returns the data part (excluding popped headers and trailers) as a
     * sequence of bytes. The length of the returned chunk is the same as the
     * value returned by getDataLength().
     */
    const Ptr<const BytesChunk> peekDataBytes(int flags = 0) const {
        return peekDataAt<BytesChunk>(b(0), getDataLength(), flags);
    }

    /**
     * Returns the data part (excluding popped headers and trailers) in the
     * requested representation. The length of the returned chunk is the same as
     * the value returned by getDataLength().
     */
    template <typename T>
    const Ptr<const T> peekData(int flags = 0) const {
        return peekDataAt<T>(b(0), getDataLength(), flags);
    }
    //@}

    /** @name Querying related functions */
    //@{
    /**
     * Returns the designated part of the packet as an immutable chunk in its
     * current representation. If the length is unspecified, then the length of
     * the result is chosen according to the internal representation.
     */
    const Ptr<const Chunk> peekAt(b offset, b length = b(-1), int flags = 0) const;

    /**
     * Returns true if the designated part of the packet is available in the
     * requested representation. If the length is unspecified, then the length
     * of the result is chosen according to the internal representation.
     */
    template <typename T>
    bool hasAt(b offset, b length = b(-1)) const {
        CHUNK_CHECK_USAGE(b(0) <= offset && offset <= getTotalLength(), "offset is out of range");
        CHUNK_CHECK_USAGE(b(-1) <= length && offset + length <= getTotalLength(), "length is invalid");
        return contents->has<T>(Chunk::Iterator(true, b(offset), -1), length);
    }

    /**
     * Returns the designated part of the packet as an immutable chunk in the
     * requested representation. If the length is unspecified, then the length
     * of the result is chosen according to the internal representation.
     */
    template <typename T>
    const Ptr<const T> peekAt(b offset, b length = b(-1), int flags = 0) const {
        CHUNK_CHECK_USAGE(b(0) <= offset && offset <= getTotalLength(), "offset is out of range");
        CHUNK_CHECK_USAGE(b(-1) <= length && offset + length <= getTotalLength(), "length is invalid");
        return contents->peek<T>(Chunk::Iterator(true, b(offset), -1), length, flags);
    }

    /**
     * Returns the whole packet (including popped headers and trailers) in the
     * current representation. The length of the returned chunk is the same as
     * the value returned by getTotalLength().
     */
    const Ptr<const Chunk> peekAll(int flags = 0) const {
        return peekAt(b(0), getTotalLength(), flags);
    }

    /**
     * Returns the whole packet (including popped headers and trailers) as a
     * sequence of bits. The length of the returned chunk is the same as the
     * value returned by getTotalLength().
     */
    const Ptr<const BitsChunk> peekAllBits(int flags = 0) const {
        return peekAt<BitsChunk>(b(0), getTotalLength(), flags);
    }

    /**
     * Returns the whole packet (including popped headers and trailers) as a
     * sequence of bytes. The length of the returned chunk is the same as the
     * value returned by getTotalLength().
     */
    const Ptr<const BytesChunk> peekAllBytes(int flags = 0) const {
        return peekAt<BytesChunk>(b(0), getTotalLength(), flags);
    }

    /**
     * Returns the whole packet (including popped headers and trailers) in the
     * requested representation. The length of the returned chunk is the same as
     * the value returned by getTotalLength().
     */
    template <typename T>
    const Ptr<const T> peekAll(int flags = 0) const {
        return peekAt<T>(b(0), getTotalLength(), flags);
    }
    //@}

    /** @name Filling with data related functions */
    //@{
    /**
     * Inserts the provided chunk at the beginning of the packet. The popped
     * header length must be zero before calling this function.
     */
    void prepend(const Ptr<const Chunk>& chunk);

    /**
     * Inserts the provided chunk at the end of the packet. The popped trailer
     * length must be zero before calling this function.
     */
    void append(const Ptr<const Chunk>& chunk);
    //@}

    /** @name Removing data related functions */
    //@{
    /**
     * Removes the requested amount from the beginning of the packet. The popped
     * header length must be zero before calling this function.
     */
    void removeFromBeginning(b length);

    /**
     * Removes the requested amount from the end of the packet. The popped trailer
     * length must be zero before calling this function.
     */
    void removeFromEnd(b length);

    /**
     * Removes all popped headers, and sets the header pop length to zero.
     * The popped trailers and the data part of the packet isn't affected.
     */
    void removePoppedHeaders();

    /**
     * Removes all popped trailers, and sets the trailer pop length to zero.
     * The popped headers and the data part of the packet isn't affected.
     */
    void removePoppedTrailers();

    /**
     * Removes all popped headers and trailers, but the data part isn't affected.
     */
    void removePoppedChunks();
    //@}

    /**
     * Removes all data from packet
     */
    void removeAll();
    /**
     * Returns a human readable string representation.
     */
    virtual std::string str() const override { return contents->str(); }
};

inline std::ostream& operator<<(std::ostream& os, const Packet *packet) { return os << packet->str(); }

inline std::ostream& operator<<(std::ostream& os, const Packet& packet) { return os << packet.str(); }

} // namespace

#endif // #ifndef __INET_PACKET_H_

