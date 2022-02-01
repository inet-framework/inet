//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.contract;

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

