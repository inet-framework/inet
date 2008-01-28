

package org.omnetpp.inet.examples.RTP.Unicast;

import ned.cBasicChannel;
import org.omnetpp.inet.Transport.RTP.RTPHost;


network RTPNetwork
{
    parameters:
        bool debug = default(false);
    types:
        channel ErrorChannel extends cBasicChannel
        {
            //delay normal (0.004, 0.0041)
            error = 0.0;
        }
    submodules:
        sender: RTPHost {
            parameters:
                debug = debug;
                profileName = "RTPAVProfile";
                destinationAddress = "10.0.0.2";
                portNumber = 5004;
                bandwidth = 8000;
                fileName = "../Data/moving.mpg.gdf";
                payloadType = 32;
                numOfProcessors = 1;
                routingFile = "server1.irt";
        }
        receiver: RTPHost {
            parameters:
                debug = debug;
                profileName = "RTPAVProfile";
                destinationAddress = "10.0.0.1";
                portNumber = 5004;
                bandwidth = 8000;
                fileName = "";
                payloadType = 0;
                numOfProcessors = 1;
                routingFile = "client2.irt";

        }
    connections:
        sender.pppg[0] <--> ErrorChannel <--> receiver.pppg[0];
}

