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

//
// MPLS label
//
class MplsLabel extends cObject
{
    long Label;     // @bit(20)  Label value. A label with the value of 1 represents the router alert label.
    short tc;       // @bit(3)   Traffic Class field for QoS (quality of service) priority and ECN (Explicit Congestion Notification). Prior to 2009 this field was called EXP.
//    bool s;         // @bit(1)   bottom of stack flag. If this is set, it signifies that the current label is the last in the stack.
    short ttl;      // @bit(8)   time to live
}

class MplsHeader extends FieldsChunk
{
    MplsLabel labels[];
}

cplusplus(MplsHeader) {{
  public:
    /**
     * Returns a string with the labels, starting with the top of stack.
     */
    virtual std::string str() const override;

    /**
     * Swap Label operation
     */
    inline void swapLabel(MplsLabel newLabel) { ASSERT(labels_arraysize > 0); labels[labels_arraysize - 1] = newLabel; }

    /**
     * Pushes new label on the label stack
     */
    inline void pushLabel(MplsLabel newLabel) { insertLabels(newLabel); }

    /**
     * Pops the top label
     */
    inline void popLabel() { ASSERT(labels_arraysize > 0); eraseLabels(labels_arraysize - 1); }

    /**
     * Returns true if the label stack is not empty
     */
    inline bool hasLabel() { return labels_arraysize > 0; }

    /**
     * Returns the top label
     */
    inline MplsLabel getTopLabel() { ASSERT(labels_arraysize > 0); return labels[labels_arraysize - 1]; }

    virtual b getChunkLength() const override { return b(32) * labels_arraysize; }
}}

