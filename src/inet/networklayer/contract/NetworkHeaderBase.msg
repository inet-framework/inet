//
// Copyright (C) 2017 Opensim Ltd
// Author: Zoltan Bojthe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

cplusplus {{
#include "inet/common/Protocol.h"
#include "inet/networklayer/common/L3Address.h"

namespace inet {

typedef const Protocol ConstProtocol;

class INET_API NetworkHeaderBase : public FieldsChunk
{
  protected:
    virtual NetworkHeaderBase *dup() const override { throw omnetpp::cRuntimeError("You forgot to manually add a dup() function to class %s", getClassName()); }
  public:
    virtual L3Address getSourceAddress() const = 0;
    virtual void setSourceAddress(const L3Address& address) = 0;
    virtual L3Address getDestinationAddress() const = 0;
    virtual void setDestinationAddress(const L3Address& address) = 0;
    virtual int getProtocolId() const = 0;
    virtual void setProtocolId(int protocol) = 0;
    virtual ConstProtocol *getProtocol() const = 0;
    virtual void setProtocol(ConstProtocol *protocol) = 0;
};

} // namespace inet

}}

namespace inet;

class noncobject L3Address;
class noncobject ConstProtocol;

class NetworkHeaderBase extends FieldsChunk
{
    @existingClass(true);

    L3Address sourceAddress @opaque;
    L3Address destinationAddress @opaque;
    int protocolId;
    ConstProtocol *protocol;
};

