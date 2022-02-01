//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.contract;

//
// Contains the common interface for all node types in a network.
//
moduleinterface INetworkNode
{
    parameters:
        @display("i=device/pc");
        @networkNode;
        @labels(node,ethernet-node,wireless-node);
}

