//
// Copyright (C) 2017 OpenSim Ltd.
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//


import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.networklayer.contract.ipv4.Ipv4Address;

namespace inet;

class MplsHeader extends FieldsChunk
{
    chunkLength = B(4);
    uint32_t label;     // @bit(20)  Label value. A label with the value of 1 represents the router alert label.
    uint8_t tc;       // @bit(3)   Traffic Class field for QoS (quality of service) priority and ECN (Explicit Congestion Notification). Prior to 2009 this field was called EXP.
    bool s;         // @bit(1)   bottom of stack flag. If this is set, it signifies that the current label is the last in the stack.
    uint8_t ttl;      // @bit(8)   time to live
}

cplusplus(MplsHeader) {{
  public:
    /**
     * Returns MPLS header in human readable format string.
     */
    virtual std::string str() const override;
}}

