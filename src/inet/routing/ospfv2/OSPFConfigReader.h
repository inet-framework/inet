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

#ifndef __INET_OSPFCONFIGREADER_H
#define __INET_OSPFCONFIGREADER_H

#include <vector>

#include "inet/common/INETDefs.h"

#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIPv4RoutingTable.h"
#include "inet/routing/ospfv2/OSPFPacket_m.h"
#include "inet/routing/ospfv2/router/OSPFRouter.h"

namespace inet {

namespace ospf {

/**
 * Configuration reader for the OSPF module.
 */
class INET_API OSPFConfigReader
{
  private:
    cModule *ospfModule = nullptr;
    IInterfaceTable *ift = nullptr;    // provides access to the interface table
    Router *ospfRouter = nullptr;    // data structure to fill in

  private:
    cPar& par(const char *name) const { return ospfModule->par(name); }
    int getIntAttrOrPar(const cXMLElement& ifConfig, const char *name) const;
    bool getBoolAttrOrPar(const cXMLElement& ifConfig, const char *name) const;
    const char *getStrAttrOrPar(const cXMLElement& ifConfig, const char *name) const;

    /**
     * Looks up the interface name in IInterfaceTable, and returns interfaceId a.k.a ifIndex.
     */
    int resolveInterfaceName(const std::string& name) const;

    /**
     * Search an InterfaceEntry in IInterfaceTable by interface name or toward module name
     * an returns the InterfaceEntry pointer or throws an error.
     */
    InterfaceEntry *getInterfaceByXMLAttributesOf(const cXMLElement& ifConfig);

    /**
     * Loads a list of OSPF Areas connected to this router from the config XML.
     * @param routerNode [in]  XML node describing this router.
     * @param areaList   [out] A set of OSPF Areas connected to this router.
     */
    void getAreaListFromXML(const cXMLElement& routerNode, std::set<AreaID>& areaList) const;

    /**
     * Loads basic configuration information for a given area from the config XML.
     * Reads the configured address ranges, and whether this Area should be handled as a stub Area.
     */
    void loadAreaFromXML(const cXMLElement& asConfig, AreaID areaID);

    /**
     * Loads authenticationType and authenticationKey attributes for a router interface
     */
    void loadAuthenticationConfig(Interface *intf, const cXMLElement& ifConfig);

    /**
     * Loads OSPF configuration information for a router interface.
     * Handles POINTTOPOINT, BROADCAST, NBMA and POINTTOMULTIPOINT interfaces.
     */
    void loadInterfaceParameters(const cXMLElement& ifConfig);

    /**
     * Loads the configuration information of a route outside of the Autonomous System (external route).
     */
    void loadExternalRoute(const cXMLElement& externalRouteConfig);

    /**
     * Loads the configuration of a host route (a host directly connected to the router).
     */
    void loadHostRoute(const cXMLElement& hostRouteConfig);

    /**
     * Loads the configuration of an OSPf virtual link (virtual connection between two backbone routers).
     */
    void loadVirtualLink(const cXMLElement& virtualLinkConfig);

    void joinMulticastGroups(int interfaceId);

  public:
    OSPFConfigReader(cModule *ospfModule, IInterfaceTable *ift);
    virtual ~OSPFConfigReader();

    /**
     * Loads the configuration of the OSPF data structure from the config XML.
     * Returns true if the configuration was successfully loaded.
     */
    bool loadConfigFromXML(cXMLElement *asConfig, Router *ospfRouter);
};

} // namespace ospf

} // namespace inet

#endif // ifndef __INET_OSPFCONFIGREADER_H

