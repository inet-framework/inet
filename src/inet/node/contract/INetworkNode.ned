//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.contract;

//
// INetworkNode defines the common interface for all network node types in INET.
// It serves as the base interface that all node modules must implement to be
// recognized and treated as network nodes within the simulation.
//
// This interface provides no specific functionality itself but establishes
// the common ground for different node types such as hosts, routers, switches,
// and access points.
//
moduleinterface INetworkNode
{
    parameters:
        @display("i=device/pc");
        @networkNode;
        @labels(node,ethernet-node,wireless-node);
}

