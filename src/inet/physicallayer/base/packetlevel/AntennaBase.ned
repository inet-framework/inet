//
// Copyright (C) 2013 OpenSim Ltd.
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

package inet.physicallayer.base.packetlevel;

import inet.mobility.contract.IMobility;
import inet.physicallayer.contract.packetlevel.IAntenna;

//
// This module servces as the base module for antenna models.
//
module AntennaBase like IAntenna
{
    parameters:
        string mobilityModule; // the path to the mobility module relative to this module, only used if no mobility submodule is present
        int numAntennas = default(1); // the number of antennas in the antenna array
        mobility.subjectModule = default(""); // disable subject module of mobility submodule by default 
        @display("i=device/antennatower");
    submodules:
        mobility: <default("")> like IMobility if typename != "" {
            parameters:
                @display("p=100,100;is=s");
        }
}

