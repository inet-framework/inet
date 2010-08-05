//
package inet.examples.bgpv4.BGPandOSPFSimple;

import inet.nodes.inet.BGPRouter;
import inet.nodes.inet.OSPFRouter;
import inet.nodes.inet.StandardHost;
import inet.world.ChannelInstaller;

channel LINK_100 extends ned.DatarateChannel // 100 Gbps
{
    parameters:
        delay = 0;
        //datarate = 100e6;
}

module Test
{
    submodules:
        channelInstaller: ChannelInstaller {
            parameters:
                channelClass = "ThruputMeteringChannel";
                channelAttrs = "format=#N";
                @display("p=130,40;i=block/cogwheel_s");
        }
        A: BGPRouter {
            parameters:
                @display("p=226,109;i=cisco/router");
            gates:
                pppg[2];
        }
        B: BGPRouter {
            parameters:
                @display("p=314,109;i=cisco/router");
            gates:
                pppg[2];
        }
        RA1: OSPFRouter {
            parameters:
                @display("p=134,105;i=device/router");
            gates:
                pppg[1];
                ethg[1];
        }
        RB1: OSPFRouter {
            parameters:
                @display("p=406,105;i=device/router");
            gates:
                pppg[1];
                ethg[1];
        }
        HA11: StandardHost {
            parameters:
                @display("p=42,98;i=device/pc");
            gates:
                ethg[1];
        }
        HB11: StandardHost {
            parameters:
                @display("p=494,98;i=device/pc");
            gates:
                ethg[1];
        }
    connections:
        //LAN A1 et A2
        HA11.ethg[0] <--> LINK_100 <--> RA1.ethg[0];

        //OSPF A
        RA1.pppg[0] <--> LINK_100 <--> A.pppg[1];

        //BGP
        A.pppg[0] <--> LINK_100 <--> B.pppg[0];

        //LAN B1 et B2
        HB11.ethg[0] <--> LINK_100 <--> RB1.ethg[0];

        //OSPF B
        RB1.pppg[0] <--> LINK_100 <--> B.pppg[1];
}

network BGPNet extends Test
{
    parameters:
}

