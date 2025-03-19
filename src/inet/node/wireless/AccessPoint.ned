//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.wireless;

import inet.common.MessageDispatcher;
import inet.common.lifecycle.NodeStatus;
import inet.linklayer.contract.IEthernetInterface;
import inet.linklayer.contract.IEthernetLayer;
import inet.linklayer.contract.IMacForwardingTable;
import inet.linklayer.contract.IMacRelayUnit;
import inet.linklayer.contract.IWirelessInterface;
import inet.mobility.contract.IMobility;
import inet.networklayer.common.InterfaceTable;
import inet.node.contract.INetworkNode;

//
// A wireless access point device that connects wireless stations (like WirelessHost)
// to a wired network. Supports multiple wireless radios and multiple Ethernet ports.
//
// The access point relays frames between the wireless and wired interfaces using
// a built-in MAC relay unit (bridging). It implements the IEEE 802.11 AP functionality
// including association, authentication, and frame forwarding between network segments.
//
// Key features:
// - Configurable number of wireless interfaces (radios)
// - Configurable number of Ethernet ports
// - IEEE 802.11 management functionality
// - MAC-layer bridging between interfaces
// - Customizable wireless management type (simplified or full-featured)
// - Stationary by default, but mobility can be added
//
// The wireless management can be configured to use either the full IEEE 802.11 management
// protocol (~Ieee80211MgmtAp) or a simplified version (~Ieee80211MgmtApSimplified) that
// doesn't support scanning, authentication, and association processes.
//
// @see ~WirelessHost, ~EthernetSwitch, ~StandardHost
//
module AccessPoint like INetworkNode
{
    parameters:
        @networkNode();
        @labels(node,ethernet-node,wireless-node);
        @display("i=device/accesspoint");
        int numWlanInterfaces = default(1);               // The number of radios in the access point
        bool hasStatus = default(false);
        string fcsMode @enum("declared","computed") = default("declared");
        wlan[*].protocol = default("ethernetmac");
        wlan[*].mgmt.typename = default("Ieee80211MgmtAp");
        wlan[*].llc.typename = default("Ieee80211Portal");
        wlan[*].agent.typename = default("");
        wlan[*].radio.antenna.mobilityModule = default("^.^.^.mobility");
        eth[*].encap.typename = "";
        *.macTableModule = default(absPath(".macTable"));
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        bridging.hasStp = false;
        *.fcsMode = this.fcsMode;
        ethernet.registerProtocol = default(true);
    gates:
        input radioIn[numWlanInterfaces] @directIn;
        inout ethg[] @labels(EtherFrame-conn);
    submodules:
        status: NodeStatus if hasStatus {
            @display("p=100,100;is=s");
        }
        interfaceTable: InterfaceTable {
            @display("p=100,200;is=s");
        }
        macTable: <default("MacForwardingTable")> like IMacForwardingTable {
            @display("p=100,300;is=s");
        }
        mobility: <default("StationaryMobility")> like IMobility {
            @display("p=100,400;is=s");
        }
        bridging: <default(firstAvailable("Ieee8021dRelay","MacRelayUnit"))> like IMacRelayUnit if sizeof(ethg)+numWlanInterfaces > 1 && typename != "" {
            @display("p=800,100");
        }
        bl: MessageDispatcher if sizeof(ethg)+numWlanInterfaces > 1 && exists(bridging) {
            @display("p=800,200;b=1200,5,,,,1");
        }
        ethernet: <default("EthernetEncapsulation")> like IEthernetLayer if typename != "" && exists(bridging) {
            @display("p=700,300");
        }
        li: MessageDispatcher {
            @display("p=800,400;b=1000,5,,,,1");
        }
        wlan[numWlanInterfaces]: <default("Ieee80211Interface")> like IWirelessInterface {
            @display("p=250,500,row,150;q=queue");
        }
        eth[sizeof(ethg)]: <default("EthernetInterface")> like IEthernetInterface {
            mac.promiscuous = true;
            @display("p=850,500,row;q=txQueue");
        }
    connections allowunconnected:
        bl.out++ --> bridging.lowerLayerIn if sizeof(ethg)+numWlanInterfaces > 1 && exists(bridging);
        bl.in++ <-- bridging.lowerLayerOut if sizeof(ethg)+numWlanInterfaces > 1 && exists(bridging);

        bl.out++ --> li.in++ if exists(bl);
        li.out++ --> bl.in++ if exists(bl);

        bl.out++ --> ethernet.upperLayerIn if exists(ethernet);
        ethernet.upperLayerOut --> bl.in++ if exists(ethernet);
        ethernet.lowerLayerOut --> li.in++ if exists(ethernet);
        li.out++ --> ethernet.lowerLayerIn if exists(ethernet);

        // connections to network outside
        // wireless interfaces MUST be connected first (i.e. ports 0..numWlanInterfaces-1)
        // because broadcasts must be handled differently for wireless IFs by the bridging
        for i=0..numWlanInterfaces-1 {
            radioIn[i] --> { @display("m=s"); } --> wlan[i].radioIn;
            wlan[i].upperLayerOut --> li.in++ if sizeof(ethg)+numWlanInterfaces > 1 && exists(bridging);
            wlan[i].upperLayerIn <-- li.out++ if sizeof(ethg)+numWlanInterfaces > 1 && exists(bridging);
        }
        // ethernet must be connected only AFTER wireless ports
        for i=0..sizeof(ethg)-1 {
            eth[i].phys <--> { @display("m=s"); } <--> ethg[i];
            eth[i].upperLayerIn <-- li.out++ if sizeof(ethg)+numWlanInterfaces>1;
            eth[i].upperLayerOut --> li.in++ if sizeof(ethg)+numWlanInterfaces>1;
        }
}

