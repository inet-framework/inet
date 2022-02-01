//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

