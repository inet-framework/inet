//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.classifier;

import inet.queueing.base.PacketClassifierBase;
import inet.queueing.contract.IPacketClassifier;

//
// This module connects one packet producer to multiple packet consumers. It
// can be pushed with packets from the connected packet producer. When this
// happens, the classifier pushes the packet to one of its connected packet
// consumers based on the configured packet classifier function. The packet
// classifier function takes a packet and returns an integer which in turn
// is used for determining the output gate.
//
simple PacketClassifier extends PacketClassifierBase like IPacketClassifier
{
    parameters:
        string classifierClass; // determines the output queue of packets, the parameter must be the name of a C++ class which implements the IPacketClassifierFunction interface and is registered via Register_Class
        @class(PacketClassifier);
}
