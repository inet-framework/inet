//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/vsg/linklayer/LinkBreakVsgVisualizer.h"

#include <algorithm>

#include "inet/common/ModuleAccess.h"
#include "inet/visualizer/vsg/util/VsgScene.h"
#include "inet/visualizer/vsg/util/VsgUtils.h"

namespace inet {

namespace visualizer {

Define_Module(LinkBreakVsgVisualizer);

LinkBreakVsgVisualizer::LinkBreakVsgVisualization::LinkBreakVsgVisualization(
        ::vsg::ref_ptr<::vsg::MatrixTransform> node,
        int transmitterModuleId, int receiverModuleId) :
    LinkBreakVisualization(transmitterModuleId, receiverModuleId),
    node(node)
{
}

void LinkBreakVsgVisualizer::refreshDisplay() const
{
    // Let the base class handle fade-out timing and alpha updates.
    LinkBreakVisualizerBase::refreshDisplay();

    // Reposition each surviving marker at the current midpoint of its link.
    // This mirrors what LinkBreakCanvasVisualizer does in its refreshDisplay().
    for (auto& it : linkBreakVisualizations) {
        auto linkBreakVsgVisualization = static_cast<const LinkBreakVsgVisualization *>(it.second);
        auto transmitter = getSimulation()->getModule(linkBreakVsgVisualization->transmitterModuleId);
        auto receiver    = getSimulation()->getModule(linkBreakVsgVisualization->receiverModuleId);
        if (transmitter == nullptr || receiver == nullptr)
            continue;
        auto txPos = getPosition(getContainingNode(transmitter));
        auto rxPos = getPosition(getContainingNode(receiver));
        auto midpoint = (txPos + rxPos) / 2;
        linkBreakVsgVisualization->node->matrix = ::vsg::translate(::vsg::dvec3(midpoint.x, midpoint.y, midpoint.z));
    }

    // Keep the canvas animation speed in sync with pending link-break visualizations,
    // mirroring what LinkBreakOsgVisualizer does.
    visualizationTargetModule->getCanvas()->setAnimationSpeed(linkBreakVisualizations.empty() ? 0 : fadeOutAnimationSpeed, this);
}

const LinkBreakVisualizerBase::LinkBreakVisualization *LinkBreakVsgVisualizer::createLinkBreakVisualization(
        cModule *transmitter, cModule *receiver) const
{
    // The OSG twin renders a textured-quad billboard (osg::Texture2D + osg::Geode) using
    // the configured icon image, tinted by iconTintColor. VSG has no ready-to-use
    // textured-billboard helper in VsgUtils yet, so we approximate with a colored sphere.
    // TODO: textured icon — replace sphere with createTexturedBillboard() once VsgUtils
    //       gains that helper.

    const double radius = 8.0;

    auto markerGroup = ::vsg::Group::create();
    markerGroup->addChild(inet::vsg::createSphere(Coord::ZERO, radius, iconTintColor, 1.0));

    // Compute the initial midpoint position and bake it into the transform.
    // refreshDisplay() will keep rewriting the matrix as nodes move.
    auto txPos = getPosition(getContainingNode(transmitter));
    auto rxPos = getPosition(getContainingNode(receiver));
    auto midpoint = (txPos + rxPos) / 2;

    auto transform = inet::vsg::createPositionAttitudeTransform(midpoint, Quaternion::IDENTITY);
    transform->addChild(markerGroup);

    return new LinkBreakVsgVisualization(transform, transmitter->getId(), receiver->getId());
}

void LinkBreakVsgVisualizer::addLinkBreakVisualization(const LinkBreakVisualization *linkBreakVisualization)
{
    LinkBreakVisualizerBase::addLinkBreakVisualization(linkBreakVisualization);
    auto linkBreakVsgVisualization = static_cast<const LinkBreakVsgVisualization *>(linkBreakVisualization);
    auto scene = inet::vsg::TopLevelScene::getSimulationScene(visualizationTargetModule);
    scene->addChild(linkBreakVsgVisualization->node);
}

void LinkBreakVsgVisualizer::removeLinkBreakVisualization(const LinkBreakVisualization *linkBreakVisualization)
{
    LinkBreakVisualizerBase::removeLinkBreakVisualization(linkBreakVisualization);
    auto linkBreakVsgVisualization = static_cast<const LinkBreakVsgVisualization *>(linkBreakVisualization);
    auto scene = inet::vsg::TopLevelScene::getSimulationScene(visualizationTargetModule);
    auto& ch = scene->children;
    ch.erase(std::remove(ch.begin(), ch.end(),
                         ::vsg::ref_ptr<::vsg::Node>(linkBreakVsgVisualization->node)),
             ch.end());
}

void LinkBreakVsgVisualizer::setAlpha(const LinkBreakVisualization *linkBreakVisualization, double alpha) const
{
    auto linkBreakVsgVisualization = static_cast<const LinkBreakVsgVisualization *>(linkBreakVisualization);
    auto transform = linkBreakVsgVisualization->node;

    // VSG has no detachable material alpha — opacity is baked into geometry pipelines.
    // Rebuild the sphere at the new opacity, matching the approach used in
    // PacketDropVsgVisualizer::setAlpha().
    // TODO: a per-instance opacity push-constant would be more efficient and avoid the
    //       per-frame subgraph rebuild during fade-out.
    auto markerGroup = ::vsg::ref_ptr<::vsg::Group>(
            dynamic_cast<::vsg::Group *>(transform->children[0].get()));
    if (!markerGroup)
        return;
    markerGroup->children.clear();
    markerGroup->addChild(inet::vsg::createSphere(Coord::ZERO, 8.0, iconTintColor, alpha));
}

} // namespace visualizer

} // namespace inet
