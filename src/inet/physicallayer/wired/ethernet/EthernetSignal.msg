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

//
// Abstract base class for various Ethernet traffic types:
// EtherJam, EtherFilledIfg and EtherPhyFrame.
//
packet EthernetSignalBase extends Signal
{
    bool srcMacFullDuplex;  // meta-info, for detecting model misconfiguration
    double bitrate;  // meta-info, for detecting model misconfiguration
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
