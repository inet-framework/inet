//
// Copyright (C) 2021 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.common;

//
// Provides a placeholder implementation of the IEEE 802.1d QoS classifier interface
// that automatically removes itself from the module hierarchy during initialization.
// This module is used when QoS classification is not needed but the module structure
// requires a classifier component.
//
// When this module is used, packets pass through unchanged without any QoS classification
// or priority assignment. This simplifies network configurations where QoS is not
// required while maintaining a consistent module structure.
//
// The module connects its input directly to its output, and during initialization,
// it removes itself from the module hierarchy, effectively becoming transparent
// to the simulation.
//
// @see ~IIeee8021dQosClassifier, ~QosClassifier
//
module OmittedIeee8021dQosClassifier like IIeee8021dQosClassifier
{
    parameters:
        @class(::inet::OmittedModule);
        @display("i=block/classifier,black,50");
    gates:
        input in;
        output out;
    connections:
        in --> out;
}
