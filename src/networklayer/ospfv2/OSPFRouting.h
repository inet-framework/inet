//
// Copyright (C) 2006 Andras Babos and Andras Varga
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

#ifndef __INET_OSPFROUTING_H
#define __INET_OSPFROUTING_H

#include <vector>
#include <omnetpp.h>
#include "IRoutingTable.h"
#include "IInterfaceTable.h"
#include "OSPFPacket_m.h"
#include "OSPFRouter.h"

/**
 * OMNeT++ module class acting as a facade for the OSPF datastructure.
 * Handles the configuration loading and forwards the OMNeT++ messages(OSPF packets).
 */
class OSPFRouting :  public cSimpleModule
{
  private:
    IInterfaceTable*     ift;        ///< Provides access to the interface table.
    IRoutingTable*       rt;         ///< Provides access to the IP routing table.
    OSPF::Router*       ospfRouter; ///< Root object of the OSPF datastructure.

    int     ResolveInterfaceName(const std::string& name) const;
    void    GetAreaListFromXML(const cXMLElement& routerNode, std::map<std::string, int>& areaList) const;
    void    LoadAreaFromXML(const cXMLElement& asConfig, const std::string& areaID);
    void    LoadInterfaceParameters(const cXMLElement& ifConfig);
    void    LoadExternalRoute(const cXMLElement& externalRouteConfig);
    void    LoadHostRoute(const cXMLElement& hostRouteConfig);
    void    LoadVirtualLink(const cXMLElement& virtualLinkConfig);

    bool    LoadConfigFromXML(const char * filename);

  public:
    OSPFRouting();
    virtual ~OSPFRouting(void);

  protected:
    virtual int numInitStages() const  {return 5;}
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
};

#endif  // __INET_OSPFROUTING_H


