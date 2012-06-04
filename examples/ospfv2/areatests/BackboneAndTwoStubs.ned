

package inet.examples.ospfv2.areatests;

import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
import inet.nodes.ospfv2.OSPFRouter;
import inet.util.ThruputMeteringChannel;
import inet.world.scenario.ScenarioManager;


network OSPF_BackboneAndTwoStubsTest
{
    parameters:
        int numBBRouters = default(3);
        @display("bgb=561,300");
    types:
        channel C extends ThruputMeteringChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
            thruputDisplayFormat = "#N";
        }
    submodules:
        BBR[numBBRouters]: OSPFRouter;
        Stub1: TestArea {
            parameters:
                numHosts = 2;
                extGates = 1;
                @display("p=138,156;b=136,136");
        }
        Stub2: TestArea {
            parameters:
                numHosts = 2;
                extGates = 1;
                @display("p=409,156;b=136,136");
        }
        configurator: IPv4NetworkConfigurator {
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
                @display("p=75,43");
        }
        scenarioManager: ScenarioManager {
            @display("p=187,43");
        }
    connections:
        BBR[0].ethg++  <--> C <--> Stub1.ethg[0];
        BBR[numBBRouters-1].ethg++  <--> C <--> Stub2.ethg[0];
        for i=1..numBBRouters-1 {
            BBR[i].ethg++ <--> C <--> BBR[i-1].ethg++;
        }
        //BBR[0].ethg++ <--> C <--> BBR[numBBRouters-1].ethg++;
}
