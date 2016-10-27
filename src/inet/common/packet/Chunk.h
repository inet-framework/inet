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

// TODO: flag for disabling serialization completely

class Chunk : public cObject
{
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

    virtual void replace(const std::shared_ptr<Chunk>& chunk, int64_t byteOffset, int64_t byteLength);
    virtual std::shared_ptr<Chunk> merge(const std::shared_ptr<Chunk>& other) const { return nullptr; }

    virtual void serialize(ByteOutputStream& stream) const;
    virtual void deserialize(ByteInputStream& stream);

    virtual const char *getSerializerClassName() const { return nullptr; }
    virtual std::string str() const override;
};

inline std::ostream& operator<<(std::ostream& os, const Chunk *chunk) {
    return os << chunk->str();
}

inline std::ostream& operator<<(std::ostream& os, const Chunk& chunk) {
    return os << chunk.str();
}

#endif // #ifndef __INET_CHUNK_H_

