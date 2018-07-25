//
// Copyright (C) 2018 OpenSim Ltd.
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
// Author: Zoltan Bojthe
//

package inet.emulation.common.tap;

//
// Tap bridge for connect real tap interface to simulation
//
// TapBridge write the packets arrived on 'toTap' gate to the specified
// real tap device, and send out packets arrived from real tap device
// on 'fromTap' gate.
//
simple TapBridge
{
    parameters:
        string device;  // name of existing real tap interface
        @display("i=block/rxtx");
    gates:
        input toTap @labels(EtherFrame);
        output fromTap @labels(EtherFrame);
}

