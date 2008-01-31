

package inet.examples.ospfv2.areas;

import inet.world.ChannelInstaller;


network OSPF_AreaTest
{
    submodules:
        channelInstaller: ChannelInstaller {
            parameters:
                channelClass = "ThruputMeteringChannel";
                channelAttrs = "format=#N";
                @display("p=456,36");
        }
        Area1: TwoNetsArea {
            parameters:
                @display("p=108,112;b=136,128");
            gates:
                ethg[1];
        }
        Area2: OneNetArea {
            parameters:
                @display("p=324,112;b=136,128");
            gates:
                ethg[1];
        }
    connections:
        Area1.ethg[0] <--> {  delay = 0.1us; } <--> Area2.ethg[0];
}

