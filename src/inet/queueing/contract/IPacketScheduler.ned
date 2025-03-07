//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// Module interface for packet scheduler components in the queueing framework.
// Packet schedulers select packets from multiple input sources and forward them
// to a single output.
//
// Key characteristics:
// - Has multiple active inputs and one passive output
// - Selects packets from the inputs based on a scheduling algorithm
// - Forwards packets without delay or reordering
// - Implements various scheduling disciplines like priority, round-robin, etc.
//
// Packet schedulers are commonly used in network devices to:
// - Implement quality of service mechanisms
// - Prioritize certain traffic types over others
// - Fairly distribute bandwidth among multiple flows
// - Enforce service level agreements
//
// Common implementations include PriorityScheduler, WrrScheduler (Weighted
// Round Robin), and ContentBasedScheduler.
//
moduleinterface IPacketScheduler extends IPassivePacketSource
{
    parameters:
        @display("i=block/join");
    gates:
        input in[] @labels(pull);
}
