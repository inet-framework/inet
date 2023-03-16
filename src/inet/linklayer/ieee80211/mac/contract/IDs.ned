//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.contract;

//
// Interface for modules that implement the frame Distribution Service
// for IEEE 802.11. The job of the Distribution Service is deciding what
// to do with correctly received frames. They may be discarded (e.g. on
// address mismatch), sent up to higher layers, and/or transmitted back
// on the wireless LAN after switching addresses.
//
moduleinterface IDs
{
    parameters:
        @display("i=block/routing");
}

