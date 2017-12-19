// A LMAC packet header definition
//
// @author Anna Foerster

import inet.common.INETDefs;
import inet.linklayer.base.MacHeaderBase;

namespace inet;

class LightweightMacHeader extends MacHeaderBase
{
    int        mySlot; // the node's current slot number
    MacAddress occupiedSlots[]; // currently known occupied slots
}
