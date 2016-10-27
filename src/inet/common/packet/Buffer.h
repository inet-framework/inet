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

#include "inet/common/packet/SequenceChunk.h"

namespace inet {

class Buffer : public cObject
{
  protected:
    int64_t pushedByteLength = 0;
    int64_t poppedByteLength = 0;
    std::shared_ptr<SequenceChunk> data;
    SequenceChunk::ForwardIterator iterator;

  public:
    Buffer();
    Buffer(const Buffer& other);

    virtual Buffer *dup() const override { return new Buffer(*this); }

    int64_t getPushedByteLength() const { return pushedByteLength; }
    int64_t getPoppedByteLength() const { return poppedByteLength; }

    bool isImmutable() const { return data->isImmutable(); }
    bool isMutable() const { return !data->isImmutable(); }
    void assertMutable() const { data->assertMutable(); }
    void assertImmutable() const { data->assertImmutable(); }
    void makeImmutable() { data->makeImmutable(); }

    template <typename T>
    bool has(int64_t byteLength = -1) const {
        return data->peek<T>(iterator, byteLength) != nullptr;
    }
    template <typename T>
    std::shared_ptr<T> peek(int64_t byteLength = -1) const {
        return data->peek<T>(iterator, byteLength);
    }
    std::shared_ptr<SliceChunk> peek(int64_t byteLength = -1) const {
        return data->peek(iterator, byteLength);
    }
    template <typename T>
    std::shared_ptr<T> pop(int64_t byteLength = -1) {
        const auto& chunk = data->peek<T>(iterator, byteLength);
        if (chunk != nullptr) {
            auto byteLength = chunk->getByteLength();
            iterator.move(byteLength);
            poppedByteLength += byteLength;
        }
        return chunk;
    }

    void push(const std::shared_ptr<Chunk>& chunk, bool flatten = true) { data->append(chunk, flatten); pushedByteLength += chunk->getByteLength(); }
    void push(Buffer *buffer, bool flatten = true) { data->append(buffer->data, flatten); pushedByteLength += buffer->getByteLength(); }

    int64_t getByteLength() const { return data->getByteLength() - iterator.getPosition(); }

    virtual std::string str() const override { return data->str(); }
};

inline std::ostream& operator<<(std::ostream& os, const Buffer *buffer) { return os << buffer->str(); }

inline std::ostream& operator<<(std::ostream& os, const Buffer& buffer) { return os << buffer.str(); }

} // namespace

#endif // #ifndef __INET_BUFFER_H_

