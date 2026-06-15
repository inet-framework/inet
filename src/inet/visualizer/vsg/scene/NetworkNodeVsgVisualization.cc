//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/vsg/scene/NetworkNodeVsgVisualization.h"

#include <vsg/maths/transform.h>

#include <algorithm>

#include "inet/visualizer/vsg/util/VsgUtils.h"

namespace inet {

namespace visualizer {

NetworkNodeVsgVisualization::NetworkNodeVsgVisualization(cModule *networkNode, bool displayModuleName) :
    NetworkNodeVisualization(networkNode)
{
    cDisplayString& displayString = networkNode->getDisplayString();

    // TODO: render the node's 2D icon (display string "i" tag) as a textured billboard quad, or
    // its 3D model ("osgModel" par), to match the OSG visualizer. For now a colored box marker.
    double markerSize = 10;
    cFigure::Color color("#4080C0");
    // sit the marker ON the ground (bottom at z=0) rather than centered on the ground plane
    auto mainPart = inet::vsg::createBox(Coord(0, 0, markerSize / 2), Coord(markerSize, markerSize, markerSize), color);
    mainPart->setValue("omnetpp.object", (int64_t)(intptr_t)(cObject *)networkNode); // selectable: click -> select module

    auto group = ::vsg::Group::create();
    group->addChild(mainPart);
    size = ::vsg::dvec3(markerSize, 0, markerSize);

    if (displayModuleName)
        // createLabel is a camera-facing, screen-constant-size billboard, so use OSG's characterSize 18
        // (it no longer scales with the world, so the small value that compensated for that is gone).
        group->addChild(inet::vsg::createLabel(networkNode->getFullName(), Coord(0, 0, markerSize + 2), cFigure::BLACK, 18));

    annotationNode = ::vsg::Group::create();
    group->addChild(annotationNode);
    addChild(group);

    double x = atol(displayString.getTagArg("p", 0));
    double y = atol(displayString.getTagArg("p", 1));
    position = Coord(x, y, 0.0);
    matrix = ::vsg::translate(::vsg::dvec3(x, y, 0.0));
}

void NetworkNodeVsgVisualization::updateAnnotationPositions()
{
    double spacing = 4;
    double totalHeight = 0;
    for (auto& annotation : annotations) {
        double dz = size.z + spacing + totalHeight;
        annotation.transform->matrix = ::vsg::translate(::vsg::dvec3(0, 0, dz));
        totalHeight += annotation.size.z + spacing;
    }
}

void NetworkNodeVsgVisualization::addAnnotation(::vsg::ref_ptr<::vsg::Node> node, ::vsg::dvec3 size, double priority)
{
    auto transform = ::vsg::MatrixTransform::create();
    transform->addChild(node);
    annotations.push_back(Annotation(node, transform, size, priority));
    std::stable_sort(annotations.begin(), annotations.end(), [] (const Annotation& a1, const Annotation& a2) {
        return a1.priority < a2.priority;
    });
    annotationNode->addChild(transform);
    updateAnnotationPositions();
}

void NetworkNodeVsgVisualization::removeAnnotation(::vsg::Node *node)
{
    for (auto it = annotations.begin(); it != annotations.end(); it++) {
        if (it->node.get() == node) {
            auto transform = it->transform;
            auto& ch = annotationNode->children;
            ch.erase(std::remove(ch.begin(), ch.end(), transform), ch.end());
            annotations.erase(it);
            break;
        }
    }
    updateAnnotationPositions();
}

void NetworkNodeVsgVisualization::removeAnnotation(int index)
{
    auto it = annotations.begin() + index;
    auto transform = it->transform;
    auto& ch = annotationNode->children;
    ch.erase(std::remove(ch.begin(), ch.end(), transform), ch.end());
    annotations.erase(it);
    updateAnnotationPositions();
}

} // namespace visualizer

} // namespace inet
