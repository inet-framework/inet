//
// Copyright (C) 2012 OpenSim Ltd.
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
// @author Zoltan Bojthe
//

package inet.networklayer.internetcloud;

//
// Delay module interface for InternetCloudNetworkLayer
//
// Delays and/or drops the packets based on what interface card 
// the packet has arrived on and on what interface it is leaving the cloud. 
//
// @see InternetCloudNetwokLayer, InternetCloud
moduleinterface ICloudDelayer
{
    parameters:
        @display("i=block/delay");
    gates:
        input ipIn;
        output ipOut;
}
