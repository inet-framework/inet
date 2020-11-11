
package inet.examples.ospfv2.areas;

import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.StandardHost;
import inet.node.ospfv2.OspfRouter;
import ned.DatarateChannel;


module OneNetArea
{
    parameters:
        @display("p=10,10;b=404,108");
    gates:
        inout ethg[];
    types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
        }
    submodules:
        H3: StandardHost {
            parameters:
                @display("p=550,110;i=device/laptop");
            gates:
                ethg[1];
        }
        N3: EthernetSwitch {
            parameters:
                @display("p=400,110");
            gates:
                ethg[2];
        }
        R5: OspfRouter {
            parameters:
                @display("p=250,110");
            gates:
                ethg[2];
        }
        R4: OspfRouter {
            parameters:
                @display("p=100,110");
            gates:
                ethg[2];
        }
    connections:
        R4.ethg[0] <--> C <--> R5.ethg[1];
        R5.ethg[0] <--> C <--> N3.ethg[1];
        N3.ethg[0] <--> C <--> H3.ethg[0];

        // towards other areas
        ethg[0] <--> R4.ethg[1];
}

