//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.Simsignals;
import inet.common.geometry.Geometry;
import inet.networklayer.common.NetworkInterface;

cplusplus{{
#include "inet/visualizer/base/PacketDropVisualizerBase.h"
}}

namespace inet::visualizer;

class PacketDrop extends inet::PacketDropDetails
{
    @existingClass;
    @descriptor(readonly);
    cPacket *packet_;
    cModule *module;
    cModule *networkNode;
    NetworkInterface *networkInterface;
    Coord position;
}
