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
#include "inet/common/packet/tag/TagSet.h"

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
 * packet. Chunks are automatically merged as they are inserted into a packet,
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
    Ptr<const Chunk> content;
    Chunk::ForwardIterator frontIterator;
    Chunk::BackwardIterator backIterator;
    b totalLength;
    TagSet tags;

  protected:
    const Chunk *getContent() const { return content.get(); } // only for class descriptor

    bool isIteratorConsistent(const Chunk::Iterator& iterator) {
        Chunk::Iterator copy(iterator);
        content->seekIterator(copy, iterator.getPosition());
        return iterator.getPosition() == copy.getPosition() && (iterator.getIndex() == -1 || iterator.getIndex() == copy.getIndex());
    }

  public:
    explicit Packet(const char *name = nullptr, short kind = 0);
    Packet(const char *name, const Ptr<const Chunk>& content);
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
    virtual bool hasBitError() const override { return cPacket::hasBitError() || content->isIncorrect(); }
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
    b getFrontOffset() const { return frontIterator.getPosition(); }

    /**
     * Changes the header pop offset measured from the beginning of the packet.
     * The value must be in the range [0, getTotalLength()].
     */
    void setFrontOffset(b offset);

    /**
     * Returns the total length of popped headers.
     */

    /**
     * Returns the designated header as an immutable chunk in its current
     * representation. If the length is unspecified, then the length of the
     * result is chosen according to the internal representation.
     */
    const Ptr<const Chunk> peekAtFront(b length = b(-1), int flags = 0) const;

    /**
     * Inserts the provided header at the beginning of the packet. The inserted
     * header is automatically marked immutable. The popped header length must
     * be zero before calling this function.
     */

    /**
     * Pops the designated header and returns it as an immutable chunk in its
     * current representation. If the length is unspecified, then the length of
     * the result is chosen according to the internal representation.
     */
    const Ptr<const Chunk> popAtFront(b length = b(-1), int flags = 0);

    /**
     * Removes the designated header and returns it as a mutable chunk in its
     * current representation. If the length is unspecified, then the length of
     * the result is chosen according to the internal representation. The popped
     * header length must be zero before calling this function.
     */

    /**
     * Returns true if the designated header is available in the requested
     * representation. If the length is unspecified, then the length of the
     * result is chosen according to the internal representation.
     */
    template <typename T>
    bool hasAtFront(b length = b(-1)) const {
        CHUNK_CHECK_USAGE(b(-1) <= length && length <= getDataLength(), "length is invalid");
        return content->has<T>(frontIterator, length);
    }

    /**
     * Returns the designated header as an immutable chunk in the requested
     * representation. If the length is unspecified, then the length of the
     * result is chosen according to the internal representation.
     */
    template <typename T>
    const Ptr<const T> peekAtFront(b length = b(-1), int flags = 0) const {
        auto dataLength = getDataLength();
        CHUNK_CHECK_USAGE(b(-1) <= length && length <= dataLength, "length is invalid");
        const auto& chunk = content->peek<T>(frontIterator, length, flags);
        if (chunk == nullptr || chunk->getChunkLength() <= dataLength)
            return chunk;
        else
            return content->peek<T>(frontIterator, dataLength, flags);
    }

    /**
     * Pops the designated header and returns it as an immutable chunk in the
     * requested representation. If the length is unspecified, then the length
     * of the result is chosen according to the internal representation.
     */
    template <typename T>
    const Ptr<const T> popAtFront(b length = b(-1), int flags = 0) {
        CHUNK_CHECK_USAGE(b(-1) <= length && length <= getDataLength(), "length is invalid");
        const auto& chunk = peekAtFront<T>(length, flags);
        if (chunk != nullptr) {
            content->moveIterator(frontIterator, chunk->getChunkLength());
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
    //@}

    /** @name Trailer querying related functions */
    //@{
    /**
     * Returns the trailer pop offset measured from the beginning of the packet.
     * The returned value is in the range [0, getTotalLength()].
     */
    b getBackOffset() const { return getTotalLength() - backIterator.getPosition(); }

    /**
     * Changes the trailer pop offset measured from the beginning of the packet.
     * The value must be in the range [0, getTotalLength()].
     */
    void setBackOffset(b offset);

    /**
     * Returns the total length of popped trailers.
     */

    /**
     * Returns the designated trailer as an immutable chunk in its current
     * representation. If the length is unspecified, then the length of the
     * result is chosen according to the internal representation.
     */
    const Ptr<const Chunk> peekAtBack(b length = b(-1), int flags = 0) const;

    /**
     * Inserts the provided trailer at the end of the packet. The inserted trailer
     * is automatically marked immutable. The popped trailer length must be zero
     * before calling this function.
     */

    /**
     * Pops the designated trailer and returns it as an immutable chunk in its
     * current representation. If the length is unspecified, then the length of
     * the result is chosen according to the internal representation.
     */
    const Ptr<const Chunk> popAtBack(b length = b(-1), int flags = 0);

    /**
     * Removes the designated trailer and returns it as a mutable chunk in its
     * current representation. If the length is unspecified, then the length of
     * the result is chosen according to the internal representation. The popped
     * trailer length must be zero before calling this function.
     */

    /**
     * Returns true if the designated trailer is available in the requested
     * representation. If the length is unspecified, then the length of the
     * result is chosen according to the internal representation.
     */
    template <typename T>
    bool hasAtBack(b length = b(-1)) const {
        CHUNK_CHECK_USAGE(b(-1) <= length && length <= getDataLength(), "length is invalid");
        return content->has<T>(backIterator, length);
    }

    /**
     * Returns the designated trailer as an immutable chunk in the requested
     * representation. If the length is unspecified, then the length of the
     * result is chosen according to the internal representation.
     */
    template <typename T>
    const Ptr<const T> peekAtBack(b length = b(-1), int flags = 0) const {
        auto dataLength = getDataLength();
        CHUNK_CHECK_USAGE(b(-1) <= length && length <= dataLength, "length is invalid");
        const auto& chunk = content->peek<T>(backIterator, length, flags);
        if (chunk == nullptr || chunk->getChunkLength() <= dataLength)
            return chunk;
        else
            return content->peek<T>(backIterator, dataLength, flags);
    }

    /**
     * Pops the designated trailer and returns it as an immutable chunk in the
     * requested representation. If the length is unspecified, then the length
     * of the result is chosen according to the internal representation.
     */
    template <typename T>
    const Ptr<const T> popAtBack(b length = b(-1), int flags = 0) {
        CHUNK_CHECK_USAGE(b(-1) <= length && length <= getDataLength(), "length is invalid");
        const auto& chunk = peekAtBack<T>(length, flags);
        if (chunk != nullptr) {
            content->moveIterator(backIterator, chunk->getChunkLength());
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
    //@}

    /** @name Data querying related functions */
    //@{
    /**
     * Returns the current length of the data part of the packet. This is the
     * same as the difference between the header and trailer pop offsets.
     * The returned value is in the range [0, getTotalLength()].
     */
    b getDataLength() const { return getTotalLength() - frontIterator.getPosition() - backIterator.getPosition(); }

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
        return content->has<T>(Chunk::Iterator(true, frontIterator.getPosition() + offset, -1), length);
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
        return content->peek<T>(Chunk::Iterator(true, frontIterator.getPosition() + offset, -1), length, flags);
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
    const Ptr<const BitsChunk> peekDataAsBits(int flags = 0) const {
        return peekDataAt<BitsChunk>(b(0), getDataLength(), flags);
    }

    /**
     * Returns the data part (excluding popped headers and trailers) as a
     * sequence of bytes. The length of the returned chunk is the same as the
     * value returned by getDataLength().
     */
    const Ptr<const BytesChunk> peekDataAsBytes(int flags = 0) const {
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
        return content->has<T>(Chunk::Iterator(true, b(offset), -1), length);
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
        return content->peek<T>(Chunk::Iterator(true, b(offset), -1), length, flags);
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
    const Ptr<const BitsChunk> peekAllAsBits(int flags = 0) const {
        return peekAt<BitsChunk>(b(0), getTotalLength(), flags);
    }

    /**
     * Returns the whole packet (including popped headers and trailers) as a
     * sequence of bytes. The length of the returned chunk is the same as the
     * value returned by getTotalLength().
     */
    const Ptr<const BytesChunk> peekAllAsBytes(int flags = 0) const {
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
    void insertAtFront(const Ptr<const Chunk>& chunk);

    /**
     * Inserts the provided chunk at the end of the packet. The popped trailer
     * length must be zero before calling this function.
     */
    void insertAtBack(const Ptr<const Chunk>& chunk);
    //@}

    /** @name Removing data related functions */
    //@{
    /**
     * Removes the requested amount from the beginning of the packet. The popped
     * header length must be zero before calling this function.
    void eraseAtFront(b length);
    void eraseAtBack(b length);
    void eraseAll();
    const Ptr<Chunk> removeAtFront(b length = b(-1), int flags = 0);
     */
    const Ptr<Chunk> removeAtBack(b length = b(-1), int flags = 0);

    /**
     * Removes the requested amount from the end of the packet. The popped trailer
     * length must be zero before calling this function.
    template <typename T>
    const Ptr<T> removeAtFront(b length = b(-1), int flags = 0) {
        CHUNK_CHECK_USAGE(b(-1) <= length && length <= getDataLength(), "length is invalid");
        CHUNK_CHECK_USAGE(frontIterator.getPosition() == b(0), "front popped length is non-zero");
        const auto& chunk = popAtFront<T>(length, flags);
        trimFront();
        return makeExclusivelyOwnedMutableChunk(chunk);
    }
     */
    template <typename T>
    const Ptr<T> removeAtBack(b length = b(-1), int flags = 0) {
        CHUNK_CHECK_USAGE(b(-1) <= length && length <= getDataLength(), "length is invalid");
        CHUNK_CHECK_USAGE(backIterator.getPosition() == b(0), "back popped length is non-zero");
        const auto& chunk = popAtBack<T>(length, flags);
        trimBack();
        return makeExclusivelyOwnedMutableChunk(chunk);
    }

    /**
     * Removes all popped headers, and sets the header pop length to zero.
     * The popped trailers and the data part of the packet isn't affected.
     */
    const Ptr<Chunk> removeAll();

    /**
     * Removes all popped trailers, and sets the trailer pop length to zero.
     * The popped headers and the data part of the packet isn't affected.
     */
    void trimFront();

    /**
     * Removes all popped headers and trailers, but the data part isn't affected.
     */
    void trimBack();

    /**
     * Removes all data from packet.
     */
    void trim();
    //@}

    /** @name Tag related functions */
    //@{
    /**
     * Returns all tags.
     */
    TagSet& getTags() { return tags; }

    /**
     * Returns the number of packet tags.
     */
    int getNumTags() const {
        return tags.getNumTags();
    }

    /**
     * Returns the packet tag at the given index.
     */
    cObject *getTag(int index) const {
        return tags.getTag(index);
    }

    /**
     * Clears the set of packet tags.
     */
    void clearTags() {
        tags.clearTags();
    }

    /**
     * Copies the set of packet tags from the other packet.
     */
    void copyTags(const Packet& source) {
        tags.copyTags(source.tags);
    }

    /**
     * Returns the packet tag for the provided type or returns nullptr if no such packet tag is found.
     */
    template<typename T> T *findTag() const {
        return tags.findTag<T>();
    }

    /**
     * Returns the packet tag for the provided type or throws an exception if no such packet tag is found.
     */
    template<typename T> T *getTag() const {
        return tags.getTag<T>();
    }

    /**
     * Returns a newly added packet tag for the provided type, or throws an exception if such a packet tag is already present.
     */
    template<typename T> T *addTag() {
        return tags.addTag<T>();
    }

    /**
     * Returns a newly added packet tag for the provided type if absent, or returns the packet tag that is already present.
     */
    template<typename T> T *addTagIfAbsent() {
        return tags.addTagIfAbsent<T>();
    }

    /**
     * Removes the packet tag for the provided type, or throws an exception if no such packet tag is found.
     */
    template<typename T> T *removeTag() {
        return tags.removeTag<T>();
    }

    /**
     * Removes the packet tag for the provided type if present, or returns nullptr if no such packet tag is found.
     */
    template<typename T> T *removeTagIfPresent() {
        return tags.removeTagIfPresent<T>();
    }
    //@}

    /**
     * Returns a human readable string representation.
     */
    virtual std::string str() const override;
};

INET_API TagSet& getTags(cMessage *msg);

inline std::ostream& operator<<(std::ostream& os, const Packet *packet) { return os << packet->str(); }

inline std::ostream& operator<<(std::ostream& os, const Packet& packet) { return os << packet.str(); }

} // namespace

#endif // #ifndef __INET_PACKET_H_

