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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

package inet.queueing.gate;

import inet.queueing.base.PacketGateBase;
import inet.queueing.contract.IPacketGate;

//
// This module allows or forbids packets to pass through depending on whether
// the gate is open or closed.
//
simple PacketGate extends PacketGateBase like IPacketGate
{
    parameters:
        bool initiallyOpen = default(false);
        double openTime @unit(s);
        double closeTime @unit(s);
        @class(PacketGate);
}
