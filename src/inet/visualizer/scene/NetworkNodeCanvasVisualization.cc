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

NetworkNodeCanvasVisualization::Annotation::Annotation(cFigure *figure, const cFigure::Point& size, Placement placementHint, double placementPriority) :
    figure(figure),
    bounds(cFigure::Rectangle(NaN, NaN, size.x, size.y)),
    placementHint(placementHint),
    placementPriority(placementPriority)
{
}

NetworkNodeCanvasVisualization::NetworkNodeCanvasVisualization(cModule *networkNode, double annotationSpacing, double placementPenalty) :
    NetworkNodeVisualizerBase::NetworkNodeVisualization(networkNode),
    cGroupFigure(networkNode->getFullName()),
    annotationSpacing(annotationSpacing),
    placementPenalty(placementPenalty)
{
    annotationFigure = new cPanelFigure("annotation");
    addFigure(annotationFigure);
    submoduleBounds = getEnvir()->getSubmoduleBounds(networkNode);
    submoduleBounds.height += 32; // TODO: KLUDGE: extend bounds for submodule name label
    submoduleBounds.x = -submoduleBounds.width / 2;
    submoduleBounds.y = -submoduleBounds.height / 2;
    if (networkNode->hasPar("canvasImage") && strlen(networkNode->par("canvasImage")) != 0) {
        auto imageFigure = new cImageFigure("node");
        imageFigure->setTooltip("This image represents a network node");
        imageFigure->setTooltip("");
        imageFigure->setAssociatedObject(networkNode);
        imageFigure->setImageName(networkNode->par("canvasImage"));
        if (networkNode->hasPar("canvasImageColor") && strlen(networkNode->par("canvasImageColor")) != 0)
            imageFigure->setTintColor(cFigure::parseColor(networkNode->par("canvasImageColor")));
        addFigure(imageFigure);
    }
}

void NetworkNodeCanvasVisualization::refreshDisplay()
{
    if (isLayoutInvalid) {
        layout();
        isLayoutInvalid = false;
    }
}

void NetworkNodeCanvasVisualization::addAnnotation(cFigure *figure, cFigure::Point size, Placement placementHint, double placementPriority)
{
    annotations.push_back(Annotation(figure, size, placementHint, placementPriority));
    annotationFigure->addFigure(figure);
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
    annotationFigure->removeFigure(figure);
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
    for (const auto& rc: rcs) {
        if (containsPoint(rc, pt))
            return;
    }
    pts.push_back(pt);
}

static double getDistance(const cFigure::Rectangle& rc, const cFigure::Point& pt) {
    if (pt.x <= rc.x && pt.y <= rc.y)
        return pt.distanceTo(getTopLeft(rc));
    else if (rc.x <= pt.x && pt.x <= rc.x + rc.width && pt.y <= rc.y)
        return fabs(pt.y - rc.y);
    else if (pt.x >= rc.x + rc.width && pt.y <= rc.y)
        return pt.distanceTo(getTopRight(rc));
    else if (pt.x <= rc.x && rc.y <= pt.y && pt.y <= rc.y + rc.height)
        return fabs(pt.x - rc.x);
    else if (pt.x >= rc.x + rc.width && rc.y <= pt.y && pt.y <= rc.y + rc.height)
        return fabs(pt.x - (rc.x + rc.width));
    else if (pt.x <= rc.x && pt.y >= rc.y + rc.height)
        return pt.distanceTo(getBottomLeft(rc));
    else if (rc.x <= pt.x && pt.x <= rc.x + rc.width && pt.y >= rc.y + rc.height)
        return fabs(pt.y - (rc.y + rc.height));
    else if (pt.x >= rc.x + rc.width && pt.y >= rc.y + rc.height)
        return pt.distanceTo(getBottomRight(rc));
    else
        return 0;
}

static double getClosestPlacementDistance(const cFigure::Rectangle& rc, Placement placement, const cFigure::Point& pt) {
    double size = 1000;
    double distance = std::numeric_limits<double>::infinity();
    if (placement & PLACEMENT_TOP_LEFT)
        distance = std::min(distance, getDistance(cFigure::Rectangle(rc.x - size, rc.y - size, size, size), pt));
    if (placement & PLACEMENT_TOP_CENTER)
        distance = std::min(distance, getDistance(cFigure::Rectangle(rc.x, rc.y - size, rc.width, size), pt));
    if (placement & PLACEMENT_TOP_RIGHT)
        distance = std::min(distance, getDistance(cFigure::Rectangle(rc.x + rc.width, rc.y - size, size, size), pt));
    if (placement & PLACEMENT_CENTER_LEFT)
        distance = std::min(distance, getDistance(cFigure::Rectangle(rc.x - size, rc.y, size, rc.height), pt));
    if (placement & PLACEMENT_CENTER_RIGHT)
        distance = std::min(distance, getDistance(cFigure::Rectangle(rc.x + rc.width, rc.y, size, rc.height), pt));
    if (placement & PLACEMENT_BOTTOM_LEFT)
        distance = std::min(distance, getDistance(cFigure::Rectangle(rc.x - size, rc.y + rc.height, size, size), pt));
    if (placement & PLACEMENT_BOTTOM_CENTER)
        distance = std::min(distance, getDistance(cFigure::Rectangle(rc.x, rc.y + rc.height, rc.width, size), pt));
    if (placement & PLACEMENT_BOTTOM_RIGHT)
        distance = std::min(distance, getDistance(cFigure::Rectangle(rc.x + rc.width, rc.y + rc.height, size, size), pt));
    return distance;
}

bool NetworkNodeCanvasVisualization::Annotation::comparePlacementPriority(const Annotation& a1, const Annotation& a2) {
    return a1.placementPriority < a2.placementPriority;
}

void NetworkNodeCanvasVisualization::layout()
{
    std::vector<cFigure::Rectangle> rcs;  // rectangles of annotations already positioned
    std::vector<cFigure::Point> pts;  // candidate points where annotations may be positioned
    cFigure::Rectangle extendendSubmoduleBounds = submoduleBounds;
    extendendSubmoduleBounds.x -= annotationSpacing / 2;
    extendendSubmoduleBounds.y -= annotationSpacing / 2;
    extendendSubmoduleBounds.width += annotationSpacing;
    extendendSubmoduleBounds.height += annotationSpacing;
    rcs.push_back(extendendSubmoduleBounds);
    pts.push_back(getTopLeft(extendendSubmoduleBounds));
    pts.push_back(getTopCenter(extendendSubmoduleBounds));
    pts.push_back(getTopRight(extendendSubmoduleBounds));
    pts.push_back(getCenterLeft(extendendSubmoduleBounds));
    pts.push_back(getCenterCenter(extendendSubmoduleBounds));
    pts.push_back(getCenterRight(extendendSubmoduleBounds));
    pts.push_back(getBottomLeft(extendendSubmoduleBounds));
    pts.push_back(getBottomCenter(extendendSubmoduleBounds));
    pts.push_back(getBottomRight(extendendSubmoduleBounds));

    std::sort(annotations.begin(), annotations.end(), Annotation::comparePlacementPriority);

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
        cFigure::Point rs = cFigure::Point(annotation.bounds.width + annotationSpacing, annotation.bounds.height + annotationSpacing);

        // find the best minimizing the distance cost function
        double bestDistance = std::numeric_limits<double>::infinity();
        cFigure::Rectangle bestRc;

        // for all candidate points
        for (auto pt: pts) {
            // align annotation to candidate points with its various points
            for (int k = 0; k < 9; k++) {
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
                        // candidate point is center center
                        candidateRc = createRectangle(pt - cFigure::Point(rs.x / 2, rs.y / 2), rs);
                        break;
                    case 5:
                        // candidate point is center right
                        candidateRc = createRectangle(pt - cFigure::Point(rs.x, rs.y / 2), rs);
                        break;
                    case 6:
                        // candidate point is bottom left
                        candidateRc = createRectangle(pt - cFigure::Point(0, rs.y), rs);
                        break;
                    case 7:
                        // candidate point is bottom center
                        candidateRc = createRectangle(pt - cFigure::Point(rs.x / 2, rs.y), rs);
                        break;
                    case 8:
                        // candidate point is bottom right
                        candidateRc = createRectangle(pt - cFigure::Point(rs.x, rs.y), rs);
                        break;
                    default:
                        throw cRuntimeError("Invalid case");
                }

                double distance = 0;
                if (annotation.placementHint != PLACEMENT_CENTER_CENTER) {
                    distance += getClosestPlacementDistance(submoduleBounds, annotation.placementHint, getTopLeft(candidateRc)) * placementPenalty;
                    distance += getClosestPlacementDistance(submoduleBounds, annotation.placementHint, getTopRight(candidateRc)) * placementPenalty;
                    distance += getClosestPlacementDistance(submoduleBounds, annotation.placementHint, getBottomLeft(candidateRc)) * placementPenalty;
                    distance += getClosestPlacementDistance(submoduleBounds, annotation.placementHint, getBottomRight(candidateRc)) * placementPenalty;

                    // find an already positioned annotation which would intersect the candidate rectangle
                    bool intersects = false;
                    for (const auto& rc: rcs) {
                        if (intersectsRectangle(candidateRc, rc)) {
                            intersects = true;
                            break;
                        }
                    }
                    if (intersects)
                        continue;
                }

                // if better than the current best
                distance += getCenterCenter(submoduleBounds).distanceTo(getCenterCenter(candidateRc));
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
        annotation.bounds.x = bestRc.x + annotationSpacing / 2;
        annotation.bounds.y = bestRc.y + annotationSpacing / 2;
        annotation.figure->setTransform(cFigure::Transform().translate(annotation.bounds.x, annotation.bounds.y));

        // delete candidate points covered by best rc
        for (auto j = pts.begin(); j != pts.end(); ) {
            if (containsPoint(bestRc, *j))
                j = pts.erase(j);
            else
                ++j;
        }

        // push new candidates
        pushUnlessContains(pts, rcs, getTopLeft(bestRc));
        pushUnlessContains(pts, rcs, getTopCenter(bestRc));
        pushUnlessContains(pts, rcs, getTopRight(bestRc));
        pushUnlessContains(pts, rcs, getCenterLeft(bestRc));
        pushUnlessContains(pts, rcs, getCenterCenter(bestRc));
        pushUnlessContains(pts, rcs, getCenterRight(bestRc));
        pushUnlessContains(pts, rcs, getBottomLeft(bestRc));
        pushUnlessContains(pts, rcs, getBottomCenter(bestRc));
        pushUnlessContains(pts, rcs, getBottomRight(bestRc));

        rcs.push_back(bestRc);
    }
}

} // namespace visualizer

} // namespace inet

