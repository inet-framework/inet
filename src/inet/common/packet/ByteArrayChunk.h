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

#ifndef __INET_BYTEARRAYCHUNK_H_
#define __INET_BYTEARRAYCHUNK_H_

#include "Chunk.h"

namespace inet {

class ByteArrayChunk : public Chunk
{
  protected:
    std::vector<uint8_t> bytes;

  public:
    ByteArrayChunk() { }
    ByteArrayChunk(const std::vector<uint8_t>& bytes);

    const std::vector<uint8_t>& getBytes() const { return bytes; }
    void setBytes(const std::vector<uint8_t>& bytes);

    virtual int64_t getByteLength() const override { return bytes.size(); }

    virtual void replace(const std::shared_ptr<Chunk>& chunk, int64_t byteOffset, int64_t byteLength) override;
    virtual std::shared_ptr<Chunk> merge(const std::shared_ptr<Chunk>& other) const override;

    virtual const char *getSerializerClassName() const override { return "ByteArrayChunkSerializer"; }
    virtual std::string str() const override;
};

} // namespace

#endif // #ifndef __INET_BYTEARRAYCHUNK_H_

