package inet.node.inet;

import inet.applications.contract.IApp;
import inet.routing.contract.IManetRouting;


module ManetRouter extends AdhocHost
{
    submodules:
        routingApp: <default("")> like IApp if typename != "" {      // Aodv or other manet routing over UDP/TCP protocol
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

