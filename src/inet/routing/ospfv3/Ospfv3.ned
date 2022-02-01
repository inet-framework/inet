//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.routing.ospfv3;

import inet.routing.contract.IOspf;

module Ospfv3 like IOspf
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
        string routingTableModule6;
        *.interfaceTableModule = default(absPath(this.interfaceTableModule));
        *.routingTableModule = default(absPath(this.routingTableModule));
        *.routingTableModule6 = default(absPath(this.routingTableModule6));
        @display("bgb=510,238;i=block/network2");
    gates:
        input ipIn @labels(Ipv6ControlInfo/up);
        output ipOut @labels(Ipv6ControlInfo/down);

    submodules:
        ospfv3Splitter: Ospfv3Splitter {
            @display("p=254,176");
        }

    connections:
        ipIn --> ospfv3Splitter.ipIn;
        ipOut <-- ospfv3Splitter.ipOut;
}

