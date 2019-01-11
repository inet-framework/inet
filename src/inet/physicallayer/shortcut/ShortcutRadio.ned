//
// Copyright (C) OpenSim Ltd.
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

package inet.physicallayer.shortcut;

import inet.physicallayer.base.packetlevel.PhysicalLayerBase;
import inet.physicallayer.contract.packetlevel.IRadio;

//
// This module implements a simple shortcut to peer radio protocol that completely
// bypasses the physical medium. This radio module directly sends packets to the
// other radio module without any physical layer processing in the radio medium.
// Packets received from the upper layer protocols may be lost. Physical layer
// overhead is simply simulated by physical header bits, preamble transmission
// duration and a propagation delay.
//
simple ShortcutRadio extends PhysicalLayerBase like IRadio
{
    parameters:
        string interfaceTableModule;
        double bitrate @unit(bps);
        volatile int lengthOverhead @unit(b) = default(0b); // extra length added to packets
        volatile double durationOverhead @unit(s) = default(0s); // extra transmission duration added to packets
        volatile double propagationDelay @unit(s) = default(0s); // propagation time to all destinations
        volatile double packetLoss = default(0); // packet loss probability
        @class(ShortcutRadio);
        @signal[transmissionStateChanged](type=long);    // type=inet::physicallayer::TransmissionState
}

