//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.transportlayer.sctp;

import inet.node.inet.StandardHost;

//
// SCTP NAT router
//
module SctpNatRouter extends StandardHost
{
    parameters:
        forwarding = true;
        natHook.networkProtocolModule = default(absPath(".ipv4.ip"));
        natHook.natTableModule = default(absPath(".natTable"));
        submodules:
        natHook: SctpNatHook {
            parameters:
                @display("p=175,640;i=block/table;is=s");
        }
        natTable: SctpNatTable {
            parameters:
                @display("p=75,640;i=block/table;is=s");
        }
}

