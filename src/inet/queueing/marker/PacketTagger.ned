//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.marker;

import inet.queueing.base.PacketTaggerBase;
import inet.queueing.contract.IPacketMarker;

//
// This module attaches various request tags to packets.
//
simple PacketTagger extends PacketTaggerBase like IPacketMarker
{
    parameters:
        string filterClass = default("inet::queueing::AnyPacketFilter"); // determines which packets are tagged
        @class(PacketTagger);
}
