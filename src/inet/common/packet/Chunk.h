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

class SliceChunk;

/**
 * This class represents a piece of data.
 */
class Chunk : public cObject, public std::enable_shared_from_this<Chunk>
{
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

    bool isMutable() const { return !isImmutable_; }
    bool isImmutable() const { return isImmutable_; }
    void assertMutable() const { assert(!isImmutable_); }
    void assertImmutable() const { assert(isImmutable_); }
    void makeImmutable() { isImmutable_ = true; }
    // NOTE: there is no makeMutable() intentionally

    bool isComplete() const { return !isIncomplete_; }
    bool isIncomplete() const { return isIncomplete_; }
    void assertComplete() const { assert(!isIncomplete_); }
    void assertIncomplete() const { assert(isIncomplete_); }
    void makeIncomplete() { isIncomplete_ = true; }
    // NOTE: there is no makeComplete() intentionally

    bool isCorrect() const { return !isIncorrect_; }
    bool isIncorrect() const { return isIncorrect_; }
    void assertCorrect() const { assert(!isIncorrect_); }
    void assertIncorrect() const { assert(isIncorrect_); }
    void makeIncorrect() { isIncorrect_ = true; }
    // NOTE: there is no makeCorrect() intentionally

    virtual int64_t getByteLength() const = 0;

    virtual std::shared_ptr<Chunk> merge(const std::shared_ptr<Chunk>& other) const { return nullptr; }

    virtual const char *getSerializerClassName() const { return nullptr; }

    virtual std::string str() const override;

    static void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk);
    static std::shared_ptr<Chunk> deserialize(ByteInputStream& stream, const std::type_info& typeInfo);

    static std::shared_ptr<Chunk> createChunk(const std::type_info& typeInfo, const std::shared_ptr<Chunk>& chunk, int64_t byteOffset, int64_t byteLength);

    template <typename T>
    std::shared_ptr<T> peekAt(int64_t byteOffset = 0, int64_t byteLength = -1) const {
        if (!enableImplicitChunkSerialization)
            throw cRuntimeError("Implicit chunk serialization is disabled to prevent unpredictable performance degradation (you may consider changing the value of the ENABLE_IMPLICIT_CHUNK_SERIALIZATION variable)");
        // TODO: prevents easy access for application buffer
        // assertImmutable();
        // TODO: eliminate const_cast
        const auto& chunk = T::createChunk(typeid(T), const_cast<Chunk *>(this)->shared_from_this(), byteOffset, byteLength);
        chunk->makeImmutable();
        if ((chunk->isComplete() && byteLength == -1) || byteLength == chunk->getByteLength())
            return std::dynamic_pointer_cast<T>(chunk);
        else
            return nullptr;
    }
    std::shared_ptr<SliceChunk> peekAt(int64_t byteOffset = 0, int64_t byteLength = -1) const;
};

inline std::ostream& operator<<(std::ostream& os, const Chunk *chunk) { return os << chunk->str(); }

inline std::ostream& operator<<(std::ostream& os, const Chunk& chunk) { return os << chunk.str(); }

} // namespace

#endif // #ifndef __INET_CHUNK_H_

