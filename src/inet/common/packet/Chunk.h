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
 * some other data such as a protocol buffer. When the actual bytes in the
 * data is irrelevant, a chunk can be as simple as an integer specifying its
 * length. Sometimes it might be necessary to represent the data bytes as is,
 * but most often it's better to have a representation that is easy to inspect
 * during debugging. In any case, a chunk can be always converted to a sequence
 * of bytes.
 *
 * Chunks are initially:
 *  - mutable, then may become immutable (but never the other way around)
 *  - complete, then may become incomplete (but never the other way around)
 *  - correct, then may become incorrect (but never the other way around)
 * Chunks can be safely shared using std::shared_ptr, and in fact most usages
 * take advantage of immutable chunks using shared pointers.
 *
 * In general, chunks support the following operations:
 *  - insert to the beginning or end
 *  - remove from the beginning or end
 *  - query length and peek an arbitrary part
 *  - serialize to and deserialize from a sequence of bytes
 *
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
 *       - a SequenceChunk potentially containing SliceChunks at both ends
 *    e) any other chunk returns a SliceChunk
 * 2) Peeking with providing a return type always returns a chunk of the
 *    requested type (or a subtype)
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
  public:
    class Iterator
    {
      protected:
        std::shared_ptr<const Chunk> chunk = nullptr;
        bool isForward_ = true;
        int64_t position = 0;
        int index = 0;

      public:
        Iterator(const std::shared_ptr<const Chunk>& chunk, bool isForward = true, int64_t position = 0, int index = 0);
        Iterator(const Iterator& other);
        virtual ~Iterator() { }

        Iterator& operator=(const Iterator& other);

        bool isForward() const { return isForward_; }
        bool isBackward() const { return !isForward_; }
        int64_t getPosition() const { return position; }
        int getIndex() const { return index; }

        virtual void move(int64_t byteLength) { position += byteLength; }
        virtual void seek(int64_t byteOffset) { position = byteOffset; }
    };

  public:
    static bool enableImplicitChunkSerialization;

  protected:
    // TODO: convert these booleans to a single integer flags
    bool isImmutable_ = false;
    bool isIncomplete_ = false;
    bool isIncorrect_ = false;
    std::vector<uint8_t> *serializedBytes = nullptr;

  protected:
    virtual const char *getSerializerClassName() const { return nullptr; }

    virtual void handleChange() override;

  protected:
    /**
     * Creates a new chunk of the given type that represents the designated part
     * of the provided chunk.
     */
    static std::shared_ptr<Chunk> createChunk(const std::type_info& typeInfo, const std::shared_ptr<Chunk>& chunk, int64_t byteOffset, int64_t byteLength);

  public:
    Chunk() { }
    Chunk(const Chunk& other);
    virtual ~Chunk();

    virtual std::shared_ptr<Chunk> dupShared() const { return std::shared_ptr<Chunk>(static_cast<Chunk *>(dup())); };

    /** @name Mutability related functions */
    //@{
    // NOTE: there is no makeMutable() intentionally
    bool isMutable() const { return !isImmutable_; }
    bool isImmutable() const { return isImmutable_; }
    void assertMutable() const { assert(!isImmutable_); }
    void assertImmutable() const { assert(isImmutable_); }
    void makeImmutable() { isImmutable_ = true; }
    //@}

    /** @name Completeness related functions */
    //@{
    // NOTE: there is no makeComplete() intentionally
    bool isComplete() const { return !isIncomplete_; }
    bool isIncomplete() const { return isIncomplete_; }
    void assertComplete() const { assert(!isIncomplete_); }
    void assertIncomplete() const { assert(isIncomplete_); }
    void makeIncomplete() { isIncomplete_ = true; }
    //@}

    /** @name Correctness related functions */
    //@{
    // NOTE: there is no makeCorrect() intentionally
    bool isCorrect() const { return !isIncorrect_; }
    bool isIncorrect() const { return isIncorrect_; }
    void assertCorrect() const { assert(!isIncorrect_); }
    void assertIncorrect() const { assert(isIncorrect_); }
    void makeIncorrect() { isIncorrect_ = true; }
    //@}

    /** @name Iteration related functions */
    //@{
    Iterator createForwardIterator() const { return Iterator(shared_from_this(), true); }
    Iterator createBackwardIterator() const { return Iterator(shared_from_this(), false); }
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
    virtual int64_t getByteLength() const = 0;

    virtual std::shared_ptr<Chunk> peek(int64_t byteOffset = 0, int64_t byteLength = -1) const {
        return peek(Iterator(shared_from_this(), true, 0, -1), byteLength);
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
        if (!enableImplicitChunkSerialization)
            throw cRuntimeError("Implicit chunk serialization is disabled to prevent unpredictable performance degradation (you may consider changing the value of the ENABLE_IMPLICIT_CHUNK_SERIALIZATION variable)");
        // TODO: prevents easy access for application buffer
        // assertImmutable();
        // TODO: eliminate const_cast
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
    static void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk);

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

