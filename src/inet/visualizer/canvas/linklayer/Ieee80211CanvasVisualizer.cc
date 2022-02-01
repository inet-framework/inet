//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/canvas/linklayer/Ieee80211CanvasVisualizer.h"

#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/common/L3AddressResolver.h"

#ifdef INET_WITH_IEEE80211
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtSta.h"
#endif

namespace inet {

namespace visualizer {

Define_Module(Ieee80211CanvasVisualizer);

Ieee80211CanvasVisualizer::Ieee80211CanvasVisualization::~Ieee80211CanvasVisualization()
{
    delete figure;
}

Ieee80211CanvasVisualizer::Ieee80211CanvasVisualization::Ieee80211CanvasVisualization(NetworkNodeCanvasVisualization *networkNodeVisualization, LabeledIconFigure *figure, int networkNodeId, int interfaceId) :
    Ieee80211Visualization(networkNodeId, interfaceId),
    networkNodeVisualization(networkNodeVisualization),
    figure(figure)
{
}

void Ieee80211CanvasVisualizer::initialize(int stage)
{
    Ieee80211VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        networkNodeVisualizer.reference(this, "networkNodeVisualizerModule", true);
    }
}

void Ieee80211CanvasVisualizer::refreshDisplay() const
{
#ifdef INET_WITH_IEEE80211
    auto simulation = getSimulation();
    for (auto& entry : ieee80211Visualizations) {
        auto networkNode = simulation->getModule(entry.second->networkNodeId);
        if (networkNode != nullptr) {
            L3AddressResolver addressResolver;
            auto interfaceTable = addressResolver.findInterfaceTableOf(networkNode);
            if (interfaceTable != nullptr) {
                auto networkInterface = interfaceTable->getInterfaceById(entry.second->interfaceId);
                auto mgmt = dynamic_cast<inet::ieee80211::Ieee80211MgmtSta *>(networkInterface->getSubmodule("mgmt"));
                if (mgmt != nullptr) {
                    auto apInfo = mgmt->getAssociatedAp();
                    std::string icon = getIcon(W(apInfo->rxPower));
                    auto canvasVisualization = check_and_cast<const Ieee80211CanvasVisualization *>(entry.second);
                    auto iconFigure = check_and_cast<LabeledIconFigure *>(canvasVisualization->figure)->getIconFigure();
                    iconFigure->setImageName(icon.substr(0, icon.find_first_of(".")).c_str());
                }
            }
        }
    }
#endif
}

Ieee80211VisualizerBase::Ieee80211Visualization *Ieee80211CanvasVisualizer::createIeee80211Visualization(cModule *networkNode, NetworkInterface *networkInterface, std::string ssid, W power)
{
    std::string icon = getIcon(power);
    auto labeledIconFigure = new LabeledIconFigure("ieee80211Association");
    labeledIconFigure->setTags((std::string("ieee80211_association ") + tags).c_str());
    labeledIconFigure->setAssociatedObject(networkInterface);
    labeledIconFigure->setZIndex(zIndex);
    auto iconFigure = labeledIconFigure->getIconFigure();
    iconFigure->setTooltip("This icon represents an IEEE 802.11 association");
    iconFigure->setImageName(icon.substr(0, icon.find_first_of(".")).c_str());
    std::hash<std::string> hasher;
    iconFigure->setTintColor(iconColorSet.getColor(hasher(ssid)));
    iconFigure->setTintAmount(1);
    auto labelFigure = labeledIconFigure->getLabelFigure();
    labelFigure->setTooltip("This label represents the SSID of an IEEE 802.11 association");
    labelFigure->setFont(labelFont);
    labelFigure->setColor(labelColor);
    labelFigure->setText(ssid.c_str());
    labelFigure->setPosition(iconFigure->getBounds().getSize() / 2);
    auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
    return new Ieee80211CanvasVisualization(networkNodeVisualization, labeledIconFigure, networkNode->getId(), networkInterface->getInterfaceId());
}

void Ieee80211CanvasVisualizer::addIeee80211Visualization(const Ieee80211Visualization *ieee80211Visualization)
{
    Ieee80211VisualizerBase::addIeee80211Visualization(ieee80211Visualization);
    auto ieee80211CanvasVisualization = static_cast<const Ieee80211CanvasVisualization *>(ieee80211Visualization);
    ieee80211CanvasVisualization->networkNodeVisualization->addAnnotation(ieee80211CanvasVisualization->figure, ieee80211CanvasVisualization->figure->getBounds().getSize(), placementHint, placementPriority);
}

void Ieee80211CanvasVisualizer::removeIeee80211Visualization(const Ieee80211Visualization *ieee80211Visualization)
{
    Ieee80211VisualizerBase::removeIeee80211Visualization(ieee80211Visualization);
    auto ieee80211CanvasVisualization = static_cast<const Ieee80211CanvasVisualization *>(ieee80211Visualization);
    if (networkNodeVisualizer != nullptr)
        ieee80211CanvasVisualization->networkNodeVisualization->removeAnnotation(ieee80211CanvasVisualization->figure);
}

} // namespace visualizer

} // namespace inet

