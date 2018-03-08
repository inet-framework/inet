// A LMAC packet header definition
//
// @author Anna Foerster

import inet.common.INETDefs;
import inet.linklayer.base.MacHeaderBase;

namespace inet;

//TODO split to 2 enums: for packets / for self messages
enum LMacType
{
    LMAC_CONTROL = 167;     // packet type
    LMAC_TIMEOUT = 168;
    LMAC_WAKEUP = 169;
    LMAC_SEND_DATA = 170;
    LMAC_SETUP_PHASE_END = 171;
    LMAC_CHECK_CHANNEL = 172;
    LMAC_SOMEBODY = 173;
    LMAC_DATA = 174;     // packet type
    LMAC_START_LMAC = 175;
    LMAC_SEND_CONTROL = 176;
};

class LMacHeader extends MacHeaderBase
{
    LMacType   type;
    int        mySlot; // the node's current slot number
    MacAddress occupiedSlots[]; // currently known occupied slots
}
