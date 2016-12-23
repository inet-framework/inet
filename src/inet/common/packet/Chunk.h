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

#ifndef __INET_CHUNK_H_
#define __INET_CHUNK_H_

#include <memory>
#include "inet/common/packet/ByteInputStream.h"
#include "inet/common/packet/ByteOutputStream.h"

namespace inet {

/**
 * This class represents a piece of data that is usually part of a packet or
 * some other data such as a protocol buffer. The chunk interface is designed
 * to be very flexible in terms of alternative representations. For example,
 * when the actual bytes in the data is irrelevant, a chunk can be as simple as
 * an integer specifying its length. Contrary, sometimes it might be necessary
 * to represent the data bytes as is. For example, when one is using an external
 * program to send or receive the data. But most often it's better to have a
 * representation that is easy to inspect and understand during debugging.
 * Fortunately this representation can be easily generated using the omnetpp
 * message compiler. In any case, chunks can always be converted to and from a
 * sequence of bytes using the corresponding serializer.
 *
 * TODO: document polymorphism, nesting, automatic compacting, crc handling, implementing lossy channels
 *
 * Chunks can represent data in different ways:
 *  - ByteCountChunk contains a length field only
 *  - BytesChunk contains a sequence of bytes
 *  - SliceChunk contains a slice of another chunk
 *  - SequenceChunk contains a sequence of other chunks
 *  - cPacketChunk contains a cPacket for compatibility
 *  - message compiler generated chunks contain various user defined fields
 *
 * Chunks are initially:
 *  - mutable, then may become immutable (but never the other way around)
 *    immutable chunks cannot be changed anymore (sharing data)
 *  - complete, then may become incomplete (but never the other way around)
 *    incomplete chunks are not totally filled in (deserializing insufficient data)
 *  - correct, then may become incorrect (but never the other way around)
 *    incorrect chunks contain bit errors (using lossy channels)
 *  - properly represented, then may become improperly represented (but never the other way around)
 *    improperly represented chunks misrepresent their data (deserializing data)
 *
 * Chunks can be safely shared using std::shared_ptr. In fact, the reason for
 * having immutable chunks is to allow for efficient sharing using shared
 * pointers. For example, when a Packet data structure is duplicated the copy
 * can share immutable chunks with the original.
 *
 * In general, chunks support the following operations:
 *  - insert to the beginning or end
 *  - remove from the beginning or end
 *  - query length and peek an arbitrary part
 *  - serialize to and deserialize from a sequence of bytes
 *  - copy to a new mutable chunk
 *  - convert to a human readable string
 *
 * General rules for peeking into a chunk:
 * 1) Peeking never returns
 *    a) a SliceChunk containing another SliceChunk
 *    b) a SliceChunk containing the whole of another chunk
 *    c) a SliceChunk containing a BytesChunk
 *    d) a SliceChunk containing a ByteCountChunk
 *    e) a SequenceChunk containing connecting BytesChunks
 *    f) a SequenceChunk containing connecting ByteCountChunks
 *    g) a SequenceChunk containing another SequenceChunk
 *    h) a SequenceChunk containing only one chunk
 * 2) Peeking (various special cases)
 *    a) an empty part of the chunk returns nullptr
 *    b) the whole of the chunk returns the chunk
 *    c) any part that is directly represented by another chunk returns that chunk
 * 3) Peeking without providing a return type for a
 *    a) BytesChunk always returns a BytesChunk containing the bytes
 *       of the requested part
 *    b) ByteCountChunk always returns a ByteCountChunk containing the requested length
 *    c) SliceChunk always returns a SliceChunk containing the requested slice
 *       of the chunk that is used by the original SliceChunk
 *    d) SequenceChunk may return
 *       - an element chunk
 *       - a SliceChunk of an element chunk
 *       - a SliceChunk using the original SequenceChunk
 *    e) any other chunk returns a SliceChunk
 * 4) Peeking with providing a return type always returns a chunk of the
 *    requested type (or a subtype thereof)
 *    a) Peeking with a ByteCountChunk return type for any chunk returns a
 *       ByteCountChunk containing the requested byte length
 *    b) Peeking with a BytesChunk return type for any chunk returns a
 *       BytesChunk containing a part of the serialized bytes of the
 *       original chunk
 *    c) Peeking with a SliceChunk return type for any chunk returns a
 *       SliceChunk containing the original chunk
 *    d) Peeking with a SequenceChunk return type is an error
 *    e) Peeking with a any other return type for any chunk returns a chunk of
 *       the requested type containing data deserialized from the bytes that
 *       were serialized from the original chunk
 */
// TODO: performance related; avoid iteration in SequenceChunk::getChunkLength, avoid peek for simplifying, use vector instead of deque, reverse order for frequent prepends?
// TODO: review insert functions for the chunk->insert(chunk) case
// TODO: consider not allowing appending mutable chunks?
// TODO: consider adding a simplify function as peek(0, getChunkLength())?
// TODO: consider returning a result chunk from insertAtBeginning and insertAtEnd
// TODO: when peeking an incomplete fixed size header, what does getChunkLength() return for such a header?
// TODO: peek is misleading with BytesChunk and default length, consider introducing an enum to replace -1 length values
// TODO: chunks may be incorrect/incomplete/improper, this is inconvenient for each protocol to check all chunks in the data part of the packet
// TODO: what shall we do about optional subfields such as Address2, Address3, QoS, etc.?
//       message compiler could support @optional fields, inspectors could hide them, etc.
// TODO: how do we represent the random subfield sequences (options) right after mandatory header part?, possible alternatives:
// packet
// - IpHeader
//   - IpHeaderMandatory
//   - IpOption1
//   - IpOption2
// - TcpHeader
//   - TcpHeaderMandatory
//   - TcpOption1
//   - TcpOption2
//
// packet
// - IpHeader
// - IpOptions
//   - IpOption1
//   - IpOption2
// - TcpHeader
// - TcpOptions
//   - TcpOption1
//   - TcpOption2
//
// packet
// - IpHeader
// - IpOption1
// - IpOption2
// - TcpHeader
// - TcpOption1
// - TcpOption2
class INET_API Chunk : public cObject, public std::enable_shared_from_this<Chunk>
{
  protected:
    /**
     * This enum specifies bitmasks for the flags field.
     */
    enum Flag {
        FLAG_IMMUTABLE  = 1,
        FLAG_INCOMPLETE = 2,
        FLAG_INCORRECT  = 4,
        FLAG_IMPROPERLY_REPRESENTED = 8,
    };

  public:
    /**
     * This enum is used to avoid std::dynamic_cast and std::dynamic_pointer_cast.
     */
    enum Type {
        TYPE_BYTECOUNT,
        TYPE_BYTES,
        TYPE_SLICE,
        TYPE_CPACKET,
        TYPE_SEQUENCE,
        TYPE_FIELDS
    };

    class INET_API Iterator
    {
      protected:
        const bool isForward_;
        int64_t position;
        int index;

      public:
        Iterator(int64_t position) : isForward_(true), position(position), index(position == 0 ? 0 : -1) { }
        explicit Iterator(bool isForward, int64_t position, int index) : isForward_(isForward), position(position), index(index) { }
        Iterator(const Iterator& other) : isForward_(other.isForward_), position(other.position), index(other.index) { }

        bool isForward() const { return isForward_; }
        bool isBackward() const { return !isForward_; }

        int64_t getPosition() const { return position; }
        void setPosition(uint64_t position) { this->position = position; }

        int getIndex() const { return index; }
        void setIndex(int index) { this->index = index; }
    };

    /**
     * Position and index are measured from the beginning.
     */
    class INET_API ForwardIterator : public Iterator
    {
      public:
        ForwardIterator(int64_t position) : Iterator(true, position, -1) { }
        explicit ForwardIterator(int64_t position, int index) : Iterator(true, position, index) { }
    };

    /**
     * Position and index are measured from the end.
     */
    class INET_API BackwardIterator : public Iterator
    {
      public:
        BackwardIterator(int64_t position) : Iterator(false, position, -1) { }
        explicit BackwardIterator(int64_t position, int index) : Iterator(false, position, index) { }
    };

  public:
    /**
     * Peeking some part into a chunk that requires automatic serialization
     * will throw an exception when implicit chunk serialization is disabled.
     */
    static bool enableImplicitChunkSerialization;

  protected:
    /**
     * The boolean flags merged into a single integer.
     */
    uint16_t flags;

  protected:
    virtual void handleChange() override;

    // NOTE: these peek functions are only used to support the peek template functions
    virtual std::shared_ptr<Chunk> peekSliceChunk(const Iterator& iterator, int64_t length = -1) const { assert(false); }
    virtual std::shared_ptr<Chunk> peekSequenceChunk1(const Iterator& iterator, int64_t length = -1) const { assert(false); }
    virtual std::shared_ptr<Chunk> peekSequenceChunk2(const Iterator& iterator, int64_t length = -1) const { assert(false); }

  protected:
    /**
     * Creates a new chunk of the given type that represents the designated part
     * of the provided chunk. The designated part starts at the provided offset
     * and has the provided length, both measured in bytes. This function isn't
     * a constructor to allow creating instances of message compiler generated
     * field based chunk classes.
     */
    static std::shared_ptr<Chunk> createChunk(const std::type_info& typeInfo, const std::shared_ptr<Chunk>& chunk, int64_t offset, int64_t length);

    template <typename T>
    std::shared_ptr<T> doPeek(const Iterator& iterator, int64_t length = -1) const {
        assertImmutable();
        const auto& chunk = T::createChunk(typeid(T), const_cast<Chunk *>(this)->shared_from_this(), iterator.getPosition(), length);
        chunk->markImmutable();
        if ((chunk->isComplete() && length == -1) || length == chunk->getChunkLength())
            return std::dynamic_pointer_cast<T>(chunk);
        else
            return nullptr;
    }

  public:
    /** @name Constructors, destructors and duplication related functions */
    //@{
    Chunk();
    Chunk(const Chunk& other);

    /**
     * Returns a mutable copy of this chunk in a shared pointer.
     */
    virtual std::shared_ptr<Chunk> dupShared() const { return std::shared_ptr<Chunk>(static_cast<Chunk *>(dup())); };
    //@}

    /** @name Mutability related functions */
    //@{
    // NOTE: there is no markMutable() intentionally
    bool isMutable() const { return !(flags & FLAG_IMMUTABLE); }
    bool isImmutable() const { return flags & FLAG_IMMUTABLE; }
    void assertMutable() const { assert(isMutable()); }
    void assertImmutable() const { assert(isImmutable()); }
    void markMutableIfExclusivelyOwned() {
        // NOTE: one for external reference and one for local variable
        assert(shared_from_this().use_count() == 2);
        flags &= ~FLAG_IMMUTABLE;
    }
    virtual void markImmutable() { flags |= FLAG_IMMUTABLE; }
    //@}

    /** @name Completeness related functions */
    //@{
    // NOTE: there is no markComplete() intentionally
    bool isComplete() const { return !(flags & FLAG_INCOMPLETE); }
    bool isIncomplete() const { return flags & FLAG_INCOMPLETE; }
    void assertComplete() const { assert(isComplete()); }
    void assertIncomplete() const { assert(isIncomplete()); }
    virtual void markIncomplete() { flags |= FLAG_INCOMPLETE; }
    //@}

    /** @name Correctness related functions */
    //@{
    // NOTE: there is no markCorrect() intentionally
    bool isCorrect() const { return !(flags & FLAG_INCORRECT); }
    bool isIncorrect() const { return flags & FLAG_INCORRECT; }
    void assertCorrect() const { assert(isCorrect()); }
    void assertIncorrect() const { assert(isIncorrect()); }
    virtual void markIncorrect() { flags |= FLAG_INCORRECT; }
    //@}

    /** @name Proper representation related functions */
    //@{
    // NOTE: there is no markProperlyRepresented() intentionally
    bool isProperlyRepresented() const { return !(flags & FLAG_IMPROPERLY_REPRESENTED); }
    bool isImproperlyRepresented() const { return flags & FLAG_IMPROPERLY_REPRESENTED; }
    void assertProperlyRepresented() const { assert(isProperlyRepresented()); }
    void assertImproperlyRepresented() const { assert(isImproperlyRepresented()); }
    virtual void markImproperlyRepresented() { flags |= FLAG_IMPROPERLY_REPRESENTED; }
    //@}

    /** @name Iteration related functions */
    //@{
    virtual void moveIterator(Iterator& iterator, int64_t length) const { iterator.setPosition(iterator.getPosition() + length); }
    virtual void seekIterator(Iterator& iterator, int64_t offset) const { iterator.setPosition(offset); }
    //@}

    /** @name Inserting data related functions */
    //@{
    /**
     * Returns true if this chunk is capable of representing the result.
     */
    virtual bool canInsertAtBeginning(const std::shared_ptr<Chunk>& chunk) { return false; }

    /**
     * Returns true if this chunk is capable of representing the result.
     */
    virtual bool canInsertAtEnd(const std::shared_ptr<Chunk>& chunk) { return false; }

    /**
     * Inserts the provided chunk at the beginning of this chunk.
     */
    virtual void insertAtBeginning(const std::shared_ptr<Chunk>& chunk) { assertMutable(); assert(false); }

    /**
     * Inserts the provided chunk at the end of this chunk.
     */
    virtual void insertAtEnd(const std::shared_ptr<Chunk>& chunk) { assertMutable(); assert(false); }
    //@}

    /** @name Removing data related functions */
    //@{
    /**
     * Returns true if this chunk is capable of representing the result.
     */
    virtual bool canRemoveFromBeginning(int64_t length) { return false; }

    /**
     * Returns true if this chunk is capable of representing the result.
     */
    virtual bool canRemoveFromEnd(int64_t length) { return false; }

    /**
     * Removes the requested number of bytes from the beginning of this chunk
     * and returns true if the removal was successful.
     */
    virtual void removeFromBeginning(int64_t length) { assertMutable(); assert(false); }

    /**
     * Removes the requested number of bytes from the end of this chunk and
     * returns true if the removal was successful.
     */
    virtual void removeFromEnd(int64_t length) { assertMutable(); assert(false); }
    //@}

    /** @name Chunk querying related functions */
    //@{
    /**
     * Returns the type of this chunk as an enum member. This can be used to
     * avoid expensive std::dynamic_cast and std::dynamic_pointer_cast operators.
     */
    virtual Type getChunkType() const = 0;

    /**
     * Returns the length of data measured in bytes represented by this chunk.
     */
    virtual int64_t getChunkLength() const = 0;

    /**
     * Returns the designated part of the data represented by this chunk in its
     * default representation. If the length is unspecified, then the length of
     * the result is chosen according to the internal representation. The result
     * is mutable iff the designated part is directly represented in this chunk
     * by a mutable chunk, otherwise the result is immutable.
     */
    virtual std::shared_ptr<Chunk> peek(const Iterator& iterator, int64_t length = -1) const;

    /**
     * Returns whether if the designated part of the data is available in the
     * requested representation.
     */
    template <typename T>
    bool has(const Iterator& iterator, int64_t length = -1) const {
        if (length != -1 && getChunkLength() < iterator.getPosition() + length)
            return false;
        else
            return peek<T>(iterator, length) != nullptr;
    }

    /**
     * Returns the designated part of the data represented by this chunk in the
     * requested representation. If the length is unspecified, then the length of
     * the result is chosen according to the internal representation. The result
     * is mutable iff the designated part is directly represented in this chunk
     * by a mutable chunk, otherwise the result is immutable.
     */
    template <typename T>
    std::shared_ptr<T> peek(const Iterator& iterator, int64_t length = -1) const {
        int64_t chunkLength = getChunkLength();
        assert(0 <= iterator.getPosition() && iterator.getPosition() <= chunkLength);
        if (length == 0 || (iterator.getPosition() == chunkLength && length == -1))
            return nullptr;
        else if (iterator.getPosition() == 0 && (length == -1 || length == chunkLength)) {
            if (auto tChunk = std::dynamic_pointer_cast<T>(const_cast<Chunk *>(this)->shared_from_this()))
                return tChunk;
        }
        switch (getChunkType()) {
            case TYPE_SLICE: {
                if (auto tChunk = std::dynamic_pointer_cast<T>(peekSliceChunk(iterator, length)))
                    return tChunk;
                break;
            }
            case TYPE_SEQUENCE:
                if (auto tChunk = std::dynamic_pointer_cast<T>(peekSequenceChunk1(iterator, length)))
                    return tChunk;
                if (auto tChunk = std::dynamic_pointer_cast<T>(peekSequenceChunk2(iterator, length)))
                    return tChunk;
                break;
            default:
                break;
        }
        return doPeek<T>(iterator, length);
    }

    /**
     * Returns a human readable string representation of the data present in
     * this chunk.
     */
    virtual std::string str() const override;
    //@}

  public:
    /** @name Chunk serialization related functions */
    //@{
    /**
     * Serializes a chunk into the given stream. The bytes representing the
     * chunk is written at the current position of the stream up to its length.
     */
    static void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk, int64_t offset = 0, int64_t length = -1);

    /**
     * Deserializes a chunk from the given stream. The returned chunk will be
     * an instance of the provided type. The chunk represents the bytes in the
     * stream starting from the current stream position up to the length
     * required by the deserializer of the chunk.
     */
    static std::shared_ptr<Chunk> deserialize(ByteInputStream& stream, const std::type_info& typeInfo);
    //@}
};

inline std::ostream& operator<<(std::ostream& os, const Chunk *chunk) { return os << chunk->str(); }

inline std::ostream& operator<<(std::ostream& os, const Chunk& chunk) { return os << chunk.str(); }

} // namespace

#endif // #ifndef __INET_CHUNK_H_

