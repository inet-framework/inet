//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/vsg/power/EnergyStorageVsgVisualizer.h"

#include "inet/common/ModuleAccess.h"
#include "inet/visualizer/vsg/util/VsgUtils.h"

namespace inet {

using namespace power;

namespace visualizer {

Define_Module(EnergyStorageVsgVisualizer);

EnergyStorageVsgVisualizer::EnergyStorageVsgVisualization::EnergyStorageVsgVisualization(NetworkNodeVsgVisualization *networkNodeVisualization, ::vsg::ref_ptr<::vsg::Group> node, const IEnergyStorage *energyStorage) :
    EnergyStorageVisualization(energyStorage),
    networkNodeVisualization(networkNodeVisualization),
    node(node)
{
}

void EnergyStorageVsgVisualizer::initialize(int stage)
{
    EnergyStorageVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL)
        networkNodeVisualizer.reference(this, "networkNodeVisualizerModule", true);
    else if (stage == INITSTAGE_LAST) {
        for (auto energyStorageVisualization : energyStorageVisualizations) {
            auto energyStorageVsgVisualization = static_cast<const EnergyStorageVsgVisualization *>(energyStorageVisualization);
            energyStorageVsgVisualization->networkNodeVisualization->addAnnotation(energyStorageVsgVisualization->node, ::vsg::dvec3(0, 0, 0), 0);
        }
    }
}

EnergyStorageVisualizerBase::EnergyStorageVisualization *EnergyStorageVsgVisualizer::createEnergyStorageVisualization(const IEnergyStorage *energyStorage) const
{
    auto module = check_and_cast<const cModule *>(energyStorage);
    auto node = ::vsg::Group::create();
    auto networkNode = getContainingNode(module);
    auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
    return new EnergyStorageVsgVisualization(networkNodeVisualization, node, energyStorage);
}

void EnergyStorageVsgVisualizer::refreshEnergyStorageVisualization(const EnergyStorageVisualization *energyStorageVisualization) const
{
    auto energyStorageVsgVisualization = static_cast<const EnergyStorageVsgVisualization *>(energyStorageVisualization);
    auto energyStorage = energyStorageVisualization->energyStorage;
    double nominal = getNominalCapacity(energyStorage);
    double residual = getResidualCapacity(energyStorage);
    // TODO: approximation — the OSG version was also unimplemented (had a TODO); here we show
    //       a text label with the charge level as a percentage. A proper bar (like the canvas
    //       BarFigure) would require a custom geometry rebuilt on each refresh.
    //       (EnergyStorageVsgVisualizer.cc)
    char buf[64];
    if (nominal > 0)
        snprintf(buf, sizeof(buf), "%.0f%%", 100.0 * residual / nominal);
    else
        snprintf(buf, sizeof(buf), "%.3g J", residual);
    std::string text(buf);
    if (energyStorageVsgVisualization->lastText == text)
        return; // unchanged — avoid rebuilding the label
    energyStorageVsgVisualization->lastText = text;
    energyStorageVsgVisualization->node->children.clear();
    energyStorageVsgVisualization->node->addChild(inet::vsg::createLabel(text.c_str(), Coord::ZERO, color, 18));
}

} // namespace visualizer

} // namespace inet
