//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.physicallayer.common.Signal;

cplusplus {{
#include "inet/linklayer/ethernet/common/Ethernet.h"
}}

namespace inet::physicallayer;

enum EthernetSignalType
{
    NONE   = 0;
    BEACON = 1;
    COMMIT = 2;
    DATA   = 3;
    JAM    = 4;
}

//
// See Table 147–1—4B/5B Encoding in the IEEE Std 802.3cg, IEEE Standard for Ethernet.
//
// Name  4B   5B     Special function
//
// T     N/A  01101  ESD / HB
// R     N/A  00111  ESDOK / ESDBRS
// K     N/A  10001  ESDERR
// S     N/A  11001  ESDJAB
//
enum EthernetEsdType
{
    ESDNONE = -1;
    ESD     = 0;
    ESDBRS  = 1;
    ESDOK   = 2;
    ESDERR  = 3;
    ESDJAB  = 4;
}

//
// Abstract base class for various Ethernet traffic types:
// EtherJam, EtherFilledIfg and EtherPhyFrame.
//
packet EthernetSignalBase extends Signal
{
    bool srcMacFullDuplex;  // meta-info, for detecting model misconfiguration
    double bitrate;  // meta-info, for detecting model misconfiguration
    // end signal delimiter (ESD)
    EthernetEsdType esd1 = ESDNONE; // ESD or ESDBRS
    EthernetEsdType esd2 = ESDNONE; // ESDOK, ESDERR or ESDJAB
}

//
// Represents dataframe on the Ethernet.
//
packet EthernetSignal extends EthernetSignalBase
{
}

//
// Represents jam on the Ethernet.
//
packet EthernetJamSignal extends EthernetSignalBase
{
    srcMacFullDuplex = false;
    long abortedPkTreeID = 0;  // this field stores the packetTreeId of the aborted packet
}

//
// Represents a filled inter-frame gap in burst mode.
// Also used for calculating IFG times.
//
packet EthernetFilledIfgSignal extends EthernetSignalBase
{
    srcMacFullDuplex = false;
    bitLength = b(INTERFRAME_GAP_BITS).get();
}
