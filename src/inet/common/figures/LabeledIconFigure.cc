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

#include "inet/common/figures/LabeledIconFigure.h"

namespace inet {

LabeledIconFigure::LabeledIconFigure(const char *name) :
    cGroupFigure(name)
{
    iconFigure = new cIconFigure("icon");
    iconFigure->setAnchor(cFigure::ANCHOR_NW);
    labelFigure = new cLabelFigure("label");
    labelFigure->setAnchor(cFigure::ANCHOR_CENTER);
    labelFigure->setTags("label");
    labelFigure->setHalo(true);
    addFigure(iconFigure);
    addFigure(labelFigure);
}

void LabeledIconFigure::setTooltip(const char *tooltip)
{
    iconFigure->setTooltip(tooltip);
    labelFigure->setTooltip(tooltip);
}

void LabeledIconFigure::setAssociatedObject(cObject * object)
{
    iconFigure->setAssociatedObject(object);
    labelFigure->setAssociatedObject(object);
}

cFigure::Rectangle LabeledIconFigure::getBounds() const
{
    auto iconBounds = iconFigure->getBounds();
    auto labelBounds = labelFigure->getBounds();
    auto x = std::min(iconBounds.x, labelBounds.x);
    auto y = std::min(iconBounds.y, labelBounds.y);
    auto width = std::max(iconBounds.x + iconBounds.width, labelBounds.x + labelBounds.width) - x;
    auto height = std::max(iconBounds.y + iconBounds.height, labelBounds.y + labelBounds.height) - y;
    return cFigure::Rectangle(x, y, width, height);
}

void LabeledIconFigure::setOpacity(double opacity)
{
    iconFigure->setOpacity(opacity);
    labelFigure->setOpacity(opacity);
}

void LabeledIconFigure::setPosition(cFigure::Point position)
{
    iconFigure->setPosition(position);
    labelFigure->setPosition(position);
}

} // namespace inet

