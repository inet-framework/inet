//
// Copyright (C) OpenSim Ltd
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

package inet.physicallayer.common.packetlevel;

import inet.mobility.contract.IMobility;
import inet.physicallayer.contract.packetlevel.IAntenna;
import inet.physicallayer.contract.packetlevel.IRadio;
import inet.physicallayer.contract.packetlevel.ITransmitter;

module NoiseSource like IRadio
{
    parameters:
        string radioMediumModule = default("radioMedium"); // module path of the medium module where this radio communicates
        volatile double sleepInterval @unit(s);
        *.mobilityModule = default(absPath(".mobility"));
        @networkNode;
        @class(NoiseSource);
        @display("i=block/wtx");
    gates:
        input upperLayerIn @labels(ILinkLayerFrame/down) @loose;
        output upperLayerOut @labels(ILinkLayerFrame/up) @loose;
        input radioIn @labels(ISignal) @loose;
    submodules:
        mobility: <default("StationaryMobility")> like IMobility {
            parameters:
                @display("p=100,100");
        }
        antenna: <default("IsotropicAntenna")> like IAntenna {
            parameters:
                @display("p=100,200");
        }
        transmitter: <default("NoiseDimensionalTransmitter")> like ITransmitter {
            parameters:
                @display("p=100,300");
        }
}

