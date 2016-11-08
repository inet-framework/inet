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
 * some other data such as a protocol buffer.
 */
class Chunk : public cObject, public std::enable_shared_from_this<Chunk>
{
  public:
    class Iterator
    {
      protected:
        int64_t position = 0;

      public:
        Iterator(int64_t position = 0);
        Iterator(const Iterator& other);
        virtual ~Iterator() { }

        int64_t getPosition() const { return position; }

        void move(int64_t byteLength) { position += byteLength; }
        void seek(int64_t byteOffset) { position = byteOffset; }
    };

  public:
    static bool enableImplicitChunkSerialization;

  protected:
    bool isImmutable_ = false;
    bool isIncomplete_ = false;
    bool isIncorrect_ = false;
    std::vector<uint8_t> *serializedBytes = nullptr;

  protected:
    void handleChange();

  public:
    Chunk() { }
    Chunk(const Chunk& other);
    virtual ~Chunk();

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

    virtual int64_t getByteLength() const = 0;

    virtual std::shared_ptr<Chunk> merge(const std::shared_ptr<Chunk>& other) const { return nullptr; }

    virtual const char *getSerializerClassName() const { return nullptr; }

    /**
     * Returns a human readable string representation of the data present in
     * this chunk.
     */
    virtual std::string str() const override;

    /**
     * Creates a new chunk of the given type that represents the designated part
     * of the other provided chunk.
     */
    static std::shared_ptr<Chunk> createChunk(const std::type_info& typeInfo, const std::shared_ptr<Chunk>& chunk, int64_t byteOffset, int64_t byteLength);

    /** @name Chunk serialization related functions */
    //@{
    /**
     * Serializes a chunk into the given stream. The bytes representing the
     * chunk is written at the current position of the stream up to the length
     * required by the serializer of the chunk.
     */
    static void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk);

    /**
     * Deserializes a chunk from the given stream that is an instance of the
     * provided type. The chunk represents the bytes in the stream starting
     * from the current stream position up to the length required by the
     * deserializer of the chunk.
     */
    static std::shared_ptr<Chunk> deserialize(ByteInputStream& stream, const std::type_info& typeInfo);
    //@}

    /** @name Chunk querying related functions */
    //@{
    /**
     * Returns the designated part of the data represented by this chunk in its
     * default representation.
     */
    // TODO: rename to peek! conflict?
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
};

inline std::ostream& operator<<(std::ostream& os, const Chunk *chunk) { return os << chunk->str(); }

inline std::ostream& operator<<(std::ostream& os, const Chunk& chunk) { return os << chunk.str(); }

} // namespace

#endif // #ifndef __INET_CHUNK_H_

