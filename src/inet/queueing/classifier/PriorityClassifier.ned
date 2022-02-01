//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.classifier;

import inet.queueing.base.PacketClassifierBase;
import inet.queueing.contract.IPacketClassifier;

//
// This classifier pushes packets into the first non-full among its connected
// packet consumers.
//
simple PriorityClassifier extends PacketClassifierBase like IPacketClassifier
{
    parameters:
        @class(PriorityClassifier);
}
