//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/vsg/common/PacketDropVsgVisualizer.h"

#include <algorithm>
#include <cmath>

#include "inet/common/ModuleAccess.h"
#include "inet/visualizer/vsg/util/VsgScene.h"
#include "inet/visualizer/vsg/util/VsgUtils.h"

namespace inet {

namespace visualizer {

Define_Module(PacketDropVsgVisualizer);

PacketDropVsgVisualizer::PacketDropVsgVisualization::PacketDropVsgVisualization(::vsg::ref_ptr<::vsg::MatrixTransform> node, const PacketDrop *packetDrop) :
    PacketDropVisualization(packetDrop),
    node(node)
{
}

void PacketDropVsgVisualizer::refreshDisplay() const
{
    PacketDropVisualizerBase::refreshDisplay();
    // Keep the canvas animation speed in sync with pending drop visualizations,
    // mirroring what PacketDropOsgVisualizer does.
    visualizationTargetModule->getCanvas()->setAnimationSpeed(packetDropVisualizations.empty() ? 0 : fadeOutAnimationSpeed, this);
}

const PacketDropVisualizerBase::PacketDropVisualization *PacketDropVsgVisualizer::createPacketDropVisualization(PacketDrop *packetDrop) const
{
    // Choose a tint colour for this drop reason, matching the OSG iconTintColorSet logic.
    auto iconTintColor = iconTintColorSet.getColor(packetDrop->getReason() % iconTintColorSet.getSize());

    // OSG renders a textured quad (msg/packet_s icon) auto-rotated to face the camera.
    // VSG cannot load arbitrary images as a texture via VsgUtils without additional plumbing.
    // TODO: replace the sphere with a textured-quad billboard using the "msg/packet_s" image
    //       once VsgUtils gains createTexturedBillboard() support.
    // Approximate: a coloured sphere at the packet drop position.
    const double radius = 8.0;
    const auto& pos = packetDrop->getPosition();

    // Build a mutable container: a MatrixTransform (for position/fade movement) holding a
    // Group (for opacity rebuilds in setAlpha).
    auto markerGroup = ::vsg::Group::create();
    markerGroup->addChild(inet::vsg::createSphere(Coord::ZERO, radius, iconTintColor, 1.0));

    // Text label above the sphere showing the configured labelFormat string.
    std::string labelText = getPacketDropVisualizationText(packetDrop);
    if (!labelText.empty())
        markerGroup->addChild(inet::vsg::createLabel(labelText.c_str(), Coord(0, radius + 4.0, 0), labelColor, 14));

    // Position transform in world space.
    auto transform = inet::vsg::createPositionAttitudeTransform(pos, Quaternion::IDENTITY);
    transform->addChild(markerGroup);

    return new PacketDropVsgVisualization(transform, packetDrop);
}

void PacketDropVsgVisualizer::addPacketDropVisualization(const PacketDropVisualization *packetDropVisualization)
{
    PacketDropVisualizerBase::addPacketDropVisualization(packetDropVisualization);
    auto packetDropVsgVisualization = static_cast<const PacketDropVsgVisualization *>(packetDropVisualization);
    auto scene = inet::vsg::TopLevelScene::getSimulationScene(visualizationTargetModule);
    scene->addChild(packetDropVsgVisualization->node);
}

void PacketDropVsgVisualizer::removePacketDropVisualization(const PacketDropVisualization *packetDropVisualization)
{
    PacketDropVisualizerBase::removePacketDropVisualization(packetDropVisualization);
    auto packetDropVsgVisualization = static_cast<const PacketDropVsgVisualization *>(packetDropVisualization);
    auto scene = inet::vsg::TopLevelScene::getSimulationScene(visualizationTargetModule);
    auto& ch = scene->children;
    ch.erase(std::remove(ch.begin(), ch.end(), ::vsg::ref_ptr<::vsg::Node>(packetDropVsgVisualization->node)), ch.end());
}

void PacketDropVsgVisualizer::setAlpha(const PacketDropVisualization *packetDropVisualization, double alpha) const
{
    auto packetDropVsgVisualization = static_cast<const PacketDropVsgVisualization *>(packetDropVisualization);
    auto transform = packetDropVsgVisualization->node;

    // VSG has no detachable material alpha — opacity is baked into the geometry pipeline.
    // Rebuild the sphere with the new opacity, reusing the same tint colour.
    // TODO: a per-instance opacity uniform (e.g. vsg_Color.a push-constant) would be more
    //       efficient and avoid the per-frame subgraph rebuild during fade-out.
    auto iconTintColor = iconTintColorSet.getColor(packetDropVisualization->packetDrop->getReason() % iconTintColorSet.getSize());
    const double radius = 8.0;

    // The markerGroup is the first (and only) child of the transform.
    // children is a vector of ::vsg::ref_ptr<::vsg::Node>; cast the first element.
    auto markerGroup = ::vsg::ref_ptr<::vsg::Group>(dynamic_cast<::vsg::Group *>(transform->children[0].get()));
    if (!markerGroup)
        return;
    markerGroup->children.clear();
    markerGroup->addChild(inet::vsg::createSphere(Coord::ZERO, radius, iconTintColor, alpha));

    // Rebuild label if any (label opacity is not easily controlled; skip when nearly faded).
    if (alpha > 0.1) {
        std::string labelText = getPacketDropVisualizationText(packetDropVisualization->packetDrop);
        if (!labelText.empty())
            markerGroup->addChild(inet::vsg::createLabel(labelText.c_str(), Coord(0, radius + 4.0, 0), labelColor, 14));
    }

    // Animate position upward and outward as alpha decreases, mirroring the OSG trajectory.
    double dx = 10.0 / alpha;
    double dy = 10.0 / alpha;
    double dz = 58.0 - std::pow((dx / 4.0 - 9.0), 2.0);
    const auto& position = packetDropVisualization->packetDrop->getPosition();
    // Rewrite the MatrixTransform matrix to the new position.
    transform->matrix = ::vsg::translate(position.x + dx, position.y + dy, position.z + dz);
}

} // namespace visualizer

} // namespace inet
