//
// Copyright (C) 2010 Philipp Berndt
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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


package inet.networklayer.internetcloud;

//
// Delay module for InternetCloudNetworkLayer
//
// Calculates delay for incoming IPv4Datagram packets based on src and dest addresses
// and send packet with calculated delay.
//
// All non-IPv4 traffic passes through undelayed.
//
// @see InternetCloudNetwokLayer, InternetCloud, InternetCloudNetworkConfigurator
//
simple GnpDelayGenerator
{
    parameters:
        bool usePingErRttData = default(false);     // set GnpLatencyModel::usePingErInsteadOfGnp
        bool useAnalyticalRtt = default(false);     // set GnpLatencyModel::useAnalyticalFunctionInsteadOfGnp
        bool usePingErJitter = default(false);     // set GnpLatencyModel::usePingErJitter
        bool usePingErPacketLoss = default(false);     // set GnpLatencyModel::usePingErPacketLoss
        bool useAccessLatency = default(false);     // set GnpLatencyModel::useAccessLatency
        @display("i=block/delay");
    gates:
        input ipIn;
        output ipOut;
}

