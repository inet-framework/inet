package inet.examples.bgpv4.BgpUpdate;

import inet.common.misc.ThruputMeteringChannel;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.visualizer.integrated.IntegratedCanvasVisualizer;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;

network BGPTest
{
    types:
        channel LINK_100 extends DatarateChannel
        {
            parameters:
                delay = 50us;
                datarate = 100Mbps;
        }

    submodules:
        A: BGPRouterEx {
            parameters:
                @display("p=400,100");
            gates:
                pppg[1];
                ethg[1];
        }
        B: BGPRouterEx {
            parameters:
                @display("p=550,100");
            gates:
                pppg[1];
                ethg[1];
        }
        H1: StandardHost {
            parameters:
                @display("p=250,100;i=device/laptop");
            gates:
                ethg[1];
        }
        H2: StandardHost {
            parameters:
                @display("p=700,100;i=device/laptop");
            gates:
                ethg[1];
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100");
            config = xmldoc("IPv4Config.xml");
            addStaticRoutes = false;
            addDefaultRoutes = false;
            addSubnetRoutes = false;
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=100,38");
        }

    connections:
        H1.ethg[0] <--> LINK_100 <--> A.ethg[0];
        A.pppg[0] <--> LINK_100 <--> B.pppg[0];
        B.ethg[0] <--> LINK_100 <--> H2.ethg[0];
}

