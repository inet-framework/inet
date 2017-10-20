// A LMAC packet header definition
//
// @author Anna Foerster

cplusplus {{
    #include "inet/common/INETDefs.h"
}}

import inet.linklayer.base.MacHeaderBase;

namespace inet;

class noncobject MACAddress;
class MacHeaderBase extends cObject;

class LMacHeader extends MacHeaderBase
{
    int        mySlot; // the node's current slot number
    MACAddress occupiedSlots[]; // currently known occupied slots
}
