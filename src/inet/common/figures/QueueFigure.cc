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

void QueueFigure::setElementCount(int elementCount)
{
    for (auto box : boxes) {
        removeFigure(box);
        delete box;
    }
    boxes.clear();
    for (int i = 0; i < elementCount; i++) {
        auto box = new cRectangleFigure("box");
        box->setOutlined(false);
        box->setFilled(true);
        box->setFillColor(color);
        box->setBounds(cFigure::Rectangle(spacing, spacing + i * (elementHeight + spacing), elementWidth, elementHeight));
        boxes.push_back(box);
        addFigure(box);
    }
    setBounds(cFigure::Rectangle(0, 0, 2 * spacing + elementWidth, spacing + elementCount * (elementHeight + spacing)));
}

void QueueFigure::setValue(int value)
{
    for (int i = 0; i < boxes.size(); i++)
        boxes[i]->setVisible((boxes.size() - i) <= value);
}

} // namespace inet
