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

import inet.transportlayer.common.CRC;

namespace inet;

class FieldsChunk extends cObject;

class APSKPhyHeader extends FieldsChunk
{
    chunkLength = B(6);
    uint16_t lengthField;
    uint16_t crc = 0;
    int crcMode @enum(CrcMode) = -1;
}
