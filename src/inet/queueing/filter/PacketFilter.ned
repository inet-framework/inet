//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.filter;

import inet.queueing.base.PacketFilterBase;
import inet.queueing.contract.IPacketFilter;

//
// Extensible packet filter that uses a pluggable filtering function to selectively
// forward packets. This module provides a framework for implementing custom packet
// filtering algorithms.
//
// Operation mechanism:
// - In push mode: Receives packets from a producer, forwards matching packets to
//   the consumer and drops non-matching packets
// - In pull mode: Pulls packets from a provider until finding one that matches
//   the filter criteria, then forwards it to the collector
// - The filtering decision is made by the configured filter function
//
// The PacketFilter serves as a base for implementing custom filtering algorithms
// without having to implement a complete filter module. By providing different
// filter function implementations, various filtering policies can be realized.
//
// The filterClass parameter specifies the C++ class that implements the
// filtering algorithm. This class must implement the IPacketFilterFunction
// interface and be registered via Register_Class.
//
// @see ~ContentBasedFilter, ~LabelFilter, ~RedDropper, ~OrdinalBasedDropper
//
simple PacketFilter extends PacketFilterBase like IPacketFilter
{
    parameters:
        string filterClass; // Determines the packets that pass the filter, the parameter must be the name of a C++ class that implements the IPacketFilterFunction interface and is registered via Register_Class
        @class(PacketFilter);
}
