// A LMAC packet format definition
//
// @author Anna Foerster

cplusplus {{
    #include "inet/linklayer/base/MacHeaderBase_m.h"
}}

namespace inet;

class noncobject MACAddress;
class MacHeaderBase;

packet LMacFrame extends MacHeaderBase
{
    int        mySlot; // the node's current slot number
    MACAddress occupiedSlots[]; // currently known occupied slots
}
