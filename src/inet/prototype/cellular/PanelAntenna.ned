//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.prototype.cellular;

import inet.physicallayer.wireless.common.base.packetlevel.AntennaBase;

//
// A dummy base station panel antenna: a sector element pattern in the horizontal plane (the
// parabolic main lobe of 3GPP TR 38.901 Table 7.3-1, cut at the front-to-back ratio), and
// `numBeams` fixed beams spread over `beamSpan` around the boresight. The element pattern is the
// gain that ~IAntenna reports; a ~CellularTransmitter asks for the gain of the beam of each
// allocation. The radios of one band refer to the same panel through ~SharedAntennaRef.
//
module PanelAntenna extends AntennaBase
{
    parameters:
        double boresight @unit(deg) = default(0deg); // Azimuth of the main lobe, relative to the orientation of the node
        double maxElementGain @unit(dB) = default(8dB); // Gain of the element pattern at the boresight
        double halfPowerBeamwidth @unit(deg) = default(65deg); // 3 dB width of the element pattern
        double frontToBackRatio @unit(dB) = default(30dB); // Attenuation of the element pattern outside the main lobe
        int numBeams = default(0); // Number of fixed beams; 0 means the panel has no beams
        double beamSpan @unit(deg) = default(120deg); // Azimuth range that the beam directions cover
        double beamWidth @unit(deg) = default(15deg); // 3 dB width of one beam
        double maxBeamGain @unit(dB) = default(10dB); // Gain of a beam in its direction, on top of the element pattern
        double sideLobeRatio @unit(dB) = default(20dB); // Attenuation of a beam outside its main lobe
        mobilityModule = default("^.mobility");
        @class(PanelAntenna);
        @display("i=device/antennatower");
}

