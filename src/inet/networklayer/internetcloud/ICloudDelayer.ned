//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.internetcloud;

//
// Delay module interface for InternetCloud.
//
// Delays or drops the packets based on which interface card the packet
// has arrived on and on which interface it is leaving the cloud.
// This module registers self to ~Ipv4 module as an INetfilter::IHook hook
// and uses the datagramForwardHook for delay/drop packets
//
// @see ~InternetCloud
//
moduleinterface ICloudDelayer
{
    parameters:
        string networkProtocolModule;
        @display("i=block/delay");
}

