// A LMAC packet header definition
//
// @author Anna Foerster

import inet.common.INETDefs;
import inet.linklayer.base.MacHeaderBase;

namespace inet;

class noncobject MACAddress;

class LMacHeader extends MacHeaderBase
{
    int        mySlot; // the node's current slot number
    MACAddress occupiedSlots[]; // currently known occupied slots
}
