//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.classifier;

simple DynamicClassifier extends PacketClassifier
{
    parameters:
        string submoduleName;
        string moduleType;
        @class(DynamicClassifier);
}
