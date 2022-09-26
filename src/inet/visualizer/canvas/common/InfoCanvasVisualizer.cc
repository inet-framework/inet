//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/canvas/common/InfoCanvasVisualizer.h"

#include "inet/common/ModuleAccess.h"

namespace inet {

namespace visualizer {

Define_Module(InfoCanvasVisualizer);

InfoCanvasVisualizer::InfoCanvasVisualization::InfoCanvasVisualization(NetworkNodeCanvasVisualization *networkNodeVisualization, BoxedLabelFigure *figure, int moduleId) :
    InfoVisualization(moduleId),
    networkNodeVisualization(networkNodeVisualization),
    figure(figure)
{
}

InfoCanvasVisualizer::InfoCanvasVisualization::~InfoCanvasVisualization()
{
    delete figure;
}

void InfoCanvasVisualizer::initialize(int stage)
{
    InfoVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        networkNodeVisualizer.reference(this, "networkNodeVisualizerModule", true);
    }
}

InfoVisualizerBase::InfoVisualization *InfoCanvasVisualizer::createInfoVisualization(cModule *module) const
{
    auto figure = new BoxedLabelFigure("info");
    figure->setTags((std::string("info ") + tags).c_str());
    figure->setTooltip("This label represents some module information");
    figure->setAssociatedObject(module);
    figure->setZIndex(zIndex);
    figure->setFont(font);
    figure->setText(getInfoVisualizationText(module).c_str());
    figure->setLabelColor(textColor);
    figure->setBackgroundColor(backgroundColor);
    figure->setOpacity(opacity);
    auto networkNode = getContainingNode(module);
    auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
    return new InfoCanvasVisualization(networkNodeVisualization, figure, module->getId());
}

void InfoCanvasVisualizer::addInfoVisualization(const InfoVisualization *infoVisualization)
{
    InfoVisualizerBase::addInfoVisualization(infoVisualization);
    auto infoCanvasVisualization = static_cast<const InfoCanvasVisualization *>(infoVisualization);
    auto figure = infoCanvasVisualization->figure;
    infoCanvasVisualization->networkNodeVisualization->addAnnotation(figure, figure->getBounds().getSize(), placementHint, placementPriority);
}

void InfoCanvasVisualizer::removeInfoVisualization(const InfoVisualization *infoVisualization)
{
    InfoVisualizerBase::removeInfoVisualization(infoVisualization);
    auto infoCanvasVisualization = static_cast<const InfoCanvasVisualization *>(infoVisualization);
    auto figure = infoCanvasVisualization->figure;
    if (networkNodeVisualizer != nullptr)
        infoCanvasVisualization->networkNodeVisualization->removeAnnotation(figure);
}

void InfoCanvasVisualizer::refreshInfoVisualization(const InfoVisualization *infoVisualization, const char *info) const
{
    auto infoCanvasVisualization = static_cast<const InfoCanvasVisualization *>(infoVisualization);
    auto figure = infoCanvasVisualization->figure;
    figure->setText(info);
    infoCanvasVisualization->networkNodeVisualization->setAnnotationSize(figure, figure->getBounds().getSize());
}

} // namespace visualizer

} // namespace inet

