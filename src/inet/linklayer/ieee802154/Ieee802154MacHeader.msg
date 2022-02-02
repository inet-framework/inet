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
import inet.linklayer.common.MacAddress;

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
    int networkProtocol = -1;
    long       sequenceId; // Sequence Number to detect duplicate messages
}
