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

#ifndef __INET_CPACKETCHUNK_H_
#define __INET_CPACKETCHUNK_H_

#include "inet/common/packet/Chunk.h"

namespace inet {

class cPacketChunk : public Chunk
{
  protected:
    cPacket *packet = nullptr;

  public:
    cPacketChunk() { }
    cPacketChunk(cPacket *packet);

    virtual int64_t getByteLength() const override { return packet->getByteLength(); }

    virtual std::string str() const override;
};

} // namespace

#endif // #ifndef __INET_CPACKETCHUNK_H_

