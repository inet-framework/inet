//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.emulation.networklayer.ipv4;

import inet.networklayer.ipv4.IIpv4;

//
// Provides IPv4 layer functionality for network emulation that connects
// simulated applications to a real network on the host computer. Uses a real
// TUN device on the host OS to capture and inject packets at the IP layer,
// allowing simulated applications to communicate with real-world network
// endpoints through the host's networking stack.
//
module ExtUpperIpv4 like IIpv4
{
    gates:
        input transportIn @labels(Ipv4ControlInfo/down,TcpHeader,UdpHeader,SctpHeader);
        output transportOut @labels(Ipv4ControlInfo/up,TcpHeader,UdpHeader,SctpHeader);
        input queueIn @labels(Ipv4Header,ArpPacket,Ieee802Ctrl);
        output queueOut @labels(Ipv4Header,ArpPacket,Ieee802Ctrl);
    submodules:
        tun: ExtIpv4TunDevice {
            parameters:
                @display("p=100,150");
        }
    connections allowunconnected:
        queueIn --> tun.lowerLayerIn;
        queueOut <-- tun.lowerLayerOut;
}
