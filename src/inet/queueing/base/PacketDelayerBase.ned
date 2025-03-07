//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// Base module for packet delayer components in the queueing framework.
// Packet delayers introduce controlled time delays between receiving
// and forwarding packets, allowing simulation of various network
// delay characteristics.
//
// This module serves as the foundation for implementing different
// delay models such as constant delays, random delays, or delay
// patterns based on packet attributes.
//
// Key features:
// - Operates in push mode, receiving packets on input and forwarding
//   them to output after a specified delay
// - Supports integration with external clock modules for synchronized timing
// - Configurable scheduling priority for precise control of event ordering
// - Option to schedule or skip zero-delay events
//
// Derived modules can implement specific delay calculation strategies
// by defining how long each packet should be delayed before forwarding.
//
// @see ~PacketDelayer, ~BitRatePacketDelayer
//
simple PacketDelayerBase extends PacketProcessorBase
{
    parameters:
        string clockModule = default(""); // Relative path of a module that implements IClock; optional
        int schedulingPriority = default(0); // Priority used when scheduling timer events for delayed packet processing
        bool scheduleZeroDelay = default(true); // When true, even zero-delay packets are scheduled as events; when false, they're processed immediately
        @class(PacketDelayerBase);
        @display("i=block/delay");
    gates:
        input in @labels(push); // Input gate for receiving packets
        output out @labels(push); // Output gate for forwarding delayed packets
}
