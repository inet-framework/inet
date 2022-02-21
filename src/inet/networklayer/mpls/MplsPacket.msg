//
// Copyright (C) 2017 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
