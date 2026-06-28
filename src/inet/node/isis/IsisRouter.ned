//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.node.isis;

import inet.node.inet.Router;
import inet.routing.contract.IIsis;

//
// A router running the IS-IS routing protocol. Extends ~Router by activating
// the node's IEEE 802.2 LLC layer (IS-IS PDUs ride directly on the link layer)
// and adding an ~Isis submodule wired into the network/link-layer message
// dispatcher (`nl`), through which it reaches the LLC and the interfaces.
//
module IsisRouter extends Router
{
    parameters:
        @display("i=abstract/router");
        llc.typename = default("Ieee8022Llc");
        llc.registerProtocol = default(true);
    submodules:
        isis: <"Isis"> like IIsis {
            @display("p=825,376");
        }
    connections allowunconnected:
        isis.socketOut --> nl.in++;
        isis.socketIn <-- nl.out++;
}
