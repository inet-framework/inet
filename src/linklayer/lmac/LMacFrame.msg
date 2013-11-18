// A LMAC packet format definition
//
// @author Anna Foerster

class noncobject MACAddress;

cplusplus {{
    #include "MACAddress.h"
}}

packet LMacFrame
{
    int        mySlot; // the node's current slot number
    MACAddress occupiedSlots[]; // currently known occupied slots
    MACAddress destAddr;   // destination mac address
    MACAddress srcAddr;    // source mac address
}
