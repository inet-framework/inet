//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/ModuleAccess.h"
#include "inet/visualizer/common/InfoCanvasVisualizer.h"

namespace inet {

namespace visualizer {

Define_Module(InfoCanvasVisualizer);

void InfoCanvasVisualizer::initialize(int stage)
{
    InfoVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        networkNodeVisualizer = getModuleFromPar<NetworkNodeCanvasVisualizer>(par("networkNodeVisualizerModule"), this);
        auto simulation = getSimulation();
        for (int i = 0; i < moduleIds.size(); i++) {
            double width = 100;
            double height = 24;
            double spacing = 4;
            auto labelFigure = new cLabelFigure("text");
            labelFigure->setColor(fontColor);
            labelFigure->setPosition(cFigure::Point(spacing, spacing));
            auto rectangleFigure = new cRectangleFigure("border");
            rectangleFigure->setCornerRx(spacing);
            rectangleFigure->setCornerRy(spacing);
            rectangleFigure->setFilled(true);
            rectangleFigure->setFillOpacity(0.5);
            rectangleFigure->setFillColor(backgroundColor);
            rectangleFigure->setLineColor(cFigure::BLACK);
            rectangleFigure->setBounds(cFigure::Rectangle(0, 0, width, height));
            auto groupFigure = new cGroupFigure("info");
            groupFigure->addFigure(rectangleFigure);
            groupFigure->addFigure(labelFigure);
            groupFigure->setZIndex(zIndex);
            auto module = simulation->getModule(moduleIds[i]);
            auto visualization = networkNodeVisualizer->getNeworkNodeVisualization(getContainingNode(module));
            visualization->addAnnotation(groupFigure, cFigure::Point(width, height));
            figures.push_back(groupFigure);
        }
    }
}

void InfoCanvasVisualizer::setInfo(int i, const char *info) const
{
    auto rectangleFigure = static_cast<cRectangleFigure *>(figures[i]->getFigure(0));
    auto labelFigure = static_cast<cLabelFigure *>(figures[i]->getFigure(1));
    double spacing = 4;
    int width, height, ascent;
    getSimulation()->getEnvir()->getTextExtent(labelFigure->getFont(), info, width, height, ascent);
    rectangleFigure->setBounds(cFigure::Rectangle(0, 0, 2 * spacing + width , 2 * spacing + height));
    labelFigure->setText(info);
}

} // namespace visualizer

} // namespace inet

