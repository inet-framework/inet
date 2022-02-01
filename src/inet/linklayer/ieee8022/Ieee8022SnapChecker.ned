//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8022;

import inet.queueing.base.PacketFilterBase;

simple Ieee8022SnapChecker extends PacketFilterBase like IIeee8022SnapChecker
{
    parameters:
        @class(Ieee8022SnapChecker);
        @display("i=block/checker");
        @signal[packetDropped](type=inet::Packet);
}

