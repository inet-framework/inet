//
// This program is property of its copyright holder. All rights reserved.
//

import inet.common.INETDefs;
import inet.common.Simsignals;
import inet.common.geometry.Geometry;

cplusplus{{
#include "inet/visualizer/base/PacketDropVisualizerBase.h"
}}

namespace inet::visualizer;

class PacketDrop extends inet::PacketDropDetails
{
    @existingClass;
    @descriptor(readOnly);
    cPacket *packet_;
    cModule *module;
    cModule *networkNode;
    cObject *networkInterface;
    Coord position;
}

