

package inet.examples.ospfv2.areas;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import ned.TransmissionChannel;


network OSPF_AreaTest
{
    parameters:
        *.eth[*].bitrate = 100 Mbps;
    submodules:
        Area1: TwoNetsArea {
            parameters:
                @display("p=250,100;b=100,100");
            gates:
                ethg[1];
        }
        Area2: OneNetArea {
            parameters:
                @display("p=500,100;b=100,100");
            gates:
                ethg[1];
        }
        configurator: Ipv4NetworkConfigurator {
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
                @display("p=100,100;is=s");
        }
    connections:
        Area1.ethg[0] <--> TransmissionChannel {  delay = 0.1us; } <--> Area2.ethg[0];
}

