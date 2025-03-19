//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.contract;

//
// Interface for wireless network interfaces.
//
moduleinterface IWirelessInterface extends INetworkInterface
{
    parameters:
        @display("i=block/ifcard");
    gates:
        input radioIn @labels(IWirelessSignal);
}

