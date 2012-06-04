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

#include "INETDefs.h"

#include "IInterfaceTable.h"
#include "IRoutingTable.h"
#include "OSPFPacket_m.h"
#include "OSPFRouter.h"


/**
 * OMNeT++ module class acting as a facade for the OSPF data structure.
 * Handles the configuration loading and forwards the OMNeT++ messages (OSPF packets).
 */
class OSPFRouting :  public cSimpleModule
{
  private:
    IInterfaceTable* ift;        ///< Provides access to the interface table.
    IRoutingTable*   rt;         ///< Provides access to the IP routing table.
    OSPF::Router*    ospfRouter; ///< Root object of the OSPF data structure.

    int getIntAttrOrPar(const cXMLElement& ifConfig, const char *name) const;
    bool getBoolAttrOrPar(const cXMLElement& ifConfig, const char *name) const;
    const char *getStrAttrOrPar(const cXMLElement& ifConfig, const char *name) const;

    /**
     * Looks up the interface name in IInterfaceTable, and returns interfaceId a.k.a ifIndex.
     */
    int   resolveInterfaceName(const std::string& name) const;

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
    void  getAreaListFromXML(const cXMLElement& routerNode, std::set<OSPF::AreaID>& areaList) const;

    /**
     * Loads basic configuration information for a given area from the config XML.
     * Reads the configured address ranges, and whether this Area should be handled as a stub Area.
     * @param asConfig [in] XML node describing the configuration of the whole Autonomous System.
     * @param areaID   [in] The Area to be added to the OSPF data structure.
     */
    void  loadAreaFromXML(const cXMLElement& asConfig, OSPF::AreaID areaID);

    /**
     * Loads authenticationType and authenticationKey attibutes for a router interface
     */
    void loadAuthenticationConfig(OSPF::Interface* intf, const cXMLElement& ifConfig);

    /**
     * Loads OSPF configuration information for a router interface.
     * Handles POINTTOPOINT, BROADCAST, NBMA and POINTTOMULTIPOINT interfaces.
     * @param ifConfig [in] XML node describing the configuration of an OSPF interface.
     */
    void  loadInterfaceParameters(const cXMLElement& ifConfig);

    /**
     * Loads the configuration information of a route outside of the Autonomous System(external route).
     * @param externalRouteConfig [in] XML node describing the parameters of an external route.
     */
    void  loadExternalRoute(const cXMLElement& externalRouteConfig);

    /**
     * Loads the configuration of a host getRoute(a host directly connected to the router).
     * @param hostRouteConfig [in] XML node describing the parameters of a host route.
     */
    void  loadHostRoute(const cXMLElement& hostRouteConfig);

    /**
     * Loads the configuration of an OSPf virtual link(virtual connection between two backbone routers).
     * @param virtualLinkConfig [in] XML node describing the parameters of a virtual link.
     */
    void  loadVirtualLink(const cXMLElement& virtualLinkConfig);

    /**
     * Loads the configuration of the OSPF data structure from the config XML.
     * @param filename [in] The path of the XML config file.
     * @return True if the configuration was succesfully loaded.
     * @throws an getError() otherwise.
     */
    bool  loadConfigFromXML(cXMLElement *asConfig);

    void joinMulticastGroups(int interfaceId);

  public:
    OSPFRouting();
    virtual ~OSPFRouting();

    /**
     * Insert a route learn by BGP in OSPF routingTable as an external route.
     * Used by the BGPRouting module.
     * @ifIndex: interface ID
     */
    void insertExternalRoute(int ifIndex, const OSPF::IPv4AddressRange& netAddr);

    /**
     * Return true if the route is in OSPF external LSA Table, false else.
     * Used by the BGPRouting module.
     */
    bool checkExternalRoute(const IPv4Address& route);

  protected:
    virtual int numInitStages() const  {return 5;}

    /**
     * OMNeT++ init method.
     * Runs at stage 4, after interfaces are registered(stage 0), the routing
     * table is initialized(stage 1), and routerId gets assigned(stage 3).
     * Loads OSPF configuration information from the config XML.
     * @param stage [in] The initialization stage.
     */
    virtual void initialize(int stage);

    /**
     * Forwards OSPF messages to the message handler object of the OSPF data structure.
     * @param msg [in] The OSPF message.
     */
    virtual void handleMessage(cMessage *msg);
};

#endif  // __INET_OSPFROUTING_H


