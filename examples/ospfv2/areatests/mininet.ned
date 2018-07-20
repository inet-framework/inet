

package inet.examples.ospfv2.areatests;

import inet.common.misc.ThruputMeteringChannel;
import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import inet.node.ospfv2.OspfRouter;


network OSPF_mininet
{
    parameters:
        int numBBr = 2;
        int numSBr = 2;
        int numHst = 2;
    types:
        channel C extends ThruputMeteringChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
            thruputDisplayFormat = "#N";
        }
    submodules:
        BBR[numBBr]: OspfRouter {
            @display("p=700,150,col,100");
        }
        BBRtoS1: OspfRouter {
            @display("p=850,150");
        }
        S1R[numSBr]: OspfRouter {
            @display("p=1000,150,col,100");
        }
        S1H[numHst]: StandardHost {
            @display("p=1150,150,col,100");
        }
        BBRtoS2: OspfRouter {
            @display("p=550,150");
        }
        S2R[numSBr]: OspfRouter {
            @display("p=400,150,col,100");
        }
        S2H[numHst]: StandardHost {
            @display("p=250,150,col,100");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                config = xml("<config>"+
                            "<interface hosts='BBR[*]' address='192.168.100.x' netmask='255.255.255.x' />"+
                            "<interface hosts='BBRtoS*' towards='BBR[*]' address='192.168.100.x' netmask='255.255.255.x' />"+

                            "<interface hosts='BBRtoS1' towards='S1R[*]' address='192.168.201.x' netmask='255.255.255.x' />"+
                            "<interface hosts='S1H[*] S1R[*]' address='192.168.201.x' netmask='255.255.255.x' />"+

                            "<interface hosts='BBRtoS2' towards='S2R[*]' address='192.168.202.x' netmask='255.255.255.x' />"+
                            "<interface hosts='S2H[*] S2R[*]' address='192.168.202.x' netmask='255.255.255.x' />"+

                            "<route hosts='S*H[*]' destination='*' netmask='0.0.0.0' interface='eth0' />"+
                            "</config>");
                addStaticRoutes = false;
                addDefaultRoutes = false;
                @display("p=100,100;is=s");
        }
        scenarioManager: ScenarioManager {
            @display("p=100,200;is=s");
        }
    connections:
        BBRtoS1.ethg++ <--> C <--> BBR[0].ethg++;
        BBRtoS1.ethg++ <--> C <--> S1R[0].ethg++;
        BBRtoS2.ethg++ <--> C <--> S2R[0].ethg++;
        BBRtoS2.ethg++ <--> C <--> BBR[numBBr-1].ethg++;
        for i=1..numBBr-1 {
            BBR[i-1].ethg++ <--> C <--> BBR[i].ethg++;
        }
        for i=1..numSBr-1 {
            S1R[i-1].ethg++ <--> C <--> S1R[i].ethg++;
            S2R[i-1].ethg++ <--> C <--> S2R[i].ethg++;
        }
        for i=0..numHst-1 {
            S1R[numSBr-1].ethg++ <--> C <--> S1H[i].ethg++;
            S2R[numSBr-1].ethg++ <--> C <--> S2H[i].ethg++;
        }
}

