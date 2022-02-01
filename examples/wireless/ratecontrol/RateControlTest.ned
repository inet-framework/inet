//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.wireless.ratecontrol;

import inet.applications.ethernet.EtherAppClient;
import inet.applications.ethernet.EtherAppServer;
import inet.environment.common.PhysicalEnvironment;
import inet.linklayer.ieee80211.Ieee80211Interface;
import inet.mobility.single.RectangleMobility;
import inet.networklayer.common.InterfaceTable;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;


module SinkClient
{
    parameters:
        @networkNode();
        @labels(node);
        @display("i=device/cellphone");
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
        }
        mobility: RectangleMobility {
            parameters:
                @display("p=100,200;is=s");
        }
    connections allowunconnected:
        radioIn --> { @display("m=s"); } --> wlan.radioIn;
        wlan.upperLayerOut --> sink.in;
        sink.out --> wlan.upperLayerIn;
}

module Client
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
        }
        mobility: RectangleMobility {
            parameters:
                @display("p=100,200;is=s");
        }
    connections allowunconnected:
        wlan.radioIn <-- radioIn;
        cli.out --> wlan.upperLayerIn;
}

network RateControlTest
{
    parameters:
        int numCli;
        @display("bgb=1300,460");
    submodules:
        cliHost[numCli]: Client {
            parameters:
                @display("r=,,#707070");
        }
        sinkClient: SinkClient {
            parameters:
                @display("p=200,200;r=,,#707070");
        }
        physicalEnvironment: PhysicalEnvironment {
            parameters:
                @display("p=100,100;is=s");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,200;is=s");
        }
}

