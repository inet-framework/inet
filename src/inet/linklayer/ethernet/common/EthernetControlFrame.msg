//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

namespace inet;

enum EthernetControlOpCode
{
    ETHERNET_CONTROL_PAUSE = 1;
}

//
// This is a base class for all IEEE 802.3 Ethernet control frames defined the
// IEEE Std 802.3-2018, IEEE Standard for Ethernet.
//
class EthernetControlFrameBase extends FieldsChunk
{
    int opCode = -1;
}

//
// This class represents an IEEE 802.3 Ethernet MAC Pause frame as defined in
// the section 31.4.1 MAC Control frame format of the IEEE Std 802.3-2018, IEEE
// Standard for Ethernet.
//
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-------------------------------+-------------------------------+
// |             OpCode            |           PauseTime           |
// +-------------------------------+-------------------------------+
//
class EthernetPauseFrame extends EthernetControlFrameBase
{
    chunkLength = B(4); // B(2) + B(2)
    opCode = ETHERNET_CONTROL_PAUSE;
    int pauseTime; // in 512 bit-time units
}
