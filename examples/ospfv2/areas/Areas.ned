

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
                            "<interface hosts='Area1.H1 Area1.R1' towards='Area1.H1 Area1.R1' address='192.168.1.x' netmask='255.255.255.0' />"+
                            "<interface hosts='Area1.H2 Area1.R2' towards='Area1.H2 Area1.R2' address='192.168.2.x' netmask='255.255.255.0' />"+
                            "<interface hosts='Area2.H3 Area2.R5' towards='Area2.H3 Area2.R5' address='192.168.3.x' netmask='255.255.255.0' />"+
                            "<interface hosts='Area1.R1' towards='Area1.R2' address='192.168.11.2' netmask='255.255.255.252' />"+
                            "<interface hosts='Area1.R1' towards='Area1.R3' address='192.168.11.1' netmask='255.255.255.252' />"+
                            "<interface hosts='Area1.R2' towards='Area1.R1' address='192.168.12.1' netmask='255.255.255.252' />"+
                            "<interface hosts='Area1.R2' towards='Area1.R3' address='192.168.12.2' netmask='255.255.255.252' />"+
                            "<interface hosts='Area1.R3' towards='Area1.R1' address='192.168.13.3' netmask='255.255.255.248' />"+
                            "<interface hosts='Area1.R3' towards='Area1.R2' address='192.168.13.2' netmask='255.255.255.248' />"+
                            "<interface hosts='Area1.R3' towards='Area2.R4' address='192.168.13.1' netmask='255.255.255.248' />"+
                            "<interface hosts='Area2.R4' towards='Area1.R3' address='192.168.14.2' netmask='255.255.255.252' />"+
                            "<interface hosts='Area2.R4' towards='Area2.R5' address='192.168.14.1' netmask='255.255.255.252' />"+
                            "<interface hosts='Area2.R5' towards='Area2.R4' address='192.168.15.2' netmask='255.255.255.252' />"+
                            "<multicast-group hosts='Area*.R*' address='224.0.0.5 224.0.0.6' />"+
                            "<route hosts='Area*.H*' destination='*' netmask='0.0.0.0' interface='eth0' />"+
                            "<route hosts='Area*.R*' destination='224.0.0.0' netmask='240.0.0.0' interface='eth0' />"+
                            "<route hosts='Area*.R*' destination='224.0.0.0' netmask='240.0.0.0' interface='eth1' />"+
                            "<route hosts='Area1.R*' destination='224.0.0.0' netmask='240.0.0.0' interface='eth2' />"+
                            "</config>");
                addStaticRoutes = false;
                addDefaultRoutes = false;
                assignDisjunctSubnetAddresses = false;
                @display("p=75,43");
        }
    connections:
        Area1.ethg[0] <--> ThruputMeteringChannel {  delay = 0.1us; datarate = 100 Mbps; thruputDisplayFormat = "#N"; } <--> Area2.ethg[0];
}

