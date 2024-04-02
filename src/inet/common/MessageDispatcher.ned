//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.common;

import inet.queueing.base.PacketProcessorBase;

//
// This module connects multiple applications, protocols and interfaces with
// each other and automatically dispatches messages and packets between them.
// It allows many different configurations from layered architectures where
// message dispatchers separate different communication layers to centralized
// architectures where a single message dispatcher is connected to all components.
//
// Configuring the dispatch mechanism:
//  - protocols must register by calling registerService and/or registerProtocol
//  - interfaces must register by calling registerInterface
//  - sockets must register by sending socket messages
//  - packets must have tags attached which specify the destination protocol, interface or socket
//
simple MessageDispatcher extends PacketProcessorBase
{
    parameters:
        string interfaceTableModule;
        bool forwardServiceRegistration = default(true);
        bool forwardProtocolRegistration = default(true);
        object interfaceMapping = default({}); // maps interface names to submodule names to override automatically learned dispatching (e.g. {"eth0", "bridging"}); * means all learned interfaces, ? means all other interfaces
        object serviceMapping = default({}); // maps protocols to submodule names to override automatically learned dispatching (e.g. {"ethernetmac": "bridging"}); * means all learned services, ? means all other services
        object protocolMapping = default({}); // maps protocols to submodule names to override automatically learned dispatching (e.g. {"ipv4": "bridging"}); * means all learned protocols, ? means all other protocols
        @class(MessageDispatcher);
        @display("i=-;b=100,5,,,,1;");
    gates:
        input in[];
        output out[];
}

