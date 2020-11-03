//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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
