package inet.nodes.ieee802154;

import inet.world.ieee802154.base.Display;
import inet.networklayer.ieee802154.Ieee802154StarRouting;
import inet.mobility.IMobility;
import inet.linklayer.ieee802154.Ieee802154Nic;
import inet.battery.models.InetSimpleBattery;
import inet.base.NotificationBoard;
import inet.applications.ieee802154app.Ieee802154TestApp;


module Ieee802154Node
{
    parameters:
        @node();
        string mobilityType;
        int numHosts;
    gates:
        input radioIn;
    submodules:
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=65,70;i=block/control");

        }
        mobility: <mobilityType> like IMobility {
            parameters:
                @display("p=65,150;i=block/cogwheel");

        }
        disp: Display {
            parameters:
                @display("p=65,230;i=block/app2");

        }
        // battery module
        battery: InetSimpleBattery {
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
