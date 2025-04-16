//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.contract;

//
// Interface for Address Resolution Protocol (ARP) implementations. Provides
// address resolution services for mapping network layer addresses (e.g., IPv4)
// to link layer addresses (e.g., MAC). Modules implementing this interface
// maintain a cache of address mappings and handle resolution requests from
// the network layer.
//
moduleinterface IArp
{
    parameters:
        @display("i=block/layer");
        @signal[arpResolutionInitiated](type=inet::IArp::Notification);
        @signal[arpResolutionCompleted](type=inet::IArp::Notification);
        @signal[arpResolutionFailed](type=inet::IArp::Notification);
    gates:
        input ifIn @labels(ArpPacket,Ieee802Ctrl);
        output ifOut @labels(ArpPacket,Ieee802Ctrl);
}

