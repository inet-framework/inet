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

#include "BarFigure.h"

namespace inet {

BarFigure::BarFigure(double value, double minValue, double maxValue, const char *name) :
    cGroupFigure(name),
    value(value),
    minValue(minValue),
    maxValue(maxValue)
{
    auto lineFigure = new cLineFigure();
    lineFigure->setStart(position + cFigure::Point(0, height));
    lineFigure->setEnd(position + cFigure::Point(0, height));
    lineFigure->setLineWidth(width);
    lineFigure->setZoomLineWidth(false);
    addFigure(lineFigure);
    refreshDisplay();
}

void BarFigure::setPosition(const cFigure::Point& position)
{
    this->position = position;
    refreshDisplay();
}

void BarFigure::setValue(double value)
{
    this->value = value;
    refreshDisplay();
}

void BarFigure::refreshDisplay()
{
    auto lineFigure = static_cast<cLineFigure *>(getFigure(0));
    auto alpha = (value - minValue) / (maxValue - minValue);
    alpha = std::min(std::max(alpha, 0.0), 1.0);
    lineFigure->setStart(position + cFigure::Point(0, height));
    lineFigure->setEnd(position + cFigure::Point(0, height * (1 - alpha)));
}

} // namespace inet

