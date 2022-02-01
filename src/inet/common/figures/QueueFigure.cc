//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/figures/QueueFigure.h"

namespace inet {

QueueFigure::QueueFigure(const char *name) :
    cRectangleFigure(name)
{
}

void QueueFigure::setMaxElementCount(int maxElementCount)
{
    if (this->maxElementCount != maxElementCount) {
        this->maxElementCount = maxElementCount;
        for (auto box : boxes) {
            removeFigure(box);
            delete box;
        }
        boxes.clear();
        const auto& bounds = getBounds();
        elementWidth = bounds.width - 2 * spacing;
        elementHeight = (bounds.height - (maxElementCount + 1) * spacing) / maxElementCount;
        continuous = elementHeight < spacing * 2;
        if (continuous) {
            auto box = new cRectangleFigure("box");
            box->setOutlined(false);
            box->setFilled(true);
            box->setFillColor(color);
            boxes.push_back(box);
            addFigure(box);
        }
        else {
            for (int i = 0; i < maxElementCount; i++) {
                auto box = new cRectangleFigure("box");
                box->setOutlined(false);
                box->setFilled(true);
                box->setFillColor(color);
                box->setBounds(cFigure::Rectangle(spacing, spacing + i * (elementHeight + spacing), elementWidth, elementHeight));
                boxes.push_back(box);
                addFigure(box);
            }
        }
    }
}

void QueueFigure::setElementCount(int elementCount)
{
    if (this->elementCount != elementCount) {
        this->elementCount = elementCount;
        if (continuous) {
            const auto& bounds = getBounds();
            double width = bounds.width - 2 * spacing;
            double height = (bounds.height - 2 * spacing) * elementCount / maxElementCount;
            boxes[0]->setBounds(cFigure::Rectangle(spacing, bounds.height - spacing - height, width, height));
        }
        else
            for (size_t i = 0; i < boxes.size(); i++)
                boxes[i]->setVisible((boxes.size() - i) <= (size_t)elementCount);
    }
}

} // namespace inet

