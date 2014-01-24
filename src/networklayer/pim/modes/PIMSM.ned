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

//
// TODO
//
simple PIMSM
{
    parameters:
    	@display("i=block/network2");
        string pimInterfaceTableModule;
        string pimNeighborTableModule;

        volatile double triggeredHelloDelay @unit(s) = uniform(0s,5s);
        double helloPeriod @unit(s) = 30s;
        double joinPrunePeriod @unit(s) = default(60s); // period between Join/Prune messages (called t_periodic in the RFC)
        double defaultOverrideInterval @unit(s) = 2.5s;
        double defaultPropagationDelay @unit(s) = 0.5s;
        double keepAlivePeriod @unit(s) = 210s;
        double rpKeepAlivePeriod @unit(s) = 3 * registerSuppressionTime + registerProbeTime;
        double registerSuppressionTime @unit(s) = 60s;
        double registerProbeTime @unit(s) = 5s;

        string RP = default("");
        string sptThreshold = default("infinity");
    gates:
        input ipIn;
        output ipOut;
}
