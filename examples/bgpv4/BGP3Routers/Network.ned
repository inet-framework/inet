
package inet.examples.bgpv4.BGP3Routers;

import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
import inet.nodes.bgp.BGPRouter;
import inet.nodes.ethernet.EtherSwitch;
import inet.nodes.inet.StandardHost;
import inet.nodes.ospfv2.OSPFRouter;
import inet.util.ThruputMeteringChannel;


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
        A: BGPRouter {
            parameters:
                @display("p=266,141");
            gates:
                pppg[4];
        }
        B: BGPRouter {
            parameters:
                @display("p=338,141");
            gates:
                pppg[4];
        }
        C: BGPRouter {
            parameters:
                @display("p=298,197");
            gates:
                pppg[4];
        }
        RA1: OSPFRouter {
            parameters:
                @display("p=198,113;i=device/router");
            gates:
                pppg[2];
                ethg[1];
        }
        RA2: OSPFRouter {
            parameters:
                @display("p=198,187;i=device/router");
            gates:
                pppg[2];
                ethg[1];
        }
        RB1: OSPFRouter {
            parameters:
                @display("p=406,105;i=device/router");
            gates:
                pppg[2];
                ethg[1];
        }
        RB2: OSPFRouter {
            parameters:
                @display("p=406,187;i=device/router");
            gates:
                pppg[2];
                ethg[1];
        }
        RC1: OSPFRouter {
            parameters:
                @display("p=262,273;i=device/router");
            gates:
                pppg[2];
                ethg[1];
        }
        RC2: OSPFRouter {
            parameters:
                @display("p=342,275;i=device/router");
            gates:
                pppg[2];
                ethg[1];
        }
        PA1: EtherSwitch {
            parameters:
                @display("p=124,70;i=device/switch");
            gates:
                ethg[3];
        }
        PA2: EtherSwitch {
            parameters:
                @display("p=114,190;i=device/switch");
            gates:
                ethg[3];
        }
        PB1: EtherSwitch {
            parameters:
                @display("p=484,78;i=device/switch");
            gates:
                ethg[3];
        }
        PB2: EtherSwitch {
            parameters:
                @display("p=492,190;i=device/switch");
            gates:
                ethg[3];
        }
        PC1: EtherSwitch {
            parameters:
                @display("p=244,350;i=device/switch");
            gates:
                ethg[3];
        }
        PC2: EtherSwitch {
            parameters:
                @display("p=364,350;i=device/switch");
            gates:
                ethg[3];
        }
        HA11: StandardHost {
            parameters:
                @display("p=42,34;i=device/pc");
            gates:
                ethg[1];
        }
        HA12: StandardHost {
            parameters:
                @display("p=34,90;i=device/pc");
            gates:
                ethg[1];
        }
        HA21: StandardHost {
            parameters:
                @display("p=34,156;i=device/pc");
            gates:
                ethg[1];
        }
        HA22: StandardHost {
            parameters:
                @display("p=34,218;i=device/pc");
            gates:
                ethg[1];
        }
        HB11: StandardHost {
            parameters:
                @display("p=558,34;i=device/pc");
            gates:
                ethg[1];
        }
        HB12: StandardHost {
            parameters:
                @display("p=558,90;i=device/pc");
            gates:
                ethg[1];
        }
        HB21: StandardHost {
            parameters:
                @display("p=558,156;i=device/pc");
            gates:
                ethg[1];
        }
        HB22: StandardHost {
            parameters:
                @display("p=558,220;i=device/pc");
            gates:
                ethg[1];
        }
        HC11: StandardHost {
            parameters:
                @display("p=206,426;i=device/pc");
            gates:
                ethg[1];
        }
        HC12: StandardHost {
            parameters:
                @display("p=262,426;i=device/pc");
            gates:
                ethg[1];
        }
        HC21: StandardHost {
            parameters:
                @display("p=334,428;i=device/pc");
            gates:
                ethg[1];
        }
        HC22: StandardHost {
            parameters:
                @display("p=398,428;i=device/pc");
            gates:
                ethg[1];
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=71,374");
            config = xmldoc("IPv4Config.xml");
            //assignAddresses = false;
            //assignDisjunctSubnetAddresses = false;
            addStaticRoutes = false;
            addDefaultRoutes = false;
            addSubnetRoutes = false;
        }
    connections:
        //LAN A1 et A2
        HA11.ethg[0] <--> LINK_100 <--> PA1.ethg[0];
        HA12.ethg[0] <--> LINK_100 <--> PA1.ethg[1];
        HA21.ethg[0] <--> LINK_100 <--> PA2.ethg[0];
        HA22.ethg[0] <--> LINK_100 <--> PA2.ethg[1];
        PA1.ethg[2] <--> LINK_100 <--> RA1.ethg[0];
        PA2.ethg[2] <--> LINK_100 <--> RA2.ethg[0];

        //LAN B1 et B2
        HB11.ethg[0] <--> LINK_100 <--> PB1.ethg[0];
        HB12.ethg[0] <--> LINK_100 <--> PB1.ethg[1];
        HB21.ethg[0] <--> LINK_100 <--> PB2.ethg[0];
        HB22.ethg[0] <--> LINK_100 <--> PB2.ethg[1];
        PB1.ethg[2] <--> LINK_100 <--> RB1.ethg[0];
        PB2.ethg[2] <--> LINK_100 <--> RB2.ethg[0];

        //LAN C1 et C2
        HC11.ethg[0] <--> LINK_100 <--> PC1.ethg[0];
        HC12.ethg[0] <--> LINK_100 <--> PC1.ethg[1];
        HC21.ethg[0] <--> LINK_100 <--> PC2.ethg[0];
        HC22.ethg[0] <--> LINK_100 <--> PC2.ethg[1];
        PC1.ethg[2] <--> LINK_100 <--> RC1.ethg[0];
        PC2.ethg[2] <--> LINK_100 <--> RC2.ethg[0];

        //OSPF A
        RA1.pppg[1] <--> LINK_100 <--> RA2.pppg[1];
        RA1.pppg[0] <--> LINK_100 <--> A.pppg[2];
        RA2.pppg[0] <--> LINK_100 <--> A.pppg[3];

        //OSPF B
        RB1.pppg[1] <--> LINK_100 <--> RB2.pppg[1];
        RB1.pppg[0] <--> LINK_100 <--> B.pppg[2];
        RB2.pppg[0] <--> LINK_100 <--> B.pppg[3];

        //OSPF C
        RC1.pppg[1] <--> LINK_100 <--> RC2.pppg[1];
        RC1.pppg[0] <--> LINK_100 <--> C.pppg[2];
        RC2.pppg[0] <--> LINK_100 <--> C.pppg[3];

        //BGP
        A.pppg[0] <--> LINK_100 <--> B.pppg[0];
        A.pppg[1] <--> LINK_100 <--> C.pppg[0];
        B.pppg[1] <--> LINK_100 <--> C.pppg[1];

}


