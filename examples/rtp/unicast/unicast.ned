package inet.examples.rtp.unicast;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.rtp.RtpHost;


network RTPNetwork
{
    types:
        channel pppline extends ned.DatarateChannel
        {
            parameters:
                delay = normal(0.00015s, 0.00005s);
                datarate = 10Mbps;
                ber = 0.00001;
        }

    submodules:
        sender: RtpHost {
            parameters:
                profileName = "inet.transportlayer.rtp.RtpAvProfile";
                destinationAddress = "receiver";
                portNumber = 5004;
                bandwidth = 8000;
                @display("p=250,100");
        }
        receiver: RtpHost {
            parameters:
                profileName = "inet.transportlayer.rtp.RtpAvProfile";
                destinationAddress = "sender";
                portNumber = 5004;
                bandwidth = 8000;
                @display("p=400,100");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100;is=s");
        }
    connections:
        sender.pppg++ <--> pppline <--> receiver.pppg++;
}
