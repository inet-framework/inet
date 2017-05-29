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

#include <algorithm>
#include "inet/common/INETMath.h"
#include "inet/common/figures/BoxedLabelFigure.h"
#include "inet/visualizer/scene/NetworkNodeCanvasVisualization.h"

namespace inet {

namespace visualizer {

NetworkNodeCanvasVisualization::Annotation::Annotation(cFigure *figure, const cFigure::Point& size, Displacement displacement, double priority) :
    figure(figure),
    bounds(cFigure::Rectangle(NaN, NaN, size.x, size.y)),
    displacement(displacement),
    priority(priority)
{
}

static BoxedLabelFigure *createRectangle(const char *label) {
    auto figure = new BoxedLabelFigure();
    figure->setText(label);
    return figure;
}

NetworkNodeCanvasVisualization::NetworkNodeCanvasVisualization(cModule *networkNode, double annotationSpacing) :
    cPanelFigure(networkNode->getFullName()),
    networkNode(networkNode),
    annotationSpacing(annotationSpacing)
{
    submoduleBounds = getEnvir()->getSubmoduleBounds(networkNode);
    submoduleBounds.x = -submoduleBounds.width / 2;
    submoduleBounds.y = -submoduleBounds.height / 2;
}

void NetworkNodeCanvasVisualization::refreshDisplay()
{
    if (isLayoutInvalid)
        layout();
}

void NetworkNodeCanvasVisualization::addAnnotation(cFigure *figure, cFigure::Point size, Displacement displacement, double priority)
{
    annotations.push_back(Annotation(figure, size, displacement, priority));
    addFigure(figure);
    isLayoutInvalid = true;
}

void NetworkNodeCanvasVisualization::removeAnnotation(cFigure *figure)
{
    for (auto it = annotations.begin(); it != annotations.end(); it++) {
        auto& annotation = *it;
        if (annotation.figure == figure) {
            annotations.erase(it);
            break;
        }
    }
    removeFigure(figure);
    isLayoutInvalid = true;
}

void NetworkNodeCanvasVisualization::setAnnotationSize(cFigure *figure, cFigure::Point size)
{
    for (auto it = annotations.begin(); it != annotations.end(); it++) {
        auto& annotation = *it;
        if (annotation.figure == figure) {
            annotation.bounds.width = size.x;
            annotation.bounds.height = size.y;
            break;
        }
    }
    isLayoutInvalid = true;
}

void NetworkNodeCanvasVisualization::setAnnotationVisible(cFigure *figure, bool visible)
{
    figure->setVisible(visible);
    isLayoutInvalid = true;
}

static cFigure::Point getTopLeft(const cFigure::Rectangle& rc) {
    return cFigure::Point(rc.x, rc.y);
}

static cFigure::Point getTopCenter(const cFigure::Rectangle& rc) {
    return cFigure::Point(rc.x + rc.width / 2, rc.y);
}

static cFigure::Point getTopRight(const cFigure::Rectangle& rc) {
    return cFigure::Point(rc.x + rc.width, rc.y);
}

static cFigure::Point getCenterLeft(const cFigure::Rectangle& rc) {
    return cFigure::Point(rc.x, rc.y + rc.height / 2);
}

static cFigure::Point getCenterCenter(const cFigure::Rectangle& rc) {
    return cFigure::Point(rc.x + rc.width / 2, rc.y + rc.height / 2);
}

static cFigure::Point getCenterRight(const cFigure::Rectangle& rc) {
    return cFigure::Point(rc.x + rc.width, rc.y + rc.height / 2);
}

static cFigure::Point getBottomLeft(const cFigure::Rectangle& rc) {
    return cFigure::Point(rc.x, rc.y + rc.height);
}

static cFigure::Point getBottomCenter(const cFigure::Rectangle& rc) {
    return cFigure::Point(rc.x + rc.width / 2, rc.y + rc.height);
}

static cFigure::Point getBottomRight(const cFigure::Rectangle& rc) {
    return cFigure::Point(rc.x + rc.width, rc.y + rc.height);
}

static bool containsPoint(const cFigure::Rectangle& rc, const cFigure::Point& p) {
    return rc.x < p.x && p.x < rc.x + rc.width && rc.y < p.y && p.y < rc.y + rc.height;
}

static bool intersectsRectangle(const cFigure::Rectangle& rc1, const cFigure::Rectangle& rc2) {
    return rc1.x < rc2.x + rc2.width && rc1.x + rc1.width > rc2.x &&
           rc1.y < rc2.y + rc2.height && rc1.y + rc1.height > rc2.y;
}

static cFigure::Rectangle createRectangle(const cFigure::Point& pt, const cFigure::Point& rs)
{
    return cFigure::Rectangle(pt.x, pt.y, rs.x, rs.y);
}

static void pushUnlessContains(std::vector<cFigure::Point>& pts, const std::vector<cFigure::Rectangle>& rcs, const cFigure::Point& pt)
{
    for (int j = 0; j < (int)rcs.size(); j++) {
        cFigure::Rectangle rc = rcs[j];
        if (containsPoint(rc, pt))
            return;
    }
    pts.push_back(pt);
}

static Displacement getDisplacement(const cFigure::Rectangle& rc, const cFigure::Point& pt) {
    if (pt.x <= rc.x && pt.y <= rc.y)
        return DISPLACEMENT_TOP_LEFT;
    else if (rc.x <= pt.x && pt.x <= rc.x + rc.width && pt.y <= rc.y)
        return DISPLACEMENT_TOP_CENTER;
    else if (pt.x >= rc.x + rc.width && pt.y <= rc.y)
        return DISPLACEMENT_TOP_RIGHT;
    else if (pt.x <= rc.x && rc.y <= pt.y && pt.y <= rc.y + rc.height)
        return DISPLACEMENT_CENTER_LEFT;
    else if (pt.x >= rc.x + rc.width && rc.y <= pt.y && pt.y <= rc.y + rc.height)
        return DISPLACEMENT_CENTER_RIGHT;
    else if (pt.x <= rc.x && pt.y >= rc.y + rc.height)
        return DISPLACEMENT_BOTTOM_LEFT;
    else if (rc.x <= pt.x && pt.x <= rc.x + rc.width && pt.y >= rc.y + rc.height)
        return DISPLACEMENT_BOTTOM_CENTER;
    else if (pt.x >= rc.x + rc.width && pt.y >= rc.y + rc.height)
        return DISPLACEMENT_BOTTOM_RIGHT;
    else
        return DISPLACEMENT_NONE;
}

bool NetworkNodeCanvasVisualization::Annotation::comparePriority(const Annotation& a1, const Annotation& a2) {
    return a1.priority < a2.priority;
}

void NetworkNodeCanvasVisualization::layout()
{
    std::vector<cFigure::Rectangle> rcs;  // rectangles of annotations already positioned
    std::vector<cFigure::Point> pts;  // candidate points where annotations may be positioned
    cFigure::Rectangle extendendSubmoduleBounds = submoduleBounds;
    extendendSubmoduleBounds.x -= annotationSpacing;
    extendendSubmoduleBounds.y -= annotationSpacing;
    extendendSubmoduleBounds.width += 2 * annotationSpacing;
    extendendSubmoduleBounds.height += 2 * annotationSpacing;
    rcs.push_back(extendendSubmoduleBounds);
    pts.push_back(getTopLeft(extendendSubmoduleBounds));
    pts.push_back(getTopCenter(extendendSubmoduleBounds));
    pts.push_back(getTopRight(extendendSubmoduleBounds));
    pts.push_back(getCenterLeft(extendendSubmoduleBounds));
    pts.push_back(getCenterRight(extendendSubmoduleBounds));
    pts.push_back(getBottomLeft(extendendSubmoduleBounds));
    pts.push_back(getBottomCenter(extendendSubmoduleBounds));
    pts.push_back(getBottomRight(extendendSubmoduleBounds));

    std::sort(annotations.begin(), annotations.end(), Annotation::comparePriority);

    // delete all annotation positions
    for (auto it = annotations.begin(); it != annotations.end(); it++) {
        auto& annotation = *it;
        annotation.bounds.x = NaN;
        annotation.bounds.y = NaN;
    }

    for (auto it = annotations.begin(); it != annotations.end(); it++) {
        auto& annotation = *it;
        if (!annotation.figure->isVisible())
            continue;
        cFigure::Point rs = cFigure::Point(annotation.bounds.width, annotation.bounds.height);

        // find the best minimizing the distance cost function
        double bestDistance = std::numeric_limits<double>::infinity();
        cFigure::Rectangle bestRc;

        // for all candidate points
        for (int j = 0; j < (int)pts.size(); j++) {
            cFigure::Point pt = pts[j];

            // align annotation to candidate points with its various points
            for (int k = 0; k < 8; k++) {
                cFigure::Rectangle candidateRc;
                switch (k) {
                    case 0:
                        // candidate point is top left
                        candidateRc = createRectangle(pt - cFigure::Point(0, 0), rs);
                        break;
                    case 1:
                        // candidate point is top center
                        candidateRc = createRectangle(pt - cFigure::Point(rs.x / 2, 0), rs);
                        break;
                    case 2:
                        // candidate point is top right
                        candidateRc = createRectangle(pt - cFigure::Point(rs.x, 0), rs);
                        break;
                    case 3:
                        // candidate point is center left
                        candidateRc = createRectangle(pt - cFigure::Point(0, rs.y / 2), rs);
                        break;
                    case 4:
                        // candidate point is center right
                        candidateRc = createRectangle(pt - cFigure::Point(rs.x, rs.y / 2), rs);
                        break;
                    case 5:
                        // candidate point is bottom left
                        candidateRc = createRectangle(pt - cFigure::Point(0, rs.y), rs);
                        break;
                    case 6:
                        // candidate point is bottom center
                        candidateRc = createRectangle(pt - cFigure::Point(rs.x / 2, rs.y), rs);
                        break;
                    case 7:
                        // candidate point is bottom right
                        candidateRc = createRectangle(pt - cFigure::Point(rs.x, rs.y), rs);
                        break;
                }

                Displacement candidateDisplacement = getDisplacement(submoduleBounds, getCenterCenter(candidateRc));
                if (!(annotation.displacement & candidateDisplacement))
                    continue;

                // find an already positioned annotation which would intersect the candidate rectangle
                bool intersects = false;
                for (int l = 0; l < (int)rcs.size(); l++) {
                    cFigure::Rectangle rc = rcs[l];
                    if (intersectsRectangle(candidateRc, rc)) {
                        intersects = true;
                        break;
                    }
                }
                if (intersects)
                    continue;

                // if better than the current best
                double distance = getCenterCenter(submoduleBounds).distanceTo(getCenterCenter(candidateRc));
                if (distance < bestDistance) {
                    bestRc = candidateRc;
                    bestDistance = distance;
                    if (distance == 0)
                        goto found;
                }
            }
        }

    found:
        // store position and rectangle
        annotation.bounds = cFigure::Rectangle(bestRc.x, bestRc.y, annotation.bounds.width, annotation.bounds.height);
        annotation.figure->setTransform(cFigure::Transform().translate(annotation.bounds.x, annotation.bounds.y));

        // grow rectangle
        bestRc.x -= annotationSpacing;
        bestRc.y -= annotationSpacing;
        bestRc.width += 2 * annotationSpacing;
        bestRc.height += 2 * annotationSpacing;

        // delete candidate points covered by best rc
        for (int j = 0; j < (int)pts.size(); j++) {
            cFigure::Point pt = pts[j];

            if (containsPoint(bestRc, pt))
                pts.erase(pts.begin() + j--);
        }

        // push new candidates
        pushUnlessContains(pts, rcs, getTopLeft(bestRc));
        pushUnlessContains(pts, rcs, getTopCenter(bestRc));
        pushUnlessContains(pts, rcs, getTopRight(bestRc));
        pushUnlessContains(pts, rcs, getCenterLeft(bestRc));
        pushUnlessContains(pts, rcs, getCenterRight(bestRc));
        pushUnlessContains(pts, rcs, getBottomLeft(bestRc));
        pushUnlessContains(pts, rcs, getBottomCenter(bestRc));
        pushUnlessContains(pts, rcs, getBottomRight(bestRc));

        rcs.push_back(bestRc);
    }
}

} // namespace visualizer

} // namespace inet

