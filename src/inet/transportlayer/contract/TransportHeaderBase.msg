//
// Copyright (C) 2017 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
