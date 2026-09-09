//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/vsg/linklayer/Ieee80211VsgVisualizer.h"

#include "inet/common/ModuleAccess.h"
#include "inet/visualizer/vsg/util/VsgUtils.h"

namespace inet {

namespace visualizer {

Define_Module(Ieee80211VsgVisualizer);

Ieee80211VsgVisualizer::Ieee80211VsgVisualization::Ieee80211VsgVisualization(
        NetworkNodeVsgVisualization *networkNodeVisualization,
        ::vsg::ref_ptr<::vsg::Group> node,
        int networkNodeId, int interfaceId) :
    Ieee80211Visualization(networkNodeId, interfaceId),
    networkNodeVisualization(networkNodeVisualization),
    node(node)
{
}

void Ieee80211VsgVisualizer::initialize(int stage)
{
    Ieee80211VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL)
        networkNodeVisualizer.reference(this, "networkNodeVisualizerModule", true);
}

Ieee80211VisualizerBase::Ieee80211Visualization *Ieee80211VsgVisualizer::createIeee80211Visualization(
        cModule *networkNode, NetworkInterface *networkInterface, std::string ssid, W power)
{
    // This node is attached as an annotation, so NetworkNodeVsgVisualization wraps it in a billboard
    // AutoScaleTransform (camera-facing, constant on-screen size) — author content in the X-Y plane.
    auto node = ::vsg::Group::create();

    // Tint color derived from the SSID hash (mirrors the OSG iconColorSet logic).
    std::hash<std::string> hasher;
    auto color = iconColorSet.getColor(hasher(ssid));

    // The signal-strength icon getIcon(power), tinted; fall back to a colored sphere if it can't load.
    const double iconSize = 32;
    ::vsg::ref_ptr<::vsg::Data> iconImage;
    try { iconImage = inet::vsg::createImage(inet::vsg::resolveImageResource(getIcon(power).c_str(), networkNode).c_str()); }
    catch (const std::exception&) { iconImage = nullptr; }
    if (iconImage)
        node->addChild(inet::vsg::createTexturedQuad(iconImage, iconSize, color));
    else
        node->addChild(inet::vsg::createSphere(Coord(0, 0, 6), 6, color, 1.0));

    // SSID label above the icon (+Y is screen-up under the billboard wrapper).
    if (!ssid.empty())
        node->addChild(inet::vsg::createText(ssid.c_str(), Coord(0, iconSize / 2 + 4, 0), labelColor, 14));

    auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
    return new Ieee80211VsgVisualization(networkNodeVisualization, node, networkNode->getId(), networkInterface->getInterfaceId());
}

void Ieee80211VsgVisualizer::addIeee80211Visualization(const Ieee80211Visualization *ieee80211Visualization)
{
    Ieee80211VisualizerBase::addIeee80211Visualization(ieee80211Visualization);
    auto ieee80211VsgVisualization = static_cast<const Ieee80211VsgVisualization *>(ieee80211Visualization);
    // Stack the annotation above the node icon (z offset ~32 units, priority 0 — same as OSG).
    ieee80211VsgVisualization->networkNodeVisualization->addAnnotation(
            ieee80211VsgVisualization->node, ::vsg::dvec3(0, 0, 32), 0);
}

void Ieee80211VsgVisualizer::removeIeee80211Visualization(const Ieee80211Visualization *ieee80211Visualization)
{
    Ieee80211VisualizerBase::removeIeee80211Visualization(ieee80211Visualization);
    auto ieee80211VsgVisualization = static_cast<const Ieee80211VsgVisualization *>(ieee80211Visualization);
    if (networkNodeVisualizer != nullptr)
        ieee80211VsgVisualization->networkNodeVisualization->removeAnnotation(
                ieee80211VsgVisualization->node);
}

} // namespace visualizer

} // namespace inet
