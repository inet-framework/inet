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

#include "inet/routing/ospfv2/OspfConfigReader.h"

#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/routing/ospfv2/messagehandler/MessageHandler.h"
#include "inet/routing/ospfv2/router/OspfArea.h"
#include "inet/routing/ospfv2/router/OspfCommon.h"
#include "inet/routing/ospfv2/interface/OspfInterface.h"
#include "inet/common/PatternMatcher.h"
#include "inet/common/XMLUtils.h"
#include "inet/common/ModuleAccess.h"

namespace inet {

namespace ospf {

using namespace xmlutils;

OspfConfigReader::OspfConfigReader(cModule *ospfModule, IInterfaceTable *ift) :
        ospfModule(ospfModule), ift(ift)
{
}

OspfConfigReader::~OspfConfigReader()
{
}

InterfaceEntry *OspfConfigReader::getInterfaceByXMLAttributesOf(const cXMLElement& ifConfig)
{
    const char *ifName = ifConfig.getAttribute("ifName");
    if (ifName && *ifName) {
        InterfaceEntry *ie = ift->getInterfaceByName(ifName);
        if (!ie)
            throw cRuntimeError("Error reading XML config: IInterfaceTable contains no interface named '%s' at %s", ifName, ifConfig.getSourceLocation());
        return ie;
    }

    const char *toward = getMandatoryFilledAttribute(ifConfig, "toward");
    cModule *destnode = getSimulation()->getSystemModule()->getModuleByPath(toward);
    if (!destnode)
        throw cRuntimeError("toward module `%s' not found at %s", toward, ifConfig.getSourceLocation());

    cModule *host = ift->getHostModule();
    for (int i = 0; i < ift->getNumInterfaces(); i++) {
        InterfaceEntry *ie = ift->getInterface(i);
        if (ie) {
            int gateId = ie->getNodeOutputGateId();
            if ((gateId != -1) && (host->gate(gateId)->pathContains(destnode)))
                return ie;
        }
    }
    throw cRuntimeError("Error reading XML config: IInterfaceTable contains no interface toward '%s' at %s", toward, ifConfig.getSourceLocation());
}

int OspfConfigReader::resolveInterfaceName(const std::string& name) const
{
    InterfaceEntry *ie = ift->getInterfaceByName(name.c_str());
    if (!ie)
        throw cRuntimeError("Error reading XML config: IInterfaceTable contains no interface named '%s'", name.c_str());

    return ie->getInterfaceId();
}

void OspfConfigReader::getAreaListFromXML(const cXMLElement& routerNode, std::set<AreaId>& areaList) const
{
    cXMLElementList routerConfig = routerNode.getChildren();
    for (auto & elem : routerConfig) {
        std::string nodeName = (elem)->getTagName();
        if ((nodeName == "PointToPointInterface") ||
            (nodeName == "BroadcastInterface") ||
            (nodeName == "NBMAInterface") ||
            (nodeName == "PointToMultiPointInterface"))
        {
            AreaId areaID = Ipv4Address(getStrAttrOrPar(*elem, "areaID"));
            if (areaList.find(areaID) == areaList.end())
                areaList.insert(areaID);
        }
    }
}

void OspfConfigReader::loadAreaFromXML(const cXMLElement& asConfig, AreaId areaID)
{
    std::string areaXPath("Area[@id='");
    areaXPath += areaID.str(false);
    areaXPath += "']";

    cXMLElement *areaConfig = asConfig.getElementByPath(areaXPath.c_str());
    if (areaConfig == nullptr) {
        throw cRuntimeError("No configuration for Area ID: %s at %s", areaID.str(false).c_str(), asConfig.getSourceLocation());
    }
    else {
        EV_DEBUG << "    loading info for Area id = " << areaID.str(false) << "\n";
    }

    Area *area = new Area(ift, areaID);
    cXMLElementList areaDetails = areaConfig->getChildren();
    for (auto & areaDetail : areaDetails) {
        std::string nodeName = (areaDetail)->getTagName();
        if (nodeName == "AddressRange") {
            Ipv4AddressRange addressRange;
            addressRange.address = ipv4AddressFromAddressString(getMandatoryFilledAttribute(*areaDetail, "address"));
            addressRange.mask = ipv4NetmaskFromAddressString(getMandatoryFilledAttribute(*areaDetail, "mask"));
            addressRange.address = addressRange.address & addressRange.mask;
            std::string status = getMandatoryFilledAttribute(*areaDetail, "status");
            area->addAddressRange(addressRange, status == "Advertise");
        }
        else if (nodeName == "Stub") {
            if (areaID == BACKBONE_AREAID)
                throw cRuntimeError("The backbone cannot be configured as a stub at %s", (areaDetail)->getSourceLocation());
            area->setExternalRoutingCapability(false);
            area->setStubDefaultCost(atoi(getMandatoryFilledAttribute(*areaDetail, "defaultCost")));
        }
        else
            throw cRuntimeError("Invalid node '%s' at %s", nodeName.c_str(), (areaDetail)->getSourceLocation());
    }
    // Add the Area to the router
    ospfRouter->addArea(area);
}

int OspfConfigReader::getIntAttrOrPar(const cXMLElement& ifConfig, const char *name) const
{
    const char *attrStr = ifConfig.getAttribute(name);
    if (attrStr && *attrStr)
        return atoi(attrStr);
    return par(name);
}

bool OspfConfigReader::getBoolAttrOrPar(const cXMLElement& ifConfig, const char *name) const
{
    const char *attrStr = ifConfig.getAttribute(name);
    if (attrStr && *attrStr) {
        if (strcmp(attrStr, "true") == 0 || strcmp(attrStr, "1") == 0)
            return true;
        if (strcmp(attrStr, "false") == 0 || strcmp(attrStr, "0") == 0)
            return false;
        throw cRuntimeError("Invalid boolean attribute %s = '%s' at %s", name, attrStr, ifConfig.getSourceLocation());
    }
    return par(name);
}

const char *OspfConfigReader::getStrAttrOrPar(const cXMLElement& ifConfig, const char *name) const
{
    const char *attrStr = ifConfig.getAttribute(name);
    if (attrStr && *attrStr)
        return attrStr;
    return par(name);
}

void OspfConfigReader::joinMulticastGroups(int interfaceId)
{
    InterfaceEntry *ie = ift->getInterfaceById(interfaceId);
    if (!ie)
        throw cRuntimeError("Interface id=%d does not exist", interfaceId);
    if (!ie->isMulticast())
        return;
    Ipv4InterfaceData *ipv4Data = ie->ipv4Data();
    if (!ipv4Data)
        throw cRuntimeError("Interface %s (id=%d) does not have Ipv4 data", ie->getInterfaceName(), interfaceId);
    ipv4Data->joinMulticastGroup(Ipv4Address::ALL_OSPF_ROUTERS_MCAST);
    ipv4Data->joinMulticastGroup(Ipv4Address::ALL_OSPF_DESIGNATED_ROUTERS_MCAST);
}

void OspfConfigReader::loadAuthenticationConfig(Interface *intf, const cXMLElement& ifConfig)
{
    std::string authenticationType = getStrAttrOrPar(ifConfig, "authenticationType");
    if (authenticationType == "SimplePasswordType") {
        intf->setAuthenticationType(SIMPLE_PASSWORD_TYPE);
    }
    else if (authenticationType == "CrytographicType") {
        intf->setAuthenticationType(CRYTOGRAPHIC_TYPE);
    }
    else if (authenticationType == "NullType") {
        intf->setAuthenticationType(NULL_TYPE);
    }
    else {
        throw cRuntimeError("Invalid AuthenticationType '%s' at %s", authenticationType.c_str(), ifConfig.getSourceLocation());
    }

    std::string key = getStrAttrOrPar(ifConfig, "authenticationKey");
    AuthenticationKeyType keyValue;
    memset(keyValue.bytes, 0, sizeof(keyValue.bytes));
    int keyLength = key.length();
    if ((keyLength > 4) && (keyLength <= 18) && (keyLength % 2 == 0) && (key[0] == '0') && (key[1] == 'x')) {
        for (int i = keyLength; (i > 2); i -= 2) {
            keyValue.bytes[(i - 2) / 2 - 1] = hexPairToByte(key[i - 1], key[i]);
        }
    }
    intf->setAuthenticationKey(keyValue);
}

void OspfConfigReader::loadInterfaceParameters(const cXMLElement& ifConfig)
{
    Interface *intf = new Interface;
    InterfaceEntry *ie = getInterfaceByXMLAttributesOf(ifConfig);
    int ifIndex = ie->getInterfaceId();

    std::string interfaceType = ifConfig.getTagName();

    EV_DEBUG << "        loading " << interfaceType << " " << ie->getInterfaceName() << " (ifIndex=" << ifIndex << ")\n";

    intf->setIfIndex(ift, ifIndex);
    if (interfaceType == "PointToPointInterface") {
        intf->setType(Interface::POINTTOPOINT);
    }
    else if (interfaceType == "BroadcastInterface") {
        intf->setType(Interface::BROADCAST);
    }
    else if (interfaceType == "NBMAInterface") {
        intf->setType(Interface::NBMA);
    }
    else if (interfaceType == "PointToMultiPointInterface") {
        intf->setType(Interface::POINTTOMULTIPOINT);
    }
    else {
        delete intf;
        throw cRuntimeError("Unknown interface type '%s' for interface %s (ifIndex=%d) at %s",
                interfaceType.c_str(), ie->getInterfaceName(), ifIndex, ifConfig.getSourceLocation());
    }

    joinMulticastGroups(ifIndex);

    AreaId areaID = Ipv4Address(getStrAttrOrPar(ifConfig, "areaID"));
    intf->setAreaId(areaID);

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

    for (auto & ifDetail : ifDetails) {
        std::string nodeName = (ifDetail)->getTagName();
        if ((interfaceType == "NBMAInterface") && (nodeName == "NBMANeighborList")) {
            cXMLElementList neighborList = (ifDetail)->getChildren();
            for (auto & elem : neighborList) {
                std::string neighborNodeName = (elem)->getTagName();
                if (neighborNodeName == "NBMANeighbor") {
                    Neighbor *neighbor = new Neighbor;
                    neighbor->setAddress(ipv4AddressFromAddressString(getMandatoryFilledAttribute(*elem, "networkInterfaceAddress")));
                    neighbor->setPriority(atoi(getMandatoryFilledAttribute(*elem, "neighborPriority")));
                    intf->addNeighbor(neighbor);
                }
            }
        }
        if ((interfaceType == "PointToMultiPointInterface") && (nodeName == "PointToMultiPointNeighborList")) {
            cXMLElementList neighborList = (ifDetail)->getChildren();
            for (auto & elem : neighborList) {
                std::string neighborNodeName = (elem)->getTagName();
                if (neighborNodeName == "PointToMultiPointNeighbor") {
                    Neighbor *neighbor = new Neighbor;
                    neighbor->setAddress(ipv4AddressFromAddressString((elem)->getNodeValue()));
                    intf->addNeighbor(neighbor);
                }
            }
        }
    }
    // add the interface to it's Area
    Area *area = ospfRouter->getAreaByID(areaID);
    if (area != nullptr) {
        area->addInterface(intf);
        intf->processEvent(Interface::INTERFACE_UP);    // notification should come from the blackboard...
    }
    else {
        delete intf;
        throw cRuntimeError("Loading %s ifIndex[%d] in Area %s aborted at %s", interfaceType.c_str(), ifIndex, areaID.str(false).c_str(), ifConfig.getSourceLocation());
    }
}

void OspfConfigReader::loadExternalRoute(const cXMLElement& externalRouteConfig)
{
    InterfaceEntry *ie = getInterfaceByXMLAttributesOf(externalRouteConfig);
    int ifIndex = ie->getInterfaceId();

    OspfAsExternalLsaContents asExternalRoute;
    //RoutingTableEntry externalRoutingEntry; // only used here to keep the path cost calculation in one place
    Ipv4AddressRange networkAddress;

    EV_DEBUG << "        loading ExternalInterface " << ie->getInterfaceName() << " ifIndex[" << ifIndex << "]\n";

    joinMulticastGroups(ifIndex);

    networkAddress.address = ipv4AddressFromAddressString(getMandatoryFilledAttribute(externalRouteConfig, "advertisedExternalNetworkAddress"));
    networkAddress.mask = ipv4NetmaskFromAddressString(getMandatoryFilledAttribute(externalRouteConfig, "advertisedExternalNetworkMask"));
    networkAddress.address = networkAddress.address & networkAddress.mask;
    asExternalRoute.setNetworkMask(networkAddress.mask);

    int routeCost = getIntAttrOrPar(externalRouteConfig, "externalInterfaceOutputCost");
    asExternalRoute.setRouteCost(routeCost);

    std::string metricType = getStrAttrOrPar(externalRouteConfig, "externalInterfaceOutputType");
    if (metricType == "Type2") {
        asExternalRoute.setE_ExternalMetricType(true);
        //externalRoutingEntry.setType2Cost(routeCost);
        //externalRoutingEntry.setPathType(RoutingTableEntry::TYPE2_EXTERNAL);
    }
    else if (metricType == "Type1") {
        asExternalRoute.setE_ExternalMetricType(false);
        //externalRoutingEntry.setCost(routeCost);
        //externalRoutingEntry.setPathType(RoutingTableEntry::TYPE1_EXTERNAL);
    }
    else {
        throw cRuntimeError("Invalid 'externalInterfaceOutputType' at interface '%s' at ", ie->getInterfaceName(), externalRouteConfig.getSourceLocation());
    }

    asExternalRoute.setForwardingAddress(ipv4AddressFromAddressString(getMandatoryFilledAttribute(externalRouteConfig, "forwardingAddress")));

    long externalRouteTagVal = 0;    // default value
    const char *externalRouteTag = externalRouteConfig.getAttribute("externalRouteTag");
    if (externalRouteTag && *externalRouteTag) {
        char *endp = nullptr;
        externalRouteTagVal = strtol(externalRouteTag, &endp, 0);
        if (*endp)
            throw cRuntimeError("Invalid externalRouteTag='%s' at %s", externalRouteTag, externalRouteConfig.getSourceLocation());
    }
    asExternalRoute.setExternalRouteTag(externalRouteTagVal);

    // add the external route to the OSPF data structure
    ospfRouter->updateExternalRoute(networkAddress.address, asExternalRoute, ifIndex);
}

void OspfConfigReader::loadHostRoute(const cXMLElement& hostRouteConfig)
{
    HostRouteParameters hostParameters;
    AreaId hostArea;

    InterfaceEntry *ie = getInterfaceByXMLAttributesOf(hostRouteConfig);
    int ifIndex = ie->getInterfaceId();

    hostParameters.ifIndex = ifIndex;

    EV_DEBUG << "        loading HostInterface " << ie->getInterfaceName() << " ifIndex[" << ifIndex << "]\n";

    joinMulticastGroups(hostParameters.ifIndex);

    hostArea = ipv4AddressFromAddressString(getStrAttrOrPar(hostRouteConfig, "areaID"));
    hostParameters.address = ipv4AddressFromAddressString(getMandatoryFilledAttribute(hostRouteConfig, "attachedHost"));
    hostParameters.linkCost = getIntAttrOrPar(hostRouteConfig, "linkCost");

    // add the host route to the OSPF data structure.
    Area *area = ospfRouter->getAreaByID(hostArea);
    if (area != nullptr) {
        area->addHostRoute(hostParameters);
    }
    else {
        throw cRuntimeError("Loading HostInterface '%s' aborted, unknown area %s at %s", ie->getInterfaceName(), hostArea.str(false).c_str(), hostRouteConfig.getSourceLocation());
    }
}

void OspfConfigReader::loadVirtualLink(const cXMLElement& virtualLinkConfig)
{
    Interface *intf = new Interface;
    std::string endPoint = getMandatoryFilledAttribute(virtualLinkConfig, "endPointRouterID");
    Neighbor *neighbor = new Neighbor;

    EV_DEBUG << "        loading VirtualLink to " << endPoint << "\n";

    intf->setType(Interface::VIRTUAL);
    neighbor->setNeighborID(ipv4AddressFromAddressString(endPoint.c_str()));
    intf->addNeighbor(neighbor);

    intf->setTransitAreaId(ipv4AddressFromAddressString(getMandatoryFilledAttribute(virtualLinkConfig, "transitAreaID")));

    intf->setRetransmissionInterval(getIntAttrOrPar(virtualLinkConfig, "retransmissionInterval"));

    intf->setTransmissionDelay(getIntAttrOrPar(virtualLinkConfig, "interfaceTransmissionDelay"));

    intf->setHelloInterval(getIntAttrOrPar(virtualLinkConfig, "helloInterval"));

    intf->setRouterDeadInterval(getIntAttrOrPar(virtualLinkConfig, "routerDeadInterval"));

    loadAuthenticationConfig(intf, virtualLinkConfig);

    // add the virtual link to the OSPF data structure.
    Area *transitArea = ospfRouter->getAreaByID(intf->getAreaId());
    Area *backbone = ospfRouter->getAreaByID(BACKBONE_AREAID);

    if ((backbone != nullptr) && (transitArea != nullptr) && (transitArea->getExternalRoutingCapability())) {
        backbone->addInterface(intf);
    }
    else {
        throw cRuntimeError("Loading VirtualLink to %s through Area %s aborted at ", endPoint.c_str(), intf->getAreaId().str(false).c_str(), virtualLinkConfig.getSourceLocation());
        //delete intf;
    }
}

bool OspfConfigReader::loadConfigFromXML(cXMLElement *asConfig, Router *ospfRouter)
{
    this->ospfRouter = ospfRouter;

    if (strcmp(asConfig->getTagName(), "OSPFASConfig"))
        throw cRuntimeError("Cannot read OSPF configuration, unexpected element '%s' at %s", asConfig->getTagName(), asConfig->getSourceLocation());

    cModule *myNode = findContainingNode(ospfModule);

    ASSERT(myNode);
    std::string nodeFullPath = myNode->getFullPath();
    std::string nodeShortenedFullPath = nodeFullPath.substr(nodeFullPath.find('.') + 1);

    // load information on this router
    cXMLElementList routers = asConfig->getElementsByTagName("Router");
    cXMLElement *routerNode = nullptr;
    for (auto & router : routers) {
        const char *nodeName = getMandatoryFilledAttribute(*(router), "name");
        inet::PatternMatcher pattern(nodeName, true, true, true);
        if (pattern.matches(nodeFullPath.c_str()) || pattern.matches(nodeShortenedFullPath.c_str())) {    // match Router@name and fullpath of my node
            routerNode = router;
            break;
        }
    }
    if (routerNode == nullptr) {
        throw cRuntimeError("No configuration for Router '%s' at '%s'", nodeFullPath.c_str(), asConfig->getSourceLocation());
    }

    EV_DEBUG << "OspfConfigReader: Loading info for Router " << nodeFullPath << "\n";

    bool rfc1583Compatible = getBoolAttrOrPar(*routerNode, "RFC1583Compatible");
    ospfRouter->setRFC1583Compatibility(rfc1583Compatible);

    std::set<AreaId> areaList;
    getAreaListFromXML(*routerNode, areaList);

    // if the router is an area border router then it MUST be part of the backbone(area 0)
    if ((areaList.size() > 1) && (areaList.find(BACKBONE_AREAID) == areaList.end())) {
        areaList.insert(BACKBONE_AREAID);
    }
    // load area information
    for (const auto & elem : areaList) {
        loadAreaFromXML(*asConfig, elem);
    }

    // load interface information
    cXMLElementList routerConfig = routerNode->getChildren();
    for (auto & elem : routerConfig) {
        std::string nodeName = (elem)->getTagName();
        if ((nodeName == "PointToPointInterface") ||
            (nodeName == "BroadcastInterface") ||
            (nodeName == "NBMAInterface") ||
            (nodeName == "PointToMultiPointInterface"))
        {
            loadInterfaceParameters(*(elem));
        }
        else if (nodeName == "ExternalInterface") {
            loadExternalRoute(*(elem));
        }
        else if (nodeName == "HostInterface") {
            loadHostRoute(*(elem));
        }
        else if (nodeName == "VirtualLink") {
            loadVirtualLink(*(elem));
        }
        else {
            throw cRuntimeError("Invalid '%s' node in Router '%s' at %s",
                    nodeName.c_str(), nodeFullPath.c_str(), (elem)->getSourceLocation());
        }
    }
    return true;
}

} // namespace ospf

} // namespace inet

