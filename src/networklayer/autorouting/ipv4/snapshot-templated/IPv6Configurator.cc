//
// Copyright (C) 2011 Opensim Ltd
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

#include "IPv6Configurator.h"
#include "IPv6InterfaceData.h"
#include "IPConfigurator.cc"

Define_Module(IPv6Configurator);

void IPv6Configurator::initialize(int stage)
{
    if (stage==2) //TODO parameter: melyik stage-ben csinal a cimkonfiguralast, es melyikben a route-okat
    {
        Topology topology("topology");
        NetworkInfo networkInfo;

        // extract topology into the Topology object, then fill in a LinkInfo[] vector
        extractTopology(topology, networkInfo);

        // assign addresses to IPv4 nodes
        assignAddresses(topology, networkInfo);

        // dump the result if requested
        if (par("dumpAddresses").boolValue()) {
            dumpAddresses(networkInfo);
        }
    }
}

void IPv6Configurator::assignAddress(InterfaceEntry *interfaceEntry, Uint128 address, Uint128 netmask)
{
    IPv6InterfaceData *interfaceData = interfaceEntry->ipv6Data();
    interfaceData->assignAddress(address, false, 0, 0);
}
