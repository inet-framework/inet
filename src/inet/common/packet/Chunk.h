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
#include "inet/common/packet/ByteStream.h"

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
 * TODO: polymorphism, nesting, compacting
 *
 * Chunks can represent data in different ways:
 *  - ByteLengthChunk contains a length field only
 *  - ByteArrayChunk contains a sequence of bytes
 *  - SliceChunk contains a slice of another chunk
 *  - SequenceChunk contains a sequence of other chunks
 *  - cPacketChunk contains a cPacket for compatibility
 *  - message compiler generated chunks contain various user defined fields
 *
 * Chunks are initially:
 *  - mutable, then may become immutable (but never the other way around)
 *    immutable chunks cannot be changed anymore
 *  - complete, then may become incomplete (but never the other way around)
 *    incomplete chunks are not totally filled in (e.g. due to deserialization)
 *  - correct, then may become incorrect (but never the other way around)
 *    incorrect chunks contain bit errors
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
 * 1) Peeking (various special cases)
 *    a) an empty part of the chunk returns an empty chunk
 *    b) the whole of the chunk returns the chunk
 *    c) any part that is directly represented by a chunk returns that chunk
 * 1) Peeking without providing a return type for a
 *    a) ByteArrayChunk always returns a ByteArrayChunk containing the bytes
 *       of the requested part
 *    b) ByteLengthChunk always returns a ByteLengthChunk containing the
 *       requested byte length
 *    c) SliceChunk always returns a SliceChunk containing the requested slice
 *       of the chunk that is used by the original SliceChunk
 *    d) SequenceChunk may return
 *       - an element chunk
 *       - a SliceChunk of an element chunk
 *       - a SliceChunk using the original SequenceChunk
 *       - a SequenceChunk potentially containing SliceChunks at both ends
 *    e) any other chunk returns a SliceChunk
 * 2) Peeking with providing a return type always returns a chunk of the
 *    requested type (or a subtype thereof)
 *    a) Peeking with a ByteLengthChunk return type for any chunk returns a
 *       ByteLengthChunk containing the requested byte length
 *    b) Peeking with a ByteArrayChunk return type for any chunk returns a
 *       ByteArrayChunk containing a part of the serialized bytes of the
 *       original chunk
 *    c) Peeking with a SliceChunk return type for any chunk returns a
 *       SliceChunk containing the original chunk
 *    d) Peeking with a SequenceChunk return type is an error
 *    e) Peeking with a any other return type for any chunk returns a chunk of
 *       the requested type containing data deserialized from the bytes that
 *       were serialized from the original chunk
 */
class Chunk : public cObject, public std::enable_shared_from_this<Chunk>
{
  protected:
    /**
     * This enum specifies bitmasks for the flags field.
     */
    enum Flag {
        FLAG_IMMUTABLE  = 1,
        FLAG_INCOMPLETE = 2,
        FLAG_INCORRECT  = 4,
    };

  public:
    /**
     * This enum is used to avoid std::dynamic_cast and std::dynamic_pointer_cast.
     */
    enum Type {
        TYPE_BYTELENGTH,
        TYPE_BYTEARRAY,
        TYPE_SLICE,
        TYPE_SEQUENCE,
        TYPE_OTHER
    };

    class Iterator
    {
      protected:
        const bool isForward_;
        int64_t position;
        int index;

      public:
        Iterator(bool isForward = true, int64_t position = 0, int index = 0);
        Iterator(const Iterator& other);

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
    class ForwardIterator : public Iterator
    {
      public:
        ForwardIterator(int64_t position = 0, int index = 0) :
            Iterator(true, position, index)
        {
        }
    };

    /**
     * Position and index are measured from the end.
     */
    class BackwardIterator : public Iterator
    {
      public:
        BackwardIterator(int64_t position = 0, int index = 0) :
            Iterator(false, position, index)
        {
        }
    };

  public:
    /**
     * Peeking some part into a chunk that requires automatic serialization
     * will throw an exception when implicit chunk serialization is disabled.
     */
    static bool enableImplicitChunkSerialization;

  protected:
    uint16_t flags;
    std::vector<uint8_t> *serializedBytes;

  protected:
    virtual const char *getSerializerClassName() const { return nullptr; }

    virtual void handleChange() override;

    virtual std::shared_ptr<Chunk> peekWithIterator(const Iterator& iterator, int64_t byteLength = -1) const { return nullptr; }
    virtual std::shared_ptr<Chunk> peekWithLinearSearch(const Iterator& iterator, int64_t byteLength = -1) const { return nullptr; }

  protected:
    /**
     * Creates a new chunk of the given type that represents the designated part
     * of the provided chunk.
     */
    static std::shared_ptr<Chunk> createChunk(const std::type_info& typeInfo, const std::shared_ptr<Chunk>& chunk, int64_t byteOffset, int64_t byteLength);

  public:
    Chunk();
    Chunk(const Chunk& other);
    virtual ~Chunk();

    virtual std::shared_ptr<Chunk> dupShared() const { return std::shared_ptr<Chunk>(static_cast<Chunk *>(dup())); };

    virtual Type getChunkType() const { return TYPE_OTHER; }

    /** @name Mutability related functions */
    //@{
    // NOTE: there is no makeMutable() intentionally
    bool isMutable() const { return !(flags & FLAG_IMMUTABLE); }
    bool isImmutable() const { return flags & FLAG_IMMUTABLE; }
    void assertMutable() const { assert(isMutable()); }
    void assertImmutable() const { assert(isImmutable()); }
    virtual void makeImmutable() { flags |= FLAG_IMMUTABLE; }
    //@}

    /** @name Completeness related functions */
    //@{
    // NOTE: there is no makeComplete() intentionally
    bool isComplete() const { return !(flags & FLAG_INCOMPLETE); }
    bool isIncomplete() const { return flags & FLAG_INCOMPLETE; }
    void assertComplete() const { assert(isComplete()); }
    void assertIncomplete() const { assert(isIncomplete()); }
    void makeIncomplete() { flags |= FLAG_INCOMPLETE; }
    //@}

    /** @name Correctness related functions */
    //@{
    // NOTE: there is no makeCorrect() intentionally
    bool isCorrect() const { return !(flags & FLAG_INCORRECT); }
    bool isIncorrect() const { return flags & FLAG_INCORRECT; }
    void assertCorrect() const { assert(isCorrect()); }
    void assertIncorrect() const { assert(isIncorrect()); }
    void makeIncorrect() { flags |= FLAG_INCORRECT; }
    //@}

    /** @name Iteration related functions */
    //@{
    Iterator createForwardIterator() const { return Iterator(true); }
    Iterator createBackwardIterator() const { return Iterator(false); }

    virtual void moveIterator(Iterator& iterator, int64_t byteLength) const { iterator.setPosition(iterator.getPosition() + byteLength); }
    virtual void seekIterator(Iterator& iterator, int64_t byteOffset) const { iterator.setPosition(byteOffset); }
    //@}

    /** @name Inserting data related functions */
    //@{
    /**
     * Inserts the provided chunk at the beginning of this chunk and returns
     * true if the insertion was successful.
     */
    virtual bool insertToBeginning(const std::shared_ptr<Chunk>& chunk) { assertMutable(); return false; }

    /**
     * Inserts the provided chunk at the end of this chunk and returns true if
     * the insertion was successful.
     */
    virtual bool insertToEnd(const std::shared_ptr<Chunk>& chunk) { assertMutable(); return false; }
    //@}

    /** @name Removing data related functions */
    //@{
    /**
     * Removes the requested number of bytes from the beginning of this chunk
     * and returns true if the removal was successful.
     */
    virtual bool removeFromBeginning(int64_t byteLength) { assertMutable(); return false; }

    /**
     * Removes the requested number of bytes from the end of this chunk and
     * returns true if the removal was successful.
     */
    virtual bool removeFromEnd(int64_t byteLength) { assertMutable(); return false; }
    //@}

    /** @name Chunk querying related functions */
    //@{
    /**
     * Returns the length of data measured in bytes represented by this chunk.
     */
    virtual int64_t getByteLength() const = 0; // TODO: rename to getChunkLength()?

    /**
     * Returns the designated part of the data represented by this chunk in its
     * default representation.
     */
    virtual std::shared_ptr<Chunk> peek(int64_t byteOffset = 0, int64_t byteLength = -1) const {
        return peek(Iterator(true, byteOffset, -1), byteLength);
    }

    /**
     * Returns the designated part of the data represented by this chunk in its
     * default representation.
     */
    virtual std::shared_ptr<Chunk> peek(const Iterator& iterator, int64_t byteLength = -1) const;

    /**
     * Returns the designated part of the data represented by this chunk in the
     * requested representation.
     */
    template <typename T>
    std::shared_ptr<T> peek(const Iterator& iterator, int64_t byteLength = -1) const {
        if (auto tChunk = std::dynamic_pointer_cast<T>(peekWithIterator(iterator, byteLength)))
            return tChunk;
        if (auto tChunk = std::dynamic_pointer_cast<T>(peekWithLinearSearch(iterator, byteLength)))
            return tChunk;
        if (!enableImplicitChunkSerialization)
            throw cRuntimeError("Implicit chunk serialization is disabled to prevent unpredictable performance degradation (you may consider changing the value of the ENABLE_IMPLICIT_CHUNK_SERIALIZATION variable)");
        // TODO: prevents easy access for application buffer
        // assertImmutable();
        const auto& chunk = T::createChunk(typeid(T), const_cast<Chunk *>(this)->shared_from_this(), iterator.getPosition(), byteLength);
        chunk->makeImmutable();
        if ((chunk->isComplete() && byteLength == -1) || byteLength == chunk->getByteLength())
            return std::dynamic_pointer_cast<T>(chunk);
        else
            return nullptr;
    }
    //@}

    /**
     * Returns a human readable string representation of the data present in
     * this chunk.
     */
    virtual std::string str() const override;

  public:
    /** @name Chunk serialization related functions */
    //@{
    /**
     * Serializes a chunk into the given stream. The bytes representing the
     * chunk is written at the current position of the stream up to its length.
     */
    static void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk); // TODO: serialize a part only?

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

