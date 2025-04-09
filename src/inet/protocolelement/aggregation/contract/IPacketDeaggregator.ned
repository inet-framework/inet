//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.aggregation.contract;

//
// Interface for packet deaggregator modules that split an aggregated packet back
// into its original component packets. Deaggregation is the reverse process of
// aggregation, extracting the individual packets that were previously combined to
// reduce header overhead. Implementations of this interface receive an aggregated
// packet and forward each extracted subpacket separately.
//
moduleinterface IPacketDeaggregator
{
    parameters:
        @display("i=block/fork");
}
