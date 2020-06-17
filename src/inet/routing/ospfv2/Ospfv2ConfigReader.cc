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

#include <algorithm>
#include <map>
#include <memory.h>
#include <stdlib.h>
#include <string>

#include "inet/common/ModuleAccess.h"
#include "inet/common/PatternMatcher.h"
#include "inet/common/XMLUtils.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/routing/ospfv2/Ospfv2ConfigReader.h"
#include "inet/routing/ospfv2/interface/Ospfv2Interface.h"
#include "inet/routing/ospfv2/messagehandler/MessageHandler.h"
#include "inet/routing/ospfv2/router/Ospfv2Area.h"
#include "inet/routing/ospfv2/router/Ospfv2Common.h"

namespace inet {
namespace ospfv2 {

using namespace xmlutils;

Ospfv2ConfigReader::Ospfv2ConfigReader(cModule *ospfModule, IInterfaceTable *ift) :
        ospfModule(ospfModule), ift(ift)
{
}

Ospfv2ConfigReader::~Ospfv2ConfigReader()
{
}

bool Ospfv2ConfigReader::loadConfigFromXML(cXMLElement *asConfig, Router *ospfRouter)
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

    EV_DEBUG << "Ospfv2ConfigReader: Loading info for Router " << nodeFullPath << "\n";

    bool rfc1583Compatible = getBoolAttrOrPar(*routerNode, "RFC1583Compatible");
    ospfRouter->setRFC1583Compatibility(rfc1583Compatible);

    std::set<AreaId> areaList;
    getAreaListFromXML(*routerNode, areaList);

    // load area information
    for (const auto & elem : areaList) {
        loadAreaFromXML(*asConfig, elem);
    }

    cXMLElementList routerConfig = routerNode->getChildren();

    // load interface information
    for (int n = 0; n < ift->getNumInterfaces(); n++) {
        InterfaceEntry *intf = ift->getInterface(n);
        cXMLElement *ifConfig = findMatchingConfig(routerConfig, *intf);
        if(ifConfig) {
            if(intf->isLoopback())
                loadLoopbackParameters(*ifConfig, *intf);
            else
                loadInterfaceParameters(*ifConfig, *intf);
        }
    }

    // load remaining information
    for (auto & elem : routerConfig) {
        std::string nodeName = elem->getTagName();
        if ((nodeName == "PointToPointInterface") ||
            (nodeName == "BroadcastInterface") ||
            (nodeName == "NBMAInterface") ||
            (nodeName == "PointToMultiPointInterface") ||
            (nodeName == "LoopbackInterface"))
        {
            continue;
        }
        else if (nodeName == "ExternalInterface") {
            loadExternalRoute(*elem);
        }
        else if (nodeName == "HostInterface") {
            loadHostRoute(*elem);
        }
        else if (nodeName == "VirtualLink") {
            loadVirtualLink(*elem, *asConfig);
        }
        else {
            throw cRuntimeError("Invalid '%s' node in Router '%s' at %s",
                    nodeName.c_str(), nodeFullPath.c_str(), elem->getSourceLocation());
        }
    }

    bool DistributeDefaultRoute = getBoolAttrOrPar(*routerNode, "DistributeDefaultRoute");
    if(DistributeDefaultRoute)
        initiateDefaultRouteDistribution();

    return true;
}

void Ospfv2ConfigReader::getAreaListFromXML(const cXMLElement& routerNode, std::set<AreaId>& areaList) const
{
    cXMLElementList routerConfig = routerNode.getChildren();
    for (auto & elem : routerConfig) {
        std::string nodeName = (elem)->getTagName();
        if ((nodeName == "PointToPointInterface") ||
            (nodeName == "BroadcastInterface") ||
            (nodeName == "NBMAInterface") ||
            (nodeName == "LoopbackInterface") ||
            (nodeName == "PointToMultiPointInterface"))
        {
            AreaId areaID = Ipv4Address(getStrAttrOrPar(*elem, "areaID"));
            if (areaList.find(areaID) == areaList.end())
                areaList.insert(areaID);
        }
    }
}

void Ospfv2ConfigReader::loadAreaFromXML(const cXMLElement& asConfig, AreaId areaID)
{
    std::string areaXPath("Area[@id='");
    areaXPath += areaID.str(false);
    areaXPath += "']";

    auto crcMode = parseCrcMode(par("crcMode"), false);

    cXMLElement *areaConfig = asConfig.getElementByPath(areaXPath.c_str());
    if (areaConfig == nullptr) {
        if(areaID != Ipv4Address("0.0.0.0"))
            throw cRuntimeError("No configuration for Area ID: %s at %s", areaID.str(false).c_str(), asConfig.getSourceLocation());
        Ospfv2Area *area = new Ospfv2Area(crcMode, ift, areaID);
        area->addWatches();
        ospfRouter->addArea(area);
        return;
    }

    EV_DEBUG << "    loading info for Area id = " << areaID.str(false) << "\n";

    Ospfv2Area *area = new Ospfv2Area(crcMode, ift, areaID);
    area->addWatches();
    cXMLElementList areaDetails = areaConfig->getChildren();
    for (auto & areaDetail : areaDetails) {
        std::string nodeName = (areaDetail)->getTagName();
        if (nodeName == "AddressRange") {
            Ipv4AddressRange addressRange;
            addressRange.address = ipv4AddressFromAddressString(getMandatoryFilledAttribute(*areaDetail, "address"));
            addressRange.mask = ipv4NetmaskFromAddressString(getMandatoryFilledAttribute(*areaDetail, "mask"));
            addressRange.address = addressRange.address & addressRange.mask;
            const char *adv = areaDetail->getAttribute("advertise");
            if(!adv)
                area->addAddressRange(addressRange, true);
            else
                area->addAddressRange(addressRange, std::string(adv) == "true");
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

void Ospfv2ConfigReader::loadInterfaceParameters(const cXMLElement& ifConfig, InterfaceEntry& ie)
{
    std::string intfModeStr = getStrAttrOrPar(ifConfig, "interfaceMode");
    if(intfModeStr == "NoOSPF")
        return;

    std::string interfaceType = ifConfig.getTagName();
    int ifIndex = ie.getInterfaceId();
    std::string ifName = ie.getInterfaceName();

    EV_DEBUG << "        loading " << interfaceType << " " << ifName << " (ifIndex=" << ifIndex << ")\n";

    Ospfv2Interface *intf = new Ospfv2Interface;

    intf->setInterfaceName(ifName);
    AreaId areaID = Ipv4Address(getStrAttrOrPar(ifConfig, "areaID"));
    intf->setAreaId(areaID);
    intf->setIfIndex(ift, ifIndex); // should be called before calling setType()

    if (interfaceType == "PointToPointInterface")
        intf->setType(Ospfv2Interface::POINTTOPOINT);
    else if (interfaceType == "BroadcastInterface")
        intf->setType(Ospfv2Interface::BROADCAST);
    else if (interfaceType == "NBMAInterface")
        intf->setType(Ospfv2Interface::NBMA);
    else if (interfaceType == "PointToMultiPointInterface")
        intf->setType(Ospfv2Interface::POINTTOMULTIPOINT);
    else {
        delete intf;
        throw cRuntimeError("Unknown interface type '%s' for interface %s (ifIndex=%d) at %s",
                interfaceType.c_str(), ifName.c_str(), ifIndex, ifConfig.getSourceLocation());
    }

    if(intfModeStr == "Active")
        intf->setMode(Ospfv2Interface::ACTIVE);
    else if(intfModeStr == "Passive")
        intf->setMode(Ospfv2Interface::PASSIVE);
    else {
        delete intf;
        throw cRuntimeError("Unknown interface mode '%s' for interface %s (ifIndex=%d) at %s",
                interfaceType.c_str(), ifName.c_str(), ifIndex, ifConfig.getSourceLocation());
    }

    intf->setCrcMode(parseCrcMode(par("crcMode"), false));

    Metric cost = getIntAttrOrPar(ifConfig, "interfaceOutputCost");
    if(cost == 0)
        intf->setOutputCost(round(par("referenceBandwidth").intValue() / ie.getDatarate()));
    else
        intf->setOutputCost(cost);

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

    joinMulticastGroups(ifIndex);

    // add the interface to it's Area
    Ospfv2Area *area = ospfRouter->getAreaByID(areaID);
    if (area != nullptr) {
        area->addInterface(intf);
        intf->processEvent(Ospfv2Interface::INTERFACE_UP);    // notification should come from the blackboard...
    }
    else {
        delete intf;
        throw cRuntimeError("Loading %s ifIndex[%d] in Area %s aborted at %s", interfaceType.c_str(), ifIndex, areaID.str(false).c_str(), ifConfig.getSourceLocation());
    }
}

void Ospfv2ConfigReader::loadExternalRoute(const cXMLElement& externalRouteConfig)
{
    for(auto &ie : getInterfaceByXMLAttributesOf(externalRouteConfig)) {
        Ospfv2AsExternalLsaContents asExternalRoute;
        Ipv4AddressRange networkAddress;
        int ifIndex = ie->getInterfaceId();

        EV_DEBUG << "        loading ExternalInterface " << ie->getInterfaceName() << " ifIndex[" << ifIndex << "]\n";

        joinMulticastGroups(ifIndex);

        networkAddress.address = ipv4AddressFromAddressString(getMandatoryFilledAttribute(externalRouteConfig, "advertisedExternalNetworkAddress"));
        networkAddress.mask = ipv4NetmaskFromAddressString(getMandatoryFilledAttribute(externalRouteConfig, "advertisedExternalNetworkMask"));
        networkAddress.address = networkAddress.address & networkAddress.mask;
        asExternalRoute.setNetworkMask(networkAddress.mask);
        asExternalRoute.setExternalTOSInfoArraySize(1);
        auto& tosInfo = asExternalRoute.getExternalTOSInfoForUpdate(0);

        int routeCost = getIntAttrOrPar(externalRouteConfig, "externalInterfaceOutputCost");
        tosInfo.tos = 0;
        tosInfo.routeCost = routeCost;

        std::string metricType = getStrAttrOrPar(externalRouteConfig, "externalInterfaceOutputType");
        if (metricType == "Type1")
            tosInfo.E_ExternalMetricType = false;
        else if (metricType == "Type2")
            tosInfo.E_ExternalMetricType = true;
        else
            throw cRuntimeError("Invalid 'externalInterfaceOutputType' at interface '%s' at ", ie->getInterfaceName(), externalRouteConfig.getSourceLocation());

        tosInfo.forwardingAddress = ipv4AddressFromAddressString(getStrAttrOrPar(externalRouteConfig, "forwardingAddress"));

        long externalRouteTagVal = 0;    // default value
        const char *externalRouteTag = externalRouteConfig.getAttribute("externalRouteTag");
        if (externalRouteTag && *externalRouteTag) {
            char *endp = nullptr;
            externalRouteTagVal = strtol(externalRouteTag, &endp, 0);
            if (*endp)
                throw cRuntimeError("Invalid externalRouteTag='%s' at %s", externalRouteTag, externalRouteConfig.getSourceLocation());
        }
        tosInfo.externalRouteTag = externalRouteTagVal;

        // add the external route to the OSPF data structure
        ospfRouter->updateExternalRoute(networkAddress.address, asExternalRoute, ifIndex);
    }
}

void Ospfv2ConfigReader::loadHostRoute(const cXMLElement& hostRouteConfig)
{
    std::string intfModeStr = getStrAttrOrPar(hostRouteConfig, "interfaceMode");
    if(intfModeStr == "NoOSPF")
        return;

    HostRouteParameters hostParameters;
    AreaId hostArea;

    for(auto &ie : getInterfaceByXMLAttributesOf(hostRouteConfig)) {
        int ifIndex = ie->getInterfaceId();

        hostParameters.ifIndex = ifIndex;

        EV_DEBUG << "        loading HostInterface " << ie->getInterfaceName() << " ifIndex[" << ifIndex << "]\n";

        joinMulticastGroups(hostParameters.ifIndex);

        hostArea = ipv4AddressFromAddressString(getStrAttrOrPar(hostRouteConfig, "areaID"));
        hostParameters.address = ipv4AddressFromAddressString(getMandatoryFilledAttribute(hostRouteConfig, "attachedHost"));
        hostParameters.linkCost = getIntAttrOrPar(hostRouteConfig, "linkCost");

        // add the host route to the OSPF data structure.
        Ospfv2Area *area = ospfRouter->getAreaByID(hostArea);
        if (area != nullptr) {
            area->addHostRoute(hostParameters);
        }
        else {
            throw cRuntimeError("Loading HostInterface '%s' aborted, unknown area %s at %s", ie->getInterfaceName(), hostArea.str(false).c_str(), hostRouteConfig.getSourceLocation());
        }
    }
}

void Ospfv2ConfigReader::loadLoopbackParameters(const cXMLElement& loConfig, InterfaceEntry& ie)
{
    int ifIndex = ie.getInterfaceId();
    EV_DEBUG << "        loading LoopbackInterface " << ie.getInterfaceName() << " ifIndex[" << ifIndex << "]\n";

    joinMulticastGroups(ifIndex);

    // Loopbacks are considered host routes in OSPF, and they are advertised as /32
    HostRouteParameters hostParameters;
    hostParameters.ifIndex = ifIndex;
    AreaId hostArea = ipv4AddressFromAddressString(getStrAttrOrPar(loConfig, "areaID"));
    hostParameters.address = ie.getIpv4Address();
    hostParameters.linkCost = getIntAttrOrPar(loConfig, "linkCost");

    // add the host route to the OSPF data structure.
    Ospfv2Area *area = ospfRouter->getAreaByID(hostArea);
    if (area != nullptr)
        area->addHostRoute(hostParameters);
    else
        throw cRuntimeError("Loading LoopbackInterface '%s' aborted, unknown area %s at %s", ie.getInterfaceName(), hostArea.str(false).c_str(), loConfig.getSourceLocation());
}

void Ospfv2ConfigReader::loadVirtualLink(const cXMLElement& virtualLinkConfig, cXMLElement& asConfig)
{
    std::string endPoint = getMandatoryFilledAttribute(virtualLinkConfig, "endPointRouterID");
    Ipv4Address routerId = ipv4AddressFromAddressString(endPoint.c_str());

    EV_DEBUG << "        loading VirtualLink to OSPF router " << routerId.str(false) << "\n";

    Neighbor *neighbor = new Neighbor;
    neighbor->setNeighborID(routerId);

    Ospfv2Interface *intf = new Ospfv2Interface;

    intf->setType(Ospfv2Interface::VIRTUAL);
    intf->setInterfaceName("virtual");
    intf->addNeighbor(neighbor);
    intf->setTransitAreaId(ipv4AddressFromAddressString(getMandatoryFilledAttribute(virtualLinkConfig, "transitAreaID")));
    intf->setRetransmissionInterval(getIntAttrOrPar(virtualLinkConfig, "retransmissionInterval"));
    intf->setTransmissionDelay(getIntAttrOrPar(virtualLinkConfig, "interfaceTransmissionDelay"));
    intf->setHelloInterval(getIntAttrOrPar(virtualLinkConfig, "helloInterval"));
    intf->setRouterDeadInterval(getIntAttrOrPar(virtualLinkConfig, "routerDeadInterval"));

    const char* ospfCrcMode = par("crcMode");
    intf->setCrcMode(parseCrcMode(ospfCrcMode, false));

    loadAuthenticationConfig(intf, virtualLinkConfig);

    AreaId transitAreaId = intf->getTransitAreaId();
    Ospfv2Area *transitArea = ospfRouter->getAreaByID(transitAreaId);
    if (!transitArea) {
        delete intf;
        throw cRuntimeError("Virtual link to router %s cannot be configured through a non-existence transit area '%s' at %s", routerId.str(false).c_str(), transitAreaId.str(false).c_str(), virtualLinkConfig.getSourceLocation());
    }
    else if (!transitArea->getExternalRoutingCapability()) {
        delete intf;
        throw cRuntimeError("Virtual link to router %s cannot be configured through a stub area '%s' at %s", routerId.str(false).c_str(), transitAreaId.str(false).c_str(), virtualLinkConfig.getSourceLocation());
    }

    // add the virtual link to the OSPF data structure
    Ospfv2Area *backbone = ospfRouter->getAreaByID(BACKBONE_AREAID);
    if (!backbone) {
        loadAreaFromXML(asConfig, BACKBONE_AREAID);
        backbone = ospfRouter->getAreaByID(BACKBONE_AREAID);
    }
    backbone->addInterface(intf);
}

void Ospfv2ConfigReader::loadAuthenticationConfig(Ospfv2Interface *intf, const cXMLElement& ifConfig)
{
    std::string authenticationType = getStrAttrOrPar(ifConfig, "authenticationType");
    if (authenticationType == "SimplePasswordType")
        intf->setAuthenticationType(SIMPLE_PASSWORD_TYPE);
    else if (authenticationType == "CrytographicType")
        intf->setAuthenticationType(CRYTOGRAPHIC_TYPE);
    else if (authenticationType == "NullType")
        intf->setAuthenticationType(NULL_TYPE);
    else
        throw cRuntimeError("Invalid AuthenticationType '%s' at %s", authenticationType.c_str(), ifConfig.getSourceLocation());

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

void Ospfv2ConfigReader::initiateDefaultRouteDistribution()
{
    Ipv4Route *entry = ospfRouter->getDefaultRoute();
    // if a default route exist on this router
    if(entry) {
        EV_DEBUG << "        distributing the default route. \n";

        Ipv4AddressRange networkAddress;
        networkAddress.address = ipv4AddressFromAddressString("0.0.0.0");
        networkAddress.mask = ipv4NetmaskFromAddressString("0.0.0.0");
        networkAddress.address = networkAddress.address & networkAddress.mask;

        Ospfv2AsExternalLsaContents asExternalRoute;
        asExternalRoute.setNetworkMask(networkAddress.mask);
        // default route is advertised with cost of 1 of 'type 2' external metric
        asExternalRoute.setExternalTOSInfoArraySize(1);
        auto& tosInfo = asExternalRoute.getExternalTOSInfoForUpdate(0);
        tosInfo.E_ExternalMetricType = true;
        tosInfo.tos = 0;
        tosInfo.externalRouteTag = 0;
        tosInfo.forwardingAddress = ipv4AddressFromAddressString("0.0.0.0");
        tosInfo.routeCost = 1;

        // add the external route to the OSPF data structure
        ospfRouter->updateExternalRoute(networkAddress.address, asExternalRoute);
    }
}

cXMLElement * Ospfv2ConfigReader::findMatchingConfig(const cXMLElementList& routerConfig, const InterfaceEntry& intf)
{
    for (auto & ifConfig : routerConfig) {
        std::string nodeName = ifConfig->getTagName();
        if ((nodeName == "PointToPointInterface") ||
                (nodeName == "BroadcastInterface") ||
                (nodeName == "NBMAInterface") ||
                (nodeName == "PointToMultiPointInterface") ||
                (nodeName == "LoopbackInterface"))
        {
            const char *ifName = (*ifConfig).getAttribute("ifName");
            if (ifName && *ifName) {
                inet::PatternMatcher pattern(ifName, true, true, true);
                if (pattern.matches(intf.getFullName()) ||
                        pattern.matches(intf.getInterfaceFullPath().c_str()) ||
                        pattern.matches(intf.getInterfaceName())) {
                    return ifConfig;
                }

                continue;
            }

            const char *toward = getMandatoryFilledAttribute(*ifConfig, "toward");
            cModule *destnode = getSimulation()->getSystemModule()->getModuleByPath(toward);
            if (!destnode)
                throw cRuntimeError("toward module `%s' not found at %s", toward, (*ifConfig).getSourceLocation());

            int gateId = intf.getNodeOutputGateId();
            if ((gateId != -1) && ift->getHostModule()->gate(gateId)->pathContains(destnode))
                return ifConfig;
        }
    }

    return nullptr;
}

std::vector<InterfaceEntry *> Ospfv2ConfigReader::getInterfaceByXMLAttributesOf(const cXMLElement& ifConfig)
{
    std::vector<InterfaceEntry *> results;
    const char *ifName = ifConfig.getAttribute("ifName");
    if (ifName && *ifName) {
        inet::PatternMatcher pattern(ifName, true, true, true);
        for (int n = 0; n < ift->getNumInterfaces(); n++) {
            InterfaceEntry *intf = ift->getInterface(n);
            if (pattern.matches(intf->getFullName()) ||
                    pattern.matches(intf->getInterfaceFullPath().c_str()) ||
                    pattern.matches(intf->getInterfaceName())) {
                results.push_back(intf);
            }
        }
        return results;
    }

    const char *toward = getMandatoryFilledAttribute(ifConfig, "toward");
    cModule *destnode = getSimulation()->getSystemModule()->getModuleByPath(toward);
    if (!destnode)
        throw cRuntimeError("'ifName' or 'toward' module `%s' not found at %s", toward, ifConfig.getSourceLocation());

    cModule *host = ift->getHostModule();
    for (int i = 0; i < ift->getNumInterfaces(); i++) {
        InterfaceEntry *ie = ift->getInterface(i);
        if (ie) {
            int gateId = ie->getNodeOutputGateId();
            if ((gateId != -1) && (host->gate(gateId)->pathContains(destnode))) {
                results.push_back(ie);
                return results;
            }
        }
    }
    throw cRuntimeError("Error reading XML config: IInterfaceTable contains no interface toward '%s' at %s", toward, ifConfig.getSourceLocation());
}

int Ospfv2ConfigReader::getIntAttrOrPar(const cXMLElement& ifConfig, const char *name) const
{
    const char *attrStr = ifConfig.getAttribute(name);
    if (attrStr && *attrStr)
        return atoi(attrStr);
    return par(name);
}

bool Ospfv2ConfigReader::getBoolAttrOrPar(const cXMLElement& ifConfig, const char *name) const
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

const char *Ospfv2ConfigReader::getStrAttrOrPar(const cXMLElement& ifConfig, const char *name) const
{
    const char *attrStr = ifConfig.getAttribute(name);
    if (attrStr && *attrStr)
        return attrStr;
    return par(name);
}

void Ospfv2ConfigReader::joinMulticastGroups(int interfaceId)
{
    InterfaceEntry *ie = ift->getInterfaceById(interfaceId);
    if (!ie)
        throw cRuntimeError("Interface id=%d does not exist", interfaceId);
    if (!ie->isMulticast())
        return;
    auto& ipv4Data = ie->findProtocolDataForUpdate<Ipv4InterfaceData>();
    if (!ipv4Data)
        throw cRuntimeError("Interface %s (id=%d) does not have Ipv4 data", ie->getInterfaceName(), interfaceId);
    ipv4Data->joinMulticastGroup(Ipv4Address::ALL_OSPF_ROUTERS_MCAST);
    ipv4Data->joinMulticastGroup(Ipv4Address::ALL_OSPF_DESIGNATED_ROUTERS_MCAST);
}

} // namespace ospfv2
} // namespace inet

