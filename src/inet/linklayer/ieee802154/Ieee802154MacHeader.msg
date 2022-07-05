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

enum Ieee802154MacFrameType {
    IEEE802154_FCF_BEACON = 0;
    IEEE802154_FCF_DATA = 1;
    IEEE802154_FCF_ACK = 2;
    IEEE802154_FCF_CMD = 3;
    // 4..7: reserved
};

enum Ieee802154MacAddressingMode {
    IEEE802154_FCF_ADDR_NONE =  0x0000; // PAN identifier and address fields are not present
    IEEE802154_FCF_ADDR_SHORT = 0x0002; // address field contains a short address (16 bit)
    IEEE802154_FCF_ADDR_EXT =   0x0003; // address field contains an extended address (64 bit)
}

// Frame version definitions
enum Ieee802154FrameVersion {
    IEEE802154_VERSION_2003 = 0x0;
    IEEE802154_VERSION_2006 = 0x1;
}

class Ieee802154MacHeader_ extends FieldsChunk
{
    // FCF:
    Ieee802154MacFrameType frameType @bit(3);
    bool securityEnabled = false;   // security not supported yet
    bool framePending;
    bool ackRequest;
    bool panIdCompression; // The PAN ID Compression field specifies whether the MAC frame is
                           // to be sent containing only one of the PAN identifier fields
                           // when both source and destination addresses are present.
                           // If this field is set to true and both the source and destination addresses are present,
                           // the frame shall contain only the DestPanId field.
    short reserved @bit(3) = 0;
    Ieee802154MacAddressingMode destAddressingMode @bit(2) = IEEE802154_FCF_ADDR_EXT;
    short frameVersion @bit(2) = IEEE802154_VERSION_2006;
    Ieee802154MacAddressingMode sourceAddressingMode @bit(2) = IEEE802154_FCF_ADDR_EXT;

    uint8_t sequenceNumber; // Sequence Number to detect duplicate messages

    uint16_t destPanId = 0; // 0/16 bits, specified by destAddressingMode
    uint64_t destAddress = 0; // 0/16/64 bits, specified by destAddressingMode
    uint16_t srcPanId = 0; // 0/16 bits, specified by srcAddressingMode
    uint64_t srcAddress = 0; // 0/16/64 bits, specified by srcAddressingMode
    // AuxiliarySecurityHeader auxiliarySecurityHeader; // 0/5/6/10/14 bytes
}

// TODO: Protocol:
//  Ieee802154MacHeader doesn't contain the higher layer protocol type
//  - Zigbee begins directly after Ieee802154MacHeader
//
