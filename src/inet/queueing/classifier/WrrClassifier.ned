//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.classifier;

import inet.queueing.base.PacketClassifierBase;
import inet.queueing.contract.IPacketClassifier;

//
// Implements a weighted round-robin (WRR) packet classifier that distributes
// packets among its output gates based on assigned weights. WRR provides
// proportional bandwidth allocation while ensuring that all queues receive service.
//
// Key features:
// - Fair bandwidth allocation according to predefined weights
// - Prevents starvation of low-priority traffic
// - Provides QoS differentiation while maintaining throughput guarantees
// - Token-based implementation for accurate weight enforcement
//
// Operation mechanism:
// - Each output gate is assigned a weight via the weights parameter
// - Initially, each output gate receives tokens equal to its weight
// - When classifying a packet, the first output gate with available packets and tokens is selected
// - One token is consumed from the selected gate's allocation
// - When no gates have enough tokens to classify a packet, all token buckets are refilled
//
// The weights parameter specifies relative bandwidth allocation. For example,
// with weights="3 2 1", the first output will receive 50% of bandwidth,
// the second 33%, and the third 17% (assuming all queues are backlogged).
//
// @see ~WrrScheduler, ~PriorityClassifier, ~MarkovClassifier
//
simple WrrClassifier extends PacketClassifierBase like IPacketClassifier
{
    parameters:
        string weights; // Space-separated list of weights for each output gate (e.g., "3 2 1")
        @class(WrrClassifier);
}
