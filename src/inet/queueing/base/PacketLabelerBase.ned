//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// This is a base module for various packet labeler modules.
//
simple PacketLabelerBase extends PacketMarkerBase
{
    parameters:
        string labels = default(""); // a space separated list of labels
        @class(PacketLabelerBase);
        @display("i=block/star");
}
