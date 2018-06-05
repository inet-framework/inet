package inet.node.inet;

import inet.routing.contract.IManetRouting;

module ManetRouter extends AdhocHost
{
    submodules:
        routing: <> like IManetRouting {
            @display("p=700,300");
        }
    connections:
        routing.ipOut --> tn.in++;
        routing.ipIn <-- tn.out++;
}
