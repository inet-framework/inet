package inet.tests.basePhyLayer;

import inet.physicallayer.radio.detailed.radiochannel.DetailedRadioChannel;
import inet.tests.ExtTestNetwork;

// Test network for PHY implementation tests.
network tbpSim extends ExtTestNetwork
{
    parameters:
        int run; // total number of hosts in the network
        bool showPassed;
    submodules:
        radioChannel: DetailedRadioChannel {
            parameters:
                @display("p=201,0;i=abstract/multicast;is=s");
        }
        testManager: BasePhyTests {
            @display("p=54,97");
        }
        node[numHosts]: TestHost {
            parameters:
                numHosts = numHosts;
                @display("p=149,96;i=laptop");

        }
    connections allowunconnected:
}

