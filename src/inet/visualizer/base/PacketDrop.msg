//
// This program is property of its copyright holder. All rights reserved.
//

import inet.common.INETDefs;
import inet.common.Simsignals;
import inet.common.geometry.Geometry;
import inet.networklayer.common.InterfaceEntry;

cplusplus{{
#include "inet/visualizer/base/PacketDropVisualizerBase.h"
}}

namespace inet::visualizer;

class PacketDrop extends inet::PacketDropDetails
{
    @existingClass;
    @descriptor(readonly);
    cPacket *packet_;
    cModule *module;
    cModule *networkNode;
    InterfaceEntry *networkInterface;
    Coord position;
}

