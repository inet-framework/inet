//
// Copyright (C) 2010 Philipp Berndt
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

package inet.world.inet;

//
// Delays \IPDatagrams based on their source and destination address
//
simple InternetPropagationDelayGate
{
    parameters:
        bool usePingErRttData    = default(false);
        bool useAnalyticalRtt    = default(false);
        bool usePingErJitter     = default(false);
        bool usePingErPacketLoss = default(false);
        bool useAccessLatency    = default(false);
        @display("i=block/delay");
    gates:
        input  ipIn;
        output ipOut;
}

