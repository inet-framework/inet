//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.common;

import inet.queueing.base.PacketClassifierBase;
import inet.queueing.contract.IPacketClassifier;

//
// Packet classifier that separates Ethernet PAUSE frames from other frame types. Forwards PAUSE frames 
// to the first output gate and all other Ethernet frames to the second output gate. Enables prioritized 
// handling of flow control frames in Ethernet networks by allowing PAUSE frames to bypass normal queuing 
// mechanisms.
//
// @see ~EthernetMacHeader, ~IPacketClassifier, ~PacketClassifierBase
//
simple EthernetFrameClassifier extends PacketClassifierBase like IPacketClassifier
{
    parameters:
        @class(EthernetFrameClassifier);
        @display("i=block/classifier");
}
