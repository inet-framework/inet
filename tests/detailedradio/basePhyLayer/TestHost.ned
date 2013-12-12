package inet.tests.basePhyLayer;

import inet.tests.TestWirelessHost;

module TestHost extends TestWirelessHost
{
    parameters:
        nicType = "inet.tests.basePhyLayer.TestNic";
        netwl.headerLength = 12bit;
        int numHosts; // total number of hosts in the network

    connections allowunconnected:
}

