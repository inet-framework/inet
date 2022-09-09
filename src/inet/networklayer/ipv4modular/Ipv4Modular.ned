//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.ipv4modular;

import inet.networklayer.ipv4.IIpv4;
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
        ipv4SocketTable: Ipv4SocketTable {
            @display("p=90,100");
        }
        socketIn: Ipv4SocketCommandProcessor {
            @display("p=200,100");
            socketTableModule = "^.ipv4SocketTable";
        }
        encap: Ipv4Encapsulate {
            @display("p=200,200");
        }
        localOut: Ipv4LocalOut {
            @display("p=200,400");
        }
        m1: PacketMultiplexer {
            @display("p=200,500");
        }
        frag: Ipv4Fragmenter {
            @display("p=200,700");
        }

        prepareFwd: Ipv4PrepareForward {
            @display("p=400,500");
        }

        frameReceiver: Ipv4FrameReceiver {
            @display("p=500,700");
        }
        routingDecision: Ipv4RoutingDecision {
            @display("p=500,500");
        }
        defrag: Ipv4Defragmenter {
            @display("p=500,400");
        }
        decap: Ipv4Decapsulate {
            @display("p=500,200");
        }
        socketOut: Ipv4SocketPacketProcessor {
            @display("p=500,100");
            socketTableModule = "^.ipv4SocketTable";
        }
    connections:
        transportIn --> { @display("m=n"); } --> socketIn.in;
        socketIn.out --> encap.in;
        encap.out --> localOut.in;
        localOut.out --> m1.in++;
        m1.out --> frag.in;
        frag.out --> { @display("m=s"); } --> queueOut;

        queueIn --> { @display("m=s"); } --> frameReceiver.in;
        frameReceiver.out --> routingDecision.in;
        routingDecision.localOut --> defrag.in;
        defrag.out --> decap.in;
        decap.out --> socketOut.in;
        socketOut.out --> { @display("m=n"); } --> transportOut;

        routingDecision.forwardOut --> prepareFwd.in;
        prepareFwd.out --> m1.in++;

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
        *.crcMode = this.crcMode;
        *.timeToLive = this.timeToLive;
        *.multicastTimeToLive = this.multicastTimeToLive;
        *.fragmentTimeout = this.fragmentTimeout;
        *.limitedBroadcast = this.limitedBroadcast;
        *.directBroadcastInterfaces = this.directBroadcastInterfaces;
}
