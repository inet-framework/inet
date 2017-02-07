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

#include "inet/visualizer/scene/NetworkNodeCanvasVisualization.h"

namespace inet {

namespace visualizer {

NetworkNodeCanvasVisualization::Annotation::Annotation(cFigure *figure, cFigure::Point size) :
    figure(figure),
    size(size)
{
}

NetworkNodeCanvasVisualization::NetworkNodeCanvasVisualization(cModule *networkNode) :
    cGroupFigure(networkNode->getFullName()),
    networkNode(networkNode)
{
    // TODO: determine size from icon
    // const char *icon = networkNode->getDisplayString().getTagArg("i", 0);
    size.x = 40;
    size.y = 40;
}

void NetworkNodeCanvasVisualization::updateAnnotationPositions()
{
    double spacing = 4;
    double totalHeight = 0;
    for (auto annotation : annotations) {
        double dx = -annotation.size.x / 2;
        double dy = -size.y / 2 - spacing - totalHeight - annotation.size.y;
        annotation.figure->setTransform(cFigure::Transform().translate(dx, dy));
        totalHeight += annotation.size.y + spacing;
    }
}

void NetworkNodeCanvasVisualization::addAnnotation(cFigure *figure, cFigure::Point size)
{
    annotations.push_back(Annotation(figure, size));
    addFigure(figure);
    updateAnnotationPositions();
}

void NetworkNodeCanvasVisualization::removeAnnotation(cFigure *figure)
{
    for (auto it = annotations.begin(); it != annotations.end(); it++) {
        if ((*it).figure == figure) {
            annotations.erase(it);
            break;
        }
    }
    removeFigure(figure);
    updateAnnotationPositions();
}

void NetworkNodeCanvasVisualization::setAnnotationSize(cFigure *figure, cFigure::Point size)
{
    for (auto it = annotations.begin(); it != annotations.end(); it++) {
        if ((*it).figure == figure) {
            it->size = size;
        }
    }
    updateAnnotationPositions();
}

} // namespace visualizer

} // namespace inet

