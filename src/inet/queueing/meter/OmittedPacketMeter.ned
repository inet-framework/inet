//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.meter;

import inet.queueing.contract.IPacketMeter;

//
// This module provides a placeholder implementation of the IPacketMeter interface
// that automatically removes itself from the module hierarchy during initialization.
// It creates a direct connection between its input and output gates, effectively
// bypassing the metering functionality.
//
// When this module is used, packets pass through unchanged without any metering,
// labeling, or tagging operations being performed.
//
// meter: <default("OmittedPacketMeter")> like IPacketMeter if enableMetering else OmittedPacketMeter;
//
// @see ~TokenBucketMeter, ~MultiTokenBucketMeter, ~ExponentialRateMeter
//
module OmittedPacketMeter like IPacketMeter
{
    parameters:
        @class(::inet::OmittedModule);
        @display("i=block/star,black,50");
    gates:
        input in;
        output out;
    connections:
        in --> out;
}
