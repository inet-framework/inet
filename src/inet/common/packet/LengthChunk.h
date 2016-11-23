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

#ifndef __INET_LENGTHCHUNK_H_
#define __INET_LENGTHCHUNK_H_

#include "inet/common/packet/Chunk.h"

namespace inet {

class LengthChunk : public Chunk
{
  friend Chunk;

  protected:
    int64_t length;

  protected:
    virtual const char *getSerializerClassName() const override { return "inet::LengthChunkSerializer"; }

  protected:
    static std::shared_ptr<Chunk> createChunk(const std::type_info& typeInfo, const std::shared_ptr<Chunk>& chunk, int64_t offset, int64_t length);

  public:
    LengthChunk();
    LengthChunk(const LengthChunk& other);
    LengthChunk(int64_t length);

    virtual LengthChunk *dup() const override { return new LengthChunk(*this); }
    virtual std::shared_ptr<Chunk> dupShared() const override { return std::make_shared<LengthChunk>(*this); }

    int64_t getLength() const { return length; }
    void setLength(int64_t length);

    virtual Type getChunkType() const override { return TYPE_LENGTH; }
    virtual int64_t getChunkLength() const override { return length; }

    virtual bool insertToBeginning(const std::shared_ptr<Chunk>& chunk) override;
    virtual bool insertToEnd(const std::shared_ptr<Chunk>& chunk) override;

    virtual bool removeFromBeginning(int64_t length) override;
    virtual bool removeFromEnd(int64_t length) override;

    virtual std::shared_ptr<Chunk> peek(const Iterator& iterator, int64_t length = -1) const override;

    virtual std::string str() const override;
};

} // namespace

#endif // #ifndef __INET_LENGTHCHUNK_H_

