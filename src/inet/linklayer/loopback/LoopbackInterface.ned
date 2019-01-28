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
// Author: Zoltan Bojthe
//

package inet.linklayer.loopback;

import inet.linklayer.contract.ILoopbackInterface;
import inet.networklayer.common.InterfaceEntry;

//
// Loopback interface. Complements the ~Loopback module.
//
module LoopbackInterface extends InterfaceEntry like ILoopbackInterface
{
    parameters:
        @class(InterfaceEntry);
        @display("i=block/ifcard;bgl=53");
        string interfaceTableModule;
        *.interfaceTableModule = default(absPath(interfaceTableModule));
    gates:
        input upperLayerIn;
        output upperLayerOut;
    submodules:
        lo: Loopback {
            parameters:
                @display("p=250,200");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> lo.upperLayerIn;
        lo.upperLayerOut --> { @display("m=n"); } --> upperLayerOut;
}

