//
// This NIC implements an IEEE 802.15.4 network interface card with an interface queue.
//
package inet.linklayer.ieee802154;

import inet.networklayer.queue.IOutputQueue;
import inet.linklayer.ieee802154.phyLayer.Ieee802154Phy;
import inet.linklayer.ieee802154.mac.CSMA802154;


module Ieee802154csmaNic
{
    parameters:
        // name of the interface queue type
        string ifqType; // name of the interface queue type
    gates:
        input upperLayerIn; // to upper layers
        output upperLayerOut; // from upper layers
        input radioIn; // to receive AirFrames
    submodules:
        ifq: <ifqType> like IOutputQueue {
            parameters:
                @display("p=70,80;i=block/passiveq;q=IFQueue");
        }
        mac: CSMA802154 {
            parameters:
                queueModule = "ifq";
                @display("p=70,180;i=block/layer");
        }
        phy: Ieee802154Phy {
            parameters:
                @display("p=70,280;i=block/wrxtx");
        }
    connections:
        radioIn --> phy.radioIn;
        phy.upperLayerOut --> { @display("m=m,50,50,75,0"); } --> mac.lowerLayerIn;
        phy.upperLayerIn <-- { @display("m=m,25,0,25,0"); } <-- mac.lowerLayerOut;
        mac.upperLayerOut --> upperLayerOut;
        mac.upperLayerIn <-- ifq.out;
        ifq.in <-- upperLayerIn;
}

