//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/vsg/physicallayer/RadioVsgVisualizer.h"

#include <algorithm>

#include "inet/common/ModuleAccess.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"
#include "inet/visualizer/vsg/util/VsgScene.h"
#include "inet/visualizer/vsg/util/VsgUtils.h"

namespace inet {

namespace visualizer {

using namespace inet::physicallayer;

Define_Module(RadioVsgVisualizer);

RadioVsgVisualizer::RadioVsgVisualization::RadioVsgVisualization(::vsg::ref_ptr<::vsg::Group> node, const int radioModuleId) :
    RadioVisualization(radioModuleId),
    node(node)
{
}

void RadioVsgVisualizer::initialize(int stage)
{
    RadioVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    // Nothing extra needed at INITSTAGE_LOCAL; radioVisualizations are
    // created on demand from receiveSignal (base class subscribes).
}

RadioVisualizerBase::RadioVisualization *RadioVsgVisualizer::createRadioVisualization(const IRadio *radio) const
{
    auto module = check_and_cast<const cModule *>(radio);
    auto networkNode = getContainingNode(module);

    // Place a small sphere marker at the network node's 3D position.
    // TODO: render radio-mode and reception/transmission state icons (images or
    //       colored annuli) once a VSG image/icon helper is available.
    Coord pos = getPosition(networkNode);

    auto group = ::vsg::Group::create();

    // Radio-mode indicator: small sphere (radius 2 m), color-coded by mode once refreshed.
    // Default color is grey; refreshRadioVisualization updates it.
    auto sphere = inet::vsg::createSphere(pos, 2.0, cFigure::GREY, 0.8);
    group->addChild(sphere);

    // Label showing the module name so the user can pick it.
    if (displayRadioMode) {
        auto label = inet::vsg::createLabel(module->getFullName(), Coord(pos.x, pos.y, pos.z + 3.0), cFigure::BLACK);
        group->addChild(label);
    }

    // Picking tag so clicking the marker identifies the radio module.
    group->setValue("omnetpp.object", (int64_t)(intptr_t)(cObject *)module);

    return new RadioVsgVisualization(group, module->getId());
}

void RadioVsgVisualizer::addRadioVisualization(const RadioVisualization *radioVisualization)
{
    RadioVisualizerBase::addRadioVisualization(radioVisualization);
    auto radioVsgVisualization = static_cast<const RadioVsgVisualization *>(radioVisualization);
    auto scene = inet::vsg::TopLevelScene::getSimulationScene(visualizationTargetModule);
    scene->addChild(radioVsgVisualization->node);
}

void RadioVsgVisualizer::removeRadioVisualization(const RadioVisualization *radioVisualization)
{
    RadioVisualizerBase::removeRadioVisualization(radioVisualization);
    auto radioVsgVisualization = static_cast<const RadioVsgVisualization *>(radioVisualization);
    auto scene = inet::vsg::TopLevelScene::getSimulationScene(visualizationTargetModule);
    auto& ch = scene->children;
    ::vsg::ref_ptr<::vsg::Node> nodeRef(radioVsgVisualization->node);
    ch.erase(std::remove(ch.begin(), ch.end(), nodeRef), ch.end());
}

void RadioVsgVisualizer::refreshRadioVisualization(const RadioVisualization *radioVisualization) const
{
    // TODO: rebuild the group's sphere color based on current radio mode /
    //       reception/transmission state.  VSG bakes color into geometry so this
    //       requires replacing child nodes rather than mutating a state set
    //       (unlike the OSG path).  Deferred until a mutable-color helper exists.
    (void)radioVisualization;
}

} // namespace visualizer

} // namespace inet
