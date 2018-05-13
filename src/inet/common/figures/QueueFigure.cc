//
// Copyright (C) OpenSim Ltd.
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
        else  {
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
