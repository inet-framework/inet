//
// Copyright (C) 2005 Eric Wu
// Copyright (C) 2004 Andras Varga
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

#ifndef __INET_FLATNETWORKCONFIGURATOR6_H
#define __INET_FLATNETWORKCONFIGURATOR6_H

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * Configures IPv6 addresses and routing tables for a "flat" network,
 * "flat" meaning that all hosts and routers will have the same
 * network address.
 *
 * For more info please see the NED file.
 */
class INET_API FlatNetworkConfigurator6 : public cSimpleModule
{
  protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);

    virtual void configureAdvPrefixes(cTopology& topo);
    virtual void addOwnAdvPrefixRoutes(cTopology& topo);
    virtual void addStaticRoutes(cTopology& topo);

    virtual void setDisplayString(int numIPNodes, int numNonIPNodes);
    virtual bool isIPNode(cTopology::Node *node);
};

} // namespace inet

#endif // ifndef __INET_FLATNETWORKCONFIGURATOR6_H

