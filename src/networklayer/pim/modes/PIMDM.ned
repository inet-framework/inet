//
// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 3
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
// Authors: Veronika Rybova, Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)


package inet.networklayer.pim.modes;

// Simple module pimDM which is connected to pimSplitter module
simple PIMDM
{
    parameters:
    	@display("i=block/network2");
        string pimInterfaceTableModule;
        string pimNeighborTableModule;

        int designatedRouterPriority = default(-1);

        volatile double triggeredHelloDelay @unit(s) = uniform(0s,5s);
        double helloPeriod @unit(s) = 30s;
        double holdTime @unit(s) = default(3.5 * helloPeriod);
        double pruneInterval @unit(s) = 180s;
        double pruneLimitInterval @unit(s) = 210s;
        double overrideInterval @unit(s) = 2.5s;
        double propagationDelay @unit(s) = 0.5s;
        double graftRetryInterval @unit(s) = 3s;
        double sourceActiveInterval @unit(s) = 210s; // Cisco has 180s, after that, route is flushed
        double stateRefreshInterval @unit(s) = 60s;
        double assertTime @unit(s) = 180s;

        @signal[sentHelloPk](type=cPacket);
        @signal[rcvdHelloPk](type=cPacket);
        @signal[sentGraftPk](type=cPacket);
        @signal[rcvdGraftPk](type=cPacket);
        @signal[sentGraftAckPk](type=cPacket);
        @signal[rcvdGraftAckPk](type=cPacket);
        @signal[sentJoinPrunePk](type=cPacket);
        @signal[rcvdJoinPrunePk](type=cPacket);
        @signal[sentAssertPk](type=cPacket);
        @signal[rcvdAssertPk](type=cPacket);
        @signal[sentStateRefreshPk](type=cPacket);
        @signal[rcvdStateRefreshPk](type=cPacket);

    gates:
        input ipIn;
        output ipOut;
}
