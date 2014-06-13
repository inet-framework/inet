
package inet.examples.rip.simpletest;

import inet.common.misc.ThruputMeteringChannel;
import inet.linklayer.ethernet.EtherHub;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.networklayer.configurator.ipv6.FlatNetworkConfigurator6;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.inet.StandardHost;
import inet.node.rip.RIPRouter;


network SimpleTest
{
    parameters:
        @display("p=10,10;b=712,152");
    types:
        channel C extends ThruputMeteringChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
            thruputDisplayFormat = "#N";
        }
    submodules:
        rtr: RoutingTableRecorder {
            @display("p=159,43");
        }
        H1: StandardHost {
            parameters:
                @display("p=56,92;i=device/laptop");
            gates:
                ethg[1];
        }
        N1: EtherHub {
            parameters:
                @display("p=184,92");
            gates:
                ethg[2];
        }
        R1: RIPRouter {
            parameters:
                @display("p=296,92");
            gates:
                ethg[2];
        }
        R2: RIPRouter {
            parameters:
                @display("p=416,92");
            gates:
                ethg[2];
        }
        N2: EtherHub {
            parameters:
                @display("p=532,92");
            gates:
                ethg[2];
        }
        H2: StandardHost {
            parameters:
                @display("p=660,92;i=device/laptop");
            gates:
                ethg[1];
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                config = xml("<config>"+
                            "<interface among='H1 R1' address='192.168.1.x' netmask='255.255.255.0' />"+
                            "<interface among='H2 R2' address='192.168.2.x' netmask='255.255.255.0' />"+
                            "<interface among='R1 R2' address='192.168.60.x' netmask='255.255.255.0' />"+
                            "<route hosts='H1 H2' destination='*' netmask='0.0.0.0' interface='eth0' />"+
                            "</config>");
                addStaticRoutes = false;
                addDefaultRoutes = false;
                @display("p=75,31");
        }
        configurator6: FlatNetworkConfigurator6 {
            @display("p=184,31;is=s");
        }
    connections:
        H1.ethg[0] <--> C <--> N1.ethg[0];
        N1.ethg[1] <--> C <--> R1.ethg[0];
        R1.ethg[1] <--> C <--> R2.ethg[0];
        R2.ethg[1] <--> C <--> N2.ethg[0];
        N2.ethg[1] <--> C <--> H2.ethg[0];
}

