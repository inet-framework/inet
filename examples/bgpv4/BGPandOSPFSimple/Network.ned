//
package inet.examples.bgpv4.BGPandOSPFSimple;

import inet.common.misc.ThruputMeteringChannel;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.bgp.BGPRouter;
import inet.node.inet.StandardHost;
import inet.node.ospfv2.OSPFRouter;


network BGPNet
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
        A: BGPRouter {
            parameters:
                @display("p=226,109");
            gates:
                pppg[2];
        }
        B: BGPRouter {
            parameters:
                @display("p=314,109");
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
        configurator: IPv4NetworkConfigurator {
            @display("p=275,23");
            config = xmldoc("IPv4Config.xml");
            //assignAddresses = false;
            //assignDisjunctSubnetAddresses = false;
            addStaticRoutes = false;
            addDefaultRoutes = false;
            addSubnetRoutes = false;
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


