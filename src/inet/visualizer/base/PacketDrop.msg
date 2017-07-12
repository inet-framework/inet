//
// This program is property of its copyright holder. All rights reserved.
//

cplusplus{{
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/Simsignals_m.h"
#include "inet/visualizer/base/PacketDropVisualizerBase.h"
}}

namespace inet::visualizer;

class noncobject Coord;
class inet::PacketDropDetails extends cObject;

class PacketDrop extends inet::PacketDropDetails
{
    @existingClass();
    @readOnly();
    cPacket *packet_;
    cModule *module;
    cModule *networkNode;
    cObject *interfaceEntry;
    Coord position @opaque;
}