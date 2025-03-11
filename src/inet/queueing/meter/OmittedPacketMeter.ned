//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.meter;

import inet.queueing.contract.IPacketMeter;

//
// Placeholder implementation of the IPacketMeter interface that automatically
// removes itself from the module hierarchy during initialization. Creates a direct
// connection between its input and output gates, effectively bypassing the
// metering functionality.
//
// The OmittedPacketMeter is particularly useful for:
// - Creating configurable network models where metering can be easily enabled or disabled
// - Simplifying model structure when packet metering is not needed in certain scenarios
// - Maintaining consistent module interfaces throughout a model while selectively
//   omitting functionality in specific instances
// - Facilitating parameter studies where the impact of packet metering needs to be evaluated
//
// When this module is used, packets pass through unchanged without any metering,
// labeling, or tagging operations being performed.
//
// Usage example:
// In a network configuration, you can use a parameter to conditionally select
// either a functional meter or an OmittedPacketMeter:
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
