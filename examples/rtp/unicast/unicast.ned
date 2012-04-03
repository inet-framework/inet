package inet.examples.rtp.unicast;

import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
import inet.networklayer.extras.Dummy;
import inet.transport.rtp.RTPHost;


network RTPNetwork
{
    types:
        channel ethernet extends ned.DatarateChannel
        {
            parameters:
                delay = normal(0.00015s, 0.00005s);
                datarate = 10Mbps;
                ber = 0.00001;
        }

    submodules:
        sender: RTPHost {
            parameters:
                profileName = "inet.transport.rtp.RTPAVProfile";
                destinationAddress = "receiver";
                portNumber = 5004;
                bandwidth = 8000;
        }
        receiver: RTPHost {
            parameters:
                profileName = "inet.transport.rtp.RTPAVProfile";
                destinationAddress = "sender";
                portNumber = 5004;
                bandwidth = 8000;
        }
        configurator: IPv4NetworkConfigurator;
    connections:
        sender.pppg++ <--> ethernet <--> receiver.pppg++;
}
