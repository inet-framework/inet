package inet.tests.basePhyLayer;

import inet.linklayer.WirelessNic;

module TestNic extends WirelessNic
{
    parameters:
        radioType = "inet.tests.basePhyLayer.TestPhyLayer";
        macType = "inet.tests.basePhyLayer.TestMacLayer";
}
