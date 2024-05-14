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

package inet.physicallayer.wireless.noise;

import inet.physicallayer.wireless.common.base.packetlevel.TransmitterBase;
import inet.physicallayer.wireless.common.contract.packetlevel.ITransmitter;

module NoiseTransmitter extends TransmitterBase like ITransmitter
{
    parameters:
        volatile double duration @unit(s);
        volatile double centerFrequency @unit(Hz) = default(nan Hz);
        volatile double bandwidth @unit(Hz) = default(0Hz);
        volatile double power @unit(W);
        int gainFunctionCacheLimit = default(0); // Limits the number of gain functions cached for a given duration, centerFrequency, and bandwidth
        @class(NoiseTransmitter);
        @display("i=block/wtx");
}

