//
// Copyright (C) 2005 Christian Dankbar, Irene Ruengeler, Michael Tuexen
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.linklayer.ext;

import inet.linklayer.contract.IExternalNic;

//
// Implements an interface that corresponds to a real interface
// on the host running the simulation. Suitable for hardware-in-the-loop
// simulations.
// 
// Requires cSocketRTScheduler to be configured as scheduler in omnetpp.ini.
//
simple ExtInterface like IExternalNic
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string filterString;
        string device;
        int mtu @unit("B") = default(1500B);
    gates:
        input upperLayerIn;
        output upperLayerOut;
}


