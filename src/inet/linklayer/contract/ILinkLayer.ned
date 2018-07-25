//
// Copyright (C) 2013 Opensim Ltd.
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

package inet.linklayer.contract;

//
// This module interface provides an abstraction for the upper interface of
// different link layers.
//
moduleinterface ILinkLayerUpper
{
    parameters:
        @display("i=block/rxtx");
    gates:
        input upperLayerIn @labels(INetworkHeader/down);
        output upperLayerOut @labels(INetworkHeader/up);
}

//
// This module interface provides an abstraction for the lower interface of
// different link layers.
//
moduleinterface ILinkLayerLower
{
    parameters:
        @display("i=block/rxtx");
    gates:
        input lowerLayerIn @labels(ILinkFrame/up);
        output lowerLayerOut @labels(ILinkFrame/down);
}

//
// This module interface provides an abstraction for both upper and lower
// interfaces of different link layers.
//
moduleinterface ILinkLayer extends ILinkLayerUpper, ILinkLayerLower
{
    parameters:
        @display("i=block/rxtx");
}

