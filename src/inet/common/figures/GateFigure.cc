//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

