


package inet.examples.ospfv2.fulltest;

import inet.common.misc.ThruputMeteringChannel;
import inet.linklayer.ethernet.EtherHub;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import inet.node.ospfv2.OSPFRouter;


channel C extends ThruputMeteringChannel
{
    delay = 0.1us;
    datarate = 100Mbps;
    thruputDisplayFormat = "#N";
}

module OspfLan
{
    parameters:
        int h; // number of hosts on the hub
        @display("i=old/cloud");
    gates:
        inout ethg[];
    submodules:
        hub: EtherHub {
            @display("is=s");
        }
        host[h]: StandardHost {
            @display("is=s");
        }
    connections:
        for i=0..sizeof(ethg)-1 {
            hub.ethg++ <--> ethg[i];
        }
        for i=0..h-1 {
            hub.ethg++ <--> C <--> host[i].ethg++;
        }
}

module OSPF_Area1
{
    parameters:
        @display("p=10,42;b=432,504");
    gates:
        inout ethg[3];
    submodules:
        RT1: OSPFRouter {
            parameters:
                @display("p=232,136");
            gates:
                ethg[2];
        }
        RT2: OSPFRouter {
            parameters:
                @display("p=236,348");
            gates:
                ethg[2];
        }
        N1: OspfLan {
            parameters:
                h = 2;
                @display("p=140,136");
        }
        N2: OspfLan {
            parameters:
                h = 2;
                @display("p=140,348");
        }
        N3: OspfLan {
            parameters:
                h = 1;
                @display("p=360,252");
        }
        N4: OspfLan {
            parameters:
                h = 2;
                @display("p=360,453");
        }
    connections:
        N1.ethg++ <--> C <--> RT1.ethg[0];
        RT1.ethg[1] <--> C <--> N3.ethg++;
        N2.ethg++ <--> C <--> RT2.ethg[0];
        RT2.ethg[1] <--> C <--> N3.ethg++;

        // towards other areas
        N3.ethg++ <--> { @display("m=n"); } <--> ethg[0];
        N3.ethg++ <--> { @display("m=e"); } <--> ethg[1];
        N4.ethg++ <--> { @display("m=e"); } <--> ethg[2];
}

module OSPF_Area2
{
    parameters:
        @display("p=10,10;b=424,400");
    gates:
        inout ethg[4];
    submodules:
        RT8: OSPFRouter {
            parameters:
                @display("p=304,196");
            gates:
                ethg[2];
        }
        N6: OspfLan {
            parameters:
                h = 1;
                @display("p=304,100");
        }
        N7: OspfLan {
            parameters:
                h = 2;
                @display("p=304,300");
        }
        N8: OspfLan {
            parameters:
                h = 1;
                @display("p=104,100");
        }
    connections:
        N6.ethg++ <--> C <--> RT8.ethg[0];
        RT8.ethg[1] <--> C <--> N7.ethg++;

        // towards other areas
        N8.ethg++ <--> ethg[0];
        N8.ethg++ <--> { @display("m=n"); } <--> ethg[1];
        N6.ethg++ <--> { @display("m=e"); } <--> ethg[2];
        N6.ethg++ <--> ethg[3];
}

module OSPF_Area3
{
    parameters:
        @display("p=10,10;b=264,528");
    gates:
        inout ethg[1];
    submodules:
        RT9: OSPFRouter {
            parameters:
                @display("p=148,200");
            gates:
                ethg[2];
        }
        RT12: OSPFRouter {
            parameters:
                @display("p=148,360");
            gates:
                ethg[3];
        }
        N9: OspfLan {
            parameters:
                h = 1;
                @display("p=148,284");
        }
        N10: OspfLan {
            parameters:
                h = 2;
                @display("p=148,452");
        }
        N11: OspfLan {
            parameters:
                h = 2;
                @display("p=144,112");
        }
        H1: StandardHost {
            parameters:
                @display("p=64,360;i=device/pc_s");
        }
    connections:
        RT9.ethg[1] <--> C <--> N9.ethg++;
        RT9.ethg[0] <--> C <--> N11.ethg++;
        RT12.ethg[1] <--> C <--> H1.ethg++;
        RT12.ethg[0] <--> C <--> N9.ethg++;
        RT12.ethg[2] <--> C <--> N10.ethg++;

        // towards other areas
        ethg[0] <--> { @display("m=e"); } <--> N9.ethg++;
}

network OSPF_RFC2328_Fig6
{
    parameters:
        @display("p=10,10;b=704,560");
    submodules:
        Area1: OSPF_Area1 {
            parameters:
                @display("p=120,172;b=176,168");
        }
        Area2: OSPF_Area2 {
            parameters:
                @display("p=448,468;b=176,168");
        }
        Area3: OSPF_Area3 {
            parameters:
                @display("p=128,468;b=176,168");
        }
        RT3: OSPFRouter {
            parameters:
                @display("p=276,208");
            gates:
                ethg[3];
        }
        RT4: OSPFRouter {
            parameters:
                @display("p=276,144");
            gates:
                ethg[2];
        }
        RT5: OSPFRouter {
            parameters:
                @display("p=496,144");
            gates:
                ethg[6];
        }
        RT6: OSPFRouter {
            parameters:
                @display("p=424,208");
            gates:
                ethg[3];
        }
        RT7: OSPFRouter {
            parameters:
                @display("p=496,328");
            gates:
                ethg[4];
        }
        RT10: OSPFRouter {
            parameters:
                @display("p=424,328");
            gates:
                ethg[3];
        }
        RT11: OSPFRouter {
            parameters:
                @display("p=284,468");
            gates:
                ethg[2];
        }
        N12: OspfLan {
            parameters:
                h = 1;
                @display("p=620,277");
        }
        N13: OspfLan {
            parameters:
                h = 1;
                @display("p=620,97");
        }
        N14: OspfLan {
            parameters:
                h = 1;
                @display("p=496,42");
        }
        N15: OspfLan {
            parameters:
                h = 1;
                @display("p=608,356");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                config = xml("<config>"+"\n"+
                            "<interface among='Area1.N1.* Area1.RT1' address='10.1.11.x' netmask='255.255.255.0' />"+"\n"+
                            "<interface among='Area1.N2.* Area1.RT2' address='10.1.12.x' netmask='255.255.255.0' />"+"\n"+
                            "<interface among='Area1.N3.* Area1.RT1 Area1.RT2 RT3 RT4' address='10.1.13.x' netmask='255.255.255.0' metric='1' />"+"\n"+
                            "<interface among='Area1.N4.* RT3' address='10.1.14.x' netmask='255.255.255.0' metric='1' />"+"\n"+
                            "<interface among='Area1.N2.* Area1.RT2' address='10.1.15.x' netmask='255.255.255.0' />"+"\n"+

                            "<interface among='Area2.N8.* RT10 RT11' address='10.2.21.x' netmask='255.255.255.0' />"+"\n"+
                            "<interface hosts='Area2.N6.*' address='10.2.22.x' netmask='255.255.255.0' />"+"\n"+
                            "<interface hosts='RT7 RT10 Area2.RT8' towards='Area2.N6.*' address='10.2.22.x' netmask='255.255.255.0' />"+"\n"+
                            "<interface among='Area2.N7.* Area2.RT8' address='10.2.23.x' netmask='255.255.255.0' />"+"\n"+

                            "<interface among='Area3.N10.* Area3.RT12' address='10.3.31.x' netmask='255.255.255.0' />"+"\n"+
                            "<interface among='Area3.N11.* Area3.RT9' address='10.3.32.x' netmask='255.255.255.0' />"+"\n"+
                            "<interface among='Area3.H1 Area3.RT12' address='10.3.33.x' netmask='255.255.255.0' />"+"\n"+
                            "<interface among='Area3.N9 Area3.N9.* Area3.RT* RT11' address='10.3.34.x' netmask='255.255.255.0' />"+"\n"+

                            "<interface hosts='RT5 RT7' towards='N12.*' address='10.99.12.x' netmask='255.255.255.0' />"+"\n"+
                            "<interface hosts='N12.*' address='10.99.12.x' netmask='255.255.255.0' />"+"\n"+
                            "<interface among='RT5 N13.*' address='10.99.13.x' netmask='255.255.255.0' />"+"\n"+
                            "<interface among='RT5 N14.*' address='10.99.14.x' netmask='255.255.255.0' />"+"\n"+
                            "<interface among='RT3 RT4 RT5 RT6 RT7 RT10' address='10.0.104.x' netmask='255.255.255.x' />"+"\n"+
                            "<interface among='RT7 N15.*' address='10.99.15.x' netmask='255.255.255.0' />"+"\n"+

                            "<route hosts='Area*.H* Area*.N*.host[*] N*.host[*]' destination='*' netmask='0.0.0.0' interface='eth0' />"+"\n"+
                            "</config>"+"\n");
                addStaticRoutes = false;
                addDefaultRoutes = false;
                @display("p=120,43");
        }
    connections:
        RT3.ethg[0] <--> C <--> Area1.ethg[1];
        RT3.ethg[1] <--> C <--> Area1.ethg[2];
        RT3.ethg[2] <--> C <--> RT6.ethg[0];
        RT4.ethg[0] <--> C <--> Area1.ethg[0];
        RT4.ethg[1] <--> C <--> RT5.ethg[1];
        RT5.ethg[5] <--> C <--> N14.ethg++;
        RT5.ethg[3] <--> C <--> RT7.ethg[0];
        RT5.ethg[2] <--> C <--> RT6.ethg[2];
        RT5.ethg[4] <--> C <--> N12.ethg++;
        RT5.ethg[0] <--> C <--> N13.ethg++;
        RT6.ethg[1] <--> C <--> RT10.ethg[0];
        RT7.ethg[1] <--> C <--> Area2.ethg[3];
        RT7.ethg[3] <--> C <--> N12.ethg++;
        RT7.ethg[2] <--> C <--> N15.ethg++;
        RT10.ethg[1] <--> C <--> Area2.ethg[1];
        RT10.ethg[2] <--> C <--> Area2.ethg[2];
        RT11.ethg[1] <--> C <--> Area2.ethg[0];
        RT11.ethg[0] <--> C <--> Area3.ethg[0];
}

