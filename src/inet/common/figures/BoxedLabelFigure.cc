//
// Copyright (C) 2016 OpenSim Ltd.
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

#include "inet/common/figures/BoxedLabelFigure.h"

namespace inet {

BoxedLabelFigure::BoxedLabelFigure(const char *name) :
    cGroupFigure(name)
{
    labelFigure = new cLabelFigure("text");
    labelFigure->setPosition(cFigure::Point(spacing, spacing));
    rectangleFigure = new cRectangleFigure("border");
    rectangleFigure->setCornerRx(spacing);
    rectangleFigure->setCornerRy(spacing);
    rectangleFigure->setFilled(true);
    rectangleFigure->setFillOpacity(0.5);
    rectangleFigure->setLineColor(cFigure::BLACK);
    addFigure(rectangleFigure);
    addFigure(labelFigure);
    setText("");
}

const cFigure::Rectangle& BoxedLabelFigure::getBounds() const
{
    return rectangleFigure->getBounds();
}

const cFigure::Color& BoxedLabelFigure::getFontColor() const
{
    return labelFigure->getColor();
}

void BoxedLabelFigure::setFontColor(cFigure::Color color)
{
    labelFigure->setColor(color);
}

const cFigure::Color& BoxedLabelFigure::getBackgroundColor() const
{
    return rectangleFigure->getFillColor();
}

void BoxedLabelFigure::setBackgroundColor(cFigure::Color color)
{
    rectangleFigure->setFillColor(color);
}

const char *BoxedLabelFigure::getText() const
{
    return labelFigure->getText();
}

void BoxedLabelFigure::setText(const char *text)
{
    int width, height, ascent;
    getSimulation()->getEnvir()->getTextExtent(labelFigure->getFont(), text, width, height, ascent);
    rectangleFigure->setBounds(cFigure::Rectangle(0, 0, width + spacing * 2, height + spacing * 2));
    labelFigure->setText(text);
}

} // namespace inet

