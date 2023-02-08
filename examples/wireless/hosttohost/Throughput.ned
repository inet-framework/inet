//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.wireless.hosttohost;

import inet.applications.ethernet.EtherAppClient;
import inet.applications.ethernet.EtherAppServer;
import inet.linklayer.ieee80211.Ieee80211Interface;
import inet.mobility.single.CircleMobility;
import inet.mobility.static.StationaryMobility;
import inet.networklayer.common.InterfaceTable;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;


module ThroughputClient
{
    parameters:
        @display("i=device/wifilaptop");
        @networkNode();
        *.interfaceTableModule = "^.interfaceTable";
        wlan.radio.antenna.mobilityModule = default("^.^.^.mobility");
    gates:
        input radioIn @directIn;
    submodules:
        cli: EtherAppClient {
            parameters:
                interface = "wlan";
                @display("p=300,100");
        }
        wlan: Ieee80211Interface {
            parameters:
                @display("p=250,200;q=queue");
                mgmt.typename = "Ieee80211MgmtStaSimplified";
                agent.typename = "";
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=100,100;is=s");
        }
        mobility: CircleMobility {
            parameters:
                @display("p=100,200;is=s");
        }
    connections allowunconnected:
        wlan.radioIn <-- radioIn;
        cli.out --> wlan.upperLayerIn;
}

module ThroughputServer
{
    parameters:
        @display("i=device/wifilaptop");
        @networkNode();
        *.interfaceTableModule = "^.interfaceTable";
        wlan.radio.antenna.mobilityModule = default("^.^.^.mobility");
    gates:
        input radioIn @directIn;

    submodules:
        sink: EtherAppServer {
            parameters:
                @display("p=300,100");
        }
        wlan: Ieee80211Interface {
            parameters:
                @display("p=250,200;q=queue");
                mgmt.typename = "Ieee80211MgmtStaSimplified";
                agent.typename = "";
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=100,100;is=s");
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

network Throughput
{
    parameters:
        int numCli;
    submodules:
        visualizer: <default(firstAvailableOrEmpty("IntegratedCanvasVisualizer"))> like IIntegratedVisualizer if typename != "" {
            parameters:
                @display("p=100,200;is=s");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,100;is=s");
        }
        cliHost[numCli]: ThroughputClient {
            parameters:
                @display("r=,,#707070");
        }
        srvHost: ThroughputServer {
            parameters:
                @display("p=350,350;r=,,#707070");
        }
        ap: AccessPoint {
            parameters:
                @display("p=200,200;r=,,#707070");
                wlan[*].mgmt.typename = "Ieee80211MgmtApSimplified";
                wlan[*].agent.typename = "";
        }
}
