//
// Copyright (C) OpenSim Ltd
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
#include "inet/common/packet/chunk/FieldsChunk.h"
}}

namespace inet;

class FieldsChunk extends cObject;

namespace inet::physicallayer;

class Ieee80211PhyHeader extends FieldsChunk
{
}

// The length field is a 12-bit integer indicates the number of
// octets in the PSDU that the MAC is currently requesting the PHY to
// transmit.
//
// The rate field (18.3.4.2 RATE field) is a 4-bit integer carries
// information about the modulation and coding.
//
// There are other fields which are not explicitly represented here,
// for e.g. Parity, Reserved, SERVICE, Pad Bits.
//
// The PSDU field must be contained as an encapsulated packet.
//
// This packet is intended to represent the whole PPDU frame in the
// IEEE 802.11 PHY. For detailed description, you may refer to 18.3.2 PLCP
// frame format, Figure 18-1—PPDU frame format in IEEE Std. 802.11-2012.
//
packet Ieee80211OfdmPhyHeader extends Ieee80211PhyHeader
{
    // The PLCP header is composed of RATE (4), Reserved (1), LENGTH (12), Parity (1),
    // Tail (6) and SERVICE (16) fields.
    chunkLength = B(5);
    uint8_t rate;
    uint16_t lengthField;
}
