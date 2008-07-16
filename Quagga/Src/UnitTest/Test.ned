
package org.omnetpp.inet.Network.Quagga.UnitTest;

import ned.DatarateChannel;
import org.omnetpp.inet.World.ChannelInstaller;



network Test
{
    types:
        channel ethernetlineX extends DatarateChannel
        {
            parameters:
                delay = 10ms;
                datarate = 100000;
        }
    submodules:
        channelInstaller: ChannelInstaller {
            parameters:
                channelClass = "ThruputMeteringChannel";
                channelAttrs = "format=u #N";
                @display("p=284,50");
        }
        r1: TestRouter {
            parameters:
                @display("p=100,30");
            gates:
                pppg[2];
        }
        r2: TestRouter {
            parameters:
                @display("p=50,100");
            gates:
                pppg[4];
        }
        r3: TestRouter {
            parameters:
                @display("p=150,100");
            gates:
                pppg[4];
        }
        r4: TestRouter {
            parameters:
                @display("p=10,150");
            gates:
                pppg[2];
        }
        r5: TestRouter {
            parameters:
                @display("p=100,150");
            gates:
                pppg[3];
        }
    connections:
        r1.pppg[0] <--> ethernetlineX <--> r2.pppg[0];
        r1.pppg[1] <--> ethernetlineX <--> r3.pppg[0];
        r2.pppg[1] <--> ethernetlineX <--> r3.pppg[1];
        r2.pppg[2] <--> ethernetlineX <--> r4.pppg[0];
        r2.pppg[3] <--> ethernetlineX <--> r5.pppg[0];
        r3.pppg[2] <--> ethernetlineX <--> r5.pppg[1];
        r4.pppg[1] <--> ethernetlineX <--> r5.pppg[2];
}

