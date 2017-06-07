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

#include "inet/common/figures/LabeledLineFigure.h"

namespace inet {

LabeledLineFigure::LabeledLineFigure(const char *name) :
    cGroupFigure(name)
{
    lineFigure = new cLineFigure("line");
    panelFigure = new cPanelFigure("panel");
    addFigure(lineFigure);
    addFigure(panelFigure);
    labelFigure = new cTextFigure("label");
    labelFigure->setAnchor(cFigure::ANCHOR_S);
    labelFigure->setTags("label");
    labelFigure->setHalo(true);
    panelFigure->addFigure(labelFigure);
}

void LabeledLineFigure::setStart(cFigure::Point point)
{
    lineFigure->setStart(point);
    updateLabelPosition();
}

void LabeledLineFigure::setEnd(cFigure::Point point)
{
    lineFigure->setEnd(point);
    updateLabelPosition();
}

void LabeledLineFigure::updateLabelPosition()
{
    auto position = (lineFigure->getStart() + lineFigure->getEnd()) / 2;
    auto direction = lineFigure->getEnd() - lineFigure->getStart();
    double alpha = atan2(-direction.y, direction.x);
    if (alpha > M_PI / 2 || alpha < -M_PI / 2)
        alpha += M_PI;
    panelFigure->setTransform(cFigure::Transform().rotate(-alpha));
    panelFigure->setPosition(position);
    labelFigure->setPosition(cFigure::Point(0, -lineFigure->getLineWidth() / 2));
}

} // namespace inet

