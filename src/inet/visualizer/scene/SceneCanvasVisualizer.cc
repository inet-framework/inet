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
#include "inet/visualizer/scene/SceneCanvasVisualizer.h"

namespace inet {

namespace visualizer {

Define_Module(SceneCanvasVisualizer);

void SceneCanvasVisualizer::initialize(int stage)
{
    SceneVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        cCanvas *canvas = visualizerTargetModule->getCanvas();
        canvasProjection.setRotation(Rotation(computeViewAngle(par("viewAngle"))));
        canvasProjection.setScale(parse2D(par("viewScale")));
        canvasProjection.setTranslation(parse2D(par("viewTranslation")));
        CanvasProjection::setCanvasProjection(canvas, &canvasProjection);
        axisLayer = new cGroupFigure("axisLayer");
        axisLayer->setZIndex(zIndex);
        axisLayer->insertBelow(canvas->getSubmodulesLayer());
        double axisLength = par("axisLength");
        if (!std::isnan(axisLength))
            initializeAxis(axisLength);
        std::string descriptionFigurePath = par("descriptionFigure");
        if (!descriptionFigurePath.empty())
            displayDescription(descriptionFigurePath.c_str());
    }
}

void SceneCanvasVisualizer::initializeAxis(double axisLength)
{
    cLineFigure *xAxis = new cLineFigure("xAxis");
    cLineFigure *yAxis = new cLineFigure("yAxis");
    cLineFigure *zAxis = new cLineFigure("zAxis");
    auto axisTags = std::string("axis ") + tags;
    xAxis->setTags(axisTags.c_str());
    yAxis->setTags(axisTags.c_str());
    zAxis->setTags(axisTags.c_str());
    xAxis->setTooltip("This arrow represents the X axis of the playground");
    yAxis->setTooltip("This arrow represents the Y axis of the playground");
    zAxis->setTooltip("This arrow represents the Z axis of the playground");
    xAxis->setLineWidth(1);
    yAxis->setLineWidth(1);
    zAxis->setLineWidth(1);
    xAxis->setEndArrowhead(cFigure::ARROW_BARBED);
    yAxis->setEndArrowhead(cFigure::ARROW_BARBED);
    zAxis->setEndArrowhead(cFigure::ARROW_BARBED);
    xAxis->setZoomLineWidth(false);
    yAxis->setZoomLineWidth(false);
    zAxis->setZoomLineWidth(false);
    xAxis->setStart(canvasProjection.computeCanvasPoint(Coord::ZERO));
    yAxis->setStart(canvasProjection.computeCanvasPoint(Coord::ZERO));
    zAxis->setStart(canvasProjection.computeCanvasPoint(Coord::ZERO));
    xAxis->setEnd(canvasProjection.computeCanvasPoint(Coord(axisLength, 0, 0)));
    yAxis->setEnd(canvasProjection.computeCanvasPoint(Coord(0, axisLength, 0)));
    zAxis->setEnd(canvasProjection.computeCanvasPoint(Coord(0, 0, axisLength)));
    axisLayer->addFigure(xAxis);
    axisLayer->addFigure(yAxis);
    axisLayer->addFigure(zAxis);
    cLabelFigure *xLabel = new cLabelFigure("xAxisLabel");
    cLabelFigure *yLabel = new cLabelFigure("yAxisLabel");
    cLabelFigure *zLabel = new cLabelFigure("zAxisLabel");
    auto axisLabelTags = std::string("axis label ") + tags;
    xLabel->setTags(axisLabelTags.c_str());
    yLabel->setTags(axisLabelTags.c_str());
    zLabel->setTags(axisLabelTags.c_str());
    xLabel->setText("X");
    yLabel->setText("Y");
    zLabel->setText("Z");
    xLabel->setPosition(canvasProjection.computeCanvasPoint(Coord(axisLength, 0, 0)));
    yLabel->setPosition(canvasProjection.computeCanvasPoint(Coord(0, axisLength, 0)));
    zLabel->setPosition(canvasProjection.computeCanvasPoint(Coord(0, 0, axisLength)));
    axisLayer->addFigure(xLabel);
    axisLayer->addFigure(yLabel);
    axisLayer->addFigure(zLabel);
}

void SceneCanvasVisualizer::handleParameterChange(const char* name)
{
    if (name && !strcmp(name, "viewAngle")) {
        canvasProjection.setRotation(Rotation(computeViewAngle(par("viewAngle"))));
        // TODO: update all visualizers
    }
    else if (name && !strcmp(name, "viewScale")) {
        canvasProjection.setScale(parse2D(par("viewScale")));
        // TODO: update all visualizers
    }
    else if (name && !strcmp(name, "viewTranslation")) {
        canvasProjection.setTranslation(parse2D(par("viewTranslation")));
        // TODO: update all visualizers
    }
}

EulerAngles SceneCanvasVisualizer::computeViewAngle(const char* viewAngle)
{
    double x, y, z;
    if (!strcmp(viewAngle, "x"))
    {
        x = 0;
        y = M_PI / 2;
        z = M_PI / -2;
    }
    else if (!strcmp(viewAngle, "y"))
    {
        x = M_PI / 2;
        y = 0;
        z = 0;
    }
    else if (!strcmp(viewAngle, "z"))
    {
        x = y = z = 0;
    }
    else if (!strncmp(viewAngle, "isometric", 9))
    {
        int v;
        int l = strlen(viewAngle);
        switch (l) {
            case 9: v = 0; break;
            case 10: v = viewAngle[9] - '0'; break;
            case 11: v = (viewAngle[9] - '0') * 10 + viewAngle[10] - '0'; break;
            default: throw cRuntimeError("Invalid isometric viewAngle parameter");
        }
        // 1st axis can point on the 2d plane in 6 directions
        // 2nd axis can point on the 2d plane in 4 directions (the opposite direction is forbidden)
        // 3rd axis can point on the 2d plane in 2 directions
        // this results in 6 * 4 * 2 = 48 different configurations
        x = math::deg2rad(45 + v % 4 * 90);
        y = math::deg2rad(v / 24 % 2 ? 35.27 : -35.27);
        z = math::deg2rad(30 + v / 4 % 6 * 60);
    }
    else if (sscanf(viewAngle, "%lf %lf %lf", &x, &y, &z) == 3)
    {
        x = math::deg2rad(x);
        y = math::deg2rad(y);
        z = math::deg2rad(z);
    }
    else
        throw cRuntimeError("The viewAngle parameter must be a predefined string or a triplet representing three degrees");
    return EulerAngles(x, y, z);
}

cFigure::Point SceneCanvasVisualizer::parse2D(const char* text)
{
    double x, y;
    if (sscanf(text, "%lf %lf", &x, &y) != 2)
        throw cRuntimeError("The parameter must be a pair of doubles: %s", text);
    return cFigure::Point(x, y);
}

void SceneCanvasVisualizer::displayDescription(const char *descriptionFigurePath)
{
    cFigure* descriptionFigure = visualizerTargetModule->getCanvas()->getFigureByPath(descriptionFigurePath);
    if (!descriptionFigure)
        throw cRuntimeError("Figure \"%s\" not found", descriptionFigurePath);
    auto descriptionTextFigure = check_and_cast<cAbstractTextFigure*>(descriptionFigure);

    auto config = getEnvir()->getConfigEx();
    const char *activeConfig = config->getActiveConfigName();
    std::string description = std::string(activeConfig) + ": " + config->getConfigDescription(activeConfig);
    descriptionTextFigure->setText(description.c_str());
}


} // namespace visualizer

} // namespace inet

