//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_OSPFV2CONFIGREADER_H
#define __INET_OSPFV2CONFIGREADER_H

#include <vector>

#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/routing/ospfv2/Ospfv2Packet_m.h"
#include "inet/routing/ospfv2/router/Ospfv2Router.h"

namespace inet {

namespace ospfv2 {

/**
 * Configuration reader for the OSPF module.
 */
class INET_API Ospfv2ConfigReader
{
  private:
    cModule *ospfModule = nullptr;
    IInterfaceTable *ift = nullptr; // provides access to the interface table
    Router *ospfRouter = nullptr; // data structure to fill in

  private:
    cPar& par(const char *name) const { return ospfModule->par(name); }
    int getIntAttrOrPar(const cXMLElement& ifConfig, const char *name) const;
    bool getBoolAttrOrPar(const cXMLElement& ifConfig, const char *name) const;
    const char *getStrAttrOrPar(const cXMLElement& ifConfig, const char *name) const;

    /**
     * Search an NetworkInterface in IInterfaceTable by interface name or toward module name
     * an returns the NetworkInterface pointer or throws an error.
     */
    std::vector<NetworkInterface *> getInterfaceByXMLAttributesOf(const cXMLElement& ifConfig);

    cXMLElement *findMatchingConfig(const cXMLElementList& routerConfig, const NetworkInterface& intf);

    /**
     * Loads a list of OSPF Areas connected to this router from the config XML.
     * @param routerNode [in]  XML node describing this router.
     * @param areaList   [out] A set of OSPF Areas connected to this router.
     */
    void getAreaListFromXML(const cXMLElement& routerNode, std::set<AreaId>& areaList) const;

    /**
     * Loads basic configuration information for a given area from the config XML.
     * Reads the configured address ranges, and whether this Area should be handled as a stub Area.
     */
    void loadAreaFromXML(const cXMLElement& asConfig, AreaId areaID);

    /**
     * Loads authenticationType and authenticationKey attributes for a router interface
     */
    void loadAuthenticationConfig(Ospfv2Interface *intf, const cXMLElement& ifConfig);

    /**
     * Loads OSPF configuration information for a router interface.
     * Handles POINTTOPOINT, BROADCAST, NBMA and POINTTOMULTIPOINT interfaces.
     */
    void loadInterfaceParameters(const cXMLElement& ifConfig, NetworkInterface& intf);

    /**
     * Loads the configuration information of a route outside of the Autonomous System (external route).
     */
    void loadExternalRoute(const cXMLElement& externalRouteConfig);

    /**
     * Loads the configuration of a host route (a host directly connected to the router).
     */
    void loadHostRoute(const cXMLElement& hostRouteConfig);

    /**
     * Loads the configuration of a loopback interface.
     */
    void loadLoopbackParameters(const cXMLElement& loConfig, NetworkInterface& intf);

    /**
     * Loads the configuration of an OSPf virtual link (virtual connection between two backbone routers).
     */
    void loadVirtualLink(const cXMLElement& virtualLinkConfig, cXMLElement& asConfig);

    /**
     * Distributes the configure default route on this router with AS-External LSAs to other OSPF routers.
     */
    void initiateDefaultRouteDistribution();

    void joinMulticastGroups(int interfaceId);

  public:
    Ospfv2ConfigReader(cModule *ospfModule, IInterfaceTable *ift);
    virtual ~Ospfv2ConfigReader();

    /**
     * Loads the configuration of the OSPF data structure from the config XML.
     * Returns true if the configuration was successfully loaded.
     */
    bool loadConfigFromXML(cXMLElement *asConfig, Router *ospfRouter);
};

} // namespace ospfv2

} // namespace inet

#endif

