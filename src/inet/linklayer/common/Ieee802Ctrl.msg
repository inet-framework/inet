//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.linklayer.common.MacAddress;

namespace inet;

//
// Message kind values used with in communication between L3 and IEEE 802 L2
//
enum Ieee802MessageKind
{
    IEEE802CTRL_DATA = 2003;  // data to/from higher layer
    IEEE802CTRL_SENDPAUSE = 2006;  // higher layer wants MAC to send PAUSE frame
}

// used with IEEE802CTRL_SENDPAUSE
class Ieee802PauseCommand extends cObject
{
    MacAddress destinationAddress;
    int pauseUnits;
}
