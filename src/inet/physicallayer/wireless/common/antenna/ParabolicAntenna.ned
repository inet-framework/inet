//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.antenna;

import inet.physicallayer.wireless.common.base.packetlevel.AntennaBase;

//
// This model is based on a parabolic approximation of the main lobe radiation
// pattern. A similar model appears in
//
// George Calcev and Matt Dillon, "Antenna Tilt Control in CDMA Networks"
// in Proc. of the 2nd Annual International Wireless Internet Conference (WICON), 2006
//
// though the latter addresses also the elevation plane, which the present model doesn't.
//
module ParabolicAntenna extends AntennaBase
{
    parameters:
        double maxGain @unit(dB);    // maximum gain of the antenna radiation pattern
        double minGain @unit(dB);    // minimum gain of the antenna radiation pattern
        double beamWidth @unit(deg); // 3dB beam width
        @class(ParabolicAntenna);
}
