//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.contract;

//
// This is the module interface for all Ethernet node types in a network.
//
moduleinterface IEthernetNetworkNode extends INetworkNode
{
    parameters:
        int numEthInterfaces;
        @display("i=device/pc");
    gates:
        inout ethg[numEthInterfaces];
}

