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
// Calculates delay for incoming packets based on src and dest interfaces
// and send packet with calculated delay. The src info was set by TaggerChannel
// between INICs and NetworkLayer.
// Should use TaggerChannel for connect interfaces for network layer.
//
// @see InternetCloudNetwokLayer, InternetCloud, InternetCloudNetworkConfigurator
//
moduleinterface ICloudDelayer
{
    parameters:
        @display("i=block/delay");
    gates:
        input ipIn;
        output ipOut;
}

module CloudNopDelayer like ICloudDelayer
{
    gates:
        input ipIn;
        output ipOut;
    connections:
        ipIn --> ipOut;
}
