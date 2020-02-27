package inet.examples.bgpv4.BgpUpdate;

import inet.common.misc.ThruputMeteringChannel;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import inet.visualizer.integrated.IntegratedCanvasVisualizer;
import ned.TransmissionChannel;


network BgpNetwork
{
    parameters:
        *.eth[*].bitrate = 100 Mbps;
    types:
        channel LINK_100 extends ThruputMeteringChannel
        {
            datarate = 100 Mbps;
            delay = 50us;
            displayAsTooltip = true;
            thruputDisplayFormat = "#N";
        }
        channel ethlink extends TransmissionChannel
        {
            delay = 50us;
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
    connections:
        H1.ethg[0] <--> ethlink <--> A.ethg[0];
        A.pppg[0] <--> LINK_100 <--> B.pppg[0];
        B.ethg[0] <--> ethlink <--> H2.ethg[0];
}

