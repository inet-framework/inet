//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.classifier;

//
// Classifies packets based on the attached ~UserPriority tag. Routes packets
// to different output gates according to their priority level, enabling
// Quality of Service (QoS) differentiation in network simulations.
//
// The classifier uses the PacketUserPriorityReqClassifier implementation which:
// - Examines each packet for a UserPriority tag
// - Routes the packet to the output gate corresponding to its priority value
// - Supports the standard 802.1D/802.11e priority levels (0-7)
//
// The number of output gates should match the number of priority levels
// being used in the simulation.
//
// @see ~PriorityClassifier, ~ContentBasedClassifier, ~LabelClassifier
//
simple UserPriorityClassifier extends PacketClassifier
{
    parameters:
        classifierClass = default("inet::queueing::PacketUserPriorityReqClassifier");
}
