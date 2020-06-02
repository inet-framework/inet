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

#include <functional>
#include "inet/common/packet/chunk/BitsChunk.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/common/packet/chunk/SequenceChunk.h"
#include "inet/common/packet/tag/SharingRegionTagSet.h"
#include "inet/common/packet/tag/SharingTagSet.h"
#include "inet/common/TagBase.h"

namespace inet {

/**
 * This class represents network packets, datagrams, frames and other kinds of
 * data used by communication protocols. The underlying data structure supports
 * efficient construction, duplication, encapsulation, aggregation, fragmentation
 * and serialization. The data structure also supports dual representation by
 * default: data can be accessed as raw bytes and also as field based classes.
 *
 * Internally, packet stores the data in different kind of chunks. See the Chunk
 * class and its subclasses for details. All chunks are immutable in a packet.
 * Chunks are automatically merged if possible as they are inserted into a packet,
 * and they are also shared among packets when duplicating.
 *
 * Packets are conceptually divided into three parts during processing: front
 * popped part, data part, and back popped part. These parts are separated by
 * iterators called front and back pointer, which are stored within the packet.
 *
 *     +----------------------------------------------------------------+
 *     |                                                                |
 *     |             Packet content is divided into 3 parts             |
 *     |                                                                |
 *     +---------------------+---------------------+--------------------+
 *     |                     |                     |                    |
 *     |  Front popped part  |      Data part      |  Back popped part  |
 *     |                     |                     |                    |
 *     +----------------------------------------------------------------+
 *                           ^                     ^
 *                           |                     |
 *                      Front pointer          Back pointer
 *
 * During packet processing, as the packet is passed through the protocol layers
 * at the receiver, headers and trailers are popped from the beginning and the
 * end. This effectively reduces the remaining unprocessed part called the data
 * part, but it doesn't affect the data stored in the packet. Popping headers and
 * trailers is most often a very efficient operation, because it doesn't require
 * copying or changing chunks, only updating the front and back iterators.
 *
 * In general, this class supports the following operations:
 *  - query the length of the packet and the data part
 *  - insert to the beginning or end of the packet
 *  - remove from the beginning or end of the packet
 *  - peek or pop from the front or the back of the data part
 *  - serialize to and deserialize from a sequence of bits or bytes
 *  - copy the packet, it's a cheap operation
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
    /**
     * Position of the front separator measured from the front of the packet.
     */
    Chunk::ForwardIterator frontIterator;
    /**
     * Position of the back separator measured from the back of the packet.
     */
    Chunk::BackwardIterator backIterator;
    /**
     * Cached total length of the packet including the popped frond and back parts.
     */
    b totalLength;

    /**
     * The set of tags attached to the packet as whole.
     */
    SharingTagSet tags;
    /**
     * The set of tags attached to regions of the content of the packet.
     */
    SharingRegionTagSet regionTags;

  protected:
    /** @name Class descriptor functions */
    //@{
    const Chunk *getContent() const { return content.get(); }
    const ChunkTemporarySharedPtr *getDissection() const;
    const ChunkTemporarySharedPtr *getFront() const;
    const ChunkTemporarySharedPtr *getData() const;
    const ChunkTemporarySharedPtr *getBack() const;
    const TagBase *_getTag(int index) { return tags.getTag(index).get(); }
    const SharingRegionTagSet::RegionTag<TagBase>& _getRegionTag(int index) { return regionTags.getRegionTag(index); }
    //@}

    /** @name Self checking functions */
    //@{
    bool isIteratorConsistent(const Chunk::Iterator& iterator) {
        Chunk::Iterator copy(iterator);
        content->seekIterator(copy, iterator.getPosition());
        return iterator.getPosition() == copy.getPosition() && (iterator.getIndex() == -1 || iterator.getIndex() == copy.getIndex());
    }
    //@}

  public:
    /** @name Constructors */
    //@{
    explicit Packet(const char *name = nullptr, short kind = 0);
    Packet(const char *name, const Ptr<const Chunk>& content);
    Packet(const Packet& other);
    //@}

    /** @name Supported cPacket interface functions */
    //@{
    virtual Packet *dup() const override { return new Packet(*this); }
    virtual void forEachChild(cVisitor *v) override;
    virtual bool hasBitError() const override { return cPacket::hasBitError() || content->isIncorrect(); }
    //@}

    /** @name Unsupported cPacket interface functions */
    //@{
    virtual void encapsulate(cPacket *packet) override { throw cRuntimeError("Invalid operation"); }
    virtual cPacket *decapsulate() override { throw cRuntimeError("Invalid operation"); }
    virtual cPacket *getEncapsulatedPacket() const override { return nullptr; }
    virtual void setControlInfo(cObject *p) override { throw cRuntimeError("Invalid operation"); }
    //@}

    /** @name Length querying functions */
    //@{
    /**
     * Returns the total packet length ignoring front and back offsets.
     * The returned value is in the range [0, +infinity).
     */
    b getTotalLength() const { return totalLength; }

    /**
     * Returns the length in bits between the front and back offsets.
     * The returned value is in the range [0, +infinity).
     */
    virtual int64_t getBitLength() const override { return b(getDataLength()).get(); }

    virtual void setBitLength(int64_t value) override { throw cRuntimeError("Invalid operation"); }
    //@}

    /** @name Data part front querying functions */
    //@{
    /**
     * Returns the front offset measured from the beginning of the packet.
     * The returned value is in the range [0, getTotalLength()].
     */
    b getFrontOffset() const { return frontIterator.getPosition(); }

    /**
     * Changes the front offset measured from the beginning of the packet.
     * The value must be in the range [0, getTotalLength()].
     */
    void setFrontOffset(b offset);

    /**
     * Returns the designated part as an immutable chunk in its current
     * representation. If the length is unspecified, then the length of the
     * result is chosen according to the current representation. The flags
     * parameter is a combination of Chunk::PeekFlag enumeration members.
     */
    const Ptr<const Chunk> peekAtFront(b length = b(-1), int flags = 0) const;

    /**
     * Pops the designated part and returns it as an immutable chunk in its
     * current representation. Increases the front offset with the length of
     * the returned chunk. If the length is unspecified, then the length of
     * the result is chosen according to the current representation. The
     * flags parameter is a combination of Chunk::PeekFlag enumeration members.
     */
    const Ptr<const Chunk> popAtFront(b length = b(-1), int flags = 0);

    /**
     * Returns true if the designated part is available in the requested
     * representation. If the length is unspecified, then the length of the
     * result is chosen according to the current representation.
     */
    template <typename T>
    bool hasAtFront(b length = b(-1)) const {
        auto dataLength = getDataLength();
        CHUNK_CHECK_USAGE(b(-1) <= length && length <= dataLength, "length is invalid");
        return content->has<T>(frontIterator, length == b(-1) ? -dataLength: length);
    }

    /**
     * Returns the designated part as an immutable chunk in the requested
     * representation. If the length is unspecified, then the length of the
     * result is chosen according to the current representation. The flags
     * parameter is a combination of Chunk::PeekFlag enumeration members.
     */
    template <typename T>
    const Ptr<const T> peekAtFront(b length = b(-1), int flags = 0) const {
        auto dataLength = getDataLength();
        CHUNK_CHECK_USAGE(b(-1) <= length && length <= dataLength, "length is invalid");
        const auto& chunk = content->peek<T>(frontIterator, length == b(-1) ? -dataLength : length, flags);
        CHUNK_CHECK_IMPLEMENTATION(chunk == nullptr || chunk->getChunkLength() <= dataLength);
        return chunk;
    }

    /**
     * Pops the designated part and returns it as an immutable chunk in the
     * requested representation. Increases the front offset with the length of
     * the returned chunk. If the length is unspecified, then the length of the
     * result is chosen according to the current representation. The flags
     * parameter is a combination of Chunk::PeekFlag enumeration members.
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
    //@}

    /** @name Data part back querying functions */
    //@{
    /**
     * Returns the back offset measured from the beginning of the packet.
     * The returned value is in the range [0, getTotalLength()].
     */
    b getBackOffset() const { return getTotalLength() - backIterator.getPosition(); }

    /**
     * Changes the back offset measured from the beginning of the packet.
     * The value must be in the range [0, getTotalLength()].
     */
    void setBackOffset(b offset);

    /**
     * Returns the designated part as an immutable chunk in its current
     * representation. If the length is unspecified, then the length of the
     * result is chosen according to the current representation. The flags
     * parameter is a combination of Chunk::PeekFlag enumeration members.
     */
    const Ptr<const Chunk> peekAtBack(b length = b(-1), int flags = 0) const;

    /**
     * Pops the designated part and returns it as an immutable chunk in its
     * current representation. Decreases the back offset with the length of the
     * returned chunk. If the length is unspecified, then the length of the
     * result is chosen according to the current representation. The flags
     * parameter is a combination of Chunk::PeekFlag enumeration members.
     */
    const Ptr<const Chunk> popAtBack(b length = b(-1), int flags = 0);

    /**
     * Returns true if the designated part is available in the requested
     * representation. If the length is unspecified, then the length of the
     * result is chosen according to the current representation.
     */
    template <typename T>
    bool hasAtBack(b length) const {
        CHUNK_CHECK_USAGE(b(0) <= length && length <= getDataLength(), "length is invalid");
        return content->has<T>(backIterator, length);
    }

    /**
     * Returns the designated part as an immutable chunk in the requested
     * representation. Decreases the back offset with the length of the returned
     * chunk. The flags parameter is a combination of Chunk::PeekFlag enumeration
     * members.
     */
    template <typename T>
    const Ptr<const T> peekAtBack(b length, int flags = 0) const {
        auto dataLength = getDataLength();
        CHUNK_CHECK_USAGE(b(0) <= length && length <= dataLength, "length is invalid");
        const auto& chunk = content->peek<T>(backIterator, length, flags);
        CHUNK_CHECK_IMPLEMENTATION(chunk == nullptr || chunk->getChunkLength() <= dataLength);
        return chunk;
    }

    /**
     * Pops the designated part and returns it as an immutable chunk in the
     * requested representation. If the length is unspecified, then the length
     * of the result is chosen according to the current representation. The
     * flags parameter is a combination of Chunk::PeekFlag enumeration members.
     */
    template <typename T>
    const Ptr<const T> popAtBack(b length, int flags = 0) {
        CHUNK_CHECK_USAGE(b(0) <= length && length <= getDataLength(), "length is invalid");
        const auto& chunk = peekAtBack<T>(length, flags);
        if (chunk != nullptr) {
            content->moveIterator(backIterator, chunk->getChunkLength());
            CHUNK_CHECK_IMPLEMENTATION(getDataLength() >= b(0));
        }
        return chunk;
    }
    //@}

    /** @name Data part querying functions */
    //@{
    /**
     * Returns the current length of the data part of the packet. This is the
     * same as the difference between the front and back offsets.
     * The returned value is in the range [0, getTotalLength()].
     */
    b getDataLength() const { return getTotalLength() - frontIterator.getPosition() - backIterator.getPosition(); }

    /**
     * Returns the designated data part as an immutable chunk in its current
     * representation. If the length is unspecified, then the length of the
     * result is chosen according to the current representation. The flags
     * parameter is a combination of Chunk::PeekFlag enumeration members.
     */
    const Ptr<const Chunk> peekDataAt(b offset, b length = b(-1), int flags = 0) const;

    /**
     * Returns true if the designated data part is available in the requested
     * representation. If the length is unspecified, then the length of the
     * result is chosen according to the current representation.
     */
    template <typename T>
    bool hasDataAt(b offset, b length = b(-1)) const {
        auto dataLength = getDataLength();
        CHUNK_CHECK_USAGE(b(0) <= offset && offset <= dataLength, "offset is out of range");
        CHUNK_CHECK_USAGE(b(-1) <= length && offset + length <= dataLength, "length is invalid");
        return content->has<T>(Chunk::Iterator(true, frontIterator.getPosition() + offset, -1), length == b(-1) ? -(dataLength - offset) : length);
    }

    /**
     * Returns the designated data part as an immutable chunk in the requested
     * representation. If the length is unspecified, then the length of the
     * result is chosen according to the current representation. The flags
     * parameter is a combination of Chunk::PeekFlag enumeration members.
     */
    template <typename T>
    const Ptr<const T> peekDataAt(b offset, b length = b(-1), int flags = 0) const {
        auto dataLength = getDataLength();
        CHUNK_CHECK_USAGE(b(0) <= offset && offset <= dataLength, "offset is out of range");
        CHUNK_CHECK_USAGE(b(-1) <= length && offset + length <= dataLength, "length is invalid");
        return content->peek<T>(Chunk::Iterator(true, frontIterator.getPosition() + offset, -1), length == b(-1) ? -(dataLength - offset) : length, flags);
    }

    /**
     * Returns the whole data part (excluding front and back popped parts) in the
     * current representation. The length of the returned chunk is the same as
     * the value returned by getDataLength(). The flags parameter is a combination
     * of Chunk::PeekFlag enumeration members.
     */
    const Ptr<const Chunk> peekData(int flags = 0) const {
        return peekDataAt(b(0), getDataLength(), flags);
    }

    /**
     * Returns the whole data part (excluding front and back popped parts) as a
     * sequence of bits. The length of the returned chunk is the same as the
     * value returned by getDataLength(). The flags parameter is a combination
     * of Chunk::PeekFlag enumeration members.
     */
    const Ptr<const BitsChunk> peekDataAsBits(int flags = 0) const {
        return peekDataAt<BitsChunk>(b(0), getDataLength(), flags);
    }

    /**
     * Returns the whole data part (excluding front and back popped parts) as a
     * sequence of bytes. The length of the returned chunk is the same as the
     * value returned by getDataLength(). The flags parameter is a combination
     * of Chunk::PeekFlag enumeration members.
     */
    const Ptr<const BytesChunk> peekDataAsBytes(int flags = 0) const {
        return peekDataAt<BytesChunk>(b(0), getDataLength(), flags);
    }

    /**
     * Returns the data part (excluding front and back popped parts) in the
     * requested representation. The length of the returned chunk is the same as
     * the value returned by getDataLength(). The flags parameter is a combination
     * of Chunk::PeekFlag enumeration members.
     */
    template <typename T>
    const Ptr<const T> peekData(int flags = 0) const {
        return peekDataAt<T>(b(0), getDataLength(), flags);
    }
    //@}

    /** @name Content querying functions */
    //@{
    /**
     * Returns the designated part of the packet as an immutable chunk in its
     * current representation. If the length is unspecified, then the length of
     * the result is chosen according to the current representation. The flags
     * parameter is a combination of Chunk::PeekFlag enumeration members.
     */
    const Ptr<const Chunk> peekAt(b offset, b length = b(-1), int flags = 0) const;

    /**
     * Returns true if the designated part of the packet is available in the
     * requested representation. If the length is unspecified, then the length
     * of the result is chosen according to the current representation.
     */
    template <typename T>
    bool hasAt(b offset, b length = b(-1)) const {
        auto totalLength = getTotalLength();
        CHUNK_CHECK_USAGE(b(0) <= offset && offset <= totalLength, "offset is out of range");
        CHUNK_CHECK_USAGE(b(-1) <= length && offset + length <= totalLength, "length is invalid");
        return content->has<T>(Chunk::Iterator(true, b(offset), -1), length == b(-1) ? -(totalLength - offset) : length);
    }

    /**
     * Returns the designated part of the packet as an immutable chunk in the
     * requested representation. If the length is unspecified, then the length
     * of the result is chosen according to the current representation. The
     * flags parameter is a combination of Chunk::PeekFlag enumeration members.
     */
    template <typename T>
    const Ptr<const T> peekAt(b offset, b length = b(-1), int flags = 0) const {
        auto totalLength = getTotalLength();
        CHUNK_CHECK_USAGE(b(0) <= offset && offset <= totalLength, "offset is out of range");
        CHUNK_CHECK_USAGE(b(-1) <= length && offset + length <= totalLength, "length is invalid");
        return content->peek<T>(Chunk::Iterator(true, b(offset), -1), length == b(-1) ? -(totalLength - offset) : length, flags);
    }

    /**
     * Returns the whole packet (including front and back popped parts) in the
     * current representation. The length of the returned chunk is the same as
     * the value returned by getTotalLength(). The flags parameter is a combination
     * of Chunk::PeekFlag enumeration members.
     */
    const Ptr<const Chunk> peekAll(int flags = 0) const {
        return peekAt(b(0), getTotalLength(), flags);
    }

    /**
     * Returns the whole packet (including front and back popped parts) as a
     * sequence of bits. The length of the returned chunk is the same as the
     * value returned by getTotalLength(). The flags parameter is a combination
     * of Chunk::PeekFlag enumeration members.
     */
    const Ptr<const BitsChunk> peekAllAsBits(int flags = 0) const {
        return peekAt<BitsChunk>(b(0), getTotalLength(), flags);
    }

    /**
     * Returns the whole packet (including front and back popped parts) as a
     * sequence of bytes. The length of the returned chunk is the same as the
     * value returned by getTotalLength(). The flags parameter is a combination
     * of Chunk::PeekFlag enumeration members.
     */
    const Ptr<const BytesChunk> peekAllAsBytes(int flags = 0) const {
        return peekAt<BytesChunk>(b(0), getTotalLength(), flags);
    }

    /**
     * Returns the whole packet (including front and back popped parts) in the
     * requested representation. The length of the returned chunk is the same as
     * the value returned by getTotalLength(). The flags parameter is a combination
     * of Chunk::PeekFlag enumeration members.
     */
    template <typename T>
    const Ptr<const T> peekAll(int flags = 0) const {
        return peekAt<T>(b(0), getTotalLength(), flags);
    }
    //@}

    /** @name Content erasing functions */
    //@{
    /**
     * Erases the requested amount of data from the beginning of the packet. The
     * length of front popped part must be zero before calling this function.
     */
    void eraseAtFront(b length);

    /**
     * Erases the requested amount of data from the end of the packet. The length
     * of back popped part must be zero before calling this function.
     */
    void eraseAtBack(b length);

    /**
     * Erases all data from packet and resets both front and back offsets to zero.
     */
    void eraseAll();

    /**
     * Erases the front popped part and sets the front offset to zero. The back
     * popped part and the data part of the packet isn't affected.
     */
    void trimFront();

    /**
     * Erases the back popped part and sets the back offset to zero. The front
     * popped part and the data part of the packet isn't affected.
     */
    void trimBack();

    /**
     * Removes both front and back popped parts and sets both front and back
     * offsets to zero. The data part of the packet isn't affected.
     */
    void trim();
    //@}

    /** @name Content removing functions */
    //@{
    /**
     * Removes the designated part and returns it as a mutable chunk in its
     * current representation. If the length is unspecified, then the length of
     * the result is chosen according to the current representation. The length
     * of front popped part must be zero before calling this function. The flags
     * parameter is a combination of Chunk::PeekFlag enumeration members.
     */
    const Ptr<Chunk> removeAtFront(b length = b(-1), int flags = 0);

    /**
     * Removes the designated part and returns it as a mutable chunk in its
     * current representation. If the length is unspecified, then the length of
     * the result is chosen according to the current representation. The length
     * of back popped part must be zero before calling this function. The flags
     * parameter is a combination of Chunk::PeekFlag enumeration members.
     */
    const Ptr<Chunk> removeAtBack(b length = b(-1), int flags = 0);

    /**
     * Removes the designated part and returns it as a mutable chunk in the
     * requested representation. If the length is unspecified, then the length
     * of the result is chosen according to the current representation. The
     * length of front popped part must be zero before calling this function.
     * The flags parameter is a combination of Chunk::PeekFlag enumeration members.
     */
    template <typename T>
    const Ptr<T> removeAtFront(b length = b(-1), int flags = 0) {
        CHUNK_CHECK_USAGE(b(-1) <= length && length <= getDataLength(), "length is invalid");
        CHUNK_CHECK_USAGE(frontIterator.getPosition() == b(0), "front popped length is non-zero");
        const auto& chunk = popAtFront<T>(length, flags);
        trimFront();
        return makeExclusivelyOwnedMutableChunk(chunk);
    }

    /**
     * Removes the designated part and returns it as a mutable chunk in the
     * requested representation. If the length is unspecified, then the length
     * of the result is chosen according to the current representation. The
     * length of back popped part must be zero before calling this function.
     * The flags parameter is a combination of Chunk::PeekFlag enumeration members.
     */
    template <typename T>
    const Ptr<T> removeAtBack(b length, int flags = 0) {
        CHUNK_CHECK_USAGE(b(0) <= length && length <= getDataLength(), "length is invalid");
        CHUNK_CHECK_USAGE(backIterator.getPosition() == b(0), "back popped length is non-zero");
        const auto& chunk = popAtBack<T>(length, flags);
        trimBack();
        return makeExclusivelyOwnedMutableChunk(chunk);
    }

    /**
     * Removes all data from the packet and returns it as a mutable chunk in the
     * current representation. Resets both front and back offsets to zero.
     */
    const Ptr<Chunk> removeAll();
    //@}

    /** @name Content insertion functions */
    //@{
    /**
     * Inserts the provided part at the beginning of the packet. The inserted
     * part is automatically marked immutable. The length of front popped part
     * must be zero before calling this function.
     */
    void insertAtFront(const Ptr<const Chunk>& chunk);

    /**
     * Inserts the provided part at the end of the packet. The inserted part is
     * automatically marked immutable. The length of back popped part must be
     * zero before calling this function.
     */
    void insertAtBack(const Ptr<const Chunk>& chunk);

    /**
     */
    void insertData(const Ptr<const Chunk>& chunk);

    /**
     */
    void insertDataAt(b offset, const Ptr<const Chunk>& chunk);

    /**
     */
    void insertAll(const Ptr<const Chunk>& chunk);

    /**
     */
    void insertAt(b offset, const Ptr<const Chunk>& chunk);
    //@}

    /** @name Content replacing functions */
    //@{
    /**
     */
    void replaceAtFront(const Ptr<const Chunk>& chunk, b length = b(-1), int flags = 0);

    /**
     */
    void replaceAtBack(const Ptr<const Chunk>& chunk, b length, int flags = 0);

    /**
     */
    void replaceData(const Ptr<const Chunk>& chunk, int flags = 0);

    /**
     */
    void replaceDataAt(const Ptr<const Chunk>& chunk, b offset, b length = b(-1), int flags = 0);

    /**
     */
    void replaceAll(const Ptr<const Chunk>& chunk, int flags = 0);

    /**
     */
    void replaceAt(const Ptr<const Chunk>& chunk, b offset, b length = b(-1), int flags = 0);

    /**
     */
    template <typename T>
    void replaceAtFront(const Ptr<const Chunk>& chunk, b length = b(-1), int flags = 0) {
        CHUNK_CHECK_USAGE(b(-1) <= length && length <= getDataLength(), "length is invalid");
        replaceAt<T>(chunk, getFrontOffset(), length, flags);
    }

    /**
     */
    template <typename T>
    void replaceAtBack(const Ptr<const Chunk>& chunk, b length, int flags = 0) {
        CHUNK_CHECK_USAGE(b(0) <= length && length <= getDataLength(), "length is invalid");
        replaceAt<T>(chunk, getBackOffset() - length, length, flags);
    }

    /**
     */
    template <typename T>
    void replaceData(const Ptr<const Chunk>& chunk, int flags = 0) {
        replaceAt<T>(chunk, getFrontOffset(), getDataLength(), flags);
    }

    /**
     */
    template <typename T>
    void replaceDataAt(const Ptr<const Chunk>& chunk, b offset, b length = b(-1), int flags = 0) {
        replaceAt<T>(chunk, getFrontOffset() + offset, length, flags);
    }

    /**
     */
    template <typename T>
    void replaceAll(const Ptr<const Chunk>& chunk, int flags = 0) {
        replaceAt<T>(chunk, b(0), getTotalLength(), flags);
    }

    /**
     */
    template <typename T>
    void replaceAt(const Ptr<const Chunk>& chunk, b offset, b length = b(-1), int flags = 0) {
    }
    //@}

    /** @name Content updating functions */
    //@{
    /**
     * Updates the designated part by applying the provided function on the
     * requested mutable representation. The changes are reflected in the packet.
     * If the length is unspecified, then the length of the part is chosen
     * according to the current representation. The flags parameter is a
     * combination of Chunk::PeekFlag enumeration members.
     */
    void updateAtFront(std::function<void (const Ptr<Chunk>&)> f, b length = b(-1), int flags = 0);

    /**
     * Updates the designated part by applying the provided function on the
     * requested mutable representation. The changes are reflected in the packet.
     * If the length is unspecified, then the length of the part is chosen
     * according to the current representation. The flags parameter is a
     * combination of Chunk::PeekFlag enumeration members.
     */
    void updateAtBack(std::function<void (const Ptr<Chunk>&)> f, b length, int flags = 0);

    /**
     * Updates the designated part by applying the provided function on the
     * requested mutable representation. The changes are reflected in the packet.
     * If the length is unspecified, then the length of the part is chosen
     * according to the current representation. The flags parameter is a
     * combination of Chunk::PeekFlag enumeration members.
     */
    void updateData(std::function<void (const Ptr<Chunk>&)> f, int flags = 0);

    /**
     * Updates the designated part by applying the provided function on the
     * requested mutable representation. The changes are reflected in the packet.
     * If the length is unspecified, then the length of the part is chosen
     * according to the current representation. The flags parameter is a
     * combination of Chunk::PeekFlag enumeration members.
     */
    void updateDataAt(std::function<void (const Ptr<Chunk>&)> f, b offset, b length = b(-1), int flags = 0);

    /**
     * Updates the designated part by applying the provided function on the
     * requested mutable representation. The changes are reflected in the packet.
     * If the length is unspecified, then the length of the part is chosen
     * according to the current representation. The flags parameter is a
     * combination of Chunk::PeekFlag enumeration members.
     */
    void updateAll(std::function<void (const Ptr<Chunk>&)> f, int flags = 0);

    /**
     * Updates the designated part by applying the provided function on the
     * requested mutable representation. The changes are reflected in the packet.
     * If the length is unspecified, then the length of the part is chosen
     * according to the current representation. The flags parameter is a
     * combination of Chunk::PeekFlag enumeration members.
     */
    void updateAt(std::function<void (const Ptr<Chunk>&)> f, b offset, b length = b(-1), int flags = 0);

    /**
     * Updates the designated part by applying the provided function on the
     * requested mutable representation. The changes are reflected in the packet.
     * If the length is unspecified, then the length of the part is chosen
     * according to the current representation. The flags parameter is a
     * combination of Chunk::PeekFlag enumeration members.
     */
    template <typename T>
    void updateAtFront(std::function<void (const Ptr<T>&)> f, b length = b(-1), int flags = 0) {
        CHUNK_CHECK_USAGE(b(-1) <= length && length <= getDataLength(), "length is invalid");
        updateAt<T>(f, getFrontOffset(), length, flags);
    }

    /**
     * Updates the designated part by applying the provided function on the
     * requested mutable representation. The changes are reflected in the packet.
     * If the length is unspecified, then the length of the part is chosen
     * according to the current representation. The flags parameter is a
     * combination of Chunk::PeekFlag enumeration members.
     */
    template <typename T>
    void updateAtBack(std::function<void (const Ptr<T>&)> f, b length, int flags = 0) {
        CHUNK_CHECK_USAGE(b(0) <= length && length <= getDataLength(), "length is invalid");
        updateAt<T>(f, getBackOffset() - length, length, flags);
    }

    /**
     * Updates the designated part by applying the provided function on the
     * requested mutable representation. The changes are reflected in the packet.
     * If the length is unspecified, then the length of the part is chosen
     * according to the current representation. The flags parameter is a
     * combination of Chunk::PeekFlag enumeration members.
     */
    template <typename T>
    void updateData(std::function<void (const Ptr<T>&)> f, int flags = 0) {
        updateAt<T>(f, getFrontOffset(), getDataLength(), flags);
    }

    /**
     * Updates the designated part by applying the provided function on the
     * requested mutable representation. The changes are reflected in the packet.
     * If the length is unspecified, then the length of the part is chosen
     * according to the current representation. The flags parameter is a
     * combination of Chunk::PeekFlag enumeration members.
     */
    template <typename T>
    void updateDataAt(std::function<void (const Ptr<T>&)> f, b offset, b length = b(-1), int flags = 0) {
        updateAt<T>(f, getFrontOffset() + offset, length, flags);
    }

    /**
     * Updates the designated part by applying the provided function on the
     * requested mutable representation. The changes are reflected in the packet.
     * If the length is unspecified, then the length of the part is chosen
     * according to the current representation. The flags parameter is a
     * combination of Chunk::PeekFlag enumeration members.
     */
    template <typename T>
    void updateAll(std::function<void (const Ptr<T>&)> f, int flags = 0) {
        updateAt<T>(f, b(0), getTotalLength(), flags);
    }

    /**
     * Updates the designated part by applying the provided function on the
     * requested mutable representation. The changes are reflected in the packet.
     * If the length is unspecified, then the length of the part is chosen
     * according to the current representation. The length of back popped part
     * must be zero before calling this function. The flags parameter is a
     * combination of Chunk::PeekFlag enumeration members.
     */
    template <typename T>
    void updateAt(std::function<void (const Ptr<T>&)> f, b offset, b length = b(-1), int flags = 0) {
        auto totalLength = getTotalLength();
        CHUNK_CHECK_USAGE(b(0) <= offset && offset <= totalLength, "offset is out of range");
        CHUNK_CHECK_USAGE(b(-1) <= length && offset + length <= totalLength, "length is invalid");
        const auto& chunk = peekAt<T>(offset, length, flags);
        b chunkLength = chunk->getChunkLength();
        b frontLength = offset;
        b backLength = totalLength - offset - chunkLength;
        const auto& mutableChunk = makeExclusivelyOwnedMutableChunk(chunk);
        f(mutableChunk);
        CHUNK_CHECK_USAGE(chunkLength == mutableChunk->getChunkLength(), "length is different");
        if (mutableChunk != chunk) {
            const auto& sequenceChunk = makeShared<SequenceChunk>();
            if (frontLength > b(0)) {
                const auto& frontPart = peekAt(b(0), frontLength);
                sequenceChunk->insertAtBack(frontPart);
            }
            mutableChunk->markImmutable();
            sequenceChunk->insertAtBack(mutableChunk);
            if (backLength > b(0)) {
                const auto& backPart = peekAt(totalLength - backLength, backLength);
                sequenceChunk->insertAtBack(backPart);
            }
            sequenceChunk->markImmutable();
            content = sequenceChunk;
        }
    }
    //@}

    /** @name Whole tagging functions */
    //@{
    /**
     * Returns all tags.
     */
    SharingTagSet& getTags() { return tags; }

    /**
     * Returns the number of packet tags.
     */
    int getNumTags() const {
        return tags.getNumTags();
    }

    /**
     * Returns the packet tag at the given index.
     */
    const Ptr<const TagBase> getTag(int index) const {
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
    template<typename T> const Ptr<const T> findTag() const {
        return tags.findTag<T>();
    }

    /**
     * Returns the packet tag for the provided type or returns nullptr if no such packet tag is found.
     */
    template<typename T> const Ptr<T> findTagForUpdate() {
        return tags.findTagForUpdate<T>();
    }

    /**
     * Returns the packet tag for the provided type or throws an exception if no such packet tag is found.
     */
    template<typename T> const Ptr<const T> getTag() const {
        return tags.getTag<T>();
    }

    /**
     * Returns the packet tag for the provided type or throws an exception if no such packet tag is found.
     */
    template<typename T> const Ptr<T> getTagForUpdate() {
        return tags.getTagForUpdate<T>();
    }

    /**
     * Returns a newly added packet tag for the provided type, or throws an exception if such a packet tag is already present.
     */
    template<typename T> const Ptr<T> addTag() {
        return tags.addTag<T>();
    }

    /**
     * Returns a newly added packet tag for the provided type if absent, or returns the packet tag that is already present.
     */
    template<typename T> const Ptr<T> addTagIfAbsent() {
        return tags.addTagIfAbsent<T>();
    }

    /**
     * Removes the packet tag for the provided type, or throws an exception if no such packet tag is found.
     */
    template<typename T> const Ptr<T> removeTag() {
        return tags.removeTag<T>();
    }

    /**
     * Removes the packet tag for the provided type if present, or returns nullptr if no such packet tag is found.
     */
    template<typename T> const Ptr<T> removeTagIfPresent() {
        return tags.removeTagIfPresent<T>();
    }
    //@}

    /** @name Region tagging functions */
    //@{
    /**
     * Returns the number of region tags.
     */
    int getNumRegionTags() const {
        return regionTags.getNumTags();
    }

    /**
     * Returns the region tag at the given index.
     */
    const Ptr<const TagBase> getRegionTag(int index) const {
        return regionTags.getTag(index);
    }

    /**
     * Clears the set of region tags in the given region.
     */
    void clearRegionTags(b offset = b(0), b length = b(-1)) {
        regionTags.clearTags(offset, length == b(-1) ? getTotalLength() - offset : length);
    }

    /**
     * Copies the set of region tags from the source region to the provided region.
     */
    void copyRegionTags(const Packet& source, b sourceOffset = b(0), b offset = b(0), b length = b(-1)) {
        regionTags.copyTags(source.regionTags, sourceOffset, offset, length == b(-1) ? getTotalLength() - offset : length);
    }

    /**
     * Returns the region tag for the provided type and range, or returns nullptr if no such region tag is found.
     */
    template<typename T> const Ptr<const T> findRegionTag(b offset = b(0), b length = b(-1)) const {
        return regionTags.findTag<T>(offset, length == b(-1) ? getTotalLength() - offset : length);
    }

    /**
     * Returns the region tag for the provided type and range, or throws an exception if no such region tag is found.
     */
    template<typename T> const Ptr<const T> getRegionTag(b offset = b(0), b length = b(-1)) const {
        return regionTags.getTag<T>(offset, length == b(-1) ? getTotalLength() - offset : length);
    }

    /**
     * Maps all tags in the provided range to to the function.
     */
    template<typename T> void mapAllRegionTags(b offset, b length, std::function<void (b, b, const Ptr<const T>&)> f) const {
        return regionTags.mapAllTags<const T>(offset, length == b(-1) ? getTotalLength() - offset : length, f);
    }

    /**
     * Maps all tags in the provided range to to the function.
     */
    template<typename T> void mapAllRegionTagsForUpdate(b offset, b length, std::function<void (b, b, const Ptr<T>&)> f) {
        return regionTags.mapAllTagsForUpdate<T>(offset, length == b(-1) ? getTotalLength() - offset : length, f);
    }

    /**
     * Returns all region tags for the provided type and range in a detached vector of region tags.
     */
    template<typename T> std::vector<SharingRegionTagSet::RegionTag<const T>> getAllRegionTags(b offset = b(0), b length = b(-1)) const {
        return regionTags.getAllTags<const T>(offset, length == b(-1) ? getTotalLength() - offset : length);
    }

    /**
     * Returns all region tags for the provided type and range in a detached vector of region tags.
     */
    template<typename T> std::vector<SharingRegionTagSet::RegionTag<T>> getAllRegionTagsForUpdate(b offset = b(0), b length = b(-1)) {
        return regionTags.getAllTagsForUpdate<T>(offset, length == b(-1) ? getTotalLength() - offset : length);
    }

    /**
     * Returns a newly added region tag for the provided type and range, or throws an exception if such a region tag is already present.
     */
    template<typename T> const Ptr<T> addRegionTag(b offset = b(0), b length = b(-1)) {
        return regionTags.addTag<T>(offset, length == b(-1) ? getTotalLength() - offset : length);
    }

    /**
     * Returns a newly added region tag for the provided type and range if absent, or returns the region tag that is already present.
     */
    template<typename T> const Ptr<T> addRegionTagIfAbsent(b offset = b(0), b length = b(-1)) {
        return regionTags.addTagIfAbsent<T>(offset, length == b(-1) ? getTotalLength() - offset : length);
    }

    /**
     * Returns the newly added region tags for the provided type and range where the tag is absent.
     */
    template<typename T> std::vector<SharingRegionTagSet::RegionTag<T>> addRegionTagsWhereAbsent(b offset = b(0), b length = b(-1)) {
        return regionTags.addTagsWhereAbsent<T>(offset, length == b(-1) ? getTotalLength() - offset : length);
    }

    /**
     * Removes the region tag for the provided type and range, or throws an exception if no such region tag is found.
     */
    template <typename T> const Ptr<T> removeRegionTag(b offset, b length) {
        return regionTags.removeTag<T>(offset, length == b(-1) ? getTotalLength() - offset : length);
    }

    /**
     * Removes the region tag for the provided type and range if present, or returns nullptr if no such region tag is found.
     */
    template <typename T> const Ptr<T> removeRegionTagIfPresent(b offset, b length) {
        return regionTags.removeTagIfPresent<T>(offset, length == b(-1) ? getTotalLength() - offset : length);
    }

    /**
     * Removes and returns all region tags for the provided type and range.
     */
    template <typename T> std::vector<SharingRegionTagSet::RegionTag<T>> removeRegionTagsWherePresent(b offset, b length) {
        return regionTags.removeTagsWherePresent<T>(offset, length == b(-1) ? getTotalLength() - offset : length);
    }
    //@}

    /** @name Utility functions */
    //@{
    /**
     * Returns a human readable string representation.
     */
    virtual std::string str() const override;
    //@}
};

INET_API SharingTagSet& getTags(cMessage *msg);

inline std::ostream& operator<<(std::ostream& os, const Packet *packet) { return os << packet->str(); }

inline std::ostream& operator<<(std::ostream& os, const Packet& packet) { return os << packet.str(); }

} // namespace

#endif // #ifndef __INET_PACKET_H_

