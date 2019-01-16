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

#include "inet/common/figures/BarFigure.h"

namespace inet {

BarFigure::BarFigure(const char *name) :
    cRectangleFigure(name)
{
    valueFigure = new cRectangleFigure();
    valueFigure->setFilled(true);
    addFigure(valueFigure);
}

void BarFigure::setValue(double value)
{
    if (this->value != value) {
        this->value = value;
        refreshDisplay();
    }
}

void BarFigure::refreshDisplay()
{
    auto alpha = (value - minValue) / (maxValue - minValue);
    alpha = std::min(std::max(alpha, 0.0), 1.0);
    auto& bounds = getBounds();
    auto height = (bounds.height - 2 * spacing) * alpha;
    valueFigure->setBounds(cFigure::Rectangle(spacing, bounds.height - spacing - height, bounds.width - 2 * spacing, height));
}

} // namespace inet

