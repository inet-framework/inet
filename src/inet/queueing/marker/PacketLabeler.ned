//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.marker;

import inet.queueing.base.PacketMarkerBase;
import inet.queueing.contract.IPacketMarker;

//
// This module attaches a ~LabelsTag to packets based on the configured filters.
// For each matching filter a new label is attached.
//
simple PacketLabeler extends PacketMarkerBase like IPacketMarker
{
    parameters:
        string filterClasses = default(""); // determines the packets which are marked, the parameter must be the name of a C++ class which implements the IPacketFilterFunction interface and is registered via Register_Class
        string labels = default(""); // a space separated list of labels
        @display("i=block/star");
        @class(PacketLabeler);
}
