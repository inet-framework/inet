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
    // The OSG twin renders a textured-quad billboard (osg::Texture2D + osg::Geode) loaded
    // from the icon image (e.g. "misc/signal_power_N.png"), tinted by a color derived from
    // the SSID hash. VSG has no ready-to-use textured-billboard helper in VsgUtils yet, so
    // we approximate with:
    //   - a small colored sphere whose color encodes the SSID (same iconColorSet logic), and
    //   - a text label showing the SSID.
    // TODO: textured icon — replace with createTexturedBillboard() once VsgUtils supports it.

    auto node = ::vsg::Group::create();

    // Derive a color from the SSID hash, mirroring the OSG logic.
    std::hash<std::string> hasher;
    auto color = iconColorSet.getColor(hasher(ssid));

    // Sphere approximating the signal-strength icon.
    const double radius = 6.0;
    node->addChild(inet::vsg::createSphere(Coord(0, 0, radius), radius, color, 1.0));

    // SSID label just above the sphere.
    if (!ssid.empty())
        node->addChild(inet::vsg::createLabel(ssid.c_str(), Coord(0, 0, radius * 2 + 2), labelColor, 14));

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
