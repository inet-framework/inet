package inet.examples.bgpv4.BgpOpen;

import inet.common.misc.ThruputMeteringChannel;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.visualizer.integrated.IntegratedCanvasVisualizer;


network BgpNetwork
{
    types:
        channel LINK_100 extends ThruputMeteringChannel
        {
            delay = 50us;
            datarate = 100Mbps;
            displayAsTooltip = true;
            thruputDisplayFormat = "#N";
        }
    submodules:
        visualizer: IntegratedCanvasVisualizer {
            parameters:
                @display("p=100,100;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,200;is=s");
                config = xmldoc("IPv4Config.xml");
                addStaticRoutes = false;
                addDefaultRoutes = false;
                addSubnetRoutes = false;
        }
        A: BGPRouterSimple {
            parameters:
                @display("p=250,100");
        }
        B: BGPRouterSimple {
            parameters:
                @display("p=400,100");
        }
    connections:
        A.pppg++ <--> LINK_100 <--> B.pppg++;
}

