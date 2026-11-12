//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.prototype.cellular;

import inet.physicallayer.wireless.common.base.packetlevel.TransmitterBase;
import inet.physicallayer.wireless.common.contract.packetlevel.ITransmitter;

//
// Transmits one allocation per packet on one component carrier. The packet's ~AllocationReq
// tag gives the PRBs, the number of OFDM symbols, the power per PRB, the beam and the intended
// receiver. The transmission's power spectral density is a lattice with one cell per (symbol,
// PRB) of the carrier, zero outside the allocation, so it is a regular dimensional signal that
// interferes with every other dimensional signal on the medium.
//
// The transmitter builds the analog model itself, so it has no `analogModel` submodule:
// ~ITransmitterAnalogModel gets no packet, and so cannot see the allocation (change C15).
//
module CellularTransmitter extends TransmitterBase like ITransmitter
{
    parameters:
        double centerFrequency @unit(Hz); // Center frequency of the carrier
        double subcarrierSpacing @unit(Hz) = default(30kHz); // Subcarrier spacing of the numerology, 15 kHz times a power of 2
        int numPrbs = default(273); // Number of PRBs of the carrier
        double power @unit(W); // Maximum total transmission power on the carrier, which the MAC divides among the PRBs
        signalAnalogRepresentation = default("dimensional");
        analogModel.typename = "";
        @class(CellularTransmitter);
}

