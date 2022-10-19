//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.ipv4layer.ipv4_modular;

import inet.networklayer.contract.ipv4.IIpv4;
import inet.networklayer.contract.netfilter.INetfilterHookManager;
import inet.queueing.common.PacketMultiplexer;


module Ipv4Modular like IIpv4
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
        string arpModule;
        string icmpModule;
        @class(Ipv4Modular);
        *.interfaceTableModule = default(absPath(this.interfaceTableModule));
        *.routingTableModule = default(absPath(this.routingTableModule));
        *.arpModule = default(absPath(this.arpModule));
        *.icmpModule = default(absPath(this.icmpModule));
    gates:
        input transportIn @labels(Ipv4ControlInfo/down,TcpHeader,UdpHeader,SctpHeader);
        output transportOut @labels(Ipv4ControlInfo/up,TcpHeader,UdpHeader,SctpHeader);
        input queueIn @labels(Ipv4Header,ArpPacket,Ieee802Ctrl);
        output queueOut @labels(Ipv4Header,ArpPacket,Ieee802Ctrl);
    submodules:
        hookManager: <default("")> like INetfilterHookManager {
            @display("p=100,100");
        }
        ipv4SocketTable: Ipv4SocketTable {
            @display("p=100,200");
        }
        socketIn: Ipv4SocketCommandProcessor {
            @display("p=250,100");
            socketTableModule = "^.ipv4SocketTable";
        }
        encapsulation: Ipv4Encapsulate {
            @display("p=250,200");
        }
        outputHook: <default("OmittedNetfilterHook")> like INetfilterHook {
            @display("p=250,300");
        }
        localOut: Ipv4LocalOut {
            @display("p=250,400");
        }
        m1: PacketMultiplexer {
            @display("p=250,500");
        }
        postroutingHook: <default("OmittedNetfilterHook")> like INetfilterHook {
            @display("p=250,600");
        }
        fragmentation: Ipv4Fragmenter {
            @display("p=250,700");
        }

        routingDecision: Ipv4RoutingDecision {
            @display("p=550,500");
        }
        forwardHook: <default("OmittedNetfilterHook")> like INetfilterHook {
            @display("p=400,500");
        }

        headerChecker: Ipv4HeaderChecker {
            @display("p=700,700");
        }
        preroutingHook: <default("OmittedNetfilterHook")> like INetfilterHook {
            @display("p=700,600");
        }
        deliveryDecision: Ipv4DeliveryDecision {
            @display("p=700,500");
        }
        defragmentation: Ipv4Defragmenter {
            @display("p=700,400");
        }
        inputHook: <default("OmittedNetfilterHook")> like INetfilterHook {
            @display("p=700,300");
        }
        decapsulation: Ipv4Decapsulate {
            @display("p=700,200");
        }
        socketOut: Ipv4SocketPacketProcessor {
            @display("p=700,100");
            socketTableModule = "^.ipv4SocketTable";
        }
    connections:
        transportIn --> { @display("m=n"); } --> socketIn.in;
        socketIn.out --> encapsulation.in;
        encapsulation.out --> outputHook.in;
        outputHook.out --> localOut.in;
        localOut.out --> m1.in++;
        m1.out --> postroutingHook.in;
        postroutingHook.out --> fragmentation.in;
        fragmentation.out --> { @display("m=s"); } --> queueOut;

        queueIn --> { @display("m=s"); } --> headerChecker.in;
        headerChecker.out --> preroutingHook.in;
        preroutingHook.out --> deliveryDecision.in;
        deliveryDecision.localOut --> defragmentation.in;
        defragmentation.out --> inputHook.in;
        inputHook.out --> decapsulation.in;
        decapsulation.out --> socketOut.in;
        socketOut.out --> { @display("m=n"); } --> transportOut;

        deliveryDecision.forwardOut --> routingDecision.in;
        routingDecision.out --> forwardHook.in;
        forwardHook.out --> m1.in++;

        socketIn.socketOut --> socketOut.socketIn;
}

module Ipv4ModularCompatible extends Ipv4Modular
{
    parameters:
        string crcMode @enum("declared","computed") = default("declared");
        int timeToLive = default(32);
        int multicastTimeToLive = default(32);
        double fragmentTimeout @unit(s) = default(60s);
        bool limitedBroadcast = default(false); // send out limited broadcast packets comming from higher layer
        string directBroadcastInterfaces = default("");   // list of interfaces that direct broadcast is enabled (by default direct broadcast is disabled on all interfaces)
        hookManager.typename = "Ipv4HookManager";
        outputHook.typename = "Ipv4NetfilterHook";
        postroutingHook.typename = "Ipv4NetfilterHook";
        forwardHook.typename = "Ipv4NetfilterHook";
        preroutingHook.typename = "Ipv4NetfilterHook";
        inputHook.typename = "Ipv4NetfilterHook";
        *.crcMode = this.crcMode;
        *.timeToLive = this.timeToLive;
        *.multicastTimeToLive = this.multicastTimeToLive;
        *.fragmentTimeout = this.fragmentTimeout;
        *.limitedBroadcast = this.limitedBroadcast;
        *.directBroadcastInterfaces = this.directBroadcastInterfaces;
}
