// A LMAC packet format definition
//
// @author Anna Foerster

cplusplus {{
    #include "inet/linklayer/common/MACFrameBase_m.h"
}}

namespace inet;

class noncobject MACAddress;
class MACFrameBase;

packet LMacFrame extends MACFrameBase
{
    int        mySlot; // the node's current slot number
    MACAddress occupiedSlots[]; // currently known occupied slots
}
