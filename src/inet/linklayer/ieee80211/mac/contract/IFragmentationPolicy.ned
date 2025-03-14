//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.contract;

//
// Interface for IEEE 802.11 MAC fragmentation policies that determine when and how 
// to fragment MAC frames. Fragmentation policies divide large frames into smaller 
// fragments when they exceed a certain threshold, improving transmission reliability 
// in error-prone wireless environments. Each fragment is transmitted as a separate 
// frame with its own sequence number and requires individual acknowledgment.
//
// @see ~BasicFragmentationPolicy
//
moduleinterface IFragmentationPolicy
{
    parameters:
        @display("i=block/control");
}
