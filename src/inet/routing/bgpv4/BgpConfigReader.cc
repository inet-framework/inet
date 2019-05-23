//
// Copyright (C) 2010 Helene Lageber
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

#include "inet/common/ModuleAccess.h"
#include "inet/routing/bgpv4/BgpConfigReader.h"
#include "inet/routing/bgpv4/BgpSession.h"

namespace inet {

namespace bgp {

BgpConfigReader::BgpConfigReader(cModule *bgpModule, IInterfaceTable *ift) :
        bgpModule(bgpModule), ift(ift)
{
}

void BgpConfigReader::loadConfigFromXML(cXMLElement *bgpConfig, BgpRouter *bgpRouter)
{
    this->bgpRouter = bgpRouter;

    if (strcmp(bgpConfig->getTagName(), "BGPConfig"))
        throw cRuntimeError("Cannot read BGP configuration, unaccepted '%s' node at %s", bgpConfig->getTagName(), bgpConfig->getSourceLocation());

    // load bgp timer parameters informations
    cXMLElement *paramNode = bgpConfig->getElementByPath("TimerParams");
    if (paramNode == nullptr)
        throw cRuntimeError("BGP Error: No configuration for BGP timer parameters");
    cXMLElementList timerConfig = paramNode->getChildren();
    simtime_t delayTab[NB_TIMERS];
    loadTimerConfig(timerConfig, delayTab);

    // find my AS
    cXMLElementList ASList = bgpConfig->getElementsByTagName("AS");
    int routerPosition;
    AsId myAsId = findMyAS(ASList, routerPosition);
    if (myAsId == 0)
        throw cRuntimeError("BGP Error:  No AS configuration for Router ID: %s", bgpRouter->getRouterId().str().c_str());

    bgpRouter->setAsId(myAsId);

    // load AS information
    char ASXPath[32];
    sprintf(ASXPath, "AS[@id='%d']", myAsId);
    cXMLElement *ASNode = bgpConfig->getElementByPath(ASXPath);
    if (ASNode == nullptr)
        throw cRuntimeError("BGP Error:  No configuration for AS ID: %d", myAsId);
    cXMLElementList ASConfig = ASNode->getChildren();

    // load EGP Session informations
    cXMLElementList sessionList = bgpConfig->getElementsByTagName("Session");
    simtime_t saveStartDelay = delayTab[3];
    loadEbgpSessionConfig(ASConfig, sessionList, delayTab);
    delayTab[3] = saveStartDelay;

    // get all BGP speakers in my AS
    auto routerInSameASList = findInternalPeers(ASConfig);

    // create an IGP Session with each BGP speaker in my AS
    if (routerInSameASList.size()) {
        unsigned int routerPeerPosition = 1;
        delayTab[3] += sessionList.size() * 2;
        for (auto it = routerInSameASList.begin(); it != routerInSameASList.end(); it++, routerPeerPosition++) {
            SessionId newSessionID = bgpRouter->createIbgpSession(*it /*peer address*/);
            delayTab[3] += calculateStartDelay(routerInSameASList.size(), routerPosition, routerPeerPosition);
            bgpRouter->setTimer(newSessionID, delayTab);
            bgpRouter->setSocketListen(newSessionID);
        }
    }

    // should be called after all (E-BGP/I-BGP) sessions are created
    loadASConfig(ASConfig);
}

void BgpConfigReader::loadTimerConfig(cXMLElementList& timerConfig, simtime_t *delayTab)
{
    for (auto & elem : timerConfig) {
        std::string nodeName = (elem)->getTagName();
        if (nodeName == "connectRetryTime") {
            delayTab[0] = (double)atoi((elem)->getNodeValue());
        }
        else if (nodeName == "holdTime") {
            delayTab[1] = (double)atoi((elem)->getNodeValue());
        }
        else if (nodeName == "keepAliveTime") {
            delayTab[2] = (double)atoi((elem)->getNodeValue());
        }
        else if (nodeName == "startDelay") {
            delayTab[3] = (double)atoi((elem)->getNodeValue());
        }
    }
}

AsId BgpConfigReader::findMyAS(cXMLElementList& asList, int& outRouterPosition)
{
    // find my own Ipv4 address in the configuration file and return the AS id under which it is configured
    // and also the 1 based position of the entry inside the AS config element
    for (auto & elem : asList) {
        cXMLElementList routerList = (elem)->getChildrenByTagName("Router");
        outRouterPosition = 1;
        for (auto & routerList_routerListIt : routerList) {
            Ipv4Address routerAddr = Ipv4Address((routerList_routerListIt)->getAttribute("interAddr"));
            for (int i = 0; i < ift->getNumInterfaces(); i++) {
                if (ift->getInterface(i)->getProtocolData<Ipv4InterfaceData>()->getIPAddress() == routerAddr)
                    return atoi((routerList_routerListIt)->getParentNode()->getAttribute("id"));
            }
            outRouterPosition++;
        }
    }

    return 0;
}

void BgpConfigReader::loadEbgpSessionConfig(cXMLElementList& ASConfig, cXMLElementList& sessionList, simtime_t *delayTab)
{
    simtime_t saveStartDelay = delayTab[3];
    for (auto sessionListIt = sessionList.begin(); sessionListIt != sessionList.end(); sessionListIt++, delayTab[3] = saveStartDelay) {
        auto numRouters = (*sessionListIt)->getChildren();
        if(numRouters.size() != 2)
            throw cRuntimeError("BGP Error: Number of routers is invalid for session ID : %s", (*sessionListIt)->getAttribute("id"));

        Ipv4Address routerAddr1 = Ipv4Address((*sessionListIt)->getFirstChild()->getAttribute("exterAddr"));
        Ipv4Address routerAddr2 = Ipv4Address((*sessionListIt)->getLastChild()->getAttribute("exterAddr"));
        if (isInInterfaceTable(ift, routerAddr1) == -1 && isInInterfaceTable(ift, routerAddr2) == -1)
            continue;

        Ipv4Address peerAddr;
        Ipv4Address myAddr;
        if (isInInterfaceTable(ift, routerAddr1) != -1) {
            peerAddr = routerAddr2;
            myAddr = routerAddr1;
            delayTab[3] += atoi((*sessionListIt)->getAttribute("id"));
        }
        else {
            peerAddr = routerAddr1;
            myAddr = routerAddr2;
            delayTab[3] += atoi((*sessionListIt)->getAttribute("id")) + bgpModule->par("ExternalPeerStartDelayOffset").doubleValue();
        }

        if (peerAddr.isUnspecified())
            throw cRuntimeError("BGP Error: No valid external address for session ID : %s", (*sessionListIt)->getAttribute("id"));

        SessionInfo externalInfo;

        externalInfo.myAddr = myAddr;
        externalInfo.checkConnection = bgpModule->par("connectedCheck").boolValue();
        externalInfo.ebgpMultihop = bgpModule->par("ebgpMultihop").intValue();
        if(externalInfo.ebgpMultihop < 1)
            throw cRuntimeError("BGP Error: ebgpMultihop parameter must be >= 1");
        else if(externalInfo.ebgpMultihop > 1) // if E-BGP multi-hop is enabled, then turn off checkConnection
            externalInfo.checkConnection = false;

        for (auto & elem : ASConfig) {
            if (std::string(elem->getTagName()) == "Router") {
                if (isInInterfaceTable(ift, Ipv4Address(elem->getAttribute("interAddr"))) != -1) {
                    for (auto & entry : elem->getChildren()) {
                        if(std::string(entry->getTagName()) == "Neighbor") {
                            const char *peer = entry->getAttribute("address");
                            if(peer && *peer && peerAddr.equals(Ipv4Address(peer))) {
                                externalInfo.checkConnection = getBoolAttrOrPar(*entry, "connectedCheck");
                                externalInfo.ebgpMultihop = getIntAttrOrPar(*entry, "ebgpMultihop");
                                if(externalInfo.ebgpMultihop > 1) // if E-BGP multi-hop is enabled, then turn off checkConnection
                                    externalInfo.checkConnection = false;
                            }
                        }
                    }
                }
            }
        }

        SessionId newSessionID = bgpRouter->createEbgpSession(peerAddr.str().c_str(), externalInfo);
        bgpRouter->setTimer(newSessionID, delayTab);
        bgpRouter->setSocketListen(newSessionID);
    }
}

std::vector<const char *> BgpConfigReader::findInternalPeers(cXMLElementList& ASConfig)
{
    std::vector<const char *> routerInSameASList;
    for (auto & elem : ASConfig) {
        std::string nodeName = elem->getTagName();
        if (nodeName == "Router") {
            Ipv4Address internalAddr = Ipv4Address(elem->getAttribute("interAddr"));
            if (isInInterfaceTable(ift, internalAddr) == -1)
                routerInSameASList.push_back(elem->getAttribute("interAddr"));
            else
                bgpRouter->setInternalAddress(internalAddr);
        }
    }
    return routerInSameASList;
}

void BgpConfigReader::loadASConfig(cXMLElementList& ASConfig)
{
    // set the default values
    bgpRouter->setDefaultConfig();

    for (auto & elem : ASConfig) {
        std::string nodeName = elem->getTagName();
        if (nodeName == "Router") {
            Ipv4Address internalAddr = Ipv4Address(elem->getAttribute("interAddr"));
            if (isInInterfaceTable(ift, internalAddr) != -1) {
                bgpRouter->setRedistributeInternal(getBoolAttrOrPar(*elem, "redistributeInternal"));
                bgpRouter->setRedistributeOspf(getStrAttrOrPar(*elem, "redistributeOspf"));
                bgpRouter->setRedistributeRip(getBoolAttrOrPar(*elem, "redistributeRip"));

                for (auto & entry : elem->getChildren()) {
                    std::string nodeName = entry->getTagName();
                    if (nodeName == "Network") {
                        const char *address = entry->getAttribute("address");
                        if(address && *address)
                            bgpRouter->addToAdvertiseList(Ipv4Address(address));
                        else
                            throw cRuntimeError("BGP Error: attribute 'address' is mandatory in 'Network'");
                    }
                    else if(nodeName == "Neighbor") {
                        const char *peer = entry->getAttribute("address");
                        if(peer && *peer) {
                            bool nextHopSelf = getBoolAttrOrPar(*entry, "nextHopSelf");
                            bgpRouter->setNextHopSelf(Ipv4Address(peer), nextHopSelf);

                            int localPreference = getIntAttrOrPar(*entry, "localPreference");
                            bgpRouter->setLocalPreference(Ipv4Address(peer), localPreference);
                        }
                        else
                            throw cRuntimeError("BGP Error: attribute 'address' is mandatory in 'Neighbor'");
                    }
                    else
                        throw cRuntimeError("BGP Error: attribute '%s' is invalid in 'Router'", nodeName.c_str());
                }
            }
        }
        else if (nodeName == "DenyRoute" || nodeName == "DenyRouteIN" || nodeName == "DenyRouteOUT") {
            BgpRoutingTableEntry *entry = new BgpRoutingTableEntry();     //FIXME Who will delete this entry?
            entry->setDestination(Ipv4Address((elem)->getAttribute("Address")));
            entry->setNetmask(Ipv4Address((elem)->getAttribute("Netmask")));
            bgpRouter->addToPrefixList(nodeName, entry);
        }
        else if (nodeName == "DenyAS" || nodeName == "DenyASIN" || nodeName == "DenyASOUT") {
            AsId ASCur = atoi((elem)->getNodeValue());
            bgpRouter->addToAsList(nodeName, ASCur);
        }
        else
            throw cRuntimeError("BGP Error: unknown element named '%s' for AS %u", nodeName.c_str(), bgpRouter->getAsId());
    }
}

int BgpConfigReader::isInInterfaceTable(IInterfaceTable *ifTable, Ipv4Address addr)
{
    for (int i = 0; i < ifTable->getNumInterfaces(); i++) {
        if (ifTable->getInterface(i)->getProtocolData<Ipv4InterfaceData>()->getIPAddress() == addr) {
            return i;
        }
    }
    return -1;
}

int BgpConfigReader::isInInterfaceTable(IInterfaceTable *ifTable, std::string ifName)
{
    for (int i = 0; i < ifTable->getNumInterfaces(); i++) {
        if (std::string(ifTable->getInterface(i)->getInterfaceName()) == ifName) {
            return i;
        }
    }
    return -1;
}

unsigned int BgpConfigReader::calculateStartDelay(int rtListSize, unsigned char rtPosition, unsigned char rtPeerPosition)
{
    unsigned int startDelay = 0;
    if (rtPeerPosition == 1) {
        if (rtPosition == 1) {
            startDelay = 1;
        }
        else {
            startDelay = (rtPosition - 1) * 2;
        }
        return startDelay;
    }

    if (rtPosition < rtPeerPosition) {
        startDelay = 2;
    }
    else if (rtPosition > rtPeerPosition) {
        startDelay = (rtListSize - 1) * 2 - 2 * (rtPeerPosition - 2);
    }
    else {
        startDelay = (rtListSize - 1) * 2 + 1;
    }
    return startDelay;
}

bool BgpConfigReader::getBoolAttrOrPar(const cXMLElement& ifConfig, const char *name) const
{
    const char *attrStr = ifConfig.getAttribute(name);
    if (attrStr && *attrStr) {
        if (strcmp(attrStr, "true") == 0 || strcmp(attrStr, "1") == 0)
            return true;
        if (strcmp(attrStr, "false") == 0 || strcmp(attrStr, "0") == 0)
            return false;
        throw cRuntimeError("Invalid boolean attribute %s = '%s' at %s", name, attrStr, ifConfig.getSourceLocation());
    }
    return bgpModule->par(name);
}

int BgpConfigReader::getIntAttrOrPar(const cXMLElement& ifConfig, const char *name) const
{
    const char *attrStr = ifConfig.getAttribute(name);
    if (attrStr && *attrStr)
        return atoi(attrStr);
    return bgpModule->par(name);
}

const char *BgpConfigReader::getStrAttrOrPar(const cXMLElement& ifConfig, const char *name) const
{
    const char *attrStr = ifConfig.getAttribute(name);
    if (attrStr && *attrStr)
        return attrStr;
    return bgpModule->par(name);
}

} // namespace bgp

} // namespace inet

