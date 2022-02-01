//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.classifier;

//
// This module classifies packets based on the attached ~UserPriority tag.
//
simple UserPriorityClassifier extends PacketClassifier
{
    parameters:
        classifierClass = default("inet::queueing::PacketUserPriorityReqClassifier");
}
