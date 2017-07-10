//
// This program is property of its copyright holder. All rights reserved.
//

cplusplus{{
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/NotifierConsts_m.h"
#include "inet/visualizer/base/PacketDropVisualizerBase.h"
}}

namespace inet::visualizer;

cplusplus{{
typedef cPacket * PacketPtr;
typedef cObject * InterfacePtr;
typedef cModule * ModulePtr;
}}

class inet::PacketDropDetails extends cObject;
class PacketPtr extends cObject;
class InterfacePtr extends cObject;
class ModulePtr extends cObject;
class noncobject Coord;

class PacketDrop extends inet::PacketDropDetails
{
    @existingClass();
    @readOnly();
    PacketPtr packet_ = nullptr;
    ModulePtr module = nullptr;
    ModulePtr networkNode = nullptr;
    InterfacePtr interfaceEntry = nullptr;
    Coord position @opaque;
}