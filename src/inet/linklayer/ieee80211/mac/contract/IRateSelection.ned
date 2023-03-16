//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.contract;

//
// Interface for frame rate selection modules. Rate selection decides what bit rate
// (or MCS) should be used for frames of various types, such as control frames,
// management frames and data frames.
//
moduleinterface IRateSelection
{
    parameters:
        @display("i=block/cogwheel");
}

