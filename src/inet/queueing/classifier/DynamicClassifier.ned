//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.queueing.classifier;

//
// Extends PacketClassifier with the ability to dynamically create new output
// gates and corresponding submodules as needed. When a packet is classified to
// a category that doesn't have a corresponding output gate yet, the classifier
// creates a new gate and instantiates a new module of the specified type to
// process that packet category.
//
simple DynamicClassifier extends PacketClassifier
{
    parameters:
        string submoduleName;
        string moduleType;
        @class(DynamicClassifier);
}
