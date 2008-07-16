
package org.omnetpp.inet.examples.Quagga.SimpleTest;

import ned.DatarateChannel;
import org.omnetpp.inet.NetworkInterfaces.Ethernet.EtherHub;
import org.omnetpp.inet.Nodes.INET.QuaggaRouter;
import org.omnetpp.inet.Nodes.INET.StandardHost;
import org.omnetpp.inet.World.ChannelInstaller;


network SimpleTest
{
    parameters:
        @display("p=10,10;b=712,152");
    types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
        }
    submodules:
        channelInstaller: ChannelInstaller {
            parameters:
                channelClass = "ThruputMeteringChannel";
                channelAttrs = "format=#N";
                @display("p=130,40");
        }
        H1: StandardHost {
            parameters:
                @display("p=56,92;i=device/laptop");
            gates:
                ethg[1];
        }
        N1: EtherHub {
            parameters:
                @display("p=184,92");
            gates:
                ethg[2];
        }
        R1: QuaggaRouter {
            parameters:
                @display("p=296,92");
            gates:
                ethg[1];
        }
        R2: QuaggaRouter {
            parameters:
                @display("p=416,92");
            gates:
                ethg[1];
        }
        N2: EtherHub {
            parameters:
                @display("p=532,92");
            gates:
                ethg[2];
        }
        H2: StandardHost {
            parameters:
                @display("p=660,92;i=device/laptop");
            gates:
                ethg[1];
        }
    connections:
        H1.ethg[0] <--> C <--> N1.pppg[0];
        N1.pppg[1] <--> C <--> R1.ethg[0];

        R1.pppg++ <--> {  datarate = 10Mbps; delay = 0.1us; } <--> R2.pppg++;

        R2.ethg[0] <--> C <--> N2.pppg[0];
        N2.pppg[1] <--> C <--> H2.ethg[0];
}

