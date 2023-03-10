//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/canvas/scene/SceneCanvasVisualizer.h"

#include "inet/common/ModuleAccess.h"

namespace inet {

namespace visualizer {

Define_Module(SceneCanvasVisualizer);

void SceneCanvasVisualizer::initialize(int stage)
{
    SceneVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        cCanvas *canvas = visualizationTargetModule->getCanvas();
        bool invertY;
        canvasProjection = CanvasProjection::getCanvasProjection(canvas);
        canvasProjection->setRotation(parseViewAngle(par("viewAngle"), invertY));
        canvasProjection->setScale(parse2D(par("viewScale"), invertY));
        canvasProjection->setTranslation(parse2D(par("viewTranslation")));
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
    xAxis->setTooltip("Scene X axis");
    yAxis->setTooltip("Scene Y axis");
    zAxis->setTooltip("Scene Z axis");
    xAxis->setLineWidth(1);
    yAxis->setLineWidth(1);
    zAxis->setLineWidth(1);
    xAxis->setEndArrowhead(cFigure::ARROW_BARBED);
    yAxis->setEndArrowhead(cFigure::ARROW_BARBED);
    zAxis->setEndArrowhead(cFigure::ARROW_BARBED);
    xAxis->setZoomLineWidth(false);
    yAxis->setZoomLineWidth(false);
    zAxis->setZoomLineWidth(false);
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
    axisLayer->addFigure(xLabel);
    axisLayer->addFigure(yLabel);
    axisLayer->addFigure(zLabel);
    refreshAxis(axisLength);
}

void SceneCanvasVisualizer::refreshAxis(double axisLength)
{
    auto xAxis = check_and_cast<cLineFigure *>(axisLayer->getFigure(0));
    auto yAxis = check_and_cast<cLineFigure *>(axisLayer->getFigure(1));
    auto zAxis = check_and_cast<cLineFigure *>(axisLayer->getFigure(2));
    auto xLabel = check_and_cast<cLabelFigure *>(axisLayer->getFigure(3));
    auto yLabel = check_and_cast<cLabelFigure *>(axisLayer->getFigure(4));
    auto zLabel = check_and_cast<cLabelFigure *>(axisLayer->getFigure(5));
    xAxis->setStart(canvasProjection->computeCanvasPoint(Coord::ZERO));
    yAxis->setStart(canvasProjection->computeCanvasPoint(Coord::ZERO));
    zAxis->setStart(canvasProjection->computeCanvasPoint(Coord::ZERO));
    xAxis->setEnd(canvasProjection->computeCanvasPoint(Coord(axisLength, 0, 0)));
    yAxis->setEnd(canvasProjection->computeCanvasPoint(Coord(0, axisLength, 0)));
    zAxis->setEnd(canvasProjection->computeCanvasPoint(Coord(0, 0, axisLength)));
    xLabel->setPosition(canvasProjection->computeCanvasPoint(Coord(axisLength, 0, 0)));
    yLabel->setPosition(canvasProjection->computeCanvasPoint(Coord(0, axisLength, 0)));
    zLabel->setPosition(canvasProjection->computeCanvasPoint(Coord(0, 0, axisLength)));
}

void SceneCanvasVisualizer::handleParameterChange(const char *name)
{
    if (!hasGUI()) return;
    if (!strcmp(name, "viewAngle")) {
        bool invertY;
        canvasProjection->setRotation(parseViewAngle(par("viewAngle"), invertY));
        canvasProjection->setScale(parse2D(par("viewScale"), invertY));
        // TODO update all visualizers
    }
    else if (!strcmp(name, "viewScale")) {
        canvasProjection->setScale(parse2D(par("viewScale")));
        // TODO update all visualizers
    }
    else if (!strcmp(name, "viewTranslation")) {
        canvasProjection->setTranslation(parse2D(par("viewTranslation")));
        // TODO update all visualizers
    }
    double axisLength = par("axisLength");
    if (!std::isnan(axisLength))
        refreshAxis(axisLength);
}

RotationMatrix SceneCanvasVisualizer::parseViewAngle(const char *viewAngle, bool& invertY)
{
    double a, b, c;
    if (*viewAngle == 'x' || *viewAngle == 'y' || *viewAngle == 'z') {
        double matrix[3][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
        cStringTokenizer tokenizer(viewAngle);
        while (tokenizer.hasMoreTokens()) {
            const char *axisToken = tokenizer.nextToken();
            if (axisToken == nullptr)
                throw cRuntimeError("Missing token in viewAngle parameter value");
            int i;
            if (!strcmp(axisToken, "x"))
                i = 0;
            else if (!strcmp(axisToken, "y"))
                i = 1;
            else if (!strcmp(axisToken, "z"))
                i = 2;
            else
                throw cRuntimeError("Invalid token in viewAngle parameter value: '%s'", axisToken);
            const char *directionToken = tokenizer.nextToken();
            if (directionToken == nullptr)
                throw cRuntimeError("Missing token in viewAngle parameter value");
            Coord v;
            if (!strcmp(directionToken, "right"))
                v = Coord::X_AXIS;
            else if (!strcmp(directionToken, "left"))
                v = -Coord::X_AXIS;
            else if (!strcmp(directionToken, "up"))
                v = Coord::Y_AXIS;
            else if (!strcmp(directionToken, "down"))
                v = -Coord::Y_AXIS;
            else if (!strcmp(directionToken, "out"))
                v = Coord::Z_AXIS;
            else if (!strcmp(directionToken, "in"))
                v = -Coord::Z_AXIS;
            else
                throw cRuntimeError("Invalid token in viewAngle parameter value: '%s'", directionToken);
            if (matrix[0][i] != 0 || matrix[1][i] != 0 || matrix[2][i] != 0)
                throw cRuntimeError("Invalid viewAngle parameter vale: '%s'", viewAngle);
            matrix[0][i] = v.x;
            matrix[1][i] = v.y;
            matrix[2][i] = v.z;
        }
        // NOTE: the rotation matrix cannot contain flipping, so the following code flips the resulting Y axis if needed, because the OMNeT++ 2D canvas also flips Y axis while drawing
        double determinant = matrix[0][0] * ((matrix[1][1] * matrix[2][2]) - (matrix[2][1] * matrix[1][2])) - matrix[0][1] * (matrix[1][0] * matrix[2][2] - matrix[2][0] * matrix[1][2]) + matrix[0][2] * (matrix[1][0] * matrix[2][1] - matrix[2][0] * matrix[1][1]);
        if (determinant == -1) {
            matrix[1][0] *= -1;
            matrix[1][1] *= -1;
            matrix[1][2] *= -1;
            invertY = false;
        }
        else
            invertY = true;
        return RotationMatrix(matrix);
    }
    else if (!strncmp(viewAngle, "isometric", 9)) {
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
        deg alpha = deg(45 + v % 4 * 90);
        deg beta = deg(v / 24 % 2 ? 35.27 : -35.27);
        deg gamma = deg(30 + v / 4 % 6 * 60);
        invertY = false;
        return RotationMatrix(EulerAngles(alpha, beta, gamma));
    }
    else if (sscanf(viewAngle, "%lf %lf %lf", &a, &b, &c) == 3) {
        invertY = false;
        return RotationMatrix(EulerAngles(deg(a), deg(b), deg(c)));
    }
    else
        throw cRuntimeError("Invalid viewAngle parameter value: '%s'", viewAngle);
}

cFigure::Point SceneCanvasVisualizer::parse2D(const char *text, bool invertY)
{
    double x, y;
    if (sscanf(text, "%lf %lf", &x, &y) != 2)
        throw cRuntimeError("The parameter must be a pair of doubles: %s", text);
    return cFigure::Point(x, invertY ? -y : y);
}

void SceneCanvasVisualizer::displayDescription(const char *descriptionFigurePath)
{
    cFigure *descriptionFigure = visualizationTargetModule->getCanvas()->getFigureByPath(descriptionFigurePath);
    if (!descriptionFigure)
        throw cRuntimeError("Figure \"%s\" not found", descriptionFigurePath);
    auto descriptionTextFigure = check_and_cast<cAbstractTextFigure *>(descriptionFigure);

#if OMNETPP_BUILDNUM < 2000
    auto config = getEnvir()->getConfigEx();
    const char *activeConfig = config->getActiveConfigName();
    std::string description = std::string(activeConfig) + ": " + config->getConfigDescription(activeConfig);
#else
    auto cfg = getEnvir()->getConfig();
    std::string description = std::string(cfg->getVariable(CFGVAR_CONFIGNAME)) + ": " + cfg->getVariable(CFGVAR_DESCRIPTION);
#endif
    descriptionTextFigure->setText(description.c_str());
}

} // namespace visualizer

} // namespace inet

