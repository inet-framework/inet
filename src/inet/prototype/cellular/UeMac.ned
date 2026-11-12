//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.prototype.cellular;

import inet.common.SimpleModule;

//
// The dummy MAC entity of a UE, above one ~CellularRadio per component carrier. At
// initialization it attaches to the sector that it hears best. It sends the uplink allocations
// that the sector grants, sharing `maxTransmissionPower` among the carriers of the same time
// (each carrier asks for the maximum power of its transmitter, and all are scaled down equally
// if the sum is too high), and it reports the effective SNIR of each downlink transport block.
//
simple UeMac extends SimpleModule
{
    parameters:
        string controlChannelModule = default("^.^.controlChannel"); // Path of the ~IdealControlChannel, relative to this module; by default the module named controlChannel in the network
        double maxTransmissionPower @unit(W) = default(0.2W); // Maximum total transmission power of the UE over all carriers
        @class(UeMac);
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
