//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// Base module for all packet processing components in the queueing framework.
// This module serves as the foundation for the entire packet processing
// architecture, providing common functionality for statistics collection
// and visualization.
//
// The PacketProcessorBase is the root of the inheritance hierarchy for all
// packet processing modules, including:
// - Packet sources and sinks
// - Packet queues and buffers
// - Packet filters and classifiers
// - Packet schedulers and servers
// - Packet markers and shapers
// - Packet gates and meters
//
// All derived modules inherit the ability to track:
// - Number of processed packets
// - Total length of processed packets
// - Processing rates and statistics
//
// Available display string text format directives:
// - %p - number of processed packets
// - %l - total processed packet length
//
simple PacketProcessorBase
{
    parameters:
        string displayStringTextFormat = default("processed %p pk (%l)"); // Determines the text that is written on top of the submodule
        @display("i=block/star");
}
