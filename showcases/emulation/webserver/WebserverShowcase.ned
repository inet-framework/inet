package inet.showcases.emulation.webserver;

import inet.node.ethernet.Eth1G;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.StandardHost;
import inet.visualizer.contract.IIntegratedVisualizer;

network WebserverShowcase
{
    parameters:
        int numClients;
        @display("bgb=600,400");
    submodules:
        visualizer: <default(firstAvailableOrEmpty("IntegratedMultiCanvasVisualizer"))> like IIntegratedVisualizer if typename != "" {
            parameters:
                @display("p=100,100;is=s");
        }
        client[numClients]: StandardHost {
            parameters:
                @display("r=,,#707070;p=300,100,r,100");
        }
        switch: EthernetSwitch {
            parameters:
                @display("p=300,200");
        }
        server: StandardHost {
            parameters:
                @display("r=,,#707070;p=300,300");
        }
    connections:
        for i=0..numClients-1 {
            client[i].ethg++ <--> Eth1G <--> switch.ethg++;
        }
        server.ethg++ <--> Eth1G <--> switch.ethg++;
}

