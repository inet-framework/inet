package inet.examples.inet.broadcast;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;
import ned.TransmissionChannel;


network UDPBroadcastNetwork
{
    parameters:
        int numTargets;
        *.eth[*].bitrate = 10Mbps;
    types:
        channel C extends DatarateChannel
        {
            datarate = 10Mbps;
            delay = 0.1us;
        }
        channel E extends TransmissionChannel {
            delay = 0.1us;
        }
    submodules:
        target[numTargets]: StandardHost {
            parameters:
                @display("i=device/pc3;p=850,100,col,100");
        }
        client: StandardHost {
            parameters:
                @display("p=250,200;i=device/pc2");
        }
        R1: Router {
            parameters:
                @display("p=400,200");
        }
        R2: Router {
            @display("p=550,200");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                config = xmldoc("config.xml");
                @display("p=100,100;is=s");
        }
        switch: EtherSwitch {
            @display("p=700,199");
        }
        nonTarget: StandardHost {
            @display("p=550,100");
        }
    connections:
        client.pppg++ <--> C <--> R1.pppg++;
        R1.pppg++ <--> C <--> R2.pppg++;
        R2.pppg++ <--> C <--> nonTarget.pppg++;
        R2.ethg++ <--> E <--> switch.ethg++;
        for i=0..numTargets-1 {
            switch.ethg++ <--> E <--> target[i].ethg++;
        }
}

