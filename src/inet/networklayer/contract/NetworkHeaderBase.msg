//
// Copyright (C) 2017 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

    L3Address sourceAddress @byValue @toValue(.str()) @fromValue(L3Address($));
    L3Address destinationAddress @byValue @toValue(.str()) @fromValue(L3Address($));
    const Protocol *protocol;
};
