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

#ifndef __UNITTEST_H_
#define __UNITTEST_H_

#include "inet/common/packet/Defs.h"

namespace inet {

class CompoundHeaderSerializer : public SequenceChunkSerializer
{
  public:
    virtual std::shared_ptr<Chunk> deserialize(ByteInputStream& stream) const override;
};

class TlvHeaderSerializer : public ChunkSerializer
{
  public:
    virtual void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const override;
    virtual std::shared_ptr<Chunk> deserialize(ByteInputStream& stream) const override;
};

class TlvHeader1Serializer : public ChunkSerializer
{
  public:
    virtual void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const override;
    virtual std::shared_ptr<Chunk> deserialize(ByteInputStream& stream) const override;
};

class TlvHeader2Serializer : public ChunkSerializer
{
  public:
    virtual void serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const override;
    virtual std::shared_ptr<Chunk> deserialize(ByteInputStream& stream) const override;
};

class UnitTest : public cSimpleModule
{
  protected:
    void initialize() override;
};

} // namespace

#endif // #ifndef __UNITTEST_H_

