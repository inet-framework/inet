//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// Module interface for packet classifier components in the queueing framework.
// Packet classifiers direct incoming packets to one of multiple outputs based
// on packet attributes or other criteria.
//
// Key characteristics:
// - Has one passive input and multiple active outputs
// - Classifies packets based on various criteria (content, priority, label, etc.)
// - Forwards packets to the appropriate output without delay or reordering
// - Operates in push mode (packets are pushed in and then pushed out)
//
// Packet classifiers are commonly used in network devices to:
// - Separate traffic into different service classes
// - Direct packets to different processing paths based on their properties
// - Implement traffic filtering and access control
// - Support quality of service mechanisms
//
// Common implementations include PriorityClassifier, ContentBasedClassifier,
// LabelClassifier, and MarkovClassifier.
//
moduleinterface IPacketClassifier extends IPassivePacketSink
{
    parameters:
        @display("i=block/classifier");
    gates:
        output out[] @labels(push);
}
