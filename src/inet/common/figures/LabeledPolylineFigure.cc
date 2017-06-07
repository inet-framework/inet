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

#include "inet/common/figures/LabeledPolylineFigure.h"

namespace inet {

LabeledPolylineFigure::LabeledPolylineFigure(const char *name) :
    cGroupFigure(name)
{
    polylineFigure = new cPolylineFigure("line");
    panelFigure = new cPanelFigure("panel");
    addFigure(polylineFigure);
    addFigure(panelFigure);
    labelFigure = new cTextFigure("label");
    labelFigure->setAnchor(cFigure::ANCHOR_S);
    labelFigure->setTags("label");
    labelFigure->setHalo(true);
    panelFigure->addFigure(labelFigure);
}

void LabeledPolylineFigure::setPoints(const std::vector<cFigure::Point>& points)
{
    polylineFigure->setPoints(points);
    updateLabelPosition();
}

void LabeledPolylineFigure::updateLabelPosition()
{
    auto points = polylineFigure->getPoints();
    int index = (points.size() - 1) / 2;
    auto position = (points[index] + points[index + 1]) / 2;
    auto direction = points[index + 1] - points[index];
    double alpha = atan2(-direction.y, direction.x);
    if (alpha > M_PI / 2 || alpha < -M_PI / 2)
        alpha += M_PI;
    panelFigure->setTransform(cFigure::Transform().rotate(-alpha));
    panelFigure->setPosition(position);
    labelFigure->setPosition(cFigure::Point(0, -polylineFigure->getLineWidth() / 2));
}

} // namespace inet

