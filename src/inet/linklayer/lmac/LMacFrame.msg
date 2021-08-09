// A LMAC packet format definition
//
// @author Anna Foerster

import inet.linklayer.common.MACAddress;
import inet.linklayer.base.MACFrameBase;

namespace inet;

packet LMacFrame extends MACFrameBase
{
    int        mySlot; // the node's current slot number
    MACAddress occupiedSlots[]; // currently known occupied slots
}
