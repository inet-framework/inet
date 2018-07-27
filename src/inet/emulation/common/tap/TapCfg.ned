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
// TapCfg configures the simulated interface based on real tap interface 
// in initialize phase.
// Should be inserted to inside of an InterfaceEntry module
//
simple TapCfg
{
    parameters:
        string device;  // name of existing real tap interface
        @display("i=block/rxtx");
}

