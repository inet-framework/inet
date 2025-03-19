//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.contract;

//
// Interface for Ethernet network interfaces.
//
moduleinterface IEthernetInterface extends IWiredInterface
{
    parameters:
        double bitrate @unit(bps);
        @display("i=block/ifcard");
}

