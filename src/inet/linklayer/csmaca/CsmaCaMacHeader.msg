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

cplusplus {{
#include "inet/linklayer/common/MACAddress.h"
#include "inet/common/packet/chunk/FieldsChunk.h"
}}

namespace inet;

class noncobject MACAddress;
class FieldsChunk extends cObject;

class CsmaCaMacHeader extends FieldsChunk
{
    MACAddress transmitterAddress;
    MACAddress receiverAddress;
}

class CsmaCaMacAckHeader extends CsmaCaMacHeader
{
}

class CsmaCaMacDataHeader extends CsmaCaMacHeader
{
    int networkProtocol = -1;
    int priority;  // 802.1d UP (User Priority) 
}

enum FcsMode {
    FCS_DECLARED = 1;
    FCS_COMPUTED = 2;
}

class CsmaCaMacTrailer extends FieldsChunk
{
    chunkLength = B(4);
    uint32_t fcs = 0;
    int fcsMode @enum(FcsMode) = -1;
}
