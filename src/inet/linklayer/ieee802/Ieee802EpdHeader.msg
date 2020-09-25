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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

namespace inet;

//
// This class represents an IEEE 802 EtherType Protocol Discrimination (EPD)
// header as defined in the section 9.2 EtherTypes of the IEEE Std 802-2014,
// IEEE Standard for Local and Metropolitan Area Networks: Overview and
// Architecture.
//
class Ieee802EpdHeader extends FieldsChunk
{
    chunkLength = B(2);
    uint16_t etherType; // Protocol identification values are assigned by the IEEE RA 19 and are used to identify the protocol that is to be invoked to process the user data in the frame.
}
