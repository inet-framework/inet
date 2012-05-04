

package inet.examples.ospfv2.areas;

import inet.base.UnimplementedModule;
import inet.util.ThruputMeteringChannel;


network OSPF_AreaTest
{
    submodules:
        Area1: TwoNetsArea {
            parameters:
                @display("p=108,148;b=136,128");
            gates:
                ethg[1];
        }
        Area2: OneNetArea {
            parameters:
                @display("p=322,148;b=136,128");
            gates:
                ethg[1];
        }
        configurator: UnimplementedModule {
            @display("p=75,43");
        }
    connections:
        Area1.ethg[0] <--> ThruputMeteringChannel {  delay = 0.1us; datarate = 100 Mbps; thruputDisplayFormat = "#N"; } <--> Area2.ethg[0];
}

