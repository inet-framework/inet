package inet.examples.inet.broadcast;

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;


network UDPBroadcastNetwork
{
    parameters:
        int numTargets;
    types:
        channel C extends DatarateChannel
        {
            datarate = 10Mbps;
            delay = 0.1us;
        }
    submodules:
        target[numTargets]: StandardHost {
            parameters:
                @display("i=device/pc3;p=514,115");
        }
        client: StandardHost {
            parameters:
                @display("p=62,115;i=device/pc2");
        }
        R1: Router {
            parameters:
                @display("p=180,115");
        }
        R2: Router {

            @display("p=279,115");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                config = xmldoc("config.xml");
                @display("p=62,31");
        }
        switch: EtherSwitch {
            @display("p=389,114");
        }
        nonTarget: StandardHost {
            @display("p=389,31");
        }
    connections:
        client.pppg++ <--> C <--> R1.pppg++;
        R1.pppg++ <--> C <--> R2.pppg++;
        R2.pppg++ <--> C <--> nonTarget.pppg++;
        R2.ethg++ <--> C <--> switch.ethg++;
        for i=0..numTargets-1 {
            switch.ethg++ <--> C <--> target[i].ethg++;
        }
}
