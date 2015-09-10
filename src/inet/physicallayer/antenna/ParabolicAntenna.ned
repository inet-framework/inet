//
// Copyright (C) 2013 OpenSim Ltd
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.physicallayer.antenna;

import inet.physicallayer.base.packetlevel.AntennaBase;

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
