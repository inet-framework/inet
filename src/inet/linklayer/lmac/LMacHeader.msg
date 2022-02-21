// A LMAC packet header definition
//
// @author Anna Foerster

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.linklayer.common.MacAddress;

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

//
// Base class for ~LMac headers.
//
class LMacHeaderBase extends FieldsChunk
{
    MacAddress srcAddr;
    MacAddress destAddr;
    LMacType   type;
    int        mySlot; // the node's current slot number
    MacAddress occupiedSlots[]; // currently known occupied slots
}

//
// Control Frame for ~LMac. Frame size is configurable
// in the MAC layer.
//
class LMacControlFrame extends LMacHeaderBase
{
}

//
// Dataframe header for ~LMac. Frame size is configurable
// in the MAC layer.
//
class LMacDataFrameHeader extends LMacHeaderBase
{
    int        networkProtocol = -1;
}
