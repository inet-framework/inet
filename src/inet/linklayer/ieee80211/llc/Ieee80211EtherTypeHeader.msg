//
// Copyright (C) 2018 Raphael Riebl, TH Ingolstadt
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

cplusplus {{
#include "inet/common/ProtocolGroup.h"
}}

namespace inet;

// IEEE 802.11 in 5.9 GHz band requires EtherType Protocol Discrimination (EPD)
class Ieee80211EtherTypeHeader extends FieldsChunk
{
    chunkLength = B(2);
    int etherType = -1; // ~EtherType (2 bytes)
}

cplusplus(Ieee80211EtherTypeHeader) {{
    virtual const Protocol* getProtocol() const
    {
        return ProtocolGroup::ethertype.findProtocol(getEtherType());
    }
}}
