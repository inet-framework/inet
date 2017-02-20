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
    labelFigure = new cLabelFigure("label");
    rectangleFigure = new cRectangleFigure("box");
    rectangleFigure->setFilled(true);
    rectangleFigure->setLineColor(cFigure::BLACK);
    addFigure(rectangleFigure);
    addFigure(labelFigure);
    setInset(inset);
    setText(" "); // TODO: empty string causes negative width/height in Tkenv
}

void BoxedLabelFigure::setInset(double inset)
{
    this->inset = inset;
    labelFigure->setPosition(cFigure::Point(inset, inset));
    rectangleFigure->setCornerRx(inset);
    rectangleFigure->setCornerRy(inset);
}

void BoxedLabelFigure::setTags(const char *tags)
{
    labelFigure->setTags(tags);
    rectangleFigure->setTags(tags);
}

void BoxedLabelFigure::setTooltip(const char *tooltip)
{
    labelFigure->setTooltip(tooltip);
    rectangleFigure->setTooltip(tooltip);
}

void BoxedLabelFigure::setAssociatedObject(cObject * object)
{
    labelFigure->setAssociatedObject(object);
    rectangleFigure->setAssociatedObject(object);
}

const cFigure::Rectangle& BoxedLabelFigure::getBounds() const
{
    return rectangleFigure->getBounds();
}

const cFigure::Font& BoxedLabelFigure::getFont() const
{
    return labelFigure->getFont();
}

void BoxedLabelFigure::setFont(cFigure::Font font)
{
    labelFigure->setFont(font);
}

const cFigure::Color& BoxedLabelFigure::getLabelColor() const
{
    return labelFigure->getColor();
}

void BoxedLabelFigure::setLabelColor(cFigure::Color color)
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
    double width, height, ascent;
    getSimulation()->getEnvir()->getTextExtent(labelFigure->getFont(), text, width, height, ascent);
    rectangleFigure->setBounds(cFigure::Rectangle(0, 0, width + inset * 2, height + inset * 2));
    labelFigure->setText(text);
}

double BoxedLabelFigure::getOpacity() const
{
    return rectangleFigure->getFillOpacity();
}

void BoxedLabelFigure::setOpacity(double opacity)
{
    rectangleFigure->setFillOpacity(opacity);
    rectangleFigure->setLineOpacity(opacity);
    labelFigure->setOpacity(opacity);
}

} // namespace inet

