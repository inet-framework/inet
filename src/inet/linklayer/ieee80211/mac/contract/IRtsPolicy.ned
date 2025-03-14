//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.contract;

//
// Interface for IEEE 802.11 Request-to-Send (RTS) policies that determine when 
// to use the RTS/CTS mechanism for frame transmissions. RTS policies decide whether 
// to send an RTS frame before a data frame based on factors such as frame size, 
// destination, and network conditions. The RTS/CTS exchange helps prevent hidden 
// node problems by reserving the medium before transmitting larger data frames, 
// reducing collision probability in wireless networks with limited visibility 
// between nodes.
//
// @see ~RtsPolicy, ~QosRtsPolicy, ~ICtsPolicy
//
moduleinterface IRtsPolicy
{
    parameters:
        string rateSelectionModule;
        @display("i=block/control");
}
