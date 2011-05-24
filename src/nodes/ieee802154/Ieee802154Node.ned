package inet.underTest.wpan.nodes;

import inet.underTest.wpan.linklayer.ieee802154.Ieee802154Nic;
import inet.underTest.wpan.networklayer.Ieee802154StarRouting.Ieee802154StarRouting;

//import inet.underTest.wpan.battery.Ieee802154Battery;
import inet.battery.InetSimpleBattery;
import inet.underTest.wpan.applications.Ieee802154TestApp.Ieee802154TestApp;
import inet.mobility.BasicMobility;
import inet.base.NotificationBoard;
import inet.underTest.wpan.Isabel.Base.Display;

//
// This module implements simple ZigBee nodes with IEEE 802.15.4 MAC and PHY layer, plus a simple test app layer
//

module Ieee802154Node
{
    parameters:
        @node();
        string mobilityType;
        volatile double numHosts;
    gates:
        input radioIn;
    submodules:
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=65,70;i=block/control");

        }
        mobility: <mobilityType> like BasicMobility {
            parameters:
                @display("p=65,150;i=block/cogwheel");

        }
        disp: Display {
            parameters:
                @display("p=65,230;i=block/app2");

        }
        // battery module
        battery: InetSimpleBattery{
            parameters:
                @display("p=65,310;i=block/plug");

        }
        // application-layer
        app: Ieee802154TestApp {
            parameters:
                @display("p=250,100;i=block/source");

        }
        // network layer
        net: Ieee802154StarRouting {
            parameters:
                @display("p=250,200;i=block/network2");

        }
        // network interface
        nic: Ieee802154Nic {
            parameters:
                @display("p=250,300;q=queue;i=block/ifcard");

        }
    connections allowunconnected:
        radioIn --> nic.radioIn;
        nic.uppergateOut --> net.lowergateIn;
        nic.uppergateIn <-- net.lowergateOut;
        net.uppergateOut --> app.lowergateIn;
        net.uppergateIn <-- app.lowergateOut;
}
