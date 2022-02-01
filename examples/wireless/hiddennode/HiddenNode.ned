//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.wireless.hiddennode;

import inet.applications.ethernet.EtherAppClient;
import inet.applications.ethernet.EtherAppServer;
import inet.linklayer.ieee80211.Ieee80211Interface;
import inet.mobility.static.StationaryMobility;
import inet.networklayer.common.InterfaceTable;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;


module Node
{
    parameters:
        @display("i=device/wifilaptop");
        @networkNode();
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        wlan.radio.antenna.mobilityModule = default("^.^.^.mobility");
    gates:
        input radioIn @directIn;

    submodules:
        interfaceTable: InterfaceTable {
            @display("p=100,100;is=s");
        }
        cli: EtherAppClient {
            parameters:
                interface = "wlan";
                @display("p=300,100");
        }
        wlan: Ieee80211Interface {
            parameters:
                @display("p=250,200;q=queue");
                mgmt.typename = "Ieee80211MgmtAdhoc";
                agent.typename = "";
                mac.typename = "Ieee80211Mac";
        }
        mobility: StationaryMobility {
            parameters:
                @display("p=100,200;is=s");
        }
    connections allowunconnected:
        wlan.radioIn <-- radioIn;
        cli.out --> wlan.upperLayerIn;
}

module SinkNode
{
    parameters:
        @display("i=device/wifilaptop");
        @networkNode();
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        wlan.radio.antenna.mobilityModule = default("^.^.^.mobility");
    gates:
        input radioIn @directIn;

    submodules:
        interfaceTable: InterfaceTable {
            @display("p=100,100;is=s");
        }
        sink: EtherAppServer {
            parameters:
                @display("p=300,100");
        }
        wlan: Ieee80211Interface {
            parameters:
                @display("p=250,200;q=queue");
                mgmt.typename = "Ieee80211MgmtAdhoc";
                agent.typename = "";
                mac.typename = "Ieee80211Mac";
        }
        mobility: StationaryMobility {
            parameters:
                @display("p=100,200;is=s");
        }
    connections allowunconnected:
        wlan.radioIn <-- radioIn;
        sink.in <-- wlan.upperLayerOut;
        sink.out --> wlan.upperLayerIn;
}


network HiddenNode
{
    parameters:
    submodules:
        A: Node {
            parameters:
                @display("r=,,#707070;p=159,143");
        }
        C: Node {
            parameters:
                @display("r=,,#707070;p=473,143");
        }
        B: SinkNode {
            parameters:
                @display("p=316,143;r=,,#707070");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,100;is=s");
        }
}

