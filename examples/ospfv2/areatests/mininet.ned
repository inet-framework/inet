

package inet.examples.ospfv2.areatests;

import inet.common.misc.ThruputMeteringChannel;
import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import inet.node.ospfv2.OSPFRouter;


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
        BBR[numBBr]: OSPFRouter;
        BBRtoS1: OSPFRouter;
        S1R[numSBr]: OSPFRouter;
        S1H[numHst]: StandardHost;
        BBRtoS2: OSPFRouter;
        S2R[numSBr]: OSPFRouter;
        S2H[numHst]: StandardHost;
        configurator: IPv4NetworkConfigurator {
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
                @display("p=75,43");
        }
        scenarioManager: ScenarioManager {
            @display("p=187,43");
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

