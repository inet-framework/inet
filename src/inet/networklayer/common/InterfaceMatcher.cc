//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/common/InterfaceMatcher.h"

#include <set>

#include "inet/common/ModuleAccess.h"
#include "inet/common/PatternMatcher.h"
#include "inet/common/stlutils.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

InterfaceMatcher::Matcher::Matcher(const char *pattern)
{
    matchesany = opp_isempty(pattern);
    if (matchesany)
        return;
    cStringTokenizer tokenizer(pattern);
    while (tokenizer.hasMoreTokens())
        matchers.push_back(new inet::PatternMatcher(tokenizer.nextToken(), true, true, true));
}

InterfaceMatcher::Matcher::~Matcher()
{
    for (auto& elem : matchers)
        delete elem;
}

bool InterfaceMatcher::Matcher::matches(const char *s) const
{
    if (matchesany)
        return true;
    for (auto& elem : matchers)
        if (elem->matches(s))
            return true;

    return false;
}

InterfaceMatcher::Selector::Selector(const char *hostPattern, const char *namePattern, const char *towardsPattern, const InterfaceMatcher *parent)
    : hostMatcher(hostPattern), nameMatcher(namePattern), towardsMatcher(towardsPattern), parent(parent)
{
}

// Note: "hosts", "interfaces" and "towards" must ALL match on the interface
bool InterfaceMatcher::Selector::matches(const NetworkInterface *ie)
{
    cModule *hostModule = ie->getInterfaceTable()->getHostModule();
    std::string hostFullPath = hostModule->getFullPath();
    std::string hostShortenedFullPath = hostFullPath.substr(hostFullPath.find('.') + 1);

    return (hostMatcher.matchesAny() || hostMatcher.matches(hostShortenedFullPath.c_str()) || hostMatcher.matches(hostFullPath.c_str())) &&
           (nameMatcher.matchesAny() || nameMatcher.matches(ie->getFullName())) &&
           (towardsMatcher.matchesAny() || parent->linkContainsMatchingHost(ie, towardsMatcher));
}

InterfaceMatcher::InterfaceMatcher(const cXMLElementList& xmlSelectors)
{
    for (auto& xmlSelector : xmlSelectors) {
        cXMLElement *interfaceElement = xmlSelector;
        const char *hostAttr = interfaceElement->getAttribute("hosts"); // "host* router[0..3]"
        const char *interfaceAttr = interfaceElement->getAttribute("names"); // i.e. interface names, like "eth* ppp0"

        const char *towardsAttr = interfaceElement->getAttribute("towards"); // neighbor host names, like "ap switch"
        const char *amongAttr = interfaceElement->getAttribute("among"); // neighbor host names, like "host[*] router1"

        if (amongAttr && *amongAttr) { // among="X Y Z" means hosts = "X Y Z" towards = "X Y Z"
            if ((hostAttr && *hostAttr) || (towardsAttr && *towardsAttr))
                throw cRuntimeError("The 'hosts'/'towards' and 'among' attributes are mutually exclusive, at %s", interfaceElement->getSourceLocation());
            towardsAttr = hostAttr = amongAttr;
        }

        try {
            selectors.push_back(new Selector(hostAttr, interfaceAttr, towardsAttr, this));
        }
        catch (std::exception& e) {
            throw cRuntimeError("Error in XML <interface> element at %s: %s", interfaceElement->getSourceLocation(), e.what());
        }
    }
}

InterfaceMatcher::~InterfaceMatcher()
{
    for (auto& elem : selectors)
        delete elem;
}

/**
 * Returns the index of the first selector that matches the interface.
 */
int InterfaceMatcher::findMatchingSelector(const NetworkInterface *ie)
{
    for (int i = 0; i < (int)selectors.size(); i++)
        if (selectors[i]->matches(ie))
            return i;

    return -1;
}

static bool hasInterfaceTable(cModule *module)
{
    return L3AddressResolver().findInterfaceTableOf(module);
}

static cGate *findRemoteGate(cGate *startGate)
{
    for (cGate *gate = startGate->getNextGate(); gate; gate = gate->getNextGate())
        if (isNetworkNode(gate->getOwnerModule()))
            return gate;

    return nullptr;
}

bool InterfaceMatcher::linkContainsMatchingHost(const NetworkInterface *ie, const Matcher& hostMatcher) const
{
    int outGateId = ie->getNodeOutputGateId();
    cModule *node = ie->getInterfaceTable()->getHostModule();
    cGate *outGate = node->gate(outGateId);

    std::vector<cModule *> hostNodes;
    std::vector<cModule *> deviceNodes;
    collectNeighbors(outGate, hostNodes, deviceNodes, node);

    for (auto neighbour : hostNodes) {

        std::string hostFullPath = neighbour->getFullPath();
        std::string hostShortenedFullPath = hostFullPath.substr(hostFullPath.find('.') + 1);
        if (hostMatcher.matches(hostShortenedFullPath.c_str()) || hostMatcher.matches(hostFullPath.c_str()))
            return true;
    }

    return false;
}

void InterfaceMatcher::collectNeighbors(cGate *outGate, std::vector<cModule *>& hostNodes, std::vector<cModule *>& deviceNodes, cModule *excludedNode) const
{
    cGate *neighborGate = findRemoteGate(outGate);
    if (!neighborGate)
        return;

    cModule *neighborNode = neighborGate->getOwnerModule();
    if (hasInterfaceTable(neighborNode)) {
        // neighbor is a host or router
        if (neighborNode != excludedNode && !contains(hostNodes, neighborNode))
            hostNodes.push_back(neighborNode);
    }
    else {
        // assume that neighbor is an L2 or L1 device (bus/hub/switch/bridge/access point/etc); visit all its output links
        if (!contains(deviceNodes, neighborNode)) {
            if (neighborNode != excludedNode)
                deviceNodes.push_back(neighborNode);
            for (cModule::GateIterator it(neighborNode); !it.end(); it++) {
                cGate *gate = *it;
                if (gate->getType() == cGate::OUTPUT)
                    collectNeighbors(gate, hostNodes, deviceNodes, excludedNode);
            }
        }
    }
}

} // namespace inet

