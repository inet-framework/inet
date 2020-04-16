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

import inet.physicallayer.contract.packetlevel.IErrorModel;
import inet.physicallayer.contract.packetlevel.IReceiver;

//
// This module servces as a base module for narrowband receiver models.
//
module NarrowbandReceiverBase extends SnirReceiverBase like IReceiver
{
    parameters:
        double energyDetection @unit(dBm); // no signal is detected at all below this reception power threshold (idle state)
        double sensitivity @unit(dBm);     // reception is not possible if the signal power is below sensitivity (idle or busy states)
        double centerFrequency @unit(Hz); // center frequency of the band where this receiver listens on the medium
        double bandwidth @unit(Hz);        // bandwidth of the band where this receiver listens on the medium
        string modulation;                 // one of "BPSK", "16-QAM", "256-QAM"
        @display("i=block/wrx");

    submodules:
        errorModel: <> like IErrorModel if typename != "" {
            @display("p=100,100");
        }
}

