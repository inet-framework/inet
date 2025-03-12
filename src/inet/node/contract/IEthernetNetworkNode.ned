//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.contract;

//
// Interface for all network nodes that have Ethernet connectivity. Extends the base
// INetworkNode interface by adding Ethernet-specific gates and parameters.
//
// This interface is implemented by various node types that participate in
// Ethernet networks, such as hosts, switches, and routers with Ethernet interfaces.
// It ensures that all Ethernet-capable nodes have the necessary gates to connect
// to Ethernet networks.
//
// @see ~EthernetHost, ~EthernetSwitch, ~StandardHost, ~Router
//
moduleinterface IEthernetNetworkNode extends INetworkNode
{
    parameters:
        int numEthInterfaces;
        @display("i=device/pc");
    gates:
        inout ethg[numEthInterfaces];
}

