//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.classifier;

//
// Dynamically creates and uses other classifier modules during simulation runtime.
// Allows for flexible and adaptable packet classification strategies that can
// change based on simulation conditions.
//
// The DynamicClassifier works by:
// - Creating classifier modules of the specified type on demand
// - Using these dynamically created classifiers to determine packet routing
// - Allowing classification strategies to evolve during simulation
//
// Key features:
// - Runtime creation of classifier modules
// - Flexible adaptation to changing network conditions
// - Support for dynamic classification policies
// - Ability to switch between different classification strategies
//
// The moduleType parameter specifies the type of classifier module to create,
// while submoduleName defines the name pattern for the created modules.
//
// @see ~PacketClassifier, ~ContentBasedClassifier, ~PriorityClassifier
//
simple DynamicClassifier extends PacketClassifier
{
    parameters:
        string submoduleName; // Name pattern for dynamically created classifier modules
        string moduleType;    // Type of classifier module to create dynamically
        @class(DynamicClassifier);
}
