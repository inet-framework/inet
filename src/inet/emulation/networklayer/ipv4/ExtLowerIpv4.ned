//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.emulation.networklayer.ipv4;

import inet.networklayer.ipv4.IIpv4;

module ExtLowerIpv4 like IIpv4
{
    gates:
        input transportIn @labels(Ipv4ControlInfo/down,TcpHeader,UdpHeader,SctpHeader);
        output transportOut @labels(Ipv4ControlInfo/up,TcpHeader,UdpHeader,SctpHeader);
        input queueIn @labels(Ipv4Header,ArpPacket,Ieee802Ctrl);
        output queueOut @labels(Ipv4Header,ArpPacket,Ieee802Ctrl);
    submodules:
        encap: Ipv4Encap {
            parameters:
                @display("p=100,50");
        }
        socket: ExtIpv4Socket {
            parameters:
                @display("p=100,150");
        }
    connections allowunconnected:
        transportIn --> encap.upperLayerIn;
        transportOut <-- encap.upperLayerOut;
        encap.lowerLayerOut --> socket.upperLayerIn;
        encap.lowerLayerIn <-- socket.upperLayerOut;
}
