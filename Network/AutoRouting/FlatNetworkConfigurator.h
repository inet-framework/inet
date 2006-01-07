//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef __FLATNETWORKCONFIGURATOR_H__
#define __FLATNETWORKCONFIGURATOR_H__

#include <omnetpp.h>
#include "INETDefs.h"


/**
 * Configures IP addresses and routing tables for a "flat" network,
 * "flat" meaning that all hosts and routers will have the same
 * network address.
 *
 * For more info please see the NED file.
 */
class INET_API FlatNetworkConfigurator : public cSimpleModule
{
  public:
    typedef std::vector<std::string> StringVector;
  protected:
    virtual int numInitStages() const  {return 3;}
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);

    void extractTopology(cTopology& topo, const StringVector& types);
    void assignAddresses(std::vector<uint32>& nodeAddresses, cTopology& topo, const StringVector& nonIPTypes);
    void addDefaultRoutes(std::vector<bool>& usesDefaultRoute, cTopology& topo, const std::vector<uint32>& nodeAddresses, const StringVector& nonIPTypes);
    void fillRoutingTables(const std::vector<bool>& usesDefaultRoute, cTopology& topo, const std::vector<uint32>& nodeAddresses, const StringVector& nonIPTypes);

    void setDisplayString(cTopology& topo, const StringVector& nonIPTypes);
    bool isNonIPType(cTopology::Node *node, const StringVector& nonIPTypes);
};

#endif

