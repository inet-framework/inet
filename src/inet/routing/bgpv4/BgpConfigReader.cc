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

#include "inet/routing/bgpv4/BgpConfigReader.h"
#include "inet/common/ModuleAccess.h"
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

    //find my AS
    cXMLElementList ASList = bgpConfig->getElementsByTagName("AS");
    int routerPosition;
    AsId myAsId = findMyAS(ASList, routerPosition);
    if (myAsId == 0)
        throw cRuntimeError("BGP Error:  No AS configuration for Router ID: %s", bgpRouter->getRouterId().str().c_str());

    bgpRouter->setAsId(myAsId);

    // load EGP Session informations
    cXMLElementList sessionList = bgpConfig->getElementsByTagName("Session");
    simtime_t saveStartDelay = delayTab[3];
    loadSessionConfig(sessionList, delayTab);
    delayTab[3] = saveStartDelay;

    // load AS information
    char ASXPath[32];
    sprintf(ASXPath, "AS[@id='%d']", myAsId);

    cXMLElement *ASNode = bgpConfig->getElementByPath(ASXPath);
    std::vector<const char *> routerInSameASList;
    if (ASNode == nullptr)
        throw cRuntimeError("BGP Error:  No configuration for AS ID: %d", myAsId);

    cXMLElementList ASConfig = ASNode->getChildren();
    routerInSameASList = loadASConfig(ASConfig);

    //create IGP Session(s)
    if (routerInSameASList.size()) {
        unsigned int routerPeerPosition = 1;
        delayTab[3] += sessionList.size() * 2;
        for (auto it = routerInSameASList.begin(); it != routerInSameASList.end(); it++, routerPeerPosition++) {
            SessionId newSessionID = bgpRouter->createSession(IGP, (*it));
            delayTab[3] += calculateStartDelay(routerInSameASList.size(), routerPosition, routerPeerPosition);
            bgpRouter->setTimer(newSessionID, delayTab);
            bgpRouter->setSocketListen(newSessionID);
        }
    }
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
                if (ift->getInterface(i)->ipv4Data()->getIPAddress() == routerAddr)
                    return atoi((routerList_routerListIt)->getParentNode()->getAttribute("id"));
            }
            outRouterPosition++;
        }
    }

    return 0;
}

void BgpConfigReader::loadSessionConfig(cXMLElementList& sessionList, simtime_t *delayTab)
{
    simtime_t saveStartDelay = delayTab[3];
    for (auto sessionListIt = sessionList.begin(); sessionListIt != sessionList.end(); sessionListIt++, delayTab[3] = saveStartDelay) {
        const char *exterAddr = (*sessionListIt)->getFirstChild()->getAttribute("exterAddr");
        Ipv4Address routerAddr1 = Ipv4Address(exterAddr);
        exterAddr = (*sessionListIt)->getLastChild()->getAttribute("exterAddr");
        Ipv4Address routerAddr2 = Ipv4Address(exterAddr);
        if (isInInterfaceTable(ift, routerAddr1) == -1 && isInInterfaceTable(ift, routerAddr2) == -1) {
            continue;
        }
        Ipv4Address peerAddr;
        if (isInInterfaceTable(ift, routerAddr1) != -1) {
            peerAddr = routerAddr2;
            delayTab[3] += atoi((*sessionListIt)->getAttribute("id"));
        }
        else {
            peerAddr = routerAddr1;
            delayTab[3] += atoi((*sessionListIt)->getAttribute("id")) + 0.5;
        }
        if (peerAddr.isUnspecified()) {
            throw cRuntimeError("BGP Error: No valid external address for session ID : %s", (*sessionListIt)->getAttribute("id"));
        }

        SessionId newSessionID = bgpRouter->createSession(EGP, peerAddr.str().c_str());
        bgpRouter->setTimer(newSessionID, delayTab);
        bgpRouter->setSocketListen(newSessionID);
    }
}

std::vector<const char *> BgpConfigReader::loadASConfig(cXMLElementList& ASConfig)
{
    //create deny Lists
    std::vector<const char *> routerInSameASList;

    for (auto & elem : ASConfig) {
        std::string nodeName = (elem)->getTagName();
        if (nodeName == "Router") {
            if (isInInterfaceTable(ift, Ipv4Address((elem)->getAttribute("interAddr"))) == -1) {
                routerInSameASList.push_back((elem)->getAttribute("interAddr"));
            }
            continue;
        }
        if (nodeName == "DenyRoute" || nodeName == "DenyRouteIN" || nodeName == "DenyRouteOUT") {
            BgpRoutingTableEntry *entry = new BgpRoutingTableEntry();     //FIXME Who will delete this entry?
            entry->setDestination(Ipv4Address((elem)->getAttribute("Address")));
            entry->setNetmask(Ipv4Address((elem)->getAttribute("Netmask")));
            bgpRouter->addToPrefixList(nodeName, entry);
        }
        else if (nodeName == "DenyAS" || nodeName == "DenyASIN" || nodeName == "DenyASOUT") {
            AsId ASCur = atoi((elem)->getNodeValue());
            bgpRouter->addToAsList(nodeName, ASCur);
        }
        else {
            throw cRuntimeError("BGP Error: unknown element named '%s' for AS %u", nodeName.c_str(), bgpRouter->getAsId());
        }
    }
    return routerInSameASList;
}

int BgpConfigReader::isInInterfaceTable(IInterfaceTable *ifTable, Ipv4Address addr)
{
    for (int i = 0; i < ifTable->getNumInterfaces(); i++) {
        if (ifTable->getInterface(i)->ipv4Data()->getIPAddress() == addr) {
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

} // namespace bgp

} // namespace inet

