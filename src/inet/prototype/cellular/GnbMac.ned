//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.prototype.cellular;

import inet.common.SimpleModule;

//
// The dummy MAC entity of a base station sector, above one ~CellularRadio per component
// carrier; this is how carrier aggregation appears: one MAC entity, several radios. Each slot of
// each carrier, it divides the PRBs among `scheduledUesPerSlot` attached UEs round-robin, and
// sends one downlink allocation per UE, with the beam that points closest to the UE and the
// highest MCS that the last SNIR report allows. It also grants uplink allocations
// `uplinkGrantDelay` slots ahead, through ~IdealControlChannel. Traffic is full buffer; there
// is no HARQ, RLC, PDCP or upper layer.
//
simple GnbMac extends SimpleModule
{
    parameters:
        string controlChannelModule = default("^.^.^.controlChannel"); // Path of the ~IdealControlChannel, relative to this module; by default the module named controlChannel in the network
        int scheduledUesPerSlot = default(4); // Number of UEs that get an allocation in each slot of each carrier and direction
        string tddPattern = default("DDDSU"); // Slot pattern of TDD carriers: D downlink, S special (used as downlink), U uplink
        int downlinkFirstSymbol = default(2); // First OFDM symbol of downlink allocations, after the control region
        int downlinkNumSymbols = default(12); // Number of OFDM symbols of downlink allocations
        int uplinkNumSymbols = default(14); // Number of OFDM symbols of uplink allocations
        int uplinkGrantDelay = default(1); // Number of slots between an uplink grant and its allocation
        string allocationType @enum("contiguous","bitmap") = default("contiguous"); // Downlink PRB sets: one contiguous block per UE, or interleaved RBGs
        int rbgSize = default(16); // Number of PRBs of one RBG in bitmap allocations
        double mcsMargin @unit(dB) = default(3dB); // Margin of the link adaptation below the reported SNIR
        @class(GnbMac);
        @display("i=block/layer");
        @signal[transportBlockReceived](type=inet::Packet);
        @signal[transportBlockFailed](type=inet::Packet);
        @signal[effectiveSnirMeasured](type=double);
        @statistic[receivedTransportBlocks](title="received transport blocks"; source=transportBlockReceived; record=count,"sum(packetBytes)"; interpolationmode=none);
        @statistic[failedTransportBlocks](title="failed transport blocks"; source=transportBlockFailed; record=count; interpolationmode=none);
        @statistic[effectiveSnir](title="effective SNIR"; source=effectiveSnirMeasured; unit=dB; record=mean,histogram; interpolationmode=none);
    gates:
        input lowerLayerIn[];
        output lowerLayerOut[];
}
