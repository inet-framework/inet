
package inet.examples.ospfv2.backbone;

import inet.common.misc.ThruputMeteringChannel;
import inet.linklayer.ethernet.EtherHub;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import inet.node.ospfv2.OspfRouter;


network Backbone
{
    parameters:
        @display("p=10,10;b=736,568");
    types:
        channel C extends ThruputMeteringChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
            thruputDisplayFormat = "#N";
        }
    submodules:
        R1: OspfRouter {
            parameters:
                @display("p=500,460");
            gates:
                ethg[3];
        }
        N1: EtherHub {
            parameters:
                @display("p=500,370");
            gates:
                ethg[2];
        }
        N2: EtherHub {
            parameters:
                @display("p=617,520");
            gates:
                ethg[3];
        }
        R2: OspfRouter {
            parameters:
                @display("p=500,280");
            gates:
                ethg[2];
        }
        R4: OspfRouter {
            parameters:
                @display("p=266,580");
            gates:
                ethg[2];
        }
        R3: OspfRouter {
            parameters:
                @display("p=734,580");
            gates:
                ethg[2];
        }
        N4: EtherHub {
            parameters:
                @display("p=500,190");
            gates:
                ethg[4];
        }
        N5: EtherHub {
            parameters:
                @display("p=850,640");
            gates:
                ethg[3];
        }
        R5: OspfRouter {
            parameters:
                @display("p=675,415");
            gates:
                ethg[2];
        }
        R7: OspfRouter {
            parameters:
                @display("p=325,415");
            gates:
                ethg[2];
        }
        R6: OspfRouter {
            parameters:
                @display("p=500,640");
            gates:
                ethg[2];
        }
        N6: EtherHub {
            parameters:
                @display("p=150,640");
            gates:
                ethg[3];
        }
        N3: EtherHub {
            parameters:
                @display("p=383,520");
            gates:
                ethg[2];
        }
        H1: StandardHost {
            parameters:
                @display("p=500,550;i=device/laptop");
            gates:
                ethg[1];
        }
        H2: StandardHost {
            parameters:
                @display("p=500,100;i=device/laptop");
            gates:
                ethg[1];
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                config = xml("<config>"+
                            "<interface hosts='R2' towards='N1 R1' address='192.168.1.1' netmask='255.255.255.0' metric='1' />"+
                            "<interface hosts='R1' towards='N1 R2' address='192.168.1.2' netmask='255.255.255.0' metric='1' />"+

                            "<interface hosts='R3' towards='N2 R1' address='192.168.2.1' netmask='255.255.255.0' metric='2' />"+
                            "<interface hosts='H1' address='192.168.2.2' netmask='255.255.255.0' metric='1' />"+
                            "<interface hosts='R1' towards='N2 R3' address='192.168.2.3' netmask='255.255.255.0' metric='2' />"+

                            "<interface hosts='R1' towards='N3 R4' address='192.168.3.1' netmask='255.255.255.0' metric='3' />"+
                            "<interface hosts='R4' towards='N3 R1' address='192.168.3.2' netmask='255.255.255.0' metric='3' />"+

                            "<interface hosts='H2' address='192.168.4.1' netmask='255.255.255.0' metric='1' />"+
                            "<interface hosts='R5' towards='N4 H2' address='192.168.4.2' netmask='255.255.255.0' metric='4' />"+
                            "<interface hosts='R2' towards='N4 H2' address='192.168.4.3' netmask='255.255.255.0' metric='4' />"+
                            "<interface hosts='R7' towards='N4 H2' address='192.168.4.4' netmask='255.255.255.0' metric='4' />"+

                            "<interface hosts='R6' towards='N5 R5' address='192.168.5.1' netmask='255.255.255.0' metric='5' />"+
                            "<interface hosts='R3' towards='N5 R5' address='192.168.5.2' netmask='255.255.255.0' metric='5' />"+
                            "<interface hosts='R5' towards='N5 R6' address='192.168.5.3' netmask='255.255.255.0' metric='5' />"+

                            "<interface hosts='R7' towards='N6 R6' address='192.168.6.1' netmask='255.255.255.0' metric='6' />"+
                            "<interface hosts='R4' towards='N6 R6' address='192.168.6.2' netmask='255.255.255.0' metric='6' />"+
                            "<interface hosts='R6' towards='N6 R7' address='192.168.6.3' netmask='255.255.255.0' metric='6' />"+

                            "<multicast-group hosts='R*' address='224.0.0.5 224.0.0.6' />"+

                            "<route hosts='H*' destination='*' netmask='0.0.0.0' interface='eth0' />"+

                            "<route hosts='R*' destination='224.0.0.0' netmask='240.0.0.0' interface='eth0' />"+
                            "<route hosts='R*' destination='224.0.0.0' netmask='240.0.0.0' interface='eth1' />"+
                            "<route hosts='R1' destination='224.0.0.0' netmask='240.0.0.0' interface='eth2' />"+
                            "</config>");
                addStaticRoutes = false;
                addDefaultRoutes = false;
                @display("p=100,100;is=s");
        }
    connections:
        R1.ethg[0] <--> C <--> N1.ethg[1];
        R1.ethg[1] <--> C <--> N2.ethg[2];
        N1.ethg[0] <--> C <--> R2.ethg[1];
        R2.ethg[0] <--> C <--> N4.ethg[2];
        N4.ethg[1] <--> C <--> R5.ethg[1];
        R5.ethg[0] <--> C <--> N5.ethg[2];
        N2.ethg[0] <--> C <--> R3.ethg[1];
        R3.ethg[0] <--> C <--> N5.ethg[1];
        N5.ethg[0] <--> C <--> R6.ethg[0];
        N6.ethg[2] <--> C <--> R6.ethg[1];
        R1.ethg[2] <--> C <--> N3.ethg[0];
        N3.ethg[1] <--> C <--> R4.ethg[0];
        R4.ethg[1] <--> C <--> N6.ethg[1];
        N4.ethg[3] <--> C <--> R7.ethg[0];
        R7.ethg[1] <--> C <--> N6.ethg[0];
        N4.ethg[0] <--> C <--> H2.ethg[0];
        N2.ethg[1] <--> C <--> H1.ethg[0];
}

