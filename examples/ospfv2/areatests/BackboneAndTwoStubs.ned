

package inet.examples.ospfv2.areatests;

import inet.common.misc.ThruputMeteringChannel;
import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ospfv2.OspfRouter;


network OSPF_BackboneAndTwoStubsTest
{
    parameters:
        int numBBRouters = default(3);
    types:
        channel C extends ThruputMeteringChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
            thruputDisplayFormat = "#N";
        }
    submodules:
        BBR[numBBRouters]: OspfRouter {
            @display("p=450,150,col,90");
        }
        Stub1: TestArea {
            parameters:
                numHosts = 2;
                extGates = 1;
                @display("p=300,150;b=100,100");
        }
        Stub2: TestArea {
            parameters:
                numHosts = 2;
                extGates = 1;
                @display("p=600,150;b=100,100");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                config = xml("<config>"+
                            "<interface among='BBR[*]' address='192.168.100.x' netmask='255.255.255.x' />"+
                            "<interface hosts='Stub1.*' address='192.168.91.x' netmask='255.255.255.x' />"+
                            "<interface hosts='BBR[*]' towards='Stub1.*' address='192.168.91.x' netmask='255.255.255.x' />"+
                            "<interface hosts='Stub2.*' address='192.168.92.x' netmask='255.255.255.x' />"+
                            "<interface hosts='BBR[*]' towards='Stub2.*' address='192.168.92.x' netmask='255.255.255.x' />"+
                            "<multicast-group hosts='**.R[*] **.B[*] BBR[*]' address='224.0.0.5 224.0.0.6' />"+
                            "<route hosts='**.H[*]' destination='*' netmask='0.0.0.0' interface='eth0' />"+
                            "<route hosts='**.B[*] BBR[*]' destination='224.0.0.0' netmask='240.0.0.0' interface='eth0' />"+
                            "<route hosts='**.R[*] **.B[*] BBR[*]' destination='224.0.0.0' netmask='240.0.0.0' interface='eth1' />"+
                            "<route hosts='**.R[*] **.B[*]' destination='224.0.0.0' netmask='240.0.0.0' interface='eth2' />"+
                            "</config>");
                addStaticRoutes = false;
                addDefaultRoutes = false;
                @display("p=100,100;is=s");
        }
        scenarioManager: ScenarioManager {
            @display("p=100,200;is=s");
        }
    connections:
        BBR[0].ethg++ <--> C <--> Stub1.ethg[0];
        BBR[numBBRouters-1].ethg++ <--> C <--> Stub2.ethg[0];
        for i=1..numBBRouters-1 {
            BBR[i].ethg++ <--> C <--> BBR[i-1].ethg++;
        }
        //BBR[0].ethg++ <--> C <--> BBR[numBBRouters-1].ethg++;
}
