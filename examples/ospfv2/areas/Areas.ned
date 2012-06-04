

package inet.examples.ospfv2.areas;

import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
import inet.util.ThruputMeteringChannel;


network OSPF_AreaTest
{
    submodules:
        Area1: TwoNetsArea {
            parameters:
                @display("p=108,148;b=136,128");
            gates:
                ethg[1];
        }
        Area2: OneNetArea {
            parameters:
                @display("p=322,148;b=136,128");
            gates:
                ethg[1];
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                config = xml("<config>"+
                            "<interface among='Area1.*' address='192.168.11.x' netmask='255.255.255.x' />"+

                            "<interface among='Area2.*' address='192.168.22.x' netmask='255.255.255.x' />"+

                            "<interface hosts='Area1.*' towards='Area2.*' address='192.168.0.x' netmask='255.255.255.x' />"+
                            "<interface hosts='Area2.*' towards='Area1.*' address='192.168.0.x' netmask='255.255.255.x' />"+

                            "<route hosts='Area*.H*' destination='*' netmask='0.0.0.0' interface='eth0' />"+
                            "</config>");
                addStaticRoutes = false;
                addDefaultRoutes = false;
                assignDisjunctSubnetAddresses = false;
                @display("p=75,43");
        }
    connections:
        Area1.ethg[0] <--> ThruputMeteringChannel {  delay = 0.1us; datarate = 100 Mbps; thruputDisplayFormat = "#N"; } <--> Area2.ethg[0];
}

