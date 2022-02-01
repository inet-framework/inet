//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.virtual;

simple VirtualTunnel
{
    parameters:
        string interfaceTableModule;
        string realInterface;
        string protocol = default(vlanId != -1 ? "ieee8021qctag" : "");
        string address @enum("auto","copy") = default("copy");
        int vlanId = default(-1);
        bool steal = default(vlanId != -1);
        @display("i=block/cogwheel");
    gates:
        input upperLayerIn;
        output upperLayerOut;
}
