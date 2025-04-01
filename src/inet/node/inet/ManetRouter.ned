package inet.node.inet;

import inet.applications.contract.IApp;
import inet.routing.contract.IManetRouting;

//
// A mobile node with ad hoc routing capabilities intended for
// Mobile Ad hoc Networks (MANETs). Extends AdhocHost by adding support for
// MANET routing protocols.
//
// Mobile Ad hoc Networks consist of wireless nodes that can dynamically form a network
// without relying on pre-existing infrastructure.
//
module ManetRouter extends AdhocHost
{
    submodules:
        routingApp: <default("")> like IApp if typename != "" {      // AODV or other MANET routing over UDP/TCP protocol
            @display("p=913,75");
        }
        routing: <default("")> like IManetRouting  if typename != "" {
            @display("p=913,225");
        }
    connections:
        routing.ipOut --> tn.in++ if exists(routing);
        routing.ipIn <-- tn.out++ if exists(routing);
        routingApp.socketOut --> at.in++ if exists(routingApp);
        routingApp.socketIn <-- at.out++ if exists(routingApp);
}

