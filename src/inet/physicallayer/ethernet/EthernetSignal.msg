//
// Copyright (C) OpenSim Ltd.
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
import inet.physicallayer.common.packetlevel.Signal;

cplusplus {{
#include "inet/linklayer/ethernet/Ethernet.h"
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

