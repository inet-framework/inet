package inet.tests;

import inet.mobility.contract.IMobility;
import inet.networklayer.contract.INetworkLayer;
import inet.linklayer.IWirelessNic;
import inet.physicallayer.radio.contract.IRadioChannel;

// Basis definition of test networks.
network BaseTestNetwork
{
}

// Basis definition of test networks which are common
// network definition. 
network ExtTestNetwork
{
    parameters:
        int numHosts; // total number of hosts in the network
}

// Basis definition for all test nodes.
module TestNode
{
}

// Basis definition for all test objects.
simple TestObject
{
}

module TestWirelessHost
{
    parameters:
        string networkType = default("");
        string mobilityType = default("StationaryMobility");
        string nicType;
        @display("bgb=210,450,white;i=device/palm;b=40,40,rect");
        @node();

    gates:
        input radioIn; // gate for sendDirect

    submodules:
        mobility: <mobilityType> like IMobility {
            parameters:
                @display("p=140,30;i=block/cogwheel");
        }
        nic: <nicType> like IWirelessNic {
            parameters:
                @display("p=70,380;i=block/wrxtx,#008000");
        }
        // OSI network layer
        networkLayer: <networkType> like INetworkLayer if networkType != "" {
            parameters:
                @display("p=70,310;i=block/layer");
        }

    connections allowunconnected:
        nic.upperLayerOut --> networkLayer.ifIn++ if networkType != "";
        nic.upperLayerIn <-- networkLayer.ifOut++ if networkType != "";
        radioIn --> nic.radioIn;
}
