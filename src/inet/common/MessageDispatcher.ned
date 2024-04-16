//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.common;

import inet.queueing.base.PacketProcessorBase;

//
// This module facilitates the interconnection of applications, protocols, and network interfaces,
// dispatching messages and packets among them. It supports diverse configurations, ranging from
// layered architectures with distinct message dispatchers for separate communication layers, to
// centralized structures where a single dispatcher manages connections to all components.
//
// This module keeps track of four different mappings for dispatching messages and packets:
//  - network interface mapping specifies which gate should be used when dispatching towards a specific
//    network interface
//  - service mapping specifies which gate should be used when dispatching service data units (SDUs)
//    towards a specific protocol (e.g. IPv4 SDU, Ethernet SDU)
//  - protocol mapping specifies which gate should be used when dispatching protocol data units (PDUs)
//    towards a specific protocol (e.g. IPv4 PDU, Ethernet PDU)
//  - socket mapping specifies which gate should be used when dispatching towards a specific socket
//
// To configure the dispatch mechanism:
//  - protocol modules must register using registerService() and/or registerProtocol() functions
//  - network interface modules must register using registerInterface() function
//  - sockets must use ~SocketReq or ~SocketInd tags when sending socket open/close commands
//  - manual configuration is also possible using the module's mapping parameters
//
// To use the disptach mechanism, messages and packets must have a specific tag attached that define the target:
//  - service or protocol targets can be specified using the ~DispatchProtocolReq tag
//  - network interface targets can be specified using the ~InterfaceReq tag
//  - socket targets can be specified using the ~SocketReq or ~SocketInd tags
//
// Automatically learned services, protocols and network interfaces are shared among connected message dispatchers.
//
simple MessageDispatcher extends PacketProcessorBase
{
    parameters:
        string interfaceTableModule; // module path to the ~InterfaceTable module
        object interfaceMapping = default({}); // maps network interface names to connected submodule names (e.g. {"eth0": "bridging"}), overrides automatically learned dispatching; * overrides all learned network interfaces, ? overrides default
        object serviceMapping = default({}); // maps protocols to connected submodule names (e.g. {"ethernetmac": "bridging"}), overrides automatically learned dispatching; * overrides all learned services, ? overrides default
        object protocolMapping = default({}); // maps protocols to connected submodule names (e.g. {"ipv4": "bridging"}), overrides automatically learned dispatching; * overrides all learned protocols, ? overrides default
        @class(MessageDispatcher);
        @display("i=-;b=100,5,,,,1;");
    gates:
        input in[];
        output out[];
}

