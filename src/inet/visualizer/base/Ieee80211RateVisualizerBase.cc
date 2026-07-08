//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/base/Ieee80211RateVisualizerBase.h"

#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/common/L3AddressResolver.h"

#ifdef INET_WITH_IEEE80211
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/contract/IRateSelection.h"
#include "inet/linklayer/ieee80211/mac/rateselection/QosRateSelection.h"
#include "inet/linklayer/ieee80211/mac/rateselection/RateSelection.h"
#include "inet/physicallayer/wireless/ieee80211/mode/IIeee80211Mode.h"
#endif // INET_WITH_IEEE80211

namespace inet {

namespace visualizer {

Ieee80211RateVisualizerBase::Ieee80211RateVisualization::Ieee80211RateVisualization(int networkNodeId, int interfaceId) :
    networkNodeId(networkNodeId),
    interfaceId(interfaceId)
{
}

void Ieee80211RateVisualizerBase::preDelete(cComponent *root)
{
    if (displayRates) {
        unsubscribe();
        removeAllRateVisualizations();
    }
}

void Ieee80211RateVisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        displayRates = par("displayRates");
        nodeFilter.setPattern(par("nodeFilter"));
        interfaceFilter.setPattern(par("interfaceFilter"));
        maxRate = par("maxRate");
        barWidth = par("barWidth");
        barSpacing = par("barSpacing");
        maxBarHeight = par("maxBarHeight");
        ColorSet rateColorSet;
        rateColorSet.parseColors(par("rateColor"));
        for (size_t i = 0; i < rateColorSet.getSize(); i++)
            rateColors.push_back(rateColorSet.getColor(i));
        rateFormat = par("rateFormat").stringValue();
        rateLabelFont = cFigure::parseFont(par("rateLabelFont"));
        rateLabelColor = cFigure::Color(par("rateLabelColor"));
        stationLabelFont = cFigure::parseFont(par("stationLabelFont"));
        stationLabelColor = cFigure::Color(par("stationLabelColor"));
        nameRotation = par("nameRotation").doubleValueInUnit("rad"); // @unit(deg) converted to radians
        displayTitle = par("displayTitle");
        titleFont = cFigure::parseFont(par("titleFont"));
        titleColor = cFigure::Color(par("titleColor"));
        holdTime = par("holdTime");
        placementHint = parsePlacement(par("placementHint"));
        placementPriority = par("placementPriority");
        if (displayRates)
            subscribe();
    }
}

void Ieee80211RateVisualizerBase::handleParameterChange(const char *name)
{
    if (!hasGUI()) return;
    if (!strcmp(name, "nodeFilter"))
        nodeFilter.setPattern(par("nodeFilter"));
    else if (!strcmp(name, "interfaceFilter"))
        interfaceFilter.setPattern(par("interfaceFilter"));
    removeAllRateVisualizations();
}

void Ieee80211RateVisualizerBase::subscribe()
{
#ifdef INET_WITH_IEEE80211
    visualizationSubjectModule->subscribe(ieee80211::IRateSelection::datarateSelectedSignal, this);
#endif // INET_WITH_IEEE80211
}

void Ieee80211RateVisualizerBase::unsubscribe()
{
#ifdef INET_WITH_IEEE80211
    // NOTE: lookup the module again because it may have been deleted first
    auto visualizationSubjectModule = findModuleFromPar<cModule>(par("visualizationSubjectModule"), this);
    if (visualizationSubjectModule != nullptr)
        visualizationSubjectModule->unsubscribe(ieee80211::IRateSelection::datarateSelectedSignal, this);
#endif // INET_WITH_IEEE80211
}

Ieee80211RateVisualizerBase::Ieee80211RateVisualization *Ieee80211RateVisualizerBase::getRateVisualization(int networkNodeId, int interfaceId)
{
    auto key = std::pair<int, int>(networkNodeId, interfaceId);
    auto it = ieee80211RateVisualizations.find(key);
    return it == ieee80211RateVisualizations.end() ? nullptr : it->second;
}

void Ieee80211RateVisualizerBase::addRateVisualization(Ieee80211RateVisualization *rateVisualization)
{
    auto key = std::pair<int, int>(rateVisualization->networkNodeId, rateVisualization->interfaceId);
    ieee80211RateVisualizations[key] = rateVisualization;
}

void Ieee80211RateVisualizerBase::removeRateVisualization(Ieee80211RateVisualization *rateVisualization)
{
    auto key = std::pair<int, int>(rateVisualization->networkNodeId, rateVisualization->interfaceId);
    ieee80211RateVisualizations.erase(ieee80211RateVisualizations.find(key));
}

void Ieee80211RateVisualizerBase::removeAllRateVisualizations()
{
    std::vector<Ieee80211RateVisualization *> removedRateVisualizations;
    for (auto it : ieee80211RateVisualizations)
        removedRateVisualizations.push_back(it.second);
    for (auto visualization : removedRateVisualizations) {
        removeRateVisualization(visualization);
        delete visualization;
    }
}

cModule *Ieee80211RateVisualizerBase::findNetworkNodeByMacAddress(const MacAddress& address) const
{
    L3AddressResolver addressResolver;
    for (cModule::SubmoduleIterator it(getSimulation()->getSystemModule()); !it.end(); ++it) {
        cModule *node = *it;
        if (!isNetworkNode(node))
            continue;
        auto interfaceTable = addressResolver.findInterfaceTableOf(node);
        if (interfaceTable == nullptr)
            continue;
        for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
            auto interface = interfaceTable->getInterface(i);
            if (interface->getMacAddress() == address)
                return node;
        }
    }
    return nullptr;
}

void Ieee80211RateVisualizerBase::addOrUpdateObservedRate(cModule *networkNode, NetworkInterface *networkInterface, const MacAddress& stationAddress, double bitrate)
{
    auto rateVisualization = getRateVisualization(networkNode->getId(), networkInterface->getInterfaceId());
    if (rateVisualization == nullptr) {
        rateVisualization = createRateVisualization(networkNode, networkInterface);
        addRateVisualization(rateVisualization);
    }
    auto& entry = rateVisualization->rates[stationAddress];
    entry.bitrate = bitrate;
    entry.observed = true;
    entry.lastUpdate = simTime();
    if (entry.staNodeId == -1) {
        auto staNode = findNetworkNodeByMacAddress(stationAddress);
        if (staNode != nullptr) {
            entry.staNodeId = staNode->getId();
            entry.staName = staNode->getFullName();
        }
        else
            entry.staName = stationAddress.str();
    }
}

void Ieee80211RateVisualizerBase::refreshRateEntries(cModule *networkNode, NetworkInterface *networkInterface, Ieee80211RateVisualization *rateVisualization) const
{
    // prune stale observed entries according to holdTime
    if (holdTime > 0) {
        for (auto it = rateVisualization->rates.begin(); it != rateVisualization->rates.end();) {
            if (it->second.observed && it->second.lastUpdate >= 0 && simTime() - it->second.lastUpdate > holdTime)
                it = rateVisualization->rates.erase(it);
            else
                ++it;
        }
    }
#ifdef INET_WITH_IEEE80211
    // merge configured per-receiver rates (as a fallback for stations not transmitted to yet)
    using namespace inet::ieee80211;
    auto mac = networkInterface->getSubmodule("mac");
    if (mac == nullptr)
        return;
    const std::map<MacAddress, const physicallayer::IIeee80211Mode *> *perReceiverModes = nullptr;
    if (auto dcf = mac->getSubmodule("dcf")) {
        if (auto rs = dynamic_cast<RateSelection *>(dcf->getSubmodule("rateSelection")))
            perReceiverModes = &rs->getPerReceiverDataFrameModes();
    }
    if (perReceiverModes == nullptr) {
        if (auto hcf = mac->getSubmodule("hcf")) {
            if (auto qrs = dynamic_cast<QosRateSelection *>(hcf->getSubmodule("rateSelection")))
                perReceiverModes = &qrs->getPerReceiverDataFrameModes();
        }
    }
    if (perReceiverModes != nullptr) {
        for (auto& kv : *perReceiverModes) {
            const MacAddress& stationAddress = kv.first;
            if (stationAddress.isMulticast() || stationAddress.isBroadcast())
                continue;
            auto it = rateVisualization->rates.find(stationAddress);
            if (it != rateVisualization->rates.end() && it->second.observed)
                continue; // an observed rate takes precedence over the configured one
            auto& entry = rateVisualization->rates[stationAddress];
            entry.bitrate = kv.second->getDataMode()->getNetBitrate().get<bps>();
            entry.observed = false;
            if (entry.staNodeId == -1) {
                auto staNode = findNetworkNodeByMacAddress(stationAddress);
                if (staNode != nullptr) {
                    entry.staNodeId = staNode->getId();
                    entry.staName = staNode->getFullName();
                }
                else
                    entry.staName = stationAddress.str();
            }
        }
    }
#endif // INET_WITH_IEEE80211
}

void Ieee80211RateVisualizerBase::ensureConfiguredVisualizations()
{
    if (!displayRates)
        return;
#ifdef INET_WITH_IEEE80211
    using namespace inet::ieee80211;
    L3AddressResolver addressResolver;
    for (cModule::SubmoduleIterator it(getSimulation()->getSystemModule()); !it.end(); ++it) {
        cModule *node = *it;
        if (!isNetworkNode(node) || !nodeFilter.matches(node))
            continue;
        auto interfaceTable = addressResolver.findInterfaceTableOf(node);
        if (interfaceTable == nullptr)
            continue;
        for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
            auto networkInterface = interfaceTable->getInterface(i);
            if (!interfaceFilter.matches(networkInterface))
                continue;
            if (getRateVisualization(node->getId(), networkInterface->getInterfaceId()) != nullptr)
                continue;
            auto mac = networkInterface->getSubmodule("mac");
            if (mac == nullptr)
                continue;
            bool hasConfigured = false;
            if (auto dcf = mac->getSubmodule("dcf")) {
                if (auto rs = dynamic_cast<RateSelection *>(dcf->getSubmodule("rateSelection")))
                    hasConfigured = !rs->getPerReceiverDataFrameModes().empty();
            }
            if (!hasConfigured) {
                if (auto hcf = mac->getSubmodule("hcf")) {
                    if (auto qrs = dynamic_cast<QosRateSelection *>(hcf->getSubmodule("rateSelection")))
                        hasConfigured = !qrs->getPerReceiverDataFrameModes().empty();
                }
            }
            if (hasConfigured)
                addRateVisualization(createRateVisualization(node, networkInterface));
        }
    }
#endif // INET_WITH_IEEE80211
}

std::string Ieee80211RateVisualizerBase::formatRate(double bitrate) const
{
    std::string result;
    for (size_t i = 0; i < rateFormat.length(); i++) {
        char c = rateFormat[i];
        if (c == '%' && i + 1 < rateFormat.length()) {
            char code = rateFormat[++i];
            char buf[32];
            switch (code) {
                case 'm': // rounded Mbps, no unit suffix
                    snprintf(buf, sizeof(buf), "%g", std::round(bitrate / 1e6));
                    result += buf;
                    break;
                case 'r': // rounded Mbps with an "M" suffix
                    snprintf(buf, sizeof(buf), "%g", std::round(bitrate / 1e6));
                    result += buf;
                    result += "M";
                    break;
                case 'g': // bps
                    snprintf(buf, sizeof(buf), "%g", bitrate);
                    result += buf;
                    break;
                case '%':
                    result += '%';
                    break;
                default:
                    result += '%';
                    result += code;
                    break;
            }
        }
        else
            result += c;
    }
    return result;
}

cFigure::Color Ieee80211RateVisualizerBase::getRateColor(double bitrate) const
{
    if (rateColors.empty())
        return cFigure::Color("grey");
    if (rateColors.size() == 1)
        return rateColors[0];
    double fraction = maxRate > 0 ? bitrate / maxRate : 0;
    if (fraction < 0) fraction = 0;
    if (fraction > 1) fraction = 1;
    // map fraction in [0,1] to a position along the gradient (first color = slow, last = fast)
    double pos = fraction * (rateColors.size() - 1);
    int index = (int)std::floor(pos);
    if (index >= (int)rateColors.size() - 1)
        return rateColors.back();
    double t = pos - index;
    const auto& c0 = rateColors[index];
    const auto& c1 = rateColors[index + 1];
    auto lerp = [](uint8_t a, uint8_t b, double t) { return (uint8_t)std::round(a + (b - a) * t); };
    return cFigure::Color(lerp(c0.red, c1.red, t), lerp(c0.green, c1.green, t), lerp(c0.blue, c1.blue, t));
}

void Ieee80211RateVisualizerBase::receiveSignal(cComponent *source, simsignal_t signal, double value, cObject *details)
{
#ifdef INET_WITH_IEEE80211
    Enter_Method("%s", cComponent::getSignalName(signal));

    if (signal == ieee80211::IRateSelection::datarateSelectedSignal) {
        auto networkNode = getContainingNode(check_and_cast<cModule *>(source));
        if (!nodeFilter.matches(networkNode))
            return;
        auto networkInterface = getContainingNicModule(check_and_cast<cModule *>(source));
        if (networkInterface == nullptr || !interfaceFilter.matches(networkInterface))
            return;
        auto packet = dynamic_cast<Packet *>(details);
        if (packet == nullptr)
            return;
        const auto& header = packet->peekAtFront<ieee80211::Ieee80211MacHeader>();
        auto receiverAddress = header->getReceiverAddress();
        if (receiverAddress.isMulticast() || receiverAddress.isBroadcast() || receiverAddress.isUnspecified())
            return;
        // only track data frames
        if (dynamicPtrCast<const ieee80211::Ieee80211DataHeader>(header) == nullptr)
            return;
        addOrUpdateObservedRate(networkNode, networkInterface, receiverAddress, value);
    }
    else
        throw cRuntimeError("Unknown signal");
#endif // INET_WITH_IEEE80211
}

} // namespace visualizer

} // namespace inet
