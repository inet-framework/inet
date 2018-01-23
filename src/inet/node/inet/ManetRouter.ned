package inet.node.inet;

import inet.routing.contract.IManetRouting;

module ManetRouter extends WirelessHost
{
    parameters:
        @display("i=device/cellphone");
        string routingProtocolType;
        forwarding = true;
        wlan[*].mgmtType = default("Ieee80211MgmtAdhoc");
    submodules:
        routing: <routingProtocolType> like IManetRouting {
            @display("p=700,300");
        }
    connections:
        routing.ipOut --> tn.in++;
        routing.ipIn <-- tn.out++;
}
