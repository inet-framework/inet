//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.prototype.cellular;

import inet.physicallayer.wireless.common.medium.RadioMedium;

//
// One radio medium for all carriers of all cells. The carrier is a property of each
// transmission and each listening, so carriers of different numerologies, and other
// dimensional INET signals, share the medium and interfere where their spectra overlap.
//
// The listening filter delivers each allocation only to its intended receiver (see
// ~CellularReceiver); every other transmission counts only as interference. The check for
// transmissions that start at the same time is off, because every cell starts its allocations
// at the same symbol boundaries. The noise figure of the receivers is part of the background
// noise power spectral density, because receivers have no noise figure (change C6).
//
module CellularRadioMedium extends RadioMedium
{
    parameters:
        signalAnalogRepresentation = default("dimensional");
        listeningFilter = default(true);
        sameTransmissionStartTimeCheck = default("ignore");
        transmissionPurgeMode = default("onTransmission");
        pathLoss.typename = default("FreeSpacePathLoss");
        // thermal noise of -174 dBm/Hz plus a 7 dB noise figure
        backgroundNoise.powerSpectralDensity = default(-107dBmWpMHz);
}
