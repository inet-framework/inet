//
// Copyright (C) 2006 Andras Babos and Andras Varga
// Copyright (C) 2012 OpenSim Ltd.
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

#include <string>
#include <map>
#include <stdlib.h>
#include <memory.h>

#include "OSPFRouting.h"

#include "InterfaceTableAccess.h"
#include "IPv4Address.h"
#include "IPv4ControlInfo.h"
#include "IPv4InterfaceData.h"
#include "IPvXAddressResolver.h"
#include "MessageHandler.h"
#include "OSPFArea.h"
#include "OSPFcommon.h"
#include "OSPFInterface.h"
#include "PatternMatcher.h"
#include "RoutingTableAccess.h"
#include "XMLUtils.h"


Define_Module(OSPFRouting);


OSPFRouting::OSPFRouting()
{
    ospfRouter = NULL;
}

OSPFRouting::~OSPFRouting()
{
    delete ospfRouter;
}


void OSPFRouting::initialize(int stage)
{
    // we have to wait for stage 2 until interfaces get registered(stage 0)
    // and routerId gets assigned(stage 3)
    if (stage == 4)
    {
        rt = RoutingTableAccess().get();
        ift = InterfaceTableAccess().get();

        // Get routerId
        ospfRouter = new OSPF::Router(rt->getRouterId(), this);

        // read the OSPF AS configuration
        cXMLElement *ospfConfig = par("ospfConfig").xmlValue();
        if (!loadConfigFromXML(ospfConfig))
            error("Error reading AS configuration from %s", ospfConfig->getSourceLocation());

        ospfRouter->addWatches();
    }
}


void OSPFRouting::handleMessage(cMessage *msg)
{
    ospfRouter->getMessageHandler()->messageReceived(msg);
}


void OSPFRouting::insertExternalRoute(int ifIndex, const OSPF::IPv4AddressRange &netAddr)
{
    simulation.setContext(this);
    OSPFASExternalLSAContents newExternalContents;
    newExternalContents.setRouteCost(OSPF_BGP_DEFAULT_COST);
    newExternalContents.setExternalRouteTag(OSPF_EXTERNAL_ROUTES_LEARNED_BY_BGP);
    const IPv4Address netmask = netAddr.mask;
    newExternalContents.setNetworkMask(netmask);
    ospfRouter->updateExternalRoute(netAddr.address, newExternalContents, ifIndex);
}


bool OSPFRouting::checkExternalRoute(const IPv4Address &route)
{
    for (unsigned long i=1; i < ospfRouter->getASExternalLSACount(); i++)
    {
        OSPF::ASExternalLSA* externalLSA = ospfRouter->getASExternalLSA(i);
        IPv4Address externalAddr = externalLSA->getHeader().getLinkStateID();
        if (externalAddr == route) //FIXME was this meant???
            return true;
    }
    return false;
}

InterfaceEntry *OSPFRouting::getInterfaceByXMLAttributesOf(const cXMLElement& ifConfig)
{
    const char *ifName = ifConfig.getAttribute("ifName");
    if (ifName && *ifName) {
        InterfaceEntry* ie = ift->getInterfaceByName(ifName);
        if (!ie)
            throw cRuntimeError("Error reading XML config: IInterfaceTable contains no interface named '%s' at %s", ifName, ifConfig.getSourceLocation());
        return ie;
    }

    const char *toward = getRequiredAttribute(ifConfig, "toward");
    cModule *destnode = simulation.getModuleByPath(toward);
    if (!destnode)
        error("toward module `%s' not found at %s", toward, ifConfig.getSourceLocation());

    cModule *host = ift->getHostModule();
    for (int i=0; i < ift->getNumInterfaces(); i++)
    {
        InterfaceEntry *ie = ift->getInterface(i);
        if (ie)
        {
            int gateId = ie->getNodeOutputGateId();
            if ((gateId != -1) && (host->gate(gateId)->pathContains(destnode)))
                return ie;
        }
    }
    throw cRuntimeError("Error reading XML config: IInterfaceTable contains no interface toward '%s' at %s", toward, ifConfig.getSourceLocation());
}

int OSPFRouting::resolveInterfaceName(const std::string& name) const
{
    InterfaceEntry* ie = ift->getInterfaceByName(name.c_str());
    if (!ie)
        throw cRuntimeError("Error reading XML config: IInterfaceTable contains no interface named '%s'", name.c_str());

    return ie->getInterfaceId();
}


void OSPFRouting::getAreaListFromXML(const cXMLElement& routerNode, std::set<OSPF::AreaID>& areaList) const
{
    cXMLElementList routerConfig = routerNode.getChildren();
    for (cXMLElementList::iterator routerConfigIt = routerConfig.begin(); routerConfigIt != routerConfig.end(); routerConfigIt++) {
        std::string nodeName = (*routerConfigIt)->getTagName();
        if ((nodeName == "PointToPointInterface") ||
            (nodeName == "BroadcastInterface") ||
            (nodeName == "NBMAInterface") ||
            (nodeName == "PointToMultiPointInterface"))
        {
            OSPF::AreaID areaID = IPv4Address(getStrAttrOrPar(**routerConfigIt, "areaID"));
            if (areaList.find(areaID) == areaList.end())
                areaList.insert(areaID);
        }
    }
}


void OSPFRouting::loadAreaFromXML(const cXMLElement& asConfig, OSPF::AreaID areaID)
{
    std::string areaXPath("Area[@id='");
    areaXPath += areaID.str(false);
    areaXPath += "']";

    cXMLElement* areaConfig = asConfig.getElementByPath(areaXPath.c_str());
    if (areaConfig == NULL) {
        error("No configuration for Area ID: %s at %s", areaID.str(false).c_str(), asConfig.getSourceLocation());
    }
    else {
        EV << "    loading info for Area id = " << areaID.str(false) << "\n";
    }

    OSPF::Area* area = new OSPF::Area(areaID);
    cXMLElementList areaDetails = areaConfig->getChildren();
    for (cXMLElementList::iterator arIt = areaDetails.begin(); arIt != areaDetails.end(); arIt++) {
        std::string nodeName = (*arIt)->getTagName();
        if (nodeName == "AddressRange") {
            OSPF::IPv4AddressRange addressRange;
            addressRange.address = ipv4AddressFromAddressString(getRequiredAttribute(**arIt, "address"));
            addressRange.mask = ipv4NetmaskFromAddressString(getRequiredAttribute(**arIt, "mask"));
            addressRange.address = addressRange.address & addressRange.mask;
            std::string status = getRequiredAttribute(**arIt, "status");
            area->addAddressRange(addressRange, status == "Advertise");
        }
        else if (nodeName == "Stub") {
            if (areaID == OSPF::BACKBONE_AREAID)
                error("The backbone cannot be configured as a stub at %s", (*arIt)->getSourceLocation());
            area->setExternalRoutingCapability(false);
            area->setStubDefaultCost(atoi(getRequiredAttribute(**arIt, "defaultCost")));
        }
        else
            error("Invalid node '%s' at %s", nodeName.c_str(), (*arIt)->getSourceLocation());
    }
    // Add the Area to the router
    ospfRouter->addArea(area);
}

int OSPFRouting::getIntAttrOrPar(const cXMLElement& ifConfig, const char *name) const
{
    const char* attrStr = ifConfig.getAttribute(name);
    if (attrStr && *attrStr)
        return atoi(attrStr);
    return par(name).longValue();
}

bool OSPFRouting::getBoolAttrOrPar(const cXMLElement& ifConfig, const char *name) const
{
    const char* attrStr = ifConfig.getAttribute(name);
    if (attrStr && *attrStr)
    {
        if (strcmp(attrStr, "true") == 0 || strcmp(attrStr, "1") == 0)
            return true;
        if (strcmp(attrStr, "false") == 0 || strcmp(attrStr, "0") == 0)
            return false;
        throw cRuntimeError("Invalid boolean attribute %s = '%s' at %s", name, attrStr, ifConfig.getSourceLocation());
    }
    return par(name).boolValue();
}

const char *OSPFRouting::getStrAttrOrPar(const cXMLElement& ifConfig, const char *name) const
{
    const char* attrStr = ifConfig.getAttribute(name);
    if (attrStr && *attrStr)
        return attrStr;
    return par(name).stringValue();
}

void OSPFRouting::joinMulticastGroups(int interfaceId)
{
    InterfaceEntry *ie = ift->getInterfaceById(interfaceId);
    if (!ie)
        error("Interface id=%d does not exist", interfaceId);
    IPv4InterfaceData *ipv4Data = ie->ipv4Data();
    if (!ipv4Data)
        error("Interface %s (id=%d) does not have IPv4 data", ie->getName(), interfaceId);
    ipv4Data->joinMulticastGroup(IPv4Address::ALL_OSPF_ROUTERS_MCAST);
    ipv4Data->joinMulticastGroup(IPv4Address::ALL_OSPF_DESIGNATED_ROUTERS_MCAST);
}

void OSPFRouting::loadAuthenticationConfig(OSPF::Interface* intf, const cXMLElement& ifConfig)
{
    std::string authenticationType = getStrAttrOrPar(ifConfig, "authenticationType");
    if (authenticationType == "SimplePasswordType") {
        intf->setAuthenticationType(OSPF::SIMPLE_PASSWORD_TYPE);
    } else if (authenticationType == "CrytographicType") {
        intf->setAuthenticationType(OSPF::CRYTOGRAPHIC_TYPE);
    } else if (authenticationType == "NullType") {
        intf->setAuthenticationType(OSPF::NULL_TYPE);
    } else {
        throw cRuntimeError("Invalid AuthenticationType '%s' at %s", authenticationType.c_str(), ifConfig.getSourceLocation());
    }

    std::string key = getStrAttrOrPar(ifConfig, "authenticationKey");
    OSPF::AuthenticationKeyType keyValue;
    memset(keyValue.bytes, 0, 8 * sizeof(char));
    int keyLength = key.length();
    if ((keyLength > 4) && (keyLength <= 18) && (keyLength % 2 == 0) && (key[0] == '0') && (key[1] == 'x')) {
        for (int i = keyLength; (i > 2); i -= 2) {
            keyValue.bytes[(i - 2) / 2] = hexPairToByte(key[i - 1], key[i]);
        }
    }
    intf->setAuthenticationKey(keyValue);
}

void OSPFRouting::loadInterfaceParameters(const cXMLElement& ifConfig)
{
    OSPF::Interface* intf = new OSPF::Interface;
    InterfaceEntry *ie = getInterfaceByXMLAttributesOf(ifConfig);
    int ifIndex = ie->getInterfaceId();

    std::string interfaceType = ifConfig.getTagName();

    EV << "        loading " << interfaceType << " " << ie->getName() << " (ifIndex=" << ifIndex << ")\n";

    intf->setIfIndex(ifIndex);
    if (interfaceType == "PointToPointInterface") {
        intf->setType(OSPF::Interface::POINTTOPOINT);
    } else if (interfaceType == "BroadcastInterface") {
        intf->setType(OSPF::Interface::BROADCAST);
    } else if (interfaceType == "NBMAInterface") {
        intf->setType(OSPF::Interface::NBMA);
    } else if (interfaceType == "PointToMultiPointInterface") {
        intf->setType(OSPF::Interface::POINTTOMULTIPOINT);
    } else {
        delete intf;
        error("Unknown interface type '%s' for interface %s (ifIndex=%d) at %s",
                interfaceType.c_str(), ie->getName(), ifIndex, ifConfig.getSourceLocation());
    }

    joinMulticastGroups(ifIndex);

    OSPF::AreaID areaID = IPv4Address(getStrAttrOrPar(ifConfig, "areaID"));
    intf->setAreaID(areaID);

    intf->setOutputCost(getIntAttrOrPar(ifConfig, "interfaceOutputCost"));

    intf->setRetransmissionInterval(getIntAttrOrPar(ifConfig, "retransmissionInterval"));

    intf->setTransmissionDelay(getIntAttrOrPar(ifConfig, "interfaceTransmissionDelay"));

    if (interfaceType == "BroadcastInterface" || interfaceType == "NBMAInterface")
        intf->setRouterPriority(getIntAttrOrPar(ifConfig, "routerPriority"));

    intf->setHelloInterval(getIntAttrOrPar(ifConfig, "helloInterval"));

    intf->setRouterDeadInterval(getIntAttrOrPar(ifConfig, "routerDeadInterval"));

    loadAuthenticationConfig(intf, ifConfig);

    if (interfaceType == "NBMAInterface")
        intf->setPollInterval(getIntAttrOrPar(ifConfig, "pollInterval"));

    cXMLElementList ifDetails = ifConfig.getChildren();

    for (cXMLElementList::iterator ifElemIt = ifDetails.begin(); ifElemIt != ifDetails.end(); ifElemIt++) {
        std::string nodeName = (*ifElemIt)->getTagName();
        if ((interfaceType == "NBMAInterface") && (nodeName == "NBMANeighborList")) {
            cXMLElementList neighborList = (*ifElemIt)->getChildren();
            for (cXMLElementList::iterator neighborIt = neighborList.begin(); neighborIt != neighborList.end(); neighborIt++) {
                std::string neighborNodeName = (*neighborIt)->getTagName();
                if (neighborNodeName == "NBMANeighbor") {
                    OSPF::Neighbor* neighbor = new OSPF::Neighbor;
                    neighbor->setAddress(ipv4AddressFromAddressString(getRequiredAttribute(**neighborIt, "networkInterfaceAddress")));
                    neighbor->setPriority(atoi(getRequiredAttribute(**neighborIt, "neighborPriority")));
                    intf->addNeighbor(neighbor);
                }
            }
        }
        if ((interfaceType == "PointToMultiPointInterface") && (nodeName == "PointToMultiPointNeighborList")) {
            cXMLElementList neighborList = (*ifElemIt)->getChildren();
            for (cXMLElementList::iterator neighborIt = neighborList.begin(); neighborIt != neighborList.end(); neighborIt++) {
                std::string neighborNodeName = (*neighborIt)->getTagName();
                if (neighborNodeName == "PointToMultiPointNeighbor") {
                    OSPF::Neighbor* neighbor = new OSPF::Neighbor;
                    neighbor->setAddress(ipv4AddressFromAddressString((*neighborIt)->getNodeValue()));
                    intf->addNeighbor(neighbor);
                }
            }
        }
    }
    // add the interface to it's Area
    OSPF::Area* area = ospfRouter->getAreaByID(areaID);
    if (area != NULL) {
        area->addInterface(intf);
        intf->processEvent(OSPF::Interface::INTERFACE_UP); // notification should come from the blackboard...
    } else {
        delete intf;
        error("Loading %s ifIndex[%d] in Area %s aborted at %s", interfaceType.c_str(), ifIndex, areaID.str(false).c_str(), ifConfig.getSourceLocation());
    }
}


void OSPFRouting::loadExternalRoute(const cXMLElement& externalRouteConfig)
{
    InterfaceEntry *ie = getInterfaceByXMLAttributesOf(externalRouteConfig);
    int ifIndex = ie->getInterfaceId();

    OSPFASExternalLSAContents asExternalRoute;
    OSPF::RoutingTableEntry externalRoutingEntry; // only used here to keep the path cost calculation in one place
    OSPF::IPv4AddressRange networkAddress;

    EV << "        loading ExternalInterface " << ie->getName() << " ifIndex[" << ifIndex << "]\n";

    joinMulticastGroups(ifIndex);

    networkAddress.address = ipv4AddressFromAddressString(getRequiredAttribute(externalRouteConfig, "advertisedExternalNetworkAddress"));
    networkAddress.mask = ipv4NetmaskFromAddressString(getRequiredAttribute(externalRouteConfig, "advertisedExternalNetworkMask"));
    networkAddress.address = networkAddress.address & networkAddress.mask;
    asExternalRoute.setNetworkMask(networkAddress.mask);

    int routeCost = getIntAttrOrPar(externalRouteConfig, "externalInterfaceOutputCost");
    asExternalRoute.setRouteCost(routeCost);

    std::string metricType = getStrAttrOrPar(externalRouteConfig, "externalInterfaceOutputType");
    if (metricType == "Type2") {
        asExternalRoute.setE_ExternalMetricType(true);
        externalRoutingEntry.setType2Cost(routeCost);
        externalRoutingEntry.setPathType(OSPF::RoutingTableEntry::TYPE2_EXTERNAL);
    } else if (metricType == "Type1") {
        asExternalRoute.setE_ExternalMetricType(false);
        externalRoutingEntry.setCost(routeCost);
        externalRoutingEntry.setPathType(OSPF::RoutingTableEntry::TYPE1_EXTERNAL);
    } else {
        throw cRuntimeError("Invalid 'externalInterfaceOutputType' at interface '%s' at ", ie->getName(), externalRouteConfig.getSourceLocation());
    }

    asExternalRoute.setForwardingAddress(ipv4AddressFromAddressString(getRequiredAttribute(externalRouteConfig, "forwardingAddress")));

    long externalRouteTagVal = 0;   // default value
    const char *externalRouteTag = externalRouteConfig.getAttribute("externalRouteTag");
    if (externalRouteTag && *externalRouteTag)
    {
        char *endp = NULL;
        externalRouteTagVal = strtol(externalRouteTag, &endp, 0);
        if(*endp)
            throw cRuntimeError("Invalid externalRouteTag='%s' at %s", externalRouteTag, externalRouteConfig.getSourceLocation());
    }
    asExternalRoute.setExternalRouteTag(externalRouteTagVal);

    // add the external route to the OSPF data structure
    ospfRouter->updateExternalRoute(networkAddress.address, asExternalRoute, ifIndex);
}


void OSPFRouting::loadHostRoute(const cXMLElement& hostRouteConfig)
{
    OSPF::HostRouteParameters hostParameters;
    OSPF::AreaID hostArea;

    InterfaceEntry *ie = getInterfaceByXMLAttributesOf(hostRouteConfig);
    int ifIndex = ie->getInterfaceId();

    hostParameters.ifIndex = ifIndex;

    EV << "        loading HostInterface " << ie->getName() << " ifIndex[" << ifIndex << "]\n";

    joinMulticastGroups(hostParameters.ifIndex);

    hostArea = ipv4AddressFromAddressString(getStrAttrOrPar(hostRouteConfig, "areaID"));
    hostParameters.address = ipv4AddressFromAddressString(getRequiredAttribute(hostRouteConfig, "attachedHost"));
    hostParameters.linkCost = getIntAttrOrPar(hostRouteConfig, "linkCost");

    // add the host route to the OSPF data structure.
    OSPF::Area* area = ospfRouter->getAreaByID(hostArea);
    if (area != NULL) {
        area->addHostRoute(hostParameters);
    } else {
        error("Loading HostInterface '%s' aborted, unknown area %s at %s", ie->getName(), hostArea.str(false).c_str(), hostRouteConfig.getSourceLocation());
    }
}


void OSPFRouting::loadVirtualLink(const cXMLElement& virtualLinkConfig)
{
    OSPF::Interface* intf = new OSPF::Interface;
    std::string endPoint = getRequiredAttribute(virtualLinkConfig, "endPointRouterID");
    OSPF::Neighbor* neighbor = new OSPF::Neighbor;

    EV << "        loading VirtualLink to " << endPoint << "\n";

    intf->setType(OSPF::Interface::VIRTUAL);
    neighbor->setNeighborID(ipv4AddressFromAddressString(endPoint.c_str()));
    intf->addNeighbor(neighbor);

    intf->setTransitAreaID(ipv4AddressFromAddressString(getRequiredAttribute(virtualLinkConfig, "transitAreaID")));

    intf->setRetransmissionInterval(getIntAttrOrPar(virtualLinkConfig, "retransmissionInterval"));

    intf->setTransmissionDelay(getIntAttrOrPar(virtualLinkConfig, "interfaceTransmissionDelay"));

    intf->setHelloInterval(getIntAttrOrPar(virtualLinkConfig, "helloInterval"));

    intf->setRouterDeadInterval(getIntAttrOrPar(virtualLinkConfig, "routerDeadInterval"));

    loadAuthenticationConfig(intf, virtualLinkConfig);

    // add the virtual link to the OSPF data structure.
    OSPF::Area* transitArea = ospfRouter->getAreaByID(intf->getAreaID());
    OSPF::Area* backbone = ospfRouter->getAreaByID(OSPF::BACKBONE_AREAID);

    if ((backbone != NULL) && (transitArea != NULL) && (transitArea->getExternalRoutingCapability())) {
        backbone->addInterface(intf);
    } else {
        error("Loading VirtualLink to %s through Area %s aborted at ", endPoint.c_str(), intf->getAreaID().str(false).c_str(), virtualLinkConfig.getSourceLocation());
        delete intf;
    }
}


bool OSPFRouting::loadConfigFromXML(cXMLElement *asConfig)
{
    if (strcmp(asConfig->getTagName(), "OSPFASConfig"))
        error("Cannot read OSPF configuration, unaccepted '%s' note at %s", asConfig->getTagName(), asConfig->getSourceLocation());

    cModule *myNode = findContainingNode(this);

    ASSERT(myNode);
    std::string nodeFullPath = myNode->getFullPath();
    std::string nodeShortenedFullPath = nodeFullPath.substr(nodeFullPath.find('.') + 1);

    // load information on this router
    cXMLElementList routers = asConfig->getElementsByTagName("Router");
    cXMLElement* routerNode = NULL;
    for (cXMLElementList::iterator routerIt = routers.begin(); routerIt != routers.end(); routerIt++) {
        const char* nodeName = getRequiredAttribute(*(*routerIt), "name");
        inet::PatternMatcher pattern(nodeName, true, true, true);
        if (pattern.matches(nodeFullPath.c_str()) || pattern.matches(nodeShortenedFullPath.c_str()))   // match Router@name and fullpath of my node
        {
            routerNode = *routerIt;
            break;
        }
    }
    if (routerNode == NULL) {
        error("No configuration for Router '%s' at '%s'", nodeFullPath.c_str(), asConfig->getSourceLocation());
    }

    EV << "OSPFRouting: Loading info for Router " << nodeFullPath << "\n";

    bool rfc1583Compatible = getBoolAttrOrPar(*routerNode, "RFC1583Compatible");
    ospfRouter->setRFC1583Compatibility(rfc1583Compatible);

    std::set<OSPF::AreaID> areaList;
    getAreaListFromXML(*routerNode, areaList);

    // if the router is an area border router then it MUST be part of the backbone(area 0)
    if ((areaList.size() > 1) && (areaList.find(OSPF::BACKBONE_AREAID) == areaList.end())) {
        areaList.insert(OSPF::BACKBONE_AREAID);
    }
    // load area information
    for (std::set<OSPF::AreaID>::iterator areaIt = areaList.begin(); areaIt != areaList.end(); areaIt++) {
        loadAreaFromXML(*asConfig, *areaIt);
    }

    // load interface information
    cXMLElementList routerConfig = routerNode->getChildren();
    for (cXMLElementList::iterator routerConfigIt = routerConfig.begin(); routerConfigIt != routerConfig.end(); routerConfigIt++) {
        std::string nodeName = (*routerConfigIt)->getTagName();
        if ((nodeName == "PointToPointInterface") ||
            (nodeName == "BroadcastInterface") ||
            (nodeName == "NBMAInterface") ||
            (nodeName == "PointToMultiPointInterface"))
        {
            loadInterfaceParameters(*(*routerConfigIt));
        }
        else if (nodeName == "ExternalInterface") {
            loadExternalRoute(*(*routerConfigIt));
        }
        else if (nodeName == "HostInterface") {
            loadHostRoute(*(*routerConfigIt));
        }
        else if (nodeName == "VirtualLink") {
            loadVirtualLink(*(*routerConfigIt));
        } else {
            throw cRuntimeError("Invalid '%s' node in Router '%s' at %s",
                    nodeName.c_str(), nodeFullPath.c_str(), (*routerConfigIt)->getSourceLocation());
        }

    }
    return true;
}

