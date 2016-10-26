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

#ifndef __BUFFER_H_
#define __BUFFER_H_

#include "Chunk.h"

class Buffer : public cObject
{
  protected:
    std::shared_ptr<SequenceChunk> data;
    SequenceChunk::ForwardIterator iterator;

  public:
    Buffer() :
        data(std::make_shared<SequenceChunk>()),
        iterator(data->createForwardIterator())
    {
    }

    Buffer(const Buffer& other) :
        data(other.data),
        iterator(other.iterator)
    {
    }

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
        if (chunk != nullptr)
            iterator.move(chunk->getByteLength());
        return chunk;
    }

    void push(const std::shared_ptr<Chunk>& chunk) { data->append(chunk); }
    void push(const std::shared_ptr<SliceChunk>& chunk) { data->append(chunk); }
    void push(const std::shared_ptr<SequenceChunk>& chunk) { data->append(chunk); }
    void push(Buffer *buffer) { data->append(buffer->data); }

    int64_t getByteLength() const { return data->getByteLength() - iterator.getPosition(); }

    virtual std::string str() const override {
        return data->str();
    }

    virtual Buffer *dup() const override { return new Buffer(*this); }
};

inline std::ostream& operator<<(std::ostream& os, const Buffer *buffer) { return os << buffer->str(); }

inline std::ostream& operator<<(std::ostream& os, const Buffer& buffer) { return os << buffer.str(); }

#endif // #ifndef __BUFFER_H_

