//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


import inet.applications.contract.IApp;
import inet.common.MessageDispatcher;
import inet.linklayer.acking.AckingMac;
import inet.linklayer.ethernet.EthernetInterface;
import inet.linklayer.ieee80211.Ieee80211Interface;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.networklayer.ipv4.Ipv4;
import inet.node.ethernet.Eth1G;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.AdhocHost;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.physicallayer.wireless.apsk.packetlevel.ApskScalarRadio;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.transportlayer.tcp.Tcp;

//!WiredNetworkExample
network WiredNetworkExample
{
    parameters:
        int numClients; // number of clients in the network
    submodules:
        configurator: Ipv4NetworkConfigurator; // network autoconfiguration
        server: StandardHost; // predefined standard host
        router: Router; // predefined router
        switch: EthernetSwitch; // predefined ethernet switch
        client[numClients]: StandardHost;
    connections: // network level connections
        router.pppg++ <--> { datarate = 1GBps; } <--> server.pppg++; // PPP
        switch.ethg++ <--> Eth1G <--> router.ethg++; // bidirectional ethernet
        for i=0..numClients-1 {
            client[i].ethg++ <--> Eth1G <--> switch.ethg++; // ethernet
        }
}
//!End

//!WirelessNetworkExample
network WirelessNetworkExample
    submodules:
        configurator: Ipv4NetworkConfigurator;
        radioMedium: Ieee80211ScalarRadioMedium;
        host1: WirelessHost { @display("p=200,100"); }
        host2: WirelessHost { @display("p=500,100"); }
        accessPoint: AccessPoint { @display("p=374,200"); }
}
//!End

//!MobileAdhocNetworkExample
network MobileAdhocNetworkExample
{
    parameters:
        int numHosts; // number of nodes in the network
    submodules:
        configurator: Ipv4NetworkConfigurator; // network autoconfiguration
        radioMedium: Ieee80211ScalarRadioMedium; // 802.11 physical medium
        host[numHosts]: AdhocHost; // ad-hoc wifi nodes
}
//!End

//!NetworkNodeExample
module NetworkNodeExample
{
    parameters:
    gates:
        inout ethg; // ethernet interface connector
        input radioIn; // incoming radio frames from physical medium
    submodules:
        app: <> like IApp; // configurable application
        tcp: Tcp; // standard TCP protocol
        ip: Ipv4; // standard IP protocol
        md: MessageDispatcher; // connects multiple interfaces to IP
        wlan: Ieee80211Interface; // standard wifi interface
        eth: EthernetInterface; // standard ethernet interface
        interfaceTable: InterfaceTable;
    connections: // network node internal connections
        app.socketOut --> tcp.appIn; // application sends data stream
        app.socketIn <-- tcp.appOut; // application receives data stream
        tcp.ipOut --> ip.transportIn; // TCP sends segments
        tcp.ipIn <-- ip.transportOut; // TCP receives segments
        ip.queueOut --> md.in++; // IP sends datagrams
        ip.queueIn <-- md.out++; // IP receives datagrams
        md.out++ --> wlan.upperLayerIn;
        md.in++ <-- wlan.upperLayerOut;
        md.out++ --> eth.upperLayerIn;
        md.in++ <-- eth.upperLayerOut;
        eth.phys <--> ethg; // Ethernet sends frames to cable
        radioIn --> wlan.radioIn; // IEEE 802.11 sends frames to medium
}
//!End

//!WirelessInterfaceExample
module WirelessInterfaceExample
{
    gates:
        input upperLayerIn; // packets from network layer in the same host
        output upperLayerOut; // packets to network layer in the same host
        input radioIn; // incoming packets from other hosts in the network
    submodules:
        mac: AckingMac; // simple MAC supporting ACKs
        radio: ApskScalarRadio; // simple radio supporting many modulations
    connections: // network interface internal connections
        mac.upperLayerOut --> upperLayerOut;
        mac.upperLayerIn <-- upperLayerIn;
        radio.upperLayerOut --> mac.lowerLayerIn;
        radio.upperLayerIn <-- mac.lowerLayerOut;
        radioIn --> radio.radioIn;
}
//!End

module AutomaticWiredInterfacesExample
{
    submodules:
        router1: Router;
        router2: Router;
    connections: // network level connections
//!AutomaticWiredInterfacesExample
        router1.ethg++ <--> Eth1G <--> router2.ethg++; // automatic interfaces
//!End
}
