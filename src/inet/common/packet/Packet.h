//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKET_H
#define __INET_PACKET_H

#include <functional>

#include "inet/common/IPrintableObject.h"
#include "inet/common/TagBase.h"
#include "inet/common/packet/chunk/BitsChunk.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/common/packet/chunk/EmptyChunk.h"
#include "inet/common/packet/chunk/SequenceChunk.h"
#include "inet/common/packet/tag/IRegionTaggedObject.h"
#include "inet/common/packet/tag/ITaggedObject.h"

namespace inet {

#ifdef INET_WITH_SELFDOC
#define SELFDOC_FUNCTION  \
        selfDoc(__FUNCTION__, ""); \
        SelfDocTempOff
#define SELFDOC_FUNCTION_CHUNK(chunk)  \
        { auto p = chunk.get(); selfDoc(__FUNCTION__, opp_typename(typeid(*p))); } \
        SelfDocTempOff
#define SELFDOC_FUNCTION_T  \
        selfDoc(__FUNCTION__, opp_typename(typeid(T))); \
        SelfDocTempOff
#else
#define SELFDOC_FUNCTION
#define SELFDOC_FUNCTION_CHUNK(chunk)
#define SELFDOC_FUNCTION_T
#endif

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
 *  - get and set the front and back offsets
 *  - peek content anywhere in the packet
 *  - pop content from the front or the back of the packet
 *  - check the presense of content anywhere in the packet
 *  - insert content anywhere in the packet
 *  - remove and return content anywhere in the packet
 *  - update content in place destructively anywhere in the packet
 *  - replace content destructively anywhere in the packet
 *  - erase content destructively anywhere in the packet
 *  - serialize to and deserialize from a sequence of bits or bytes
 *  - copy the packet
 *  - attach tags (metadata) to the packet as a whole
 *  - attache tags (metadata) to regions of the content of the packet
 *  - convert to a human readable string
 *
 * Packets can have packet tags attached to the whole packet. Packet tags are
 * identified by their type. Tags are usually small data structures that hold
 * some relevant information.
 *
 * Packets can also have region tags are attached to a specific region of their
 * data. Region tags are identified by their type. Regions are identified by
 * their offset and length, and they are not allowed to overlap.
 */
class INET_API Packet : public cPacket, public IPrintableObject, public ITaggedObject, public IRegionTaggedObject
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
    bool isConsistent() {
        CHUNK_CHECK_IMPLEMENTATION(content->isImmutable());
        CHUNK_CHECK_IMPLEMENTATION(content->getChunkLength() == getTotalLength());
        CHUNK_CHECK_IMPLEMENTATION(getFrontOffset() <= getTotalLength());
        CHUNK_CHECK_IMPLEMENTATION(getBackOffset() <= getTotalLength());
        CHUNK_CHECK_IMPLEMENTATION(getDataLength() >= b(0));
        CHUNK_CHECK_IMPLEMENTATION(isIteratorConsistent(frontIterator));
        CHUNK_CHECK_IMPLEMENTATION(isIteratorConsistent(backIterator));
        return true;
    }

    bool isIteratorConsistent(const Chunk::Iterator& iterator) {
        Chunk::Iterator copy(iterator);
        content->seekIterator(copy, iterator.getPosition());
        return iterator.getPosition() == copy.getPosition() && (iterator.getIndex() == -1 || iterator.getIndex() == copy.getIndex());
    }
    //@}

#ifdef INET_WITH_SELFDOC
    static void selfDoc(const char * packetAction, const char *typeName);
#endif

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
    virtual void parsimPack(cCommBuffer *buffer) const override;
    virtual void parsimUnpack(cCommBuffer *buffer) override;
    //@}

    /** @name Unsupported cPacket interface functions */
    //@{
    virtual void encapsulate(cPacket *packet) override { throw cRuntimeError("Invalid operation"); }
    virtual cPacket *decapsulate() override { throw cRuntimeError("Invalid operation"); }
    virtual cPacket *getEncapsulatedPacket() const override { return nullptr; }
    virtual void setControlInfo(cObject *p) override { throw cRuntimeError("Invalid operation"); }
    virtual void setBitLength(int64_t value) override { throw cRuntimeError("Invalid operation"); }
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

    /**
     * Returns the current length of the data part of the packet. This is the
     * same as the difference between the front and back offsets.
     * The returned value is in the range [0, getTotalLength()].
     */
    b getDataLength() const { return getTotalLength() - frontIterator.getPosition() - backIterator.getPosition(); }
    //@}

    /** @name Front and back offset related functions */
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
     * Returns the back offset measured from the beginning of the packet.
     * The returned value is in the range [0, getTotalLength()].
     */
    b getBackOffset() const { return getTotalLength() - backIterator.getPosition(); }

    /**
     * Changes the back offset measured from the beginning of the packet.
     * The value must be in the range [0, getTotalLength()].
     */
    void setBackOffset(b offset);
    //@}

    /** @name Content peeking functions */
    //@{
    /**
     * Returns the designated part from the beginning of the data part of the
     * packet as an immutable chunk in the current representation. If the length
     * is unspecified, then the length of the result is chosen according to the
     * current representation. The flags parameter is a combination of
     * Chunk::PeekFlag enumeration members.
     */
    const Ptr<const Chunk> peekAtFront(b length = b(-1), int flags = 0) const {
        return peekAtFront<Chunk>(length, flags);
    }

    /**
     * Returns the designated part from the end of the data part of the packet
     * as an immutable chunk in the current representation. If the length is
     * unspecified, then the length of the result is chosen according to the
     * current representation. The flags parameter is a combination of
     * Chunk::PeekFlag enumeration members.
     */
    const Ptr<const Chunk> peekAtBack(b length = b(-1), int flags = 0) const {
        return peekAtBack<Chunk>(length, flags);
    }

    /**
     * Returns the whole data part (excluding front and back popped parts) in the
     * current representation. The length of the returned chunk is the same as
     * the value returned by getDataLength(). The flags parameter is a combination
     * of Chunk::PeekFlag enumeration members.
     */
    const Ptr<const Chunk> peekData(int flags = 0) const {
        SELFDOC_FUNCTION;
        return peekDataAt(b(0), getDataLength(), flags);
    }

    /**
     * Returns the whole data part (excluding front and back popped parts) as a
     * sequence of bits. The length of the returned chunk is the same as the
     * value returned by getDataLength(). The flags parameter is a combination
     * of Chunk::PeekFlag enumeration members.
     */
    const Ptr<const BitsChunk> peekDataAsBits(int flags = 0) const {
        SELFDOC_FUNCTION;
        return peekDataAt<BitsChunk>(b(0), getDataLength(), flags);
    }

    /**
     * Returns the whole data part (excluding front and back popped parts) as a
     * sequence of bytes. The length of the returned chunk is the same as the
     * value returned by getDataLength(). The flags parameter is a combination
     * of Chunk::PeekFlag enumeration members.
     */
    const Ptr<const BytesChunk> peekDataAsBytes(int flags = 0) const {
        SELFDOC_FUNCTION;
        return peekDataAt<BytesChunk>(b(0), getDataLength(), flags);
    }

    /**
     * Returns the whole packet (including front and back popped parts) in the
     * current representation. The length of the returned chunk is the same as
     * the value returned by getTotalLength(). The flags parameter is a combination
     * of Chunk::PeekFlag enumeration members.
     */
    const Ptr<const Chunk> peekAll(int flags = 0) const {
        SELFDOC_FUNCTION;
        return peekAt(b(0), getTotalLength(), flags);
    }

    /**
     * Returns the whole packet (including front and back popped parts) as a
     * sequence of bits. The length of the returned chunk is the same as the
     * value returned by getTotalLength(). The flags parameter is a combination
     * of Chunk::PeekFlag enumeration members.
     */
    const Ptr<const BitsChunk> peekAllAsBits(int flags = 0) const {
        SELFDOC_FUNCTION;
        return peekAt<BitsChunk>(b(0), getTotalLength(), flags);
    }

    /**
     * Returns the whole packet (including front and back popped parts) as a
     * sequence of bytes. The length of the returned chunk is the same as the
     * value returned by getTotalLength(). The flags parameter is a combination
     * of Chunk::PeekFlag enumeration members.
     */
    const Ptr<const BytesChunk> peekAllAsBytes(int flags = 0) const {
        SELFDOC_FUNCTION;
        return peekAt<BytesChunk>(b(0), getTotalLength(), flags);
    }

    /**
     * Returns the designated data part as an immutable chunk in the current
     * representation. If the length is unspecified, then the length of the
     * result is chosen according to the current representation. The flags
     * parameter is a combination of Chunk::PeekFlag enumeration members.
     */
    const Ptr<const Chunk> peekDataAt(b offset, b length = b(-1), int flags = 0) const {
        SELFDOC_FUNCTION;
        return peekDataAt<Chunk>(offset, length, flags);
    }

    /**
     * Returns the designated part of the packet as an immutable chunk in the
     * current representation. If the length is unspecified, then the length of
     * the result is chosen according to the current representation. The flags
     * parameter is a combination of Chunk::PeekFlag enumeration members.
     */
    const Ptr<const Chunk> peekAt(b offset, b length = b(-1), int flags = 0) const {
        SELFDOC_FUNCTION;
        return peekAt<Chunk>(offset, length, flags);
    }

    /**
     * Returns the designated part from the beginning of the data part of the
     * packet as an immutable chunk in the requested representation. If the
     * length is unspecified, then the length of the result is chosen according
     * to the current representation. The flags parameter is a combination of
     * Chunk::PeekFlag enumeration members.
     */
    template<typename T>
    const Ptr<const T> peekAtFront(b length = b(-1), int flags = 0) const {
        SELFDOC_FUNCTION_T;
        auto dataLength = getDataLength();
        CHUNK_CHECK_USAGE(b(-1) <= length && length <= dataLength, "length is invalid");
        const auto& chunk = content->peek<T>(frontIterator, length == b(-1) ? -dataLength : length, flags);
        CHUNK_CHECK_IMPLEMENTATION(chunk == nullptr || chunk->getChunkLength() <= dataLength);
        return chunk;
    }

    /**
     * Returns the designated part from the end of the data part of the packet
     * as an immutable chunk in the requested representation. The flags parameter
     * is a combination of Chunk::PeekFlag enumeration members.
     */
    template<typename T>
    const Ptr<const T> peekAtBack(b length, int flags = 0) const {
        SELFDOC_FUNCTION_T;
        auto dataLength = getDataLength();
        CHUNK_CHECK_USAGE(b(0) <= length && length <= dataLength, "length is invalid");
        const auto& chunk = content->peek<T>(backIterator, length, flags);
        CHUNK_CHECK_IMPLEMENTATION(chunk == nullptr || chunk->getChunkLength() <= dataLength);
        return chunk;
    }

    /**
     * Returns the data part (excluding front and back popped parts) in the
     * requested representation. The length of the returned chunk is the same as
     * the value returned by getDataLength(). The flags parameter is a combination
     * of Chunk::PeekFlag enumeration members.
     */
    template<typename T>
    const Ptr<const T> peekData(int flags = 0) const {
        SELFDOC_FUNCTION_T;
        return peekDataAt<T>(b(0), getDataLength(), flags);
    }

    /**
     * Returns the whole packet (including front and back popped parts) in the
     * requested representation. The length of the returned chunk is the same as
     * the value returned by getTotalLength(). The flags parameter is a combination
     * of Chunk::PeekFlag enumeration members.
     */
    template<typename T>
    const Ptr<const T> peekAll(int flags = 0) const {
        SELFDOC_FUNCTION_T;
        return peekAt<T>(b(0), getTotalLength(), flags);
    }

    /**
     * Returns the designated data part as an immutable chunk in the requested
     * representation. If the length is unspecified, then the length of the
     * result is chosen according to the current representation. The flags
     * parameter is a combination of Chunk::PeekFlag enumeration members.
     */
    template<typename T>
    const Ptr<const T> peekDataAt(b offset, b length = b(-1), int flags = 0) const {
        SELFDOC_FUNCTION_T;
        auto dataLength = getDataLength();
        CHUNK_CHECK_USAGE(b(0) <= offset && offset <= dataLength, "offset is out of range");
        CHUNK_CHECK_USAGE(b(-1) <= length && offset + length <= dataLength, "length is invalid");
        return content->peek<T>(Chunk::Iterator(true, frontIterator.getPosition() + offset, -1), length == b(-1) ? -(dataLength - offset) : length, flags);
    }

    /**
     * Returns the designated part of the packet as an immutable chunk in the
     * requested representation. If the length is unspecified, then the length
     * of the result is chosen according to the current representation. The
     * flags parameter is a combination of Chunk::PeekFlag enumeration members.
     */
    template<typename T>
    const Ptr<const T> peekAt(b offset, b length = b(-1), int flags = 0) const {
        SELFDOC_FUNCTION_T;
        auto totalLength = getTotalLength();
        CHUNK_CHECK_USAGE(b(0) <= offset && offset <= totalLength, "offset is out of range");
        CHUNK_CHECK_USAGE(b(-1) <= length && offset + length <= totalLength, "length is invalid");
        return content->peek<T>(Chunk::Iterator(true, b(offset), -1), length == b(-1) ? -(totalLength - offset) : length, flags);
    }
    //@}

    /** @name Content popping functions */
    //@{
    /**
     * Pops the designated part from the beginning of the data part of the
     * packet and returns it as an immutable chunk in the current representation.
     * Increases the front offset with the length of the returned chunk. If the
     * length is unspecified, then the length of the result is chosen according
     * to the current representation. The flags parameter is a combination of
     * Chunk::PeekFlag enumeration members.
     */
    const Ptr<const Chunk> popAtFront(b length = b(-1), int flags = 0) {
        return popAtFront<Chunk>(length, flags);
    }

    /**
     * Pops the designated part from the end of the data part of the packet and
     * returns it as an immutable chunk in the current representation. Decreases
     * the back offset with the length of the returned chunk. If the length is
     * unspecified, then the length of the result is chosen according to the
     * current representation. The flags parameter is a combination of
     * Chunk::PeekFlag enumeration members.
     */
    const Ptr<const Chunk> popAtBack(b length = b(-1), int flags = 0) {
        return popAtBack<Chunk>(length, flags);
    }

    /**
     * Pops the designated part from the beginning of the data part of the packet
     * and returns it as an immutable chunk in the requested representation.
     * Increases the front offset with the length of the returned chunk. If the
     * length is unspecified, then the length of the result is chosen according
     * to the current representation. The flags parameter is a combination of
     * Chunk::PeekFlag enumeration members.
     */
    template<typename T>
    const Ptr<const T> popAtFront(b length = b(-1), int flags = 0) {
        SELFDOC_FUNCTION_T;
        CHUNK_CHECK_USAGE(b(-1) <= length && length <= getDataLength(), "length is invalid");
        const auto& chunk = peekAtFront<T>(length, flags);
        if (chunk != nullptr) {
            content->moveIterator(frontIterator, chunk->getChunkLength());
            CHUNK_CHECK_IMPLEMENTATION(getDataLength() >= b(0));
        }
        return chunk;
    }

    /**
     * Pops the designated part from the end of the data part of the packet and
     * returns it as an immutable chunk in the requested representation. Decreases
     * the back offset with the length of the returned chunk. The flags parameter
     * is a combination of Chunk::PeekFlag enumeration members.
     */
    template<typename T>
    const Ptr<const T> popAtBack(b length, int flags = 0) {
        SELFDOC_FUNCTION_T;
        CHUNK_CHECK_USAGE(b(0) <= length && length <= getDataLength(), "length is invalid");
        const auto& chunk = peekAtBack<T>(length, flags);
        if (chunk != nullptr) {
            content->moveIterator(backIterator, chunk->getChunkLength());
            CHUNK_CHECK_IMPLEMENTATION(getDataLength() >= b(0));
        }
        return chunk;
    }
    //@}

    /** @name Content presence checking functions */
    //@{
    /**
     * Returns true if the designated part at the beginning of the data part of
     * the packet is completely available in the requested representation. If
     * the length is unspecified, then the length of the result is chosen
     * according to the current representation.
     */
    template<typename T>
    bool hasAtFront(b length = b(-1)) const {
        SELFDOC_FUNCTION_T;
        auto dataLength = getDataLength();
        CHUNK_CHECK_USAGE(b(-1) <= length && length <= dataLength, "length is invalid");
        return content->has<T>(frontIterator, length == b(-1) ? -dataLength : length);
    }

    /**
     * Returns true if the designated part at the end of the data part of the
     * packet is completely available in the requested representation.
     */
    template<typename T>
    bool hasAtBack(b length) const {
        SELFDOC_FUNCTION_T;
        CHUNK_CHECK_USAGE(b(0) <= length && length <= getDataLength(), "length is invalid");
        return content->has<T>(backIterator, length);
    }

    /**
     * Returns true if the whole data part of the packet is completely available
     * in the requested representation.
     */
    template<typename T>
    bool hasData() const {
        SELFDOC_FUNCTION_T;
        return content->has<T>(frontIterator, getDataLength());
    }

    /**
     * Returns true if the whole content of the packet is completely available
     * in the requested representation.
     */
    template<typename T>
    bool hasAll() const {
        SELFDOC_FUNCTION_T;
        return content->has<T>(Chunk::ForwardIterator(b(0)), getTotalLength());
    }

    /**
     * Returns true if the designated data part is completely available in the
     * requested representation. If the length is unspecified, then the length
     * of the result is chosen according to the current representation.
     */
    template<typename T>
    bool hasDataAt(b offset, b length = b(-1)) const {
        SELFDOC_FUNCTION_T;
        auto dataLength = getDataLength();
        CHUNK_CHECK_USAGE(b(0) <= offset && offset <= dataLength, "offset is out of range");
        CHUNK_CHECK_USAGE(b(-1) <= length && offset + length <= dataLength, "length is invalid");
        return content->has<T>(Chunk::Iterator(true, frontIterator.getPosition() + offset, -1), length == b(-1) ? -(dataLength - offset) : length);
    }

    /**
     * Returns true if the designated part of the packet is completely available
     * in the requested representation. If the length is unspecified, then the
     * length of the result is chosen according to the current representation.
     */
    template<typename T>
    bool hasAt(b offset, b length = b(-1)) const {
        SELFDOC_FUNCTION_T;
        auto totalLength = getTotalLength();
        CHUNK_CHECK_USAGE(b(0) <= offset && offset <= totalLength, "offset is out of range");
        CHUNK_CHECK_USAGE(b(-1) <= length && offset + length <= totalLength, "length is invalid");
        return content->has<T>(Chunk::Iterator(true, b(offset), -1), length == b(-1) ? -(totalLength - offset) : length);
    }
    //@}

    /** @name Content insertion functions */
    //@{
    /**
     * Inserts the provided chunk at the beginning of the data part of the packet.
     * The inserted chunk is automatically marked immutable. The front and back
     * offsets are not affected.
     */
    void insertAtFront(const Ptr<const Chunk>& chunk) {
        SELFDOC_FUNCTION_CHUNK(chunk);
        insertAt(chunk, getFrontOffset());
    }

    /**
     * Inserts the provided chunk at the end of the data part of the packet. The
     * inserted chunk is automatically marked immutable. The front and back offsets
     * are not affected.
     */
    void insertAtBack(const Ptr<const Chunk>& chunk) {
        SELFDOC_FUNCTION_CHUNK(chunk);
        insertAt(chunk, getBackOffset());
    }

    /**
     * Inserts the provided chunk as the data part of the packet. The inserted
     * chunk is automatically marked immutable. The front and back offsets are
     * not affected. The data part of the packet must be empty before calling
     * this function.
     */
    void insertData(const Ptr<const Chunk>& chunk) {
        SELFDOC_FUNCTION_CHUNK(chunk);
        CHUNK_CHECK_USAGE(getDataLength() == b(0), "data part is not empty");
        insertAt(chunk, getFrontOffset());
    }

    /**
     * Inserts the provided chunk as the content of the packet. The inserted
     * chunk is automatically marked immutable. The front and back offsets are
     * not affected. The packet must be completely empty before calling this
     * function.
     */
    void insertAll(const Ptr<const Chunk>& chunk) {
        SELFDOC_FUNCTION_CHUNK(chunk);
        CHUNK_CHECK_USAGE(getTotalLength() == b(0), "content is not empty");
        insertAt(chunk, b(0));
    }

    /**
     * Inserts the provided chunk at the given offset of the data part of the
     * packet. The inserted chunk is automatically marked immutable. The front
     * and back offsets are not affected. The offset must be in the range
     * [0, getDataLength()].
     */
    void insertDataAt(const Ptr<const Chunk>& chunk, b offset) {
        SELFDOC_FUNCTION_CHUNK(chunk);
        CHUNK_CHECK_USAGE(b(0) <= offset && offset <= getDataLength(), "offset is out of range");
        insertAt(chunk, getFrontOffset() + offset);
    }

    /**
     * Inserts the provided chunk at the given offset of the packet. The inserted
     * chunk is automatically marked immutable. The front and back offsets are
     * updated according to the offset parameter. The offset must be in the range
     * [0, getTotalLength()].
     */
    void insertAt(const Ptr<const Chunk>& chunk, b offset);
    //@}

    /** @name Content removing functions */
    //@{
    /**
     * Removes the designated part from the beginning of the data part of the
     * packet and returns it as a mutable chunk in the current representation.
     * If the length is unspecified, then the length of the result is chosen
     * according to the current representation. The front and back offsets are
     * not affected. The flags parameter is a combination of Chunk::PeekFlag
     * enumeration members.
     */
    const Ptr<Chunk> removeAtFront(b length = b(-1), int flags = 0) {
        return removeAtFront<Chunk>(length, flags);
    }

    /**
     * Removes the designated part from the end of the data part of the
     * packet and returns it as a mutable chunk in the current representation.
     * If the length is unspecified, then the length of the result is chosen
     * according to the current representation. The front and back offsets are
     * not affected. The flags parameter is a combination of Chunk::PeekFlag
     * enumeration members.
     */
    const Ptr<Chunk> removeAtBack(b length = b(-1), int flags = 0) {
        return removeAtBack<Chunk>(length, flags);
    }

    /**
     * Removes the data part of the packet and returns it as a mutable chunk in
     * the current representation. The front and back offsets are not affected.
     * The flags parameter is a combination of Chunk::PeekFlag enumeration members.
     */
    const Ptr<Chunk> removeData(int flags = 0) {
        return removeData<Chunk>(flags);
    }

    /**
     * Removes all content from the packet and returns it as a mutable chunk in
     * the current representation. Resets both front and back offsets to zero.
     * The flags parameter is a combination of Chunk::PeekFlag enumeration members.
     */
    const Ptr<Chunk> removeAll(int flags = 0) {
        return removeAll<Chunk>(flags);
    }

    /**
     * Removes the designated part of the data part of the packet and returns
     * it as a mutable chunk in the current representation. The front and back
     * offsets are not affected. The flags parameter is a combination of
     * Chunk::PeekFlag enumeration members.
     */
    const Ptr<Chunk> removeDataAt(b offset, b length = b(-1), int flags = 0) {
        return removeDataAt<Chunk>(offset, length, flags);
    }

    /**
     * Removes the designated part of the packet and returns it as a mutable
     * chunk in the current representation. The front and back offsets are
     * updated according to the offset and length parameters. The flags parameter
     * is a combination of Chunk::PeekFlag enumeration members.
     */
    const Ptr<Chunk> removeAt(b offset, b length = b(-1), int flags = 0) {
        return removeAt<Chunk>(offset, length, flags);
    }

    /**
     * Removes the designated part from the beginning of the data part of the
     * packet and returns it as a mutable chunk in the requested representation.
     * If the length is unspecified, then the length of the result is chosen
     * according to the current representation. The front and back offsets are
     * not affected. The flags parameter is a combination of Chunk::PeekFlag
     * enumeration members.
     */
    template<typename T>
    const Ptr<T> removeAtFront(b length = b(-1), int flags = 0) {
        SELFDOC_FUNCTION_T;
        const auto& chunk = peekAtFront<T>(length, flags);
        eraseAtFront(chunk->getChunkLength());
        return makeExclusivelyOwnedMutableChunk(chunk);
    }

    /**
     * Removes the designated part from the end of the data part of the packet
     * and returns it as a mutable chunk in the requested representation. The
     * front and back offsets are not affected. The flags parameter is a
     * combination of Chunk::PeekFlag enumeration members.
     */
    template<typename T>
    const Ptr<T> removeAtBack(b length, int flags = 0) {
        SELFDOC_FUNCTION_T;
        const auto& chunk = peekAtBack<T>(length, flags);
        eraseAtBack(chunk->getChunkLength());
        return makeExclusivelyOwnedMutableChunk(chunk);
    }

    /**
     * Removes the data part of the packet and returns it as a mutable chunk in
     * the requested representation. The front and back offsets are not affected.
     * The flags parameter is a combination of Chunk::PeekFlag enumeration members.
     */
    template<typename T>
    const Ptr<T> removeData(int flags = 0) {
        SELFDOC_FUNCTION_T;
        const auto& chunk = peekData<T>(flags);
        eraseData();
        return makeExclusivelyOwnedMutableChunk(chunk);
    }

    /**
     * Removes all content from the packet and returns it as a mutable chunk in
     * the requested representation. Resets both front and back offsets to zero.
     * The flags parameter is a combination of Chunk::PeekFlag enumeration members.
     */
    template<typename T>
    const Ptr<T> removeAll(int flags = 0) {
        SELFDOC_FUNCTION_T;
        const auto& chunk = peekAll<T>(flags);
        eraseAll();
        return makeExclusivelyOwnedMutableChunk(chunk);
    }

    /**
     * Removes the designated part of the data part of the packet and returns
     * it as a mutable chunk in the requested representation. The front and
     * back offsets are not affected. The flags parameter is a combination of
     * Chunk::PeekFlag enumeration members.
     */
    template<typename T>
    const Ptr<T> removeDataAt(b offset, b length = b(-1), int flags = 0) {
        SELFDOC_FUNCTION_T;
        const auto& chunk = peekDataAt<T>(offset, length, flags);
        eraseDataAt(offset, chunk->getChunkLength());
        return makeExclusivelyOwnedMutableChunk(chunk);
    }

    /**
     * Removes the designated part of the packet and returns it as a mutable
     * chunk in the requested representation. The front and back offsets are
     * updated according to the offset and length parameters. The flags parameter
     * is a combination of Chunk::PeekFlag enumeration members.
     */
    template<typename T>
    const Ptr<T> removeAt(b offset, b length = b(-1), int flags = 0) {
        SELFDOC_FUNCTION_T;
        const auto& chunk = peekAt<T>(offset, length, flags);
        eraseAt(offset, chunk->getChunkLength());
        return makeExclusivelyOwnedMutableChunk(chunk);
    }
    //@}

    /** @name Content replacing functions */
    //@{
    /**
     * Replaces the designated part at the beginning of the data part of the
     * packet with the provided chunk and returns the old part as a mutable
     * chunk in the current representation. If the length is unspecified, then
     * the length of the replaced part is chosen according to the current
     * representation. The inserted part is automatically marked immutable.
     * The front and back offsets are not affected. The flags parameter is a
     * combination of Chunk::PeekFlag enumeration members.
     */
    const Ptr<Chunk> replaceAtFront(const Ptr<const Chunk>& chunk, b length = b(-1), int flags = 0) {
        return replaceAtFront<Chunk>(chunk, length, flags);
    }

    /**
     * Replaces the designated part at the end of the data part of the packet
     * with the provided chunk and returns the old part as a mutable chunk in
     * the current representation. If the length is unspecified, then the length
     * of the replaced part is chosen according to the current representation.
     * The inserted part is automatically marked immutable. The front and back
     * offsets are not affected. The flags parameter is a combination of
     * Chunk::PeekFlag enumeration members.
     */
    const Ptr<Chunk> replaceAtBack(const Ptr<const Chunk>& chunk, b length = b(-1), int flags = 0) {
        return replaceAtBack<Chunk>(chunk, length, flags);
    }

    /**
     * Replaces the data part of the packet with the provided chunk and returns
     * the old content as a mutable chunk in the current representation. The front
     * and back offsets are not affected. The flags parameter is a combination
     * of Chunk::PeekFlag enumeration members.
     */
    const Ptr<Chunk> replaceData(const Ptr<const Chunk>& chunk, int flags = 0) {
        return replaceData<Chunk>(chunk, flags);
    }

    /**
     * Replaces all content in the packet with the provided chunk and returns
     * the old part as a mutable chunk in the current representation. Resets
     * both front and back offsets to zero. The flags parameter is a combination
     * of Chunk::PeekFlag enumeration members.
     */
    const Ptr<Chunk> replaceAll(const Ptr<const Chunk>& chunk, int flags = 0) {
        return replaceAll<Chunk>(chunk, flags);
    }

    /**
     * Replaces the designated part of the data part of the packet with the
     * provided chunk and returns the old content as a mutable chunk in the
     * current representation. The front and back offsets are not affected.
     * The flags parameter is a combination of Chunk::PeekFlag enumeration members.
     */
    const Ptr<Chunk> replaceDataAt(const Ptr<const Chunk>& chunk, b offset, b length = b(-1), int flags = 0) {
        return replaceDataAt<Chunk>(chunk, offset, length, flags);
    }

    /**
     * Replaces the designated part of the packet with the provided chunk and
     * returns the old content as a mutable chunk in the current representation.
     * The front and back offsets are updated according to the offset and length
     * parameters. The flags parameter is a combination of Chunk::PeekFlag
     * enumeration members.
     */
    const Ptr<Chunk> replaceAt(const Ptr<const Chunk>& chunk, b offset, b length = b(-1), int flags = 0) {
        return replaceAt<Chunk>(chunk, offset, length, flags);
    }

    /**
     * Replaces the designated part at the beginning of the data part of the
     * packet with the provided chunk and returns the old part as a mutable
     * chunk in the requested representation. If the length is unspecified,
     * then the length of the replaced part is chosen according to the current
     * representation. The inserted part is automatically marked immutable.
     * The front and back offsets are not affected. The flags parameter is a
     * combination of Chunk::PeekFlag enumeration members.
     */
    template<typename T>
    const Ptr<T> replaceAtFront(const Ptr<const Chunk>& chunk, b length = b(-1), int flags = 0) {
        SELFDOC_FUNCTION_T;
        CHUNK_CHECK_USAGE(b(-1) <= length && length <= getDataLength(), "length is invalid");
        return replaceAt<T>(chunk, getFrontOffset(), length, flags);
    }

    /**
     * Replaces the designated part at the end of the data part of the packet
     * with the provided chunk and returns the old part as a mutable chunk in
     * the requested representation. The inserted part is automatically marked
     * immutable. The front and back offsets are not affected. The flags
     * parameter is a combination of Chunk::PeekFlag enumeration members.
     */
    template<typename T>
    const Ptr<T> replaceAtBack(const Ptr<const Chunk>& chunk, b length, int flags = 0) {
        SELFDOC_FUNCTION_T;
        CHUNK_CHECK_USAGE(b(0) <= length && length <= getDataLength(), "length is invalid");
        return replaceAt<T>(chunk, getBackOffset() - length, length, flags);
    }

    /**
     * Replaces the data part of the packet with the provided chunk and returns
     * the old content as a mutable chunk in the requested representation. The
     * inserted part is automatically marked immutable. The front and back offsets
     * are not affected. The flags parameter is a combination of Chunk::PeekFlag
     * enumeration members.
     */
    template<typename T>
    const Ptr<T> replaceData(const Ptr<const Chunk>& chunk, int flags = 0) {
        SELFDOC_FUNCTION_T;
        return replaceAt<T>(chunk, getFrontOffset(), getDataLength(), flags);
    }

    /**
     * Replaces all content in the packet with the provided chunk and returns
     * the old content as a mutable chunk in the requested representation.
     * The inserted part is automatically marked immutable. Resets both front
     * and back offsets to zero. The flags parameter is a combination of
     * Chunk::PeekFlag enumeration members.
     */
    template<typename T>
    const Ptr<T> replaceAll(const Ptr<const Chunk>& chunk, int flags = 0) {
        SELFDOC_FUNCTION_T;
        return replaceAt(chunk, b(0), getTotalLength(), flags);
    }

    /**
     * Replaces the designated part of the data part of the packet with the
     * provided chunk and returns the old content as a mutable chunk in the
     * requested representation. The front and back offsets are not affected.
     * The flags parameter is a combination of Chunk::PeekFlag enumeration members.
     */
    template<typename T>
    const Ptr<T> replaceDataAt(const Ptr<const Chunk>& chunk, b offset, b length = b(-1), int flags = 0) {
        SELFDOC_FUNCTION_T;
        CHUNK_CHECK_USAGE(b(0) <= offset && offset <= getDataLength(), "offset is out of range");
        CHUNK_CHECK_USAGE(b(-1) <= length && offset + length <= getDataLength(), "length is invalid");
        return replaceAt<T>(chunk, getFrontOffset() + offset, length, flags);
    }

    /**
     * Replaces the designated part of the packet with the provided chunk and
     * returns the old content as a mutable chunk in the requested representation.
     * The front and back offsets are updated accordign to the length and offset
     * parameters. The flags parameter is a combination of Chunk::PeekFlag
     * enumeration members.
     *
     * TODO current limitation: the new chunk must be the same size as the old one
     */
    template<typename T>
    const Ptr<T> replaceAt(const Ptr<const Chunk>& chunk, b offset, b length = b(-1), int flags = 0) {
        SELFDOC_FUNCTION_T;
        auto totalLength = getTotalLength();
        CHUNK_CHECK_USAGE(chunk != nullptr, "chunk is nullptr");
        CHUNK_CHECK_USAGE(chunk->getChunkLength() > b(0), "chunk is empty");
        CHUNK_CHECK_USAGE(b(0) <= offset && offset <= totalLength, "offset is out of range");
        CHUNK_CHECK_USAGE(b(-1) <= length && offset + length <= totalLength, "length is invalid");
        constPtrCast<Chunk>(chunk)->markImmutable();
        const auto& oldChunk = peekAt<T>(offset, length, flags);
        b chunkLength = oldChunk->getChunkLength();
        b frontLength = offset;
        const auto& frontPart = frontLength > b(0) ? peekAt(b(0), frontLength) : nullptr;
        b backLength = totalLength - offset - chunkLength;
        const auto& backPart = backLength > b(0) ? peekAt(totalLength - backLength, backLength) : nullptr;
        content = makeShared<EmptyChunk>();
        const auto& result = makeExclusivelyOwnedMutableChunk(oldChunk);
        CHUNK_CHECK_USAGE(chunkLength == chunk->getChunkLength(), "length is different");
        if (frontLength == b(0) && backLength == b(0))
            content = chunk;
        else {
            const auto& sequenceChunk = makeShared<SequenceChunk>();
            if (frontLength > b(0))
                sequenceChunk->insertAtBack(frontPart);
            sequenceChunk->insertAtBack(chunk);
            if (backLength > b(0))
                sequenceChunk->insertAtBack(backPart);
            sequenceChunk->markImmutable();
            content = sequenceChunk;
        }
        CHUNK_CHECK_IMPLEMENTATION(isConsistent());
        return result;
    }
    //@}

    /** @name Content updating functions */
    //@{
    /**
     * Updates the designated part at the beginning of the data part of the packet by applying
     * the provided function on the requested mutable representation. The
     * changes are reflected in the packet. If the length is unspecified, then
     * the length of the part is chosen according to the current representation.
     * The front and back offsets are not affected. The flags parameter is a
     * combination of Chunk::PeekFlag enumeration members.
     */
    void updateAtFront(std::function<void(const Ptr<Chunk>&)> f, b length = b(-1), int flags = 0) {
        SELFDOC_FUNCTION;
        updateAtFront<Chunk>(f, length, flags);
    }

    /**
     * Updates the designated part at the end of the data part of the packet by applying the
     * provided function on the requested mutable representation. The changes
     * are reflected in the packet. If the length is unspecified, then the
     * length of the part is chosen according to the current representation.
     * The front and back offsets are not affected. The flags parameter is a
     * combination of Chunk::PeekFlag enumeration members.
     */
    void updateAtBack(std::function<void(const Ptr<Chunk>&)> f, b length = b(-1), int flags = 0) {
        SELFDOC_FUNCTION;
        updateAtBack<Chunk>(f, length, flags);
    }

    /**
     * Updates the data part of the packet by applying the provided function on
     * the requested mutable representation. The changes are reflected in the
     * packet. The front and back offsets are not affected. The flags parameter
     * is a combination of Chunk::PeekFlag enumeration members.
     */
    void updateData(std::function<void(const Ptr<Chunk>&)> f, int flags = 0) {
        SELFDOC_FUNCTION;
        updateData<Chunk>(f, flags);
    }

    /**
     * Updates the designated part by applying the provided function on the
     * requested mutable representation. The changes are reflected in the packet.
     * The front and back offsets are not affected. The flags parameter is a
     * combination of Chunk::PeekFlag enumeration members.
     */
    void updateAll(std::function<void(const Ptr<Chunk>&)> f, int flags = 0) {
        SELFDOC_FUNCTION;
        updateAll<Chunk>(f, flags);
    }

    /**
     * Updates the designated part by applying the provided function on the
     * requested mutable representation. The changes are reflected in the packet.
     * If the length is unspecified, then the length of the part is chosen
     * according to the current representation. The front and back offsets are
     * not affected. The flags parameter is a combination of Chunk::PeekFlag
     * enumeration members.
     */
    void updateDataAt(std::function<void(const Ptr<Chunk>&)> f, b offset, b length = b(-1), int flags = 0) {
        SELFDOC_FUNCTION;
        updateDataAt<Chunk>(f, offset, length, flags);
    }

    /**
     * Updates the designated part by applying the provided function on the
     * requested mutable representation. The changes are reflected in the packet.
     * If the length is unspecified, then the length of the part is chosen
     * according to the current representation. The front and back offsets are
     * updated according to the offset and length parameters. The flags parameter
     * is a combination of Chunk::PeekFlag enumeration members.
     */
    void updateAt(std::function<void(const Ptr<Chunk>&)> f, b offset, b length = b(-1), int flags = 0) {
        SELFDOC_FUNCTION;
        updateAt<Chunk>(f, offset, length, flags);
    }

    /**
     * Updates the designated part at the beginning of the data part of the
     * packet by applying the provided function on the
     * requested mutable representation. The changes are reflected in the packet.
     * If the length is unspecified, then the length of the part is chosen
     * according to the current representation. The front and back offsets
     * are not affected. The flags parameter is a combination of Chunk::PeekFlag
     * enumeration members.
     */
    template<typename T>
    void updateAtFront(std::function<void(const Ptr<T>&)> f, b length = b(-1), int flags = 0) {
        SELFDOC_FUNCTION_T;
        CHUNK_CHECK_USAGE(b(-1) <= length && length <= getDataLength(), "length is invalid");
        updateAt<T>(f, getFrontOffset(), length, flags);
    }

    /**
     * Updates the designated part at the end of the data part of the packet by
     * applying the provided function on the requested mutable representation.
     * The changes are reflected in the packet. The front and back offsets are
     * not affected. The flags parameter is a combination of Chunk::PeekFlag
     * enumeration members.
     */
    template<typename T>
    void updateAtBack(std::function<void(const Ptr<T>&)> f, b length, int flags = 0) {
        SELFDOC_FUNCTION_T;
        CHUNK_CHECK_USAGE(b(0) <= length && length <= getDataLength(), "length is invalid");
        updateAt<T>(f, getBackOffset() - length, length, flags);
    }

    /**
     * Updates the designated part by applying the provided function on the
     * requested mutable representation. The changes are reflected in the packet.
     * The front and back offsets are not affected. The flags parameter is a
     * combination of Chunk::PeekFlag enumeration members.
     */
    template<typename T>
    void updateData(std::function<void(const Ptr<T>&)> f, int flags = 0) {
        SELFDOC_FUNCTION_T;
        updateAt<T>(f, getFrontOffset(), getDataLength(), flags);
    }

    /**
     * Updates the designated part by applying the provided function on the
     * requested mutable representation. The changes are reflected in the packet.
     * The front and back offsets are not affected. The flags parameter is a
     * combination of Chunk::PeekFlag enumeration members.
     */
    template<typename T>
    void updateAll(std::function<void(const Ptr<T>&)> f, int flags = 0) {
        SELFDOC_FUNCTION_T;
        updateAt<T>(f, b(0), totalLength, flags);
    }

    /**
     * Updates the designated part by applying the provided function on the
     * requested mutable representation. The changes are reflected in the packet.
     * If the length is unspecified, then the length of the part is chosen
     * according to the current representation. The front and back offsets are
     * not affected. The flags parameter is a combination of Chunk::PeekFlag
     * enumeration members.
     */
    template<typename T>
    void updateDataAt(std::function<void(const Ptr<T>&)> f, b offset, b length = b(-1), int flags = 0) {
        SELFDOC_FUNCTION_T;
        CHUNK_CHECK_USAGE(b(0) <= offset && offset <= getDataLength(), "offset is out of range");
        CHUNK_CHECK_USAGE(b(-1) <= length && offset + length <= getDataLength(), "length is invalid");
        updateAt<T>(f, getFrontOffset() + offset, length, flags);
    }

    /**
     * Updates the designated part by applying the provided function on the
     * requested mutable representation. The changes are reflected in the packet.
     * If the length is unspecified, then the length of the part is chosen
     * according to the current representation. The front and back offsets are
     * updated according to the offset and length parameters. The flags parameter
     * is a combination of Chunk::PeekFlag enumeration members.
     *
     * TODO current limitation: the chunk size cannot be changed
     */
    template<typename T>
    void updateAt(std::function<void(const Ptr<T>&)> f, b offset, b length = b(-1), int flags = 0) {
        SELFDOC_FUNCTION_T;
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

    /** @name Content erasing functions */
    //@{
    /**
     * Erases the requested amount of data from the beginning of the data part
     * of the packet. The front and back offsets are not affected. The length
     * parameter must be in the range [0, getDataLength()].
     */
    void eraseAtFront(b length) {
        SELFDOC_FUNCTION;
        CHUNK_CHECK_USAGE(b(-1) <= length && length <= getDataLength(), "length is invalid");
        eraseAt(getFrontOffset(), length);
    }

    /**
     * Erases the requested amount of data from the end of the data part of the
     * packet. The front and back offsets are not affected. The length parameter
     * must be in the range [0, getDataLength()].
     */
    void eraseAtBack(b length) {
        SELFDOC_FUNCTION;
        CHUNK_CHECK_USAGE(b(-1) <= length && length <= getDataLength(), "length is invalid");
        eraseAt(getBackOffset() - length, length);
    }

    /**
     * Erases the whole data part of the packet. The front and back offsets are
     * not affected.
     */
    void eraseData() {
        SELFDOC_FUNCTION;
        eraseAt(getFrontOffset(), getDataLength());
    }

    /**
     * Erases all content from the packet and sets both front and back offsets
     * to zero.
     */
    void eraseAll() {
        SELFDOC_FUNCTION;
        eraseAt(b(0), getTotalLength());
    }

    /**
     * Erases the designated part of the data part of the packet. The front and
     * back offsets are not affected. The designated part must be in the range
     * [0, getDataLength()].
     */
    void eraseDataAt(b offset, b length) {
        SELFDOC_FUNCTION;
        auto dataLength = getDataLength();
        CHUNK_CHECK_USAGE(b(0) <= offset && offset <= dataLength, "offset is out of range");
        CHUNK_CHECK_USAGE(b(-1) <= length && offset + length <= dataLength, "length is invalid");
        eraseAt(getFrontOffset() + offset, length);
    }

    /**
     * Erases the designated content of the packet. The front and back offsets
     * are updated according to the offset and length parameters. The designated
     * part must be in the range [0, getTotalLength()].
     */
    void eraseAt(b offset, b length);

    /**
     * Erases the front popped part of the packet and sets the front offset to
     * zero. The back popped part and the data part of the packet isn't affected.
     */
    void trimFront();

    /**
     * Erases the back popped part of the packet and sets the back offset to
     * zero. The front popped part and the data part of the packet isn't affected.
     */
    void trimBack();

    /**
     * Erases both the front and the back popped parts of the packet and sets
     * both the front and the back offsets to zero. The data part of the packet
     * isn't affected.
     */
    void trim();
    //@}

    /** @name Whole tagging functions */
    //@{
    /**
     * Returns all tags.
     */
    virtual SharingTagSet& getTags() override { return tags; }

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
     * Returns true if the packet tag for the provided type is found.
     */
    template<typename T> bool hasTag() const {
        return tags.findTag<T>() != nullptr;
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
     * Returns all region tags.
     */
    virtual SharingRegionTagSet& getRegionTags() override { return regionTags; }

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
    template<typename T> void mapAllRegionTags(b offset, b length, std::function<void(b, b, const Ptr<const T>&)> f) const {
        return regionTags.mapAllTags<const T>(offset, length == b(-1) ? getTotalLength() - offset : length, f);
    }

    /**
     * Maps all tags in the provided range to to the function.
     */
    template<typename T> void mapAllRegionTagsForUpdate(b offset, b length, std::function<void(b, b, const Ptr<T>&)> f) {
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
    template<typename T> const Ptr<T> removeRegionTag(b offset, b length) {
        return regionTags.removeTag<T>(offset, length == b(-1) ? getTotalLength() - offset : length);
    }

    /**
     * Removes the region tag for the provided type and range if present, or returns nullptr if no such region tag is found.
     */
    template<typename T> const Ptr<T> removeRegionTagIfPresent(b offset, b length) {
        return regionTags.removeTagIfPresent<T>(offset, length == b(-1) ? getTotalLength() - offset : length);
    }

    /**
     * Removes and returns all region tags for the provided type and range.
     */
    template<typename T> std::vector<SharingRegionTagSet::RegionTag<T>> removeRegionTagsWherePresent(b offset, b length) {
        return regionTags.removeTagsWherePresent<T>(offset, length == b(-1) ? getTotalLength() - offset : length);
    }
    //@}

    /** @name Utility functions */
    //@{
    /**
     * Returns the full name of the packet. The full name consists of packet
     * name followed by either 'start', 'progress', or 'end'.
     */
    virtual const char *getFullName() const override;

    /**
     * Prints a human readable string representation to the output stream. The
     * level argument controls the printed level of detail. The flags argument
     * allows formatted and multiline output.
     */
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    /**
     * Returns a human readable string representation.
     */
    virtual std::string str() const override;
    //@}
};

#undef SELFDOC_FUNCTION
#undef SELFDOC_FUNCTION_CHUNK
#undef SELFDOC_FUNCTION_T

} // namespace

#endif

