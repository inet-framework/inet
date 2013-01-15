
package inet.nodes.Ieee8021d;

import inet.applications.ethernet.EthernetApplication;
import inet.linklayer.ieee8021d.ethernet.EthernetLink;
import inet.linklayer.ethernet.EtherMAC;


//
// Simple ethernet host. EtherHost based. LLC removed.
//
module EthernetHost
{
    parameters:
        @node();
        @labels(node,ethernet-node);
        @display("i=device/pc2");
    gates:
        inout ethg @labels(EtherFrame);
    submodules:
        application: EthernetApplication {
            parameters:
                @display("p=116,51,col");
        }
        linklayer: EthernetLink {
            parameters:
                @display("p=116,119");
            gates:
                upperLayerIn;
                upperLayerOut;
        }
        mac: EtherMAC {
            parameters:
                queueModule = "";
                @display("p=116,200;q=queue");
        }
    connections:
        linklayer.lowerLayerIn <-- mac.upperLayerOut;
        linklayer.lowerLayerOut --> mac.upperLayerIn;

        mac.phys <--> ethg;

        application.out --> linklayer.upperLayerIn;
        application.in <-- linklayer.upperLayerOut;



//        cli.out --> relay.upperLayerIn[0];
//        cli.in <-- relay.upperLayerOut[0];

}

