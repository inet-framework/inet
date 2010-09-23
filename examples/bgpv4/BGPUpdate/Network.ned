package inet.examples.bgpv4.BGPUpdate;

import inet.nodes.inet.StandardHost;
import inet.world.ChannelInstaller;


network BGPTest
{
    types:
        channel LINK_100 extends ned.DatarateChannel
        {
            parameters:
                delay = 0;
                datarate = 100Mbps;
        }
    submodules:
        channelInstaller: ChannelInstaller {
            parameters:
                @display("p=130,40;i=block/cogwheel_s");
                channelClass = "ThruputMeteringChannel";
                channelAttrs = "format=#N";
        }
        A: BGPRouter {
            parameters:
                @display("p=160,92");
            gates:
                pppg[1];
                ethg[1];
        }
        B: BGPRouter {
            parameters:
                @display("p=256,92");
            gates:
                pppg[1];
                ethg[1];
        }
        H1: StandardHost {
            parameters:
                @display("p=56,92;i=device/laptop");
            gates:
                ethg[1];
        }
        H2: StandardHost {
            parameters:
                @display("p=364,92;i=device/laptop");
            gates:
                ethg[1];
        }
    connections:
        H1.ethg[0] <--> LINK_100 <--> A.ethg[0];
        A.pppg[0] <--> LINK_100 <--> B.pppg[0];
        B.ethg[0] <--> LINK_100 <--> H2.ethg[0];
}

