//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/figures/GateFigure.h"

namespace inet {

GateFigure::GateFigure(const char *name) :
    cRectangleFigure(name)
{
    labelFigure = new cLabelFigure();
    labelFigure->setAnchor(cFigure::ANCHOR_W);
    addFigure(labelFigure);
    positionFigure = new cLineFigure();
    positionFigure->setLineColor(cFigure::BLACK);
    positionFigure->setLineStyle(cFigure::LINE_DOTTED);
    addFigure(positionFigure);
}

void GateFigure::setBounds(const Rectangle& bounds)
{
    cRectangleFigure::setBounds(bounds);
    labelFigure->setPosition(cFigure::Point(0, bounds.height / 2));
}

void GateFigure::setLabel(const char *text)
{
    auto size = getBounds().getSize();
    labelFigure->setText(text);
    auto font = labelFigure->getFont();
    font.pointSize = size.y;
    double outWidth;
    double outHeight;
    double outAscent;
    getEnvir()->getTextExtent(font, text, outWidth, outHeight, outAscent);
    font.pointSize *= size.y / outHeight;
    labelFigure->setFont(font);
}

void GateFigure::setPosition(double position)
{
    auto size = getBounds().getSize();
    this->position = position;
    positionFigure->setStart(cFigure::Point(position, 0));
    positionFigure->setEnd(cFigure::Point(position, size.y));
}

void GateFigure::addSchedule(double start, double end, bool open)
{
    auto size = getBounds().getSize();
    auto scheduleFigure = new cRectangleFigure();
    scheduleFigure->setOutlined(false);
    scheduleFigure->setFilled(true);
    scheduleFigure->setFillColor(open ? cFigure::GREEN : cFigure::RED);
    scheduleFigure->setBounds(cFigure::Rectangle(start, 0, end - start, size.y));
    scheduleFigures.push_back(scheduleFigure);
    addFigure(scheduleFigure, 0);
}

void GateFigure::clearSchedule()
{
    for (auto scheduleFigure : scheduleFigures)
        delete removeFigure(scheduleFigure);
    scheduleFigures.clear();
}

} // namespace inet

