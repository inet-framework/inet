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

#include "OSPFRouting.h"
#include "IPAddress.h"
#include "IPAddressResolver.h"
#include "IPControlInfo.h"
#include "OSPFcommon.h"
#include "OSPFArea.h"
#include "OSPFInterface.h"
#include "MessageHandler.h"
#include "RoutingTableAccess.h"
#include "InterfaceTableAccess.h"
#include <string>
#include <map>
#include <stdlib.h>
#include <memory.h>


Define_Module(OSPFRouting);


OSPFRouting::OSPFRouting()
{
    ospfRouter = NULL;
}

/**
 * Destructor.
 * Deletes the whole OSPF datastructure.
 */
OSPFRouting::~OSPFRouting(void)
{
    delete ospfRouter;
}


/**
 * OMNeT++ init method.
 * Runs at stage 2 after interfaces are registered(stage 0) and the routing
 * table is initialized(stage 1). Loads OSPF configuration information from
 * the config XML.
 * @param stage [in] The initialization stage.
 */
void OSPFRouting::initialize(int stage)
{
    // we have to wait for stage 2 until interfaces get registered(stage 0)
    // and routerId gets assigned(stage 3)
    if (stage == 4)
    {
        rt = RoutingTableAccess().get();
        ift = InterfaceTableAccess().get();

        // Get routerId
        ospfRouter = new OSPF::Router(rt->getRouterId().getInt(), this);

        // read the OSPF AS configuration
        const char *fileName = par("ospfConfigFile");
        if (fileName == NULL || (!strcmp(fileName, "")) || !LoadConfigFromXML(fileName))
            error("Error reading AS configuration from file %s", fileName);

        ospfRouter->AddWatches();
    }
}


/**
 * Forwards OSPF messages to the message handler object of the OSPF datastructure.
 * @param msg [in] The OSPF message.
 */
void OSPFRouting::handleMessage(cMessage *msg)
{
//    if (simulation.getEventNumber() == 90591) {
//        __asm int 3;
//    }
    ospfRouter->GetMessageHandler()->MessageReceived(msg);
}

/**
 * Looks up the interface name in IInterfaceTable, and returns interfaceId a.k.a ifIndex.
 */
int OSPFRouting::ResolveInterfaceName(const std::string& name) const
{
    InterfaceEntry* ie = ift->getInterfaceByName(name.c_str());
    if (!ie)
        opp_error("error reading XML config: IInterfaceTable contains no interface named '%s'", name.c_str());
    return ie->getInterfaceId();
}

/**
 * Loads a list of OSPF Areas connected to this router from the config XML.
 * @param routerNode [in]  XML node describing this router.
 * @param areaList   [out] A hash of OSPF Areas connected to this router. The hash key is the Area ID.
 */
void OSPFRouting::GetAreaListFromXML(const cXMLElement& routerNode, std::map<std::string, int>& areaList) const
{
    cXMLElementList routerConfig = routerNode.getChildren();
    for (cXMLElementList::iterator routerConfigIt = routerConfig.begin(); routerConfigIt != routerConfig.end(); routerConfigIt++) {
        std::string nodeName = (*routerConfigIt)->getTagName();
        if ((nodeName == "PointToPointInterface") ||
            (nodeName == "BroadcastInterface") ||
            (nodeName == "NBMAInterface") ||
            (nodeName == "PointToMultiPointInterface"))
        {
            std::string areaId = (*routerConfigIt)->getChildrenByTagName("AreaID")[0]->getNodeValue();
            if (areaList.find(areaId) == areaList.end()) {
                areaList[areaId] = 1;
            }
        }
    }
}


/**
 * Loads basic configuration information for a given area from the config XML.
 * Reads the configured address ranges, and whether this Area should be handled as a stub Area.
 * @param asConfig [in] XML node describing the configuration of the whole Autonomous System.
 * @param areaID   [in] The Area to be added to the OSPF datastructure.
 */
void OSPFRouting::LoadAreaFromXML(const cXMLElement& asConfig, const std::string& areaID)
{
    std::string areaXPath("Area[@id='");
    areaXPath += areaID;
    areaXPath += "']";

    cXMLElement* areaConfig = asConfig.getElementByPath(areaXPath.c_str());
    if (areaConfig == NULL) {
        error("No configuration for Area ID: %s", areaID.c_str());
    }
    else {
        EV << "    loading info for Area id = " << areaID << "\n";
    }

    OSPF::Area* area = new OSPF::Area(ULongFromAddressString(areaID.c_str()));
    cXMLElementList areaDetails = areaConfig->getChildren();
    for (cXMLElementList::iterator arIt = areaDetails.begin(); arIt != areaDetails.end(); arIt++) {
        std::string nodeName = (*arIt)->getTagName();
        if (nodeName == "AddressRange") {
            OSPF::IPv4AddressRange addressRange;
            addressRange.address = IPv4AddressFromAddressString((*arIt)->getChildrenByTagName("Address")[0]->getNodeValue());
            addressRange.mask = IPv4AddressFromAddressString((*arIt)->getChildrenByTagName("Mask")[0]->getNodeValue());
            std::string status = (*arIt)->getChildrenByTagName("Status")[0]->getNodeValue();
            if (status == "Advertise") {
                area->AddAddressRange(addressRange, true);
            } else {
                area->AddAddressRange(addressRange, false);
            }
        }
        if ((nodeName == "Stub") && (areaID != "0.0.0.0")) {    // the backbone cannot be configured as a stub
            area->SetExternalRoutingCapability(false);
            area->SetStubDefaultCost(atoi((*arIt)->getChildrenByTagName("DefaultCost")[0]->getNodeValue()));
        }
    }
    // Add the Area to the router
    ospfRouter->AddArea(area);
}


/**
 * Loads OSPF configuration information for a router interface.
 * Handles PointToPoint, Broadcast, NBMA and PointToMultiPoint interfaces.
 * @param ifConfig [in] XML node describing the configuration of an OSPF interface.
 */
void OSPFRouting::LoadInterfaceParameters(const cXMLElement& ifConfig)
{
    OSPF::Interface* intf          = new OSPF::Interface;
    std::string      ifName        = ifConfig.getAttribute("ifName");
    int              ifIndex       = ResolveInterfaceName(ifName);
    std::string      interfaceType = ifConfig.getTagName();

    EV << "        loading " << interfaceType << " " << ifName << " ifIndex[" << ifIndex << "]\n";

    intf->SetIfIndex(ifIndex);
    if (interfaceType == "PointToPointInterface") {
        intf->SetType(OSPF::Interface::PointToPoint);
    } else if (interfaceType == "BroadcastInterface") {
        intf->SetType(OSPF::Interface::Broadcast);
    } else if (interfaceType == "NBMAInterface") {
        intf->SetType(OSPF::Interface::NBMA);
    } else if (interfaceType == "PointToMultiPointInterface") {
        intf->SetType(OSPF::Interface::PointToMultiPoint);
    } else {
        delete intf;
        error("Loading %s ifIndex[%d] aborted", interfaceType.c_str(), ifIndex);
    }

    OSPF::AreaID    areaID    = 0;
    cXMLElementList ifDetails = ifConfig.getChildren();

    for (cXMLElementList::iterator ifElemIt = ifDetails.begin(); ifElemIt != ifDetails.end(); ifElemIt++) {
        std::string nodeName = (*ifElemIt)->getTagName();
        if (nodeName == "AreaID") {
            areaID = ULongFromAddressString((*ifElemIt)->getNodeValue());
            intf->SetAreaID(areaID);
        }
        if (nodeName == "InterfaceOutputCost") {
            intf->SetOutputCost(atoi((*ifElemIt)->getNodeValue()));
        }
        if (nodeName == "RetransmissionInterval") {
            intf->SetRetransmissionInterval(atoi((*ifElemIt)->getNodeValue()));
        }
        if (nodeName == "InterfaceTransmissionDelay") {
            intf->SetTransmissionDelay(atoi((*ifElemIt)->getNodeValue()));
        }
        if (nodeName == "RouterPriority") {
            intf->SetRouterPriority(atoi((*ifElemIt)->getNodeValue()));
        }
        if (nodeName == "HelloInterval") {
            intf->SetHelloInterval(atoi((*ifElemIt)->getNodeValue()));
        }
        if (nodeName == "RouterDeadInterval") {
            intf->SetRouterDeadInterval(atoi((*ifElemIt)->getNodeValue()));
        }
        if (nodeName == "AuthenticationType") {
            std::string authenticationType = (*ifElemIt)->getNodeValue();
            if (authenticationType == "SimplePasswordType") {
                intf->SetAuthenticationType(OSPF::SimplePasswordType);
            } else if (authenticationType == "CrytographicType") {
                intf->SetAuthenticationType(OSPF::CrytographicType);
            } else {
                intf->SetAuthenticationType(OSPF::NullType);
            }
        }
        if (nodeName == "AuthenticationKey") {
            std::string key = (*ifElemIt)->getNodeValue();
            OSPF::AuthenticationKeyType keyValue;
            memset(keyValue.bytes, 0, 8 * sizeof(char));
            int keyLength = key.length();
            if ((keyLength > 4) && (keyLength <= 18) && (keyLength % 2 == 0) && (key[0] == '0') && (key[1] == 'x')) {
                for (int i = keyLength; (i > 2); i -= 2) {
                    keyValue.bytes[(i - 2) / 2] = HexPairToByte(key[i - 1], key[i]);
                }
            }
            intf->SetAuthenticationKey(keyValue);
        }
        if (nodeName == "PollInterval") {
            intf->SetPollInterval(atoi((*ifElemIt)->getNodeValue()));
        }
        if ((interfaceType == "NBMAInterface") && (nodeName == "NBMANeighborList")) {
            cXMLElementList neighborList = (*ifElemIt)->getChildren();
            for (cXMLElementList::iterator neighborIt = neighborList.begin(); neighborIt != neighborList.end(); neighborIt++) {
                std::string neighborNodeName = (*neighborIt)->getTagName();
                if (neighborNodeName == "NBMANeighbor") {
                    OSPF::Neighbor* neighbor = new OSPF::Neighbor;
                    neighbor->SetAddress(IPv4AddressFromAddressString((*neighborIt)->getChildrenByTagName("NetworkInterfaceAddress")[0]->getNodeValue()));
                    neighbor->SetPriority(atoi((*neighborIt)->getChildrenByTagName("NeighborPriority")[0]->getNodeValue()));
                    intf->AddNeighbor(neighbor);
                }
            }
        }
        if ((interfaceType == "PointToMultiPointInterface") && (nodeName == "PointToMultiPointNeighborList")) {
            cXMLElementList neighborList = (*ifElemIt)->getChildren();
            for (cXMLElementList::iterator neighborIt = neighborList.begin(); neighborIt != neighborList.end(); neighborIt++) {
                std::string neighborNodeName = (*neighborIt)->getTagName();
                if (neighborNodeName == "PointToMultiPointNeighbor") {
                    OSPF::Neighbor* neighbor = new OSPF::Neighbor;
                    neighbor->SetAddress(IPv4AddressFromAddressString((*neighborIt)->getNodeValue()));
                    intf->AddNeighbor(neighbor);
                }
            }
        }

    }
    // add the interface to it's Area
    OSPF::Area* area = ospfRouter->GetArea(areaID);
    if (area != NULL) {
        area->AddInterface(intf);
        intf->ProcessEvent(OSPF::Interface::InterfaceUp); // notification should come from the blackboard...
    } else {
        delete intf;
        error("Loading %s ifIndex[%d] in Area %d aborted", interfaceType.c_str(), ifIndex, areaID);
    }
}


/**
 * Loads the configuration information of a route outside of the Autonomous System(external route).
 * @param externalRouteConfig [in] XML node describing the parameters of an external route.
 */
void OSPFRouting::LoadExternalRoute(const cXMLElement& externalRouteConfig)
{
    std::string               ifName  = externalRouteConfig.getAttribute("ifName");
    int                       ifIndex = ResolveInterfaceName(ifName);
    OSPFASExternalLSAContents asExternalRoute;
    OSPF::RoutingTableEntry   externalRoutingEntry; // only used here to keep the path cost calculation in one place
    OSPF::IPv4AddressRange    networkAddress;

    EV << "        loading ExternalInterface " << ifName << " ifIndex[" << ifIndex << "]\n";

    cXMLElementList ifDetails = externalRouteConfig.getChildren();
    for (cXMLElementList::iterator exElemIt = ifDetails.begin(); exElemIt != ifDetails.end(); exElemIt++) {
        std::string nodeName = (*exElemIt)->getTagName();
        if (nodeName == "AdvertisedExternalNetwork") {
            networkAddress.address = IPv4AddressFromAddressString((*exElemIt)->getChildrenByTagName("Address")[0]->getNodeValue());
            networkAddress.mask    = IPv4AddressFromAddressString((*exElemIt)->getChildrenByTagName("Mask")[0]->getNodeValue());
            asExternalRoute.setNetworkMask(ULongFromIPv4Address(networkAddress.mask));
        }
        if (nodeName == "ExternalInterfaceOutputParameters") {
            std::string metricType = (*exElemIt)->getChildrenByTagName("ExternalInterfaceOutputType")[0]->getNodeValue();
            int         routeCost  = atoi((*exElemIt)->getChildrenByTagName("ExternalInterfaceOutputCost")[0]->getNodeValue());

            asExternalRoute.setRouteCost(routeCost);
            if (metricType == "Type2") {
                asExternalRoute.setE_ExternalMetricType(true);
                externalRoutingEntry.SetType2Cost(routeCost);
                externalRoutingEntry.SetPathType(OSPF::RoutingTableEntry::Type2External);
            } else {
                asExternalRoute.setE_ExternalMetricType(false);
                externalRoutingEntry.SetCost(routeCost);
                externalRoutingEntry.SetPathType(OSPF::RoutingTableEntry::Type1External);
            }
        }
        if (nodeName == "ForwardingAddress") {
            asExternalRoute.setForwardingAddress(ULongFromAddressString((*exElemIt)->getNodeValue()));
        }
        if (nodeName == "ExternalRouteTag") {
            std::string externalRouteTag = (*exElemIt)->getNodeValue();
            char        externalRouteTagValue[4];

            memset(externalRouteTagValue, 0, 4 * sizeof(char));
            int externalRouteTagLength = externalRouteTag.length();
            if ((externalRouteTagLength > 4) && (externalRouteTagLength <= 10) && (externalRouteTagLength % 2 == 0) && (externalRouteTag[0] == '0') && (externalRouteTag[1] == 'x')) {
                for (int i = externalRouteTagLength; (i > 2); i -= 2) {
                    externalRouteTagValue[(i - 2) / 2] = HexPairToByte(externalRouteTag[i - 1], externalRouteTag[i]);
                }
            }
            asExternalRoute.setExternalRouteTag((externalRouteTagValue[0] << 24) + (externalRouteTagValue[1] << 16) + (externalRouteTagValue[2] << 8) + externalRouteTagValue[3]);
        }
    }
    // add the external route to the OSPF datastructure
    ospfRouter->UpdateExternalRoute(networkAddress.address, asExternalRoute, ifIndex);
}


/**
 * Loads the configuration of a host getRoute(a host directly connected to the router).
 * @param hostRouteConfig [in] XML node describing the parameters of a host route.
 */
void OSPFRouting::LoadHostRoute(const cXMLElement& hostRouteConfig)
{
    OSPF::HostRouteParameters hostParameters;
    OSPF::AreaID              hostArea;

    std::string ifName = hostRouteConfig.getAttribute("ifName");
    hostParameters.ifIndex = ResolveInterfaceName(ifName);

    EV << "        loading HostInterface " << ifName << " ifIndex[" << static_cast<short> (hostParameters.ifIndex) << "]\n";

    cXMLElementList ifDetails = hostRouteConfig.getChildren();
    for (cXMLElementList::iterator hostElemIt = ifDetails.begin(); hostElemIt != ifDetails.end(); hostElemIt++) {
        std::string nodeName = (*hostElemIt)->getTagName();
        if (nodeName == "AreaID") {
            hostArea = ULongFromAddressString((*hostElemIt)->getNodeValue());
        }
        if (nodeName == "AttachedHost") {
            hostParameters.address = IPv4AddressFromAddressString((*hostElemIt)->getNodeValue());
        }
        if (nodeName == "LinkCost") {
            hostParameters.linkCost = atoi((*hostElemIt)->getNodeValue());
        }
    }
    // add the host route to the OSPF datastructure.
    OSPF::Area* area = ospfRouter->GetArea(hostArea);
    if (area != NULL) {
        area->AddHostRoute(hostParameters);

    } else {
        error("Loading HostInterface ifIndex[%d] in Area %d aborted", hostParameters.ifIndex, hostArea);
    }
}


/**
 * Loads the configuration of an OSPf virtual link(virtual connection between two backbone routers).
 * @param virtualLinkConfig [in] XML node describing the parameters of a virtual link.
 */
void OSPFRouting::LoadVirtualLink(const cXMLElement& virtualLinkConfig)
{
    OSPF::Interface* intf     = new OSPF::Interface;
    std::string      endPoint = virtualLinkConfig.getAttribute("endPointRouterID");
    OSPF::Neighbor*  neighbor = new OSPF::Neighbor;

    EV << "        loading VirtualLink to " << endPoint << "\n";

    intf->SetType(OSPF::Interface::Virtual);
    neighbor->SetNeighborID(ULongFromAddressString(endPoint.c_str()));
    intf->AddNeighbor(neighbor);

    cXMLElementList ifDetails = virtualLinkConfig.getChildren();
    for (cXMLElementList::iterator ifElemIt = ifDetails.begin(); ifElemIt != ifDetails.end(); ifElemIt++) {
        std::string nodeName = (*ifElemIt)->getTagName();
        if (nodeName == "TransitAreaID") {
            intf->SetTransitAreaID(ULongFromAddressString((*ifElemIt)->getNodeValue()));
        }
        if (nodeName == "RetransmissionInterval") {
            intf->SetRetransmissionInterval(atoi((*ifElemIt)->getNodeValue()));
        }
        if (nodeName == "InterfaceTransmissionDelay") {
            intf->SetTransmissionDelay(atoi((*ifElemIt)->getNodeValue()));
        }
        if (nodeName == "HelloInterval") {
            intf->SetHelloInterval(atoi((*ifElemIt)->getNodeValue()));
        }
        if (nodeName == "RouterDeadInterval") {
            intf->SetRouterDeadInterval(atoi((*ifElemIt)->getNodeValue()));
        }
        if (nodeName == "AuthenticationType") {
            std::string authenticationType = (*ifElemIt)->getNodeValue();
            if (authenticationType == "SimplePasswordType") {
                intf->SetAuthenticationType(OSPF::SimplePasswordType);
            } else if (authenticationType == "CrytographicType") {
                intf->SetAuthenticationType(OSPF::CrytographicType);
            } else {
                intf->SetAuthenticationType(OSPF::NullType);
            }
        }
        if (nodeName == "AuthenticationKey") {
            std::string key = (*ifElemIt)->getNodeValue();
            OSPF::AuthenticationKeyType keyValue;
            memset(keyValue.bytes, 0, 8 * sizeof(char));
            int keyLength = key.length();
            if ((keyLength > 4) && (keyLength <= 18) && (keyLength % 2 == 0) && (key[0] == '0') && (key[1] == 'x')) {
                for (int i = keyLength; (i > 2); i -= 2) {
                    keyValue.bytes[(i - 2) / 2] = HexPairToByte(key[i - 1], key[i]);
                }
            }
            intf->SetAuthenticationKey(keyValue);
        }
    }

    // add the virtual link to the OSPF datastructure.
    OSPF::Area* transitArea = ospfRouter->GetArea(intf->GetAreaID());
    OSPF::Area* backbone    = ospfRouter->GetArea(OSPF::BackboneAreaID);

    if ((backbone != NULL) && (transitArea != NULL) && (transitArea->GetExternalRoutingCapability())) {
        backbone->AddInterface(intf);
    } else {
        delete intf;
        error("Loading VirtualLink to %s through Area %d aborted", endPoint.c_str(), intf->GetAreaID());
    }
}


/**
 * Loads the configuration of the OSPF datastructure from the config XML.
 * @param filename [in] The path of the XML config file.
 * @return True if the configuration was succesfully loaded.
 * @throws an getError() otherwise.
 */
bool OSPFRouting::LoadConfigFromXML(const char * filename)
{
    cXMLElement* asConfig = ev.getXMLDocument(filename);
    if (asConfig == NULL) {
        error("Cannot read AS configuration from file: %s", filename);
    }

    // load information on this router
    std::string routerXPath("Router[@id='");
    IPAddress routerId(ospfRouter->GetRouterID());
    routerXPath += routerId.str();
    routerXPath += "']";

    cXMLElement* routerNode = asConfig->getElementByPath(routerXPath.c_str());
    if (routerNode == NULL) {
        error("No configuration for Router ID: %s", routerId.str().c_str());
    }
    else {
        EV << "OSPFRouting: Loading info for Router id = " << routerId.str() << "\n";
    }

    if (routerNode->getChildrenByTagName("RFC1583Compatible").size() > 0) {
        ospfRouter->SetRFC1583Compatibility(true);
    }

    std::map<std::string, int> areaList;
    GetAreaListFromXML(*routerNode, areaList);

    // load area information
    for (std::map<std::string, int>::iterator areaIt = areaList.begin(); areaIt != areaList.end(); areaIt++) {
        LoadAreaFromXML(*asConfig, areaIt->first);
    }
    // if the router is an area border router then it MUST be part of the backbone(area 0)
    if ((areaList.size() > 1) && (areaList.find("0.0.0.0") == areaList.end())) {
        LoadAreaFromXML(*asConfig, "0.0.0.0");
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
            LoadInterfaceParameters(*(*routerConfigIt));
        }
        if (nodeName == "ExternalInterface") {
            LoadExternalRoute(*(*routerConfigIt));
        }
        if (nodeName == "HostInterface") {
            LoadHostRoute(*(*routerConfigIt));
        }
        if (nodeName == "VirtualLink") {
            LoadVirtualLink(*(*routerConfigIt));
        }
    }
    return true;
}
