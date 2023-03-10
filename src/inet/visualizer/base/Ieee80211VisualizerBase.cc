//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#ifdef INET_WITH_IEEE80211
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtAp.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtSta.h"
#endif // INET_WITH_IEEE80211
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/visualizer/base/Ieee80211VisualizerBase.h"

namespace inet {

namespace visualizer {

Ieee80211VisualizerBase::Ieee80211Visualization::Ieee80211Visualization(int networkNodeId, int interfaceId) :
    networkNodeId(networkNodeId),
    interfaceId(interfaceId)
{
}

void Ieee80211VisualizerBase::preDelete(cComponent *root)
{
    if (displayAssociations) {
        unsubscribe();
        removeAllIeee80211Visualizations();
    }
}

void Ieee80211VisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        displayAssociations = par("displayAssociations");
        nodeFilter.setPattern(par("nodeFilter"));
        interfaceFilter.setPattern(par("interfaceFilter"));
        minPowerDbm = par("minPower");
        maxPowerDbm = par("maxPower");
        const char *iconsAsString = par("icons");
        cStringTokenizer tokenizer(iconsAsString);
        while (tokenizer.hasMoreTokens())
            icons.push_back(tokenizer.nextToken());
        iconColorSet.parseColors(par("iconColor"));
        labelFont = cFigure::parseFont(par("labelFont"));
        labelColor = cFigure::Color(par("labelColor"));
        placementHint = parsePlacement(par("placementHint"));
        placementPriority = par("placementPriority");
        if (displayAssociations)
            subscribe();
    }
}

void Ieee80211VisualizerBase::handleParameterChange(const char *name)
{
    if (!hasGUI()) return;
    if (!strcmp(name, "nodeFilter"))
        nodeFilter.setPattern(par("nodeFilter"));
    else if (!strcmp(name, "interfaceFilter"))
        interfaceFilter.setPattern(par("interfaceFilter"));
    removeAllIeee80211Visualizations();
}

void Ieee80211VisualizerBase::subscribe()
{
    visualizationSubjectModule->subscribe(l2AssociatedSignal, this);
    visualizationSubjectModule->subscribe(l2DisassociatedSignal, this);
    visualizationSubjectModule->subscribe(l2ApAssociatedSignal, this);
    visualizationSubjectModule->subscribe(l2ApDisassociatedSignal, this);
}

void Ieee80211VisualizerBase::unsubscribe()
{
    // NOTE: lookup the module again because it may have been deleted first
    auto visualizationSubjectModule = findModuleFromPar<cModule>(par("visualizationSubjectModule"), this);
    if (visualizationSubjectModule != nullptr) {
        visualizationSubjectModule->unsubscribe(l2AssociatedSignal, this);
        visualizationSubjectModule->unsubscribe(l2DisassociatedSignal, this);
        visualizationSubjectModule->unsubscribe(l2ApAssociatedSignal, this);
        visualizationSubjectModule->unsubscribe(l2ApDisassociatedSignal, this);
    }
}

const Ieee80211VisualizerBase::Ieee80211Visualization *Ieee80211VisualizerBase::getIeee80211Visualization(cModule *networkNode, NetworkInterface *networkInterface)
{
    auto key = std::pair<int, int>(networkNode->getId(), networkInterface->getInterfaceId());
    auto it = ieee80211Visualizations.find(key);
    return it == ieee80211Visualizations.end() ? nullptr : it->second;
}

void Ieee80211VisualizerBase::addIeee80211Visualization(const Ieee80211Visualization *ieee80211Visualization)
{
    auto key = std::pair<int, int>(ieee80211Visualization->networkNodeId, ieee80211Visualization->interfaceId);
    ieee80211Visualizations[key] = ieee80211Visualization;
}

void Ieee80211VisualizerBase::removeIeee80211Visualization(const Ieee80211Visualization *ieee80211Visualization)
{
    auto key = std::pair<int, int>(ieee80211Visualization->networkNodeId, ieee80211Visualization->interfaceId);
    ieee80211Visualizations.erase(ieee80211Visualizations.find(key));
}

void Ieee80211VisualizerBase::removeAllIeee80211Visualizations()
{
    std::vector<const Ieee80211Visualization *> removedIeee80211Visualizations;
    for (auto it : ieee80211Visualizations)
        removedIeee80211Visualizations.push_back(it.second);
    for (auto visualization : removedIeee80211Visualizations) {
        removeIeee80211Visualization(visualization);
        delete visualization;
    }
}

void Ieee80211VisualizerBase::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
#ifdef INET_WITH_IEEE80211
    Enter_Method("%s", cComponent::getSignalName(signal));

    if (signal == l2AssociatedSignal) {
        auto networkNode = getContainingNode(check_and_cast<cModule *>(source));
        if (nodeFilter.matches(networkNode)) {
            auto networkInterface = check_and_cast<NetworkInterface *>(object);
            auto apInfo = check_and_cast<inet::ieee80211::Ieee80211MgmtSta::ApInfo *>(details);
            auto ieee80211Visualization = createIeee80211Visualization(networkNode, networkInterface, apInfo->ssid, W(apInfo->rxPower));
            addIeee80211Visualization(ieee80211Visualization);
        }
    }
    else if (signal == l2DisassociatedSignal) {
        auto networkNode = getContainingNode(check_and_cast<cModule *>(source));
        if (nodeFilter.matches(networkNode)) {
            auto networkInterface = check_and_cast<NetworkInterface *>(object);
            auto ieee80211Visualization = getIeee80211Visualization(networkNode, networkInterface);
            if (ieee80211Visualization != nullptr) {
                removeIeee80211Visualization(ieee80211Visualization);
                delete ieee80211Visualization;
            }
        }
    }
    else if (signal == l2ApAssociatedSignal) {
        auto networkNode = getContainingNode(check_and_cast<cModule *>(source));
        if (nodeFilter.matches(networkNode)) {
            // KLUDGE this is the wrong way to lookup the interface and the ssid
            auto mgmt = check_and_cast<inet::ieee80211::Ieee80211MgmtAp *>(source);
            auto networkInterface = getContainingNicModule(mgmt);
            auto ieee80211Visualization = getIeee80211Visualization(networkNode, networkInterface);
            if (ieee80211Visualization == nullptr) {
                auto ieee80211Visualization = createIeee80211Visualization(networkNode, networkInterface, mgmt->par("ssid"), W(NaN));
                addIeee80211Visualization(ieee80211Visualization);
            }
        }
    }
    else if (signal == l2ApDisassociatedSignal) {
        auto networkNode = getContainingNode(check_and_cast<cModule *>(source));
        if (nodeFilter.matches(networkNode)) {
            // KLUDGE this is the wrong way to lookup the interface
            auto mgmt = check_and_cast<inet::ieee80211::Ieee80211MgmtAp *>(source);
            auto networkInterface = getContainingNicModule(mgmt);
            auto ieee80211Visualization = getIeee80211Visualization(networkNode, networkInterface);
            if (ieee80211Visualization != nullptr) {
                removeIeee80211Visualization(ieee80211Visualization);
                delete ieee80211Visualization;
            }
        }
    }
    else
        throw cRuntimeError("Unknown signal");
#endif // INET_WITH_IEEE80211
}

std::string Ieee80211VisualizerBase::getIcon(W power) const
{
    int index;
    auto powerDbm = math::mW2dBmW(mW(power).get());
    if (std::isnan(powerDbm))
        index = icons.size() - 1;
    else if (powerDbm < minPowerDbm)
        index = 0;
    else if (powerDbm > maxPowerDbm)
        index = icons.size() - 1;
    else
        index = round((icons.size() - 1) * (powerDbm - minPowerDbm) / (maxPowerDbm - minPowerDbm));
    return icons[index];
}

} // namespace visualizer

} // namespace inet

