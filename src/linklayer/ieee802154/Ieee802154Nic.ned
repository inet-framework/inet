//
// This NIC implements an IEEE 802.15.4 network interface card with an interface queue.
//
package inet.underTest.wpan.linklayer.ieee802154;

import inet.networklayer.queue.OutputQueue;
import inet.underTest.wpan.linklayer.ieee802154.mac.Ieee802154Mac;
import inet.underTest.wpan.linklayer.ieee802154.phyLayer.Ieee802154Phy;

module Ieee802154Nic
{
    parameters:
        // name of the interface queue type
        string ifqType; // name of the interface queue type
    gates:
        input uppergateIn; // to upper layers
        output uppergateOut; // from upper layers
        input radioIn; // to receive AirFrames
    submodules:
        ifq: <ifqType> like OutputQueue {
            parameters:
                @display("p=70,80;i=block/passiveq;q=IFQueue");
        }
        mac: Ieee802154Mac {
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
        phy.uppergateOut --> { @display("m=m,50,50,75,0"); } --> mac.lowergateIn;
        phy.uppergateIn <-- { @display("m=m,25,0,25,0"); } <-- mac.lowergateOut;
        mac.uppergateOut --> uppergateOut;
        mac.uppergateIn <-- ifq.out;
        ifq.in <-- uppergateIn;
}

