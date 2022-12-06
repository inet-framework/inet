//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.contract;

//
// Interface for Rx processes. The Rx process checks received frames for
// errors, manages the NAV, and notifies other processes about the channel state
// (free or busy). The channel is free only if it is free according to both
// the physical (CCA) and the virtual (NAV-based) carrier sense algorithms.
// Correctly received frames are sent up to upper layers, corrupted
// frames are discarded.
//
moduleinterface IRx
{
    parameters:
        @display("i=block/rx");
}

