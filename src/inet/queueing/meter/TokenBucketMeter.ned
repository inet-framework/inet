//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.meter;

import inet.queueing.base.PacketMeterBase;
import inet.queueing.contract.IPacketMeter;

//
// This module implements the classic token bucket algorithm for traffic measurement,
// rate limiting, and packet labeling. It is a fundamental building block for
// implementing Quality of Service (QoS) mechanisms in network simulations.
//
// The token bucket algorithm works as follows:
// - Tokens are generated at a constant rate (tokenProductionRate) and stored in a bucket
// - The bucket has a maximum capacity (maxNumTokens) to limit burst size
// - When a packet arrives, the meter checks if enough tokens are available
// - If sufficient tokens exist, they are consumed (based on packet size or fixed amount)
//   and the packet is labeled with the specified label
// - If insufficient tokens exist, the packet is labeled with the defaultLabel
// - Excess tokens that would overflow the bucket can be forwarded to another module
//
// Key features and capabilities:
// - Precise traffic rate control through token production rate configuration
// - Burst tolerance through bucket capacity configuration
// - Flexible token consumption based on packet size or fixed amounts per packet
// - Packet labeling for downstream processing (e.g., by filters or classifiers)
// - Token sharing between multiple buckets via the excessTokenModule parameter
// - Color-aware operation through the labelPriority parameter
// - Comprehensive statistics collection for monitoring and analysis
//
// Configuration examples:
// 1. Basic rate limiter (1 Mbps with 64 KB burst tolerance):
//    tokenProductionRate = 125000; // bytes/s (1 Mbps)
//    maxNumTokens = 65536; // bytes (64 KB)
//    tokenConsumptionPerBit = 0.125; // 1 token per byte
//    label = "conformant";
//    defaultLabel = "excess";
//
// 2. Packet counter (limit to 100 packets per second):
//    tokenProductionRate = 100; // tokens/s
//    maxNumTokens = 100; // maximum burst
//    tokenConsumptionPerPacket = 1; // 1 token per packet
//    tokenConsumptionPerBit = 0; // ignore packet size
//
// The TokenBucketMeter serves as the foundation for more complex metering schemes
// such as the SingleRateTwoColorMeter, SingleRateThreeColorMeter, and
// DualRateThreeColorMeter, which implement standardized traffic conditioning
// algorithms used in DiffServ architectures.
//
// @see ~MultiTokenBucketMeter, ~SingleRateTwoColorMeter, ~SingleRateThreeColorMeter, ~DualRateThreeColorMeter
//
simple TokenBucketMeter extends PacketMeterBase like IPacketMeter
{
    parameters:
        double initialNumTokens = default(0); // Initial number of tokens in the bucket at simulation start
        double maxNumTokens = default(inf); // Maximum number of tokens the bucket can hold (burst tolerance)
        double tokenProductionRate; // Rate at which tokens are generated per second
        double tokenConsumptionPerPacket = default(0); // Fixed number of tokens consumed per packet (regardless of size)
        double tokenConsumptionPerBit = default(1); // Number of tokens consumed per bit of packet length
        string label; // Label attached to packets when sufficient tokens are available
        string defaultLabel = default(""); // Label attached to packets when insufficient tokens are available (empty string means no label)
        object labelPriority = default([]); // List of labels in order of priority for color-aware operation (empty list means color-blind mode)
        string excessTokenModule = default(""); // Path to another module (~ITokenStorage) that receives excess tokens when this bucket overflows
        displayStringTextFormat = default("contains %n tk\nmetered %p pk (%l)");
        @signal[tokensChanged](type=double);
        @statistic[numTokens](title="number of tokens"; source=tokensChanged; record=vector; unit=tk; interpolationmode=linear);
        @class(TokenBucketMeter);
}
