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

class Chunk : public cObject, public std::enable_shared_from_this<Chunk>
{
  public:
    static const bool ENABLE_IMPLICIT_CHUNK_SERIALIZATION = true;

  protected:
    bool isImmutable_ = false;
    bool isIncomplete_ = false;
    bool isIncorrect_ = false;

  public:
    Chunk() { }
    Chunk(const Chunk& other);
    virtual ~Chunk() { }

    bool isMutable() const { return !isImmutable_; }
    bool isImmutable() const { return isImmutable_; }
    void assertMutable() const { assert(!isImmutable_); }
    void assertImmutable() const { assert(isImmutable_); }
    void makeImmutable() { isImmutable_ = true; }

    bool isComplete() const { return !isIncomplete_; }
    bool isIncomplete() const { return isIncomplete_; }
    void assertComplete() const { assert(!isIncomplete_); }
    void assertIncomplete() const { assert(isIncomplete_); }
    void makeIncomplete() { isIncomplete_ = true; }

    bool isCorrect() const { return !isIncorrect_; }
    bool isIncorrect() const { return isIncorrect_; }
    void assertCorrect() const { assert(!isIncorrect_); }
    void assertIncorrect() const { assert(isIncorrect_); }
    void makeIncorrect() { isIncorrect_ = true; }

    virtual int64_t getByteLength() const = 0;

    virtual std::shared_ptr<Chunk> replace(const std::shared_ptr<Chunk>& chunk, int64_t byteOffset, int64_t byteLength);
    virtual std::shared_ptr<Chunk> merge(const std::shared_ptr<Chunk>& other) const { return nullptr; }

    virtual void serialize(ByteOutputStream& stream) const;
    virtual std::shared_ptr<Chunk> deserialize(ByteInputStream& stream);

    template <typename T>
    std::shared_ptr<T> peekAt(int64_t byteOffset = 0, int64_t byteLength = -1) const {
        if (!ENABLE_IMPLICIT_CHUNK_SERIALIZATION)
            throw cRuntimeError("Chunk serialization is disabled to prevent unpredictable performance degradation, set ENABLE_CHUNK_SERIALIZATION to allow transparent chunk serialization");
        // TODO: prevents easy access for application buffer
        // assertImmutable();
        const auto& t = std::make_shared<T>();
        // TODO: eliminate const_cast
        const auto& chunk = t->replace(const_cast<Chunk *>(this)->shared_from_this(), byteOffset, byteLength);
        chunk->makeImmutable();
        if ((chunk->isComplete() && byteLength == -1) || byteLength == chunk->getByteLength())
            return std::dynamic_pointer_cast<T>(chunk);
        else
            return nullptr;
    }
    std::shared_ptr<SliceChunk> peekAt(int64_t byteOffset = 0, int64_t byteLength = -1) const;

    virtual const char *getSerializerClassName() const { return nullptr; }
    virtual std::string str() const override;
};

inline std::ostream& operator<<(std::ostream& os, const Chunk *chunk) {
    return os << chunk->str();
}

inline std::ostream& operator<<(std::ostream& os, const Chunk& chunk) {
    return os << chunk.str();
}

} // namespace

#endif // #ifndef __INET_CHUNK_H_

