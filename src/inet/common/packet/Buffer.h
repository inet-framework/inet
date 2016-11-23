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

#ifndef __INET_BUFFER_H_
#define __INET_BUFFER_H_

#include "inet/common/packet/Chunk.h"

namespace inet {

/**
 * This class represents application or protocol buffers.
 */
class Buffer : public cObject
{
  protected:
    int64_t pushedByteLength = 0;
    int64_t poppedByteLength = 0;
    std::shared_ptr<Chunk> data = nullptr;
    Chunk::Iterator iterator;

    void remove(int64_t byteLength);

  public:
    Buffer();
    Buffer(const Buffer& other);
    Buffer(const std::shared_ptr<Chunk>& data);

    virtual Buffer *dup() const override { return new Buffer(*this); }

    int64_t getPushedByteLength() const { return pushedByteLength; }
    int64_t getPoppedByteLength() const { return poppedByteLength; }

    /** @name Mutability related functions */
    //@{
    bool isImmutable() const { return data != nullptr && data->isImmutable(); }
    bool isMutable() const { return data == nullptr && !data->isMutable(); }
    void assertMutable() const { assert(isMutable()); }
    void assertImmutable() const { assert(isImmutable()); }
    void makeImmutable() { data->makeImmutable(); }
    //@}

    /** @name Data querying related functions */
    //@{
    int64_t getByteLength() const { return data->getByteLength() - iterator.getPosition(); }

    std::shared_ptr<Chunk> peek(int64_t byteLength = -1) const;

    std::shared_ptr<Chunk> peekAt(int64_t byteOffset, int64_t byteLength) const;

    std::shared_ptr<Chunk> pop(int64_t byteLength = -1);

    template <typename T>
    bool has(int64_t byteLength = -1) const {
        return peek<T>(byteLength) != nullptr;
    }
    template <typename T>
    std::shared_ptr<T> peek(int64_t byteLength = -1) const {
        return data->peek<T>(iterator, byteLength);
    }
    template <typename T>
    std::shared_ptr<T> pop(int64_t byteLength = -1) {
        const auto& chunk = peek<T>(byteLength);
        if (chunk != nullptr)
            remove(chunk->getByteLength());
        return chunk;
    }
    //@}

    /** @name Filling with data related functions */
    //@{
    void push(const std::shared_ptr<Chunk>& chunk);
    //@}

    virtual std::string str() const override { return data->str(); }
};

inline std::ostream& operator<<(std::ostream& os, const Buffer *buffer) { return os << buffer->str(); }

inline std::ostream& operator<<(std::ostream& os, const Buffer& buffer) { return os << buffer.str(); }

} // namespace

#endif // #ifndef __INET_BUFFER_H_

