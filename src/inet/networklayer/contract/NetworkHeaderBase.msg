//
// Copyright (C) 2017 OpenSim Ltd.
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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//


import inet.common.INETDefs;
import inet.common.ProtocolTag;
import inet.common.packet.chunk.Chunk;
import inet.networklayer.common.L3Address;

cplusplus {{

namespace inet {

class INET_API NetworkHeaderBase : public FieldsChunk
{
  protected:
    virtual NetworkHeaderBase *dup() const override { throw omnetpp::cRuntimeError("You forgot to manually add a dup() function to class %s", getClassName()); }
  public:
    NetworkHeaderBase() : FieldsChunk() {}
    NetworkHeaderBase(const NetworkHeaderBase& other) : FieldsChunk(other) {}

    virtual L3Address getSourceAddress() const = 0;
    virtual void setSourceAddress(const L3Address& address) = 0;
    virtual L3Address getDestinationAddress() const = 0;
    virtual void setDestinationAddress(const L3Address& address) = 0;
    virtual const Protocol *getProtocol() const = 0;
    virtual void setProtocol(const Protocol *protocol) = 0;
    virtual bool isFragment() const = 0;
};

} // namespace inet

}}

namespace inet;

class NetworkHeaderBase extends FieldsChunk
{
    @existingClass;

    L3Address sourceAddress @byValue;
    L3Address destinationAddress @byValue;
    const Protocol *protocol;
};

