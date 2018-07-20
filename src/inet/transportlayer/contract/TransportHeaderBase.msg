//
// Copyright (C) 2017 OpenSim Ltd.
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

namespace inet {

class INET_API TransportHeaderBase : public FieldsChunk
{
  protected:
    virtual TransportHeaderBase *dup() const override { throw omnetpp::cRuntimeError("You forgot to manually add a dup() function to class %s", getClassName()); }
  public:
    virtual unsigned int getSourcePort() const = 0;
    virtual void setSourcePort(unsigned int port) = 0;
    virtual unsigned int getDestinationPort() const = 0;
    virtual void setDestinationPort(unsigned int port) = 0;
};

} // namespace inet

}}


namespace inet;

//
// This base class provides an abstraction for different transport layer packets.
//
class TransportHeaderBase extends FieldsChunk
{
    @existingClass;
    unsigned int sourcePort;
    unsigned int destinationPort;
};

