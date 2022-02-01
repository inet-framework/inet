//
// Copyright (C) 2018 Raphael Riebl, TH Ingolstadt
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
