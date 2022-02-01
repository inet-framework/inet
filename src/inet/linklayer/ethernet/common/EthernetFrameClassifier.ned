//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.common;

import inet.queueing.base.PacketClassifierBase;
import inet.queueing.contract.IPacketClassifier;

//
// Classifier that forwards Ethernet PAUSE frames
// to the pauseOut gate, and other frames to the defaultOut
// gate.
//
// This component can be used to build a QoS queue that gives
// the PAUSE frames the highest priority.
//
simple EthernetFrameClassifier extends PacketClassifierBase like IPacketClassifier
{
    parameters:
        @class(EthernetFrameClassifier);
        @display("i=block/classifier");
}

