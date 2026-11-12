//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.prototype.cellular;

import inet.physicallayer.wireless.common.base.packetlevel.NarrowbandReceiverBase;

//
// Receives the allocations of one component carrier that are addressed to its radio. It listens
// on the whole carrier; with the listening filter of ~CellularRadioMedium, the medium delivers a
// transmission only to its intended receiver, and every other transmission counts only as
// interference. The decision is one per transport block, by the error model; the SNIR threshold
// of the base module does not apply, because most of the band is outside the allocation.
//
module CellularReceiver extends NarrowbandReceiverBase
{
    parameters:
        double subcarrierSpacing @unit(Hz) = default(30kHz); // Subcarrier spacing of the numerology, 15 kHz times a power of 2
        int numPrbs = default(273); // Number of PRBs of the carrier
        bandwidth = default(numPrbs * 12 * subcarrierSpacing);
        modulation = default("QPSK"); // Unused, the MCS of each transport block counts
        energyDetection = default(-70dBm); // Only the busy state of the radio depends on it, above the noise of a 100 MHz carrier
        sensitivity = default(-200dBm);
        snirThreshold = default(-200dB);
        signalAnalogRepresentation = default("dimensional");
        errorModel.typename = default("CellularErrorModel");
        @class(CellularReceiver);
}

