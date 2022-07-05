//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//**************************************************************************
// * file:        Ieee802154MacHeader.msg
// *
// * author:      Daniel Willkomm
// *
// * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
// *              Technische Universitaet Berlin, Germany.
// *
// **************************************************************************
// * part of:     framework implementation developed by tkn
// * description: - base class for MAC layer packets
// *              - subclass to create your own MAC layer packets
// **************************************************************************/

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.linklayer.common.FcsMode;
import inet.linklayer.common.MacAddress;

cplusplus {{
namespace inet {
const B IEEE802154_FCS_SIZE = B(2);
}
}}

namespace inet;

//
// A CSMA mac header format definition
//
// @author Daniel Willkomm
//
class Ieee802154MacHeader extends FieldsChunk
{
    MacAddress srcAddr;
    MacAddress destAddr;
    long       sequenceId; // Sequence Number to detect duplicate messages
}

class Ieee802154Fcs extends FieldsChunk
{
    chunkLength = IEEE802154_FCS_SIZE;
    FcsMode fcsMode = FCS_MODE_UNDEFINED;
    uint16_t fcs = 0;   // The 2 byte frame check sequence (FCS) follows the last MAC payload byte. The FCS
                        // is calculated over the MPDU. The FCS calculation is based on the International
                        // Telecommunications Union (ITU) Cyclic Redundancy Check (CRC) and uses the
                        // following polynomial: x^16 + x^12 + x^5 + 1
}

