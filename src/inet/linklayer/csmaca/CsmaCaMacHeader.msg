//
// Copyright (C) 2016 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.linklayer.common.MacAddress;

namespace inet;

class CsmaCaMacHeader extends FieldsChunk
{
    MacAddress transmitterAddress;
    MacAddress receiverAddress;
}

class CsmaCaMacAckHeader extends CsmaCaMacHeader
{
    chunkLength = B(13);
}

class CsmaCaMacDataHeader extends CsmaCaMacHeader
{
    chunkLength = B(16);
    int networkProtocol = -1;
    int priority = -1;  // 802.1d UP (User Priority)
}

enum FcsMode {
    FCS_DECLARED = 1;
    FCS_COMPUTED = 2;
}

class CsmaCaMacTrailer extends FieldsChunk
{
    chunkLength = B(4);
    uint32_t fcs = 0;
    FcsMode fcsMode;
}
