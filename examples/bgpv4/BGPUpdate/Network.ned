package inet.examples.bgpv4.BGPUpdate;

import inet.common.misc.ThruputMeteringChannel;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.StandardHost;


network BGPTest
{
    types:
        channel LINK_100 extends ThruputMeteringChannel
        {
            parameters:
                delay = 0;
                datarate = 100Mbps;
                thruputDisplayFormat = "#N";
        }
    submodules:
        A: BGPRouterEx {
            parameters:
                @display("p=160,92");
            gates:
                pppg[1];
                ethg[1];
        }
        B: BGPRouterEx {
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
        configurator: IPv4NetworkConfigurator {
            @display("p=208,23");
            config = xmldoc("IPv4Config.xml");
            //assignAddresses = false;
            //assignDisjunctSubnetAddresses = false;
            addStaticRoutes = false;
            addDefaultRoutes = false;
            addSubnetRoutes = false;
        }
    connections:
        H1.ethg[0] <--> LINK_100 <--> A.ethg[0];
        A.pppg[0] <--> LINK_100 <--> B.pppg[0];
        B.ethg[0] <--> LINK_100 <--> H2.ethg[0];
}

